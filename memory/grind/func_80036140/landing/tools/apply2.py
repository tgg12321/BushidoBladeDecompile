"""func_80036140 landing, restructured into the owner-Q16 commit sequence. Each step assumes the previous
ones were applied (gate.py + register.py = commit A come first).
usage: python3 tmp/func_80036140/apply2.py <tree> split|merge|match
  split (commit B): cdrom_SetMix + func_80035F78 move verbatim into the -G8 TU code6cac_b4.c; everything
                    after them moves verbatim into the -G0 TU code6cac_b4_post.c. No respelling.
  merge (commit C): CdlATV (D_800A36B8, g_cd_atv), CdState through
                    0x80101EA7; consumers respelled; data dlabels merged; per-word rows the still-INCLUDE_ASM
                    func_80036140 needs stay as "retire with func_80036140" aliases, the rest retire.
  match (commit D): func_80036140 + func_80036940 move verbatim from code6cac_b4_post.c into the -G8 TU
                    code6cac_b5.c, cdrom_IsIdle.. into the -G0 TU code6cac_b5_post.c; func_80036140's C
                    body replaces its INCLUDE_ASM; the transcribed jtbl_80010938 goes; alias rows retire."""
import re, sys
from pathlib import Path

R, STEP = Path(sys.argv[1]), sys.argv[2]
HERE = Path('tmp/func_80036140')


def rd(rel):
    return (R / rel).read_text(encoding='utf-8')


def wr(rel, s):
    (R / rel).write_bytes(s.encode('utf-8'))


def rep(s, old, new, count=1):
    assert s.count(old) == count, (old[:70], s.count(old))
    return s.replace(old, new)


INCLUDES_G0 = ('#define INCLUDE_ASM_USE_MACRO_INC 1\n#include "common.h"\n#include "include_asm.h"\n'
               '#include "gpu.h"\n#include "sound.h"\n#include "game.h"\n#include "system.h"\n#include "code6cac.h"\n')
INCLUDES_G8 = ('#include "common.h"\n#include "gpu.h"\n#include "sound.h"\n#include "game.h"\n'
               '#include "system.h"\n#include "code6cac.h"\n')
EXTERN_FN = re.compile(r'^extern\b[^;(]*?\b(\w+)\s*\(.*\);\s*$')
EXTERN_DATA = re.compile(r'^extern\b[^;(]*?\b(\w+)\s*(\[[^\]]*\])?\s*;\s*$')
DEFN = re.compile(r'^([A-Za-z_][\w \*]*?[\s\*])(\w+)\(([^)]*)\) \{$')
IDENT = re.compile(r'\b[A-Za-z_]\w*\b')


def decls(prefix, moved):
    """File-scope declarations of the source file (prefix = its lines before the cut) that the moved text
    names: every such `extern` line verbatim, in source order, plus a prototype for every function the
    prefix DEFINES that the moved text calls or names."""
    used = set(IDENT.findall('\n'.join(moved)))
    own = set()
    for l in moved:
        for rx in (EXTERN_FN, EXTERN_DATA, DEFN):
            m = rx.match(l)
            if m:
                own.add(m.group(2) if rx is DEFN else m.group(1))
    out, seen = [], set()
    for l in prefix:
        m = EXTERN_FN.match(l) or EXTERN_DATA.match(l)
        if m and m.group(1) in used and m.group(1) not in own and l not in seen:
            out.append(l); seen.add(l)
    for l in prefix:
        m = DEFN.match(l)
        if m and m.group(2) in used and m.group(2) not in own:
            p = f'extern {m.group(1)}{m.group(2)}({m.group(3)});'
            if p not in seen:
                out.append(p); seen.add(p)
    return out


def add_objs(after, objs, g8):
    m = rd('Makefile')
    gp = re.search(r'^GP_FILES := (.*)$', m, re.M)
    if g8:
        m = m.replace(gp.group(0), gp.group(0) + ' ' + g8)
    wr('Makefile', m)
    b = rd('engine/buildconfig.py')
    gp = re.search(r'^GP_FILES = \{(.*)\}$', b, re.M)
    if g8:
        b = b.replace(gp.group(0), 'GP_FILES = {' + gp.group(1) + f', "{g8}"' + '}')
    wr('engine/buildconfig.py', b)
    ld = rd('bb2.ld')
    for sec in ('rodata', 'text', 'data', 'bss'):
        ld = rep(ld, f'        build/src/{after}.o(.{sec});\n',
                 f'        build/src/{after}.o(.{sec});\n' + ''.join(f'        build/src/{o}.o(.{sec});\n' for o in objs))
    wr('bb2.ld', ld)


