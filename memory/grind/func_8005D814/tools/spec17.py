BLK = "    cell2 = &D_8009B3F0 + 1; /* FAKE */\n    cell2 = cell2 - 1;\n    cell3 = &D_8009B3F8;\n"
B2 = "    cell2 = &D_8009B3F0 + 1; /* FAKE */\n    cell2 = cell2 - 1;\n"
B3 = "    cell3 = &D_8009B3F8;\n"
COL = "    s.col_r = 0xFF;\n"
HC = "    s.has_color = 1;\n"
SX = "    s.x = 0;\n    s.semi = 0;\n"
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
P4END = "    s.y = 0x29;\n    shown = 0;\n"
VARIANTS = {
 'q_col': [(BLK, ""), (COL, BLK + COL)],
 'q_hc': [(BLK, ""), (HC, BLK + HC)],
 'q_sx': [(BLK, ""), (SX, BLK + SX)],
 'q_c3first': [(BLK, B3 + B2)],
 'q_c3early': [(BLK, B2), (P4END, "    s.y = 0x29;\n" + B3 + "    shown = 0;\n")],
 'q_c3early_col': [(BLK, ""), (COL, B2 + COL), (P4END, "    s.y = 0x29;\n" + B3 + "    shown = 0;\n")],
 'q_all_early': [(BLK, ""), (P4END, "    s.y = 0x29;\n" + BLK + "    shown = 0;\n")],
}
