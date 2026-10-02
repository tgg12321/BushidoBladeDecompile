"""mks23.py <own_dir> <out_dir> <first_tail_item> : from the S1 ("own") split, move the cluster object's
items from <first_tail_item> on to the top of the tail (before its first moved line), leaving the earlier
items in text1b_ro.c. Probe only (survivor test of rodata-object-alignment condition 2)."""
import os, shutil, sys
od, out, first = sys.argv[1:4]
os.makedirs(out, exist_ok=True)
ro = open(os.path.join(od, "text1b_ro.c"), encoding="utf-8").read().split("\n")
k = next(i for i, l in enumerate(ro) if l.startswith("/* %s:" % first))
keep, move = ro[:k], ro[k:]
open(os.path.join(out, "text1b_ro.c"), "w", encoding="utf-8", newline="\n").write("\n".join(keep) + "\n")
t = open(os.path.join(od, "text1b_tu1b.c"), encoding="utf-8").read().split("\n")
c = t.index('INCLUDE_ASM("asm/funcs", math_RotMatrixZYX);')
t[c:c] = move + [""]
open(os.path.join(out, "text1b_tu1b.c"), "w", encoding="utf-8", newline="\n").write("\n".join(t))
shutil.copy(os.path.join(od, "text1b.c"), os.path.join(out, "text1b.c"))
