"""Ruling 11 spellings of func_8002F770 built from the landing body tmp/func_8002F770/cand.c
(work = determinant then sqrt; temp = sum of squares then table byte). Writes tmp/func_8002F770/r11v/."""
import os

B = open("memory/grind/func_8002F770/candidate.c", encoding="utf-8").read()
O = "tmp/func_8002F770/r11v"
os.makedirs(O, exist_ok=True)


def sub(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a
        t = t.replace(a, b)
    return t


def w(n, t):
    open(f"{O}/{n}.c", "w", encoding="utf-8", newline="\n").write(t)
    return f"{O}/{n}.c"


# strip the two Ruling 11 comments for the per-value forms
import re
plain = re.sub(r"    /\* work holds two values.*?\*/\n", "", B, flags=re.S)
plain = re.sub(r"    /\* temp holds two values.*?\*/\n", "", plain, flags=re.S)

WORK2 = [  # work's value 2 (the square root) -> fresh `dist`
    ("    s32 work;\n", "    s32 work;\n    s32 dist;\n"),
    ("        work = (u32)(&g_sqrt_table_u8)[temp] >> 3;\n", "        dist = (u32)(&g_sqrt_table_u8)[temp] >> 3;\n"),
    ("            work = (u32)(temp << 16)", "            dist = (u32)(temp << 16)"),
    ("    ang_y = ratan2(i2, work);\n", "    ang_y = ratan2(i2, dist);\n"),
]
TEMP2 = [  # temp's value 2 (the table byte) -> fresh block-local `tb`
    ("            temp = (&g_sqrt_table_u8)[(u32)temp >> shift];\n",
     "            s32 tb = (&g_sqrt_table_u8)[(u32)temp >> shift];\n"),
    ("(u32)(temp << 16)", "(u32)(tb << 16)"),
]
out = [w("reuse", B)]
out.append(w("pv_both", sub(sub(plain, WORK2), TEMP2)))
out.append(w("pv_work_only", sub(plain, WORK2)))
out.append(w("pv_temp_only", sub(plain, TEMP2)))
# tb at function scope
t = sub(sub(plain, WORK2), TEMP2)
t = sub(t, [("            s32 tb = (&g_sqrt", "            tb = (&g_sqrt"), ("    s32 dist;\n", "    s32 dist;\n    s32 tb;\n")])
out.append(w("pv_both_tbfn", t))
# declaration order: dist before work
t = sub(sub(plain, WORK2), TEMP2)
t = sub(t, [("    s32 work;\n    s32 dist;\n", "    s32 dist;\n    s32 work;\n")])
out.append(w("pv_both_order", t))
# determinant in its own nested block
t = sub(sub(plain, WORK2), TEMP2)
FIRST = "    work = (d0 + m->m[1][0] * (c1 >> 12) + m->m[2][0] * (c2 >> 12)) >> 12;\n"
LAST = "    r2 = (m->m[0][0] * m->m[1][2] - m->m[0][2] * m->m[1][0]) / work;\n"
t = t.replace("    s32 work;\n", "")
i, j = t.index(FIRST), t.index(LAST) + len(LAST)
inner = "        " + t[i:j].lstrip(" ").replace("\n    ", "\n        ")
inner = inner.replace("        work = (", "        s32 det = (", 1).replace("/ work;", "/ det;")
t = t[:i] + "    {\n" + inner + "    }\n" + t[j:]
assert "work" not in t
out.append(w("pv_both_detblock", t))
# table byte read inline, no second variable (temp single-valued)
t = sub(plain, [("            temp = (&g_sqrt_table_u8)[(u32)temp >> shift];\n", ""),
                ("(u32)(temp << 16)", "(u32)((&g_sqrt_table_u8)[(u32)temp >> shift] << 16)")])
out.append(w("table_inline", t))
t = sub(sub(plain, WORK2), [("            temp = (&g_sqrt_table_u8)[(u32)temp >> shift];\n", ""),
                            ("(u32)(temp << 16)", "(u32)((&g_sqrt_table_u8)[(u32)temp >> shift] << 16)")])
out.append(w("pv_work_table_inline", t))
open("tmp/func_8002F770/r11v_list.txt", "w", encoding="utf-8", newline="\n").write(",".join(out))
print(len(out))
