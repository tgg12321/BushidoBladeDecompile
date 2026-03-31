#!/usr/bin/env python3
"""
kengo_ref.py — Dump Kengo (PS2) disassembly for a BB2 function's Kengo equivalent.

Shows the Kengo assembly alongside the BB2 assembly for functions that have
been matched to a Kengo equivalent. Useful as a decompilation reference since
the Marionation engine structure is preserved between BB2 (PS1) and Kengo (PS2).

Usage (run from project root in WSL):
  python3 tools/kengo_ref.py <name_or_addr>  [--bb2] [--list]

Arguments:
  <name_or_addr>   Kengo function name, BB2 func address (func_80XXXXXX or 80XXXXXX)
  --bb2            Show BB2 assembly alongside Kengo (default: Kengo only)
  --list           List all Kengo functions matching the given name fragment

Examples:
  python3 tools/kengo_ref.py camera_SetMatrix
  python3 tools/kengo_ref.py func_8001DA8C
  python3 tools/kengo_ref.py func_8001DA8C --bb2
  python3 tools/kengo_ref.py special_camera_Exec --bb2
  python3 tools/kengo_ref.py damage_Calc --list
"""

import sys
import re
import subprocess
import csv
import os
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).parent.parent
KENGO_ELF = ROOT / "Kengo/disc/SLUS_200.21"
KENGO_FUNCS = ROOT / "Kengo/kengo_functions_full.txt"
KENGO_MATCHES = ROOT / "kengo_matches.csv"
ASM_FUNCS = ROOT / "asm/funcs"

OBJDUMP = "mipsel-linux-gnu-objdump"


def parse_kengo_functions():
    """Parse kengo_functions_full.txt -> {name: [(addr, size, source_file), ...]}"""
    funcs = defaultdict(list)
    current_file = ""
    func_pat = re.compile(r"/\*\s+([0-9a-f]+)\s+([0-9a-f]+)\s+\*/\s+(?:static\s+)?(\w+)\(\)")
    file_pat = re.compile(r"FILE -- (.+)")
    with open(KENGO_FUNCS) as f:
        for line in f:
            fm = file_pat.search(line)
            if fm:
                current_file = fm.group(1).strip()
            m = func_pat.search(line)
            if m:
                addr = int(m.group(1), 16)
                size = int(m.group(2), 16)
                name = m.group(3)
                funcs[name].append((addr, size, current_file))
    return funcs


def load_kengo_matches():
    """Load kengo_matches.csv -> {bb2_func: kengo_name, kengo_insns, ...}"""
    mapping = {}
    with open(KENGO_MATCHES) as f:
        for row in csv.DictReader(f):
            if row["kengo_name"]:
                mapping[row["bb2_func"]] = row
    return mapping


