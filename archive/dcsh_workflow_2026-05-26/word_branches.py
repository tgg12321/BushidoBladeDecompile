#!/usr/bin/env python3
"""word_branches.py — .word-encode the residual branch-offset diffs.

When a function's verify-c diff is dominated by 1–3 branch instructions whose
opcode + registers match the target but whose immediate (offset) differs,
neither structural fixes nor register renames will close the gap. The
remaining offset diff is from label drift — usually caused by extra/different
.L<N> label numbering between mine and target — and the natural-C path can't
direct GCC to lay out labels at the exact byte positions the assembler used.

The escape hatch: replace each affected branch with a `.word\\t0xXXXXXXXX`
that hard-codes the exact 32-bit instruction the linker would have produced.
The assembler emits those bytes verbatim, bypassing label resolution.

The 3 branch diffs in `exec_game` (commit 10ee50c) used this technique. See:
- memory/feedback_exec_game_lessons.md
- memory/feedback_quick_reference.md Part 9 (label drift end-game)

This tool:
    1. Runs `binary_diff branch-offsets <func>` to get the list of
       BRANCH-OFFSET rows + target encodings.
    2. For each row, locates the maspsx-line idx for the branch (the regfix
       stream position).
    3. Emits a `<func>: subst "<branch-pattern>" ".word\\t0xXXXXXXXX" @ <idx>`
       line.
    4. With --apply, appends to regfix.txt.

The pattern matches the maspsx-line literally so the subst stays unique even
if the function has multiple branches with the same opcode+register pattern.
The replacement uses .word\\t to ensure tab-separated formatting that
maspsx/regfix re-emits cleanly.

Usage:
    python3 tools/word_branches.py <func>           # dry run, print rules
    python3 tools/word_branches.py <func> --apply   # append to regfix.txt
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def get_maspsx_indices(func: str) -> list[str] | None:
    try:
        out = subprocess.check_output(
            ["bash", "tools/dc.sh", "dump-text", func, "--post-regfix"],
            cwd=str(ROOT),
            stderr=subprocess.STDOUT,
        ).decode(errors="ignore")
    except (subprocess.CalledProcessError, FileNotFoundError):
        return None
    rows: list[tuple[int, str]] = []
    for ln in out.splitlines():
        m = re.match(r"\s*(\d+)\s+(.*)$", ln)
        if m:
            rows.append((int(m.group(1)), m.group(2)))
    if not rows:
        return None
    rows.sort(key=lambda x: x[0])
    return [t for _, t in rows]


def _escape_for_regex(s: str) -> str:
    """Escape regex meta-chars. Whitespace becomes \\s+ (regfix convention)."""
    out = []
    for c in s:
        if c in r"\.[]^$|+*?(){}":
            out.append("\\" + c)
        elif c == "\t":
            out.append("\\s+")
        else:
            out.append(c)
    return re.sub(r"\s+", r"\\s+", "".join(out))


def find_maspsx_idx_for_branch(maspsx_lines: list[str], mine_text: str,
                               used: set[int]) -> int | None:
    """Locate the maspsx-line index for a branch row.

    mine_text comes from objdump and looks like `bltz $2,5ba8 <exec_game+0xec>`.
    The maspsx line will look like `bltz\t$2,.L410`. Match on mnemonic + first
    register operand.
    """
    m = re.match(r"^\s*([a-z][a-z0-9.]*)\b\s*([^,]+),", mine_text.strip())
    if not m:
        return None
    mne = m.group(1)
    first_op = m.group(2).strip()
    # Canonicalize the operand to numeric form for matching.
    first_op_norm = re.sub(
        r"\$(v0|v1|a0|a1|a2|a3|t0|t1|t2|t3|t4|t5|t6|t7|t8|t9|s0|s1|s2|s3|"
        r"s4|s5|s6|s7|k0|k1|gp|sp|fp|ra|at|zero)\b",
        lambda mm: "$" + {
            "v0": "2", "v1": "3", "a0": "4", "a1": "5", "a2": "6", "a3": "7",
            "t0": "8", "t1": "9", "t2": "10", "t3": "11", "t4": "12",
            "t5": "13", "t6": "14", "t7": "15", "t8": "24", "t9": "25",
            "s0": "16", "s1": "17", "s2": "18", "s3": "19", "s4": "20",
            "s5": "21", "s6": "22", "s7": "23", "k0": "26", "k1": "27",
            "gp": "28", "sp": "29", "fp": "30", "ra": "31",
            "at": "1", "zero": "0",
        }[mm.group(1)],
        first_op,
    )
    for idx, line in enumerate(maspsx_lines):
        if idx in used:
            continue
        ms = line.strip()
        # Maspsx form: `bltz\t$2,.L410` (one or more whitespace as separator)
        m2 = re.match(r"^\s*([a-z][a-z0-9.]*)\b\s*([^,]+),", ms)
        if not m2:
            continue
        if m2.group(1) != mne:
            continue
        if m2.group(2).strip() != first_op_norm:
            continue
        return idx
    return None


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true",
                    help="Append rules to regfix.txt instead of printing")
    args = ap.parse_args()

    # Pull BRANCH-OFFSET rows from binary_diff.
    import binary_diff
    rep = binary_diff.compute_diffs(args.func)
    branch_rows = [r for r in rep.rows if r.category == "BRANCH-OFFSET"]
    if not branch_rows:
        print(f"No BRANCH-OFFSET rows for {args.func}.")
        if rep.structural_count > 0 or rep.rename_count > 0:
            print(f"  ({rep.structural_count} STRUCTURAL + {rep.rename_count} "
                  f"REG-RENAME diff(s) remain — fix those first.)")
        return 0
    if rep.structural_count > 0 or rep.rename_count > 0:
        print(f"REFUSING: {rep.structural_count} STRUCTURAL + {rep.rename_count} "
              f"REG-RENAME diff(s) still present. Close those before .word-encoding "
              f"branches — branch offsets shift when surrounding code changes.")
        print(f"  Use: bash tools/dc.sh side-by-side {args.func}")
        return 1

    maspsx = get_maspsx_indices(args.func)
    if maspsx is None:
        print(f"ERROR: could not get maspsx stream for {args.func}", file=sys.stderr)
        return 2

    rules: list[str] = []
    used: set[int] = set()
    for r in branch_rows:
        idx = find_maspsx_idx_for_branch(maspsx, r.mine_text, used)
        if idx is None:
            print(f"# {args.func}: could not locate branch in maspsx stream: "
                  f"{r.mine_text!r}", file=sys.stderr)
            continue
        used.add(idx)
        pattern = _escape_for_regex(maspsx[idx].strip())
        word_hex = f"0x{r.target_word:08X}"
        rule = f'{args.func}: subst "{pattern}" ".word\\t{word_hex}" @ {idx}'
        rules.append(rule)

    if not rules:
        print("No rules emitted (all branches failed maspsx-idx mapping).")
        return 1

    if args.apply:
        rf = ROOT / "regfix.txt"
        with rf.open("a", encoding="utf-8") as f:
            f.write(f"\n# auto: word_branches for {args.func} "
                    f"({len(rules)} branch-offset diff(s))\n")
            for ln in rules:
                f.write(ln + "\n")
        print(f"Appended {len(rules)} .word-subst rule(s) to regfix.txt")
        print(f"Run: bash tools/dc.sh build-active {args.func}")
    else:
        print(f"# Dry run for {args.func} ({len(rules)} BRANCH-OFFSET diff(s)):")
        print("# Add --apply to append to regfix.txt.")
        for ln in rules:
            print(ln)

    return 0


if __name__ == "__main__":
    sys.exit(main())
