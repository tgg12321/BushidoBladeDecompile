#!/usr/bin/env python3
"""Generate the Q35/Q36 evidence probes from match0/body.c into tmp/func_8005E54C/v/."""
import re
B = 'memory/grind/func_8005E54C/match0/body.c'
V = 'tmp/func_8005E54C/v/'
base = open(B, newline='\n').read()


def rep(s, old, new, count=1):
    assert s.count(old) >= 1, old
    return s.replace(old, new) if count == 0 else s.replace(old, new, count)


def w(name, s):
    open(V + name + '.c', 'w', newline='\n').write(s)


fin = rep(base, '    s16 digit[3];\n', '    volatile s16 digit[3];\n')
w('fin', fin)
w('fin_plain', base)
nod = rep(base, '    s16 digit[3];\n', '')
w('fin_nodigit', nod)
# Q36(4): the Q33 union spelling of the same local on the same body
u = rep(fin, '    s16 vals[2];\n', '    union {\n        s16 v[2];\n        s32 word;\n    } vals;\n')
u = rep(u, '*(s32 *)vals = 0;', 'vals.word = 0;')
u = u.replace('&vals[', '&vals.v[')
u = re.sub(r'(?<![.&])vals\[', 'vals.v[', u)
w('fin_union', u)
# Q35(7) real locals that could occupy the region (declaration order moves)
w('loc_wins_after', rep(nod, '    s16 wins[2];\n    Env5E54C s;\n', '    Env5E54C s;\n    s16 wins[2];\n'))
w('loc_vals_after', rep(nod, '    s16 vals[2];\n    s16 wins[2];\n    Env5E54C s;\n', '    s16 wins[2];\n    Env5E54C s;\n    s16 vals[2];\n'))
# Q35(7) producer 3: live named local on a multi-read field (R7 wins[j], read twice)
p3 = rep(nod, '''        s.x = j * 70 + 0x113;
        if (wins[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[wins[j]];''', '''        s16 w = wins[j];

        s.x = j * 70 + 0x113;
        if (w == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[w];''')
w('prod3_named', p3)
# Q35(7) producer 1: folded loop-guard compare (R4 inner loop as a guarded do-while re-using its exit test)
old = '''        for (k = 0; k < vals[j]; k++) {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;'''
assert nod.count(old) == 1
p1 = nod.replace(old, '''        k = 0;
        if (k < vals[j]) do {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;''')
old2 = '''            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
        }'''
assert p1.count(old2) == 1
p1 = p1.replace(old2, '''            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = func_8007352C((s32)&s);
            k++;
        } while (k < vals[j]);''')
w('prod1_guard', p1)
# Q35(7) producer 2: an HImode sign-extend intermediate consumed narrow a second time (portrait char)
p2 = rep(nod, '''            s.table = UesrWorkDef[c];
            s.x = j * 320 + D_8009B58C[c];''', '''            s.table = UesrWorkDef[c];
            s.x = j * 320 + D_8009B58C[(s16)c];''')
w('prod2_narrow', p2)
print('ok')
