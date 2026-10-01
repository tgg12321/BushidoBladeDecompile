#!/usr/bin/env python3
"""staticcheck.py OBJ BASE: each .sbss symbol of build/src/OBJ.o at BASE + its section offset, against the
address its name encodes (D_XXXXXXXX) or the reference ELF address; prints the first mismatches."""
import re, subprocess, sys
A = "/tmp/q56/adopt tree"
o, base = sys.argv[1], int(sys.argv[2], 16)
ref = {}
for l in subprocess.run(f"mipsel-linux-gnu-nm /tmp/q56/pre04obj/../pre04.elf 2>/dev/null", shell=True, capture_output=True, text=True).stdout.splitlines():
    p = l.split()
    if len(p) == 3:
        ref[p[2]] = int(p[0], 16)
for l in open(f"{A}/undefined_syms_auto.txt"):
    m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", l)
    if m:
        ref.setdefault(m.group(1), int(m.group(2), 16))
for l in open(f"{A}/named_syms.txt"):
    m = re.match(r"\s*(\w+)\s*=\s*(0x[0-9A-Fa-f]+)", l)
    if m:
        ref.setdefault(m.group(1), int(m.group(2), 16))
out = subprocess.run(f"mipsel-linux-gnu-objdump -t '{A}/build/src/{o}.o'", shell=True, capture_output=True, text=True).stdout
rows = []
for l in out.splitlines():
    m = re.match(r"^([0-9a-f]+)\s+l\s+O?\s*\.sbss\s+([0-9a-f]+)\s+(\S+)$", l)
    if m:
        rows.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
bad = 0
for off, size, nm in sorted(rows):
    want = ref.get(nm)
    if want is None:
        mm = re.fullmatch(r"D_([0-9A-F]{8})", nm)
        want = int(mm.group(1), 16) if mm else None
    got = base + off
    tag = "" if want == got else f"  <-- want {want:#x}" if want else "  (no reference address)"
    if tag:
        bad += 1
    if tag or bad:
        print(f"{got:08x} size {size:3} {nm}{tag}")
    if bad > 6:
        break
