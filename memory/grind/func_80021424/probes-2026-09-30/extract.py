#!/usr/bin/env python3
"""extract.py <src stem> <func> <out.c> : write <func>'s definition from src/<stem>.c to out.c (LF)."""
import re
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
stem, func, out = sys.argv[1:4]
s = (root / f"src/{stem}.c").read_bytes().decode("utf-8")
m = re.search(rf"^[A-Za-z_][^\n;]*\b{func}\s*\([^;{{]*\)\s*\{{", s, re.M)
i = m.end()
depth = 1
while depth:
    c = s[i]
    if c == "{":
        depth += 1
    elif c == "}":
        depth -= 1
    i += 1
(root / out).write_bytes((s[m.start():i] + "\n").encode())
print(out, s[m.start():i].count("\n") + 1, "lines")
