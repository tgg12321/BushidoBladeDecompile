#!/usr/bin/env python3
"""Auto-generate regfix subst rules from pipeline vs target diff.

Usage:
    python3 tools/gen_regfix.py <func_name> [<src_file>]

Runs the build pipeline (via dump_text_indices.py) to get our compiled maspsx
output, reads the target asm from asm/funcs/<func>.s, normalizes both to a
common format, and generates regfix subst rules for register-only differences.

Output: regfix.txt rules ready to append, printed to stdout.
Diagnostics go to stderr.
"""

import re
import subprocess
import sys
from pathlib import Path

# MIPS named-to-numeric register map
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

# Pseudo-instructions that expand to 2 binary instructions
PSEUDO_2WORD = re.compile(
    r'^\s*(?:la|li)\s+\$\d+\s*,',  # la/li with large immediate
    re.IGNORECASE)


def named_to_numeric(text):
    """Replace named registers with numeric form in an instruction string."""
    # Sort by length descending to avoid partial matches ($s1 before $s)
    for name in sorted(REG_NAME_TO_NUM.keys(), key=len, reverse=True):
        num = REG_NAME_TO_NUM[name]
        # Match register name at word boundary (not followed by more alphanumeric)
        text = re.sub(re.escape(name) + r'(?!\w)', num, text)
    return text


def normalize_hex_immediates(text):
    """Convert hex immediates to decimal for consistent comparison."""
    def hex_to_dec(m):
        try:
            return str(int(m.group(0), 16))
        except ValueError:
            return m.group(0)
    # Handle negative hex: -0x38 -> -56
    text = re.sub(r'-0x[0-9a-fA-F]+', lambda m: str(-int(m.group(0)[1:], 16)), text)
    # Handle positive hex: 0x38 -> 56
    text = re.sub(r'0x[0-9a-fA-F]+', hex_to_dec, text)
    # Handle (value >> N) patterns from splat: (0x80000000 >> 16) -> just the result
    text = re.sub(r'\((\d+)\s*>>\s*(\d+)\)', lambda m: str(int(m.group(1)) >> int(m.group(2))), text)
    return text


def normalize_opcode_aliases(text):
    """Normalize opcode aliases to canonical forms."""
    # bnez $r,label -> bne $r,$0,label
    text = re.sub(r'^bnez\s+(\$\d+),', r'bne \1,$0,', text)
    # beqz $r,label -> beq $r,$0,label
    text = re.sub(r'^beqz\s+(\$\d+),', r'beq \1,$0,', text)
    # negu $r1,$r2 -> subu $r1,$0,$r2
    text = re.sub(r'^negu\s+(\$\d+),(\$\d+)', r'subu \1,$0,\2', text)
    # addiu $sp,$sp,-N -> subu $sp,$sp,N (for frame setup, make comparable)
    # Actually: subu $sp,$sp,N is the maspsx form; addiu $sp,$sp,-N is the target form
    # Normalize: addiu $rX,$rY,-N -> subu $rX,$rY,N (when used as frame subtract)
    m = re.match(r'^addiu\s+(\$\d+),(\$\d+),(-\d+)$', text)
    if m and int(m.group(3)) < 0:
        text = f"subu {m.group(1)},{m.group(2)},{-int(m.group(3))}"
    # addiu $rX,$rY,N (positive) at epilogue -> addu $rX,$rY,N
    m = re.match(r'^addiu\s+(\$\d+),(\$\d+),(\d+)$', text)
    if m and m.group(1) == '$29' and m.group(2) == '$29':
        text = f"addu {m.group(1)},{m.group(2)},{m.group(3)}"
    # slti $r1,$r2,N -> slt $r1,$r2,N (for comparison purposes)
    text = re.sub(r'^slti\s+', 'slt ', text)
    # addiu $rX,$rY,N (positive, non-$sp) -> addu $rX,$rY,N
    # GCC emits addu for small immediates, assembler may emit addiu
    m = re.match(r'^addiu\s+(\$\d+),(\$\d+),(\d+)$', text)
    if m and m.group(1) != '$29':
        text = f"addu {m.group(1)},{m.group(2)},{m.group(3)}"
    # jr $31 -> j $31
    text = re.sub(r'^jr\s+(\$31)$', r'j \1', text)
    return text


