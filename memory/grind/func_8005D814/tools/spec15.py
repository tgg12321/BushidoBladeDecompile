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
NEW = """        s.header = hdr2;
        s.table = cell2;
        s.out = cur;
        cur = func_8007352C((s32)&s);
        s.header = hdr3;
        s.table = cell3;
        s.out = cur;
        cur = func_8007352C((s32)&s);
"""
DSHOWN = "    s16 shown;\n"
DECL = "    Unk8009B398Record *hdr2;\n    Unk8009B398Record *hdr3;\n    Unk8009B400Record *cell2;\n    Unk8009B400Record *cell3;\n"
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
COL = "    s.col_r = 0xFF;\n"
P4 = "    s.header = &D_8009B398[0];\n    digit[0] = digit[1] = digit[2] = arg1;\n"
P2 = "    s.header = &D_8009B398[1];\n"
P4END = "    s.y = 0x29;\n    shown = 0;\n"
VARIANTS = {}
A3 = "    hdr3 = &D_8009B398[3]; /* FAKE */\n"; A1 = "    cell3 = &D_8009B3F8; /* FAKE */\n"
H2 = "    hdr2 = &D_8009B398[2]; /* FAKE */\n"; C0 = "    cell2 = &D_8009B3F0; /* FAKE */\n"
for early, order, late in itertools.product(['p4', 'p2', 'p4end'], ['hc', 'ch'], ['loop', 'col']):
    ea = A3 + A1
    la = H2 + C0 if order == 'hc' else C0 + H2
    reps = [(OLD, NEW), (DSHOWN, DSHOWN + DECL)]
    if early == 'p4': reps.append((P4, P4.split('\n')[0] + '\n' + ea + P4.split('\n', 1)[1]))
    if early == 'p2': reps.append((P2, P2 + ea))
    if early == 'p4end': reps.append((P4END, "    s.y = 0x29;\n" + ea + "    shown = 0;\n"))
    if late == 'loop': reps.append((LOOP, la + LOOP))
    else: reps.append((COL, la + COL))
    VARIANTS['al_%s_%s_%s' % (early, order, late)] = reps
