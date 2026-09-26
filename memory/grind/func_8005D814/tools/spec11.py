import itertools
LOOPSTART = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
TOP = "    for (j = 0; j < 2; j++) {\n"
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
VARIANTS = {}
for h2, h3, c0, c1 in itertools.product(['N', 'I', 'P'], ['N', 'I', 'P'], ['B', 'T'], ['T', 'M', 'B']):
    decl = "    Unk8009B400Record *cell0;\n    Unk8009B400Record *cell1;\n"
    pre = ''; top = ''
    body = ''
    if h2 == 'I': body += "        hdr2 = &D_8009B398[2]; /* FAKE */\n"
    if h2 == 'P': pre += "    hdr2 = &D_8009B398[2]; /* FAKE */\n"
    if h2 != 'N': decl += "    Unk8009B398Record *hdr2;\n"
    if h3 == 'P': pre += "    hdr3 = &D_8009B398[3]; /* FAKE */\n"
    if h3 != 'N': decl += "    Unk8009B398Record *hdr3;\n"
    body += "        s.header = %s;\n" % ("hdr2" if h2 != 'N' else "&D_8009B398[2]")
    if c0 == 'B': body += "        cell0 = &D_8009B3F0 + 1; /* FAKE */\n"
    else: top += "        cell0 = &D_8009B3F0 + 1; /* FAKE */\n"
    if c1 == 'T': top += "        cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    if c1 == 'M': body += "        cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    if h3 == 'I': body += "        hdr3 = &D_8009B398[3]; /* FAKE */\n"
    body += "        s.table = cell0 - 1;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    body += "        s.header = %s;\n" % ("hdr3" if h3 != 'N' else "&D_8009B398[3]")
    if c1 == 'B': body += "        cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    body += "        s.table = cell1 - 1;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    reps = [(OLD, body), (DSHOWN, DSHOWN + decl)]
    if top: reps.append((LOOPSTART, TOP + top + "        SetTile(tile);"))
    if pre: reps.append((LOOPSTART, pre + LOOPSTART) if not top else (TOP + top, pre + TOP + top))
    VARIANTS['w_%s%s_%s%s' % (h2, h3, c0, c1)] = reps
