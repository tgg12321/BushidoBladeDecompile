VARIANTS = {
 'ab_h2': [("        hdr2 = &D_8009B398[2]; /* FAKE */\n", ""), ("        s.header = hdr2;\n", "        s.header = &D_8009B398[2];\n")],
 'ab_c2': [("        cell2 = &D_8009B3F0; /* FAKE */\n", ""), ("        s.table = cell2;\n", "        s.table = &D_8009B3F0;\n")],
 'ab_h3': [("    hdr3 = &D_8009B398[3]; /* FAKE */\n", ""), ("        s.header = hdr3;\n", "        s.header = &D_8009B398[3];\n")],
 'ab_c3': [("    cell3 = &D_8009B3F8; /* FAKE */\n", ""), ("        s.table = cell3;\n", "        s.table = &D_8009B3F8;\n")],
}
