#!/usr/bin/env python3
"""census_all.py [root] : how much C still reaches g_practice_menu_table (0x80101EC8, 2 records x 0x44C) other than
through PracticeMenuRec members. Counts, over src/*.c (+ include/*.h for declarations):
  handles  per-word symbols D_80101EC9..D_8010275F named in C (uses, distinct, functions)
  base     D_80101EC8 named in C (byte-offset bases: `&D_80101EC8 + k`, `(u8 *)&D_80101EC8 + off`)
  externs  per-word handle declarations in include/*.h"""
import re
import sys
from collections import defaultdict
from pathlib import Path

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2]
SYM = re.compile(r"\bD_(801[0-9A-F]{5})\b")
LO, HI = 0x80101EC8, 0x80102760
uses = 0
distinct = set()
funcs = defaultdict(int)
base_uses = 0
base_funcs = set()
for c in sorted((root / "src").glob("*.c")):
    cur = None
    for line in c.read_bytes().decode("utf-8", "replace").splitlines():
        m = re.match(r"^(?!extern|static inline|typedef)[A-Za-z_][\w\s\*]*\b(\w+)\s*\([^;]*$", line)
        if m:
            cur = m.group(1)
        for s in SYM.finditer(line):
            a = int(s.group(1), 16)
            if a == LO:
                base_uses += 1
                base_funcs.add(f"{c.stem}:{cur}")
            elif LO < a < HI:
                uses += 1
                distinct.add(s.group(0))
                funcs[f"{c.stem}:{cur}"] += 1
ext = set()
for h in (root / "include").glob("*.h"):
    if h.name == "m2c_context.h":
        continue
    for s in re.finditer(r"^extern\s+[^;]*\bD_(801[0-9A-F]{5})\b", h.read_bytes().decode("utf-8", "replace"), re.M):
        a = int(s.group(1), 16)
        if LO < a < HI:
            ext.add("D_" + s.group(1))
print(f"handles: {uses} uses, {len(distinct)} distinct, in {len(funcs)} functions; "
      f"base D_80101EC8: {base_uses} uses in {len(base_funcs)} functions; per-word externs: {len(ext)}")
if "-v" in sys.argv:
    for f, n in sorted(funcs.items(), key=lambda x: -x[1]):
        print(f"  {f}: {n}")