def normalize_instruction(text):
    """Normalize an instruction for comparison."""
    # Remove comments
    text = re.sub(r'#.*$', '', text)
    # Collapse tabs/spaces to single space
    text = re.sub(r'\s+', ' ', text.strip())
    # Normalize comma and paren spacing: ", " -> ","
    text = re.sub(r'\s*,\s*', ',', text)
    text = re.sub(r'\s*\(\s*', '(', text)
    text = re.sub(r'\s*\)\s*', ')', text)
    # Convert named registers to numeric
    text = named_to_numeric(text)
    # Normalize hex immediates to decimal
    text = normalize_hex_immediates(text)
    # Normalize opcode aliases
    text = normalize_opcode_aliases(text)
    return text


def parse_target_asm(asm_path):
    """Parse asm/funcs/<func>.s and return list of normalized instruction strings.

    Strips the glabel header, comments, and hex bytes.
    Converts named registers to numeric.
    """
    instructions = []
    lines = asm_path.read_text().splitlines()

    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue
        # Skip glabel
        if stripped.startswith('glabel'):
            continue
        # Skip labels
        if stripped.startswith('.') and stripped.endswith(':'):
            continue
        if stripped.endswith(':'):
            continue
        # Skip directives
        if stripped.startswith('.'):
            continue

        # Strip the /* offset addr bytes */ comment prefix
        m = re.match(r'/\*.*?\*/\s*(.*)', stripped)
        if m:
            insn = m.group(1).strip()
        else:
            insn = stripped

        if not insn:
            continue

        # Skip nop (delay slot placeholder in target)
        # Actually, nops ARE real instructions, keep them
        # But skip zero-encoding nops that are padding
        # (nop = sll $zero,$zero,0)

        # Normalize (named_to_numeric is called inside normalize_instruction)
        insn = normalize_instruction(insn)

        if insn:
            instructions.append(insn)

    return instructions


def get_pipeline_instructions(root, func_name, src_file=None):
    """Run dump_text_indices.py and return list of (index, raw_text, normalized_text)."""
    cmd = f"python3 tools/dump_text_indices.py {func_name}"
    if src_file:
        # Use relative path if possible
        try:
            rel = Path(src_file).relative_to(root)
            cmd += f" {rel}"
        except ValueError:
            cmd += f" {src_file}"

    result = subprocess.run(
        ["bash", "-c", cmd],
        capture_output=True, text=True, cwd=str(root)
    )
    if result.returncode != 0:
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


def extract_registers(insn):
    """Extract all register references ($N) from a normalized instruction."""
    return re.findall(r'\$\d+', insn)


def extract_opcode(insn):
    """Extract the opcode (first word) from a normalized instruction."""
    parts = insn.split()
    return parts[0] if parts else ''


def is_branch_or_jump(opcode):
    """Check if opcode is a branch or jump instruction."""
    return opcode in (
        'beq', 'bne', 'bgez', 'bgtz', 'blez', 'bltz',
        'bgezal', 'bltzal', 'j', 'jal', 'jr', 'jalr',
        'beql', 'bnel',  # likely branches
    )


def strip_branch_target(insn):
    """Remove the branch/jump target (label) from instruction for comparison."""
    opcode = extract_opcode(insn)
    if not is_branch_or_jump(opcode):
        return insn
    # For branches: target is last operand (after last comma or space)
    # For j/jal: target is the only operand
    if opcode in ('j', 'jal'):
        return opcode  # just the opcode
    if opcode == 'jr' or opcode == 'jalr':
        return insn  # register operands, keep them
    # Branch: strip last operand (the label)
    # Format: beq $r1,$r2,label or bgez $r1,label
    parts = insn.rsplit(',', 1)
    if len(parts) == 2:
        return parts[0].strip()
    return insn


