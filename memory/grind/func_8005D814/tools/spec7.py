DECL = "    s16 shown;\n"
D2 = "    Unk8009B400Record *cell2;\n"
D3 = "    Unk8009B400Record *cell3;\n"
C2 = ("        s.table = &D_8009B3F0;\n", "        s.table = cell2;\n")
C3 = ("        s.table = &D_8009B3F8;\n", "        s.table = cell3;\n")
COL = "    s.col_r = 0xFF;\n"
LOOP = "    for (j = 0; j < 2; j++) {\n        SetTile(tile);"
TOP = "    arg1--;\n"
I2 = "    cell2 = &D_8009B3F0;\n"
I3 = "    cell3 = &D_8009B3F8;\n"
VARIANTS = {
 'o_c3c2_loop': [(DECL, DECL + D2 + D3), (LOOP, I3 + I2 + LOOP), C2, C3],
 'o_c2c3_col': [(DECL, DECL + D2 + D3), (COL, I2 + I3 + COL), C2, C3],
 'o_c2c3_top': [(DECL, DECL + D2 + D3), (TOP, TOP + I2 + I3), C2, C3],
 'o_c2_col': [(DECL, DECL + D2), (COL, I2 + COL), C2],
 'o_c2_top': [(DECL, DECL + D2), (TOP, TOP + I2), C2],
 'o_d3first': [(DECL, DECL + D3 + D2), (LOOP, I2 + I3 + LOOP), C2, C3],
 'o_c3_top': [(DECL, DECL + D3), (TOP, TOP + I3), C3],
}
