#!/usr/bin/env python3
"""Auto-generate regfix rules from pipeline vs target diff.

Usage:
    python3 tools/gen_regfix.py <func_name> [<src_file>]

Runs the build pipeline (via dump_text_indices.py) to get our compiled maspsx
output, reads the target asm from asm/funcs/<func>.s, normalizes both to a
common format, and generates regfix rules for register and structural diffs.

Detects: register swaps, register substs, delay slot fill/unfill, instruction
reorders, instruction count mismatches (insert/delete).

Output: regfix.txt rules ready to append, printed to stdout.
Diagnostics go to stderr.
"""

import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

REG_NAME_TO_NUM = {
    '$zero': '$0', '$at': '$1',
    '$v0': '$2', '$v1': '$3',
    '$a0': '$4', '$a1': '$5', '$a2': '$6', '$a3': '$7',
    '$t0': '$8', '$t1': '$9', '$t2': '$10', '$t3': '$11',
    '$t4': '$12', '$t5': '$13', '$t6': '$14', '$t7': '$15',
    '$s0': '$16', '$s1': '$17', '$s2': '$18', '$s3': '$19',
    '$s4': '$20', '$s5': '$21', '$s6': '$22', '$s7': '$23',
    '$t8': '$24', '$t9': '$25',
    '$k0': '$26', '$k1': '$27',
    '$gp': '$28', '$sp': '$29', '$fp': '$30', '$ra': '$31',
}

REG_NUM_TO_NAME = {v: k for k, v in REG_NAME_TO_NUM.items()}

BRANCH_OPCODES = {
    'beq', 'bne', 'bgez', 'bgtz', 'blez', 'bltz',
    'bgezal', 'bltzal', 'j', 'jal', 'jr', 'jalr',
    'beql', 'bnel',
}


def named_to_numeric(text):
    for name in sorted(REG_NAME_TO_NUM.keys(), key=len, reverse=True):
        num = REG_NAME_TO_NUM[name]
        text = re.sub(re.escape(name) + r'(?!\w)', num, text)
    return text


def normalize_hex_immediates(text):
    text = re.sub(r'-0x[0-9a-fA-F]+', lambda m: str(-int(m.group(0)[1:], 16)), text)
    text = re.sub(r'0x[0-9a-fA-F]+', lambda m: str(int(m.group(0), 16)), text)
    text = re.sub(r'\((\d+)\s*>>\s*(\d+)\)', lambda m: str(int(m.group(1)) >> int(m.group(2))), text)
    return text


def normalize_opcode_aliases(text):
    text = re.sub(r'^bnez\s+(\$\d+),', r'bne \1,$0,', text)
    text = re.sub(r'^beqz\s+(\$\d+),', r'beq \1,$0,', text)
    text = re.sub(r'^negu\s+(\$\d+),(\$\d+)', r'subu \1,$0,\2', text)
    m = re.match(r'^addi?u\s+(\$\d+),(\$\d+),(-\d+)$', text)
    if m and int(m.group(3)) < 0:
        text = f"subu {m.group(1)},{m.group(2)},{-int(m.group(3))}"
    m = re.match(r'^addiu\s+(\$\d+),(\$\d+),(\d+)$', text)
    if m and m.group(1) == '$29' and m.group(2) == '$29':
        text = f"addu {m.group(1)},{m.group(2)},{m.group(3)}"
    text = re.sub(r'^slti\s+', 'slt ', text)
    text = re.sub(r'^sltiu\s+', 'sltu ', text)
    m = re.match(r'^addiu\s+(\$\d+),(\$\d+),(\d+)$', text)
    if m and m.group(1) != '$29':
        text = f"addu {m.group(1)},{m.group(2)},{m.group(3)}"
    text = re.sub(r'^jr\s+(\$\d+)$', r'j \1', text)
    return text


def normalize_instruction(text):
    text = re.sub(r'#.*$', '', text)
    text = re.sub(r'\s+', ' ', text.strip())
    text = re.sub(r'\s*,\s*', ',', text)
    text = re.sub(r'\s*\(\s*', '(', text)
    text = re.sub(r'\s*\)\s*', ')', text)
    text = named_to_numeric(text)
    text = normalize_hex_immediates(text)
    text = normalize_opcode_aliases(text)
    return text


def extract_opcode(insn):
    parts = insn.split()
    return parts[0] if parts else ''


def is_branch_or_jump(opcode):
    return opcode in BRANCH_OPCODES


def strip_branch_target(insn):
    opcode = extract_opcode(insn)
    if not is_branch_or_jump(opcode):
        return insn
    if opcode in ('j', 'jal'):
        # Keep function call targets (for jal), strip labels
        parts = insn.split(None, 1)
        if len(parts) > 1 and parts[1].startswith('.'):
            return opcode
        return insn
    if opcode in ('jr', 'jalr'):
        return insn
    parts = insn.rsplit(',', 1)
    if len(parts) == 2:
        return parts[0].strip()
    return insn


