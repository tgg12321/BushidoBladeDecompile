"""Spellings for the R11 (per-branch, in-arm) admission record of the colour local."""
import os
NL = chr(10)
src = open('memory/grind/func_8006C21C/candidate.c').read()
os.makedirs('tmp/c21c/red', exist_ok=True)

B1I = """                SetSemiTrans(poly, 1);

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
B1E = """                SetSemiTrans(poly, 0);

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
B2I = """                SetSemiTrans(poly, 1);

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
B2E = """                SetSemiTrans(poly, 0);

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
for b in (B1I, B1E, B2I, B2E):
    assert b in src


def arms(v_if, v_else, pre_if, pre_else):
    """v_*: expression text stored to the lower/right red channels; pre_*: statement put after SetSemiTrans."""
    b1i = B1I.replace('SetSemiTrans(poly, 1);' + NL, 'SetSemiTrans(poly, 1);' + NL + pre_if)
    b1i = b1i.replace('poly->r2 = 0;', f'poly->r2 = {v_if};').replace('poly->r3 = 0;', f'poly->r3 = {v_if};')
    b2i = B2I.replace('SetSemiTrans(poly, 1);' + NL, 'SetSemiTrans(poly, 1);' + NL + pre_if)
    b2i = b2i.replace('poly->r1 = 0;', f'poly->r1 = {v_if};').replace('poly->r3 = 0;', f'poly->r3 = {v_if};')
    b1e = B1E.replace('SetSemiTrans(poly, 0);' + NL, 'SetSemiTrans(poly, 0);' + NL + pre_else)
    b1e = b1e.replace('poly->r3 = poly->r2 = 0x80;', f'poly->r2 = {v_else};').replace(
        '                poly->b2 = 0;' + NL + '                poly->g3 = 0;',
        '                poly->b2 = 0;' + NL + f'                poly->r3 = {v_else};' + NL + '                poly->g3 = 0;')
    b2e = B2E.replace('SetSemiTrans(poly, 0);' + NL, 'SetSemiTrans(poly, 0);' + NL + pre_else)
    b2e = b2e.replace('poly->r3 = poly->r1 = 0x80;', f'poly->r1 = {v_else};').replace(
        '                poly->b1 = 0;' + NL + '                poly->g3 = 0;',
        '                poly->b1 = 0;' + NL + f'                poly->r3 = {v_else};' + NL + '                poly->g3 = 0;')
    return src.replace(B1I, b1i).replace(B1E, b1e).replace(B2I, b2i).replace(B2E, b2e)


def w(name, text, decl=''):
    if decl:
        text = text.replace('void func_8006C21C(s32 *arg0) {' + NL, 'void func_8006C21C(s32 *arg0) {' + NL + decl, 1)
    open(f'tmp/c21c/red/{name}.c', 'w', newline=NL).write(text)


# 1. the per-branch in-arm variable (the admitted form)
w('red_shared', arms('red', 'red', '                red = 0;' + NL, '                red = 0x80;' + NL), '    s32 red;' + NL)
# 2. one variable per arm (block-local, one write each)
t = arms('red', 'red', '                red = 0;' + NL, '                red = 0x80;' + NL)
t = t.replace('            if (i == 5) {' + NL + '                SetSemiTrans', '            if (i == 5) {' + NL + '                s32 red;' + NL + '                SetSemiTrans')
t = t.replace('            } else {' + NL + '                SetSemiTrans', '            } else {' + NL + '                s32 red;' + NL + '                SetSemiTrans')
assert t.count('s32 red;') == 4, t.count('s32 red;')
w('red_perarm', t)
# 3. literals in every arm, natural order
w('red_literal', arms('0', '0x80', '', ''))
# 4. u8 / s16 typed shared variable (type check)
w('red_shared_u8', arms('red', 'red', '                red = 0;' + NL, '                red = 0x80;' + NL), '    u8 red;' + NL)
print('ok')
