# func_8002CD58: dist / angle split spellings from the in-tree body
import os, re
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT = os.path.dirname(os.path.abspath(__file__)) + "/v"
src = open(ROOT + "/src/code6cac_b_tu2.c").read()
m = re.search(r"^s32 func_8002CD58\(u8 \*obj\) \{\n.*?^\}\n", src, re.S | re.M)
body = m.group(0)
open(OUT + "/landed.c", "w").write(body)
i2 = body.index("angle = ratan2(*(s32 *)(obj + 0xA8)")   # start of site 2 region
i3 = body.index("angle = ratan2(*(s32 *)(obj + 0xC8)")   # start of fallback (site 3)
def split(b, var, n2, n3, decl_after):
    a, s2, s3 = b[:i2], b[i2:i3], b[i3:]
    s2 = re.sub(r"\b%s\b" % var, n2, s2)
    s3 = re.sub(r"\b%s\b" % var, n3, s3)
    out = a + s2 + s3
    return out.replace(decl_after, decl_after + "    s32 %s;\n    s32 %s;\n" % (n2, n3), 1)
V = {}
V["dist_split"] = split(body, "dist", "xz_dist", "nxz_dist", "    s32 dist;\n")
V["angle_split"] = split(body, "angle", "angle2", "angle3", "    s32 angle;\n")
V["both_split"] = split(V["dist_split"], "angle", "angle2", "angle3", "    s32 angle;\n") if False else None
del V["both_split"]
for k, b in V.items():
    open(f"{OUT}/{k}.c", "w").write(b)
print(len(V))
# declaration-order and block-scope levers on the fully split form
ds = V["dist_split"]
open(f"{OUT}/dist_split_declfirst.c", "w").write(ds.replace("    s32 dist;\n    s32 xz_dist;\n    s32 nxz_dist;\n", "").replace("    s32 sp_tmp;\n", "    s32 dist;\n    s32 xz_dist;\n    s32 nxz_dist;\n    s32 sp_tmp;\n", 1))
open(f"{OUT}/dist_split_rev.c", "w").write(ds.replace("    s32 dist;\n    s32 xz_dist;\n    s32 nxz_dist;\n", "    s32 nxz_dist;\n    s32 xz_dist;\n    s32 dist;\n"))
# site 1 named len (its own role), sites 2/3 keep dist-named separate
open(f"{OUT}/dist_split_u32.c", "w").write(ds.replace("    s32 dist;\n", "    u32 dist;\n", 1))
print("extra")
# angle split into one name per write
def angle_names(b, names):
    parts = re.split(r"(angle = ratan2\()", b)
    # parts: [pre, sep, seg1, sep, seg2, sep, seg3, sep, seg4]
    out = parts[0]
    k = 0
    for i in range(1, len(parts), 2):
        seg = parts[i + 1]
        # the reader is the first `- angle;` after the write
        seg = seg.replace("0x800 - angle;", "0x800 - %s;" % names[k], 1)
        out += names[k] + " = ratan2(" + seg
        k += 1
    return out.replace("    s32 angle;\n", "".join("    s32 %s;\n" % n for n in dict.fromkeys(names)))
for nm, names in {"angle4": ["yaw", "pitch", "nyaw", "npitch"], "angle_yawpitch": ["yaw", "pitch", "yaw", "pitch"]}.items():
    b = angle_names(body, names)
    assert "angle" not in re.sub(r"/\*.*?\*/", "", b, flags=re.S).replace("angles", ""), nm
    open(f"{OUT}/{nm}.c", "w").write(b)
    open(f"{OUT}/{nm}_distsplit.c", "w").write(angle_names(V["dist_split"], names))