def extract_registers(insn):
    return re.findall(r'\$\d+', insn)


def is_pseudo_2word(insn):
    norm = insn.strip()
    if re.match(r'la\s+\$\d+\s*,', norm):
        return True
    m = re.match(r'li\s+\$\d+\s*,\s*(.+)', norm)
    if m:
        val_str = m.group(1).strip()
        try:
            val = int(val_str, 0)
            if val > 0xFFFF or val < -0x8000:
                return True
        except ValueError:
            return True
    if re.match(r'(?:lb|lbu|lh|lhu|lw|sb|sh|sw)\s+\$\d+\s*,\s*[A-Za-z_]', norm):
        return True
    return False


def parse_target_asm(asm_path):
    instructions = []
    lines = asm_path.read_text().splitlines()
    for line in lines:
        stripped = line.strip()
        if not stripped or stripped.startswith('glabel') or stripped.endswith(':'):
            continue
        if stripped.startswith('.'):
            continue
        if stripped.startswith('jlabel') or stripped.startswith('endlabel'):
            continue
        m = re.match(r'/\*.*?\*/\s*(.*)', stripped)
        insn = m.group(1).strip() if m else stripped
        if not insn:
            continue
        insn = normalize_instruction(insn)
        if insn:
            instructions.append(insn)
    return instructions


def get_pipeline_instructions(root, func_name, src_file=None):
    cmd = f"python3 tools/dump_text_indices.py {func_name}"
    if src_file:
        try:
            rel = Path(src_file).relative_to(root)
            cmd += f" {rel}"
        except ValueError:
            cmd += f" {src_file}"
    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )
    if result.returncode != 0 and not result.stdout.strip():
        return None, result.stderr.strip()
    instructions = []
    for line in result.stdout.splitlines():
        m = re.match(r'\s*(\d+):\s*(.*)', line)
        if m:
            idx = int(m.group(1))
            raw = m.group(2).strip()
            norm = normalize_instruction(raw)
            instructions.append((idx, raw, norm))
    return instructions, None


def align_instructions(ours, target):
    aligned = []
    ti = 0
    for our_idx, our_raw, our_norm in ours:
        if ti >= len(target):
            aligned.append((our_idx, our_raw, our_norm, None))
            continue
        tgt = target[ti]
        if is_pseudo_2word(our_raw):
            aligned.append((our_idx, our_raw, our_norm, f"[pseudo→{tgt}]"))
            ti += 2
        else:
            aligned.append((our_idx, our_raw, our_norm, tgt))
            ti += 1
    while ti < len(target):
        aligned.append((None, None, None, target[ti]))
        ti += 1
    return aligned


def classify_diff(our_norm, tgt_norm):
    if our_norm is None or tgt_norm is None:
        return 'structural', 'missing instruction'
    if tgt_norm.startswith('[pseudo'):
        return 'match', None
    our_cmp = strip_branch_target(our_norm)
    tgt_cmp = strip_branch_target(tgt_norm)
    if our_cmp == tgt_cmp:
        return 'match', None
    our_opcode = extract_opcode(our_norm)
    tgt_opcode = extract_opcode(tgt_norm)
    if our_opcode != tgt_opcode:
        return 'structural', f'opcode: {our_opcode} vs {tgt_opcode}'
    our_regs = extract_registers(our_cmp)
    tgt_regs = extract_registers(tgt_cmp)
    if len(our_regs) != len(tgt_regs):
        return 'structural', 'operand count mismatch'
    reg_diffs = []
    for i, (o, t) in enumerate(zip(our_regs, tgt_regs)):
        if o != t:
            reg_diffs.append((i, o, t))
    if reg_diffs:
        our_no_regs = re.sub(r'\$\d+', 'REG', our_cmp)
        tgt_no_regs = re.sub(r'\$\d+', 'REG', tgt_cmp)
        if our_no_regs == tgt_no_regs:
            return 'register', reg_diffs
        if sorted(our_regs) == sorted(tgt_regs):
            return 'operand_order', (our_cmp, tgt_cmp)
        return 'register', reg_diffs
    # Same opcode, same registers, but different non-register operands
    # Filter: jump table label diffs (.L<N> vs jtbl_XXX) are resolved by the
    # assembler and match in binary — not real diffs
    our_labels = re.findall(r'\.L\d+|jtbl_[0-9A-Fa-f]+', our_cmp)
    tgt_labels = re.findall(r'\.L\d+|jtbl_[0-9A-Fa-f]+', tgt_cmp)
    if our_labels or tgt_labels:
        our_no_lbl = re.sub(r'%(?:hi|lo)\([^)]+\)', '%SYM', our_cmp)
        tgt_no_lbl = re.sub(r'%(?:hi|lo)\([^)]+\)', '%SYM', tgt_cmp)
        if our_no_lbl == tgt_no_lbl:
            return 'match', None
    return 'immediate', (our_cmp, tgt_cmp)


