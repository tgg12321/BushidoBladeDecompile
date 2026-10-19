#!/usr/bin/env python3
"""Scan inline-asm + INCLUDE_ASM functions for PsyQ stdlib / BIOS trampoline
fingerprints. Outputs known_psyq_stdlib.txt with one entry per match:

    <func>  <kind>  <evidence>

Kinds:

    bios_table_<addr>     -- addiu $tX,$zero,0xA0|0xB0|0xC0; jr $tX; ...
    syscall_kernel        -- `syscall N` (or .word 0xNN..0C/0D)
    psyq_<name>           -- memcpy / memmove / memset heuristic match
    handwritten_asm       -- has add/addi/sub/syscall mnemonics GCC never emits

Use cases:
  * Reduce the active 471 backlog by tagging "allowed-as-asm" exceptions so
    they aren't counted as work.
  * Surface concrete psyq_stdlib candidates so they can be replaced with the
    PsyQ-equivalent C idiom.

Usage:
    python3 tools/psyq_stdlib_scan.py                 # scan all 1410 funcs
    python3 tools/psyq_stdlib_scan.py --func F        # scan one
    python3 tools/psyq_stdlib_scan.py --kind bios     # filter by kind
    python3 tools/psyq_stdlib_scan.py --update-file   # write known_psyq_stdlib.txt
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ASM_FUNCS = ROOT / "asm" / "funcs"
OUT_FILE = ROOT / "known_psyq_stdlib.txt"

# Patterns reused from classify_func.py but inlined to keep this standalone.
INSN_RE = re.compile(
    r"/\*\s+[0-9A-Fa-f]+\s+([0-9A-Fa-f]+)\s+[0-9A-Fa-f]+\s+\*/\s+(\S+)\s*(.*)"
)


def load_insns(func: str) -> list[tuple[str, str]] | None:
    p = ASM_FUNCS / f"{func}.s"
    if not p.exists():
        return None
    out = []
    for line in p.read_text(encoding="utf-8").splitlines():
        m = INSN_RE.search(line)
        if m:
            out.append((m.group(2), m.group(3).strip()))
    return out


def is_bios_trampoline(insns) -> tuple[bool, str | None]:
    if not 2 <= len(insns) <= 6:
        return False, None
    has_jr_t = any(m == "jr" and "$t" in o for m, o in insns)
    for m, ops in insns:
        if m == "addiu":
            mm = re.search(r"\$t\d+,\s*\$zero,\s*(0x[ABC]0)", ops)
            if mm and has_jr_t:
                return True, f"bios_table_{mm.group(1)}"
    for m, ops in insns:
        if m in ("syscall", "break"):
            return True, "syscall_kernel"
        if m == ".word" and re.match(r"^0x[0-9a-f]{0,6}0[cd]$", ops.lower().replace(" ", "")):
            return True, "syscall_kernel"
    return False, None


def is_handwritten(insns) -> bool:
    """Detect ASPSX-only or handwritten mnemonics GCC 2.7.2 never emits."""
    for m, _ in insns:
        if m in ("add", "addi", "sub", "syscall"):  # GCC uses addu/addiu/subu
            return True
    return False


def looks_like_memcpy(insns) -> tuple[bool, str | None]:
    """Tiny memcpy/memmove fingerprint."""
    if not 6 <= len(insns) <= 30:
        return False, None
    mnems = [m for m, _ in insns]
    has_lbu = "lbu" in mnems
    has_sb = "sb" in mnems
    has_neg_step = any(
        m == "addiu" and re.search(r"-1\b", o) for m, o in insns
    )
    has_pos_step = any(
        m == "addiu" and re.search(r",\s*\$\w+,\s*1\b", o) for m, o in insns
    )
    has_branch = any(m in ("bne", "bnez", "bgtz", "bgez") for m in mnems)
    if has_lbu and has_sb and has_branch:
        if has_neg_step and has_pos_step:
            return True, "memmove"
        return True, "memcpy"
    return False, None


def looks_like_memset(insns) -> bool:
    if not 4 <= len(insns) <= 18:
        return False
    mnems = [m for m, _ in insns]
    has_store = any(m in ("sb", "sw", "sh") for m in mnems)
    has_loop = any(m in ("bne", "bnez", "bgtz") for m in mnems)
    has_no_load = not any(m in ("lw", "lh", "lhu", "lb", "lbu") for m in mnems)
    return has_store and has_loop and has_no_load


def classify_func(func: str) -> tuple[str | None, str | None]:
    """Return (kind, evidence) or (None, None) if no match."""
    insns = load_insns(func)
    if insns is None:
        return None, None
    bios_ok, bios_kind = is_bios_trampoline(insns)
    if bios_ok:
        return bios_kind, f"{len(insns)} insns, BIOS trampoline shape"
    cpy_ok, cpy_kind = looks_like_memcpy(insns)
    if cpy_ok:
        return f"psyq_{cpy_kind}", f"{len(insns)} insns, lbu/sb loop"
    if looks_like_memset(insns):
        return "psyq_memset", f"{len(insns)} insns, store-loop with no load"
    if is_handwritten(insns):
        return "handwritten_asm", "uses GCC-unfriendly mnemonics (add/addi/sub/syscall)"
    return None, None


def collect_func_universe() -> list[str]:
    return sorted(p.stem for p in ASM_FUNCS.glob("*.s"))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--func", default=None, help="Scan only this function")
    ap.add_argument("--kind", default=None,
                    help="Filter output by kind prefix (bios, syscall, psyq, handwritten)")
    ap.add_argument("--update-file", action="store_true",
                    help=f"Overwrite {OUT_FILE.name} with the scan result")
    args = ap.parse_args()

    funcs = [args.func] if args.func else collect_func_universe()
    out_lines = []
    counts: dict[str, int] = {}
    for f in funcs:
        kind, ev = classify_func(f)
        if kind is None:
            continue
        if args.kind and not kind.startswith(args.kind):
            continue
        counts[kind] = counts.get(kind, 0) + 1
        out_lines.append(f"{f:36s}  {kind:24s}  {ev}")

    for line in out_lines:
        print(line)

    print(f"\n# {len(out_lines)} matches across {len(funcs)} functions:", file=sys.stderr)
    for k in sorted(counts):
        print(f"#   {k:24s} {counts[k]:>4d}", file=sys.stderr)

    if args.update_file:
        header = (
            "# known_psyq_stdlib.txt -- auto-generated by tools/psyq_stdlib_scan.py\n"
            "# Each line: <func>  <kind>  <evidence>\n"
            "# Kind prefixes:\n"
            "#   bios_table_*    BIOS jump table call (allowed-as-asm exception)\n"
            "#   syscall_kernel  raw syscall/break (allowed-as-asm exception)\n"
            "#   psyq_<name>     PsyQ stdlib heuristic match (replace with C idiom)\n"
            "#   handwritten_asm uses GCC-unfriendly mnemonics; pin to asm or rewrite\n\n"
        )
        OUT_FILE.write_text(header + "\n".join(out_lines) + "\n",
                            encoding="utf-8", newline="\n")
        print(f"\nwrote {OUT_FILE.relative_to(ROOT)}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
