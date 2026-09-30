#!/usr/bin/env python3
"""jtphase.py: every jump table (compiled JTMARK_* markers + transcribed jtbl_* symbols) with its
address, phase (addr mod 8), the rodata object it sits in, and its owning function. A run of equal
phase within one object is consistent with ONE original file starting at that phase (Sony's
object-relative .align 3); a phase change inside an object needs a file boundary there."""
import re, subprocess
R = "/home/user/BushidoBladeDecompile/"
nm = subprocess.run(["mipsel-linux-gnu-nm", "-n", R + "build/bb2.elf"], capture_output=True, text=True).stdout
mp = open(R + "build/bb2.map").read()
objs = []
for m in re.finditer(r"^ \.rodata\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) (\S+)", mp, re.M):
    a, n = int(m.group(1), 16), int(m.group(2), 16)
    if n:
        objs.append((a, a + n, m.group(3).split("/")[-1]))
def obj_of(x):
    for a, e, o in objs:
        if a <= x < e:
            return o, a
    return "?", 0
rows = []
for line in nm.splitlines():
    p = line.split()
    if len(p) != 3:
        continue
    addr, name = int(p[0], 16), p[2]
    if name.startswith("JTMARK_"):
        rows.append((addr, "C  " + name.split("__", 1)[1]))
    elif re.match(r"^jtbl_[0-9A-Fa-f]{8}$", name):
        rows.append((addr, "tr " + name))
rows.sort()
last = None
for addr, who in rows:
    o, base = obj_of(addr)
    if o != last:
        print("-- %s (rodata starts %08x, phase %d)" % (o, base, base % 8))
        last = o
    print("   %08x phase %d  rel %s  %s" % (addr, addr % 8, (addr - base) % 8, who))
