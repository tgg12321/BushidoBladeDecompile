import sys, os
# apply.py <tree> <body.c>: replace func_80052D00's INCLUDE_ASM line in the scratch tree's src/text1b.c
T, body = sys.argv[1], sys.argv[2]
p = os.path.join(T, 'src/text1b.c')
s = open(p, newline='').read()
key = 'INCLUDE_ASM("asm/funcs", func_80052D00);\n'
assert s.count(key) == 1
s = s.replace(key, open(body).read().rstrip('\n') + '\n')
open(p, 'w', newline='\n').write(s)
print('applied', body)
