"""func_80074E08 Ruling 11 for `work`: the one-variable-per-value spellings and respellings of the landing
body memory/grind/func_80074E08/candidate.c, written to r11/variants/ as sandbox candidates
(r9/prefix.h + the body with EnvA -> EnvB, as in r9/tools/r9gen.py). usage: python r11gen.py"""
import os, re

H = os.path.dirname(os.path.abspath(__file__))
G = os.path.join(H, "..", "..")
OUT = os.path.join(H, "..", "variants")
prefix = open(os.path.join(G, "r9", "prefix.h")).read()
body = open(os.path.join(G, "candidate.c")).read()
body = re.sub(r"    /\* work holds two values.*?\*/\n", "", body, flags=re.S)
body = body.replace("    EnvA s;", "    EnvB s;")
SEL1 = "    if (arg1 != 0) {\n        work = 0xE;\n    } else {\n        work = 4;\n    }\n    AddPrim(g_gpu_ot_ptr + work * 4 + 0x24, prim);\n"
W2 = "    work = ot_idx * 4;\n"
assert SEL1 in body and body.count(W2) == 1
U = ["    AddPrim(g_gpu_ot_ptr + work, arg0[7]);\n", "    AddPrim(g_gpu_ot_ptr + work + 0x24, arg0[8]);\n",
     "    AddPrim(g_gpu_ot_ptr + work, arg0[8]);\n"]
for u in U:
    assert body.count(u) == 1
V = {"reuse": body}


def split(b, v1="tile_ot", v2="ot_ofs"):
    """value 1 -> v1, value 2 -> v2 (same statements)"""
    b = b.replace(SEL1, SEL1.replace("work", v1))
    b = b.replace(W2, W2.replace("work", v2))
    for u in U:
        b = b.replace(u, u.replace("work", v2))
    return b.replace("    s32 work;\n", "    s32 %s;\n    s32 %s;\n" % (v1, v2))


V["pv"] = split(body)
V["pv_rev_decl"] = split(body).replace("    s32 tile_ot;\n    s32 ot_ofs;\n", "    s32 ot_ofs;\n    s32 tile_ot;\n")
# value 1 in a block of its own (innermost scope)
V["pv_v1_block"] = split(body).replace("    s32 tile_ot;\n", "").replace(SEL1.replace("work", "tile_ot"),
    "    {\n        s32 tile_ot;\n" + "".join("    " + l + "\n" for l in SEL1.replace("work", "tile_ot").rstrip("\n").split("\n")) + "    }\n")
# no second value: every AddPrim computes ot_idx * 4
nov = split(body).replace(W2.replace("work", "ot_ofs"), "")
for u in U:
    nov = nov.replace(u.replace("work", "ot_ofs"), u.replace("work", "ot_idx * 4"))
V["pv_no_ofs"] = nov.replace("    s32 ot_ofs;\n", "")
# value 2 at other places
s = split(body).replace(W2.replace("work", "ot_ofs"), "")
SELEND = "        rect_x = 0x5E;\n    }\n"
V["pv_ofs_after_select_all4"] = s.replace(SELEND, SELEND + "    ot_ofs = ot_idx * 4;\n").replace(
    "    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[7]);\n", "    AddPrim(g_gpu_ot_ptr + ot_ofs + 0x24, arg0[7]);\n")
# value 1 spelled as default + override / ternary / at function entry
t1 = split(body)
SEL1t = SEL1.replace("work", "tile_ot")
V["pv_v1_default"] = t1.replace(SEL1t, "    tile_ot = 4;\n    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n    AddPrim(g_gpu_ot_ptr + tile_ot * 4 + 0x24, prim);\n")
V["pv_v1_declinit"] = t1.replace(SEL1t, "    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n    AddPrim(g_gpu_ot_ptr + tile_ot * 4 + 0x24, prim);\n").replace("    s32 tile_ot;\n", "    s32 tile_ot = 4;\n")
V["pv_v1_ternary"] = t1.replace(SEL1t, "    tile_ot = arg1 != 0 ? 0xE : 4;\n    AddPrim(g_gpu_ot_ptr + tile_ot * 4 + 0x24, prim);\n")
V["pv_v1_inarg"] = t1.replace(SEL1t, "    AddPrim(g_gpu_ot_ptr + (arg1 != 0 ? 0xE : 4) * 4 + 0x24, prim);\n").replace("    s32 tile_ot;\n", "")
# value 1 shares ot_idx instead (the reopened body's held-value re-store, for the record; not counting)
for k, b in V.items():
    if k != "reuse":
        assert b != body, k
    open(os.path.join(OUT, k + ".c"), "w", newline="\n").write(prefix + b)
print(",".join("memory/grind/func_80074E08/r11/variants/%s.c" % k for k in V))
