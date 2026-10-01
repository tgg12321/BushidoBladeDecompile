#!/usr/bin/env python3
"""List every symbol/address in [0x800100A4, 0x80010430) referenced by asm/funcs/*.s (hi/lo or jtbl), with referrers."""
import re
from collections import defaultdict
from pathlib import Path

root = Path(__file__).resolve().parents[2]
lo, hi = 0x800100A4, 0x80010430
refs = defaultdict(set)
for f in sorted((root / "asm/funcs").glob("*.s")):
    for m in re.finditer(r"(?:D|jtbl)_(8001[0-9A-F]{4})", f.read_text(errors="replace")):
        a = int(m.group(1), 16)
        if lo <= a < hi:
            refs[m.group(0)].add(f.stem)
for k in sorted(refs, key=lambda s: int(s.split("_")[1], 16)):
    print(k, sorted(refs[k]))
