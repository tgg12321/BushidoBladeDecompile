import itertools
C = {0: ("        s.table = &D_8009B3F0;\n", "D_8009B3F0", "cell0"),
     1: ("        s.table = &D_8009B3F8;\n", "D_8009B3F8", "cell1")}
DECL = "    s16 shown;\n"
TOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);\n"
PRE = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
def form(f, k):
    line, sym, v = C[k]
    if f == 'A': return [], None, None
    if f == 'B': return [(line, "        %s = &%s + 1; /* FAKE */\n        s.table = %s - 1;\n" % (v, sym, v))], v, None
    if f == 'E': return [(line, "        %s = &%s - 1; /* FAKE */\n        s.table = %s + 1;\n" % (v, sym, v))], v, None
    if f == 'D': return [(line, "        %s = &%s;\n        s.table = %s;\n" % (v, sym, v))], v, None
    if f == 'C': return [(line, "        s.table = %s - 1;\n" % v)], v, ('top', "        %s = &%s + 1; /* FAKE */\n" % (v, sym))
    if f == 'F': return [(line, "        s.table = %s - 1;\n" % v)], v, ('pre', "    %s = &%s + 1; /* FAKE */\n" % (v, sym))
VARIANTS = {}
for f0, f1 in itertools.product('ABCDEF', repeat=2):
    if f0 == 'A' and f1 == 'A': continue
    reps = []; decls = ''; top = ''; pre = ''
    for f, k in ((f0, 0), (f1, 1)):
        r, v, extra = form(f, k)
        reps += r
        if v: decls += "    Unk8009B400Record *%s;\n" % v
        if extra and extra[0] == 'top': top += extra[1]
        if extra and extra[0] == 'pre': pre += extra[1]
    if decls: reps.append((DECL, DECL + decls))
    if top: reps.append((TOP, TOP + top))
    if pre: reps.append((PRE, pre + PRE))
    VARIANTS['y_%s%s' % (f0, f1)] = reps