# ---------------------------------------------------------------------------
# SWAP DETECTION — find register pairs that swap bidirectionally over a range
# ---------------------------------------------------------------------------

def detect_swaps(reg_diffs):
    """Given list of (idx, raw, our_norm, tgt_norm, [(pos, our_reg, tgt_reg)]),
    detect bidirectional register swaps and return (swap_rules, remaining_substs).

    A swap is detected when the same pair (A,B) appears as both A→B and B→A
    across multiple instructions in a contiguous range.
    """
    # Collect all register mappings: (our_reg, tgt_reg) -> [indices]
    pair_indices = defaultdict(list)
    for idx, raw, our_norm, tgt_norm, diffs in reg_diffs:
        for pos, our_reg, tgt_reg in diffs:
            pair_indices[(our_reg, tgt_reg)].append(idx)

    # Find bidirectional pairs: (A,B) and (B,A) both present
    swap_pairs = set()
    for (a, b) in pair_indices:
        if (b, a) in pair_indices and a < b:
            swap_pairs.add((a, b))

    if not swap_pairs:
        return [], reg_diffs

    swap_rules = []
    remaining = []

    for reg_a, reg_b in sorted(swap_pairs):
        ab_indices = set(pair_indices[(reg_a, reg_b)])
        ba_indices = set(pair_indices[(reg_b, reg_a)])
        all_indices = sorted(ab_indices | ba_indices)

        # Find contiguous ranges (allow gaps up to 20 instructions)
        ranges = []
        range_start = all_indices[0]
        range_end = all_indices[0]
        for idx in all_indices[1:]:
            if idx - range_end <= 20:
                range_end = idx
            else:
                ranges.append((range_start, range_end))
                range_start = idx
                range_end = idx
        ranges.append((range_start, range_end))

        for start, end in ranges:
            swap_rules.append((reg_a, reg_b, start, end))

    # Remove diffs that are covered by swaps
    swap_covered = set()
    for reg_a, reg_b, start, end in swap_rules:
        for idx, raw, our_norm, tgt_norm, diffs in reg_diffs:
            if start <= idx <= end:
                new_diffs = []
                for pos, our_reg, tgt_reg in diffs:
                    pair = (min(our_reg, tgt_reg), max(our_reg, tgt_reg))
                    if pair == (min(reg_a, reg_b), max(reg_a, reg_b)):
                        swap_covered.add((idx, pos))
                    else:
                        new_diffs.append((pos, our_reg, tgt_reg))

    for idx, raw, our_norm, tgt_norm, diffs in reg_diffs:
        leftover = [(p, o, t) for p, o, t in diffs
                     if (idx, p) not in swap_covered]
        if leftover:
            remaining.append((idx, raw, our_norm, tgt_norm, leftover))

    return swap_rules, remaining


# ---------------------------------------------------------------------------
# DELAY SLOT DETECTION — find nop -> instruction + delete patterns
# ---------------------------------------------------------------------------

def detect_delay_slot_fills(ours, aligned):
    """Detect cases where our output has a nop in a delay slot that the target
    fills with the following instruction (and the target has 2 fewer instructions).

    Pattern: our[i] = branch, our[i+1] = nop, our[i+2] = X
             target[i] = branch, target[i+1] = X, target doesn't have the extra X

    Returns list of (branch_idx, nop_idx, fill_source_idx, fill_insn) and
    updated aligned list with those diffs removed.
    """
    fills = []
    our_norms = {idx: norm for idx, raw, norm in ours}
    our_raws = {idx: raw for idx, raw, norm in ours}

    for i in range(len(ours) - 2):
        idx_br, raw_br, norm_br = ours[i]
        idx_nop, raw_nop, norm_nop = ours[i + 1]
        idx_fill, raw_fill, norm_fill = ours[i + 2]

        if norm_nop != 'nop':
            continue
        if not is_branch_or_jump(extract_opcode(norm_br)):
            continue
        if idx_nop != idx_br + 1 or idx_fill != idx_br + 2:
            continue

        # Check if target has the fill instruction in the delay slot position
        for a_idx, a_raw, a_norm, a_tgt in aligned:
            if a_idx == idx_nop and a_tgt is not None:
                tgt_stripped = strip_branch_target(a_tgt)
                fill_stripped = strip_branch_target(norm_fill)
                # Normalize both for register-independent comparison
                tgt_no_regs = re.sub(r'\$\d+', 'REG', tgt_stripped)
                fill_no_regs = re.sub(r'\$\d+', 'REG', fill_stripped)
                if tgt_no_regs == fill_no_regs:
                    fills.append((idx_br, idx_nop, idx_fill, raw_fill))
                break

    return fills


