#!/usr/bin/env python3
"""LCS-based instruction alignment for regfix generation.

Unlike gen_regfix.py (positional alignment), this uses difflib's
SequenceMatcher (LCS-based) to properly handle instruction count
mismatches. When our output has fewer instructions than the target,
it identifies exactly where insertions are needed.

Usage:
    python3 tools/lcs_align.py <func_name> [<src_file>]
"""

import difflib
import re
import subprocess
import sys
from pathlib import Path

# Import normalization from gen_regfix
sys.path.insert(0, str(Path(__file__).parent))
from gen_regfix import (
    normalize_instruction, is_pseudo_2word, parse_target_asm,
    named_to_numeric, REG_NUM_TO_NAME, escape_pattern_dollar
)


def get_pipeline_text(root, func_name, src_file=None):
    """Get our function's TEXT stream from dump_text_indices.py."""
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
            instructions.append((idx, raw))
    return instructions, None


def normalize_for_comparison(insn):
    """Normalize instruction for LCS comparison.

    Strips branch targets (label names differ between our output and target),
    normalizes registers to numeric form, normalizes hex.
    """
    norm = normalize_instruction(insn)
    # Strip branch target labels for comparison
    parts = norm.split()
    if not parts:
        return norm
    opcode = parts[0]

    branch_ops = {'beq', 'bne', 'bgez', 'bgtz', 'blez', 'bltz',
                  'bgezal', 'bltzal', 'j', 'jal', 'beql', 'bnel'}

    if opcode in ('j', 'jal'):
        if len(parts) > 1 and parts[1].startswith('.'):
            return opcode  # Strip local label
        # Keep function call targets
        return norm
    if opcode in branch_ops:
        # Strip the last comma-separated field (the label)
        comma_parts = norm.rsplit(',', 1)
        if len(comma_parts) == 2:
            return comma_parts[0].strip()

    # Strip %hi/%lo label differences but keep the instruction structure
    norm = re.sub(r'%(?:hi|lo)\([^)]+\)', '%SYM', norm)

    return norm


