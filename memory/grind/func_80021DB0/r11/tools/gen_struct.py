#!/usr/bin/env python3
"""Structural respellings of the one-variable-per-value form (split_all.c) of func_80021DB0."""
from pathlib import Path

d = Path(__file__).resolve().parent
A = (d / "split_all.c").read_text()


def w(name, s):
    assert s != A, name
    (d / f"{name}.c").write_bytes(s.encode())
    print(name)


blk = "        s32 step;\n        s32 rise;\n"
assert blk in A
# S1: every per-value local at function scope
w("s_S1_funcscope", A.replace(blk, "").replace("    s32 sel;\n", "    s32 sel;\n    s32 step;\n    s32 rise;\n"))
# S2: sel declared first, block locals reversed
w("s_S2_declorder", A.replace(blk, "        s32 rise;\n        s32 step;\n").replace("    s32 sel;\n", "").replace("    Vec3i32 base;\n", "    s32 sel;\n    Vec3i32 base;\n"))
# S3: each step loop in its own block with its counter declared there
s3 = A.replace(blk, "")
s3 = s3.replace("        cur = base;\n        for (step = 1;", "        cur = base;\n        {\n        s32 step;\n        for (step = 1;", 1)
s3 = s3.replace("            *out = probe;\n            cur = *out;\n        }\n", "            *out = probe;\n            cur = *out;\n        }\n        }\n", 1)
s3 = s3.replace("        cur = *out;\n        for (rise = 1;", "        cur = *out;\n        {\n        s32 rise;\n        for (rise = 1;", 1)
s3 = s3.replace("            cur.y = probe.y;\n        }\n", "            cur.y = probe.y;\n        }\n        }\n", 1)
w("s_S3_loopblocks", s3)
# S4: selection loop with its own counter (i not reused for the second loop)
s4 = A.replace("    s32 sel;\n", "    s32 sel;\n    s32 k;\n")
s4 = s4.replace("for (i = 0, sel = 0; i < 4; i++) {", "for (k = 0, sel = 0; k < 4; k++) {")
s4 = s4.replace("stage[i * 6 + 3]", "stage[k * 6 + 3]").replace("stage[i * 6 + 5]", "stage[k * 6 + 5]").replace("sel = i;", "sel = k;")
w("s_S4_selcounter", s4)
# S5: the record pointer computed in each arm (no shared index after the if)
s5 = A.replace("        sel = D_800A38E0;\n", "        sel = D_800A38E0;\n        stage += sel * 6 + 3;\n")
s5 = s5.replace("        }\n    }\n    stage += sel * 6 + 3;\n", "        }\n        stage += sel * 6 + 3;\n    }\n")
w("s_S5_perarm", s5)