def text(head_comment, includes, dl, body_lines):
    return head_comment + includes + ('\n' + '\n'.join(dl) + '\n' if dl else '') + '\n' + '\n'.join(body_lines)


# ================================================================================ split (commit B)
if STEP == 'split':
    S = 'src/code6cac_b2_post.c'
    L = rd(S).split('\n')
    i_mix = L.index('extern void CdMix(u8 *);')
    i_snd = L.index('void snd_SerialMixOn(void) {')
    head, mix, rest = L[:i_mix], L[i_mix:i_snd], L[i_snd:]
    wr(S, '\n'.join(head) + '\n')
    wr('src/code6cac_b4.c', text((HERE / 'b4_head.txt').read_text(), INCLUDES_G8, decls(head, mix), mix) + '\n')
    wr('src/code6cac_b4_post.c', text((HERE / 'b4_post_head2.txt').read_text(), INCLUDES_G0, decls(L[:i_snd], rest), rest))
    add_objs('code6cac_b2_post', ['code6cac_b4', 'code6cac_b4_post'], 'code6cac_b4')

# ================================================================================ merge (commit C)
elif STEP == 'merge':
    h = rd('include/code6cac.h')
    h = rep(h, 'extern u8 D_800A36B9;\nextern u8 D_800A36BA;\nextern u8 D_800A36BB;\n', (HERE / 'hdr_atv.txt').read_text())
    h = rep(h, 'extern u8 g_cd_atv_plus_0x1;\nextern u8 g_cd_atv_plus_0x2;\nextern u8 g_cd_atv_plus_0x3;\n',
            'extern CdlATV g_cd_atv;\n')
    h = rep(h, '    s16 unk3A; /* 0x80101E9A */\n} ReplayCamRec;\n',
            '    s16 unk3A; /* 0x80101E9A */\n    s16 unk3C; /* 0x80101E9C */\n    u16 unk3E; /* 0x80101E9E */\n'
            '    s32 expected_pos; /* 0x80101EA0 */\n    s32 unk44; /* 0x80101EA4 */\n} ReplayCamRec;\n')
    h = rep(h, 'extern s16 D_80101E9C;\nextern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\nextern s32 D_80101EA4;\n', '')
    h = rep(h, ' * record now runs on to 0x80101E9B (unk18..unk3A, see "Honest evidence split").\n',
            ' * record now runs on to 0x80101EA7 (unk18..unk44, see "Honest evidence split").\n')
    h = rep(h, ' *     member.  Member widths follow the accesses; 0x80101E91..93 is the\n'
               ' *     compiler\'s alignment padding.\n */\n',
            ' *     member.  Member widths follow the accesses; 0x80101E91..93 is the\n'
            ' *     compiler\'s alignment padding.\n' + (HERE / 'hdr_rec_ext.txt').read_text() + ' */\n')
    h = rep(h, '/* The CD module\'s state block, 0x80101E58..0x80101E9B, declared as ONE object.\n',
            '/* The CD module\'s state block, 0x80101E58..0x80101EA7, declared as ONE object.\n')
    h = rep(h, ' *   - 0x80101E6C..0x80101E9B: see ReplayCamRec\'s evidence split above.\n',
            ' *   - 0x80101E6C..0x80101EA7: see ReplayCamRec\'s evidence split above.\n')
    h = rep(h, '    ReplayCamRec rec; /* 0x80101E60 .. 0x80101E9B */\n', '    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */\n')
    wr('include/code6cac.h', h)
    c = rd('src/code6cac_b4.c')
    c = rep(c, 'extern void CdMix(u8 *);\nextern u8 g_cd_atv;\n', 'extern void CdMix(CdlATV *);\n')
    c = rep(c, '    g_cd_atv = (u8)arg0;\n    g_cd_atv_plus_0x1 = (u8)arg1;\n    g_cd_atv_plus_0x2 = (u8)arg2;\n'
               '    g_cd_atv_plus_0x3 = (u8)arg3;\n',
            '    g_cd_atv.val0 = (u8)arg0;\n    g_cd_atv.val1 = (u8)arg1;\n    g_cd_atv.val2 = (u8)arg2;\n'
            '    g_cd_atv.val3 = (u8)arg3;\n')
    c = rep(c, 'extern u8 D_800A36B8;\nextern s16 D_800A3840;\n', 'extern s16 D_800A3840;\n')
    c = rep(c, '    D_800A36B8 = (u8)arg1;\n    D_800A36B9 = (u8)arg2;\n    D_800A36BA = (u8)arg3;\n',
            '    D_800A36B8.val0 = (u8)arg1;\n    D_800A36B8.val1 = (u8)arg2;\n    D_800A36B8.val2 = (u8)arg3;\n')
    c = rep(c, '    D_800A36BB = (u8)arg4;\n', '    D_800A36B8.val3 = (u8)arg4;\n')
    wr('src/code6cac_b4.c', c)
    p = rd('src/code6cac_b4_post.c')
    p = rep(p, 'g_cdread_expected_pos', 'D_80101E58.rec.expected_pos', count=4)
    p = rep(p, '    D_80101E9E = 0;\n', '    D_80101E58.rec.unk3E = 0;\n')
    p = rep(p, '    s0 = (u16 *)&D_80101E9E;\n', '    s0 = &D_80101E58.rec.unk3E;\n')
    wr('src/code6cac_b4_post.c', p)
    # --- symbol rows: keep (as aliases) only what the still-INCLUDE_ASM func_80036140.s names
    s140 = Path('asm/funcs/func_80036140.s').read_text()
    ALIAS = {'D_800A36B9': 'D_800A36B8+1', 'D_800A36BA': 'D_800A36B8+2', 'D_800A36BB': 'D_800A36B8+3',
             'g_cd_atv_plus_0x1': 'g_cd_atv+1', 'g_cd_atv_plus_0x2': 'g_cd_atv+2', 'g_cd_atv_plus_0x3': 'g_cd_atv+3',
             'D_80101E9C': 'D_80101E58+0x44', 'D_80101EA4': 'D_80101E58+0x4C'}
    for n in ALIAS:
        assert re.search(r'\b' + n + r'\b', s140), n
    RETIRE_NOW = {'D_80101E9E', 'g_cdread_expected_pos', 'g_c6cb2_state_E9E', 'g_cdread_state_9C_80101E9C',
                  'g_special_cam_win_byte_a_plus_1', 'g_special_cam_win_byte_a_plus_2', 'g_special_cam_win_byte_a_plus_3'}
    for n in RETIRE_NOW:
        assert not re.search(r'\b' + n + r'\b', s140), n
    row = re.compile(r'^\s*([A-Za-z_]\w*)\s*=\s*0x[0-9A-Fa-f]+\s*;')
    for rel in ('undefined_syms_auto.txt', 'named_syms.txt'):
        out = []
        for line in rd(rel).split('\n'):
            m = row.match(line)
            n = m.group(1) if m else None
            if n in RETIRE_NOW:
                continue
            if n in ALIAS:
                if rel == 'named_syms.txt':
                    continue                     # one row per alias, kept in undefined_syms_auto.txt
                line = line + f'  /* alias of {ALIAS[n]}; retire with func_80036140 (asm/funcs/func_80036140.s is its only live referrer) */'
            out.append(line)
        s = '\n'.join(out)
        if rel == 'named_syms.txt':
            s = rep(s, '/* 2026-09-26: 0x80101E58..0x80101E9B is ONE object, CdState D_80101E58 */',
                    '/* 2026-09-26: 0x80101E58..0x80101EA7 is ONE object, CdState D_80101E58 */')
        wr(rel, s)
    d = rd('asm/data/91C98.data.s')
    for base, subs in (('D_800A36B8', ('D_800A36B9', 'D_800A36BA', 'D_800A36BB')),
                       ('g_cd_atv', ('g_cd_atv_plus_0x1', 'g_cd_atv_plus_0x2', 'g_cd_atv_plus_0x3'))):
        prev = base
        for sub in subs:
            d = rep(d, f'enddlabel {prev}\n\nnonmatching {sub}\n\ndlabel {sub}\n', '')
            prev = sub
        d = rep(d, f'enddlabel {subs[-1]}\n', f'enddlabel {base}\n')
    wr('asm/data/91C98.data.s', d)

