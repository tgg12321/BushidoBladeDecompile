"""Generate the Q21 (1) variant TUs.

Inputs: the two split TUs as tusplit.py produces them from src/text1b.c at
388f1b209 (tmp/func_800720FC/q21tu/text1b.at_split.c, text1b_mid.at_split.c:
`python3 memory/grind/func_800720FC/split/tusplit.py func_8006F97C <pre> <post> <report>`).

Covered files and their functions:
  text1b_mid.c  func_800720FC (landing body, memory/grind/func_800720FC/timers/landing_body_scalars.c)
  text1b.c      func_8006F100 (committed body)
Every variant is written to tmp/func_800720FC/q21tu/v/<name>.c.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, '.')
from engine import inlineasm  # noqa: E402

T = Path('tmp/func_800720FC/q21tu')
V = T / 'v'
V.mkdir(exist_ok=True)
pre = (T / 'text1b.at_split.c').read_text(encoding='utf-8')
post = (T / 'text1b_mid.at_split.c').read_text(encoding='utf-8')
body = Path('memory/grind/func_800720FC/timers/landing_body_scalars.c').read_text(encoding='utf-8')
import subprocess as _sp
ORIG = _sp.run(['git', 'show', '388f1b209:src/text1b.c'], capture_output=True, text=True, check=True).stdout

# The landing body declares the two scalars in front of the function.
SCALARS = "extern s16 D_800A35C8;\nextern s16 D_800A35CA;\n"
assert SCALARS in body
STORES = "                D_800A35C8 = 0xF;\n                D_800A35CA = 0x14;\n"
assert STORES in body


def mid(decl, stores, retype=True):
    """text1b_mid.c as the landing would leave it: the original text1b.c with the
    variant body in place of func_800720FC's INCLUDE_ASM, split by tusplit.py (so
    the head block carries every file-scope declaration the moved code, now
    including func_800720FC, names), minus text1b.c's own D_800A35C8 array
    declaration (the variant body declares D_800A35C8 its own way)."""
    import subprocess
    b = body.replace(SCALARS, decl).replace(STORES, stores)
    orig = ORIG.replace('extern s16 D_800A35C8[];\n', '')
    assert orig != ORIG
    src = inlineasm.substitute_body(orig, 'func_800720FC', b)
    if retype:
        src = src.replace('extern u16 D_800A3578;', 'extern s16 D_800A3578;')
    tmp_src = T / '_src.c'
    tmp_src.write_bytes(src.encode('utf-8'))
    subprocess.run([sys.executable, 'memory/grind/func_800720FC/split/tusplit.py', 'func_8006F97C',
                    str(T / '_pre.c'), str(T / '_post.c'), str(T / '_report.txt'), str(tmp_src)],
                   check=True, capture_output=True)
    return (T / '_post.c').read_text(encoding='utf-8')


def w(name, text):
    (V / f'{name}.c').write_bytes(text.encode('utf-8'))


I16 = " " * 16
# ---- text1b_mid.c (func_800720FC) under each single declaration ----
w('mid_R_scalars', mid(SCALARS, STORES))                                   # reference (landing form)
ARR = "extern s16 D_800A35C8[];\n"
w('mid_A1_array_0_1', mid(ARR, f"{I16}D_800A35C8[0] = 0xF;\n{I16}D_800A35C8[1] = 0x14;\n"))
w('mid_A2_array_1_0', mid(ARR, f"{I16}D_800A35C8[1] = 0x14;\n{I16}D_800A35C8[0] = 0xF;\n"))
w('mid_A3_array_ptrlocal', mid(ARR, f"{I16}{{\n{I16}    s16 *timer = D_800A35C8;\n\n"
                                    f"{I16}    timer[0] = 0xF;\n{I16}    timer[1] = 0x14;\n{I16}}}\n"))
w('mid_A4_array_deref', mid(ARR, f"{I16}*D_800A35C8 = 0xF;\n{I16}*(D_800A35C8 + 1) = 0x14;\n"))
w('mid_A5_sized_array', mid("extern s16 D_800A35C8[2];\n",
                            f"{I16}D_800A35C8[0] = 0xF;\n{I16}D_800A35C8[1] = 0x14;\n"))
STRUCT = "typedef struct { s16 p1; s16 p2; } Timers720FC;\nextern Timers720FC D_800A35C8;\n"
w('mid_S1_struct', mid(STRUCT, f"{I16}D_800A35C8.p1 = 0xF;\n{I16}D_800A35C8.p2 = 0x14;\n"))
w('mid_S2_struct_2_1', mid(STRUCT, f"{I16}D_800A35C8.p2 = 0x14;\n{I16}D_800A35C8.p1 = 0xF;\n"))

# ---- text1b.c (func_8006F100) under each single declaration ----
w('pre_R_array', pre)                                                      # reference (committed)
F100_USES = ['D_800A35C8[i]']
assert pre.count('extern s16 D_800A35C8[];') == 1


def pre_with(decl, sel):
    t = pre.replace('extern s16 D_800A35C8[];', decl)
    i = t.index('void func_8006F100(s32 arg0) {')
    j = t.index('D_800A355C++;\n}', i)
    seg = t[i:j].replace('    s16 dy;\n', '    s16 dy;\n    s16 *timer;\n', 1)
    seg = seg.replace('        s.ent = obj->ent;\n', '        s.ent = obj->ent;\n        timer = %s;\n' % sel, 1)
    seg = seg.replace('D_800A35C8[i]', '*timer')
    return t[:i] + seg + t[j:]


w('pre_S1_scalars_ptr', pre_with(SCALARS, 'i != 0 ? &D_800A35CA : &D_800A35C8'))
w('pre_S2_struct_ptr', pre_with(STRUCT, 'i != 0 ? &D_800A35C8.p2 : &D_800A35C8.p1'))
print(sorted(p.name for p in V.iterdir()))
