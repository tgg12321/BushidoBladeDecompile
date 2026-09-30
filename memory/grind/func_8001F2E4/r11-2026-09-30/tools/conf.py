#!/usr/bin/env python3
"""conf.py DUMPDIR name... -- for each named user variable (r11table naming),
print its pseudo, hard reg, ALLOCDBG line and the HARD registers in its .greg
conflict list (global.c records conflicts with already-allocated hard regs:
local-alloc quantities and fixed/argument registers)."""
import re, subprocess, sys
d = sys.argv[1]
names = sys.argv[2:]
tab = subprocess.run(["python3", "tmp/ff-worker/f1f2e4/r11/tools/r11table.py", d] + names,
                     capture_output=True, text=True).stdout
greg = open(f"{d}/tu.i.greg.fn").read()
REGN = {2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 7: "a3", 8: "t0", 9: "t1", 10: "t2", 11: "t3",
        12: "t4", 13: "t5", 14: "t6", 15: "t7", 16: "s0", 17: "s1", 18: "s2", 19: "s3", 24: "t8", 25: "t9",
        29: "sp", 31: "ra"}
for ln in tab.splitlines()[1:]:
    f = ln.split()
    if len(f) < 4:
        continue
    pr = f[2]
    m = re.search(rf"^;; {pr} conflicts: (.*)$", greg, re.M)
    hard = []
    if m:
        hard = [REGN.get(int(x), x) for x in m.group(1).split() if int(x) < 64]
    print(ln, "| hard-reg conflicts:", " ".join(hard) if m else "(no global conflict line)")