# ================================================================================ match (commit D)
elif STEP == 'match':
    S = 'src/code6cac_b4_post.c'
    L = rd(S).split('\n')
    i_jt = L.index("/* func_80036140's jump table (func_80036140 is still INCLUDE_ASM, so the table is")
    i_inc = L.index('INCLUDE_ASM("asm/funcs", func_80036140);')
    i_idle = L.index('s32 cdrom_IsIdle(void) {')
    assert L[i_inc - 1] == '};' and i_inc - i_jt == 9
    keep, b5, b5p = L[:i_jt], L[i_inc + 1:i_idle], L[i_idle:]
    body = (HERE / 'body_d.c').read_text().rstrip('\n').split('\n')
    wr(S, '\n'.join(keep) + '\n')
    wr('src/code6cac_b5.c', text((HERE / 'b5_head.txt').read_text(), INCLUDES_G8, decls(L[:i_inc], body + b5),
                                 body + b5) + '\n')
    wr('src/code6cac_b5_post.c', text((HERE / 'b5_post_head.txt').read_text(), INCLUDES_G0, decls(L[:i_idle], b5p), b5p))
    add_objs('code6cac_b4_post', ['code6cac_b5', 'code6cac_b5_post'], 'code6cac_b5')
    for rel in ('undefined_syms_auto.txt', 'named_syms.txt'):
        wr(rel, '\n'.join(l for l in rd(rel).split('\n') if 'retire with func_80036140' not in l))
    g = rd('.claude/rules/maspsx-gate-lists.md')
    old = 'prefill-label (fidelity): main — first and only entry (owner ruling 2026-09-04).\n'
    g = rep(g, old, old + 'comm (fidelity): cdrom_SetMix (g_cd_atv), func_80035F78 (D_800A36B8), func_80036140\n'
                          '(g_cd_atv, D_800A36B8, g_cd_result) — the first rows, landed with func_80036140 (2026-09-26).\n')
    wr('.claude/rules/maspsx-gate-lists.md', g)
