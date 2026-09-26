"""Generate tmp/func_80034708/integ/{include,src} from the live tree with the 0x8010277C..87 aggregate."""
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
new = '''/* Practice-lesson parameter block 0x80102778..0x80102787 (func_8001C444
 * initialises exactly this range): two u16 sizes, four per-player byte pairs
 * ([0] = P1, [1] = P2; func_8001DCB0, func_8003AF40 and func_80022F34 index
 * them by player) and four scalar bytes. One object: func_80034708 reaches
 * the pairs and unk_E as offsets from the address of unk_C, and its phase-A
 * reads relate to the unk_4 address the way only a base at 0x78 produces. */
typedef struct {
    u16 unk_0[2];
    u8 unk_4[2];
    u8 unk_6[2];
    u8 unk_8[2];
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

FIELD = {'unk_0': 'unk_4', 'unk_2': 'unk_6', 'unk_4': 'unk_8'}
subs = [
    (r'D_8010277C\.(unk_[024])', lambda m: 'D_80102778.' + FIELD[m.group(1)]),
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
STEMS1 = ['code6cac', 'code6cac_b', 'code6cac_b2_post', 'code6cac_c_ab', 'code6cac_c2', 'code6cac_c_mid', 'code6cac_b2_pre', 'code6cac_c', 'code6cac_c0', 'text1b', 'text1a_c', 'main', 'ings', 'system', 'text1a_pre', 'text1a_post', 'text1a_b', 'text1b_b']
for stem in [x for x in STEMS1 if os.path.exists(f'src/{x}.c')]:
    s = rd(f'src/{stem}.c')
    out = []
    for line in s.split('\n'):
        st = line.lstrip()
        if not re.match(r'(/\*|\*\s|\*/|\*$)', st):
            for a, b in subs:
                line = re.sub(a, b, line)
        out.append(line)
    wr(f'{R}/src/{stem}.c', '\n'.join(out))
print('ok')
