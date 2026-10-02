from pathlib import Path
D = Path('tmp/func_80020E74/a6/r280/s4')
D.mkdir(exist_ok=True)
b = Path('tmp/func_80020E74/a6/r280/sep_k.c').read_text()


def w(name, t):
    (D / name).write_bytes(t.encode())


i = b.index("                if (a0 == 0) {")
j = b.index("            store5_21280:")
w('no_dup.c', b[:i] + "                if (a0 == 0) goto next_21280;\n" + b[j:])
w('no_mode.c', b.replace("            s32 mode;\n", "").replace("            mode = D_800A38DC;\n", "")
  .replace("if (mode != t3)", "if (D_800A38DC != t3)").replace("if (mode != 0)", "if (D_800A38DC != 0)"))
