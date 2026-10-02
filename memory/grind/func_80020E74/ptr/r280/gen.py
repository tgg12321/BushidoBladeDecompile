from pathlib import Path
import itertools
D = Path('tmp/func_80020E74/a6/r280')
b = (D / 'base.c').read_text()
def lit(t, names):
    for n, v in names:
        t = t.replace(f"            s32 {n};\n", "").replace(f"            {n} = {v};\n", "")
        t = t.replace(f"!= {n})", f"!= {v})")
    return t
consts = [('t4', '4'), ('t3', '3'), ('t2', '1')]
for r in range(1, 4):
    for c in itertools.combinations(consts, r):
        name = 'lit_' + '_'.join(x[0] for x in c)
        (D / f'{name}.c').write_text(lit(b, c))
# drop t1
t = b.replace("            u16 t1;\n", "").replace("            t1 = val;\n", "").replace("(t1 >>", "(val >>")
(D / 'no_t1.c').write_text(t)
# drop t0
t = b.replace("            u8 t0;\n", "").replace("            t0 = D_800A384C;\n", "").replace("if (t0 != nibble)", "if (D_800A384C != nibble)")
(D / 'no_t0.c').write_text(t)
# mode direct (no a3 reuse)
t = b.replace("            a3 = D_800A38DC;\n", "").replace("if (a3 != t3)", "if (D_800A38DC != t3)").replace("if (a3 != 0)", "if (D_800A38DC != 0)")
(D / 'no_mode.c').write_text(t)
# separate mode local
t = b.replace("            u8 t0;\n", "            u8 t0;\n            s32 mode;\n").replace("            a3 = D_800A38DC;\n", "            mode = D_800A38DC;\n").replace("if (a3 != t3)", "if (mode != t3)").replace("if (a3 != 0)", "if (mode != 0)")
(D / 'mode_local.c').write_text(t)
for p in sorted(D.glob('*.c')): print(p.name)
