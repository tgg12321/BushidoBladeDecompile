import sys, os
# apply.py <tree> <body.c>: move the SelWork_800768DC block above func_800759D0, splice func_80075F80
T, body = sys.argv[1], sys.argv[2]
p = os.path.join(T, 'src/text1b_tu2.c')
s = open(p, newline='').read()
start = s.index('/* Shared select work area at D_800A36A0')
end = s.index('#define SELWORK_800768DC ((SelWork_800768DC *)D_800A36A0)\n') + len('#define SELWORK_800768DC ((SelWork_800768DC *)D_800A36A0)\n')
blk = s[start:end]
s = s[:start] + s[end:]
anchor = 'extern u8 D_8009BCE4;\n\nINCLUDE_ASM("asm/funcs", func_800759D0);\n'
assert s.count(anchor) == 1
s = s.replace(anchor, blk + '\n' + anchor)
key = 'INCLUDE_ASM("asm/funcs", func_80075F80);\n'
assert s.count(key) == 1
s = s.replace(key, open(body).read().rstrip('\n') + '\n')
open(p, 'w', newline='\n').write(s)
print('applied', body)
