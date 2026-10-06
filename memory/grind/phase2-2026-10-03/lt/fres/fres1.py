#!/usr/bin/env python3
# Lane residual (fres1): 2B344's scene-point and scene-flag helpers typed. func_8003FE40 takes the SVECTOR point
# table func_8003FA24 builds and writes its pad words by index; func_8004001C / func_80040068 take the Scene and
# set its quads' / records' quad flags by member. (The other residual candidates are debt rows, measured.)
# usage: fres1.py   writes tmp/p2/fres1/ (scratch only); BASE_REV env (default ebacf0123)
import os, subprocess
NL = chr(10)
OUT = "tmp/p2/fres1/"
BASE_REV = os.environ.get("BASE_REV", "ebacf0123")

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a[:70]
        t = t.replace(a, b)
    return t

FE40_OLD = """s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    s32 i;
    i = 0;
    if (a1 > i) {
        s16 fill = -256;
        s16 *p = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)p + 6) = fill;
            p = (s16 *)((u8 *)p + 8);
        }
    }
"""
FE40_NEW = """s16 *func_8003FE40(SVECTOR *a0, s32 a1, s16 *a2) {
    s32 i; /* FAKE: also the point index read from a2 in the second loop; its own local there: score 12 */
    for (i = 0; i < a1; i++) {
        a0[i].pad = -256;
    }
"""
INNER_OLD = """                    s32 addr;
                    i = a2[0];
                    a2++;
                    count--;
                    addr = (i << 3) + (s32)a0;
                    *(s16 *)(addr + 6) = val;
"""
INNER_NEW = """                    i = a2[0];
                    a2++;
                    count--;
                    a0[i].pad = val;
"""

def s2B344(t):
    return rep(t, [
        (FE40_OLD, FE40_NEW), (INNER_OLD, INNER_NEW),
        ("    return (s16 *)a2;\n}\n\nvoid func_8003FECC", "    return a2;\n}\n\nvoid func_8003FECC"),
        ("s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2);", "s16 *func_8003FE40(SVECTOR *a0, s32 a1, s16 *a2);"),
        ("    func_8003FE40((s16 *)init.in.points, init.in.count, cmds);", "    func_8003FE40(init.in.points, init.in.count, cmds);"),
        ("void func_8004001C(u8 *a0) {\n    s32 i;\n    for (i = 0; i < *(s16 *)a0; i++) {\n"
         "        a0[0x41A + i * 0x10] = 1;\n        a0[0xE + i * 0xD0] = 1;\n",
         "void func_8004001C(Scene *a0) {\n    s32 i;\n    for (i = 0; i < a0->count; i++) {\n"
         "        a0->quads[i].unk2 = 1;\n        a0->recs[i].quad.unk2 = 1;\n"),
        ("void func_80040068(u8 *a0) {\n    s32 i;\n    for (i = 0; i < *(s16 *)a0; i++) {\n"
         "        a0[0x41A + i * 0x10] = 0;\n        a0[0xE + i * 0xD0] = 0;\n",
         "void func_80040068(Scene *a0) {\n    s32 i;\n    for (i = 0; i < a0->count; i++) {\n"
         "        a0->quads[i].unk2 = 0;\n        a0->recs[i].quad.unk2 = 0;\n"),
        ("        func_8004001C((u8 *)s0);", "        func_8004001C(s0);"),
        ("        func_80040068((u8 *)s0);", "        func_80040068(s0);"),
    ])

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {"2B344.c": s2B344(show("src/main/2B344.c"))}
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fres1")
