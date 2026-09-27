"""Build tmp/func_8001CE60/sel_tu.c: the landing code6cac.c with the score/tiebreak bytes declared as SCALARS (the other
file's single declaration) and func_8001CE60 respelled with ordinary conditional selects instead of indexing."""
from pathlib import Path

src = Path("src/code6cac.c").read_text(encoding="utf-8")
i = src.index("/* P1/P2 round scores and tiebreakers (Q21 per-file declarations")
j = src.index("extern u8 D_800A38AA[2];\n") + len("extern u8 D_800A38AA[2];\n")
src = src[:i] + "extern u8 D_800A3898;\nextern u8 D_800A3899;\nextern u8 D_800A38AA;\nextern u8 D_800A38AB;\n" + src[j:]
reps = [
    ("D_800A38AA[D_800A38B0]++;", "if (D_800A38B0 != 0) {\n                        D_800A38AB++;\n                    } else {\n                        D_800A38AA++;\n                    }"),
    ("if (D_800A38AA[D_800A38B0] == 2) {", "if ((D_800A38B0 != 0 ? D_800A38AB : D_800A38AA) == 2) {"),
    ("D_800A3898[D_800A38B0 == 0]++;", "if (D_800A38B0 == 0) {\n                        D_800A3899++;\n                    } else {\n                        D_800A3898++;\n                    }"),
    ("D_800A38AA[D_800A38B0] = 0;", "if (D_800A38B0 != 0) {\n                        D_800A38AB = 0;\n                    } else {\n                        D_800A38AA = 0;\n                    }"),
    ("if (D_800A3898[D_800A38B0 == 0] == D_800A37F8) {", "if ((D_800A38B0 == 0 ? D_800A3899 : D_800A3898) == D_800A37F8) {"),
    ("D_800A38B0 = 1;\n            D_800A3920 = 1;\n            D_800A3898[D_800A38B0 ^ 1]++;", "D_800A38B0 = 1;\n            D_800A3920 = 1;\n            D_800A3898++;"),
    ("D_800A38B0 = 0;\n            D_800A3920 = 1;\n            D_800A3898[D_800A38B0 ^ 1]++;", "D_800A38B0 = 0;\n            D_800A3920 = 1;\n            D_800A3899++;"),
    ("D_800A3898[0] | (D_800A38AA[0] << 8) | (D_800A3898[1] << 4) | (D_800A38AA[1] << 12)", "D_800A3898 | (D_800A38AA << 8) | (D_800A3899 << 4) | (D_800A38AB << 12)"),
]
fs = src.index("void func_8001CE60(void) {")
fe = src.index("\n}\n", fs) + 3
body = src[fs:fe]
for o, n in reps:
    assert body.count(o) == 1, o
    body = body.replace(o, n)
assert "D_800A3898[" not in body and "D_800A38AA[" not in body
src = src[:fs] + body + src[fe:]
Path("tmp/func_8001CE60/sel_tu.c").write_bytes(src.encode("utf-8"))
Path("tmp/func_8001CE60/sel_body.c").write_bytes(body.encode("utf-8"))
print("ok")