# ---------------------------------------------------------------------------
# EARLY-EXIT ALIAS BREAKER — detect GCC's `move v0,a0;beqz` optimization
# ---------------------------------------------------------------------------

def detect_early_exit_alias(ours, target):
    """Detect GCC's "move v0,arg;beqz arg" early-exit aliasing optimization
    that the target binary emits as the more verbose 4-insn prelude.

    Compiled pattern (ours):
        idx 0: subu $sp,$sp,N
        idx 1: beq $X,$0,.LBL_end  (X = arg register, e.g. $4)
        idx 2: addu $V,$X,$0       (V = $2 = $v0; alias arg as v0)
        idx 3+: compute body using $X and $V

    Target pattern:
        idx 0: bne $X,$0,.LBL_compute  (skip early-exit if non-null)
        idx 1: subu $sp,$sp,N           (delay slot — stack alloc)
        idx 2: j .LBL_end                (jump to end)
        idx 3: addu $V,$0,$0             (delay slot — explicit zero)
        compute_lbl:
        idx 4+: compute body using $X (no alias)

    Returns dict with the recipe components, or None if pattern doesn't match.
    See memory/feedback_early_exit_alias_breaker.md for the full recipe.
    """
    if len(ours) < 3 or len(target) < 4:
        return None

    # Inspect ours[0..2]
    o0 = ours[0][2]  # normalized
    o1 = ours[1][2]
    o2 = ours[2][2]

    m0 = re.match(r'^subu\s+\$29,\$29,(\d+)$', o0) or \
         re.match(r'^addu\s+\$29,\$29,(-\d+)$', o0)
    if not m0:
        return None
    sp_imm = m0.group(1).lstrip('-')

    # ours[1] is `beq $X,$0,.<label>`
    m1 = re.match(r'^beq\s+(\$\d+),\$0,(\S+)$', o1) or \
         re.match(r'^bne\s+(\$\d+),\$0,(\S+)$', o1)
    if not m1:
        return None
    arg_reg = m1.group(1)
    end_label = m1.group(2)

    # ours[2] is `addu $V,$X,$0` aliasing arg into v_reg (typically $2)
    m2 = re.match(r'^addu\s+(\$\d+),(\$\d+),\$0$', o2)
    if not m2:
        return None
    v_reg = m2.group(1)
    if m2.group(2) != arg_reg:
        return None

    # Now check target's pattern
    t0 = target[0]
    t1 = target[1]
    t2 = target[2]
    t3 = target[3]

    tm0 = re.match(r'^bne\s+(\$\d+),\$0,(\S+)$', t0) or \
          re.match(r'^beq\s+(\$\d+),\$0,(\S+)$', t0)
    if not tm0 or tm0.group(1) != arg_reg:
        return None
    compute_label = tm0.group(2)

    # target[1] should be the same subu sp (now in delay slot)
    tm1 = re.match(r'^subu\s+\$29,\$29,(\d+)$', t1) or \
          re.match(r'^addu\s+\$29,\$29,(-\d+)$', t1)
    if not tm1 or tm1.group(1).lstrip('-') != sp_imm:
        return None

    # target[2] should be `j .LBL_end`
    tm2 = re.match(r'^j\s+(\S+)$', t2)
    if not tm2:
        return None
    target_end_label = tm2.group(1)

    # target[3] should be `addu $V,$0,$0` (or addu/move with both src zero)
    tm3 = re.match(r'^addu\s+(\$\d+),\$0,\$0$', t3)
    if not tm3 or tm3.group(1) != v_reg:
        return None

    # Detect the first arg-deref in compute body that uses $V instead of $X
    # (Compiled body uses $V because GCC aliased arg into v_reg.)
    first_v_deref_idx = None
    for o_idx, o_raw, o_norm in ours[3:]:
        # Look for memory ops dereferencing $V: e.g., `lbu $X,N($V)`
        if re.search(rf'\(\{re.escape(v_reg)}\)', o_norm):
            first_v_deref_idx = o_idx
            break

    return {
        'sp_imm': int(sp_imm),
        'arg_reg': arg_reg,
        'v_reg': v_reg,
        'end_label_in_ours': end_label,
        'compute_label_in_target': compute_label,
        'end_label_in_target': target_end_label,
        'first_v_deref_idx': first_v_deref_idx,
    }


# ---------------------------------------------------------------------------
# VARARGS PROLOGUE DETECTOR — pre-subu vs post-subu spill ordering
# ---------------------------------------------------------------------------

