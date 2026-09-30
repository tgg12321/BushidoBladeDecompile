import os
D = 'tmp/func_8005C8A8/s4'
os.makedirs(D, exist_ok=True)
c = open('memory/grind/func_8005C8A8/candidate.c').read()
assert c.count('    s32 size;\n') == 1 and c.count('    size = 0x4F0;\n') == 1

def w(n, s):
    assert s != c or n == 'lit'
    open(os.path.join(D, n + '.c'), 'w', newline='\n').write(s)

w('lit', c)
w('vol', c.replace('    s32 size;\n', '    volatile s32 size;\n'))
w('s16', c.replace('    s32 size;\n', '    s16 size;\n'))
w('one_struct', c.replace('    s32 size;\n', '    struct { s32 n; } size;\n').replace('size = 0x4F0;', 'size.n = 0x4F0;').replace('return size;', 'return size.n;'))
w('arr1', c.replace('    s32 size;\n', '    s32 size[1];\n').replace('size = 0x4F0;', 'size[0] = 0x4F0;').replace('return size;', 'return size[0];'))
w('ret_lit', c.replace('    s32 size;\n', '').replace('    size = 0x4F0;\n', '').replace('return size;', 'return 0x4F0;'))
