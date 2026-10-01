"""mkS.py: one-variable-per-value spellings of F (Ruling 11 (D)(3) search) -> tmp/func_800770B8/S_<name>.c"""
D = "tmp/func_800770B8/"
S = open(D + "F_split.c").read()
A = "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    list = (s32 *)(arg0 + 0x58);\n    D_800A35D8 = arg0;\n"
assert S.count(A) == 1
V = {}
V["base"] = S
# list assigned at the top (before the sp clears), after the stores, as an initializer
V["top"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    D_800A35D8 = arg0;\n").replace(
    "    do { } while (0);\n", "    do { } while (0);\n    list = (s32 *)(arg0 + 0x58);\n")
V["first"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    D_800A35D8 = arg0;\n").replace(
    "    sp[0] = 0;\n", "    list = (s32 *)(arg0 + 0x58);\n    sp[0] = 0;\n", 1)
V["after"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    D_800A35D8 = arg0;\n    list = (s32 *)(arg0 + 0x58);\n")
V["init"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    D_800A35D8 = arg0;\n").replace(
    "    s32 *list;\n", "    s32 *list = (s32 *)(arg0 + 0x58);\n")
# Ruling 4 two-statement list pointer (laneD r1 / r2)
V["r1"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    list = (s32 *)arg0;\n    list += 0x58 / 4;\n    D_800A35D8 = arg0;\n")
V["r2"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    list = (s32 *)arg0;\n    list = (s32 *)((u8 *)list + 0x58);\n    D_800A35D8 = arg0;\n")
# the list pointer as SelWork's own type of f04 through a typed local, and the store through the global
V["glob"] = S.replace("        work->f04 = list;\n", "        SELWORK->f04 = list;\n")
# arg0 itself unchanged but D_800A35D8 read back for the list (no second local)
V["darg"] = S.replace(A, "    ClearOTagR(g_gpu_ot_ptr, 0x1008);\n    D_800A35D8 = arg0;\n    list = (s32 *)(D_800A35D8 + 0x58);\n")
for k, v in V.items():
    if k != "base":
        assert v != S, k
    open(D + f"S_{k}.c", "w", newline="\n").write(v)
print(" ".join(D + f"S_{k}.c" for k in V))
