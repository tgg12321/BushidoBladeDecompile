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
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
P4END = "    s.y = 0x29;\n    shown = 0;\n"
VARIANTS = {}
for h2, h3, c1 in itertools.product(['plain', 'top'], ['plain', 'early', 'top', 'pre'], ['plain', 'top', 'pre', 'early']):
    decl = "    Unk8009B400Record *cell2;\n"
    top = ''; pre = ''; early = ''
    if h2 == 'top':
        decl += "    Unk8009B398Record *hdr2;\n"; top += "        hdr2 = &D_8009B398[2]; /* FAKE */\n"
    top += "        cell2 = &D_8009B3F0; /* FAKE */\n"
    for name, typ, val, where in (('hdr3', 'Unk8009B398Record', '&D_8009B398[3]', h3), ('cell3', 'Unk8009B400Record', '&D_8009B3F8', c1)):
        if where == 'plain': continue
        decl += "    %s *%s;\n" % (typ, name)
        line = "%s = %s; /* FAKE */\n" % (name, val)
        if where == 'top': top += "        " + line
        if where == 'pre': pre += "    " + line
        if where == 'early': early += "    " + line
    body = "        s.header = %s;\n        s.table = cell2;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n" % ("hdr2" if h2 == 'top' else "&D_8009B398[2]")
    body += "        s.header = %s;\n        s.table = %s;\n        s.out = cur;\n        cur = func_8007352C((s32)&s);\n" % (("hdr3" if h3 != 'plain' else "&D_8009B398[3]"), ("cell3" if c1 != 'plain' else "&D_8009B3F8"))
    reps = [(OLD, body), (DSHOWN, DSHOWN + decl), (LOOP, pre + "    for (j = 0; j < 2; j++) {\n" + top + "        SetTile(tile);")]
    if early: reps.append((P4END, "    s.y = 0x29;\n" + early + "    shown = 0;\n"))
    VARIANTS['lt_%s_%s_%s' % (h2, h3, c1)] = reps
