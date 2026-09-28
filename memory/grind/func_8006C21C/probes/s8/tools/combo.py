"""Measure combinations of the two policy-blocked levers on the s8 candidate."""
import os
src = open('memory/grind/func_8006C21C/candidate.c').read()
L0 = src.split('\n')
os.makedirs('tmp/c21c/cmb', exist_ok=True)
names = []
NL = chr(10)


def edit(L, ln, a, b):
    assert a in L[ln - 1], (ln, L[ln - 1], a)
    L[ln - 1] = L[ln - 1].replace(a, b, 1)


def colour(L, mode):
    t = NL.join(L)
    b1e_old = """                SetSemiTrans(poly, 0);

                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = poly->r2 = 0x80;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->g3 = 0;"""
    b1e_new = """                SetSemiTrans(poly, 0);
                col = 0x80;
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = col;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = col;
                poly->g3 = 0;"""
    b2e_old = """                SetSemiTrans(poly, 0);

                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = poly->r1 = 0x80;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->g3 = 0;"""
    b2e_new = """                SetSemiTrans(poly, 0);
                col = 0x80;
                poly->r0 = pulse;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = col;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = col;
                poly->g3 = 0;"""
    assert b1e_old in t and b2e_old in t
    t = t.replace(b1e_old, b1e_new).replace(b2e_old, b2e_new)
    if mode == 'col':
        b1i_old = """                SetSemiTrans(poly, 1);

                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r1 = pulse;
                poly->g1 = pulse;
                poly->b1 = pulse;
                poly->r2 = 0;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = 0;"""
        b2i_old = """                SetSemiTrans(poly, 1);

                poly->r0 = pulse;
                poly->g0 = pulse;
                poly->b0 = pulse;
                poly->r2 = pulse;
                poly->g2 = pulse;
                poly->b2 = pulse;
                poly->r1 = 0;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = 0;"""
        assert b1i_old in t and b2i_old in t
        ST = "SetSemiTrans(poly, 1);" + NL
        b1i_new = b1i_old.replace(ST, ST + "                col = 0;").replace("r2 = 0;", "r2 = col;").replace("r3 = 0;", "r3 = col;")
        b2i_new = b2i_old.replace(ST, ST + "                col = 0;").replace("r1 = 0;", "r1 = col;").replace("r3 = 0;", "r3 = col;")
        t = t.replace(b1i_old, b1i_new).replace(b2i_old, b2i_new)
    L[:] = t.split(NL)


def holders(L, tw):
    edit(L, 103, 's.x = 0;', 's.x = xpos;')
    edit(L, 104, 's.y = 0;', 's.y = ypos;')
    edit(L, 109, 's.semi = 0;', 's.semi = semi;')
    edit(L, 66, 's.x = 0;', 's.x = xpos;')
    edit(L, 65, 's.y = 0;', 's.y = ypos;')
    edit(L, 81, 's.y = 0;', 's.y = ypos;')
    edit(L, 68, 's.semi = 0;', 's.semi = semi;')
    if tw:
        edit(L, tw, 'mode), 0);', 'mode), tw);')


def build(name, colmode, hold, tw=None):
    L = list(L0)
    # edits from the bottom up keep line numbers valid
    if hold:
        holders(L, tw)
    if colmode:
        colour(L, colmode)
    t = '\n'.join(L)
    decl = ''
    init = ''
    if colmode:
        decl += '    s32 col;\n'
    if hold:
        decl += '    s16 xpos;\n    s16 ypos;\n    s16 semi;\n'
        init += '    xpos = 0;\n    ypos = 0;\n    semi = 0;\n'
        if tw:
            decl += '    s16 tw;\n'
            init += '    tw = 0;\n'
    t = t.replace('void func_8006C21C(s32 *arg0) {\n', 'void func_8006C21C(s32 *arg0) {\n' + decl, 1)
    t = t.replace('    s.ot_idx = 10;\n', init + '    s.ot_idx = 10;\n', 1)
    open(f'tmp/c21c/cmb/{name}.c', 'w', newline='\n').write(t)
    names.append(f'tmp/c21c/cmb/{name}.c')


build('dim', 'dim', False)
build('col', 'col', False)
build('hold3', None, True)
build('dim_hold3', 'dim', True)
build('col_hold3', 'col', True)
build('dim_hold3_tw3', 'dim', True, 99)
build('dim_hold3_tw5', 'dim', True, 130)
build('col_hold3_tw3', 'col', True, 99)
build('col_hold3_tw5', 'col', True, 130)
open('tmp/c21c/cmb/list.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))
