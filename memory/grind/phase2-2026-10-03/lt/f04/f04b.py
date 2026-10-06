#!/usr/bin/env python3
# F04 batch, part b (on f04a.py): two 51268 holders take their pointee types.
# - The POLY_FT4 cursor D_800A37D4 (bb2.h), its frame base D_800A3720 and the two buffers
#   D_800A3420 / D_800A3424 func_80060B90 sets: POLY_FT4 * (F01 debt "D_800A37D4 int-held cursor").
# - The OT-entry holder D_800A34E4: u32 * (F01 debt "34E4 word views"); its scratchpad seed
#   Unk1F800000Rec.unkA0 becomes the u32 it holds.
# usage: f04b.py [opt=<name>,...]   writes tmp/p2/f04b/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f04a as A
sys.argv = _argv
OUT = "tmp/p2/f04b/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = A.sub1
fn = A.fn

def bb2(h):
    return sub1(h, "extern s32 D_800A37D4;\n", "extern POLY_FT4 *D_800A37D4;\n")

def game(g):
    return sub1(g, "    u8 unkA0[4];\n    u32 unkA4;\n", "    u32 unkA0;\n    u32 unkA4;\n")

def s51268(s):
    if "no37D4" not in OPT:
        s = sub1(s, "static s32 D_800A3420;\nstatic s32 D_800A3424;\n", "static POLY_FT4 *D_800A3420;\nstatic POLY_FT4 *D_800A3424;\n")
        s = sub1(s, "  D_800A3420 = arg1;\n  D_800A3424 = ret;\n",
                 "  D_800A3420 = (POLY_FT4 *)arg1;\n  D_800A3424 = (POLY_FT4 *)ret;\n")
        s = sub1(s, "extern s32 D_800A3720;\nvoid func_80060E04", "extern POLY_FT4 *D_800A3720;\nvoid func_80060E04")
        s = sub1(s, "s32 D_800A3720;\ns32 *D_800A3724;\ns32 D_800A372C;\ns32 D_800A37D4;\n",
                 "POLY_FT4 *D_800A3720;\ns32 *D_800A3724;\ns32 D_800A372C;\nPOLY_FT4 *D_800A37D4;\n")
        n = s.count("    extern s32 D_800A37D4;\n"); s = s.replace("    extern s32 D_800A37D4;\n", "")
        m = s.count("    extern s32 D_800A3720;\n"); s = s.replace("    extern s32 D_800A3720;\n", "")
        print("removed block externs: D_800A37D4 %d, D_800A3720 %d" % (n, m))
        s = sub1(s, "temp_a1 = (s32)-((D_800A37D4 - D_800A3720) * 0x33333333) >> 3;", "temp_a1 = D_800A37D4 - D_800A3720;")
        s = s.replace("(POLY_FT4 *)D_800A37D4", "D_800A37D4")
        s = s.replace("(POLY_FT4 *)D_800A3720", "D_800A3720")
        s = s.replace("D_800A37D4 = (s32)prim;", "D_800A37D4 = prim;")
        s = s.replace("D_800A37D4 = (s32)end;", "D_800A37D4 = end;")
        s = s.replace("D_800A37D4 = (s32)*end;", "D_800A37D4 = *end;")
        s = s.replace("D_800A37D4 = (s32)*p_end;", "D_800A37D4 = *p_end;")
        s = s.replace("D_800A37D4 != (s32)prim", "D_800A37D4 != prim")
    if "no34E4" not in OPT:
        s = sub1(s, "static u8 *D_800A34E4;\n", "static u32 *D_800A34E4;\n")
        s = sub1(s, "    D_800A34E4 = SPAD51268->unkA0;\n", "    D_800A34E4 = &SPAD51268->unkA0;\n")
        s = re.sub(r"D_800A34E4 = g_gpu_ot_ptr \+ ([^;]+);", r"D_800A34E4 = (u32 *)(g_gpu_ot_ptr + \1);", s)
        s = s.replace("*(u32 *)D_800A34E4", "*D_800A34E4")
        def g(b):
            if "pa_s32" in OPT:
                b = sub1(b, "                D_800A34E4 = (u8 *)p_a;\n", "                D_800A34E4 = (u32 *)p_a;\n")
                return b
            # holder-first: p_a / p_a2 hold the OT entry as u32 *
            b = sub1(b, "            s32 *p_a;\n", "            u32 *p_a;\n")
            b = sub1(b, "                p_a = (s32 *)(g_gpu_ot_ptr + (s32)(entry * 4));\n",
                     "                p_a = (u32 *)(g_gpu_ot_ptr + (s32)(entry * 4));\n")
            b = sub1(b, "                D_800A34E4 = (u8 *)p_a;\n", "                D_800A34E4 = p_a;\n")
            b = sub1(b, "                    s32 *p_a2 = (s32 *)D_800A34E4;\n", "                    u32 *p_a2 = D_800A34E4;\n")
            return b
        s = fn(s, "func_80068D88", g)
    return s

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(A.write())
    out["51268.c"] = s51268(out["51268.c"])
    out["bb2.h"] = bb2(out["bb2.h"])
    out["game.h"] = game(out["game.h"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    left = [l.strip() for l in out["51268.c"].split(NL) if re.search(r"\(s32\)\*?(prim|end|p_end)|\(u8 \*\)D_800A34E4|\(POLY_FT4 \*\)D_800A3", l)]
    for l in left:
        print("LEFT", l)
    return out

if __name__ == "__main__":
    write()
    print("wrote f04b", sorted(OPT))
