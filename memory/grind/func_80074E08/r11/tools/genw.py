import re
D = "tmp/e08/w"
b = open("memory/grind/func_80074E08/r9/variants/reuse.c").read()
FIRST = "    ot_idx = 4;\n    if (arg1 != 0) {\n        ot_idx = 0xE;\n    }\n    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, prim);\n"
assert FIRST in b
SECOND_USES = ["AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[7]);", "AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[7]);",
               "AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[8]);", "AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[8]);"]
V = {}
def split(src, first_name, decl_after="    s32 ot_idx;\n"):
    s = src.replace(FIRST, FIRST.replace("ot_idx", first_name))
    return s.replace(decl_after, decl_after + "    s32 %s;\n" % first_name, 1)
V["w1_split_tileot"] = split(b, "tile_ot")
V["w2_split_tileot_declfirst"] = b.replace(FIRST, FIRST.replace("ot_idx", "tile_ot")).replace("    EnvB s;\n", "    s32 tile_ot;\n    EnvB s;\n", 1)
V["w3_ternary_arg"] = b.replace(FIRST, "    AddPrim(g_gpu_ot_ptr + (arg1 != 0 ? 0xE : 4) * 4 + 0x24, prim);\n")
V["w4_ternary_arg_ifelse"] = b.replace(FIRST, "    if (arg1 != 0) {\n        AddPrim(g_gpu_ot_ptr + 0xE * 4 + 0x24, prim);\n    } else {\n        AddPrim(g_gpu_ot_ptr + 4 * 4 + 0x24, prim);\n    }\n")
V["w5_split_ifelse_first"] = split(b, "tile_ot").replace("    tile_ot = 4;\n    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n", "    if (arg1 != 0) {\n        tile_ot = 0xE;\n    } else {\n        tile_ot = 4;\n    }\n")
V["w6_ternary_local"] = split(b, "tile_ot").replace("    tile_ot = 4;\n    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n", "    tile_ot = arg1 != 0 ? 0xE : 4;\n")
for k, v in V.items():
    assert v != b, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V))
W1 = V["w1_split_tileot"]
T1 = "    tile_ot = 4;\n    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n"
assert T1 in W1
V2 = {}
x = W1.replace(T1, "    if (arg1 != 0) {\n        tile_ot = 0xE;\n    }\n")
V2["w7_init_after_prim"] = x.replace("    prim = arg0[5];\n", "    prim = arg0[5];\n    tile_ot = 4;\n", 1)
V2["w8_init_first"] = x.replace("    prim = arg0[5];\n", "    tile_ot = 4;\n    prim = arg0[5];\n", 1)
V2["w9_decl_init"] = x.replace("    s32 tile_ot;\n", "    s32 tile_ot = 4;\n", 1)
for k, v in V2.items():
    assert v != W1, k
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V2))
