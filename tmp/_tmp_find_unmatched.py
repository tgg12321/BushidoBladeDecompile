#!/usr/bin/env python3
"""Find the unmatched stub(s) by running match with very loose tolerance."""
import subprocess, re

result = subprocess.run(
    ["python3", "tools/kengo_match.py", "--tolerance", "0.99", "--no-header", "--min-size", "1"],
    capture_output=True, text=True,
    cwd="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
)

# Find stubs with no candidates (empty lines after stub header)
cur = None
prev_was_stub = False
for line in result.stdout.splitlines():
    m = re.match(r"(\w+)  \[(\S+)\]  (\d+) insns", line)
    if m:
        if prev_was_stub and cur:
            print(f"NO MATCH: {cur[0]}  [{cur[1]}]  {cur[2]} insns")
        cur = (m.group(1), m.group(2), int(m.group(3)))
        prev_was_stub = True
        continue
    if re.match(r"  → ", line):
        prev_was_stub = False
    if line.strip() == "":
        if prev_was_stub and cur:
            print(f"NO MATCH: {cur[0]}  [{cur[1]}]  {cur[2]} insns")
            cur = None
            prev_was_stub = False
