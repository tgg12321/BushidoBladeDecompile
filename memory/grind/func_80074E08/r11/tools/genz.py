D = "tmp/e08/w"
b = open(f"{D}/w9_decl_init.c").read()
A1 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[7]);\n"
A2 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[7]);\n"
A3 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[8]);\n"
A4 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[8]);\n"
for a in (A1, A2, A3, A4): assert b.count(a) == 1
SEL_END = "        rect_x = 0x5E;\n    }\n"
V = {}
def all4(src, var):
    s = src.replace(A1, A1.replace("ot_idx * 4", var)).replace(A2, A2.replace("ot_idx * 4", var))
    return s.replace(A3, A3.replace("ot_idx * 4", var)).replace(A4, A4.replace("ot_idx * 4", var))
V["z1_reuse_all4_aftersel"] = all4(b.replace(SEL_END, SEL_END + "    tile_ot = ot_idx * 4;\n"), "tile_ot")
V["z2_reuse_last_permuter"] = b.replace(A2, "    tile_ot = ot_idx * 4;\n" + A2).replace(A4, A4.replace("ot_idx * 4", "tile_ot"))
V["z3_fresh_all4"] = all4(b.replace(SEL_END, SEL_END + "    ot_ofs = ot_idx * 4;\n"), "ot_ofs").replace("    s32 ot_idx;\n", "    s32 ot_idx;\n    s32 ot_ofs;\n")
V["z4_fresh_last"] = b.replace(A2, "    ot_ofs = ot_idx * 4;\n" + A2).replace(A4, A4.replace("ot_idx * 4", "ot_ofs")).replace("    s32 ot_idx;\n", "    s32 ot_idx;\n    s32 ot_ofs;\n")
V["z5_reuse_last_before"] = b.replace(A4, "    tile_ot = ot_idx * 4;\n" + A4.replace("ot_idx * 4", "tile_ot"))
# ot as offset in the select itself: second select writes the scaled value
z6 = b.replace("        ot_idx = 0xE;\n        rect_x = 0x14E;", "        tile_ot = 0xE * 4;\n        rect_x = 0x14E;").replace("        ot_idx = 4;\n        rect_x = 0x5E;", "        tile_ot = 4 * 4;\n        rect_x = 0x5E;")
V["z6_select_scaled_reuse"] = all4(z6, "tile_ot")
for k, v in V.items():
    assert v != b, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V))
V2 = {}
def use(src, which, var="tile_ot"):
    for a in which:
        src = src.replace(a, a.replace("ot_idx * 4", var))
    return src
pre = lambda a, s: s.replace(a, "    tile_ot = ot_idx * 4;\n" + a)
V2["z7_preA2_useA2A3A4"] = use(pre(A2, b), (A2, A3, A4))
V2["z8_preA2_useA3A4"] = use(pre(A2, b), (A3, A4))
V2["z9_preA3_useA3A4"] = use(pre(A3, b), (A3, A4))
V2["z10_preA3_useA4"] = use(pre(A3, b), (A4,))
V2["z11_preA2_useA2"] = use(pre(A2, b), (A2,))
OFS = "    offset[0] = ((DRAWENV *)SELWORK->f24)->ofs[0];\n    offset[1] = ((DRAWENV *)SELWORK->f24)->ofs[1]\n"
assert b.count(OFS) == 1
V2["z12_preofs_useA3A4"] = use(b.replace(OFS, "    tile_ot = ot_idx * 4;\n" + OFS), (A3, A4))
V2["z13_preofs_useA4"] = use(b.replace(OFS, "    tile_ot = ot_idx * 4;\n" + OFS), (A4,))
for k, v in V2.items():
    assert v != b, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V2))
