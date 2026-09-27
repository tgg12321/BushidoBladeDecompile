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
P4END = "    s.y = 0x29;\n    shown = 0;\n"
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
POS = {
 'aTP': "        tile++;\n",
 'bSY': "        s.y = D_8009B450[j].y;\n",
 'cH': "        tile->h = 1;\n",
 'dW': "        tile->w = 0x238 - D_8009B450[j].x;\n",
}
VARIANTS = {}
for pos, where, order in itertools.product(POS, ['early', 'pre'], ['hc', 'ch']):
    ins = ("        hdr2 = &D_8009B398[2]; /* FAKE */\n        cell2 = &D_8009B3F0; /* FAKE */\n" if order == 'hc'
           else "        cell2 = &D_8009B3F0; /* FAKE */\n        hdr2 = &D_8009B398[2]; /* FAKE */\n")
    ea = "    hdr3 = &D_8009B398[3]; /* FAKE */\n    cell3 = &D_8009B3F8; /* FAKE */\n"
    reps = [(OLD, NEW), (DSHOWN, DSHOWN + DECL), (POS[pos], POS[pos] + ins)]
    if where == 'early': reps.append((P4END, "    s.y = 0x29;\n" + ea + "    shown = 0;\n"))
    else: reps.append((LOOP, ea + LOOP))
    VARIANTS['mid_%s_%s_%s' % (pos, where, order)] = reps
