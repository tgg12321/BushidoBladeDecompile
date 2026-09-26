"""Apply the func_80036140 landing model to a tree (scratch tree, or '.' under the lock).
usage: python3 tmp/func_80036140/apply_model.py <tree> [--split none|A|final] [--merge] [--ext rec|split]
                                                      [--body FILE]
  --split none  : no TU split (func_80036140 stays INCLUDE_ASM)
  --split A     : only cdrom_SetMix + func_80035F78 move to the -G8 TU code6cac_b4.c; everything
                  after them moves unchanged to the -G0 TU code6cac_b4_post.c (func_80036140 stays
                  INCLUDE_ASM there, with its transcribed table)
  --split final : A, then func_80036140 + func_80036940 move to the -G8 TU code6cac_b5.c and
                  cdrom_IsIdle.. to the -G0 TU code6cac_b5_post.c; func_80036140 gets its C body and
                  the transcribed jtbl_80010938 is deleted (the compiler emits it)
  --merge       : the CdlATV respellings (header + cdrom_SetMix + func_80035F78 + CdMix prototype)
  --ext rec     : (with --merge) 0x80101E9C..0x80101EA7 join ReplayCamRec
  --ext split   : they stay separate externs
Does NOT apply the maspsx gate (gate.py does)."""
import sys, argparse
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument('tree'); ap.add_argument('--split', default='final'); ap.add_argument('--merge', action='store_true')
ap.add_argument('--ext', default='rec'); ap.add_argument('--body')
A = ap.parse_args()
R = Path(A.tree)
HERE = Path('tmp/func_80036140')


def rd(rel):
    return (R / rel).read_text(encoding='utf-8')


def wr(rel, s):
    (R / rel).write_bytes(s.encode('utf-8'))


def rep(s, old, new, count=1):
    assert s.count(old) == count, (old[:70], s.count(old))
    return s.replace(old, new)


S = 'src/code6cac_b2_post.c'
c = rd(S)
h = rd('include/code6cac.h')

# ------------------------------------------------------------------ respellings (the merges) -----
if A.merge:
    h = rep(h, 'extern u8 D_800A36B9;\nextern u8 D_800A36BA;\nextern u8 D_800A36BB;\n',
            (HERE / 'hdr_atv.txt').read_text())
    h = rep(h, 'extern u8 g_cd_atv_plus_0x1;\nextern u8 g_cd_atv_plus_0x2;\nextern u8 g_cd_atv_plus_0x3;\n',
            'extern CdlATV g_cd_atv;\n')
    c = rep(c, 'extern void CdMix(u8 *);\nextern u8 g_cd_atv;\n', 'extern void CdMix(CdlATV *);\n')
    c = rep(c, '    g_cd_atv = (u8)arg0;\n    g_cd_atv_plus_0x1 = (u8)arg1;\n    g_cd_atv_plus_0x2 = (u8)arg2;\n'
               '    g_cd_atv_plus_0x3 = (u8)arg3;\n',
            '    g_cd_atv.val0 = (u8)arg0;\n    g_cd_atv.val1 = (u8)arg1;\n    g_cd_atv.val2 = (u8)arg2;\n'
            '    g_cd_atv.val3 = (u8)arg3;\n')
    c = rep(c, 'extern u8 D_800A36B8;\nextern s16 D_800A3840;\n', 'extern s16 D_800A3840;\n')
    c = rep(c, '    D_800A36B8 = (u8)arg1;\n    D_800A36B9 = (u8)arg2;\n    D_800A36BA = (u8)arg3;\n',
            '    D_800A36B8.val0 = (u8)arg1;\n    D_800A36B8.val1 = (u8)arg2;\n    D_800A36B8.val2 = (u8)arg3;\n')
    c = rep(c, '    D_800A36BB = (u8)arg4;\n', '    D_800A36B8.val3 = (u8)arg4;\n')
