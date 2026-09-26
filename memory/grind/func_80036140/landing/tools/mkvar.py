"""Instantiate body_tpl.c into named E9C/EA4 variants under tmp/func_80036140/v/.
usage: python3 tmp/func_80036140/mkvar.py"""
from pathlib import Path
H = Path('tmp/func_80036140')
T = (H / 'body_tpl.c').read_text()
(H / 'v').mkdir(exist_ok=True)

REC = {'@E9C_POSTINC@': 'D_80101E58.rec.unk3C++', '@E9C@': 'D_80101E58.rec.unk3C',
       '@EA4_SUB4@': 'D_80101E58.rec.unk44 -= 4', '@EA4@': 'D_80101E58.rec.unk44'}
SPL = {'@E9C_POSTINC@': 'D_80101E9C++', '@E9C@': 'D_80101E9C',
       '@EA4_SUB4@': 'D_80101EA4 -= 4', '@EA4@': 'D_80101EA4'}


def inst(sub, lines=None):
    t = T
    for a, b in (lines or []):
        assert t.count(a) == 1, a
        t = t.replace(a, b)
    for k, v in sub.items():
        t = t.replace(k, v)
    assert '@' not in t
    return t


V = {}
V['rec'] = inst(REC)
V['split'] = inst(SPL)
# pointer read-modify-write locals at the two RMW sites only (pointer-rmw-global-sanctioned shape)
V['ptr_rmw'] = inst(SPL, [
    ('        if (@E9C_POSTINC@ > 0x3C) {\n            D_80101E58.rec.unk02 = 0x17;\n        }\n',
     '        {\n            s16 *p = &D_80101E9C;\n            if ((*p)++ > 0x3C) {\n                D_80101E58.rec.unk02 = 0x17;\n            }\n        }\n'),
    ('        if (@EA4@ != 0) {\n            @EA4_SUB4@;\n            if (@EA4@ <= 0) {\n',
     '        {\n        s32 *q = &D_80101EA4;\n        if (*q != 0) {\n            *q -= 4;\n            if (*q <= 0) {\n'),
    ('                break;\n            }\n        }\n        {\n            s32 ret = CdSync(1, &g_cd_result);\n            if (ret == 2) {\n                D_80101E58.rec.unk02 = 0x15;\n',
     '                break;\n            }\n        }\n        }\n        {\n            s32 ret = CdSync(1, &g_cd_result);\n            if (ret == 2) {\n                D_80101E58.rec.unk02 = 0x15;\n'),
])
# function-scope FAKE pointer aliases used at every E9C / EA4 access
fs = inst(SPL, [('    CdlATV atv;\n', '    CdlATV atv;\n    s16 *p = &D_80101E9C;\n    s32 *q = &D_80101EA4;\n')])
fs = fs.replace('D_80101E9C++', '(*p)++').replace('D_80101EA4 -= 4', '*q -= 4')
fs = fs.replace('    s16 *p = &D_80101E9C;', '    s16 *p = &@@9C;').replace('    s32 *q = &D_80101EA4;', '    s32 *q = &@@A4;')
fs = fs.replace('D_80101E9C', '*p').replace('D_80101EA4', '*q').replace('@@9C', 'D_80101E9C').replace('@@A4', 'D_80101EA4')
V['ptr_fs'] = fs
# E9C / EA4 as one-element arrays (flat-array spelling of a lone scalar)
V['arr1'] = inst({'@E9C_POSTINC@': 'D_80101E9C[0]++', '@E9C@': 'D_80101E9C[0]',
                  '@EA4_SUB4@': 'D_80101EA4[0] -= 4', '@EA4@': 'D_80101EA4[0]'})
for k, v in V.items():
    (H / 'v' / f'{k}.c').write_bytes(v.encode())
print(sorted(V))
