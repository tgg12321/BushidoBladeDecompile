"""Retire the splat alias rows whose last live referrer was asm/funcs/func_80023F08.s
(D_80101EC8; and D_800A388C, whose only referrer becomes D_800A3888[1] in func_80020D70).
usage: python undef_edit.py undefined_syms_auto.txt"""
import sys

p = sys.argv[1]
s = open(p, encoding="utf-8").read()
rows = [
    "D_80101EC8 = 0x80101EC8; /* alias of g_practice_menu_table+0x0; retire with func_80023F08 */\n",
    "D_800A388C = 0x800A388C;\n",
]
for r in rows:
    assert s.count(r) == 1, r
    s = s.replace(r, "")
old = ("D_80101ED2 = 0x80101ED2;  /* alias of g_practice_menu_table+0xA (.unk_0A); retire with func_8001CE60 "
       "and func_80023F08 (their .s are its only live referrers) */\n")
new = ("D_80101ED2 = 0x80101ED2;  /* alias of g_practice_menu_table+0xA (.unk_0A); retire with func_8003C714 "
       "(its .s is the only live referrer) */\n")
assert s.count(old) == 1
s = s.replace(old, new)
open(p, "w", encoding="utf-8", newline="\n").write(s)
