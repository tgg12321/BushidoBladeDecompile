# syms_b.py <undefined_syms_auto.txt> <named_syms.txt>: func_80055B60's symbol-file edits, in place.
# Retire the alias rows whose only assembled referrer is asm/funcs/func_80055B60.s; drop func_80055B60 from the
# retire notes of rows still serving other INCLUDE_ASM functions.
import sys

def rw(path, fn):
    s = open(path, encoding='utf-8', newline='').read()
    assert '\r' not in s
    t = fn(s)
    assert t != s, path
    open(path, 'w', encoding='utf-8', newline='\n').write(t)

def one(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)

def und(s):
    s = one(s, "D_80099D8D = 0x80099D8D;  /* alias of D_80099D88+0x5 (StatusFlagRec record 0); retire with func_80055B60 (asm/funcs/func_80055B60.s is its only assembled referrer) */\n", "")
    s = one(s, "D_80102790 = 0x80102790;  /* alias of D_80102788+0x8; retire with func_80055B60 (asm/funcs/func_80055B60.s is its only referrer) */\n", "")
    s = one(s, "retire with func_80055B60 and func_80058580 (asm/funcs/func_80055B60.s and asm/funcs/func_80058580.s are its assembled referrers) */",
            "retire with func_80058580 (asm/funcs/func_80058580.s is its only assembled referrer) */")
    s = one(s, "D_80101EC8 = 0x80101EC8; /* alias of g_practice_menu_table+0x0; retire with func_80023F08, func_80055B60 */",
            "D_80101EC8 = 0x80101EC8; /* alias of g_practice_menu_table+0x0; retire with func_80023F08 */")
    return s

def nam(s):
    s = one(s, "g_status_flag_record_table_80099D88_plus_5              = 0x80099D8D;  /* +5 from g_status_flag_record_table_80099D88 (0x80099D88) */\n", "")
    s = one(s, "g_practice_lesson_init_done_plus_4                      = 0x80102790;  /* +4 from g_practice_lesson_init_done (0x8010278C) */\n", "")
    return s

rw(sys.argv[1], und)
rw(sys.argv[2], nam)
print('syms ok')
