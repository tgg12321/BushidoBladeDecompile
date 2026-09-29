"""Landing-time source replacements for func_80070F78 (applied to a COPY of src/text1b.c).

- the func_80070F78 prototype takes the draw descriptor (DescF97C *) instead of s32 *;
- func_80070C70 declares its descriptor as DescF97C (same 0x2C-byte layout as its
  former private PrimC70 typedef, fields renamed to DescF97C's names), so it passes
  &prim with no cast; PrimC70 is deleted (no other user).
"""
import re

PROTO_OLD = 'extern void func_80070F78(s32 a0, s32 *prim);'
PROTO_NEW = 'extern void func_80070F78(s32 a0, DescF97C *s);'

PRIM_TYPEDEF = '''typedef struct PrimC70 {
    s32 p_geom;
    s32 p_static;
    s32 link;
    s32 pad0C;
    s32 zero10;
    s32 code;
    s32 mode;
    s32 zero1C;
    s32 width;
    s32 height;
    u8  byte28;
} PrimC70;

'''

FIELDS = [('p_geom', 'header'), ('p_static', 'table'), ('link', 'out'), ('zero10', 'semi'),
          ('code', 'ot_idx'), ('mode', 'x'), ('zero1C', 'y'), ('width', 'scale_x'),
          ('height', 'scale_y'), ('byte28', 'has_color')]

C70_HEAD = 'void func_80070C70(s32 arg0) {'
C70_TAIL = '    func_80073200(arg0);\n}\n'


def c70(txt):
    a = txt.index(C70_HEAD)
    b = txt.index(C70_TAIL, a) + len(C70_TAIL)
    body = txt[a:b]
    assert body.count('    PrimC70 prim;\n') == 1
    body = body.replace('    PrimC70 prim;\n', '    DescF97C prim;\n')
    for old, new in FIELDS:
        body = re.sub(r'\bprim\.' + old + r'\b', 'prim.' + new, body)
    assert body.count('func_80070F78(arg0, (s32 *)&prim);') == 1
    body = body.replace('func_80070F78(arg0, (s32 *)&prim);', 'func_80070F78(arg0, &prim);')
    return txt[:a] + body + txt[b:]


def apply(txt):
    assert txt.count(PROTO_OLD) == 1
    txt = txt.replace(PROTO_OLD, PROTO_NEW)
    assert txt.count(PRIM_TYPEDEF) == 1
    txt = txt.replace(PRIM_TYPEDEF, '')
    return c70(txt)
