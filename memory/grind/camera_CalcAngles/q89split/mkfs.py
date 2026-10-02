"""mkfs.py <x_jtbl_8001541C dir> <out> <a|b> : D_800153F0 becomes function-scope in func_80049F4C
(a: `static const` local; b: initialized automatic record), the rest of the block stays at the tail's top,
no rodata-only object."""
import os, shutil, sys
src, out, mode = sys.argv[1:4]
os.makedirs(out, exist_ok=True)
h = open(os.path.join(src, "text1b.c"), encoding="utf-8").read()
init = """{{
    0x0E00, 0x0E00, 0x0E00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0E00, 0x0A00, 0x0001, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0030, 0x0030, 0x0030, 0x1000,
}}"""
assert h.count("extern const Unk800153F0Record D_800153F0;\n") == 1
h = h.replace("extern const Unk800153F0Record D_800153F0;\n", "")
old = "    Unk800153F0Record sp10;\n    s32 i;\n    u8 *base;\n    sp10 = D_800153F0;\n"
assert h.count(old) == 1
if mode == "a":
    new = "    static const Unk800153F0Record D_800153F0 = %s;\n    Unk800153F0Record sp10;\n    s32 i;\n    u8 *base;\n    sp10 = D_800153F0;\n" % init
else:
    new = "    Unk800153F0Record sp10 = %s;\n    s32 i;\n    u8 *base;\n" % init
h = h.replace(old, new)
open(os.path.join(out, "text1b.c"), "w", encoding="utf-8", newline="\n").write(h)
shutil.copy(os.path.join(src, "text1b_tu1b.c"), os.path.join(out, "text1b_tu1b.c"))
