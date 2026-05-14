#!/usr/bin/env python3
"""Diagnose a whole-binary mismatch: map every differing byte to its function.

When `dc.sh verify --clean` reports the whole binary doesn't match, the
question "which function regressed?" is mechanical but tedious by hand: cmp the
two binaries, convert each file offset to a virtual address, look up which
function's address range contains it. During the func_8007CE0C session that
took more than a dozen tool calls. This tool does it in one.

It also catches what `verify --all` MISSES. `regfix_verify.get_all_c_functions`
finds C functions via a narrow return-type regex, so a function it doesn't
recognize (INCLUDE_ASM stub, unusual return type, regfixed asm) is silently
never checked — which is exactly why `verify --all` reported "all 1237 match"
while the whole-binary SHA1 failed. This tool works off the linker map's
symbol table, so it sees every byte of the binary.

For each affected function it reports:
  - differing byte count + vaddr range
  - whether it is the currently-active function (.bb2_active_func)
  - whether it is bridged in asmfix.txt
  - how many regfix rules it has, and how many of those are drift-fragile
    literal-`.L<N>` rules (the most common silent-regression cause)

Usage:
    python3 tools/diagnose_mismatch.py

Exit code: 0 if the binaries are identical, 1 if they differ (or on error).
"""
from __future__ import annotations

import bisect
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

PS_EXE_HEADER = 0x800
LOAD_ADDR = 0x80010000


def load_symbols(root: Path) -> list[tuple[int, str]]:
    """Sorted [(addr, name)] from build/bb2.map.

    Includes data symbols; for a code vaddr the containing entry is the
    function. On same-addr ties a non-`D_` name is preferred (the `D_<addr>`
    aliases shadow real function names in the map).
    """
    map_file = root / "build" / "bb2.map"
    if not map_file.exists():
        return []
    text = map_file.read_text(errors="ignore")
    raw = re.findall(
        r"^\s+0x([0-9a-f]+)\s+(\S+?)(?:\s*=\s*0x[0-9a-f]+)?\s*$",
        text, re.MULTILINE,
    )
    by_addr: dict[int, str] = {}
    for a, n in raw:
        addr = int(a, 16)
        cur = by_addr.get(addr)
        if cur is None or (cur.startswith("D_") and not n.startswith("D_")):
            by_addr[addr] = n
    return sorted(by_addr.items())


def func_for_vaddr(addrs: list[int], syms: list[tuple[int, str]], vaddr: int):
    """Return (name, start_addr) of the symbol whose range contains vaddr."""
    i = bisect.bisect_right(addrs, vaddr) - 1
    if i < 0:
        return None, None
    return syms[i][1], syms[i][0]


def main() -> int:
    root = ROOT
    build_exe = root / "build" / "bb2.exe"
    orig_exe = root / "disc" / "SLUS_006.63"
    if not build_exe.exists() or not orig_exe.exists():
        print("ERROR: build/bb2.exe or disc/SLUS_006.63 missing — run make first.",
              file=sys.stderr)
        return 1

    a = build_exe.read_bytes()
    b = orig_exe.read_bytes()
    if a == b:
        print("diagnose-mismatch: binaries are byte-identical (SHA1 match). Nothing to diagnose.")
        return 0

    if len(a) != len(b):
        print(f"WARNING: size differs — build={len(a)} bytes, original={len(b)} bytes.")

    n = min(len(a), len(b))
    diff_offsets = [off for off in range(n) if a[off] != b[off]]

    syms = load_symbols(root)
    if not syms:
        print("ERROR: could not load symbols from build/bb2.map.", file=sys.stderr)
        lo = diff_offsets[0]
        print(f"  {len(diff_offsets)} differing byte(s); first at file offset {lo} "
              f"(vaddr ~0x{LOAD_ADDR + lo - PS_EXE_HEADER:08X}).", file=sys.stderr)
        return 1
    addrs = [s[0] for s in syms]

    per_func: dict[str, dict] = defaultdict(
        lambda: {"bytes": 0, "min": None, "max": None, "start": None})
    nomap = 0
    for off in diff_offsets:
        vaddr = LOAD_ADDR + off - PS_EXE_HEADER
        name, start = func_for_vaddr(addrs, syms, vaddr)
        if name is None:
            nomap += 1
            continue
        rec = per_func[name]
        rec["bytes"] += 1
        rec["start"] = start
        rec["min"] = vaddr if rec["min"] is None else min(rec["min"], vaddr)
        rec["max"] = vaddr if rec["max"] is None else max(rec["max"], vaddr)

    # Cross-reference context.
    active = ""
    af = root / ".bb2_active_func"
    if af.exists():
        active = af.read_text(errors="ignore").strip()
    asmfix = ""
    if (root / "asmfix.txt").exists():
        asmfix = (root / "asmfix.txt").read_text(errors="ignore")
    regfix = ""
    if (root / "regfix.txt").exists():
        regfix = (root / "regfix.txt").read_text(errors="ignore")

    print(f"diagnose-mismatch: {len(diff_offsets)} differing byte(s) "
          f"across {len(per_func)} function(s)")
    print()

    has_sibling = False
    for name, rec in sorted(per_func.items(), key=lambda kv: kv[1]["min"] or 0):
        tags = []
        if name == active:
            tags.append("** ACTIVE FUNCTION **")
        else:
            tags.append("sibling/unrelated")
            has_sibling = True
        if re.search(rf"^{re.escape(name)}: replace_with_asmfile", asmfix, re.MULTILINE):
            tags.append("bridged in asmfix.txt")
        func_rules = re.findall(rf"^{re.escape(name)}:.*$", regfix, re.MULTILINE)
        literal_rules = [r for r in func_rules if re.search(r"\.L\d+", r)]
        if func_rules:
            tags.append(f"{len(func_rules)} regfix rule(s)")
        if literal_rules:
            tags.append(f"{len(literal_rules)} LITERAL-.L rule(s) <== prime suspect")

        nbytes = rec["bytes"]
        insn = f"{nbytes // 4} insn" if nbytes % 4 == 0 else f"~{nbytes / 4:.1f} insn"
        rng = f"0x{rec['min']:08X}"
        if rec["max"] != rec["min"]:
            rng += f"-0x{rec['max']:08X}"
        print(f"  {name}  ({nbytes} byte(s), {insn}, {rng})")
        print(f"      {', '.join(tags)}")
    if nomap:
        print(f"  ({nomap} differing byte(s) not in any mapped symbol "
              f"— likely .data / .rodata)")
    print()

    if has_sibling:
        print("  Differing bytes outside your active function => SIBLING REGRESSION.")
        print("  Most common cause: .L<N> label drift breaking literal-label regfix")
        print("  rules (a sibling's C body changed GCC's file-wide label numbering).")
        print("  Next steps:")
        print("      bash tools/make_check.py --tail 3        # rebuild; surfaces no-op'd rules")
        print("      bash tools/dc.sh regfix-drift-immune     # audit drift-fragile rules")
        print("      bash tools/dc.sh regfix-drift-immune --apply   # rewrite the safe ones")
    else:
        print("  All differing bytes are in your active function — ordinary in-progress")
        print("  mismatch. Use `dc.sh diff-align` / `dc.sh verify-c` to close it.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
