import re
R = 'memory/grind/func_8001F2E4/r11/'
split = open(R + 'one-var-per-value-form.c').read()
onlyv2 = open(R + 'variants/r11v_only_V2.c').read()
final = open(R + 'final.c').read()

def divassign(src, var, off):
    old = '*(s16 *)(obj + %s) = *(s16 *)(obj + %s) + %s / 8;' % (off, off, var)
    new = '%s /= 8;\n    *(s16 *)(obj + %s) = *(s16 *)(obj + %s) + %s;' % (var, off, off, var)
    assert old in src, (var, off)
    return src.replace(old, new)

def w(name, s):
    open('tmp/l2probe_8001F2E4/' + name, 'w', newline='\n').write(s)

p1 = divassign(divassign(divassign(split, 'd1e6', '0x1E6'), 'd1e8', '0x1E8'), 'delta', '0x1EA')
w('P1_split_all_divassign.c', p1)
w('P2_onlyV2_divassign.c', divassign(onlyv2, 'd1e8', '0x1E8'))
p3 = divassign(divassign(divassign(final, 'temp', '0x1E6'), 'temp', '0x1E8'), 'temp', '0x1EA')
w('P3_final_divassign.c', p3)
# P4: split_all, divassign, and the jitters / twist at function scope (one-var form, conflicts differ)
