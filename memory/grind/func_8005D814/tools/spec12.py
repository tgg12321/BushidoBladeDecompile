import itertools
LOOPSTART = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
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
COL = "    s.col_r = 0xFF;\n"
TOP = "    arg1--;\n"
VARIANTS = {}
for h2, c0, order, where in itertools.product(['X', 'A'], ['X', 'A'], ['hc', 'ch'], ['loop', 'col', 'top']):
    decl = "    Unk8009B398Record *hdr3;\n    Unk8009B400Record *cell1;\n"
    body = ''
    if h2 == 'X':
        decl += "    Unk8009B398Record *hdr2;\n"
        body += "        hdr2 = &D_8009B398[2] + 1; /* FAKE */\n        s.header = hdr2 - 1;\n"
    else:
        body += "        s.header = &D_8009B398[2];\n"
    if c0 == 'X':
        decl += "    Unk8009B400Record *cell0;\n"
        body += "        cell0 = &D_8009B3F0 + 1; /* FAKE */\n        s.table = cell0 - 1;\n"
    else:
        body += "        s.table = &D_8009B3F0;\n"
    body += "        s.out = cur;\n        cur = func_8007352C((s32)&s);\n        s.header = hdr3;\n        s.table = cell1;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    a = "    hdr3 = &D_8009B398[3]; /* FAKE */\n"; b = "    cell1 = &D_8009B3F8; /* FAKE */\n"
    pre = a + b if order == 'hc' else b + a
    reps = [(OLD, body), (DSHOWN, DSHOWN + decl)]
    if where == 'loop': reps.append((LOOPSTART, pre + LOOPSTART))
    if where == 'col': reps.append((COL, pre + COL))
    if where == 'top': reps.append((TOP, TOP + pre))
    VARIANTS['v_%s%s_%s_%s' % (h2, c0, order, where)] = reps
