"""Stage 1: generate tmp/func_80034708/integ/{include,src} from the live tree with the
PracticeParams aggregate at 0x80102778 (0x78..0x87). The three per-player byte pairs at
0x7C..0x81 are one u8[6] member (unk_4[2 * k + player])."""
import os, re, shutil
R = 'tmp/func_80034708/integ'
shutil.rmtree(R, ignore_errors=True)
os.makedirs(R + '/include'); os.makedirs(R + '/src')

def rd(p):
    return open(p, encoding='utf-8').read()

def wr(p, s):
    open(p, 'w', encoding='utf-8', newline='\n').write(s)

h = rd('include/code6cac.h')
old = '''extern u16 D_80102778[2];
extern s16 D_8010277A;
/* Three per-player byte pairs at 0x8010277C ([0] = P1, [1] = P2).
 * func_8001DCB0 and func_8003AF40 index each pair by player. */
typedef struct {
    u8 unk_0[2];
    u8 unk_2[2];
    u8 unk_4[2];
} PlayerBytePairs;
extern PlayerBytePairs D_8010277C;
extern u8 D_80102782[];
extern u8 D_80102783;
extern u8 D_80102784;
extern u8 D_80102785;
extern u8 D_80102786;
extern u8 D_80102787;
'''
new = '''/* Practice-lesson parameter block 0x80102778..0x80102787 (func_8001C444 sets
 * every byte of it except 0x82/0x83): two u16 values, three per-player byte
 * pairs kept as one array (unk_4[2 * k + player], [0] = P1, [1] = P2), a
 * fourth per-player pair only other functions touch (unk_A, indexed by player
 * in func_80022F34) and four single bytes. One object: func_80034708 reaches
 * unk_4 and unk_E as offsets from the address of unk_C (layout evidence:
 * memory/grind/func_80034708/evidence.md [s4]-[s5]). */
typedef struct {
    u16 unk_0[2];
    u8 unk_4[6];
    u8 unk_A[2];
    u8 unk_C;
    u8 unk_D;
    u8 unk_E;
    u8 unk_F;
} PracticeParams;
extern PracticeParams D_80102778;
'''
assert old in h
h = h.replace(old, new)
wr(R + '/include/code6cac.h', h)

PAIR = re.compile(r'D_8010277C\.unk_([024])\[')


def pair_index(text):
    """D_8010277C.unk_{0,2,4}[E] -> D_80102778.unk_4[k + E], k = 0 / 2 / 4."""
    out, i = [], 0
    while True:
        m = PAIR.search(text, i)
        if not m:
            out.append(text[i:])
            break
        out.append(text[i:m.start()])
        j, depth = m.end(), 1
        while depth:
            depth += {'[': 1, ']': -1}.get(text[j], 0)
            j += 1
        e = text[m.end():j - 1]
        k = int(m.group(1))
        if re.fullmatch(r'\d+', e):
            idx = str(k + int(e))
        elif k == 0:
            idx = e
        elif re.fullmatch(r'\w+', e):
            idx = f'{k} + {e}'
        else:
            # a computed index (e.g. the other player, `(u32)arg0 < 1u`): index the pair itself
            out.append(f'(&D_80102778.unk_4[{k}])[{e}]')
            i = j
            continue
        out.append(f'D_80102778.unk_4[{idx}]')
        i = j
    return ''.join(out)


subs = [
    (r'\bD_80102778\[', 'D_80102778.unk_0['),
    (r'tbl = D_80102778;', 'tbl = D_80102778.unk_0;'),
    (r'\bD_8010277A\b', 'D_80102778.unk_0[1]'),
    (r'D_80102782\[', 'D_80102778.unk_A['),
    (r'\bD_80102783\b', 'D_80102778.unk_A[1]'),
    (r'\bD_80102784\b', 'D_80102778.unk_C'),
    (r'\bD_80102785\b', 'D_80102778.unk_D'),
    (r'\bD_80102786\b', 'D_80102778.unk_E'),
    (r'\bD_80102787\b', 'D_80102778.unk_F'),
]
STEMS1 = ['code6cac', 'code6cac_b', 'code6cac_b2_post', 'code6cac_c_ab', 'code6cac_c2', 'code6cac_c_mid',
          'code6cac_b2_pre', 'code6cac_c', 'code6cac_c0', 'text1b', 'text1a_c', 'main', 'ings', 'system',
          'text1a_pre', 'text1a_post', 'text1a_b', 'text1b_b']
for stem in [x for x in STEMS1 if os.path.exists(f'src/{x}.c')]:
    s = rd(f'src/{stem}.c')
    out = []
    for line in s.split('\n'):
        st = line.lstrip()
        if not re.match(r'(/\*|\*\s|\*/|\*$)', st):
            line = pair_index(line)
            for a, b in subs:
                line = re.sub(a, b, line)
        out.append(line)
    wr(f'{R}/src/{stem}.c', '\n'.join(out))
print('ok')
