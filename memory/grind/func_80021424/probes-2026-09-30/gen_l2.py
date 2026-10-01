#!/usr/bin/env python3
"""gen_l2.py : L2 bodies (r1) from the HEAD versions tmp/prc/h_<func>.c — every practice-table handle respelled as a
g_practice_menu_table[k] member, keeping each read's declared type (value casts where the old extern's type
differs from the member's)."""
import re
from pathlib import Path

d = Path(__file__).resolve().parent
T = "g_practice_menu_table"
SIMPLE = [  # (pattern, replacement) applied in order
    (r"\(u16\)D_80101F32\b", f"{T}[0].unk_6A"),
    (r"\(u16\)D_8010237E\b", f"{T}[1].unk_6A"),
    (r"\bD_80101F32\b", f"(s16){T}[0].unk_6A"),   # TU-local extern s16 D_80101F32
    (r"\bD_8010237E\b", f"{T}[1].unk_6A"),        # extern u16
    (r"\bD_80101F5E\b", f"{T}[0].unk_96"),
    (r"\bD_801023AA\b", f"{T}[1].unk_96"),
    (r"\bD_80101F79\b", f"{T}[0].unk_B1"),
    (r"\bD_801023C5\b", f"{T}[1].unk_B1"),
    (r"\bD_80101F7A\b", f"{T}[0].unk_B2"),
    (r"\bD_8010231A\b", f"{T}[1].unk_06"),
]


def sub(s, pat, rep, n=None):
    s2, k = re.subn(pat, rep, s)
    if n is not None:
        assert k == n, (pat, k, n)
    return s2


def common(s):
    for p, r in SIMPLE:
        s = sub(s, p, r)
    return s


out = {}
for f in ("func_8001CE60", "func_8001E404", "func_8001C8DC"):
    out[f] = common((d / f"h_{f}.c").read_text())
out["func_8001C8DC"] = sub(out["func_8001C8DC"],
                           r"\(s32 \*\)\(\(u8 \*\)&D_80101FBC \+ \(s32\)D_800A3748 \* 0x44C\)",
                           f"&{T}[D_800A3748].unk_F4.x", 1)

s = (d / "h_func_8001EFA0.c").read_text()
s = sub(s, r"\(u8 \*\)&D_80101EC8 \+ D_800A3748 \* 1100", f"(u8 *)&{T}[D_800A3748]", 1)
s = sub(s, r"\*\(&D_80101F5E \+ D_800A3748 \* 550\)", f"{T}[D_800A3748].unk_96", 1)
out["func_8001EFA0"] = s

s = (d / "h_func_8001E878.c").read_text()
s = s.replace("    s32 *a0 = &D_80102030;\n    u8 *s0;\n", "")
s = s.replace("    s0 = (u8 *)a0 - 0x168;\n", "")
s = sub(s, r"func_8001A820\(\(s32\)a0, \(s32\)\(\(u8 \*\)a0 \+ 0x44C\), \(s32\)s0, \(s32\)\(\(u8 \*\)a0 \+ 0x2E4\)\);",
        f"func_8001A820((s32)&{T}[0].unk_168, (s32)&{T}[1].unk_168, (s32)&{T}[0], (s32)&{T}[1]);", 1)
s = sub(s, r"func_8001B478\(\(s32\)\(s0 \+ D_800A36F6 \* 1100\)\);", f"func_8001B478((s32)&{T}[D_800A36F6]);", 1)
s = sub(s, r"func_8003E6A0\(D_80101FBC, D_80101FC4\);", f"func_8003E6A0({T}[0].unk_F4.x, {T}[0].unk_F4.z);", 1)
s = sub(s, r"func_8003E6A0\(D_80102408, D_80102410\);", f"func_8003E6A0({T}[1].unk_F4.x, {T}[1].unk_F4.z);", 1)
out["func_8001E878"] = s

s = (d / "h_func_8001EA84.c").read_text()
s = s.replace("    u8 *base;\n", "    PracticeMenuRec *base;\n")
s = sub(s, r"    base = \(u8 \*\)&D_80101EC8;\n    if \(D_800A3748 == 0\) \{\n        base \+= 0x44C;\n    \}\n",
        f"    base = &{T}[0];\n    if (D_800A3748 == 0) {{\n        base++;\n    }}\n", 1)
s = sub(s, r"func_8001BC70\(base, ", "func_8001BC70((u8 *)base, ", 1)
s = sub(s, r"\(&D_80101F7B\)\[ret = \(D_800A3748 == 0\) \* 0x44C\] = 0;", f"{T}[D_800A3748 == 0].unk_B3 = 0;", 1)
s = sub(s, r"\(s32 \*\)\(\(u8 \*\)&D_80101FBC \+ \(s32\)D_800A3748 \* 0x44C\)", f"&{T}[D_800A3748].unk_F4.x", 1)
s = sub(s, r"\bD_8010231A\b", f"{T}[1].unk_06", 1)
out["func_8001EA84"] = s

s = (d / "h_func_8001FB34.c").read_text()
s = s.replace("s32 func_8001FB34(s32 *arg0, s32 arg1) {", "s32 func_8001FB34(PracticeMenuRec *arg0, s32 arg1) {")
s = s.replace("    s32 v0;\n", "    s32 v0;\n", 1)
s = sub(s, r"    v0 = \*\(s32 \*\)arg0;\n    v1 = \*\(s16 \*\)\(v0 \+ 0xC\);\n", "    v1 = arg0->unk_00->unk_0C;\n", 1)
s = sub(s, r"\*\(s16 \*\)\(\(u8 \*\)arg0 \+ 0xA\)", "arg0->unk_0A", 1)
s = sub(s, r"\*\(s16 \*\)\(\(u8 \*\)arg0 \+ 0x330\)", "arg0->unk_330", 1)
s = sub(s, r"\*\(s16 \*\)\(\(u8 \*\)arg0 \+ 0x332\)", "arg0->unk_332", 1)
s = sub(s, r"\*\(s16 \*\)\(\(u8 \*\)arg0 \+ 0x26C\)", "arg0->unk_26C", 1)
out["func_8001FB34"] = s

for f, s in out.items():
    left = re.findall(r"\bD_801(?:01[E-F][0-9A-F]{2}|02[0-6][0-9A-F]{2})\b", s)
    (d / f"{f}.l2r1.c").write_bytes(s.encode())
    print(f, "remaining handles:", left)
