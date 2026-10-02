from pathlib import Path
D = Path('tmp/func_80020E74/a6/r280')
for p in D.glob('*.c'):
    p.write_bytes(p.read_bytes().replace(b'\r\n', b'\n'))
b = (D / 'mode_local.c').read_text()


def w(name, t):
    (D / name).write_bytes(t.encode())


def sep_k(t):
    t = t.replace("            s32 mode;\n", "            s32 mode;\n            s32 k;\n").replace("            a1 = 0;\n", "            k = 0;\n")
    i = t.index("        loop2_21280:")
    head, tail = t[:i], t[i:]
    tail = tail.replace("(a1 << 2)", "(k << 2)").replace("= a1;", "= k;").replace("a1++;", "k++;").replace("if (a1 < 3)", "if (k < 3)")
    return head + tail


w('sep_k.c', sep_k(b))
