"""Variant A: no PracticeParams change at all (live header layout for 0x78..0x87), FileRecord merged, -G8 TU."""
import os, re, shutil
R = 'tmp/func_80034708/integ'
O = 'tmp/func_80034708/iA'
shutil.rmtree(O, ignore_errors=True)
shutil.copytree(R + '/include', O + '/include')
os.makedirs(O + '/src')
h = open('include/code6cac.h').read()
assert 'extern u8 D_80106A70[3];\n' in h
h = h.replace('extern u8 D_80106A70[3];\n', '')
open(O + '/include/code6cac.h', 'w', newline='\n').write(h)
s = open(R + '/src/code6cac_b3.c').read()
M = {'unk_4': 'D_8010277C.unk_0', 'unk_6': 'D_8010277C.unk_2', 'unk_8': 'D_8010277C.unk_4',
     'unk_C': 'D_80102784', 'unk_D': 'D_80102785', 'unk_E': 'D_80102786', 'unk_F': 'D_80102787'}
s = re.sub(r'D_80102778\.(unk_[0-9A-F])', lambda m: 'D_80102778' if m.group(1) == 'unk_0' else M[m.group(1)], s)
open(O + '/src/code6cac_b3.c', 'w', newline='\n').write(s)
print('A ok')
