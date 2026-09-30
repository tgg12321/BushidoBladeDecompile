# func_8002C22C spellings of record 1 (0x80102314 = D_80101EC8 + 0x44C)
import os, re
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT = os.path.dirname(os.path.abspath(__file__)) + "/v"
src = open(ROOT + "/src/code6cac_b_tu2.c").read()
m = re.search(r"^void func_8002C22C\(void\) \{\n.*?^\}\n", src, re.S | re.M)
body = m.group(0)
open(OUT + "/landed.c", "w").write(body)
DECL = "    s32 *d_tbl = &D_80102314;\n"
def sub(decl, fn):
    b = body.replace(DECL, decl)
    return re.sub(r"d_tbl\[(0x[0-9A-F]+)/4\]", lambda mm: fn(int(mm.group(1), 16)), b)
V = {
    "direct_idx": sub("", lambda o: "(&D_80102314)[0x%X/4]" % o),
    "direct_bytes": sub("", lambda o: "*(s32 *)((u8 *)&D_80102314 + 0x%X)" % o),
    "ec8_ptr": sub("    u8 *rec1 = (u8 *)&D_80101EC8 + 0x44C;\n", lambda o: "*(s32 *)(rec1 + 0x%X)" % o),
    "ec8_direct": sub("", lambda o: "*(s32 *)((u8 *)&D_80101EC8 + 0x44C + 0x%X)" % o),
    "ec8_s32ptr": sub("    s32 *rec1 = (s32 *)((u8 *)&D_80101EC8 + 0x44C);\n", lambda o: "rec1[0x%X/4]" % o),
}
for k, b in V.items():
    open(f"{OUT}/{k}.c", "w").write(b)
print(len(V))