def is_pseudo_2word(insn):
    """Check if a maspsx instruction expands to 2 binary instructions."""
    norm = insn.strip()
    # la $reg, symbol → lui + addiu/ori
    if re.match(r'la\s+\$\d+\s*,', norm):
        return True
    # li $reg, large_immediate → lui + ori
    m = re.match(r'li\s+\$\d+\s*,\s*(.+)', norm)
    if m:
        val_str = m.group(1).strip()
        try:
            val = int(val_str, 0)
            if val > 0xFFFF or val < -0x8000:
                return True
        except ValueError:
            # Symbol reference or expression
            return True
    # lb/lh/lw/sb/sh/sw with symbol (not register offset) → lui + load/store
    if re.match(r'(?:lb|lbu|lh|lhu|lw|sb|sh|sw)\s+\$\d+\s*,\s*[A-Za-z_]', norm):
        return True
    return False


def align_instructions(ours, target):
    """Align our maspsx instructions with target binary instructions.

    Returns list of (our_idx, our_norm, target_norm) tuples.
    our_idx is the TEXT index. target_norm may be None for inserted pseudo-insn
    expansion. our_norm may be None for extra target instructions.
    """
    aligned = []
    ti = 0  # target instruction pointer

    for our_idx, our_raw, our_norm in ours:
        if ti >= len(target):
            aligned.append((our_idx, our_raw, our_norm, None))
            continue

        tgt = target[ti]

        if is_pseudo_2word(our_norm):
            # This maspsx instruction expands to 2 binary instructions.
            # Skip 2 target instructions.
            aligned.append((our_idx, our_raw, our_norm, f"[pseudo→{tgt}]"))
            ti += 2
        else:
            aligned.append((our_idx, our_raw, our_norm, tgt))
            ti += 1

    # Any remaining target instructions
    while ti < len(target):
        aligned.append((None, None, None, target[ti]))
        ti += 1

    return aligned


def classify_diff(our_norm, tgt_norm):
    """Classify the difference between our instruction and target.

    Returns: ('match', None) | ('register', reg_diffs) | ('structural', desc)
    """
    if our_norm is None or tgt_norm is None:
        return 'structural', 'missing instruction'

    if tgt_norm.startswith('[pseudo'):
        return 'match', None  # pseudo-instruction, skip comparison

    # Strip branch targets for comparison
    our_cmp = strip_branch_target(our_norm)
    tgt_cmp = strip_branch_target(tgt_norm)

    if our_cmp == tgt_cmp:
        return 'match', None

    # Check if only registers differ
    our_opcode = extract_opcode(our_norm)
    tgt_opcode = extract_opcode(tgt_norm)

    if our_opcode != tgt_opcode:
        return 'structural', f'opcode mismatch: {our_opcode} vs {tgt_opcode}'

    our_regs = extract_registers(our_cmp)
    tgt_regs = extract_registers(tgt_cmp)

    if len(our_regs) != len(tgt_regs):
        return 'structural', f'operand count mismatch'

    # Check each register
    reg_diffs = []
    for i, (o, t) in enumerate(zip(our_regs, tgt_regs)):
        if o != t:
            reg_diffs.append((i, o, t))

    if reg_diffs:
        # Verify non-register parts match
        our_no_regs = re.sub(r'\$\d+', 'REG', our_cmp)
        tgt_no_regs = re.sub(r'\$\d+', 'REG', tgt_cmp)
        if our_no_regs != tgt_no_regs:
            return 'structural', f'non-register operand mismatch'
        return 'register', reg_diffs

    # Non-register difference (immediate values, etc.)
    return 'structural', 'immediate or label mismatch'


def escape_pattern_dollar(s):
    """Escape $ in a regfix pattern (LHS). Must use \\$ for literal dollar."""
    return s.replace('$', '\\$')


