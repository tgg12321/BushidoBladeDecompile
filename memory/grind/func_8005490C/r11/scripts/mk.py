"""mk.py <body.c> <outdir> [--noA]: TU copies of src/text1b.c and src/code6cac_c2.c plus
inc/game.h with the (A) ctrl-block data model applied and <body.c> spliced over
func_8005490C's INCLUDE_ASM line."""
import sys
from pathlib import Path

body, outdir = sys.argv[1], Path(sys.argv[2])
noA = "--noA" in sys.argv
ptr = "--ptr" in sys.argv
(outdir / "inc").mkdir(parents=True, exist_ok=True)


def sub(s, old, new, count=1):
    n = s.count(old)
    if n != count:
        raise SystemExit(f"expected {count} x {old!r}, found {n}")
    return s.replace(old, new)


g = Path("include/game.h").read_text()
t = Path("src/text1b.c").read_text()
c = Path("src/code6cac_c2.c").read_text()
if ptr:
    g = sub(g, "/* 0x2C */ s32 unk2C;", "/* 0x2C */ u8 *unk2C;")
    g = sub(g, "/* 0x30 */ s32 unk30;", "/* 0x30 */ u8 *unk30;")
    g = sub(g, "/* 0x34 */ s32 unk34[2];", "/* 0x34 */ u8 *unk34[2];")
    g = sub(g, "/* 0x3C */ s32 unk3C[2];", "/* 0x3C */ u8 *unk3C[2];")

if not noA:
    t = sub(t, "extern void func_8003FFC4(s32);", "extern void func_8003FFC4(s32 *);")
    if ptr:
        t = sub(t, "s32 func_80054604(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6) {",
            "s32 func_80054604(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, u8 *a6) {")
    t = sub(t, "    s32 ret;\n    s16 *t;\n    s32 p;\n    s32 v;\n",
            "    s32 ret;\n    s16 *t;\n    %s p;\n    s32 *v;\n" % ("u8 *" if ptr else "s32"))
    if ptr:
        t = sub(t, "ret = func_80044FA0(id, a6);", "ret = func_80044FA0(id, (s32)a6);")
    if ptr:
        t = sub(t, "D_800EFAE8.unk2C = (s32)func_800469C4(id);", "D_800EFAE8.unk2C = (u8 *)func_800469C4(id);")
    t = sub(t, "v = (s32)func_8004153C(0);", "v = func_8004153C(0);")
    t = sub(t, "v = (s32)func_8004153C(1);", "v = func_8004153C(1);")
    if ptr:
        t = sub(t, "game_StageCleanup(n, a6);", "game_StageCleanup(n, (s32)a6);")
    if ptr:
        t = sub(t, "void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {",
            "void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, u8 *a7) {")
    if ptr:
        t = sub(t, "    s32 *p = &D_800EFAE8.unk2C;\n", "    u8 **p = &D_800EFAE8.unk2C;\n")

    if ptr:
        c = sub(c, "extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);",
            "extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, u8 *);")
    if ptr:
        c = sub(c, "    s32 magic;\n", "    u8 *magic;\n")
    if ptr:
        c = sub(c, "magic = 0x80190800;", "magic = (u8 *)0x80190800;")
    if ptr:
        c = sub(c, "magic = 0x80118800;", "magic = (u8 *)0x80118800;")
    if ptr:
        c = sub(c, "-1, -1, -1, (s32)0x80118800);", "-1, -1, -1, (u8 *)0x80118800);")

if body != "-":
    t = sub(t, 'INCLUDE_ASM("asm/funcs", func_8005490C);\n', Path(body).read_text())
(outdir / "inc" / "game.h").write_text(g, newline="\n")
(outdir / "text1b.c").write_text(t, newline="\n")
(outdir / "code6cac_c2.c").write_text(c, newline="\n")
print("ok", outdir)
