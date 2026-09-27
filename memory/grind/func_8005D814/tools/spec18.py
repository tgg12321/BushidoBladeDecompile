B2 = "    cell2 = &D_8009B3F0 + 1; /* FAKE */\n    cell2 = cell2 - 1;\n"
B3 = "    cell3 = &D_8009B3F8;\n"
BLK = B2 + B3
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
CR = "    s.col_r = 0xFF;\n"
CB = "    s.col_b = 0x10;\n"
CG = "    s.col_g = 0x10;\n"
OT = "    s.ot_idx = arg3;\n"
VARIANTS = {
 'k_j0': [(BLK, ""), (LOOP, "    j = 0;\n" + BLK + "    for (; j < 2; j++) {\n        SetTile(tile);")],
 'k_j0b': [(BLK, B3), (LOOP, "    j = 0;\n" + B2 + "    for (; j < 2; j++) {\n        SetTile(tile);")],
 'k_afterCR': [(BLK, ""), (CR, CR + BLK)],
 'k_afterCB': [(BLK, ""), (CB, CB + BLK)],
 'k_afterCG': [(BLK, ""), (CG, CG + BLK)],
 'k_split': [(BLK, B3), (CR, B2 + CR)],
 'k_split2': [(BLK, ""), (CR, B3 + CR), (OT, OT + B2)],
}
