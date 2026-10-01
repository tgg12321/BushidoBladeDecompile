import pathlib
root = pathlib.Path(__file__).resolve().parents[2]
out = root / "tmp/rev58580dm"
body = (out / "v00_base.c").read_text()
for f in out.glob("v*.c"):
    if f.name != "v00_base.c":
        f.unlink()
def V(name, pairs):
    b = body
    for a, c in pairs:
        assert b.count(a) >= 1, (name, a)
        b = b.replace(a, c)
    (out / f"{name}.c").write_text(b, newline="\n")
V("w1_st2_u8decl", [("    u16 st2;\n", "    u8 st2;\n"), ("((u8)(st2 = D_8009A850[work4][1]) == 0xFF", "((st2 = D_8009A850[work4][1]) == 0xFF")])
V("w2_8U", [("work4 < 8U", "work4 < 8")])
V("w3_abc_inline", [("""                s32 b, c, a;
                a = CPU_S16(0x26E);
                b = CPU_S16(0x270);
                c = CPU_S16(0x272);
                work3 = 0x1000 - (((CPU_S16(0x26C) == 0 ? a + 4 + b : a + b) + c) << 8);""",
"""                work3 = 0x1000 - (((CPU_S16(0x26C) == 0 ? CPU_S16(0x26E) + 4 + CPU_S16(0x270) : CPU_S16(0x26E) + CPU_S16(0x270)) + CPU_S16(0x272)) << 8);""")])
V("w4_ok4_inline", [("""                    ok4 = 0;
                    if ((rand()""", """                    if ((rand()"""),
 ("""(CPU_S16(0x8A) == 0 || work2 >= 2)) {
                        ok4 = 1;
                    }
                    if (ok4) {
""", """(CPU_S16(0x8A) == 0 || work2 >= 2)) {
""")])
V("w5_all_neutral_casts", [("if ((u8)--p[0x362] != 0)", "if (--p[0x362] != 0)"), ("((u8)(p[0x3F2] / 3) % work2)", "((p[0x3F2] / 3) % work2)"), ("work3 = (u8)(D_800A38E2 / 10) * 2;", "work3 = (D_800A38E2 / 10) * 2;"), ("if ((u8)(D_800A38E2 % 10) == 0)", "if ((D_800A38E2 % 10) == 0)"), ("work1 = -(et < 5) & 100000;", "work1 = et < 5 ? 100000 : 0;")])
print("ok")
