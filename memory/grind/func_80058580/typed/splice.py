#!/usr/bin/env python3
"""splice.py <variant-dir> <out-dir>
Builds <out-dir>/text1b.c and <out-dir>/inc/code6cac.h from the CURRENT src/text1b.c and include/code6cac.h:
  * jtbl transcriptions removed, INCLUDE_ASM func_80058580 -> <variant>/f58580.c (or tmp/func_80058580/h/f58580.c)
  * func_80055138 body -> f55138.c if present (variant dir first, then h/)
  * `extern u8 D_8009A830;` .. end of func_80056FE8 -> f56fe8.c if present
  * header edits: hdr_edits.py (EDITS = [(old, new), ...]) if present
  * extra text1b.c edits: src_edits.py (EDITS) if present
"""
import sys, os, importlib.util
H = os.path.dirname(os.path.abspath(__file__))
var, out = sys.argv[1], sys.argv[2]
os.makedirs(out + "/inc", exist_ok=True)


def pick(name):
    for d in (var, H):
        p = os.path.join(d, name)
        if os.path.exists(p):
            return p
    return None


def load_edits(name):
    p = pick(name)
    if not p:
        return []
    spec = importlib.util.spec_from_file_location(name[:-3], p)
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m.EDITS


def rd(p):
    return open(p, encoding="utf-8").read()


src = rd("src/text1b.c")
# func_80058580 + jtbl arrays
a = src.index("/* func_80058580's three switch tables")
inc = 'INCLUDE_ASM("asm/funcs", func_80058580);'
b = src.index(inc) + len(inc)
src = src[:a] + rd(pick("f58580.c")).rstrip("\n") + src[b:]
# func_80055138
p = pick("f55138.c")
if p:
    a = src.index("void func_80055138(")
    b = src.index("\n}\n", a) + 3
    src = src[:a] + rd(p).rstrip("\n") + "\n" + src[b:]
# func_80056FE8 (+ its table externs)
p = pick("f56fe8.c")
if p:
    a = src.index("extern u8 D_8009A830;")
    b = src.index("s32 func_80056FE8(")
    b = src.index("\n}\n", b) + 3
    src = src[:a] + rd(p).rstrip("\n") + "\n" + src[b:]
for o, n in load_edits("src_edits.py"):
    assert src.count(o) == 1, ("src", o[:60], src.count(o))
    src = src.replace(o, n)
open(out + "/text1b.c", "w", encoding="utf-8", newline="\n").write(src)

hdr = rd("include/code6cac.h")
for o, n in load_edits("hdr_edits.py"):
    assert hdr.count(o) == 1, ("hdr", o[:60], hdr.count(o))
    hdr = hdr.replace(o, n)
open(out + "/inc/code6cac.h", "w", encoding="utf-8", newline="\n").write(hdr)
print("spliced", out)