def detect_varargs_prologue_mismatch(ours, target):
    """Detect the varargs-prologue ordering mismatch where GCC emits
    `sw $aN-$a3` spills BEFORE subu sp at caller-arg offsets, but the target
    spills AFTER subu sp at offsets shifted by the frame size.

    The leading spills may be a prefix of $a0-$a3 (e.g. only $a1-$a3 if $a0
    is a named arg whose address isn't explicitly taken — debug_printf-style).
    Detected when we see at least 2 consecutive `sw $aR,offset($sp)` where
    R ∈ {4,5,6,7} and offset matches caller-arg-slot conventions, followed by
    a subu sp, AND the target starts directly with subu sp (no pre-subu
    spills).

    Returns dict with diagnostic info, or None if pattern doesn't match.
    See memory/feedback_varargs_prologue_pattern.md.
    """
    if len(ours) < 3:
        return None

    # Scan leading consecutive sw $aR,offset($sp) instructions where R∈{4,5,6,7}
    # and offset is the caller's a<R-4> slot (offset = (R-4)*4).
    found_pre_subu_spills = []
    for i, (o_idx, o_raw, o_norm) in enumerate(ours):
        m = re.match(r'^sw\s+\$(\d+),(-?\d+)\(\$29\)$', o_norm)
        if not m:
            break
        reg_num = int(m.group(1))
        offset = int(m.group(2))
        if reg_num not in (4, 5, 6, 7):
            break
        # Caller's argument-save area starts at offset 0 of the caller's sp.
        # Each register slot is 4 bytes. Pre-subu addressing puts $aN at
        # offset (N-4)*4 from the (yet-to-be-decremented) sp.
        if offset != (reg_num - 4) * 4:
            break
        found_pre_subu_spills.append((o_idx, reg_num, offset))

    if len(found_pre_subu_spills) < 2:
        return None

    # The next instruction should be subu sp,sp,N
    next_idx = len(found_pre_subu_spills)
    if next_idx >= len(ours):
        return None
    o_next = ours[next_idx][2]
    m_subu = re.match(r'^subu\s+\$29,\$29,(\d+)$', o_next)
    if not m_subu:
        return None
    frame_size = int(m_subu.group(1))

    # Check that target does NOT have these pre-subu spills — instead it
    # starts with subu sp (target[0] should be the subu).
    if not target:
        return None
    tm = re.match(r'^subu\s+\$29,\$29,(\d+)$', target[0]) or \
         re.match(r'^addu\s+\$29,\$29,(-\d+)$', target[0])
    if not tm:
        return None
    if tm.group(1).lstrip('-') != str(frame_size):
        return None

    return {
        'frame_size': frame_size,
        'pre_subu_spills': found_pre_subu_spills,
        'subu_idx_in_ours': next_idx,
    }


# ---------------------------------------------------------------------------
# INSTRUCTION REORDER DETECTION — find swapped instruction pairs
# ---------------------------------------------------------------------------

def detect_instruction_swaps(aligned):
    """Detect pairs of instructions that are swapped between our output and target.

    Pattern: our[i]=A, our[i+1]=B but target[i]=B, target[i+1]=A
    """
    swaps = []
    for i in range(len(aligned) - 1):
        idx1, raw1, norm1, tgt1 = aligned[i]
        idx2, raw2, norm2, tgt2 = aligned[i + 1]
        if norm1 is None or norm2 is None or tgt1 is None or tgt2 is None:
            continue
        if tgt1.startswith('[pseudo') or tgt2.startswith('[pseudo'):
            continue

        # Check if swapped: our[i] matches target[i+1] and vice versa
        n1_cmp = strip_branch_target(norm1)
        n2_cmp = strip_branch_target(norm2)
        t1_cmp = strip_branch_target(tgt1)
        t2_cmp = strip_branch_target(tgt2)

        # Register-blind comparison
        n1_blind = re.sub(r'\$\d+', 'REG', n1_cmp)
        n2_blind = re.sub(r'\$\d+', 'REG', n2_cmp)
        t1_blind = re.sub(r'\$\d+', 'REG', t1_cmp)
        t2_blind = re.sub(r'\$\d+', 'REG', t2_cmp)

        if n1_blind == t2_blind and n2_blind == t1_blind and n1_blind != n2_blind:
            swaps.append((idx1, idx2, raw1, raw2, tgt1, tgt2))

    return swaps


def escape_pattern_dollar(s):
    return s.replace('$', '\\$')


def norm_to_raw(norm):
    """Convert normalized instruction back to tab-separated raw format for regfix."""
    parts = norm.split(None, 1)
    if len(parts) == 2:
        return f'{parts[0]}\t{parts[1]}'
    return norm


def gen_subst_rules(func_name, idx, our_raw, diffs):
    rules = []
    for pos, our_reg, tgt_reg in diffs:
        pat = escape_pattern_dollar(our_reg)
        rules.append(f'{func_name}: subst "{pat}" "{tgt_reg}" @ {idx}')
    return rules


