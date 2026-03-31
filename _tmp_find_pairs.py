#!/usr/bin/env python3
"""Find Kengo function names that match exactly 2 BB2 stubs (sibling pair candidates)."""
import subprocess, re, sys

tol = "--exact" if "--exact" in sys.argv else "--tolerance 0.05"

result = subprocess.run(
    ["python3", "tools/kengo_match.py"] + tol.split() + ["--use-affinity", "--top", "3", "--no-header"],
    capture_output=True, text=True,
    cwd="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
)

blocks = {}
cur = None
for line in result.stdout.splitlines():
    m = re.match(r"(\w+)  \[(\S+)\]  (\d+) insns", line)
    if m:
        cur = (m.group(1), m.group(2), int(m.group(3)))
        continue
    m = re.match(r"  → (\S+)\s+(\d+) insns \(\s*([+-]?\d+)\)  \[(\w+)\]", line)
    if m and cur:
        key = m.group(1)
        kengo_insns = int(m.group(2))
        diff = int(m.group(3))
        mod = m.group(4)
        blocks.setdefault(key, []).append((cur[0], cur[1], cur[2], kengo_insns, diff, mod))

print(f"=== Exact 2-pair matches (affinity) ===")
for name, stubs in sorted(blocks.items()):
    if len(stubs) == 2:
        a, b = stubs
        diff_a = f"{a[4]:+d}"
        diff_b = f"{b[4]:+d}"
        print(f"  {a[0]:<32} ({diff_a}) +  {b[0]:<32} ({diff_b})  →  {name}  [{a[3]}i]  [{a[5]}]")
