"""func_80055138 landing model (the data-model edits that ship with the body).
  include/code6cac.h : cpu_practice_honmokuroku_data_tbl -> u8 [][4]; StatusFlagRec table
                       D_80099D88[] + D_8009A8C4[][8][4] + D_8009A9B4[][2] (after D_80094C68)
  src/text1b.c       : body spliced over INCLUDE_ASM; the TU-local `extern u16 D_80099D88;`
                       dropped; func_80055948's read respelled D_80099D88[idx].flags
  src/code6cac_b.c   : func_80033DF4 `&tbl + (tableIndex * 4)` -> `tbl[tableIndex]`
  undefined_syms_auto.txt : alias-of suffixes on the record-0 field rows (prong (c) amendment)
usage (repo root):
  python3 tmp/func_80055138/r11/model.py apply <body.c>        # edit the REAL tree (lock holder only)
  python3 tmp/func_80055138/r11/model.py score <body.c> [--all] # scratch copy, score vs build/"""
import os, subprocess, sys

HDR_BLOCK = (
    '/* 0x18-byte per-character status record table (named_syms.txt:\n'
    ' * g_status_flag_record_table_80099D88). The original binary indexes it by character id\n'
    ' * with stride 0x18 (id*3<<3) in func_80055138, func_80055948, func_80055B60 and\n'
    ' * func_80058580: flags halfword at +0, bytes at +3..+8, +0xC, +0xF, +0x14, +0x15. The\n'
    ' * splat symbols D_80099D8B..D_80099D9D are fields of record 0 (alias rows in\n'
    ' * undefined_syms_auto.txt, retired with func_80055B60 / func_80058580). */\n'
    'typedef struct StatusFlagRec {\n'
    '    u16 flags;\n'
    '    u8 unk2;\n'
    '    u8 unk3;\n'
    '    u8 unk4[0x18 - 4];\n'
    '} StatusFlagRec;\n'
    'extern StatusFlagRec D_80099D88[];\n'
    '/* Rows of eight 4-byte entries: func_80055138 reads [row][col][0..1] (row*0x20 + col*4);\n'
    ' * func_80058580 reads the halfword at byte 2 of [row][col] (D_8009A8CA + row<<5 +\n'
    ' * (col-1)*4 = D_8009A8C4 + row*0x20 + col*4 + 2, 0x8005A854-78; D_8009A8CA is the alias\n'
    ' * row D_8009A8C4+6). */\n'
    'extern u8 D_8009A8C4[][8][4];\n'
    'extern u8 D_8009A9B4[][2];         /* byte pairs (func_80055138) */\n')

ALIASES = [  # (symbol, offset, referrers)
    ('D_80099D8B', 0x3, 'func_80058580'),
    ('D_80099D8C', 0x4, 'func_80058580'),
    ('D_80099D8D', 0x5, 'func_80055B60'),
    ('D_80099D8E', 0x6, 'func_80058580'),
    ('D_80099D8F', 0x7, 'func_80055B60 and func_80058580'),
    ('D_80099D94', 0xC, 'func_80058580'),
    ('D_80099D97', 0xF, 'func_80058580'),
    ('D_80099D9C', 0x14, 'func_80058580'),
    ('D_80099D9D', 0x15, 'func_80058580'),
    ('D_8009A8CA', 0x6, 'func_80058580'),
]


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


def t_hdr(h):
    h = one(h, 'extern u8 cpu_practice_honmokuroku_data_tbl;\n',
            'extern u8 cpu_practice_honmokuroku_data_tbl[][4];\n')
    return one(h, 'extern u16 D_80094C68[];\n', 'extern u16 D_80094C68[];\n' + HDR_BLOCK)


def t_text1b(s, body):
    s = one(s, 'INCLUDE_ASM("asm/funcs", func_80055138);', body.rstrip('\n'))
    s = one(s, 'extern u16 D_80099D88;\n', '')
    return one(s, '(*(&D_80099D88 + idx * 12) & 0xBF00)', '(D_80099D88[idx].flags & 0xBF00)')


def t_c6b(s):
    return one(s, 'u8 *table = &cpu_practice_honmokuroku_data_tbl + (tableIndex * 4);',
               'u8 *table = cpu_practice_honmokuroku_data_tbl[tableIndex];')


def t_syms(s):
    for sym, off, who in ALIASES:
        old = '%s = 0x%s;\n' % (sym, sym[2:])
        refs = ('asm/funcs/%s.s is its only assembled referrer' % who) if ' and ' not in who else \
            'asm/funcs/%s.s are its assembled referrers' % who.replace(' and ', '.s and asm/funcs/')
        what = ('D_80099D88+0x%X (StatusFlagRec record 0)' % off if sym.startswith('D_80099D')
                else 'D_8009A8C4+0x%X (entry [0][1], byte 2)' % off)
        s = one(s, old, '%s = 0x%s;  /* alias of %s; '
                        'retire with %s (%s) */\n' % (sym, sym[2:], what, who, refs))
    return s


def rw(path, fn, *a):
    s = open(path, encoding='utf-8').read()
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(fn(s, *a))


def apply(body):
    rw('include/code6cac.h', t_hdr)
    rw('src/text1b.c', t_text1b, body)
    rw('src/code6cac_b.c', t_c6b)
    rw('undefined_syms_auto.txt', t_syms)
    print('applied')


def score(body, stems, full):
    sys.path.insert(0, '.')
    from engine import score as S
    from engine import pipeline as P
    W = 'tmp/func_80055138/wf3'
    os.makedirs(W + '/inc', exist_ok=True)
    open(W + '/inc/code6cac.h', 'w', newline='\n').write(t_hdr(open('include/code6cac.h').read()))
    t_syms(open('undefined_syms_auto.txt').read())  # assert the rows exist
    for stem in stems:
        src = open('src/%s.c' % stem).read()
        if stem == 'text1b':
            src = t_text1b(src, body)
        if stem == 'code6cac_b':
            src = t_c6b(src)
        open('%s/%s.c' % (W, stem), 'w', newline='\n').write(src)
        out = '%s/%s.o' % (W, stem)
        cmd = P.c_pipeline_cmd(stem, out, None)
        cmd = cmd.replace('src/%s.c' % stem, '%s/%s.c' % (W, stem), 1).replace(
            '-Iinclude', '-I%s/inc -Iinclude' % W, 1)
        r = subprocess.run(['bash', '-c', 'set -o pipefail; ' + cmd], capture_output=True, text=True)
        if r.returncode:
            print('BUILD FAILED', stem, r.stderr[-3000:])
            sys.exit(1)
        ref = 'build/src/%s.o' % stem
        if not full:
            print(stem, 'func_80055138', S.score_func(out, ref, 'func_80055138'))
            continue
        nm = subprocess.run(['mipsel-linux-gnu-nm', ref], capture_output=True, text=True).stdout
        funcs = sorted({l.split()[2] for l in nm.splitlines()
                        if len(l.split()) == 3 and l.split()[1] in 'Tt' and '.' not in l.split()[2]})
        bad = [f for f in funcs if (lambda d: d['score'] or d['target_insns'] != d['build_insns'])(
            S.score_func(out, ref, f))]
        print('%s: %d functions scored, %d differ %s' % (stem, len(funcs), len(bad), bad))


if __name__ == '__main__':
    if sys.argv[1] == 'apply':
        apply(open(sys.argv[2]).read())
    else:
        full = '--all' in sys.argv
        score(open(sys.argv[2]).read(), ['text1b', 'code6cac_b'] if full else ['text1b'], full)
