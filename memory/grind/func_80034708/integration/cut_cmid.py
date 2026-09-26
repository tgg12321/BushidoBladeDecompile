p = 'tmp/func_80034708/record_members.py'
lines = open(p).read().split('\n')
a = next(i for i, l in enumerate(lines) if l.startswith('# ---- code6cac_c_mid.c func_80037F40'))
b = next(i for i, l in enumerate(lines) if l.startswith('# ---- code6cac_c2.c func_8003C714'))
keep = ['# ---- code6cac_c_mid.c func_80037F40 keeps its whole-record checksum (u8 walk over the',
        '# object) and its Quad copy ending at .color: the struct-assignment form differs by the end',
        '# pointer\'s schedule (5 words; ledger [s8]), so only the cast form reproduces the bytes.', '']
open(p, 'w', newline='\n').write('\n'.join(lines[:a] + keep + lines[b:]))
print('cut', b - a, 'lines')
