# func_8002CD58 Ruling 11 variants from ff-b final.c
import os, re
ROOT = os.path.dirname(os.path.abspath(__file__)) + "/../.."
OUT = os.path.dirname(os.path.abspath(__file__)) + "/v"
b = open(ROOT + "/memory/grind/func_8002CD58/ff-b-2026-09-30/final.c").read()
b = re.sub(r"\*\(\(\(u8 \*\)&g_sqrt_table_u8\) \+ ([^;]*?)\)(;| >> 3;)",
           lambda m: "g_sqrt_table_u8[%s]%s" % (m.group(1), m.group(2)), b)
assert "(u8 *)&g_sqrt" not in b, b
b = re.sub(r"g_sqrt_table_u8\[\(\(u32\)(\w+) >> shift\)\]", r"g_sqrt_table_u8[(u32)\1 >> shift]", b)
# strip the two FAKE comments
b = re.sub(r"    /\* FAKE: dist holds.*?\*/\n", "", b, flags=re.S)
b = b.replace("    s32 dist; /* SOTN: src/dra/4B758.c:71 @db41b28 */\n", "    s32 dist;\n")
b = re.sub(r"            /\* FAKE: nxz_sq is reused.*?\*/\n", "", b, flags=re.S)
assert "FAKE" not in b and "SOTN" not in b
reuse = re.sub(r"\bnxz_sq\b", "temp", b)
V = {"reuse": reuse}
i2 = reuse.index("yaw = ratan2(")
i3 = reuse.index("nyaw = ratan2(")
def split_dist(src, n1, n2, n3):
    a, s2, s3 = src[:i2], src[i2:i3], src[i3:]
    if n1 != "dist": a = re.sub(r"(?<!s32 )\bdist\b", n1, a)
    s2 = re.sub(r"\bdist\b", n2, s2)
    s3 = re.sub(r"\bdist\b", n3, s3)
    decl = "".join("    s32 %s;\n" % n for n in dict.fromkeys([n1, n2, n3]))
    return (a + s2 + s3).replace("    s32 dist;\n", decl, 1)
def split_temp(src):
    s = src.replace("            temp = g_sqrt_table_u8[(u32)temp >> shift];\n            dist",
                    "            s32 tbl = g_sqrt_table_u8[(u32)temp >> shift];\n            dist")
    s = s.replace("(u32)(temp << 16)", "(u32)(tbl << 16)")
    assert s != src
    return re.sub(r"\btemp\b", "nxz_sq", s)
V["pv_temp"] = split_temp(reuse)
V["pv_dist"] = split_dist(reuse, "dist", "xz_dist", "nxz_dist")
V["pv_all"] = split_temp(V["pv_dist"].replace("temp", "temp")) if True else None
V["pv_all"] = split_dist(V["pv_temp"], "dist", "xz_dist", "nxz_dist")
V["abl_v1"] = split_dist(reuse, "len", "dist", "dist")
V["abl_v2"] = split_dist(reuse, "dist", "xz_dist", "dist")
V["abl_v3"] = split_dist(reuse, "dist", "dist", "nxz_dist")
for k, s in V.items():
    open(f"{OUT}/{k}.c", "w", newline="\n").write(s)
print(sorted(V))
# shape B: dist = {v1, v2} declared in the n-small block; v3 its own nxz_dist
OPEN = "        && (u32)(*(s32 *)(obj + 0xD0) + 0x3FFF) < 0x7FFF) {\n"
def blockdecl(src, names):
    for n in names:
        src = src.replace("    s32 %s;\n" % n, "", 1)
    return src.replace(OPEN, OPEN + "".join("        s32 %s;\n" % n for n in names), 1)
V2 = {}
V2["B_blk"] = blockdecl(V["abl_v3"], ["dist"])
V2["B_blk_pv"] = blockdecl(split_dist(reuse, "len", "dist", "nxz_dist"), ["len", "dist"])
V2["B_blk_pv_rev"] = blockdecl(split_dist(reuse, "len", "dist", "nxz_dist"), ["dist", "len"])
V2["B_blk_pvtemp"] = split_temp(V2["B_blk"])
for k, s in V2.items():
    open(f"{OUT}/{k}.c", "w", newline="\n").write(s)
print(sorted(V2))
