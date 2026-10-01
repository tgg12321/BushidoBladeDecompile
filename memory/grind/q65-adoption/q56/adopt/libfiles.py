#!/usr/bin/env python3
"""libfiles.py <tree>: Q69 - which C files are Sony library code, by the project's provenance evidence.

Evidence: memory/closer/psyq-library-census.md (2026-07-09 verbatim-link scan): section (a)'s verbatim PsyQ 4.0
module placements, all inside the library span it states ("Contiguous span 0x80078948..0x8008D070 contains ALL
of it"), and its "Unmatched gaps inside the library span" section (newer-build LIBSND/LIBSPU between verbatim
modules). For every object in the built tree's map: its .text range, the share of it covered by verbatim
modules, whether it lies inside the span, and whether its shipped code reaches small data gp-relative (any
R_MIPS_GPREL16 in its object: the original was compiled -G8)."""
import os, re, subprocess, sys
T = sys.argv[1]
os.chdir(T)
cen = open("memory/closer/psyq-library-census.md").read()
span = re.search(r"Contiguous span (0x[0-9A-Fa-f]+)\.\.(0x[0-9A-Fa-f]+)", cen)
S0, S1 = int(span.group(1), 16), int(span.group(2), 16)
mods = [(int(a, 16), int(b, 16)) for a, b in
        re.findall(r"^\| (0x[0-9A-F]{8}) \| (0x[0-9A-F]{8}) \|", cen.split("## (b)")[0], re.M)]
m = open("build/bb2.map").read()
rows = []
for mm in re.finditer(r"^ \.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/src/(\w+)\.o", m, re.M):
    a, n, f = int(mm.group(1), 16), int(mm.group(2), 16), mm.group(3)
    if n == 0:
        continue
    e = a + n
    cov = sum(max(0, min(e, y) - max(a, x)) for x, y in mods)
    inside = S0 <= a and e <= S1
    gp = "R_MIPS_GPREL16" in subprocess.run(f"mipsel-linux-gnu-objdump -r build/src/{f}.o",
                                            shell=True, capture_output=True, text=True).stdout
    cls = "LIBRARY" if inside else ("MIXED" if a < S1 and e > S0 else "game")
    rows.append((a, e, f, cls, cov * 100 // n, gp))
print(f"library span {S0:#x}..{S1:#x}; {len(mods)} verbatim module placements")
for a, e, f, cls, pc, gp in sorted(rows):
    if cls != "game" or not gp:
        print(f"{f:28s} {a:#x}..{e:#x} {cls:8s} verbatim {pc:3d}%  gp-access {'YES' if gp else 'no'}")
bad = [f for a, e, f, cls, pc, gp in rows if cls == "LIBRARY" and gp]
print("LIBRARY files with gp accesses (contradiction):", bad or "none")
print("LIBRARY:", " ".join(f for a, e, f, cls, pc, gp in sorted(rows) if cls == "LIBRARY"))
print("game files with gp accesses:", " ".join(f for a, e, f, cls, pc, gp in sorted(rows) if cls != "LIBRARY" and gp))
