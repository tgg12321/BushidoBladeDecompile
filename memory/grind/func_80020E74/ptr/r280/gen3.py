import itertools
from pathlib import Path
D = Path('tmp/func_80020E74/a6/r280/s3')
D.mkdir(exist_ok=True)
b = Path('tmp/func_80020E74/a6/r280/sep_k.c').read_text()


def w(name, t):
    (D / name).write_bytes(t.encode())


def lit(t, names):
    for n, v in names:
        t = t.replace(f"            s32 {n};\n", "").replace(f"            {n} = {v};\n", "")
        t = t.replace(f"!= {n})", f"!= {v})")
    return t


consts = [('t4', '4'), ('t3', '3'), ('t2', '1')]
for r in range(1, 4):
    for c in itertools.combinations(consts, r):
        w('lit_' + '_'.join(x[0] for x in c) + '.c', lit(b, c))
w('no_t1.c', b.replace("            u16 t1;\n", "").replace("            t1 = val;\n", "").replace("(t1 >>", "(val >>"))
w('no_t0.c', b.replace("            u8 t0;\n", "").replace("            t0 = D_800A384C;\n", "").replace("if (t0 != nibble)", "if (D_800A384C != nibble)"))
i = b.index("                if (a0 == 0) {")
j = b.index("            store5_21280:")
w('no_dup.c', b[:i] + b[j:])
w('base.c', b)
