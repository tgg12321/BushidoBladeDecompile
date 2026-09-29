#!/usr/bin/env python3
"""Probes on the EXACT landed body (the function definition as spliced into src/text1b.c), for the engine
sandbox (--candidate replaces only the definition). Output: memory/grind/func_8005E54C/final_probes/*.c"""
import re, os
src = open('src/text1b.c', newline='\n').read()
a = src.index('s32 func_8005E54C(u32 arg0, s32 arg1, s32 arg2) {')
b = src.index('\n}\n', a) + 3
fn = src[a:b]
D = 'memory/grind/func_8005E54C/final_probes/'
os.makedirs(D, exist_ok=True)


def w(name, t):
    open(D + name + '.c', 'w', newline='\n').write(t)


def rep(t, old, new):
    assert t.count(old) == 1, old
    return t.replace(old, new)


w('landed', fn)
i0 = fn.index('    /* FAKE: unused here.')
i1 = fn.index('    volatile s16 digit[3];\n') + len('    volatile s16 digit[3];\n')
nod = fn[:i0] + fn[i1:]
w('no_digit', nod)
w('plain_digit', rep(fn, '    volatile s16 digit[3];\n', '    s16 digit[3];\n'))
# Q36(4): the Q33 union spelling of the same local on the same body
u = rep(fn, '    s16 points[2];\n', '    union {\n        s16 v[2];\n        s32 word;\n    } points;\n')
u = rep(u, '*(s32 *)points = 0;', 'points.word = 0;')
u = u.replace('&points[', '&points.v[')
u = re.sub(r'(?<![.&])points\[', 'points.v[', u)
w('union', u)
# Q35(7): real locals moved into the region (declaration order), on the body without the array
decl_p = nod[nod.index('    /* The per-player points pair'):nod.index('    s16 points[2];\n') + len('    s16 points[2];\n')]
w('loc_wins_after_s', rep(nod, '    s16 wins[2];\n    Env5E54C s;\n', '    Env5E54C s;\n    s16 wins[2];\n'))
w('loc_points_after_s', rep(nod.replace(decl_p, ''), '    Env5E54C s;\n', '    Env5E54C s;\n    s16 points[2];\n'))
# Q35(7) producer 3: live named local on a multi-read field (R7 wins[j], read twice)
w('prod3_named_local', rep(nod, '''        s.x = j * 70 + 0x113;
        if (wins[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[wins[j]];''', '''        s16 w = wins[j];

        s.x = j * 70 + 0x113;
        if (w == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[w];'''))
# Q35(7) producer 1: folded loop-guard compare (R4 mark loop as a guarded do-while reusing its exit test)
p1 = rep(nod, '''        for (k = 0; k < points[j]; k++) {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;''', '''        k = 0;
        if (k < points[j]) do {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;''')
p1 = rep(p1, '''            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
        }''', '''            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            k++;
        } while (k < points[j]);''')
w('prod1_loop_guard', p1)
# Q35(7) producer 2: combine orphan-USE needs an HImode sign-extend intermediate with a second narrow
# use; the only HImode value consumed twice narrow here is the portrait char c: give its second use the
# narrow value explicitly
w('prod2_himode_second_use', rep(nod, '            s.x = j * 320 + D_8009B58C[c];', '            s.x = j * 320 + D_8009B58C[(s16)c];'))
print(sorted(os.listdir(D)))
