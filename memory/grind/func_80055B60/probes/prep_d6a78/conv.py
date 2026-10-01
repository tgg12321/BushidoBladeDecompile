# conv.py <in code6cac_b_tu2.c> <out>: respell the obj-walking D_80106A78 consumers onto Obj80106A78 members.
import re, sys
src = open(sys.argv[1]).read()

FIELD = {
    0x00: ('s16', 'unk_00'), 0x02: ('s16', 'unk_02'),
    0x04: ('u8', 'unk_04'), 0x05: ('u8', 'unk_05'), 0x06: ('u8', 'unk_06'), 0x07: ('u8', 'unk_07'),
    0x08: ('u8', 'unk_08'), 0x09: ('u8', 'unk_09'), 0x0A: ('u8', 'unk_0A'), 0x0B: ('u8', 'unk_0B'),
    0x2C: ('s32', 'unk_2C.x'), 0x30: ('s32', 'unk_2C.y'), 0x34: ('s32', 'unk_2C.z'),
    0x38: ('s32', 'unk_38.x'), 0x3C: ('s32', 'unk_38.y'), 0x40: ('s32', 'unk_38.z'),
    0x44: ('s32', 'unk_44.x'), 0x48: ('s32', 'unk_44.y'), 0x4C: ('s32', 'unk_44.z'),
    0x50: ('s32', 'unk_50'),
    0x54: ('s16', 'unk_54[0]'), 0x56: ('s16', 'unk_54[1]'), 0x58: ('s16', 'unk_54[2]'),
    0x5C: ('s16', 'unk_5C[0]'), 0x5E: ('s16', 'unk_5C[1]'), 0x60: ('s16', 'unk_5C[2]'),
}
ASSIGN = r'(?=\s*(=(?!=)|\+=|-=|\*=|/=|\|=|&=|\+\+|--))'

def conv_obj(text, var='obj'):
    # address-of uses first
    text = re.sub(r'(?<!\*)\(s32 \*\)\(%s \+ 0x(2C|44)\)' % var, lambda m: '&%s->unk_%s.x' % (var, m.group(1)), text)
    text = re.sub(r'\*\(Vec3i(?:32)? \*\)\(%s \+ 0x(2C|38)\)' % var, lambda m: '%s->unk_%s' % (var, m.group(1)), text)
    text = text.replace(', %s + 0x2C, ' % var, ', (u8 *)&%s->unk_2C, ' % var)
    # stores: member, no cast
    def st(m):
        return '%s->%s' % (var, FIELD[int(m.group(2), 0)][1])
    text = re.sub(r'\*\((s16|u16|s32|u8|s8) \*\)\(%s \+ (0x[0-9A-Fa-f]+|\d+)\)' % var + ASSIGN, st, text)
    # loads: member, cast when the access width/sign differs
    def ld(m):
        ty, off = m.group(1), int(m.group(2), 0)
        fty, f = FIELD[off]
        e = '%s->%s' % (var, f)
        return e if ty == fty else '(%s)%s' % (ty, e)
    text = re.sub(r'\*\((s16|u16|s32|u8|s8) \*\)\(%s \+ (0x[0-9A-Fa-f]+|\d+)\)' % var, ld, text)
    text = re.sub(r'\(\*\(s16 \*\)%s\)\+\+' % var, '%s->unk_00++' % var, text)
    text = re.sub(r'\*\(s16 \*\)%s\b' % var, '%s->unk_00' % var, text)
    text = re.sub(r'\b%s\[(\d+)\]' % var, lambda m: '%s->%s' % (var, FIELD[int(m.group(1))][1]), text)
    text = text.replace('u8 *%s;' % var, 'Obj80106A78 *%s;' % var)
    text = text.replace('%s = (u8 *)&D_80106A78;' % var, '%s = D_80106A78;' % var)
    text = text.replace('%s += 0x64' % var, '%s++' % var)
    return text

def span(t, fn):
    m = re.search(r'\n[a-zA-Z0-9_ *]+\b%s\([^)]*\) \{\n' % fn, t)
    s = m.start() + 1
    depth, i = 0, t.index('{', s)
    while True:
        if t[i] == '{':
            depth += 1
        elif t[i] == '}':
            depth -= 1
            if depth == 0:
                return s, i + 2
        i += 1

out = src
for fn in ('func_80030580', 'func_80030D7C', 'func_80031B24'):
    s, e = span(out, fn)
    out = out[:s] + conv_obj(out[s:e]) + out[e:]
open(sys.argv[2], 'w', newline='\n').write(out)
print('ok')
