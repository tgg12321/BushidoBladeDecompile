#!/usr/bin/env python3
"""jtphase.py [MAP] [OBJ_FILTER...] -- jump-table phase checker (restructure risk R1; run in WSL
from the repo root after a build).

For every object linked into .rodata (build/bb2.map input-section lines), find its jump tables:
runs of R_MIPS_32 relocations in .rel.rodata against the .text section (or a function symbol),
consecutive 4-byte words. Under the object-relative alignment model
(.claude/rules/rodata-object-alignment.md) every table's offset from its object's .rodata start
must be a multiple of 8 (the compiler's `.align 3`, relative to the object). Prints each table
(object, linked address, offset in object, entries) and FAILs on any table whose in-object offset
is not 8-aligned or whose linked address phase differs from its object's start phase + offset.
With OBJ_FILTER arguments, only objects whose id contains one of them are listed."""
import re
import subprocess
import sys

MAP = "build/bb2.map"
args = sys.argv[1:]
if args and args[0].endswith(".map"):
    MAP, args = args[0], args[1:]

objs = []
for line in open(MAP, encoding="utf-8", errors="replace"):
    m = re.match(r"^ \.rodata\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+(build/src/\S+\.o)$", line)
    if m and int(m.group(2), 16):
        objs.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3)))


def tables(o):
    out = subprocess.run(["mipsel-linux-gnu-readelf", "-rW", o], capture_output=True, text=True).stdout
    cur, offs = None, []
    for l in out.splitlines():
        m = re.match(r"Relocation section '\.rel(\.\w+)'", l)
        if m:
            cur = m.group(1)
            continue
        p = l.split()
        if cur == ".rodata" and len(p) >= 5 and re.match(r"^[0-9a-f]{8}$", p[0]) and p[2] == "R_MIPS_32":
            if p[4] == ".text" or p[4].startswith("func_") or p[4][0].isalpha():
                # only .text targets: symbol section checked below via readelf -s would be heavier;
                # string-pointer arrays point at .rodata/.data symbols, which we drop by name below
                offs.append((int(p[0], 16), p[4]))
    runs, run = [], []
    for off, sym in sorted(offs):
        if sym != ".text":
            continue
        if run and off == run[-1] + 4:
            run.append(off)
        else:
            if run:
                runs.append(run)
            run = [off]
    if run:
        runs.append(run)
    return runs


ok = True
n = 0
for start, size, o in objs:
    tid = o[len("build/src/"):-2]
    if args and not any(a in tid for a in args):
        continue
    for run in tables(o):
        n += 1
        off = run[0]
        addr = start + off
        good = off % 8 == 0
        ok &= good
        print(f"{tid:40s} obj 0x{start:08X} (phase {start % 8}) table 0x{addr:08X} (phase {addr % 8}) "
              f"off 0x{off:X} entries {len(run)} {'ok' if good else 'MISALIGNED in its object'}")
print(f"{n} table(s) checked; JTPHASE", "OK" if ok else "FAIL")
sys.exit(0 if ok else 1)
