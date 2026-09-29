#!/usr/bin/env python3
"""Landing edits for func_8005E54C. Without --apply it only defines functions (for measurement tools).
--apply edits src/text1b.c, include/game.h, engine/volatile_cheats.py in place (LF)."""
import re, sys

BODY_SRC = 'memory/grind/func_8005E54C/match0/body.c'

GAME_H_ANCHOR = 'extern Unk8009B400Record D_8009B5F0[2][2];\n'
GAME_H_ADD = '''
/* 0x8009B490: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B490..0x8009B4AF. Object model evidence from the original binary:
 * asm/funcs/func_8005E54C.s forms ONE stride `sll $s0,$s0,4` (row counter * 16)
 * and adds it to both %lo(D_8009B490) (column 0) and %lo(D_8009B498) (column 1),
 * the same shape as D_8009B5F0 above. Data: all four records share the
 * {s16, s16, u8 x4} shape; D_8009B4B0 (a 12-byte sheet header) follows. Replaces
 * the splat per-word labels D_8009B490 / D_8009B498 in C (per-word splat symbol
 * -> aggregate merge family, owner ruling 2026-08-17). */
extern Unk8009B400Record D_8009B490[2][2];
'''

# replaces text1b.c's `extern s32 D_8009BD38;` (placed before every user in the file)
TEXT1B_DECLS = '''/* 0x8009BD24: two players x five rounds of 2-byte records; byte 0 is the
   character the round was fought with (func_8005E54C reads it at
   j * 10 + i * 2 and picks UesrWorkDef / D_8009B58C by it; func_80060414 reads
   player 0 round 0). 0x14 bytes, ending at the flag word below. */
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;
extern Unk8009BD24Record D_8009BD24[2][5];
/* 0x8009BD38: the match-settings flag word, bit fields named by bit offset.
   Every reader in this file extracts it by field: unk0 (`& 0xF`), unk10 (the
   round count - 3; also picks the results-screen layout), unk12 (`== 2`
   tests), unk14 (1 bit), unk15 (one bit per player); func_80077894 stores
   unk0. Byte 3 is not named here (text1b_b.c reads it as D_8009BD3B). */
typedef struct {
    u32 unk0 : 4;
    u32 unk4 : 6;
    u32 unk10 : 2;
    u32 unk12 : 2;
    u32 unk14 : 1;
    u32 unk15 : 2;
    u32 unk17 : 1;
    u32 unk18 : 6;
} Unk8009BD38Flags;
extern Unk8009BD38Flags D_8009BD38;
'''


def body():
    b = open(BODY_SRC, newline='\n').read()
    b = b.replace('extern Unk8009B400Record D_8009B490[2][2];\n', '')
    b = b.replace('    s16 vals[2];\n', '''    /* The per-player points pair: each round's points in the round rows,
       then the per-player totals under them. The target addresses both
       through the one frame slot sp+0x18 (a separate totals array measured
       13-204: memory/grind/func_8005E54C/evidence.md [s4 cont.]). */
    s16 points[2];
''')
    b = b.replace('*(s32 *)vals = 0;', '''/* One 32-bit store clears the whole pair (target 0x8005EA44
       `sw $zero,0x18($sp)`); the union spelling measured 197
       (memory/grind/func_8005E54C/evidence.md [s5]). Owner ruling
       2026-09-29 Q36 (no-new-park-categories.md, one cast store on a
       local array). */
    *(s32 *)points = 0;''')
    b = b.replace('    volatile s16 digit[3];\n', '''    /* FAKE: unused here. The frame keeps the 8 untouched bytes at
       sp+0x58 = descriptor + 0x30 where the COMPLETED siblings keep a real
       s16[3] digit array: func_8005D814 `s16 digit[3];` (src/text1b.c:4231,
       copied here) and func_8005F1C8 `s16 d[3];` (src/text1b.c:4911).
       Census and measurements: memory/grind/func_8005E54C/frame_census.txt,
       evidence.md [s4]/[s5]. Owner ruling 2026-09-29 Q35
       (no-new-park-categories.md, phantom-frame-slot pad family, trailing
       unused array with sibling evidence). */
    volatile s16 digit[3];
''')
    b = b.replace('    s16 i;\n    s16 j;\n    s16 k;\n', '''    /* i counts the players (first loop) and then the rounds; j is the
       player and k the mark; each phase restarts them as plain loop indices,
       the counter reuse of func_8005E098 / func_8005F1C8. Separate counters
       per phase measured 8-77 (memory/grind/func_8005E54C/evidence.md [s3]). */
    s16 i;
    s16 j;
    s16 k;
''')
    old = '''    if (D_8009BD38.unk10 == 2) {
        tile->x0 = 0x5E;'''
    assert b.count(old) == 1
    b = b.replace(old, '''    /* Each arm sets the whole (x0, y0) position: the target stores x0 once
       per arm (0x8005F0E0, 0x8005F0F8, 0x8005F104); one x0 store above the
       if/else measured 10 (memory/grind/func_8005E54C/probes/x0h.c). */
    if (D_8009BD38.unk10 == 2) {
        tile->x0 = 0x5E;''')
    b = re.sub(r'\bvals\b', 'points', b)
    assert 'vals' not in b
    return b