def gen_subst_rule(func_name, idx, our_raw, tgt_norm, our_norm):
    """Generate a regfix subst rule for a register diff at a given TEXT index.

    Returns the rule string or None if it can't be expressed as a simple subst.
    """
    # We need to find what part of our_raw to match and what to replace with.
    # Strategy: find the minimal unique substring that captures the register diff.

    our_regs = extract_registers(strip_branch_target(our_norm))
    tgt_regs = extract_registers(strip_branch_target(tgt_norm))

    if len(our_regs) != len(tgt_regs):
        return None

    rules = []
    for i, (o, t) in enumerate(zip(our_regs, tgt_regs)):
        if o == t:
            continue
        # Build a pattern that matches this specific register in context
        # Use the raw instruction text for the pattern
        # Escape the our register for pattern, use target register for replacement
        pat = escape_pattern_dollar(o)
        repl = t

        # If the register appears multiple times, we need more context
        count = our_raw.count(o.replace('$', '\t$'))  # tab-prefixed
        count += our_raw.count(o + ',')
        count += our_raw.count(o + '(')

        # Try to build a unique pattern with surrounding context
        # Find the register in the raw text and include neighboring tokens
        # For now, just use the register with some context
        raw_escaped = escape_pattern_dollar(our_raw)

        # Simple approach: match the full instruction with the register to change
        rules.append(
            f'{func_name}: subst "{pat}" "{repl}" @ {idx}'
        )

    return rules


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 tools/gen_regfix.py <func_name> [<src_file>]",
              file=sys.stderr)
        sys.exit(1)

    func_name = sys.argv[1]
    src_file = sys.argv[2] if len(sys.argv) >= 3 else None
    root = Path(__file__).resolve().parent.parent

    # Step 1: Get our pipeline output
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

    # Step 3: Align instruction streams
    aligned = align_instructions(ours, target)

    # Step 4: Compare and generate rules
    reg_diffs = []
    structural_diffs = []
    matches = 0

    for our_idx, our_raw, our_norm, tgt_norm in aligned:
        if our_norm is None or tgt_norm is None:
            if our_norm is not None:
                structural_diffs.append((our_idx, 'extra our instruction', our_norm))
            elif tgt_norm is not None:
                structural_diffs.append((None, 'extra target instruction', tgt_norm))
            continue

        kind, info = classify_diff(our_norm, tgt_norm)
        if kind == 'match':
            matches += 1
        elif kind == 'register':
            reg_diffs.append((our_idx, our_raw, our_norm, tgt_norm, info))
        elif kind == 'structural':
            structural_diffs.append((our_idx, info, f'ours={our_norm} target={tgt_norm}'))

    print(f"\n  Matches: {matches}", file=sys.stderr)
    print(f"  Register diffs: {len(reg_diffs)}", file=sys.stderr)
    print(f"  Structural diffs: {len(structural_diffs)}", file=sys.stderr)

    # Step 5: Generate rules
    if reg_diffs:
        print(f"\n# {func_name}: auto-generated register fixes ({len(reg_diffs)} diffs)")
        for our_idx, our_raw, our_norm, tgt_norm, diff_info in reg_diffs:
            # Comment showing the diff
            for pos, our_reg, tgt_reg in diff_info:
                name_our = REG_NUM_TO_NAME.get(our_reg, our_reg)
                name_tgt = REG_NUM_TO_NAME.get(tgt_reg, tgt_reg)
                print(f"# idx {our_idx}: {our_reg}({name_our}) -> {tgt_reg}({name_tgt})")

            # Generate subst rules
            rules = gen_subst_rule(func_name, our_idx, our_raw, tgt_norm, our_norm)
            if rules:
                for rule in rules:
                    print(rule)
            else:
                print(f"# idx {our_idx}: MANUAL — could not auto-generate rule")
                print(f"#   ours:   {our_norm}")
                print(f"#   target: {tgt_norm}")

    # Report structural diffs
    if structural_diffs:
        print(f"\n# STRUCTURAL DIFFS (cannot fix with regfix):", file=sys.stderr)
        for idx, desc, detail in structural_diffs:
            idx_str = f"idx {idx}" if idx is not None else "???"
            print(f"#   {idx_str}: {desc} — {detail}", file=sys.stderr)


if __name__ == '__main__':
    main()
