"""Ruling 11 (D)(4) variants for func_8002A458, generated from final_score.c.
Each variant splits some values of the reused variables into their own fresh locals,
declared at the innermost scope enclosing that value's writes, with the SAME statements.
Writes tmp/func_8002A458/r11v/<name>.c and r11v/list.txt."""
import os

D = os.path.dirname(os.path.abspath(__file__))
BASE = open(os.path.join(D, 'final_score.c'), encoding='utf-8').read()
OUT = os.path.join(D, 'r11v')
os.makedirs(OUT, exist_ok=True)

HIT = {'x': ('0x100', 0), 'y': ('0x104', 1), 'z': ('0x108', 2)}
OBJ = {'x': ('0xF4', 0), 'y': ('0xF8', 1), 'z': ('0xFC', 2)}


def split_dxyz(s, hit_vars='', obj_vars=''):
    """hit_vars / obj_vars: which of 'xyz' get their own local for the hit / obj value."""
    hdecl, odecl = [], []
    for c in hit_vars:
        off, i = HIT[c]
        old = '            d%s = *(s32 *)(scr + %s) - p[%d];\n' % (c, off, i)
        assert s.count(old) == 1, old
        s = s.replace(old, '            h%s = *(s32 *)(scr + %s) - p[%d];\n' % (c, off, i))
        s = s.replace('            hit_sq = ', '            hit_sq = ', 1)
        line_start = s.index('            hit_sq = ')
        line_end = s.index('\n', line_start)
        line = s[line_start:line_end].replace('d%s * d%s' % (c, c), 'h%s * h%s' % (c, c))
        s = s[:line_start] + line + s[line_end:]
        hdecl.append('h' + c)
    for c in obj_vars:
        off, i = OBJ[c]
        old = '            d%s = *(s32 *)(obj + %s) - p[%d];\n' % (c, off, i)
        assert s.count(old) == 1, old
        s = s.replace(old, '            o%s = *(s32 *)(obj + %s) - p[%d];\n' % (c, off, i))
        line_start = s.index('            if (hit_sq >= ')
        line_end = s.index('\n', line_start)
        line = s[line_start:line_end].replace('d%s * d%s' % (c, c), 'o%s * o%s' % (c, c))
        s = s[:line_start] + line + s[line_end:]
        odecl.append('o' + c)
    if hdecl or odecl:
        anchor = '        if (*hit != 0) {\n'
        decl = ''.join('            s32 %s;\n' % v for v in hdecl + odecl) + '\n'
        s = s.replace(anchor, anchor + decl, 1)
    return s


def split_temp(s, sum_own=False, r1_own=False, r2_own=False):
    a = s.index('    temp = dx * dx + dz * dz - dy * dy;')
    b = s.index('    *(s16 *)(scr + 0xFA) = ratan2(dx, dz);')
    seg1 = s[a:b]
    c = s.index('    len_sq = dx * dx + dy * dy + dz * dz;')
    e = s.index('    **(struct vec3w **)(scr + 0x60)')
    seg2 = s[c:e]
    decl = ''
    if sum_own:
        seg1 = seg1.replace('    temp = dx * dx + dz * dz - dy * dy;', '    h_sq = dx * dx + dz * dz - dy * dy;')
        seg1 = seg1.replace('if (temp < 0)', 'if (h_sq < 0)').replace('printf(D_80010478, temp)', 'printf(D_80010478, h_sq)')
        seg1 = seg1.replace('    temp2 = temp;', '    temp2 = h_sq;')
        seg1 = seg1.replace('if ((u32)temp < 0x400)', 'if ((u32)h_sq < 0x400)')
        seg1 = seg1.replace('(((u8 *)&D_8008D118) + temp) >> 3', '(((u8 *)&D_8008D118) + h_sq) >> 3')
        seg1 = seg1.replace('((u32)temp >> shift)', '((u32)h_sq >> shift)')
        decl += '    s32 h_sq;\n'
    if r1_own:
        seg1 = seg1.replace('        temp = (u32)*(((u8', '        hlen = (u32)*(((u8')
        seg1 = seg1.replace('            temp = (u32)(temp2 << 16)', '            hlen = (u32)(temp2 << 16)')
        seg1 = seg1.replace('-ratan2(dy, temp)', '-ratan2(dy, hlen)')
        if '    s32 hlen;' not in s:
            decl += '    s32 hlen;\n'
    if r2_own:
        seg2 = seg2.replace('        temp = (u32)*(((u8', '        len = (u32)*(((u8')
        seg2 = seg2.replace('            temp = (u32)(tbl << 16)', '            len = (u32)(tbl << 16)')
        seg2 = seg2.replace('/ temp;', '/ len;')
        decl += '    s32 len;\n'
    s = s[:a] + seg1 + s[b:c] + seg2 + s[e:]
    s = s.replace('    s32 temp2;\n', '    s32 temp2;\n' + decl, 1)
    return s