def lcs_align(our_insns, target_insns):
    """Use SequenceMatcher to align instruction streams.

    Returns list of (tag, our_indices, target_indices) where tag is
    'equal', 'replace', 'insert', or 'delete'.
    """
    # Create normalized sequences for comparison
    our_norm = [normalize_for_comparison(raw) for _, raw in our_insns]
    tgt_norm = [normalize_for_comparison(insn) for insn in target_insns]

    sm = difflib.SequenceMatcher(None, our_norm, tgt_norm, autojunk=False)
    opcodes = sm.get_opcodes()
    return opcodes


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 tools/lcs_align.py <func_name> [<src_file>]",
              file=sys.stderr)
        sys.exit(1)

    func_name = sys.argv[1]
    src_file = sys.argv[2] if len(sys.argv) >= 3 else None
    root = Path(__file__).resolve().parent.parent

    # Get our pipeline output
    print(f"Running pipeline for {func_name}...", file=sys.stderr)
    ours, err = get_pipeline_text(root, func_name, src_file)
    if ours is None:
        print(f"ERROR: Pipeline failed: {err}", file=sys.stderr)
        sys.exit(1)

    # Get target asm
    asm_path = root / 'asm' / 'funcs' / f'{func_name}.s'
    target = parse_target_asm(asm_path)

    pseudo_count = sum(1 for _, raw in ours if is_pseudo_2word(raw))
    print(f"  Our TEXT insns: {len(ours)} ({pseudo_count} pseudo → {len(ours)+pseudo_count} binary)",
          file=sys.stderr)
    print(f"  Target binary insns: {len(target)}", file=sys.stderr)

    # LCS alignment
    our_norm = [normalize_for_comparison(raw) for _, raw in ours]
    tgt_norm = [normalize_for_comparison(insn) for insn in target]

    sm = difflib.SequenceMatcher(None, our_norm, tgt_norm, autojunk=False)
    opcodes = sm.get_opcodes()

    # Count matches, inserts, deletes, replaces
    n_equal = sum(j2-j1 for tag, i1, i2, j1, j2 in opcodes if tag == 'equal')
    n_insert = sum(j2-j1 for tag, i1, i2, j1, j2 in opcodes if tag == 'insert')
    n_delete = sum(i2-i1 for tag, i1, i2, j1, j2 in opcodes if tag == 'delete')
    n_replace_ours = sum(i2-i1 for tag, i1, i2, j1, j2 in opcodes if tag == 'replace')
    n_replace_tgt = sum(j2-j1 for tag, i1, i2, j1, j2 in opcodes if tag == 'replace')

    print(f"\n  LCS alignment:", file=sys.stderr)
    print(f"    Equal: {n_equal}", file=sys.stderr)
    print(f"    Insert (in target, missing from ours): {n_insert}", file=sys.stderr)
    print(f"    Delete (in ours, not in target): {n_delete}", file=sys.stderr)
    print(f"    Replace: {n_replace_ours} ours -> {n_replace_tgt} target", file=sys.stderr)

    # Print detailed alignment
    print(f"\n{'='*80}")
    print(f"LCS Alignment: {func_name}")
    print(f"{'='*80}")

    for tag, i1, i2, j1, j2 in opcodes:
        if tag == 'equal':
            for k in range(i2 - i1):
                oi = i1 + k
                ti = j1 + k
                our_idx, our_raw = ours[oi]
                our_n = normalize_instruction(our_raw)
                tgt_n = normalize_instruction(target[ti])
                # Check if there are register diffs even though comparison matched
                if our_n != tgt_n:
                    print(f"  REGDIFF idx {our_idx:3d}  ours: {our_raw}")
                    print(f"                    tgt:  {target[ti]}")

        elif tag == 'insert':
            for k in range(j2 - j1):
                ti = j1 + k
                # Figure out where in our stream this should be inserted
                if i1 < len(ours):
                    insert_at = ours[i1][0]
                else:
                    insert_at = ours[-1][0] + 1
                print(f"  INSERT  before idx {insert_at:3d}  target[{ti}]: {target[ti]}")

        elif tag == 'delete':
            for k in range(i2 - i1):
                oi = i1 + k
                our_idx, our_raw = ours[oi]
                print(f"  DELETE  idx {our_idx:3d}  ours: {our_raw}")

        elif tag == 'replace':
            # Show both sides
            max_len = max(i2 - i1, j2 - j1)
            for k in range(max_len):
                if k < i2 - i1 and k < j2 - j1:
                    oi = i1 + k
                    ti = j1 + k
                    our_idx, our_raw = ours[oi]
                    print(f"  REPLACE idx {our_idx:3d}  ours: {our_raw}")
                    print(f"                     tgt:  {target[ti]}")
                elif k < i2 - i1:
                    oi = i1 + k
                    our_idx, our_raw = ours[oi]
                    print(f"  DELETE  idx {our_idx:3d}  ours: {our_raw}")
                else:
                    ti = j1 + k
                    if i1 < len(ours):
                        insert_at = ours[min(i1, len(ours)-1)][0]
                    else:
                        insert_at = ours[-1][0] + 1
                    print(f"  INSERT  before idx {insert_at:3d}  target[{ti}]: {target[ti]}")

    # Generate regfix rules
    print(f"\n{'='*80}")
    print(f"Suggested regfix rules")
    print(f"{'='*80}")

    insert_count = 0
    for tag, i1, i2, j1, j2 in opcodes:
        if tag == 'equal':
            for k in range(i2 - i1):
                oi = i1 + k
                ti = j1 + k
                our_idx, our_raw = ours[oi]
                our_n = normalize_instruction(our_raw)
                tgt_n = normalize_instruction(target[ti])
                if our_n != tgt_n:
                    adj_idx = our_idx + insert_count
                    # Generate subst rule
                    pat = escape_pattern_dollar(our_raw)
                    tgt_raw = target[ti]
                    # Convert target to our-style format (tab-separated)
                    tgt_parts = tgt_n.split(None, 1)
                    if len(tgt_parts) == 2:
                        tgt_fmt = f"{tgt_parts[0]}\t{tgt_parts[1]}"
                    else:
                        tgt_fmt = tgt_n
                    print(f'# idx {adj_idx}: {our_raw} -> {target[ti]}')
                    print(f'{func_name}: subst "{pat}" "{tgt_fmt}" @ {adj_idx}')

        elif tag == 'insert':
            for k in range(j2 - j1):
                ti = j1 + k
                if i1 < len(ours):
                    insert_at = ours[i1][0] + insert_count
                else:
                    insert_at = ours[-1][0] + 1 + insert_count
                tgt_insn = target[ti]
                tgt_n = normalize_instruction(tgt_insn)
                tgt_parts = tgt_n.split(None, 1)
                if len(tgt_parts) == 2:
                    tgt_fmt = f"{tgt_parts[0]}\t{tgt_parts[1]}"
                else:
                    tgt_fmt = tgt_n
                print(f'# INSERT target[{ti}]: {tgt_insn}')
                print(f'{func_name}: insert "{tgt_fmt}" @ {insert_at}')
                insert_count += 1

        elif tag == 'delete':
            for k in range(i2 - i1):
                oi = i1 + k
                our_idx, our_raw = ours[oi]
                adj_idx = our_idx + insert_count
                print(f'# DELETE ours idx {adj_idx}: {our_raw}')
                print(f'{func_name}: delete @ {adj_idx}')
                insert_count -= 1

        elif tag == 'replace':
            for k in range(max(i2-i1, j2-j1)):
                if k < i2 - i1 and k < j2 - j1:
                    oi = i1 + k
                    ti = j1 + k
                    our_idx, our_raw = ours[oi]
                    adj_idx = our_idx + insert_count
                    tgt_n = normalize_instruction(target[ti])
                    tgt_parts = tgt_n.split(None, 1)
                    if len(tgt_parts) == 2:
                        tgt_fmt = f"{tgt_parts[0]}\t{tgt_parts[1]}"
                    else:
                        tgt_fmt = tgt_n
                    pat = escape_pattern_dollar(our_raw)
                    print(f'# REPLACE idx {adj_idx}: {our_raw} -> {target[ti]}')
                    print(f'{func_name}: subst "{pat}" "{tgt_fmt}" @ {adj_idx}')
                elif k < i2 - i1:
                    oi = i1 + k
                    our_idx, our_raw = ours[oi]
                    adj_idx = our_idx + insert_count
                    print(f'# DELETE ours idx {adj_idx}: {our_raw}')
                    print(f'{func_name}: delete @ {adj_idx}')
                    insert_count -= 1
                else:
                    ti = j1 + k
                    if i1 < len(ours):
                        insert_at = ours[min(i2-1, len(ours)-1)][0] + insert_count + 1
                    else:
                        insert_at = ours[-1][0] + 1 + insert_count
                    tgt_n = normalize_instruction(target[ti])
                    tgt_parts = tgt_n.split(None, 1)
                    if len(tgt_parts) == 2:
                        tgt_fmt = f"{tgt_parts[0]}\t{tgt_parts[1]}"
                    else:
                        tgt_fmt = tgt_n
                    print(f'# INSERT target[{ti}]: {target[ti]}')
                    print(f'{func_name}: insert "{tgt_fmt}" @ {insert_at}')
                    insert_count += 1


if __name__ == '__main__':
    main()
