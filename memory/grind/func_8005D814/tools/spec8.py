C0 = "        s.table = &D_8009B3F0;\n"
C1 = "        s.table = &D_8009B3F8;\n"
DECL = "    s16 shown;\n"
def arm(sym):
    return ("        if (j != 0) {\n            s.table = &%s; /* FAKE */\n        } else {\n            s.table = &%s; /* FAKE */\n        }\n" % (sym, sym))
def jj(sym):
    return "        s.table = &%s + j - j; /* FAKE */\n" % sym
def curx(sym):
    return "        s.table = (Unk8009B400Record *)((s32)&%s + cur - cur); /* FAKE */\n" % sym
def tilex(sym):
    return "        s.table = (Unk8009B400Record *)((s32)&%s + (s32)tile - (s32)tile); /* FAKE */\n" % sym
def two(sym, v):
    return "        %s = &%s + 1; /* FAKE */\n        s.table = %s - 1;\n" % (v, sym, v)
VARIANTS = {
 'x_arm_both': [(C0, arm('D_8009B3F0')), (C1, arm('D_8009B3F8'))],
 'x_arm_c0': [(C0, arm('D_8009B3F0'))],
 'x_jj_both': [(C0, jj('D_8009B3F0')), (C1, jj('D_8009B3F8'))],
 'x_cur_both': [(C0, curx('D_8009B3F0')), (C1, curx('D_8009B3F8'))],
 'x_tile_both': [(C0, tilex('D_8009B3F0')), (C1, tilex('D_8009B3F8'))],
 'x_two_both': [(DECL, DECL + "    Unk8009B400Record *cell0;\n    Unk8009B400Record *cell1;\n"), (C0, two('D_8009B3F0', 'cell0')), (C1, two('D_8009B3F8', 'cell1'))],
 'x_tile_c0': [(C0, tilex('D_8009B3F0'))],
 'x_cur_c0': [(C0, curx('D_8009B3F0'))],
}
