"""Ruling 11 necessity probes on the landing body F4 (work = V1 loop search angle, V2 post-loop target,
V3 final step)."""
import re
import sys
s = open(sys.argv[1], encoding='utf-8').read()
PFX = sys.argv[2]
I = s.index('    work = (pitch0 < pitch1) ? pitch1 : pitch0;\n')
F = s.index('    work = math_SignExt12Div(work - base_pitch, 8);\n')
head, mid, tail = s[:I], s[I:F], s[F:]

def w(n, t):
    open(f'tmp/func_8001A820/{PFX}{n}.c', 'w', encoding='utf-8', newline='\n').write(t)

def decl(t, names):
    return t.replace('    s32 work;\n', ''.join(f'    s32 {n};\n' for n in names), 1)

def rn(t, new):
    return re.sub(r'\bwork\b', new, t)

FIN3 = ('    step = math_SignExt12Div({v2} - base_pitch, 8);\n    cam->h10 += step;\n}}\n')
# full per-value: ang (V1) / tgt (V2) / step (V3)
pv = decl(head, ['ang', 'tgt', 'step'])
pv = rn(pv, 'ang').replace('    s32 ang;\n    s32 tgt;\n    s32 step;\n', '    s32 ang;\n    s32 tgt;\n    s32 step;\n')
pv = pv.replace('    s32 ang;\n    s32 ang;\n', '    s32 ang;\n')
G_pv = pv + rn(mid, 'tgt') + FIN3.format(v2='tgt')
w('Gpv', G_pv)
# family probes on the full per-value body
END = '    cam->h10 += step;\n}\n'
w('Gpv_ds', G_pv.replace(END, '    cam->h10 += step;\n    ang = 0;\n}\n'))
w('Gpv_sa', G_pv.replace(END, '    cam->h10 += step;\n    ang = ang;\n}\n'))
w('Gpv_cp', G_pv.replace(END, '    cam->h10 += step;\n    ang++;\n    ang--;\n}\n'))
w('Gpv_cpt', G_pv.replace('    tgt = (pitch0 < pitch1) ? pitch1 : pitch0;\n', '    tgt = (pitch0 < pitch1) ? pitch1 : pitch0;\n    tgt++;\n    tgt--;\n'))
a = G_pv.index('    tgt = (pitch0 < pitch1)'); b = G_pv.index('    cam->h10 = base_pitch;\n')
w('Gpv_dw', G_pv[:a] + '    do {\n' + G_pv[a:b] + '    } while (0);\n' + G_pv[b:])
# ablations: split exactly one value, the rest shared in work
V1 = decl(head, ['ang', 'work']); V1 = rn(V1, 'ang').replace('    s32 ang;\n    s32 ang;\n', '    s32 ang;\n    s32 work;\n')
w('G_split1', V1 + mid + tail)                                     # V1 alone
V2 = decl(head, ['work', 'tgt']) + rn(mid, 'tgt') + tail.replace('math_SignExt12Div(work - base_pitch', 'math_SignExt12Div(tgt - base_pitch')
w('G_split2', V2)                                                  # V2 alone
V3 = decl(head, ['work', 'step']) + mid + FIN3.format(v2='work')
w('G_split3', V3)                                                  # V3 alone
w('G_split3_sa', V3.replace(END, '    cam->h10 += step;\n    work = work;\n}\n'))
print('ok')
