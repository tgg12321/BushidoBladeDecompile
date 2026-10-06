#!/usr/bin/env python3
# Owner ruling Q113 (rules 6dd0496dd), on top of F01d (63ca60f46): func_8006C21C's two
# `SetDrawMode(..., tw)` calls pass `(RECT *)tw`, a boundary conversion of the s16 local to the
# prototype's RECT *tw; the Q27 FAKE comment on tw says so.
# usage: q113.py [measure]   writes tmp/p2/q113/51268.c (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv; sys.argv = sys.argv[:1]
import f01d2 as D2
sys.argv = _argv
OUT = "tmp/p2/q113/"
sub1 = D2.sub1

def src(s):
    def b(t):
        old = "    SetDrawMode((DR_MODE *)arg0[7], 1, dtd, func_8006E480((s32)s.header, mode), tw);\n"
        t = sub1(t, old, old.replace(", tw);", ", (RECT *)tw);"), 2)
        t = sub1(t, """       sites write a literal 0. `tw` is passed as SetDrawMode's RECT *tw (an
       int -> pointer conversion cc1 reports): every pointer or int spelling of
       it measured (s32 + cast, RECT * local in any placement, initialised or
       assigned later, a literal 0) differs by 70 lines, because the mechanism
       needs the s16 read. */""", """       sites write a literal 0. `tw` is passed as SetDrawMode's RECT *tw through
       a `(RECT *)tw` boundary conversion (owner ruling Q113; cc1 reports it as a
       cast to pointer from an integer of different size): every other pointer
       or int spelling of it measured (s32 + cast, RECT * local in any placement,
       initialised or assigned later, a literal 0) differs by 70 lines, because
       the mechanism needs the s16 read. */""")
        return t
    return D2.fn(s, "func_8006C21C", b)

BASE_REV = "63ca60f46"   # F01d as committed (equal to f01d2.py's output)

def write():
    os.makedirs(OUT, exist_ok=True)
    def show(p):
        return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                              text=True, encoding="utf-8").stdout
    s = src(show("src/main/51268.c"))
    g, h, t = show("include/game.h"), show("include/bb2.h"), show("src/main/64FD8.c")
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

if __name__ == "__main__":
    s, g, h, t = write()
    if "measure" in sys.argv[1:]:
        D2.D1.measure(s, g, h, t, ["func_8006C21C"])
    print("wrote q113")
