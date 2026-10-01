# manual.py <conv-output> <out>: hand respellings of the remaining D_80106A78 consumers.
import re, sys
t = open(sys.argv[1]).read()

def one(old, new):
    global t
    assert t.count(old) == 1, (old[:70], t.count(old))
    t = t.replace(old, new)

def func_span(fn):
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

def replace_func(fn, body):
    global t
    s, e = func_span(fn)
    t = t[:s] + body + t[e:]

one("extern u8 D_80106A78;\n", "")
# func_80030580 returns the record
one("s32 *func_80030580(s32 *arg0, s32 arg1) {", "Obj80106A78 *func_80030580(s32 *arg0, s32 arg1) {")
one("    return (s32 *)obj;\n}", "    return obj;\n}")
assert t.count("extern s32 *func_80030580(s32 *, s32);") == 2
t = t.replace("extern s32 *func_80030580(s32 *, s32);", "extern Obj80106A78 *func_80030580(s32 *, s32);")
# func_800307D0
s, e = func_span('func_800307D0')
b = t[s:e]
b = b.replace("    s32 *obj;\n", "    Obj80106A78 *obj;\n")
b = b.replace("kind = *(s16 *)((u8 *)obj + 2);", "kind = obj->unk_02;")
b = b.replace("(u8 *)obj + 0x2C", "(u8 *)&obj->unk_2C")
t = t[:s] + b + t[e:]
# func_80030900 / cpu_set_move_command_and_dir
for fn in ('func_80030900', 'cpu_set_move_command_and_dir'):
    s, e = func_span(fn)
    b = t[s:e]
    b = b.replace("    s32 *p;\n", "    Obj80106A78 *p;\n")
    b = b.replace("*((u8 *)p + 4) = 0;", "p->unk_04 = 0;")
    b = b.replace("*(Vec3_copy *)((u8 *)p + 0x2C) = *(Vec3_copy *)", "p->unk_2C = *(Vec3i32 *)")
    b = b.replace("*(s32 *)((u8 *)p + 0x44)", "p->unk_44.x").replace("*(s32 *)((u8 *)p + 0x48)", "p->unk_44.y")
    b = b.replace("*(s32 *)((u8 *)p + 0x4C)", "p->unk_44.z")
    b = b.replace("*(s16 *)((u8 *)p + 0x5C)", "p->unk_5C[0]").replace("*(s16 *)((u8 *)p + 0x5E)", "p->unk_5C[1]")
    b = b.replace("*(s16 *)((u8 *)p + 0x60)", "p->unk_5C[2]")
    b = b.replace("*((u8 *)p + 7) = 1;", "p->unk_07 = 1;").replace("*((u8 *)p + 0xB) = ", "p->unk_0B = ")
    assert '(u8 *)p' not in b, fn
    t = t[:s] + b + t[e:]
# func_80031B24
one("    Vec3i *seg = (Vec3i *)scr;", "    Vec3i32 *seg = (Vec3i32 *)scr;")
one("        *(Vec3i **)(scr + 0x60) = &seg[0];\n        *(Vec3i **)(scr + 0x64) = &seg[1];",
    "        *(Vec3i32 **)(scr + 0x60) = &seg[0];\n        *(Vec3i32 **)(scr + 0x64) = &seg[1];")
one("            func_8002FF20(obj, *(s16 *)(rec + 2));", "            func_8002FF20((u8 *)obj, *(s16 *)(rec + 2));")
one("        func_80031890(scr, obj, j);", "        func_80031890(scr, (u8 *)obj, j);")

replace_func('func_80030208', open('tmp/d6a78/f_80030208.c').read())
replace_func('func_8003043C', open('tmp/d6a78/f_8003043C.c').read())
replace_func('func_80030524', open('tmp/d6a78/f_80030524.c').read())
replace_func('func_80030BA8', open('tmp/d6a78/f_80030BA8.c').read())
replace_func('func_80030D04', open('tmp/d6a78/f_80030D04.c').read())
open(sys.argv[2], 'w', newline='\n').write(t)
print('ok')