def disasm_range(addr, size):
    """Run objdump on a specific address range in the Kengo ELF."""
    stop = addr + size
    cmd = [
        OBJDUMP, "-d",
        f"--start-address=0x{addr:x}",
        f"--stop-address=0x{stop:x}",
        str(KENGO_ELF)
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    # Strip the ELF header lines, keep only actual instructions
    lines = []
    in_body = False
    for line in result.stdout.splitlines():
        if re.match(r"^[0-9a-f]+ <\w", line):
            in_body = True
        if in_body:
            lines.append(line)
    return lines


def read_bb2_asm(path):
    """Read BB2 asm file, returning only instruction lines stripped of the /* offset addr bytes */ prefix."""
    if not Path(path).exists():
        return None
    # BB2 instruction format: "    /* OFFSET ADDR BYTES */  mnemonic  operands"
    insn_pat = re.compile(r"/\*\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+\*/\s+(.*)")
    lines = []
    with open(path) as f:
        for line in f:
            m = insn_pat.search(line)
            if m:
                lines.append(m.group(1).strip())
    return lines


def find_bb2_asm_file(bb2_addr, kengo_name):
    """Find the BB2 asm file — it may be named by Kengo name or by address."""
    # Try Kengo name first (if renamed via symbol_addrs.txt + make setup)
    named = ASM_FUNCS / f"{kengo_name}.s"
    if named.exists():
        return named
    # Try BB2 address
    addr_named = ASM_FUNCS / f"{bb2_addr}.s"
    if addr_named.exists():
        return addr_named
    return None


def print_side_by_side(kengo_lines, bb2_insns, kengo_name, bb2_addr, bb2_file_label):
    """Print Kengo and BB2 asm side by side."""
    col_w = 55
    sep = " | "

    header_l = f"KENGO: {kengo_name}"
    header_r = f"BB2: {bb2_addr}"
    print(f"{header_l:<{col_w}}{sep}{header_r}")
    print("-" * (col_w + len(sep) + 55))

    # Kengo: extract instruction lines (lines with hex offset prefix)
    k_insns = [l.strip() for l in kengo_lines if re.match(r"\s+[0-9a-f]+:", l)]
    b_insns = bb2_insns or []

    max_rows = max(len(k_insns), len(b_insns))
    for i in range(max_rows):
        left = k_insns[i] if i < len(k_insns) else ""
        right = b_insns[i] if i < len(b_insns) else ""
        print(f"{left:<{col_w}}{sep}{right}")

    print()
    print(f"Kengo: {len(k_insns)} insns | BB2: {len(b_insns)} insns")


def print_kengo_only(kengo_lines, kengo_name, addr, size, source_file):
    """Print just the Kengo disassembly."""
    print(f"Kengo: {kengo_name}")
    print(f"  @ 0x{addr:08x}  size=0x{size:x}  ({size // 4} insns)")
    print(f"  source: {source_file}")
    print("-" * 70)
    for line in kengo_lines:
        print(line)


def main():
    args = sys.argv[1:]
    if not args or args[0] in ("-h", "--help"):
        print(__doc__)
        sys.exit(0)

    show_bb2 = "--bb2" in args
    list_mode = "--list" in args
    query_args = [a for a in args if not a.startswith("--")]
    if not query_args:
        print("ERROR: provide a function name or address")
        sys.exit(1)
    query = query_args[0]

    if not KENGO_ELF.exists():
        print(f"ERROR: Kengo ELF not found at {KENGO_ELF}")
        sys.exit(1)

    kengo_funcs = parse_kengo_functions()
    kengo_matches = load_kengo_matches()

    # --- List mode: search by fragment ---
    if list_mode:
        fragment = query.lower()
        hits = [(name, entries) for name, entries in kengo_funcs.items()
                if fragment in name.lower()]
        if not hits:
            print(f"No Kengo functions matching '{query}'")
        else:
            print(f"Kengo functions matching '{query}' ({len(hits)} results):")
            for name, entries in sorted(hits):
                for addr, size, src in entries:
                    bb2_funcs = [k for k, v in kengo_matches.items() if v["kengo_name"] == name]
                    bb2_str = f" <- BB2: {', '.join(bb2_funcs)}" if bb2_funcs else ""
                    print(f"  {name:40s}  0x{addr:08x}  {size//4:4d}i  [{src}]{bb2_str}")
        sys.exit(0)

    # --- Resolve query to a Kengo name + BB2 addr ---
    bb2_addr = None
    kengo_name = None

    # Check if it looks like a BB2 address
    if re.match(r"^(func_)?[89][0-9a-fA-F]{7}$", query):
        addr_str = query if query.startswith("func_") else f"func_{query.upper()}"
        bb2_addr = addr_str
        match_row = kengo_matches.get(addr_str)
        if not match_row:
            print(f"ERROR: {addr_str} has no Kengo match in kengo_matches.csv")
            print("  (function may be PS1-specific or in the ambiguous/unmatched pool)")
            sys.exit(1)
        kengo_name = match_row["kengo_name"]
        print(f"BB2 {addr_str} -> Kengo: {kengo_name}")
    else:
        # Direct Kengo name lookup
        kengo_name = query
        # Find the corresponding BB2 func(s) if any
        bb2_matches = [k for k, v in kengo_matches.items() if v["kengo_name"] == kengo_name]
        if bb2_matches:
            bb2_addr = bb2_matches[0]
            if len(bb2_matches) > 1:
                print(f"Note: {kengo_name} maps to {len(bb2_matches)} BB2 functions (ambiguous size match):")
                for m in bb2_matches:
                    print(f"  {m}")
                print(f"  Showing: {bb2_addr} (pass a specific BB2 address to choose)")

    # --- Look up Kengo function ---
    if kengo_name not in kengo_funcs:
        print(f"ERROR: '{kengo_name}' not found in kengo_functions_full.txt")
        # Suggest similar names
        lower = kengo_name.lower()
        suggestions = [n for n in kengo_funcs if lower in n.lower()][:5]
        if suggestions:
            print(f"  Did you mean: {', '.join(suggestions)}")
        sys.exit(1)

    entries = kengo_funcs[kengo_name]
    if len(entries) > 1:
        print(f"Note: '{kengo_name}' appears {len(entries)} times in Kengo:")
        for i, (addr, size, src) in enumerate(entries):
            print(f"  [{i}] 0x{addr:08x}  {size//4}i  [{src}]")
        print(f"  Using [0]. Pass the address directly to choose a specific one.")
    addr, size, source_file = entries[0]

    # --- Get Kengo disassembly ---
    kengo_lines = disasm_range(addr, size)
    if not kengo_lines:
        print(f"ERROR: objdump returned no output for {kengo_name} @ 0x{addr:x}")
        sys.exit(1)

    # --- Output ---
    if show_bb2 and bb2_addr:
        bb2_file_path = find_bb2_asm_file(bb2_addr, kengo_name)
        if bb2_file_path:
            bb2_insns = read_bb2_asm(str(bb2_file_path))
            print_side_by_side(kengo_lines, bb2_insns or [],
                                kengo_name, bb2_addr, bb2_addr)
        else:
            print(f"Note: BB2 asm stub not found for {bb2_addr} / {kengo_name}")
            print(f"  (may already be decompiled, or name not yet in symbol_addrs.txt)")
            print()
            print_kengo_only(kengo_lines, kengo_name, addr, size, source_file)
    else:
        print_kengo_only(kengo_lines, kengo_name, addr, size, source_file)


if __name__ == "__main__":
    main()
