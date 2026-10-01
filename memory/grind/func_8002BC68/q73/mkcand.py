"""Generate candidate bodies (whole function definitions) for sandbox --candidate.
usage: python tmp/laneH/mkcand.py   -> writes tmp/laneH/cand/<func>.<variant>.c and prints the list."""
import sys
from pathlib import Path
sys.path.insert(0, 'tmp/laneH')
from edits import _span

OUT = Path('tmp/laneH/cand')
OUT.mkdir(exist_ok=True)


def body(path, func):
    t = open(path, encoding='utf-8', newline='').read()
    a, b = _span(t, func)
    return t[a:b]


def sub(s, pairs):
    for old, new in pairs:
        assert s.count(old) >= 1, (old, s.count(old))
        s = s.replace(old, new)
    return s


V = {}
B = 'src/code6cac_b_tu2.c'
T = 'src/code6cac_tu2.c'
C = 'src/code6cac_c2.c'

STORES = [('    u8 *t2_base;\n    u8 *t3_base;\n', '    PracticeMenuRec *t2_base;\n    PracticeMenuRec *t3_base;\n'),
          ('    t2_base = &D_80101EC8;\n    t3_base = t2_base + 0x44C;\n', '    t2_base = g_practice_menu_table;\n    t3_base = t2_base + 1;\n'),
          ('*((s32 *) (t2_base + 0x134))', 't2_base->unk_134.vx'), ('*((s32 *) (t2_base + 0x13C))', 't2_base->unk_134.vz'),
          ('*((s32 *) (t3_base + 0x134))', 't3_base->unk_134.vx'), ('*((s32 *) (t3_base + 0x13C))', 't3_base->unk_134.vz')]
for func, f, (xa, za, xb, zb) in (('func_8002BC68', 'unk_D8', ('D_80101FA0', 'D_80101FA8', 'D_801023EC', 'D_801023F4')),
                                  ('func_8002BEA0', 'unk_F4', ('D_80101FBC', 'D_80101FC4', 'D_80102408', 'D_80102410'))):
    b = body(B, func)
    reads = lambda ra, rb, rc, rd: [(f'temp_a3 = {xa} - {xb};', f'temp_a3 = {ra} - {rc};'),
                                    (f'temp_t1 = {za} - {zb};', f'temp_t1 = {rb} - {rd};')]
    g = 'g_practice_menu_table'
    # Q73 form: per-word reads, struct stores/base
    V[f'{func}.q73'] = sub(b, STORES)
    # single-object alternatives
    V[f'{func}.s1_member_reads'] = sub(b, STORES + reads(f'{g}[0].{f}.x', f'{g}[0].{f}.z', f'{g}[1].{f}.x', f'{g}[1].{f}.z'))
    A0 = '    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);\n'
    V[f'{func}.s2_handle_reads'] = sub(b, STORES + [(f'    temp_a3 = {xa} - {xb};\n    temp_t1 = {za} - {zb};\n' + A0, '')] +
                                       [('    t3_base = t2_base + 1;\n',
                                         f'    t3_base = t2_base + 1;\n    temp_a3 = t2_base->{f}.x - t3_base->{f}.x;\n'
                                         f'    temp_t1 = t2_base->{f}.z - t3_base->{f}.z;\n' + A0)])
    V[f'{func}.s3_handles_first'] = sub(b, STORES + [(f'    temp_a3 = {xa} - {xb};\n    temp_t1 = {za} - {zb};\n' + A0, ''),
                                                     ('    t3_base = t2_base + 1;\n',
                                                      f'    t3_base = t2_base + 1;\n    temp_a3 = {g}[0].{f}.x - {g}[1].{f}.x;\n'
                                                      f'    temp_t1 = {g}[0].{f}.z - {g}[1].{f}.z;\n' + A0)])
    V[f'{func}.s4_direct_all'] = sub(b, reads(f'{g}[0].{f}.x', f'{g}[0].{f}.z', f'{g}[1].{f}.x', f'{g}[1].{f}.z') +
                                     [('    u8 *t2_base;\n    u8 *t3_base;\n', ''),
                                      ('    t2_base = &D_80101EC8;\n    t3_base = t2_base + 0x44C;\n', ''),
                                      ('*((s32 *) (t2_base + 0x134))', f'{g}[0].unk_134.vx'), ('*((s32 *) (t2_base + 0x13C))', f'{g}[0].unk_134.vz'),
                                      ('*((s32 *) (t3_base + 0x134))', f'{g}[1].unk_134.vx'), ('*((s32 *) (t3_base + 0x13C))', f'{g}[1].unk_134.vz')])
    V[f'{func}.s5_vec_ptrs'] = sub(b, STORES + [(f'    temp_a3 = {xa} - {xb};\n    temp_t1 = {za} - {zb};\n',
                                                 f'    {{\n        Vec3i32 *va = &{g}[0].{f};\n        Vec3i32 *vb = &{g}[1].{f};\n'
                                                 f'        temp_a3 = va->x - vb->x;\n        temp_t1 = va->z - vb->z;\n    }}\n')])

