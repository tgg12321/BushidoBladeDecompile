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
DTILE = "    Tile5D814 *tile;\n"
DSHOWN = "    s16 shown;\n"
VARIANTS = {}
for h3, c1, c0, dpos in itertools.product(['P1', 'P2', 'P0', 'N'], ['F', 'B', 'E', 'A'], ['B', 'A'], ['early', 'late']):
    decl = []
    pre = ''
    body = "        s.header = &D_8009B398[2];\n"
    if h3 == 'P2': body = "        hdr3 = &D_8009B398[3]; /* FAKE */\n" + body
    if c0 == 'B':
        body += "        cell0 = &D_8009B3F0 + 1; /* FAKE */\n"; decl.append("    Unk8009B400Record *cell0;\n")
    if h3 == 'P1': body += "        hdr3 = &D_8009B398[3]; /* FAKE */\n"
    if c1 == 'E':
        body += "        cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    body += "        s.table = %s;\n" % ("cell0 - 1" if c0 == 'B' else "&D_8009B3F0")
    body += "        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    body += "        s.header = %s;\n" % ("hdr3" if h3 in ('P1', 'P2', 'P0') else "&D_8009B398[3]")
    if c1 == 'B':
        body += "        cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    if c1 == 'F':
        pre += "    cell1 = &D_8009B3F8 + 1; /* FAKE */\n"
    body += "        s.table = %s;\n" % ("cell1 - 1" if c1 in ('F', 'B', 'E') else "&D_8009B3F8")
    body += "        s.out = cur;\n        cur = func_8007352C((s32)&s);\n"
    if h3 == 'P0':
        pre += "    hdr3 = &D_8009B398[3]; /* FAKE */\n"
    if c1 != 'A': decl.append("    Unk8009B400Record *cell1;\n")
    hdecl = "    Unk8009B398Record *hdr3;\n" if h3 != 'N' else ''
    reps = [(OLD, body)]
    if pre: reps.append((LOOPSTART, pre + LOOPSTART))
    if dpos == 'early' and hdecl:
        reps.append((DTILE, hdecl + DTILE)); reps.append((DSHOWN, DSHOWN + ''.join(decl)))
    else:
        reps.append((DSHOWN, DSHOWN + ''.join(decl) + hdecl))
    if h3 == 'N' and dpos == 'early': continue
    if h3 == 'N' and c0 == 'A' and c1 == 'A': continue
    VARIANTS['z_%s_%s_%s_%s' % (h3, c1, c0, dpos)] = reps