def format_swap_rule(func_name, reg_a, reg_b, start, end):
    return f'{func_name}: {reg_a} <-> {reg_b} @ {start}-{end}'


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 tools/gen_regfix.py <func_name> [<src_file>]",
              file=sys.stderr)
        sys.exit(1)

    func_name = sys.argv[1]
    src_file = sys.argv[2] if len(sys.argv) >= 3 else None
    root = Path(__file__).resolve().parent.parent

    # Step 1: Get pipeline output
    print(f"Running pipeline for {func_name}...", file=sys.stderr)
    ours, err = get_pipeline_instructions(root, func_name, src_file)
    if ours is None:
        print(f"ERROR: Pipeline failed: {err}", file=sys.stderr)
        sys.exit(1)
    print(f"  Our instructions: {len(ours)}", file=sys.stderr)

    # Step 2: Get target asm
    asm_path = root / 'asm' / 'funcs' / f'{func_name}.s'
    if not asm_path.exists():
        print(f"ERROR: {asm_path} not found", file=sys.stderr)
        sys.exit(1)
    target = parse_target_asm(asm_path)
    print(f"  Target instructions: {len(target)}", file=sys.stderr)

    # Step 3: Count mismatch analysis
    our_count = len(ours)
    tgt_count = len(target)
    # Adjust for pseudo-instructions that expand to 2 binary insns
    pseudo_count = sum(1 for _, raw, _ in ours if is_pseudo_2word(raw))
    our_binary = our_count + pseudo_count
    print(f"  Our binary insns: {our_binary} ({pseudo_count} pseudo expansions)",
          file=sys.stderr)
    print(f"  Target binary insns: {tgt_count}", file=sys.stderr)
    if our_binary != tgt_count:
        delta = our_binary - tgt_count
        print(f"  SIZE MISMATCH: {'+' if delta > 0 else ''}{delta} binary instructions "
              f"({'+' if delta > 0 else ''}{delta * 4} bytes)", file=sys.stderr)

    # Step 4: Align and classify
    aligned = align_instructions(ours, target)

    reg_diffs = []
    structural_diffs = []
    operand_order_diffs = []
    immediate_diffs = []
    matches = 0

    for our_idx, our_raw, our_norm, tgt_norm in aligned:
        if our_norm is None or tgt_norm is None:
            if our_norm is not None:
                structural_diffs.append((our_idx, 'extra ours', our_norm, ''))
            elif tgt_norm is not None:
                structural_diffs.append((None, 'extra target', '', tgt_norm))
            continue
        kind, info = classify_diff(our_norm, tgt_norm)
        if kind == 'match':
            matches += 1
        elif kind == 'register':
            reg_diffs.append((our_idx, our_raw, our_norm, tgt_norm, info))
        elif kind == 'operand_order':
            operand_order_diffs.append((our_idx, our_raw, info[0], info[1]))
        elif kind == 'immediate':
            immediate_diffs.append((our_idx, our_raw, info[0], info[1]))
        elif kind == 'structural':
            structural_diffs.append((our_idx, info, our_norm, tgt_norm))

    print(f"\n  Matches: {matches}", file=sys.stderr)
    print(f"  Register diffs: {len(reg_diffs)}", file=sys.stderr)
    print(f"  Operand order diffs: {len(operand_order_diffs)}", file=sys.stderr)
    print(f"  Immediate diffs: {len(immediate_diffs)}", file=sys.stderr)
    print(f"  Structural diffs: {len(structural_diffs)}", file=sys.stderr)

    # Step 5: Detect patterns
    delay_fills = detect_delay_slot_fills(ours, aligned)
    insn_swaps = detect_instruction_swaps(aligned)
    swap_rules, remaining_reg_diffs = detect_swaps(reg_diffs)
    early_exit_match = detect_early_exit_alias(ours, target)
    varargs_match = detect_varargs_prologue_mismatch(ours, target)

    if delay_fills:
        print(f"  Delay slot fills detected: {len(delay_fills)}", file=sys.stderr)
    if insn_swaps:
        print(f"  Instruction swaps detected: {len(insn_swaps)}", file=sys.stderr)
    if swap_rules:
        print(f"  Register swaps detected: {len(swap_rules)}", file=sys.stderr)
    if early_exit_match:
        print(f"  Early-exit alias pattern DETECTED (see memory/feedback_early_exit_alias_breaker.md)",
              file=sys.stderr)
    if varargs_match:
        print(f"  Varargs prologue mismatch DETECTED (see memory/feedback_varargs_prologue_pattern.md)",
              file=sys.stderr)

    # Step 6: Generate rules
    print(f"\n# {func_name}: auto-generated regfix rules")
    print(f"# matches={matches}, reg_diffs={len(reg_diffs)}, "
          f"operand_order={len(operand_order_diffs)}, "
          f"immediate={len(immediate_diffs)}, "
          f"structural={len(structural_diffs)}")

    # 6.early: Early-exit alias breaker (8-rule recipe with synthesized label)
    if early_exit_match:
        e = early_exit_match
        addr_suffix = func_name.split('_')[-1] if '_' in func_name else 'XXX'
        # Pick a label that won't collide with GCC's `.L<N>` namespace.
        synth_label = f'.LC{addr_suffix[-3:]}' if len(addr_suffix) >= 3 else f'.LC{addr_suffix}'
        sp = e['sp_imm']
        v = e['v_reg']  # numeric form, e.g. $2
        a = e['arg_reg']  # numeric form, e.g. $4
        end = e['end_label_in_ours']
        first_v = e['first_v_deref_idx']
        # Map back to named forms (maspsx output uses named regs like $sp, $zero).
        # NOTE: maspsx output uses $sp, $zero AND numeric forms inconsistently
        # across functions; we use the numeric form which works for both since
        # `\$29` and `\$sp` both appear in different positions. The patterns
        # below use a flexible alternation to accommodate both.
        sp_name = '\\$(?:sp|29)'  # $sp or $29
        zero_name = '\\$(?:zero|0)'  # $zero or $0
        print(f"\n# {func_name}: early-exit alias-breaker recipe")
        print(f"# Pattern: GCC's `move {v},{a};beqz {a}` aliases — target uses explicit zero")
        print(f"# Recipe (see memory/feedback_early_exit_alias_breaker.md):")
        # Match a flexible label-end pattern (\.L\d+) since labels shift
        # between isolated permuter and full-file builds.
        print(f'{func_name}: subst "subu\\s+{sp_name},{sp_name},{sp}" '
              f'"bnez\\t{a},{synth_label}" @ 0')
        print(f'{func_name}: subst "beq\\s+{re.escape(a)},{zero_name},\\.L\\d+" '
              f'"subu\\t$sp,$sp,{sp}" @ 1')
        print(f'{func_name}: subst "addu\\s+{re.escape(v)},{re.escape(a)},{zero_name}" '
              f'"addu\\t{v},$0,$0" @ 2')
        if first_v is not None:
            # Use re.escape on the v_reg numeric form for the pattern, but the
            # replacement uses named/numeric matching the source's typical form.
            print(f'# idx {first_v}: rewrite ({v}) -> ({a}) for arg deref')
            print(f'{func_name}: subst "\\({re.escape(v)}\\)" '
                  f'"({a})" @ {first_v}')
        print(f'{func_name}: insert "j\\t{end}" @ 2')
        print(f'{func_name}: insert_label "{synth_label}:" @ 3')
        print(f"# NOTE: Verify {synth_label} is unique vs GCC's .L<N> labels in the file.")
        print(f"# NOTE: srl→sra fixes for masked-shift idioms in body are NOT auto-emitted;")
        print(f"#       check ours-vs-target diff for `srl X,Y,3` after `andi Y,Y,0xFF`")
        print(f"#       and add `subst \"srl\\\\s+...\" \"sra\\\\t...\" @ <idx>` rules.")
        print(f"# After applying, run `dc.sh dump-text {func_name} --post-regfix` to verify.")

    # 6.varargs: Varargs prologue ordering recipe
    if varargs_match:
        v = varargs_match
        frame = v['frame_size']
        spills = v['pre_subu_spills']
        print(f"\n# {func_name}: varargs prologue ordering recipe")
        print(f"# Pattern: GCC emits {len(spills)} sw $a* spills BEFORE subu sp;")
        print(f"# target spills AFTER subu sp at offsets {frame}+0, {frame}+4, ...")
        print(f"# Recipe (see memory/feedback_varargs_prologue_pattern.md):")
        # Delete the leading pre-subu spills.
        for orig_idx, reg_num, _ in spills:
            print(f'{func_name}: delete @ {orig_idx}  # remove pre-subu sw $a{reg_num - 4}')
        print(f"# After deletes, the original idx N shifts to (N - {len(spills)}).")
        print(f"# Re-insert post-subu spills using insert_after at positions that match")
        print(f"# the target's interleaving with the call-arg setup (see target asm).")
        print(f"# Typical scaffold (positions need adjustment per target):")
        for _, reg_num, _ in spills:
            offset = frame + (reg_num - 4) * 4
            print(f'#   {func_name}: insert_after "sw\\t$a{reg_num - 4},{offset}($sp)" '
                  f'@ <pos>')
        print(f"# This recipe is a SCAFFOLD — finish manually after inspecting the")
        print(f"# target asm to determine the exact insertion positions.")

    # 6a: Delay slot fills (subst nop -> insn, delete original)
    if delay_fills:
        print(f"\n# {func_name}: delay slot fills ({len(delay_fills)} found)")
        for br_idx, nop_idx, fill_idx, fill_raw in delay_fills:
            # Find what the target has in the delay slot position
            for a_idx, a_raw, a_norm, a_tgt in aligned:
                if a_idx == nop_idx and a_tgt is not None:
                    # Use the target's instruction (with correct registers)
                    # We need to un-normalize it back to raw form
                    # Use the fill_raw from our output as template
                    print(f'# idx {nop_idx}: fill delay slot of branch at {br_idx}')
                    print(f'# target wants: {a_tgt}')
                    esc_raw = escape_pattern_dollar(fill_raw)
                    print(f'{func_name}: subst "nop" "{fill_raw}" @ {nop_idx}')
                    print(f'{func_name}: delete @ {fill_idx}')
                    break

    # 6b: Register swaps
    if swap_rules:
        print(f"\n# {func_name}: register swaps")
        for reg_a, reg_b, start, end in swap_rules:
            name_a = REG_NUM_TO_NAME.get(reg_a, reg_a)
            name_b = REG_NUM_TO_NAME.get(reg_b, reg_b)
            print(f'# {reg_a}({name_a}) <-> {reg_b}({name_b}) in [{start},{end}]')
            print(format_swap_rule(func_name, reg_a, reg_b, start, end))

    # 6c: Remaining register substs
    if remaining_reg_diffs:
        print(f"\n# {func_name}: register substs ({len(remaining_reg_diffs)} instructions)")
        for idx, raw, our_norm, tgt_norm, diffs in remaining_reg_diffs:
            comment_parts = []
            for pos, our_reg, tgt_reg in diffs:
                name_o = REG_NUM_TO_NAME.get(our_reg, our_reg)
                name_t = REG_NUM_TO_NAME.get(tgt_reg, tgt_reg)
                comment_parts.append(f'{our_reg}({name_o})->{tgt_reg}({name_t})')
            print(f'# idx {idx}: {", ".join(comment_parts)}')
            rules = gen_subst_rules(func_name, idx, raw, diffs)
            for rule in rules:
                print(rule)

    # 6d: Operand order diffs (e.g. addu $4,$16,$5 -> addu $4,$5,$16)
    if operand_order_diffs:
        print(f"\n# {func_name}: operand order substs ({len(operand_order_diffs)} instructions)")
        for idx, raw, our_cmp, tgt_cmp in operand_order_diffs:
            our_raw_fmt = norm_to_raw(our_cmp)
            tgt_raw_fmt = norm_to_raw(tgt_cmp)
            print(f'# idx {idx}: {our_cmp} -> {tgt_cmp}')
            pat = escape_pattern_dollar(our_raw_fmt)
            print(f'{func_name}: subst "{pat}" "{tgt_raw_fmt}" @ {idx}')

    # 6e: Immediate/offset diffs (e.g. 20($sp) -> 16($sp))
    if immediate_diffs:
        print(f"\n# {func_name}: immediate/offset substs ({len(immediate_diffs)} instructions)")
        for idx, raw, our_cmp, tgt_cmp in immediate_diffs:
            our_raw_fmt = norm_to_raw(our_cmp)
            tgt_raw_fmt = norm_to_raw(tgt_cmp)
            print(f'# idx {idx}: {our_cmp} -> {tgt_cmp}')
            pat = escape_pattern_dollar(our_raw_fmt)
            print(f'{func_name}: subst "{pat}" "{tgt_raw_fmt}" @ {idx}')

    # 6f: Instruction swaps (full-instruction subst pairs)
    if insn_swaps:
        print(f"\n# {func_name}: instruction reorders ({len(insn_swaps)} swaps)")
        for idx1, idx2, raw1, raw2, tgt1, tgt2 in insn_swaps:
            print(f'# idx {idx1},{idx2}: swap instructions')
            print(f'# MANUAL: use full-instruction subst or reorder')
            print(f'#   idx {idx1}: ours="{raw1}" -> target wants "{tgt2}"')
            print(f'#   idx {idx2}: ours="{raw2}" -> target wants "{tgt1}"')

    # 6g: Structural diffs (manual intervention needed)
    if structural_diffs:
        print(f"\n# {func_name}: STRUCTURAL DIFFS ({len(structural_diffs)} — need manual fix)")
        for idx, desc, our, tgt in structural_diffs:
            idx_str = f"idx {idx}" if idx is not None else "???"
            print(f'# {idx_str}: {desc}')
            if our:
                print(f'#   ours:   {our}')
            if tgt:
                print(f'#   target: {tgt}')


if __name__ == '__main__':
    main()
