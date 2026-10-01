#!/usr/bin/env python3
"""typed.py <in.c> <out.c>: structural respelling of a split twin: each value local gets the type of what
it holds (flags u8, angles s16, the script address u8 *), casts that only served the s32 carrier dropped."""
import sys, re
s = open(sys.argv[1]).read()
TYPES = {
    "n449_": "u8", "m445_": "u8", "n445_": "u8", "m449_": "u8", "side_": "u8", "n447_": "u8",
    "force_": "u8", "farflag_": "u8", "coin_": "u8", "ok_": "u8",
    "ang_": "s16", "dang_": "s16", "kind_": "u8", "kind2_": "u8", "pace_": "u8",
    "x1_": "s16", "y1_": "s16", "idx_": "u8", "bsel_": "s8", "slot_": "u8", "cnt2_": "u8",
    "kindet_": "s16", "mask_": "u32", "cmask_": "u32",
}
for name, t in TYPES.items():
    s = re.sub(r'(\n\s*)s32 %s;' % re.escape(name), r'\1%s %s;' % (t, name), s)
if "s32 script4_;" in s:
    s = s.replace("s32 script4_;", "u8 *script4_;")
    s = s.replace("script4_ = (s32)D_", "script4_ = D_")
    s = s.replace("func_80055B44(p, script4_, 2, 0);", "func_80055B44(p, (s32)script4_, 2, 0);")
    s = s.replace("script4_ = 0;", "script4_ = 0;")
s = s.replace("(u32)cmask_ &", "cmask_ &").replace("(u32)mask_ &", "mask_ &")
open(sys.argv[2], "w", newline="\n").write(s)
