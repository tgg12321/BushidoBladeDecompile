base = open('tmp/func_8005D814/var/u_p4end_pp_hc.c').read()
H2X = "        hdr2 = &D_8009B398[2] + 1; /* FAKE */\n        s.header = hdr2 - 1;\n"
C0X = "        cell0 = &D_8009B3F0 + 1; /* FAKE */\n        s.table = cell0 - 1;\n"
A3 = "    hdr3 = &D_8009B398[3]; /* FAKE */\n"
A1 = "    cell1 = &D_8009B3F8; /* FAKE */\n"
VARIANTS = {
 'a_noh2x': [(H2X, "        s.header = &D_8009B398[2];\n")],
 'a_noc0x': [(C0X, "        s.table = &D_8009B3F0;\n")],
 'a_noa3': [(A3, ""), ("        s.header = hdr3;\n", "        s.header = &D_8009B398[3];\n")],
 'a_noa1': [(A1, ""), ("        s.table = cell1;\n", "        s.table = &D_8009B3F8;\n")],
 'a_noh2x_noc0x': [(H2X, "        s.header = &D_8009B398[2];\n"), (C0X, "        s.table = &D_8009B3F0;\n")],
 'a_h2alias': [(H2X, "        s.header = hdr2;\n"), (A3, A3 + "    hdr2 = &D_8009B398[2]; /* FAKE */\n")],
}
