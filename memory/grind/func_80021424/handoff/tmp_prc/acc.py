#!/usr/bin/env python3
"""acc.py <func> [reg] : offset -> set of (opcode) for loads/stores through <reg> (default $s0) in asm/funcs/<func>.s."""
import re
import sys
from collections import defaultdict
from pathlib import Path

root = Path(__file__).resolve().parents[2]
func = sys.argv[1]
reg = sys.argv[2] if len(sys.argv) > 2 else "$s0"
acc = defaultdict(set)
for line in (root / f"asm/funcs/{func}.s").read_text().splitlines():
    m = re.search(r"\*/\s+(l[bhw]u?|s[bhw])\s+\$\w+,\s*(-?0x[0-9A-Fa-f]+|\d+)\((\$\w+)\)", line)
    if m and m.group(3) == reg:
        acc[int(m.group(2), 0)].add(m.group(1))
for off in sorted(acc):
    print(f"{off:#x}: {' '.join(sorted(acc[off]))}")
