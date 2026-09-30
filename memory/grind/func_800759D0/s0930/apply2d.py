import sys, os
# apply2d.py <tree> <body759d0.c>: D_8009BCF8 as [2][10] pages; 76D74 flat read via &D[0][0]; splice func_800759D0
T, body = sys.argv[1], sys.argv[2]
h = os.path.join(T, 'include/game.h')
s = open(h, newline='').read()
assert s.count('extern Unk8009BCF8Record D_8009BCF8[20];') == 1
s = s.replace('extern Unk8009BCF8Record D_8009BCF8[20];', 'extern Unk8009BCF8Record D_8009BCF8[2][10];')
open(h, 'w', newline='\n').write(s)
p = os.path.join(T, 'src/text1b_tu2.c')
s = open(p, newline='').read()
old = 'D_8009BCF8[*(s16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x6A)].unk1'
assert s.count(old) == 1
new76 = os.environ.get('NEW76', 'D_8009BCF8[0][*(s16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x6A)].unk1')
s = s.replace(old, new76)
key = 'INCLUDE_ASM("asm/funcs", func_800759D0);\n'
assert s.count(key) == 1
s = s.replace(key, open(body).read().rstrip('\n') + '\n')
open(p, 'w', newline='\n').write(s)
print('2d applied')
