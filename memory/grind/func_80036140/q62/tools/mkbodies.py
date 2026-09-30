"""Separate-object respellings of the E9C / EA4 read-modify-write sites (necessity re-proof, Q62 model).
body_ptr.c   : block-local pointer RMW at the two sites
body_alias.c : function-scope pointer aliases used at every E9C/EA4 access"""
from pathlib import Path
d = Path('tmp/func_80036140')
b = (d / 'body_q62.c').read_text()
b = b.replace('D_80101E58.rec.unk3C', 'D_80101E9C').replace('D_80101E58.rec.unk44', 'D_80101EA4')

def rep(t, o, n):
    assert o in t, o
    return t.replace(o, n, 1)

p = rep(b, """        if (D_80101E9C++ > 0x3C) {
            D_80101E58.rec.unk02 = 0x17;
        }""", """        {
            s16 *p = &D_80101E9C;
            if ((*p)++ > 0x3C) {
                D_80101E58.rec.unk02 = 0x17;
            }
        }""")
p = rep(p, """        if (D_80101EA4 != 0) {
            D_80101EA4 -= 4;
            if (D_80101EA4 <= 0) {""", """        if (D_80101EA4 != 0) {
            s32 *q = &D_80101EA4;
            *q -= 4;
            if (*q <= 0) {""")
(d / 'body_ptr.c').write_text(p)

a = b.replace('D_80101E9C', '*pE9C').replace('D_80101EA4', '*pEA4')
a = rep(a, "    CdlATV atv;\n", "    CdlATV atv;\n    s16 *pE9C = &D_80101E9C;\n    s32 *pEA4 = &D_80101EA4;\n")
(d / 'body_alias.c').write_text(a)
print('ok')
