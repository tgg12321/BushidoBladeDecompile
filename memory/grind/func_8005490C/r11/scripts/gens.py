"""gens.py: structural respellings of the one-variable-per-value spelling (and of each
variable in isolation, the other kept in its reuse form)."""
from pathlib import Path

import sys
V = Path(sys.argv[1])
OUT = Path(sys.argv[2])
OUT.mkdir(exist_ok=True)
split = (V / "P_split_Zsplit.c").read_text()
psplit_zsh = (V / "P_split_Zsh.c").read_text()      # player split, rot_z shared
pall_zsplit = (V / "P_all_Zsplit.c").read_text()    # player shared, rot_z split


def w(name, s):
    open(OUT / name, "w", newline="\n").write(s)


def rep(s, a, b):
    assert s.count(a) >= 1, a
    return s.replace(a, b)


INIT_SPLIT = """        player0 = func_8004153C(0);
        if (player0 != 0) {
            func_8003FFC4(player0);
        }
        player1 = func_8004153C(1);
        if (player1 != 0) {
            func_8003FFC4(player1);
        }
"""
INIT_LOOP = """        for (i = 0; i < 2; i++) {
            s32 *player0 = func_8004153C(i);
            if (player0 != 0) {
                func_8003FFC4(player0);
            }
        }
"""
INIT_COND = """        if ((player0 = func_8004153C(0)) != 0) {
            func_8003FFC4(player0);
        }
        if ((player1 = func_8004153C(1)) != 0) {
            func_8003FFC4(player1);
        }
"""
INIT_BLOCKS = """        {
            s32 *player0 = func_8004153C(0);
            if (player0 != 0) {
                func_8003FFC4(player0);
            }
        }
        {
            s32 *player1 = func_8004153C(1);
            if (player1 != 0) {
                func_8003FFC4(player1);
            }
        }
"""
DECL_INIT = "        s32 *player0;\n        s32 *player1;\n\n"
for base, tag in ((split, "split"), (psplit_zsh, "Psplit_Zsh")):
    w("S1_initloop_%s.c" % tag, rep(rep(base, INIT_SPLIT, INIT_LOOP), DECL_INIT, ""))
    w("S2_initcond_%s.c" % tag, rep(base, INIT_SPLIT, INIT_COND))
    w("S3_initblocks_%s.c" % tag, rep(rep(base, INIT_SPLIT, INIT_BLOCKS), DECL_INIT, ""))
    # function-scope declarations of the three player locals
    s = rep(base, DECL_INIT, "")
    s = rep(s, "            s32 *player2;\n", "")
    s = rep(s, "    s32 i;\n", "    s32 i;\n    s32 *player0;\n    s32 *player1;\n    s32 *player2;\n")
    w("S4_fnscope_players_%s.c" % tag, s)

# rot_z structural respellings, applied to both blocks
for base, tag in ((split, "split"), (pall_zsplit, "Pall_Zsplit")):
    s = base
    for k, ind in (("0", "        "), ("1", "                ")):
        old = (f"{ind}s32 rot_z{k};\n\n{ind}rot_z{k} = (z * c - x * sn) >> 12;\n"
               f"{ind}vec.vx = (z * sn + x * c) >> 12;\n{ind}vec.vz = rot_z{k};\n")
        assert s.count(old) == 1, old
    variants = {
        "R1_vx_temp": lambda k, ind: (f"{ind}s32 rot_x{k};\n\n{ind}rot_x{k} = (z * sn + x * c) >> 12;\n"
                                      f"{ind}vec.vz = (z * c - x * sn) >> 12;\n{ind}vec.vx = rot_x{k};\n"),
        "R2_no_xz_copies": None,
        "R3_both_temps": lambda k, ind: (f"{ind}s32 rot_z{k};\n{ind}s32 rot_x{k};\n\n"
                                         f"{ind}rot_z{k} = (z * c - x * sn) >> 12;\n"
                                         f"{ind}rot_x{k} = (z * sn + x * c) >> 12;\n"
                                         f"{ind}vec.vx = rot_x{k};\n{ind}vec.vz = rot_z{k};\n"),
        "R4_no_temp_vz_first": lambda k, ind: (f"{ind}vec.vz = (z * c - x * sn) >> 12;\n"
                                               f"{ind}vec.vx = (z * sn + x * c) >> 12;\n"),
    }
    for vn, fn in variants.items():
        s2 = base
        for k, ind in (("0", "        "), ("1", "                ")):
            old = (f"{ind}s32 rot_z{k};\n\n{ind}rot_z{k} = (z * c - x * sn) >> 12;\n"
                   f"{ind}vec.vx = (z * sn + x * c) >> 12;\n{ind}vec.vz = rot_z{k};\n")
            if fn is None:
                # no x/z copies: read vec directly, rot_z keeps the new z
                blk_old = (f"{ind}s32 x = vec.vx;\n{ind}s32 z = vec.vz;\n" + old)
                new = (f"{ind}s32 rot_z{k};\n\n{ind}rot_z{k} = (vec.vz * c - vec.vx * sn) >> 12;\n"
                       f"{ind}vec.vx = (vec.vz * sn + vec.vx * c) >> 12;\n{ind}vec.vz = rot_z{k};\n")
                s2 = rep(s2, blk_old, new)
            else:
                s2 = rep(s2, old, fn(k, ind))
        w(f"{vn}_{tag}.c", s2)
    # function-scope declaration of both (distinct) rot_z locals
    s2 = base
    for k, ind in (("0", "        "), ("1", "                ")):
        s2 = rep(s2, f"{ind}s32 rot_z{k};\n\n", "")
    s2 = rep(s2, "    s32 i;\n", "    s32 i;\n    s32 rot_z0;\n    s32 rot_z1;\n")
    w(f"R5_fnscope_rotz_{tag}.c", s2)
print(sorted(p.name for p in OUT.iterdir()))
