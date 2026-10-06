#!/usr/bin/env python3
# Lane residual (fres2): 31D3C func_80041BF4's three raw views. D_80094DF0 is the six-entry table of s16 row-list
# pointers (7D920.data.s: .word D_80094D80 ...) that func_80041AC8 and func_80041BF4 index; g_gpu_store_buf
# (0x800A9A24) is rows of 16 u16, which func_80041AC8 stores (16 x 1 StoreImage) and func_80041BF4 loads by row
# index; the terminator test reads the row-list entry by element.
# usage: fres2.py   writes tmp/p2/fres2/out/ (scratch only); BASE_REV env (default 765102ec8, ":" = index);
#        OPT env (comma list) builds a measured alternative: x_plain (x = tbl[0]: 25), x_u16 (u16 x = tbl[0]: 3).
import os, subprocess
NL = chr(10)
OUT = "tmp/p2/fres2/out/"
BASE_REV = os.environ.get("BASE_REV", "765102ec8")
OPT = set(x for x in os.environ.get("OPT", "").split(",") if x)

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a[:70]
        t = t.replace(a, b)
    return t

OFF_OLD = """    /* FAKE: off = idx << 5 taken before idx++; inline in LoadImage with idx++ after DrawSync, addiu s1 / sll v0 swap around the call setup (score 4). */
    s32 off = idx << 5;
    idx++;
"""
CALL_OLD = "    LoadImage(&rect[0], (u32 *)((u8 *)&g_gpu_store_buf + off));\n"
CALL_NEW = "    LoadImage(&rect[0], (u32 *)g_gpu_store_buf[idx++]);\n"

def s31D3C(t):
    pairs = [
        ("extern s32 D_80094DF0[];", "extern s16 *D_80094DF0[6];"),
        ("extern u16 g_gpu_store_buf;", "extern u16 g_gpu_store_buf[][16];"),
        ("  u16 *var_s1;\n", "  u16 (*var_s1)[16];\n"),
        ("  var_s0 = (s16 *) D_80094DF0[D_80094E08[*id_ptr]];", "  var_s0 = D_80094DF0[D_80094E08[*id_ptr]];"),
        ("    var_s1 = &g_gpu_store_buf;", "    var_s1 = g_gpu_store_buf;"),
        ("      var_s1 += 0x10;\n", "      var_s1++;\n"),
        ("  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[fp_ptr->unk_08] << 2));",
         "  tbl = D_80094DF0[D_80094E08[fp_ptr->unk_08]];"),
        (OFF_OLD, ""), (CALL_OLD, CALL_NEW),
    ]
    if "x_plain" in OPT:
        pairs.append(("  x = *(u16 *) tbl;", "  x = tbl[0];"))
    elif "x_u16" in OPT:
        pairs += [("  s16 *tbl;\n  s32 idx;\n  s32 x;\n", "  s16 *tbl;\n  s32 idx;\n  u16 x;\n"),
                  ("  x = *(u16 *) tbl;", "  x = tbl[0];")]
    else:
        pairs.append(("  x = *(u16 *) tbl;",
                      "  /* FAKE: x is the row's x zero-extended (the lhu beside the test's lh); as tbl[0] the pair is"
                      " one lh and the frame drops 88 -> 80: score 25 */\n  x = (u16)tbl[0];"))
    return rep(t, pairs)

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {"31D3C.c": s31D3C(show("src/main/31D3C.c"))}
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    for h in ("game.h", "bb2.h"):
        open(OUT + h, "w", encoding="utf-8", newline=NL).write(show("include/" + h))
    return out

if __name__ == "__main__":
    write()
    print("wrote fres2", sorted(OPT))