# ================================================================================ cdres (commit E)
elif STEP == 'cdres':
    # libcd's 8-byte result buffer as one object (both users now in the -G8 TU code6cac_b5.c)
    h = rd('include/code6cac.h')
    h = rep(h, 'extern u8 g_cd_result_plus_0x4;\n', (HERE / 'hdr_cdres.txt').read_text())
    wr('include/code6cac.h', h)
    S = 'src/code6cac_b5.c'
    p = rd(S)
    p = rep(p, 'extern u8 g_cd_result;\nextern u8 g_cd_result_plus_0x3;\nextern u8 g_cd_result_plus_0x5;\n', '')
    p = rep(p, 'extern u8 g_cd_result;\n', '')
    p = rep(p, 'CdSync(1, &g_cd_result)', 'CdSync(1, g_cd_result)', count=13)
    p = rep(p, 'CdReady(1, &g_cd_result)', 'CdReady(1, g_cd_result)')
    p = rep(p, 'CdControlF(0x11, (s32)&g_cd_result)', 'CdControlF(0x11, (s32)g_cd_result)')
    p = rep(p, 'g_cd_result_plus_0x4 & 0x80', 'g_cd_result[4] & 0x80')
    p = rep(p, '(s32)&g_cd_result_plus_0x3', '(s32)&g_cd_result[3]')
    p = rep(p, '(s32)&g_cd_result_plus_0x5', '(s32)&g_cd_result[5]')
    p = rep(p, 'if (g_cd_result & 0x10)', 'if (g_cd_result[0] & 0x10)', count=4)
    assert 'g_cd_result_plus' not in p and '&g_cd_result)' not in p
    wr(S, p)
    for rel in ('undefined_syms_auto.txt', 'named_syms.txt'):
        wr(rel, '\n'.join(l for l in rd(rel).split('\n') if not re.match(r'^\s*g_cd_result_plus_0x[345]\s*=', l)))
    d = rd('asm/data/91C98.data.s')
    prev = 'g_cd_result'
    for sub in ('g_cd_result_plus_0x3', 'g_cd_result_plus_0x4', 'g_cd_result_plus_0x5'):
        d = rep(d, f'enddlabel {prev}\n\nnonmatching {sub}\n\ndlabel {sub}\n', '')
        prev = sub
    d = rep(d, 'enddlabel g_cd_result_plus_0x5\n', 'enddlabel g_cd_result\n')
    wr('asm/data/91C98.data.s', d)
else:
    sys.exit('step?')
