D = "tmp/e08/w"
b = open(f"{D}/w9_decl_init.c").read()
V = {}
y = b.replace("    s32 prim;\n", "    u8 *prim;\n").replace("    prim = arg0[5];\n", "    prim = (u8 *)arg0[5];\n")
y = y.replace("AddPrim(g_gpu_ot_ptr + tile_ot * 4 + 0x24, prim);", "AddPrim(g_gpu_ot_ptr + tile_ot * 4 + 0x24, (s32)prim);")
y = y.replace("    arg0[5] = prim;\n", "    arg0[5] = (s32)prim;\n")
V["y1_prim_u8p"] = y
V["y2_tileot_u32"] = b.replace("    s32 tile_ot = 4;\n", "    u32 tile_ot = 4;\n")
V["y3_tileot_s16"] = b.replace("    s32 tile_ot = 4;\n", "    s16 tile_ot = 4;\n")
V["y4_ot_u32_both"] = b.replace("    s32 tile_ot = 4;\n", "    u32 tile_ot = 4;\n").replace("    s32 ot_idx;\n", "    u32 ot_idx;\n")
for k, v in V.items():
    assert v != b, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V))
