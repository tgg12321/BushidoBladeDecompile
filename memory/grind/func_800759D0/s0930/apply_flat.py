import sys, os
T, body = sys.argv[1], sys.argv[2]
p = os.path.join(T, 'src/text1b_tu2.c')
s = open(p, newline='').read()
key = 'INCLUDE_ASM("asm/funcs", func_800759D0);\n'
assert s.count(key) == 1
s = s.replace(key, open(body).read().rstrip('\n') + '\n')
open(p, 'w', newline='\n').write(s)
print('759d0 flat applied')