if A.ext == 'rec':
    h = rep(h, '    s16 unk3A; /* 0x80101E9A */\n} ReplayCamRec;\n',
            '    s16 unk3A; /* 0x80101E9A */\n'
            '    s16 unk3C; /* 0x80101E9C */\n'
            '    u16 unk3E; /* 0x80101E9E */\n'
            '    s32 expected_pos; /* 0x80101EA0 */\n'
            '    s32 unk44; /* 0x80101EA4 */\n'
            '} ReplayCamRec;\n')
    h = rep(h, 'extern s16 D_80101E9C;\nextern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\nextern s32 D_80101EA4;\n', '')
    c = rep(c, 'g_cdread_expected_pos', 'D_80101E58.rec.expected_pos', count=4)
    c = rep(c, '    D_80101E9E = 0;\n', '    D_80101E58.rec.unk3E = 0;\n')
    c = rep(c, '    s0 = (u16 *)&D_80101E9E;\n', '    s0 = (u16 *)&D_80101E58.rec.unk3E;\n')
    h = rep(h, ' * record now runs on to 0x80101E9B (unk18..unk3A, see "Honest evidence split").\n',
            ' * record now runs on to 0x80101EA7 (unk18..unk44, see "Honest evidence split").\n')
    h = rep(h, ' *     member.  Member widths follow the accesses; 0x80101E91..93 is the\n'
               ' *     compiler\'s alignment padding.\n */\n',
            ' *     member.  Member widths follow the accesses; 0x80101E91..93 is the\n'
            ' *     compiler\'s alignment padding.\n'
            ' *   - The extension unk3C..unk44 (0x80101E9C..0x80101EA7, 2026-09-26) is\n'
            ' *     compiler-necessity evidence (aggregate-merge prong (a), (a1)/(a2)/(a4\')):\n'
            ' *     under -G8 (code6cac_b5.c) func_80036140\'s read-modify-writes of\n'
            ' *     0x80101E9C / 0x80101EA4 keep their address in a register (la; lX 0(r);\n'
            ' *     sX 0(r)) only for a variable larger than 8 bytes -- a small one is\n'
            ' *     small data and cse folds any pointer back to the symbol -- and\n'
            ' *     cdrom_ReadyCallback reads expected_pos directly only as an offset of\n'
            ' *     THIS symbol (cse relates it to its &unk38 register); as a separate\n'
            ' *     record both differ.  unk3E and expected_pos are the fields\n'
            ' *     cdrom_StartRead / game_FrameLoop and cdrom_ReadyCallback / func_80036940\n'
            ' *     use, typed by those accesses.  Dumps and the original compiler\'s runs:\n'
            ' *     memory/grind/func_80036140/evidence.md.\n */\n')
    h = rep(h, '/* The CD module\'s state block, 0x80101E58..0x80101E9B, declared as ONE object.\n',
            '/* The CD module\'s state block, 0x80101E58..0x80101EA7, declared as ONE object.\n')
    h = rep(h, ' *   - 0x80101E6C..0x80101E9B: see ReplayCamRec\'s evidence split above.\n',
            ' *   - 0x80101E6C..0x80101EA7: see ReplayCamRec\'s evidence split above.\n')
    h = rep(h, '    ReplayCamRec rec; /* 0x80101E60 .. 0x80101E9B */\n',
            '    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */\n')
if A.ext == 'sep':   # experiment: a separate 12-byte record at 0x80101E9C
    h = rep(h, 'extern s16 D_80101E9C;\nextern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\nextern s32 D_80101EA4;\n',
            'typedef struct { s16 unk0; u16 unk2; s32 expected_pos; s32 unk8; } CdReadTail;\n'
            'extern CdReadTail D_80101E9C;\n')
    c = rep(c, 'g_cdread_expected_pos', 'D_80101E9C.expected_pos', count=4)
    c = rep(c, '    D_80101E9E = 0;\n', '    D_80101E9C.unk2 = 0;\n')
    c = rep(c, '    s0 = (u16 *)&D_80101E9E;\n', '    s0 = (u16 *)&D_80101E9C.unk2;\n')
wr('include/code6cac.h', h)

if A.split == 'none':
    wr(S, c)
    sys.exit(0)

# ------------------------------------------------------------------ the splits -------------------
L = c.split('\n')
def idx(line):
    assert L.count(line) == 1, line
    return L.index(line)
