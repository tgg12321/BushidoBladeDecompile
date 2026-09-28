"""Sanctioned-family probes on the one-variable-per-value body (r11/pv.c).
Usage: python3 memory/grind/func_800290B8/fam/mkfam.py  (writes fam/*.c next to this script)."""
import os, sys
here = os.path.dirname(os.path.abspath(__file__))
pv = open(os.path.join(here, '..', 'r11', 'pv.c')).read()

def sub(src, old, new, n=1):
    assert src.count(old) == n, (old, src.count(old))
    return src.replace(old, new)

CALL = '                func_80044B30(mark, func_8002FC80((VECTOR *)a, (VECTOR *)b, (VECTOR *)c));\n'
V = {}
# do-while(0) around the func_80044B30 call (its mark read goes to loop depth 4)
V['dw_call'] = sub(pv, CALL, '                do { /* FAKE */\n    ' + CALL + '                } while (0);\n')
# do-while(0) (single and nested) around the whole marker loop
ML0 = '    e = (Marker_290B8 *)func_8004678C();\n    for (mark = 0;'
V['dw_mloop'] = sub(sub(pv, ML0, '    e = (Marker_290B8 *)func_8004678C();\n    do { /* FAKE */\n    for (mark = 0;'),
                    '    }\n    return 0;\n}', '    }\n    } while (0);\n    return 0;\n}')
# do-while(0) around the marker-loop body's triangle loop
TL0 = '        for (tri = 0; tri < 2; tri++) {'
TL1 = '        }\n        if (swap == 0 || side == 0) continue;'
V['dw_tloop'] = sub(sub(pv, TL0, '        do { /* FAKE */\n' + TL0), TL1, '        }\n        } while (0);\n        if (swap == 0 || side == 0) continue;')
# nested do-while(0) x2 around the call
V['dw2_call'] = sub(pv, CALL, '                do { do { /* FAKE */\n    ' + CALL + '                } while (0); } while (0);\n')
# init inside a do-while(0)
V['dw_init'] = sub(sub(pv, '    for (mark = 0; e->type != 0;', '    do { mark = 0; } while (0); /* FAKE */\n    for (; e->type != 0;'), 'x', 'x', pv.count('x'))
# col / vtx statements wrapped
V['dw_col'] = sub(pv, '        s32 vtx = side * 4 + row * 2 + col;\n',
                  '        s32 vtx;\n        do { vtx = side * 4 + row * 2 + col; } while (0); /* FAKE */\n')
# loop-1 body wrapped whole
V['dw_l1'] = sub(sub(pv, '        s32 vtx = side * 4 + row * 2 + col;\n', '        s32 vtx = side * 4 + row * 2 + col;\n        do { /* FAKE */\n'),
                 '            *(s32 *)(scr + 0x88) = quads[vtx].y;\n        }\n    }\n',
                 '            *(s32 *)(scr + 0x88) = quads[vtx].y;\n        }\n        } while (0);\n    }\n')
# chain-extender on the step / the argument
V['ce_step'] = sub(pv, 'mark++, e++)', 'mark = mark + 2 - 1, e++)')
V['ce_arg'] = sub(pv, 'func_80044B30(mark, ', 'func_80044B30(mark + 1 - 1, ')
# step duplicated into every continue arm (while loop), cross-jump re-merges tails
d = sub(pv, '    for (mark = 0; e->type != 0; mark++, e++) {', '    mark = 0;\n    while (e->type != 0) {')
d = d.replace(' continue;\n        if', ' { mark++; e++; continue; }\n        if')
d = sub(d, '        if (*(s32 *)(scr + 0x8C) < e->z) continue;', '        if (*(s32 *)(scr + 0x8C) < e->z) { mark++; e++; continue; }')
d = sub(d, '        if (swap == 0 || side == 0) continue;', '        if (swap == 0 || side == 0) { mark++; e++; continue; }')
V['dup_step'] = d
# pointer alias of the index
V['pa'] = sub(sub(pv, '    s32 mark;\n', '    s32 mark;\n    s32 *pmark = &mark; /* FAKE */\n'), 'func_80044B30(mark, ', 'func_80044B30(*pmark, ')
for k, s in V.items():
    open(os.path.join(here, k + '.c'), 'w', newline='\n').write(s)
    print(k)

# --- the init wrap (dw_init) combined with the ablations and with loop-1 wraps
INIT = '    for (tmp_a = 0; e->type != 0;'
INITW = '    do { tmp_a = 0; } while (0); /* FAKE */\n    for (; e->type != 0;'
W = {}
W['dwi_abl_a'] = sub(open(os.path.join(here, '..', 'r11', 'abl_a.c')).read(), INIT, INITW)
W['dwi_abl_b'] = sub(open(os.path.join(here, '..', 'r11', 'abl_b.c')).read(), INIT, INITW)
W['dwi_col'] = sub(V['dw_init'], '        s32 vtx = side * 4 + row * 2 + col;\n',
                   '        s32 vtx;\n        do { vtx = side * 4 + row * 2 + col; } while (0); /* FAKE */\n')
W['dwi_l1'] = sub(sub(V['dw_init'], '        s32 vtx = side * 4 + row * 2 + col;\n', '        s32 vtx = side * 4 + row * 2 + col;\n        do { /* FAKE */\n'),
                  '            *(s32 *)(scr + 0x88) = quads[vtx].y;\n        }\n    }\n',
                  '            *(s32 *)(scr + 0x88) = quads[vtx].y;\n        }\n        } while (0);\n    }\n')
RCV = '        s32 row = i / 2;\n        s32 col = i & 1;\n        s32 vtx = side * 4 + row * 2 + col;\n'
W['dwi_rc'] = sub(V['dw_init'], RCV, '        s32 row, col, vtx;\n\n        do { row = i / 2; col = i & 1; } while (0); /* FAKE */\n        vtx = side * 4 + row * 2 + col;\n')
W['dwi_rcv'] = sub(V['dw_init'], RCV, '        s32 row, col, vtx;\n\n        do { row = i / 2; col = i & 1; vtx = side * 4 + row * 2 + col; } while (0); /* FAKE */\n')
for k, s in W.items():
    open(os.path.join(here, k + '.c'), 'w', newline='\n').write(s)
    print(k)
