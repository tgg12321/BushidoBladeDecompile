"""Write tmp/func_80034708/integ/bb2.ld: bb2.ld + the code6cac_b3 / code6cac_b3_post objects."""
s = open('bb2.ld', encoding='utf-8').read()
def ins_after(s, anchor, add):
    assert s.count(anchor) == 1, anchor
    return s.replace(anchor, anchor + add)
s = ins_after(s, '        build/src/code6cac_b_rodata_pre.o(.rodata);\n',
              '        build/src/code6cac_b3.o(.rodata);\n        build/src/code6cac_b3_post.o(.rodata);\n')
for sec in ('text', 'data', 'bss'):
    s = ins_after(s, f'        build/src/code6cac_b.o(.{sec});\n',
                  f'        build/src/code6cac_b3.o(.{sec});\n        build/src/code6cac_b3_post.o(.{sec});\n')
open('tmp/func_80034708/integ/bb2.ld', 'w', encoding='utf-8', newline='\n').write(s)
print('ld ok')
