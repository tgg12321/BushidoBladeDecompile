"""Stage 4: symbol-config + data-label side of the merges (prong (c)), written to integ/ copies.
Every removed row names a symbol no C code and no linked asm references any more
(tmp/func_80034708/symrefs.py); rows still referenced by an INCLUDE_ASM sibling stay,
suffixed per the 2026-09-03 amendment."""
import re, os
R = 'tmp/func_80034708/integ'

def rd(p):
    return open(p, encoding='utf-8').read()

def wr(p, s):
    os.makedirs(os.path.dirname(p) or '.', exist_ok=True)
    open(p, 'w', encoding='utf-8', newline='\n').write(s)

def drop_rows(text, names, path):
    out, dropped = [], []
    for line in text.split('\n'):
        m = re.match(r'^\s*(\w+)\s*=\s*0x[0-9A-Fa-f]+\s*;', line) or re.match(r'^(\w+)\s*$', line)
        if m and m.group(1) in names:
            dropped.append(m.group(1))
            continue
        out.append(line)
    missing = set(names) - set(dropped)
    assert not missing, (path, missing)
    return '\n'.join(out)

# undefined_syms_auto.txt
u = rd('undefined_syms_auto.txt')
u = drop_rows(u, ['D_8010277A', 'D_8010277D', 'D_8010277E', 'D_8010277F', 'D_80102780', 'D_80102781',
                  'D_80102782', 'D_80102783', 'D_80102784', 'D_80102785', 'D_80102786', 'D_80102787',
                  'D_80106A54', 'D_80106A58', 'D_80106A5C', 'D_80106A73', 'g_file_disc_size',
                  'g_file_flags', 'D_800A3176'], 'undefined_syms_auto.txt')
old = 'D_8010277C = 0x8010277C;\n'
assert old in u
u = u.replace(old, 'D_8010277C = 0x8010277C;  /* alias of D_80102778+0x4 (.unk_4[0]); retire with func_8003993C (asm/funcs/func_8003993C.s is its only live referrer) */\n')
old = 'D_80106A70 = 0x80106A70;\n'
assert old in u
u = u.replace(old, 'D_80106A70 = 0x80106A70;  /* alias of D_80106A50+0x20 (.color[0]); retire with func_8001BE20 (asm/funcs/func_8001BE20.s is its only live referrer) */\n')
wr(R + '/undefined_syms_auto.txt', u)

# named_syms.txt
n = rd('named_syms.txt')
n = drop_rows(n, ['g_practice_lesson_size_a', 'g_practice_lesson_size_b', 'g_practice_lesson_count_a',
                  'g_practice_lesson_count_b', 'g_practice_lesson_flag_a', 'g_practice_lesson_flag_b',
                  'g_practice_lesson_init_param_plus_1', 'g_practice_lesson_byte_6',
                  'g_file_disc_size', 'g_file_disc_type', 'g_file_flags', 'g_file_flags_byte',
                  'g_default_color_r', 'g_default_color_g', 'g_default_color_b',
                  'g_file_disc_type_plus_4', 'g_intro_demo_duration',
                  'g_game_flag_c_800A3176', 'g_motion_id_current_800A3174'], 'named_syms.txt')
old = '''/* parameters at 0x80102778-80102787.  0x8010277C-81 are three per-player */
/* byte pairs, declared in C as PlayerBytePairs D_8010277C (code6cac.h, */
/* merged 2026-09-25); their per-byte names are retired. */'''
assert old in n
n = n.replace(old, '''/* parameters at 0x80102778-80102787, declared in C as the one aggregate */
/* PracticeParams D_80102778 (code6cac.h, merged 2026-09-26 with */
/* func_80034708); its per-word names are retired. */''')
wr(R + '/named_syms.txt', n)

# symbol_addrs.txt (splat input)
s = rd('symbol_addrs.txt')
s = drop_rows(s, ['g_file_disc_size', 'g_file_disc_type', 'g_file_flags'], 'symbol_addrs.txt')
wr(R + '/symbol_addrs.txt', s)

# sdata_syms.txt
d = rd('sdata_syms.txt')
d = drop_rows(d, ['D_800A3176'], 'sdata_syms.txt')
wr(R + '/sdata_syms.txt', d)

# asm/data/91C98.data.s: one label for the s16[2] cursor pair
a = rd('asm/data/91C98.data.s')
old = '''dlabel D_800A3174
    /* 93974 800A3174 */ .short 0x0000
enddlabel D_800A3174

nonmatching D_800A3176

dlabel D_800A3176
    /* 93976 800A3176 */ .short 0x0000
enddlabel D_800A3176
'''
assert old in a
a = a.replace(old, '''dlabel D_800A3174
    /* 93974 800A3174 */ .short 0x0000
    /* 93976 800A3176 */ .short 0x0000
enddlabel D_800A3174
''')
wr(R + '/asm/data/91C98.data.s', a)
print('cfg ok')
