import itertools, os
exec(open('tmp/f65800/psxt/gen.py').read().split("V = {")[0])
T = T.replace("    s16 *t;\n", "    s16 *t;\n    s16 *tbl;\n")
A = T.index("        t = &D_800F0BA8[arg0];"); B = T.index("        break;\n    }\n    D_800A37D4")
inits = {'A': "t = D_800F0BA8;\n        t += arg0;", 'B': "tbl = D_800F0BA8;\n        t = tbl + arg0;",
         'C': "t = &D_800F0BA8[arg0];", 'D': "tbl = D_800F0BA8;\n        t = &tbl[arg0];"}
tests = {'ge': '%s >= 8', 'gt': '%s > 7', 'nl': '!(%s < 8)'}
reads = {'t': '*t', 'd': 'D_800F0BA8[arg0]', 'z': 't[0]'}
n_first = {'nf': True, 'sf': False}
structs = ['ifelse', 'inv', 'brk']
os.makedirs('tmp/f65800/psxt/sw', exist_ok=True)
names = []
for (ki, si), (kt, st), (kr, sr), (ke, se), (kn, nf), ks in itertools.product(inits.items(), tests.items(), reads.items(), reads.items(), n_first.items(), structs):
    tt = st % sr
    et = ("%s >= 5") % se
    th = ("            n = 10 - %s;\n            D_800A3488 = (s32)D_8009B8C8;\n" if nf else "            D_800A3488 = (s32)D_8009B8C8;\n            n = 10 - %s;\n") % sr
    th += """            prim->r0 = n << 6;
            prim->g0 = n * 0x70 / 3;
            prim->b0 = n * 0x60 / 15;
            *(s32 *)D_800A3490 = 0x2E;
"""
    el = """            if (%s) {
                D_800A3488 = (s32)D_8009B9F0;
            } else {
                D_800A3488 = (s32)D_8009B9E8;
            }
            *(s32 *)D_800A3490 = 0x2F;
""" % et
    if ks == 'ifelse':
        body = "        if (%s) {\n%s        } else {\n%s        }\n" % (tt, th, el)
    elif ks == 'inv':
        body = "        if (!(%s)) {\n%s        } else {\n%s        }\n" % (tt, el, th)
    else:
        body = "        if (%s) {\n%s            break;\n        }\n%s" % (tt, th, el)
    nm = f'tmp/f65800/psxt/sw/{ki}{kt}{kr}{ke}{kn}{ks}.c'
    open(nm, 'w', newline='\n').write(T[:A] + "        " + si + "\n" + body + T[B:])
    names.append(nm)
NL = chr(10)
open('tmp/f65800/psxt/sw.lst', 'w', newline=NL).write(NL.join(names) + NL)
print(len(names))
