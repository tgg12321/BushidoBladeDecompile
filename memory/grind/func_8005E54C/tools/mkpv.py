#!/usr/bin/env python3
"""Per-value spellings of the `vals` pair (round points R2/R6 vs totals R3/R4) from match0/body.c."""
B = 'memory/grind/func_8005E54C/match0/body.c'
V = 'tmp/func_8005E54C/v/'
s = open(B, newline='\n').read()
a = s.index('    *(s32 *)vals = 0;')
b = s.index('    SetDrawMode(mode_off, 1, 0, func_8006E480((s32)&D_8009B524, 0), 0);')
mid = s[a:b].replace('*(s32 *)vals', '*(s32 *)total').replace('vals[', 'total[')
t = s[:a] + mid + s[b:]


def w(name, txt):
    open(V + name + '.c', 'w', newline='\n').write(txt)


w('pv_after', t.replace('    volatile s16 digit[3];\n', '    volatile s16 digit[3];\n    s16 total[2];\n'))
w('pv_first', t.replace('    s16 vals[2];\n', '    s16 total[2];\n    s16 vals[2];\n'))
w('pv_before_s', t.replace('    s16 wins[2];\n', '    s16 wins[2];\n    s16 total[2];\n'))
blk = s[:a] + '    {\n        s16 total[2];\n\n' + mid + '    }\n' + s[b:]
w('pv_block', blk)
print('ok')
