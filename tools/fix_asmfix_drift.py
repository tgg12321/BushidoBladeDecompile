#!/usr/bin/env python3
"""Auto-fix `.L<N>` label drift in asmfix.txt rename rules.

Background: GCC numbers `.L<N>` labels file-wide. Adding/removing a function
in a .c file (or even adding a small pure-C stub) shifts every other
function's label numbers by +/-N. asmfix-slice rename rules of the form

    func_X: rename ".L<N>" ".L<HEX_ADDR>"

become broken when `.L<N>` no longer exists at the position the slice expects.
Build then fails — typically with one of:
    asmfix: WARNING: delete_between start did not match in <func>: ...
    asmfix: WARNING: delete_between end did not match in <func>: ...
    asmfix: WARNING: insert_before did not match in <func>: ...
    {standard input}:NNN: Error: symbol `.L<HEX>' is already defined

This is the asmfix-rename analogue of fix_label_drift.py (which handles
regfix substs). Logic, mirroring fix_label_drift's driver pattern:

  1. Run `make` and capture both the asmfix warnings and the assembler errors.
  2. For each func_X with a "did not match" warning, the rename rules for
     that func have stale source labels.
  3. For each rename rule of form `rename ".LN" ".L<HEX>"`, compute the
     target offset (HEX - func_link_addr) and find mine's actual label at
     that binary position (same approach as find_label_at.py).
  4. Propose / apply replacement of the source label.

Usage:
    python3 tools/fix_asmfix_drift.py            # dry-run from current build state
    python3 tools/fix_asmfix_drift.py --apply    # actually write asmfix.txt

Limitations:
- Only fixes `rename` rules where the target form is `.L<8-hex-digits>`
  (i.e., absolute-address-derived labels used by asmfix-slice). Other
  rename targets (e.g., `jtbl_NNN`, named labels) are skipped.
- Will not fix renames whose source label is in a .rodata section (jump
  tables) — those are parsed via different logic and don't drift the
  same way.
- If the build passes already, no fixes are proposed (we trust the build).
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASMFIX = ROOT / "asmfix.txt"

sys.path.insert(0, str(ROOT / "tools"))
from find_label_at import (  # noqa: E402
    get_func_link_address,
    parse_dump_text,
    parse_regfix_inserts_deletes,
    text_idx_to_binary_idx,
)

RENAME_RE = re.compile(
    r'^(func_\w+|[A-Za-z_]\w*):\s+rename\s+"(\.L\w+)"\s+"(\.L([0-9A-Fa-f]{8}))"\s*$'
)
ASMFIX_WARN_FUNC_RE = re.compile(
    r'^asmfix:\s+WARNING:\s+(?:delete_between\s+(?:start|end)|insert_before)\s+did\s+not\s+match\s+in\s+(\w+):'
)
ASM_DUP_LABEL_RE = re.compile(
    r"Error:\s+symbol\s+`(\.L[0-9A-Fa-f]+)'\s+is\s+already\s+defined"
)


def run_build() -> tuple[set[str], set[str]]:
    """Run `make` and return (funcs_with_asmfix_warnings, dup_labels)."""
    try:
        r = subprocess.run(
            ["make"], capture_output=True, text=True, cwd=str(ROOT), timeout=240,
        )
    except (FileNotFoundError, subprocess.TimeoutExpired):
        return set(), set()
    funcs: set[str] = set()
    dups: set[str] = set()
    for line in (r.stdout + r.stderr).splitlines():
        m = ASMFIX_WARN_FUNC_RE.match(line.strip())
        if m:
            funcs.add(m.group(1))
            continue
        m2 = ASM_DUP_LABEL_RE.search(line)
        if m2:
            dups.add(m2.group(1))
    return funcs, dups


def find_mine_label_for_offset(func: str, target_offset: int) -> str | None:
    insts, labels = parse_dump_text(func)
    if not insts:
        return None
    inserts, insert_afters, deletes = parse_regfix_inserts_deletes(func)
    target_binary_idx = target_offset // 4

    label_at_text_idx: dict[int, list[str]] = {}
    for tidx, lbl in labels:
        label_at_text_idx.setdefault(tidx, []).append(lbl)

    for idx, _text, _is_pseudo in insts:
        b = text_idx_to_binary_idx(insts, idx, inserts, insert_afters, deletes)
        if b == target_binary_idx:
            lbls = label_at_text_idx.get(idx, [])
            numeric = [l for l in lbls if re.match(r"^\.L\d+$", l)]
            if numeric:
                return numeric[0]
            if lbls:
                return lbls[0]
    return None


def parse_renames_for_func(func: str) -> list[tuple[int, str, str, int]]:
    """Return [(line_no_1based, src_label, target_label, target_addr), ...]
    for rename rules of the given func with absolute-address targets."""
    out: list[tuple[int, str, str, int]] = []
    if not ASMFIX.exists():
        return out
    for i, raw in enumerate(ASMFIX.read_text(encoding="utf-8").splitlines(), 1):
        m = RENAME_RE.match(raw.strip())
        if not m:
            continue
        if m.group(1) != func:
            continue
        try:
            tgt_addr = int(m.group(4), 16)
        except ValueError:
            continue
        if tgt_addr < 0x80000000 or tgt_addr > 0x80FFFFFF:
            continue
        out.append((i, m.group(2), m.group(3), tgt_addr))
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--func", help="Bypass build-driver: check this one function unconditionally")
    args = ap.parse_args()

    if args.func:
        broken = {args.func}
        print(f"# checking just {args.func} (bypassing build-driver)")
    else:
        print("# running make to detect asmfix-related drift...")
        broken, dups = run_build()
        if dups:
            print(f"# also seen duplicate-label assembler errors: {sorted(dups)}")
        if not broken:
            print("# no asmfix-related warnings — build either passes or fails for unrelated reasons")
            return 0
        print(f"# functions with asmfix slice warnings: {sorted(broken)}")

    fixes: list[tuple[int, str]] = []  # (line_no_1based, new_full_line)
    for func in sorted(broken):
        link_addr = get_func_link_address(func)
        if link_addr is None:
            print(f"  SKIP {func}: cannot find linked address")
            continue
        rules = parse_renames_for_func(func)
        if not rules:
            print(f"  SKIP {func}: no absolute-address rename rules")
            continue
        for line_no, src_label, tgt_label, tgt_addr in rules:
            tgt_offset = tgt_addr - link_addr
            if tgt_offset < 0:
                continue
            mine_label = find_mine_label_for_offset(func, tgt_offset)
            if mine_label is None:
                print(f"  WARN {func} line {line_no}: no label at offset 0x{tgt_offset:X} for {tgt_label}; manual check needed")
                continue
            if mine_label == src_label:
                continue  # already correct
            new_line = f'{func}: rename "{mine_label}" "{tgt_label}"'
            print(f"  FIX  {func} line {line_no}: {src_label} -> {mine_label}  (target {tgt_label} at +0x{tgt_offset:X})")
            fixes.append((line_no, new_line))

    if not fixes:
        print("# no fixes proposed (rules with broken funcs all resolve correctly, or only have non-address renames)")
        return 0

    if not args.apply:
        print(f"\n# {len(fixes)} fix(es) proposed. Re-run with --apply to write asmfix.txt.")
        return 0

    lines = ASMFIX.read_text(encoding="utf-8").splitlines(keepends=True)
    fix_by_line = {ln: new for ln, new in fixes}
    out_lines: list[str] = []
    for i, ln in enumerate(lines, 1):
        new = fix_by_line.get(i)
        if new is None:
            out_lines.append(ln)
            continue
        suffix = "\n" if ln.endswith("\n") else ""
        out_lines.append(new + suffix)
    ASMFIX.write_text("".join(out_lines), encoding="utf-8")
    print(f"\n# wrote {len(fixes)} fix(es) to asmfix.txt")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
