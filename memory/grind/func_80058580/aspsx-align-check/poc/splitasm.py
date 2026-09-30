#!/usr/bin/env python3
"""splitasm.py <in.s> <out1.s> <cut1> <out2.s> [<cut2> <out3.s> ...]: split a compiled TU's
assembler stream into consecutive parts, each part starting at the top-level item <cutN> (a C
function, an INCLUDE_ASM function or an INCLUDE_RODATA blob). The cut backs up over the item's own
preamble (.align/.text/.globl/rodata literals emitted just before it) to the end of the previous
item. Every part gets the TU header (up to the macro.inc include). This is a proxy for splitting
the .c file at the same point: GCC 2.7.2 compiles each function independently of the others.

-G8 streams (cc1 TARGET_FILE_SWITCHING: all .globl/.size stubs first, then an .extern block, then
the bodies) are cut in both regions at the same item."""
import re, sys
args = sys.argv[1:]
src = open(args[0]).read().split("\n")
outs, cuts = [args[1]], []
rest = args[2:]
while rest:
    cuts.append(rest[0]); outs.append(rest[1]); rest = rest[2:]
hdr_end = next(i for i, l in enumerate(src) if 'include/macro.inc' in l) + 1
header = src[:hdr_end]

def back_up(i, lo):
    j = i
    while j > lo:
        p = src[j - 1].strip()
        if re.match(r'^\.size\s+\S+,\s*\.Lfe', p) or p.startswith((".include", ".end", ".extern")) or \
           (re.match(r'^[a-z]', p) and not p.startswith((".", "#"))):
            break
        j -= 1
    return j

def find(pat, lo, hi):
    for i in range(lo, hi):
        if re.search(pat, src[i]):
            return i
    raise SystemExit("not found: " + pat)

first_globl = find(r'^\.globl\s', hdr_end, len(src))
first_body = next((i for i in range(hdr_end, len(src)) if re.match(r'^\.type\s', src[i])), len(src))
g8 = any(src[i].startswith('.size') for i in range(first_globl, first_body))
if not g8:
    idx = []
    for c in cuts:
        i = find(r'^\.globl\s+%s\s*$|\.include "asm/(funcs|rodata)/%s\.s"' % (re.escape(c), re.escape(c)), hdr_end, len(src))
        idx.append(back_up(i, hdr_end))
    assert idx == sorted(idx), idx
    b = [hdr_end] + idx + [len(src)]
    parts = [header + ([".text"] if k else []) + src[b[k]:b[k + 1]] for k in range(len(outs))]
else:
    ext0 = find(r'^\.extern\s', first_globl, len(src))
    body0 = first_body - 1  # the ".text" line before the first .type
    while not src[body0].startswith(".text"):
        body0 -= 1
    ia, ib = [], []
    for c in cuts:
        ia.append(back_up(find(r'^\.globl\s+%s\s*$' % re.escape(c), hdr_end, ext0), hdr_end))
        ib.append(find(r'^\.type\s+%s,' % re.escape(c), body0, len(src)))
    a = [hdr_end] + ia + [ext0]
    b = [body0 + 1] + ib + [len(src)]
    ext = src[ext0:body0 + 1]
    parts = [header + [".text"] + src[a[k]:a[k + 1]] + ext + src[b[k]:b[k + 1]] for k in range(len(outs))]
for out, p in zip(outs, parts):
    open(out, "w", newline="\n").write("\n".join(p) + "\n")
print("g8" if g8 else "plain", [len(p) for p in parts])