def split_temp2(s):
    """copy in its own function-scope local `n`, table byte in a block-scope `tbl`."""
    s = s.replace('    temp2 = temp;\n', '    n = temp;\n', 1)
    s = s.replace('    temp2 = h_sq;\n', '    n = h_sq;\n', 1)
    s = s.replace('"r"(temp2)', '"r"(n)', 1)
    s = s.replace('            s32 shift;\n            lz &= sp_tmp;\n',
                  '            s32 shift;\n            s32 tbl;\n            lz &= sp_tmp;\n', 1)
    s = s.replace('            temp2 = *(((u8 *)&D_8008D118)', '            tbl = *(((u8 *)&D_8008D118)', 1)
    s = s.replace('(u32)(temp2 << 16)', '(u32)(tbl << 16)', 1)
    s = s.replace('    s32 temp2;\n', '    s32 n;\n', 1)
    import re
    body = re.sub(r'/\*.*?\*/', '', s.split('void func_8002A458')[1], flags=re.S)
    assert 'temp2' not in body, 'temp2 left'
    return s


V = {}
V['final'] = BASE
# full one-variable-per-value spelling
full = split_dxyz(BASE, 'xyz', 'xyz')
full = split_temp(full, True, True, True)
full = split_temp2(full)
V['onevar_full'] = full
# dx/dy/dz value ablations (applied to all three variables)
V['dxyz_hit_own'] = split_dxyz(BASE, 'xyz', '')
V['dxyz_obj_own'] = split_dxyz(BASE, '', 'xyz')
V['dxyz_hitobj_shared_trio'] = split_dxyz(BASE, 'xyz', 'xyz').replace('            ox = ', '            hx = ').replace(
    '            oy = ', '            hy = ').replace('            oz = ', '            hz = ').replace(
    'ox * ox + oy * oy + oz * oz', 'hx * hx + hy * hy + hz * hz').replace(
    '            s32 ox;\n            s32 oy;\n            s32 oz;\n', '')
V['dxyz_all_own'] = split_dxyz(BASE, 'xyz', 'xyz')
# per-variable splits (one of dx/dy/dz fully split, the other two reused)
for c in 'xyz':
    V['d%s_only_split' % c] = split_dxyz(BASE, c, c)
# temp ablations
V['temp_sum_own'] = split_temp(BASE, sum_own=True)
V['temp_r1_own'] = split_temp(BASE, r1_own=True)
V['temp_r2_own'] = split_temp(BASE, r2_own=True)
V['temp_all_own'] = split_temp(BASE, True, True, True)
# temp2 split
V['temp2_split'] = split_temp2(BASE)
names = []
for k, v in V.items():
    p = os.path.join(OUT, k + '.c')
    open(p, 'w', encoding='utf-8', newline='\n').write(v)
    names.append('tmp/func_8002A458/r11v/%s.c' % k)
open(os.path.join(OUT, 'list.txt'), 'w').write(','.join(names))
print(len(names))