# other readers of the eight names (Q73 (3)): through the struct
g = 'g_practice_menu_table'
V['func_8001C8DC.r'] = sub(body(T, 'func_8001C8DC'), [('func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 0x44C))',
                                                     f'func_80022408(&{g}[D_800A3748].unk_F4.x)')])
V['func_8001EA84.r'] = sub(body(T, 'func_8001EA84'), [('func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 0x44C))',
                                                     f'func_80022408(&{g}[D_800A3748].unk_F4.x)')])
V['func_8003C9A4.r'] = sub(body(C, 'func_8003C9A4'), [('func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 1100))',
                                                     f'func_80022408(&{g}[D_800A3748].unk_F4.x)')])
V['func_8001E878.r'] = sub(body(T, 'func_8001E878'), [('func_8003E6A0(D_80101FBC, D_80101FC4);', f'func_8003E6A0({g}[0].unk_F4.x, {g}[0].unk_F4.z);'),
                                                     ('func_8003E6A0(D_80102408, D_80102410);', f'func_8003E6A0({g}[1].unk_F4.x, {g}[1].unk_F4.z);')])
V['func_8001F888.r'] = sub(body(T, 'func_8001F888'), [('s32 dx = D_80102408 - D_80101FBC;', f's32 dx = {g}[1].unk_F4.x - {g}[0].unk_F4.x;'),
                                                     ('s32 dy = D_80102410 - D_80101FC4;', f's32 dy = {g}[1].unk_F4.z - {g}[0].unk_F4.z;')])
ce = body(C, 'func_8003CE18')
V['func_8003CE18.r1'] = sub(ce, [('s32 *addr = (s32 *)&D_80101FBC;', f's32 *addr = &{g}[0].unk_F4.x;'),
                                 ('addr = (s32 *)((u8 *)addr + 0x44C);', f'addr = &{g}[1].unk_F4.x;')])
V['func_8003CE18.r2'] = sub(ce, [('        s32 *addr = (s32 *)&D_80101FBC;\n', f'        PracticeMenuRec *rec = {g};\n'),
                                 ('            addr = (s32 *)((u8 *)addr + 0x44C);\n', '            rec++;\n'),
                                 ('result = func_80022408(addr);', 'result = func_80022408(&rec->unk_F4.x);')])
V['func_8003CE18.r3'] = sub(ce, [('        s32 *addr = (s32 *)&D_80101FBC;\n', f'        PracticeMenuRec *rec = &{g}[0];\n'),
                                 ('            addr = (s32 *)((u8 *)addr + 0x44C);\n', f'            rec = &{g}[1];\n'),
                                 ('result = func_80022408(addr);', 'result = func_80022408(&rec->unk_F4.x);')])
for k, v in V.items():
    open(OUT / f'{k}.c', 'w', encoding='utf-8', newline='').write(v)
    print(k)
