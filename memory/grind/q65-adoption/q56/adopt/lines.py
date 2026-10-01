#!/usr/bin/env python3
"""lines.py <tree> file:a-b ...: print line ranges with numbers."""
import sys
T = sys.argv[1]
for spec in sys.argv[2:]:
    f, r = spec.rsplit(":", 1)
    a, b = map(int, r.split("-"))
    L = open(f"{T}/{f}", errors="replace").read().split("\n")
    print(f"---- {f}:{a}-{b}")
    for i in range(a, min(b, len(L)) + 1):
        print(f"{i:5d} {L[i-1]}")
