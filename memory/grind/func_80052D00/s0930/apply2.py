import sys, os
# apply2.py <tree> <body.c>: Cell_80052D00 as a plain struct + func_80052D00 body in the scratch tree
T, body = sys.argv[1], sys.argv[2]
p = os.path.join(T, 'src/text1b.c')
s = open(p, newline='').read()
old = "typedef union {\n    struct {\n        s16 x;\n        s16 z;\n    } c;\n    s32 w;\n} Cell_80052D00;\n"
new = "typedef struct {\n    s16 x;\n    s16 z;\n} Cell_80052D00;\n"
assert s.count(old) == 1
s = s.replace(old, new)
key = 'INCLUDE_ASM("asm/funcs", func_80052D00);\n'
assert s.count(key) == 1
s = s.replace(key, open(body).read().rstrip('\n') + '\n')
open(p, 'w', newline='\n').write(s)
print('applied2', body)
