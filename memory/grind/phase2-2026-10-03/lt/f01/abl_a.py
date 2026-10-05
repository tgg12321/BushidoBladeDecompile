#!/usr/bin/env python3
# F01a FAKE ablations: one candidate body per variant (each FAKE or cluster removed alone) under
# tmp/p2/lt/f01/abl/<name>.c, for `engine sandbox <func> --disable all --candidate <file>`.
# Base: the staged src/main/51268.c (F01a applied).
import os, re, sys
sys.path.insert(0, "memory/grind/phase2-2026-10-03/lt/f02"); sys.argv = sys.argv[:1]
import f02b2 as B

S = open("src/main/51268.c", encoding="utf-8").read()
OUT = "tmp/p2/lt/f01/abl"
os.makedirs(OUT, exist_ok=True)

def body(fn):
    i, j = B.span(S, fn)
    return S[i:j]

def rep(b, a, c, n=1):
    assert b.count(a) == n, (a[:80], b.count(a), n)
    return b.replace(a, c)

def cut_line(b, needle):
    i = b.index(needle); i = b.rindex("\n", 0, i) + 1; j = b.index("\n", i) + 1
    return b[:i] + b[j:]

V = {}

# ---- func_800620B8: sprite-table aliases, the round trip
def direct(b, var, tbl):
    # drop the alias's declaration and assignment (both carry "FAKE: alias"), use the table directly
    for l in [l for l in b.split("\n") if re.search(r"\b%s\b" % var, l) and ("FAKE: alias" in l)]:
        b = b.replace(l + "\n", "", 1)
    return re.sub(r"\b%s\b" % var, tbl, b)

b0 = body("func_800620B8")
TB = {"strip32": "D_8009BA00", "alt32": "D_8009BA50", "strip16": "D_8009BA30", "alt16": "D_8009BA58"}
b = b0
a = b.index("    /* FAKE: pointer aliases of the four sprite tables")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
for v, t in TB.items():
    b = direct(b, v, t)
V["620B8_tables"] = ("func_800620B8", b)
for v in ("alt32", "strip16", "alt16"):
    V["620B8_" + v] = ("func_800620B8", direct(b0, v, TB[v]))
V["620B8_roundtrip"] = ("func_800620B8", rep(b0, " + (s32)strip32 - (s32)strip32 + (s32)strip32;", " + (s32)strip32;"))

# ---- func_8006295C: prim staging, end cursor
b0 = body("func_8006295C")
b = rep(b0, "    POLY_FT4 *end;\n", "    POLY_FT4 *end;\n    u8 *wa;\n")
a = b.index("    /* FAKE: the work-area base (D_800A34EC) is staged through `prim`")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
b = rep(b, "    prim = (POLY_FT4 *)D_800A34EC;\n", "    wa = (u8 *)D_800A34EC;\n")
k = b.index("    prim = (POLY_FT4 *)D_800A37D4;\n")
b = b[:k].replace("(u8 *)prim + 0x", "wa + 0x") + b[k:]
V["6295C_prim"] = ("func_8006295C", b)
b = b0
a = b.index("        /* FAKE: `end` keeps the fill position")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
b = rep(b, """        end = prim;
        for (prim = (POLY_FT4 *)D_800A37D4, k = 0; prim < end; prim++, k++) {
            AddPrim(g_gpu_ot_ptr + zbuf[k] * 4, prim);
        }
        D_800A37D4 = (s32)end;
""", """        for (end = (POLY_FT4 *)D_800A37D4, k = 0; end < prim; end++, k++) {
            AddPrim(g_gpu_ot_ptr + zbuf[k] * 4, end);
        }
        D_800A37D4 = (s32)prim;
""")
V["6295C_end"] = ("func_8006295C", b)

# ---- func_80063E10: prim staging, bit, end cursor
b0 = body("func_80063E10")
b = rep(b0, "    s32 bit;\n", "    s32 bit;\n    u8 *wa;\n")
a = b.index("    /* FAKE: the work-area base (D_800A34EC) is staged through `prim`")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
b = rep(b, "    prim = (POLY_FT4 *)D_800A34EC;\n", "    wa = (u8 *)D_800A34EC;\n")
k = b.index("    if (D_800A344C[lane] < 10) {")
b = b[:k].replace("(u8 *)prim + 0x", "wa + 0x") + b[k:]
V["63E10_prim"] = ("func_80063E10", b)
b = b0
a = b.index("        /* FAKE: the slot's mask is named `bit`")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
b = rep(b, "(bit = 1 << i)", "(1 << i)")
V["63E10_bit"] = ("func_80063E10", b)
b = b0
a = b.index("    /* FAKE: `end` keeps the fill position")
e = b.index("*/\n", a) + 3
b = b[:a] + b[e:]
b = rep(b, """    end = prim;
    for (prim = (POLY_FT4 *)D_800A37D4, k = 0; prim < end; prim++, k++) {
        D_800A34E8 = (s32)prim;
""", """    for (end = (POLY_FT4 *)D_800A37D4, k = 0; end < prim; end++, k++) {
        D_800A34E8 = (s32)end;
""")
b = rep(b, "    D_800A37D4 = (s32)end;\n    return 1;\n", "    D_800A37D4 = (s32)prim;\n    return 1;\n")
V["63E10_end"] = ("func_80063E10", b)

# ---- func_80065800: named intermediates, tbl alias
b0 = body("func_80065800")
def drop_decl(b, var):
    for l in b.split("\n"):
        if re.match(r"\s+s(32|16) \*?%s; /\* FAKE" % var, l):
            i = b.index(l + "\n"); j = i + len(l) + 1
            if var == "tbl":
                j = b.index("*/\n", i) + 3
            return b[:i] + b[j:]
    raise KeyError(var)
b = drop_decl(b0, "w")
b = rep(b, "            w = *p_w;\n", "")
b = rep(b, "p_v->vx = -w;", "p_v->vx = -*p_w;"); b = rep(b, "p_v->vx = w;", "p_v->vx = *p_w;")
V["65800_w"] = ("func_80065800", b)
b = drop_decl(b0, "h")
b = rep(b, "            h = *p_h;\n", "")
b = rep(b, "p_v->vy = -h;", "p_v->vy = -*p_h;"); b = rep(b, "p_v->vy = h;", "p_v->vy = *p_h;")
V["65800_h"] = ("func_80065800", b)
b = drop_decl(b0, "sw")
b = rep(b, "        sw = (rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25;\n", "")
b = rep(b, "*p_w = (*p_w * sw >> 13) / 2;", "*p_w = (*p_w * ((rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25) >> 13) / 2;")
V["65800_sw"] = ("func_80065800", b)
b = drop_decl(b0, "sh")
b = rep(b, "        sh = rsin((D_800F0BA8[arg0] << 11) / 4551) * 15;\n", "")
b = rep(b, "*p_h = (*p_h * sh >> 12) / 2;", "*p_h = (*p_h * (rsin((D_800F0BA8[arg0] << 11) / 4551) * 15) >> 12) / 2;")
V["65800_sh"] = ("func_80065800", b)
b = drop_decl(b0, "tbl")
b = rep(b, "        tbl = D_800F0BA8;\n        t = tbl + arg0;\n", "        t = D_800F0BA8 + arg0;\n")
V["65800_tbl"] = ("func_80065800", b)

with open(OUT + "/list.txt", "w", newline="\n") as f:
    for name, (fn, b) in V.items():
        open("%s/%s.c" % (OUT, name), "w", encoding="utf-8", newline="\n").write(b)
        f.write("%s %s\n" % (name, fn))
print(len(V), "variants")
