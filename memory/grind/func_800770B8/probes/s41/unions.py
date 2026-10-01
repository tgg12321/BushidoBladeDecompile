"""unions.py <tree>: SelWork f1C/f20 become Q33/Q46 union word views (func_800770B8 clears each with one word
store: sw 0x20 / sw 0x1C at 0x800772BC/C0); every other f1C/f20 element access becomes .half[...]."""
import re, sys
T = sys.argv[1]
p = f"{T}/include/game.h"
t = open(p).read()
a = "    s16 f1C[2];\n    s16 f20[2];\n"
b = ("    union {\n        s16 half[2];\n        s32 word;\n    } f1C;\n"
     "    union {\n        s16 half[2];\n        s32 word;\n    } f20;\n")
assert t.count(a) == 1
open(p, "w", newline="\n").write(t.replace(a, b))
import glob
for f in glob.glob(f"{T}/src/*.c"):
    s = open(f).read()
    s2 = re.sub(r"->f(1C|20)\[", r"->f\1.half[", s)
    if s2 != s:
        open(f, "w", newline="\n").write(s2)
        print("respelled", f, len(re.findall(r"->f(1C|20)\[", s)))
