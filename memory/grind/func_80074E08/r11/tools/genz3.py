D = "tmp/e08/w"
A2 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[7]);\n"
A3 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[8]);\n"
A4 = "    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[8]);\n"
def z7(s):
    s = s.replace(A2, "    tile_ot = ot_idx * 4;\n" + A2.replace("ot_idx * 4", "tile_ot"))
    return s.replace(A3, A3.replace("ot_idx * 4", "tile_ot")).replace(A4, A4.replace("ot_idx * 4", "tile_ot"))
V = {}
for base in ("w5_split_ifelse_first", "w6_ternary_local", "w7_init_after_prim", "w8_init_first", "w1_split_tileot"):
    s = open(f"{D}/{base}.c").read()
    V["z7_on_" + base] = z7(s)
for k, v in V.items():
    open(f"{D}/{k}.c", "w", newline="\n").write(v)
print(",".join(f"{D}/{k}.c" for k in V))