i_mix = idx('void cdrom_SetMix(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {')
i_mix = i_mix - 2 - int(not A.merge)          # its file-scope externs: CdMix [, g_cd_atv], D_800A3854
i_snd = idx('void snd_SerialMixOn(void) {')
assert L[i_mix].startswith('extern void CdMix('), L[i_mix]
head, mix, rest = L[:i_mix], L[i_mix:i_snd], L[i_snd:]
wr(S, '\n'.join(head) + '\n')
wr('src/code6cac_b4.c', (HERE / 'b4_head.txt').read_text() + '\n'.join(mix) + '\n')
post = rest
if A.split == 'final':
    i_jt = post.index("/* func_80036140's jump table (func_80036140 is still INCLUDE_ASM, so the table is")
    i_inc = post.index('INCLUDE_ASM("asm/funcs", func_80036140);')
    i_idle = post.index('s32 cdrom_IsIdle(void) {')
    assert post[i_inc - 1] == '};' and i_inc - i_jt == 9, (i_inc, i_jt)
    b5 = post[i_inc:i_idle]
    b5_post = post[i_idle:]
    post = post[:i_jt]
    if A.body:
        body = Path(A.body).read_text()
    else:
        body = (HERE / 'body_tpl.c').read_text()
        if A.ext == 'rec':
            sub = {'@E9C_POSTINC@': 'D_80101E58.rec.unk3C++', '@E9C@': 'D_80101E58.rec.unk3C',
                   '@EA4_SUB4@': 'D_80101E58.rec.unk44 -= 4', '@EA4@': 'D_80101E58.rec.unk44'}
        elif A.ext == 'sep':
            sub = {'@E9C_POSTINC@': 'D_80101E9C.unk0++', '@E9C@': 'D_80101E9C.unk0',
                   '@EA4_SUB4@': 'D_80101E9C.unk8 -= 4', '@EA4@': 'D_80101E9C.unk8'}
        else:
            sub = {'@E9C_POSTINC@': 'D_80101E9C++', '@E9C@': 'D_80101E9C',
                   '@EA4_SUB4@': 'D_80101EA4 -= 4', '@EA4@': 'D_80101EA4'}
        for k, v in sub.items():
            body = body.replace(k, v)
    assert '@' not in body
    b5[0] = body.rstrip('\n')
    wr('src/code6cac_b5.c', (HERE / 'b5_head.txt').read_text() + '\n'.join(b5) + '\n')
    wr('src/code6cac_b5_post.c', (HERE / 'b5_post_head.txt').read_text() + '\n'.join(b5_post))
wr('src/code6cac_b4_post.c', (HERE / 'b4_post_head.txt').read_text() + '\n'.join(post) + ('\n' if A.split == 'final' else ''))

# ------------------------------------------------------------------ Makefile / mirror / ld -------
new_g8 = ['code6cac_b4'] + (['code6cac_b5'] if A.split == 'final' else [])
new_objs = ['code6cac_b4', 'code6cac_b4_post'] + (['code6cac_b5', 'code6cac_b5_post'] if A.split == 'final' else [])
m = rd('Makefile')
m = rep(m, 'GP_FILES := text1a_pre text1a_post code6cac_b3\n',
        'GP_FILES := text1a_pre text1a_post code6cac_b3 ' + ' '.join(new_g8) + '\n')
wr('Makefile', m)
b = rd('engine/buildconfig.py')
b = rep(b, 'GP_FILES = {"text1a_pre", "text1a_post", "code6cac_b3"}',
        'GP_FILES = {"text1a_pre", "text1a_post", "code6cac_b3", ' + ', '.join(f'"{x}"' for x in new_g8) + '}')
wr('engine/buildconfig.py', b)
ld = rd('bb2.ld')
for sec in ('rodata', 'text', 'data', 'bss'):
    ld = rep(ld, f'        build/src/code6cac_b2_post.o(.{sec});\n',
             f'        build/src/code6cac_b2_post.o(.{sec});\n'
             + ''.join(f'        build/src/{o}.o(.{sec});\n' for o in new_objs))
wr('bb2.ld', ld)
