import itertools
OLD = """        s.header = &D_8009B398[2];
        s.table = &D_8009B3F0;
        s.out = cur;
        cur = func_8007352C((s32)&s);
        s.header = &D_8009B398[3];
        s.table = &D_8009B3F8;
        s.out = cur;
        cur = func_8007352C((s32)&s);
"""
DSHOWN = "    s16 shown;\n"
P4 = "    s.header = &D_8009B398[0];\n    digit[0] = digit[1] = digit[2] = arg1;\n"
P3 = "    s.header = &D_8009B398[0];\n    s.has_color = 0;\n    s.y = 0x16;\n"
P2 = "    s.header = &D_8009B398[1];\n"
VARIANTS = {}
for where, h2f, c0f, order in itertools.product(['p4', 'p3', 'p2', 'p4end'], ['p', 'm'], ['p', 'm'], ['hc', 'ch']):
    decl = "    Unk8009B398Record *hdr2;\n    Unk8009B398Record *hdr3;\n    Unk8009B400Record *cell0;\n    Unk8009B400Record *cell1;\n"
    h2 = ("        hdr2 = &D_8009B398[2] + 1; /* FAKE */\n        s.header = hdr2 - 1;\n" if h2f == 'p'
          else "        hdr2 = &D_8009B398[2] - 1; /* FAKE */\n        s.header = hdr2 + 1;\n")
    c0 = ("        cell0 = &D_8009B3F0 + 1; /* FAKE */\n        s.table = cell0 - 1;\n" if c0f == 'p'
          else "        cell0 = &D_8009B3F0 - 1; /* FAKE */\n        s.table = cell0 + 1;\n")
    body = h2 + c0 + "        s.out = cur;\n        cur = func_8007352C((s32)&s);\n        s.header = hdr3;\n        s.table = cell1;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    a = "    hdr3 = &D_8009B398[3]; /* FAKE */\n"; b = "    cell1 = &D_8009B3F8; /* FAKE */\n"
    ali = a + b if order == 'hc' else b + a
    reps = [(OLD, body), (DSHOWN, DSHOWN + decl)]
    if where == 'p4': reps.append((P4, P4.split('\n')[0] + '\n' + ali + P4.split('\n', 1)[1]))
    if where == 'p3': reps.append((P3, P3.split('\n')[0] + '\n' + ali + P3.split('\n', 1)[1]))
    if where == 'p2': reps.append((P2, P2 + ali))
    if where == 'p4end': reps.append(("    s.y = 0x29;\n    shown = 0;\n", "    s.y = 0x29;\n" + ali + "    shown = 0;\n"))
    VARIANTS['u_%s_%s%s_%s' % (where, h2f, c0f, order)] = reps
