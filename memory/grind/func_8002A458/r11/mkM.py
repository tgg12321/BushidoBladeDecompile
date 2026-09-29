"""Sanctioned-FAKE-family exclusion variants for the Ruling 11 (D)(3) argument, built from the
one-variable-per-value ablations in r11v/ (run genr11.py first)."""
D = 'tmp/func_8002A458/r11v/'
A = open(D + 'dxyz_all_own.c', encoding='utf-8').read()
a = A.index('        if (*hit != 0) {\n') + len('        if (*hit != 0) {\n')
b = A.index('            *hit = 0;\n', a) + len('            *hit = 0;\n')
body = A[a:b]
decls = ''.join(l + '\n' for l in body.split('\n') if l.strip().startswith('s32 '))
rest = '\n'.join(l for l in body.split('\n') if not l.strip().startswith('s32 ') and l.strip() != '') + '\n'
# M1: per-value deltas + do{...}while(0) around the end block (do-while-zero-exception)
M1 = A[:a] + decls + '            do {\n' + rest + '            } while (0);\n' + A[b:]
open(D + 'M1_dxyz_split_dowhile.c', 'w', newline='\n').write(M1)
# M2: per-value deltas at function scope + FAKE self-assigns at function entry
M2 = A.replace(decls, '', 1)
M2 = M2.replace('    s32 hit_sq;\n', '    s32 hit_sq;\n' + decls.replace('            ', '    '), 1)
M2 = M2.replace('    *(u8 **)(scr + 0x60) = scr;\n',
                '    hx = hx;\n    hy = hy;\n    hz = hz;\n    ox = ox;\n    oy = oy;\n    oz = oz;\n'
                '    *(u8 **)(scr + 0x60) = scr;\n', 1)
open(D + 'M2_dxyz_split_selfassign.c', 'w', newline='\n').write(M2)
# M3: squared length in its own h_sq + combine-foldable chain-extender on its < 0x400 read
T = open(D + 'temp_all_own.c', encoding='utf-8').read()
M3 = T.replace('if ((u32)h_sq < 0x400)', 'if ((u32)(h_sq + ((h_sq << 4) & 0xF)) < 0x400)', 1)
assert M3 != T
open(D + 'M3_temp_split_chainext.c', 'w', newline='\n').write(M3)
# M4: copy in its own n, declared u32
S = open(D + 'temp2_split.c', encoding='utf-8').read()
M4 = S.replace('    s32 n;\n', '    u32 n;\n', 1)
assert M4 != S
open(D + 'M4_temp2_split_u32.c', 'w', newline='\n').write(M4)
# M5: copy in its own n + a zero-valued detour reading n at the table-byte use (live-range extender)
M5 = S.replace('hlen = (u32)(tbl << 16)', 'hlen = (u32)((tbl + (((s32)n << 4) & 0xF)) << 16)', 1)
assert M5 != S
open(D + 'M5_temp2_split_rangeext.c', 'w', newline='\n').write(M5)
print('ok')