def patch_text1b(s, with_body=True):
    assert s.count('extern s32 D_8009BD38;\n') == 1 and s.count('extern u8 D_8009BD24[];\n') == 1
    s = s.replace('extern s32 D_8009BD38;\n', TEXT1B_DECLS)
    s = s.replace('extern u8 D_8009BD24[];\n', '')
    s = s.replace('D_8009BD24[0] < 0xC', 'D_8009BD24[0][0].chr < 0xC')
    s = s.replace('func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8)',
                  'func_8006E534(a0, D_800A35E0, &D_8009BD24[0][0].chr, D_800A35E8)')
    s = s.replace('(((u32)D_8009BD38 >> 14) & 1) + 1', 'D_8009BD38.unk14 + 1')
    s = s.replace('(D_8009BD38 & 0x3000) == 0x2000', 'D_8009BD38.unk12 == 2')
    s = s.replace('v = D_8009BD38 & 0xF;', 'v = D_8009BD38.unk0;')
    s = s.replace('(D_8009BD38 & 0xF) * 2', 'D_8009BD38.unk0 * 2')
    old = '''        s32 *p = &D_8009BD38;
        s32 cur;
        ret = 1;
        cur = *p;
        D_800A35E4 = 0;
        cur &= ~0xF;
        cur |= result & 0xF;
        *p = cur;'''
    assert old in s
    s = s.replace(old, '''        ret = 1;
        D_800A35E4 = 0;
        D_8009BD38.unk0 = result;''')
    if with_body:
        key = 'INCLUDE_ASM("asm/funcs", func_8005E54C);'
        assert s.count(key) == 1
        s = s.replace(key, body().rstrip('\n'))
    left = re.findall(r'D_8009BD38(?!\.)', s)
    assert left == ['D_8009BD38'], left   # the extern only
    return s


def patch_game(s):
    assert s.count(GAME_H_ANCHOR) == 1
    return s.replace(GAME_H_ANCHOR, GAME_H_ANCHOR + GAME_H_ADD)


def patch_vc(s):
    anchor = '    "func_800480C0": frozenset({("pre_pad", 8)}),\n'
    assert s.count(anchor) == 1
    return s.replace(anchor, anchor + '''    # 2026-09-29 owner ruling Q35 (no-new-park-categories.md, phantom-frame-slot
    # pad family, trailing unused array with sibling evidence): func_8005E54C's
    # 8 untouched bytes at sp+0x58 = descriptor + 0x30, the s16[3] digit slot of
    # its COMPLETED siblings func_8005D814 / func_8005F1C8
    # (memory/grind/func_8005E54C/frame_census.txt, evidence.md [s4]/[s5]).
    "func_8005E54C": frozenset({("digit", 3)}),
''')


def patch(s):  # measurement: text1b.c edits + the game.h block inlined after the includes
    s = patch_text1b(s)
    inc = '#include "code6cac.h"\n'
    return s.replace(inc, inc + GAME_H_ADD, 1)


if __name__ == '__main__' and '--apply' in sys.argv:
    for path, fn in (('src/text1b.c', patch_text1b), ('include/game.h', patch_game),
                     ('engine/volatile_cheats.py', patch_vc)):
        t = open(path, newline='\n').read()
        assert '\r' not in t, path
        open(path, 'w', newline='\n').write(fn(t))
        print('patched', path)
