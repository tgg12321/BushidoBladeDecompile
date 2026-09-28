"""Struct field type sweep."""
import os, itertools
src = open('tmp/c21c/base.c').read()
os.makedirs('tmp/c21c/sw2', exist_ok=True)
names = []
REC = "typedef struct {\n    s16 x, y, w, h;\n    u8 r, g, b, pad;\n} Rec_8006C21C;"
assert REC in src
for tx, ty, tw, th in itertools.product(['s16', 'u16'], repeat=4):
    if (tx, ty, tw, th) == ('s16',) * 4:
        continue
    rec = f"typedef struct {{\n    {tx} x;\n    {ty} y;\n    {tw} w;\n    {th} h;\n    u8 r, g, b, pad;\n}} Rec_8006C21C;"
    n = f'rec_{tx[0]}{ty[0]}{tw[0]}{th[0]}'
    open(f'tmp/c21c/sw2/{n}.c', 'w', newline='\n').write(src.replace(REC, rec))
    names.append(f'tmp/c21c/sw2/{n}.c')
# poly coords unsigned
a = src.replace("    s16 x0, y0;\n    u8 r1", "    u16 x0, y0;\n    u8 r1").replace("    s16 x1, y1;\n    u8 r2", "    u16 x1, y1;\n    u8 r2").replace("    s16 x2, y2;\n    u8 r3", "    u16 x2, y2;\n    u8 r3").replace("    s16 x3, y3;\n} PolyG4", "    u16 x3, y3;\n} PolyG4")
assert a != src
open('tmp/c21c/sw2/poly_u16.c', 'w', newline='\n').write(a); names.append('tmp/c21c/sw2/poly_u16.c')
# rgb signed in rec
a = src.replace("    u8 r, g, b, pad;\n} Rec_8006C21C;", "    s8 r, g, b, pad;\n} Rec_8006C21C;")
open('tmp/c21c/sw2/rec_rgb_s8.c', 'w', newline='\n').write(a); names.append('tmp/c21c/sw2/rec_rgb_s8.c')
# poly colours signed
a = src
for c in ['r0, g0, b0, code', 'r1, g1, b1, p1', 'r2, g2, b2, p2', 'r3, g3, b3, p3']:
    a = a.replace(f'    u8 {c};', f'    s8 {c};')
open('tmp/c21c/sw2/poly_rgb_s8.c', 'w', newline='\n').write(a); names.append('tmp/c21c/sw2/poly_rgb_s8.c')
# env x,y as s16 pairs? (keep sw) skip. Env has_color s8
a = src.replace('    u8 has_color;\n    u8 col_r, col_g, col_b;', '    s8 has_color;\n    s8 col_r, col_g, col_b;')
open('tmp/c21c/sw2/env_s8.c', 'w', newline='\n').write(a); names.append('tmp/c21c/sw2/env_s8.c')
open('tmp/c21c/sw2/list.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))
