#!/usr/bin/env python3
"""bb2_diff_pin_search.py -- diff-targeted pin search for plateaued functions.

When the permuter plateaus with a small diff (1-2 instructions where the
ONLY change is a register name), the random `perm_bb2_add_pin` rarely
finds the specific var+reg combo needed. This tool does it
deterministically:

  1. Reads permuter/<func>/output-N-K/source.c (the lowest-score candidate)
  2. Runs bb2_diag_diff.py to identify the offending register substitution
  3. Greedily tries pinning EACH local variable to the target's register,
     one at a time
  4. For each combo, recompiles and runs the permuter's scorer
  5. Reports which combo (if any) brings score to 0

Limitations:
  - Only handles diffs where target uses register X and built uses register Y
    (same opcode, same dest, only source operand differs)
  - Only tries SINGLE-pin substitutions (not 2-pin or 3-pin combinations)
  - Greedy: stops at first score=0 hit, doesn't search globally

Usage:
  python3 tools/bb2_diff_pin_search.py <func>

Status: SKELETON. Some pieces work in isolation but the full pipeline
needs more glue. Core helpers implemented; main loop is a stub. See
docs/PERMUTER_PIPELINE.md for context.

To finish:
  - Parse bb2_diag_diff output into structured form (offset, target_reg,
    built_reg, opcode)
  - Map asm offset back to C source line (probably via the per-iter diff.txt
    or by walking objdump output with line numbers)
  - Collect local variables that could plausibly map to the affected
    asm location (heuristic: any local Decl in the source)
  - For each candidate var, generate a modified source.c with that var
    pinned to the target register, then run compile.sh + objdump + diff
  - Report results
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


def find_best_output(permdir: Path) -> Path | None:
    """Return the lowest-score output-N-K dir (excluding output-0-*)."""
    candidates = []
    for d in permdir.glob("output-*"):
        if not d.is_dir():
            continue
        m = re.match(r"output-(\d+)-\d+", d.name)
        if m and int(m.group(1)) > 0:
            candidates.append((int(m.group(1)), d))
    if not candidates:
        return None
    candidates.sort()
    return candidates[0][1]


def parse_diag_output(diag_text: str) -> list[dict]:
    """Parse bb2_diag_diff output into structured diffs.

    Returns list of dicts: {offset, target_text, built_text, target_reg, built_reg}
    when the diff is a single-register substitution (same opcode).
    """
    diffs = []
    blocks = re.split(r"\n(?=  off=)", diag_text)
    for block in blocks:
        if "off=" not in block:
            continue
        m_off = re.search(r"off=0x\s*([0-9a-f]+)", block)
        m_target = re.search(r"TARGET:\s*(.+?)\s*\[", block)
        m_built = re.search(r"BUILT:\s*(.+?)\s*\[", block)
        if not (m_off and m_target and m_built):
            continue
        t_text = m_target.group(1).strip()
        b_text = m_built.group(1).strip()
        # Try to identify single-reg substitution
        t_toks = t_text.split()
        b_toks = b_text.split()
        if len(t_toks) >= 2 and len(b_toks) >= 2 and t_toks[0] == b_toks[0]:
            # Same opcode -- compare operand fragments
            # Naive parse: split on commas after the mnemonic
            t_ops = t_text[len(t_toks[0]):].strip().split(",")
            b_ops = b_text[len(b_toks[0]):].strip().split(",")
            if len(t_ops) == len(b_ops):
                reg_diffs = []
                for i, (to, bo) in enumerate(zip(t_ops, b_ops)):
                    if to.strip() != bo.strip():
                        reg_diffs.append((i, to.strip(), bo.strip()))
                diffs.append({
                    "offset": m_off.group(1),
                    "opcode": t_toks[0],
                    "target_text": t_text,
                    "built_text": b_text,
                    "operand_diffs": reg_diffs,
                })
    return diffs


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("func")
    args = p.parse_args()

    permdir = Path(f"permuter/{args.func}")
    if not permdir.exists():
        sys.exit(f"No permuter dir for {args.func}")

    best = find_best_output(permdir)
    if best is None:
        sys.exit(f"No plateau output dir found in {permdir} (function may "
                 f"have matched at 0, or no permutation ever scored). "
                 f"Run bb2_retire.py or the permuter first to populate.")
    print(f"[diff-pin] using {best.name}/source.c as plateau baseline")

    # Run bb2_diag_diff and parse
    diag_cmd = ["python3", "tools/bb2_diag_diff.py", args.func]
    r = subprocess.run(diag_cmd, capture_output=True, text=True, timeout=60)
    diffs = parse_diag_output(r.stdout)
    if not diffs:
        sys.exit("[diff-pin] no structured diffs found; auto-diag output may "
                 "not contain single-reg substitutions. Check manually.")
    print(f"[diff-pin] found {len(diffs)} diffs:")
    for d in diffs:
        print(f"  off=0x{d['offset']:>4s}  {d['opcode']:8s}  "
              f"target={d['target_text']:30s}  built={d['built_text']}")

    # TODO: identify which local var maps to the asm offset that differs,
    # and try pinning it to the target register.
    print()
    print("[diff-pin] var->offset mapping not yet implemented")
    print("[diff-pin] manual next step: identify which C local feeds the "
          "differing instruction at the offset, and add `register T x "
          "asm(\"$target_reg\")` to that decl.")
    print()
    print("This tool is a SKELETON. To finish:")
    print("  - Parse output's source.c line<->offset mapping (probably via "
          "objdump --line-numbers or by inspecting cc1's debug output)")
    print("  - Generate src variants with each candidate pin")
    print("  - Run compile.sh + objdump + diff for each variant")
    print("  - Report which pin (if any) brings score to 0")


if __name__ == "__main__":
    main()
