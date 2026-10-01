#!/usr/bin/env python3
"""phases.py <tree> obj...: each object's linked .rodata range and the jump tables inside it with their phase
(address mod 8). Two tables at different phases cannot come from one original file (rodata-object-alignment
condition 1), so a merge across them would remove a proven boundary."""
import re, subprocess, sys
T = sys.argv[1]
m = open(f"{T}/build/bb2.map").read()
rng = {}
for mm in re.finditer(r"^ \.rodata\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\w+)\.o", m, re.M):
    a = int(mm.group(1), 16)
    rng[mm.group(3)] = (a, a + int(mm.group(2), 16))
nm = subprocess.run(f"mipsel-linux-gnu-nm '{T}/build/bb2.elf'", shell=True, capture_output=True, text=True).stdout
jt = sorted({int(l.split()[0], 16) for l in nm.splitlines() if len(l.split()) == 3 and "jtbl" in l.split()[2]})
# compiled tables have no jtbl_ symbol: find them from the objects' own .rodata relocation targets of jr tables
for o in sys.argv[2:]:
    if o not in rng:
        print(f"{o}: no .rodata"); continue
    a, e = rng[o]
    named = [x for x in jt if a <= x < e]
    # compiled switch tables: R_MIPS_32 relocations inside the object's .rodata point at .text labels
    rel = subprocess.run(f"mipsel-linux-gnu-objdump -r -j .rodata '{T}/build/src/{o}.o'", shell=True, capture_output=True, text=True).stdout
    offs = sorted(int(l.split()[0], 16) for l in rel.splitlines() if re.match(r"^[0-9a-f]+\s+R_MIPS_32\s+\.text", l))
    # group consecutive words into tables
    tables, cur = [], None
    for x in offs:
        if cur is not None and x == cur[-1] + 4:
            cur.append(x)
        else:
            cur = [x]; tables.append(cur)
    comp = [a + t[0] for t in tables]
    allt = sorted(set(named) | set(comp))
    print(f"{o}: rodata {a:#x}..{e:#x}; tables " + (", ".join(f"{x:#x} (phase {x % 8})" for x in allt) or "none"))
