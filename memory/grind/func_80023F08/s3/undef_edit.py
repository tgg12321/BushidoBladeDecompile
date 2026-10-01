"""undefined_syms_auto.txt edits for func_80023F08's two landing steps.
usage: python undef_edit.py undefined_syms_auto.txt A|B
A (data model): drop D_800A388C (func_80020D70 now stores D_800A3888[1]; no other referrer).
B (match):      drop D_80101EC8 (retire note: func_80023F08) and re-note D_80101ED2 (func_8003C714 is
                its last live asm referrer once func_80023F08 is C)."""
import sys

p, step = sys.argv[1:3]
s = open(p, encoding="utf-8").read()


def rep(old, new):
    global s
    assert s.count(old) == 1, old
    s = s.replace(old, new)


if step == "A":
    rep("D_800A388C = 0x800A388C;\n", "")
elif step == "B":
    rep("D_80101EC8 = 0x80101EC8; /* alias of g_practice_menu_table+0x0; retire with func_80023F08 */\n", "")
    rep("D_80101ED2 = 0x80101ED2;  /* alias of g_practice_menu_table+0xA (.unk_0A); retire with func_8001CE60 "
        "and func_80023F08 (their .s are its only live referrers) */\n",
        "D_80101ED2 = 0x80101ED2;  /* alias of g_practice_menu_table+0xA (.unk_0A); retire with func_8003C714 "
        "(its .s is the only live referrer) */\n")
else:
    raise SystemExit("step must be A or B")
open(p, "w", encoding="utf-8", newline="\n").write(s)
