#!/usr/bin/env python3
"""mkty.py VARDIR -- type probes on the closest single-value splits: the split
local declared with other integer types (reads abl_temp_flag.c, abl_temp_mag3.c,
pv_nbits.c, abl_field_h0.c; writes ty_*.c)."""
import os, sys
D = sys.argv[1]


def rd(n):
    return open(os.path.join(D, n + ".c"), encoding="utf-8").read()


def sub(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


out = {}
f = rd("abl_temp_flag")
for t in ("s32", "u16", "u32", "s8", "u8"):
    out[f"ty_flag_{t}"] = sub(f, "                s16 flag;\n", f"                {t} flag;\n")
m = rd("abl_temp_mag3")
for t in ("s32", "u16"):
    out[f"ty_mag3_{t}"] = sub(m, "                    s16 mag3;\n", f"                    {t} mag3;\n")
n = rd("pv_nbits")
for t in ("u32", "s16"):
    out[f"ty_len_{t}"] = sub(n, "                    s32 len;\n", f"                    {t} len;\n")
h = rd("abl_field_h0")
for t in ("s32", "u16"):
    out[f"ty_h0_{t}"] = sub(h, "            u32 h0;\n", f"            {t} h0;\n")
for k, t in out.items():
    with open(os.path.join(D, k + ".c"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(t)
    print(k)
