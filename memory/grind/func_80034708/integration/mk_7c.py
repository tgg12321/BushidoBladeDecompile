"""Variant: aggregate based at 0x8010277C (12 bytes), D_80102778[2] stays separate.
Writes tmp/func_80034708/i7c/{include,src/code6cac_b3.c} from the integ/ tree."""
import os, re, shutil
R = 'tmp/func_80034708/integ'
O = 'tmp/func_80034708/i7c'
shutil.rmtree(O, ignore_errors=True)
shutil.copytree(R + '/include', O + '/include')
os.makedirs(O + '/src')
h = open(O + '/include/code6cac.h').read()
i = h.index('/* Practice-lesson parameter block')
j = h.index('extern PracticeParams D_80102778;\n') + len('extern PracticeParams D_80102778;\n')
h = h[:i] + '''extern u16 D_80102778[2];
typedef struct {
    u8 unk_0[2];
    u8 unk_2[2];
    u8 unk_4[2];
    u8 unk_6[2];
    u8 unk_8;
    u8 unk_9;
    u8 unk_A;
    u8 unk_B;
} PlayerBytePairs;
extern PlayerBytePairs D_8010277C;
''' + h[j:]
open(O + '/include/code6cac.h', 'w', newline='\n').write(h)
s = open(R + '/src/code6cac_b3.c').read()
M = {'unk_4': 'unk_0', 'unk_6': 'unk_2', 'unk_8': 'unk_4', 'unk_C': 'unk_8', 'unk_D': 'unk_9', 'unk_E': 'unk_A', 'unk_F': 'unk_B'}
s = re.sub(r'D_80102778\.(unk_[0-9A-F])', lambda m: 'D_80102778' if m.group(1) == 'unk_0' else 'D_8010277C.' + M[m.group(1)], s)
open(O + '/src/code6cac_b3.c', 'w', newline='\n').write(s)
print('7c ok', s.count('D_8010277C.'))
