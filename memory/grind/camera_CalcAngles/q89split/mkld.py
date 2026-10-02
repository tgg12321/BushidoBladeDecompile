"""mkld.py <objdir>: bb2.ld with build/src/text1b.o replaced by <objdir>/text1b.o and, if present,
<objdir>/text1b_tu1b.o linked immediately after it in every section that names text1b.o."""
import os, sys
o = sys.argv[1]
tail = os.path.exists(os.path.join(o, "text1b_tu1b.o"))
out = []
for line in open("bb2.ld", encoding="utf-8").read().split("\n"):
    if "build/src/text1b.o(" in line:
        out.append(line.replace("build/src/text1b.o(", o + "/text1b.o("))
        if os.path.exists(os.path.join(o, "text1b_ro.o")) and "(.rodata)" in line:
            out.append(line.replace("build/src/text1b.o(", o + "/text1b_ro.o("))
        if tail:
            out.append(line.replace("build/src/text1b.o(", o + "/text1b_tu1b.o("))
    else:
        out.append(line)
print("\n".join(out))
