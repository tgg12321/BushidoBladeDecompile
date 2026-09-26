"""Minimisation of h2_ba00_top_r1 (0/501): which aliases/extenders are load-bearing."""
D = 'tmp/func_800620B8/s4/'
b = open(D + 'h2_ba00_top_r1.c', encoding='utf-8').read()


def sub(s, a, c):
    assert s.count(a) == 1, a
    return s.replace(a, c)


def drop_alias(s, name, sym, decl):
    s = sub(s, '    %s = %s;\n' % (name, sym), '')
    s = sub(s, decl, '')
    return s.replace('(s32)%s;' % name, '(s32)%s;' % sym).replace('(s32)%s[' % name, '(s32)%s[' % sym).replace(' - (s32)%s + (s32)%s ' % (name, name), ' - (s32)%s + (s32)%s ' % (sym, sym))


out = {}
out['m_alt32'] = drop_alias(b, 'alt32', 'D_8009BA50', '    u16 *alt32;\n')
out['m_alt16'] = drop_alias(b, 'alt16', 'D_8009BA58', '    u16 *alt16;\n')
out['m_alts'] = drop_alias(out['m_alt32'], 'alt16', 'D_8009BA58', '    u16 *alt16;\n')
out['m_strip16'] = drop_alias(b, 'strip16', 'D_8009BA30', '    u16 (*strip16)[4];\n')
out['m_no_x16'] = sub(b, '(s32)strip16[D_800A32B8 & 3] - (s32)strip16 + (s32)strip16 /* FAKE */', '(s32)strip16[D_800A32B8 & 3]')
out['m_no_x32'] = sub(b, '(s32)strip32[(u32)D_800A32B8 % 6] - (s32)strip32 + (s32)strip32 /* FAKE */', '(s32)strip32[(u32)D_800A32B8 % 6]')
for k, v in out.items():
    assert 'alt32' not in v or k not in ('m_alt32', 'm_alts')
    open(D + k + '.c', 'w', encoding='utf-8', newline='\n').write(v)
open(D + 'list4.txt', 'w').write(','.join(D + k + '.c' for k in out))
