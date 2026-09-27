SET = "    cell2 = &D_8009B3F0;\n"
USE = "        s.table = cell2;\n"
DECL = "    Unk8009B400Record *cell2;\n"
VARIANTS = {
 'r_selfloop': [(USE, "        cell2 = cell2; /* FAKE */\n" + USE)],
 'r_selfpre': [(SET, SET + "    cell2 = cell2; /* FAKE */\n")],
 'r_deadpre': [(SET, "    cell2 = 0; /* FAKE */\n" + SET)],
 'r_ext_pre': [(SET, "    cell2 = &D_8009B3F0 + 1; /* FAKE */\n    cell2 = cell2 - 1;\n")],
 'r_deadcopy_loop': [(DECL, DECL + "    Unk8009B400Record *tmp_cell;\n"), (USE, "        tmp_cell = cell2; /* FAKE */\n" + USE)],
 'r_dupuse': [(USE, USE + USE)],
}
