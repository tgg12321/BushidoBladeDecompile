#!/usr/bin/env python3
"""mkdecl.py VARDIR -- declaration-position probes for the closest single-value
ablations (case-2 flag 3, case-3 magnitude 6, a header word 6, the suffix
length 7): the same split, the new local declared at other legal scopes/orders.
Reads VARDIR/abl_temp_flag.c, abl_temp_mag3.c, abl_field_h0.c, abl_field_h3.c,
pv_nbits.c and writes dp_*.c next to them."""
import os, sys
D = sys.argv[1]


def rd(n):
    return open(os.path.join(D, n + ".c"), encoding="utf-8").read()


def sub(s, old, new):
    assert s.count(old) == 1, (old[:80], s.count(old))
    return s.replace(old, new)


out = {}
f = rd("abl_temp_flag")
FLAG_DECL = "            case 2: {\n                s16 flag;\n\n"
assert FLAG_DECL in f
base = f.replace(FLAG_DECL, "            case 2: {\n")
out["dp_flag_chbody_before_temp"] = sub(base, "            s16 temp;\n", "            s16 flag;\n            s16 temp;\n")
out["dp_flag_chbody_after_temp"] = sub(base, "            s16 temp;\n", "            s16 temp;\n            s16 flag;\n")
out["dp_flag_fnscope"] = sub(base, "    s16 x;\n", "    s16 x;\n    s16 flag;\n")
out["dp_flag_fnscope_first"] = sub(base, "    u8 *rec;\n", "    s16 flag;\n    u8 *rec;\n")

m = rd("abl_temp_mag3")
MAG3_DECL = "                    s32 nbits2;\n                    s16 mag3;\n"
assert MAG3_DECL in m
mb = m.replace(MAG3_DECL, "                    s32 nbits2;\n")
out["dp_mag3_chbody"] = sub(mb, "            s16 temp;\n", "            s16 temp;\n            s16 mag3;\n")
out["dp_mag3_fnscope"] = sub(mb, "    s16 x;\n", "    s16 x;\n    s16 mag3;\n")
out["dp_mag3_before_nbits2"] = sub(mb, "                    s32 nbits2;\n", "                    s16 mag3;\n                    s32 nbits2;\n")

h = rd("abl_field_h0")
H0_DECL = "        } else {\n            u32 h0;\n"
assert H0_DECL in h
hb = h.replace(H0_DECL, "        } else {\n")
out["dp_h0_fnscope"] = sub(hb, "    u32 field;\n", "    u32 field;\n    u32 h0;\n")
out["dp_h0_fnscope_first"] = sub(hb, "    u8 *rec;\n", "    u32 h0;\n    u8 *rec;\n")

h3 = rd("abl_field_h3")
H3_DECL = "    for (; idx2 < sub; idx2++) {\n        u32 h3;\n\n"
assert H3_DECL in h3
h3b = h3.replace(H3_DECL, "    for (; idx2 < sub; idx2++) {\n")
out["dp_h3_fnscope"] = sub(h3b, "    u32 field;\n", "    u32 field;\n    u32 h3;\n")

n = rd("pv_nbits")
LEN_DECL = "                    s32 len;\n\n"
assert LEN_DECL in n
nb = n.replace(LEN_DECL, "")
out["dp_len_caseblock"] = sub(nb, "                s32 zeros;\n", "                s32 zeros;\n                s32 len;\n")
out["dp_len_before_zeros"] = sub(nb, "                s32 zeros;\n", "                s32 len;\n                s32 zeros;\n")
for k, t in out.items():
    with open(os.path.join(D, k + ".c"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(t)
    print(os.path.join(D, k + ".c"))
