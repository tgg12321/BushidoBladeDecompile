#!/usr/bin/env python3
"""r11table.py DUMPDIR [names...]
Name every user-variable pseudo of func_8001F2E4 and print its allocation.

Pseudo naming: GCC 2.7.2 expands C declarations as they are parsed, so the
user-variable pseudos (reg/v) are numbered in textual declaration order
(parameters first). The declarations are read from the preprocessed body
(macro block-locals included, in expansion order) and zipped with the sorted
reg/v pseudo numbers of the .rtl dump; the count must agree exactly.
Allocation: .greg 'Register dispositions' (hard reg) + BB2_ALLOC_DEBUG lines
(global.c allocation order / nrefs / livelen / priority; absent = the pseudo
was allocated by local-alloc, or never allocated)."""
import re, sys
d = sys.argv[1]
want = set(sys.argv[2:])
REGN = {2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 7: "a3", 8: "t0", 9: "t1", 10: "t2", 11: "t3",
        12: "t4", 13: "t5", 14: "t6", 15: "t7", 16: "s0", 17: "s1", 18: "s2", 19: "s3", 24: "t8", 25: "t9"}

src = open(f"{d}/tu.i", encoding="utf-8", errors="replace").read()
m = re.search(r"\bvoid\s+func_8001F2E4\s*\(([^)]*)\)\s*\{", src)
params = [re.findall(r"(\w+)\s*$", p.strip())[0] for p in m.group(1).split(",")]
i = m.end()
depth = 1
j = i
while depth:
    c = src[j]
    if c == "{":
        depth += 1
    elif c == "}":
        depth -= 1
    j += 1
body = src[i:j - 1]
body = re.sub(r"^#.*$", "", body, flags=re.M)  # cpp linemarkers
# declarations: a type keyword at statement start (after { ; } or start)
decls = []
for dm in re.finditer(r"(?:(?<=[{;}])|^)\s*(?:unsigned\s+|signed\s+)?(u8|u16|u32|s8|s16|s32|int|short|char)\b((?:\s*\*?\s*\w+\s*(?:=[^;,]*)?,)*\s*\*?\s*\w+\s*(?:=[^;]*)?);", body):
    for part in re.split(r",(?![^()]*\))", dm.group(2)):
        nm = re.match(r"\s*\*?\s*(\w+)", part).group(1)
        decls.append((nm, dm.group(1)))
# lzc_out / lzc_out2 are the asm islands' "=m" frame slots: memory, never a reg/v pseudo
MEM = {"lzc_out", "lzc_out2"}
names = [(p, "param") for p in params] + [x for x in decls if x[0] not in MEM]

rtl = open(f"{d}/tu.i.rtl.fn").read()
vregs = sorted({int(x) for x in re.findall(r"reg/v:\w+ (\d+)", rtl)})
if len(vregs) != len(names):
    print(f"MISMATCH: {len(vregs)} reg/v pseudos vs {len(names)} declarations", file=sys.stderr)
greg = open(f"{d}/tu.i.greg.fn").read()
disp = {int(a): int(b) for a, b in re.findall(r"(\d+) in (\d+)", greg.split("Register dispositions:")[1])}
alloc = {}
for ln in open(f"{d}/alloc.txt"):
    am = re.search(r"ord=(\d+) pseudo=(\d+) hardreg=(-?\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)", ln)
    if am:
        alloc[int(am.group(2))] = am.groups()
count = {}
print(f"{'name':10s} {'type':6s} {'pseudo':>6s} {'reg':>4s}  allocator")
for (nm, ty), pr in zip(names, vregs):
    count[nm] = count.get(nm, 0) + 1
    if want and nm not in want:
        continue
    hr = disp.get(pr)
    reg = REGN.get(hr, str(hr)) if hr is not None else "-"
    if pr in alloc:
        o, _, _, nr, ll, pri = alloc[pr]
        how = f"global ord={o} nrefs={nr} livelen={ll} pri={pri}"
    elif hr is not None:
        how = "local-alloc"
    else:
        how = "no hard reg (eliminated/unused)"
    label = nm if count[nm] == 1 else f"{nm}#{count[nm]}"
    print(f"{label:10s} {ty:6s} {pr:6d} {reg:>4s}  {how}")
