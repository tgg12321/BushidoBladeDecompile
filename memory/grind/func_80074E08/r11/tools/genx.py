D = "tmp/e08/w"
b = open(f"{D}/w9_decl_init.c").read()
P = "    prim += 0x10;\n    arg0[5] = prim;\n"
assert P in b
V = {}
V["x1_store_sum"] = b.replace(P, "    arg0[5] = prim + 0x10;\n")
V["x2_decl_order"] = b.replace("    s32 prim;\n    s32 ot_idx;\n    s32 tile_ot = 4;\n", "    s32 tile_ot = 4;\n    s32 prim;\n    s32 ot_idx;\n")
V["x3_both"] = V["x2_decl_order"].replace(P, "    arg0[5] = prim + 0x10;\n")
V["x4_decl_init_prim"] = b.replace("    s32 prim;\n", "    s32 prim = arg0[5];\n").replace("    prim = arg0[5];\n", "")
V["x5_both_init"] = V["x4_decl_init_prim"].replace(P, "    arg0[5] = prim + 0x10;\n")
for k, v in V.items():
    assert v != b, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V))
