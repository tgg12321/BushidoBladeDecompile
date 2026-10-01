import re, pathlib
root = pathlib.Path(__file__).resolve().parents[2]
src = (root / "src/text1b.c").read_text(encoding="utf-8")
i = src.index("s32 func_80058580(u8 *p) {")
j = src.index("\n}\n", i) + 3
body = src[i:j]
out = root / "tmp/rev58580dm"
(out / "v00_base.c").write_text(body, newline="\n")
def V(name, pairs):
    b = body
    for a, c in pairs:
        n = b.count(a)
        assert n >= 1, (name, a)
        b = b.replace(a, c)
    (out / f"{name}.c").write_text(b, newline="\n")
V("vA_st2_nocast", [("((u8)(st2 = D_8009A850[work4][1]) == 0xFF", "((st2 = D_8009A850[work4][1]) == 0xFF")])
V("vB_dec_nocast", [("if ((u8)--p[0x362] != 0)", "if (--p[0x362] != 0)")])
V("vC_s16_nocast", [("if ((s16)work2 < score)", "if (work2 < score)")])
V("vD_s8_nocast", [("(work1 = (s8)besti) != -1", "(work1 = besti) != -1")])
V("vE_u8div3_nocast", [("((u8)(p[0x3F2] / 3) % work2)", "((p[0x3F2] / 3) % work2)")])
V("vF_u8div10_nocast", [("work3 = (u8)(D_800A38E2 / 10) * 2;", "work3 = (D_800A38E2 / 10) * 2;")])
V("vG_u8mod10_nocast", [("if ((u8)(D_800A38E2 % 10) == 0)", "if ((D_800A38E2 % 10) == 0)")])
V("vH_shift4", [("((CPU_S16(0x438) * 0x100) >> 12)", "(CPU_S16(0x438) >> 4)")])
V("vI_noretest", [("(CPU_S16(0xE) >= 6 && p[0x34A] == 0)) &&\n              wtype == 1)) {", "(CPU_S16(0xE) >= 6 && p[0x34A] == 0)))) {")])
V("vJ_nodowhile", [("do {\n                            et = e[0] & 7;\n                        } while (0);", "et = e[0] & 7;")])
V("vK_vava", [("va * (vn ? 1000 : 300)", "va * va")])
V("vL_ternary", [("work1 = -(et < 5) & 100000;", "work1 = et < 5 ? 100000 : 0;")])
V("vN_opp6A_u16", [("st2 == CPU_OPP[0x6A]", "st2 == *(u16 *)(CPU_OPP + 0x6A)")])
V("vO_u32_nocast", [("(u32)work3 & ", "work3 & ")])
V("vM_noq", [("q = ep;\n", ""), ("if (q[0] == 0x40)", "if (ep[0] == 0x40)"), ("q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1]", "ep[4] << 24 | ep[3] << 16 | ep[2] << 8 | ep[1]")])
V("vMJ_noq_nodowhile", [("q = ep;\n", ""), ("if (q[0] == 0x40)", "if (ep[0] == 0x40)"), ("q[4] << 24 | q[3] << 16 | q[2] << 8 | q[1]", "ep[4] << 24 | ep[3] << 16 | ep[2] << 8 | ep[1]"), ("do {\n                            et = e[0] & 7;\n                        } while (0);", "et = e[0] & 7;")])
V("vQ_noob", [("lim = CPU_SARR(0x3FE) + (ob = CPU_OARR(0x3FE));", "lim = CPU_SARR(0x3FE) + CPU_OARR(0x3FE);"), ("lim = ob;", "lim = CPU_OARR(0x3FE);")])
print("ok")
