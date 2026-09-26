"""Generate s3 variants: merged TexRec[12] (field access / u16 access) from candidate.c (split)."""
import os
R = '.'
split = open(R + '/memory/grind/func_800620B8/candidate.c', encoding='utf-8').read()
D = R + '/tmp/func_800620B8/s3/'
open(D + 'split.c', 'w', newline='\n').write(split)

TD = '''    typedef struct {
        u16 clut_x;
        u16 clut_y;
        u16 u;
        u16 v;
    } TexRec;
'''
def sub(s, a, b):
    assert s.count(a) == 1, a
    return s.replace(a, b)

m = split
m = sub(m, '    extern s32 D_800A32B8;\n', TD + '    extern s32 D_800A32B8;\n')
m = sub(m, '''    extern u16 D_8009BA00[6][4];
    extern u16 D_8009BA30[4][4];
    extern u16 D_8009BA50[4];
    extern u16 D_8009BA58[4];
''', '    extern TexRec D_8009BA00[12];\n')
m = sub(m, '(s32)D_8009BA00[(u32)D_800A32B8 % 6]', '(s32)&D_8009BA00[(u32)D_800A32B8 % 6]')
m = sub(m, '(s32)D_8009BA50;', '(s32)&D_8009BA00[10];')
m = sub(m, '(s32)D_8009BA30[D_800A32B8 & 3]', '(s32)&D_8009BA00[(D_800A32B8 & 3) + 6]')
m = sub(m, '(s32)D_8009BA58;', '(s32)&D_8009BA00[11];')
open(D + 'merged_u16.c', 'w', newline='\n').write(m)
f = m
f = sub(f, '((u16 *)D_800A3488)[2] + 0x1F', '((TexRec *)D_800A3488)->u + 0x1F')
f = sub(f, '((u16 *)D_800A3488)[3] + 0x1F', '((TexRec *)D_800A3488)->v + 0x1F')
f = sub(f, '((u16 *)D_800A3488)[2] + 0xF', '((TexRec *)D_800A3488)->u + 0xF')
f = sub(f, '((u16 *)D_800A3488)[3] + 0x13', '((TexRec *)D_800A3488)->v + 0x13')
f = sub(f, '(((((u16 *)D_800A348C)[0] >> 4) & 0x3F) + (((u16 *)D_800A348C)[1] << 6))',
        '(((((TexRec *)D_800A348C)->clut_x >> 4) & 0x3F) + (((TexRec *)D_800A348C)->clut_y << 6))')
f = sub(f, '*(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];', '*(u16 *)D_800A3498 = ((TexRec *)D_800A3488)->u;')
f = sub(f, '*(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];', '*(u16 *)D_800A34A0 = ((TexRec *)D_800A3488)->v;')
open(D + 'merged_tex.c', 'w', newline='\n').write(f)
# split with TexRec field access (the same reads, split symbols) for a like-for-like split measurement
s = f
s = sub(s, '    extern TexRec D_8009BA00[12];\n', '''    extern TexRec D_8009BA00[6];
    extern TexRec D_8009BA30[4];
    extern TexRec D_8009BA50;
    extern TexRec D_8009BA58;
''')
s = sub(s, '(s32)&D_8009BA00[10];', '(s32)&D_8009BA50;')
s = sub(s, '(s32)&D_8009BA00[(D_800A32B8 & 3) + 6]', '(s32)&D_8009BA30[D_800A32B8 & 3]')
s = sub(s, '(s32)&D_8009BA00[11];', '(s32)&D_8009BA58;')
open(D + 'split_tex.c', 'w', newline='\n').write(s)
print('ok')
