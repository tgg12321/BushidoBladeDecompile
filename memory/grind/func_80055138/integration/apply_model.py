"""Apply the func_80055138 landing model to the REAL tree (lock holder only):
include/code6cac.h, src/code6cac_b.c, src/text1b.c. usage (WSL): python3 tmp/func_80055138/apply_model.py <body.c>"""
import sys

def rw(path, fn):
    s = open(path, encoding='utf-8').read()
    t = fn(s)
    with open(path, 'w', encoding='utf-8', newline='\n') as f:
        f.write(t)

def one(s, o, n):
    assert s.count(o) == 1, o
    return s.replace(o, n)

def hdr(h):
    h = one(h, 'extern u8 cpu_practice_honmokuroku_data_tbl;\n', 'extern u8 cpu_practice_honmokuroku_data_tbl[][4];\n')
    return one(h, 'extern u8 D_80101EC8;\n', 'extern u8 D_80101EC8;\n' + (
        '/* Per-character status-flag record table (0x80099D88, stride 0x18). */\n'
        'typedef struct StatusFlagRec {\n'
        '    u16 flags;\n'
        '    u8  unk2;\n'
        '    u8  unk3;\n'
        '    u8  unk4[0x18 - 4];\n'
        '} StatusFlagRec;\n'
        'extern StatusFlagRec D_80099D88[];\n'))

body = open(sys.argv[1], encoding='utf-8').read().rstrip('\n')

def t1b(s):
    s = one(s, 'INCLUDE_ASM("asm/funcs", func_80055138);', body)
    s = one(s, 'extern u16 D_80099D88;\n', '')
    return one(s, '(*(&D_80099D88 + idx * 12) & 0xBF00)', '(D_80099D88[idx].flags & 0xBF00)')

def c6b(s):
    return one(s, 'u8 *table = &cpu_practice_honmokuroku_data_tbl + (tableIndex * 4);',
               'u8 *table = cpu_practice_honmokuroku_data_tbl[tableIndex];')

rw('include/code6cac.h', hdr)
rw('src/text1b.c', t1b)
rw('src/code6cac_b.c', c6b)
print('applied')
