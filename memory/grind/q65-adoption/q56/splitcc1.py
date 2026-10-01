#!/usr/bin/env python3
"""splitcc1.py <in.s> <out1.s> <out2.s> <cut-func> <drop1,..|-> <drop2,..|->
Split a TU's PRE-maspsx stream (cpp|cc1|prologue_fix, -G0) into two files at top-level function
<cut-func> — the proxy for splitting the .c file there (GCC 2.7.2 compiles each function on its own).
The trailing file-scope `.comm` block (cc1 -G0 writes tentative definitions at the end) goes to both
parts, except that each part omits the symbols its own file does not define (drop1 / drop2)."""
import re, sys
src = open(sys.argv[1]).read().split("\n")
o1, o2, cut = sys.argv[2], sys.argv[3], sys.argv[4]
d1, d2 = [set(x.split(",")) - {"-"} for x in sys.argv[5:7]]

def keep(t, d):
    return [l for l in t if not (l.startswith("\t.comm") and l.split()[1].split(",")[0] in d)]

hdr = next(i for i, l in enumerate(src) if l.strip() == "#NO_APP") + 1
tail = len(src)
while tail > 0 and (src[tail - 1].startswith(("\t.comm", "\t.ident", "\t.lcomm", "\t.local")) or not src[tail - 1].strip()):
    tail -= 1
i = next(j for j in range(hdr, tail) if re.match(r"^\t\.globl\t%s$" % re.escape(cut), src[j]))
# back up over this function's own preamble (.text/.align/.rdata literals) to the previous item's end
j = i
while j > hdr:
    p = src[j - 1].strip()
    if p.startswith(".size") or p.startswith(".end") or p.startswith("#NO_APP"):
        break
    j -= 1
t = src[tail:]
open(o1, "w", newline="\n").write("\n".join(src[:j] + keep(t, d1)) + "\n")
inc = next(k for k, l in enumerate(src) if re.search(r'\.include "include/(macro|labels)\.inc"', l)) + 1
open(o2, "w", newline="\n").write("\n".join(src[:inc] + [" #NO_APP", "\t.text"] + src[j:tail] + keep(t, d2)) + "\n")
print(f"split {sys.argv[1]} at {cut}: part1 {j} lines, part2 {tail - j} lines; drops {sorted(d1)} / {sorted(d2)}")
