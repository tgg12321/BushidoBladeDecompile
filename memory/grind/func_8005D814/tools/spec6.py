import itertools
L = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
DECL = "    s16 shown;\n"
items = {
 'h2': ("hdr2", "Unk8009B398Record", "&D_8009B398[2]", "        s.header = &D_8009B398[2];\n", "        s.header = hdr2;\n"),
 'c2': ("cell2", "Unk8009B400Record", "&D_8009B3F0", "        s.table = &D_8009B3F0;\n", "        s.table = cell2;\n"),
 'h3': ("hdr3", "Unk8009B398Record", "&D_8009B398[3]", "        s.header = &D_8009B398[3];\n", "        s.header = hdr3;\n"),
 'c3': ("cell3", "Unk8009B400Record", "&D_8009B3F8", "        s.table = &D_8009B3F8;\n", "        s.table = cell3;\n"),
}
VARIANTS = {}
keys = ['h2', 'c2', 'h3', 'c3']
for r in range(1, 5):
    for combo in itertools.combinations(keys, r):
        if combo == ('c2', 'c3'):
            continue
        decl = ''.join("    %s *%s;\n" % (items[k][1], items[k][0]) for k in combo)
        init = ''.join("    %s = %s;\n" % (items[k][0], items[k][2]) for k in combo)
        reps = [(DECL, DECL + decl), (L, init + L)] + [(items[k][3], items[k][4]) for k in combo]
        VARIANTS['x_' + '_'.join(combo)] = reps
