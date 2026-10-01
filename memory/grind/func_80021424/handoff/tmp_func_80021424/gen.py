#!/usr/bin/env python3
"""Respell each consumer of the overlapping 0x801027B0 / 0x800A3860 handles through the one declaration."""
from pathlib import Path

d = Path(__file__).resolve().parent


def rw(func, pairs, tag="m"):
    s = (d / f"{func}.base.c").read_bytes().decode("utf-8")
    for a, b in pairs:
        assert a in s, (func, a)
        s = s.replace(a, b)
    (d / f"{func}.{tag}.c").write_bytes(s.encode())
    print(func, tag)


rw("func_80020D70", [("D_800A3864 = (s32)0x80190800;", "D_800A3860[1] = (Tbl800A3860Entry *)0x80190800;")])
rw("func_80021210", [("D_801027C0)", "D_801027B0[0][4])"), ("D_801027D4)", "D_801027B0[1][4])")])
rw("func_8003CF84", [("(u32 *)D_801027C0", "(u32 *)D_801027B0[0][4]"), ("(u32 *)D_801027D4", "(u32 *)D_801027B0[1][4]")])
for f, fld in (("func_80021904", "D_80101F4E"), ("func_80021974", "D_80101F4C")):
    s = (d / f"{f}.base.c").read_bytes().decode("utf-8")
    head = s.split("    s32 a0_2")[0]
    body = head + "    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;\n}\n"
    (d / f"{f}.m.c").write_bytes(body.encode())
    # keep the original statement order: pointer first, then the table word
    body2 = head + ("    Tbl800A3860Entry *base = D_800A3860[v1];\n    u16 idx = base->f4E[v0];\n"
                    "    s32 tbl = D_801027B0[v1][0];\n    return tbl + idx * 2;\n}\n")
    (d / f"{f}.m2.c").write_bytes(body2.encode())
    print(f, "m m2")
rw("func_80021A98", [("            s32 idx = a3 * 5;\n", ""), ("(&D_801027B4)[idx]", "D_801027B0[a3][1]"),
                     ("(&D_801027B8)[idx]", "D_801027B0[a3][2]")])
rw("func_80021A98", [("(&D_801027B4)[idx]", "D_801027B0[0][idx + 1]"),
                     ("(&D_801027B8)[idx]", "D_801027B0[0][idx + 2]")], "keepidx")
rw("func_80022F34", [("D_801027BC[idx1][0]", "D_801027B0[idx1][3]"), ("D_801027BC[idx2][0]", "D_801027B0[idx2][3]")])
