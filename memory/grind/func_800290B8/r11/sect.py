#!/usr/bin/env python3
"""sect.py <dumpfile> [func] -> print only that function's section of a cc1 RTL dump."""
import sys
path = sys.argv[1]
func = sys.argv[2] if len(sys.argv) > 2 else "func_800290B8"
out, on = [], False
for line in open(path):
    if line.startswith(";; Function "):
        on = line.split()[2] == func
    if on:
        out.append(line)
sys.stdout.write("".join(out))
