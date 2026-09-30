"""Full-file spellings of src/text1b_tu2.c for items 4/5 (func_800768DC,
func_800770B8) and the other consumers of D_800A35D0 / D_8009BCE4 in the TU."""
import os
src = open("src/text1b_tu2.c", encoding="utf-8").read()
out = "tmp/ffc/768dc"
os.makedirs(out, exist_ok=True)


def sub(s, a, b, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, b)


def w(name, s):
    open(f"{out}/{name}.c", "w", encoding="utf-8", newline="\n").write(s)


w("f0_control", src)

# F1: D_800A35D0 as the per-player pair table it is; D_8009BCE4 as its 20 bytes.
s = src
s = sub(s, "extern u8 D_8009BD20[][2];\nextern s16 D_800A35D0;\n",
        "extern u8 D_8009BD20[][2];\nextern s16 D_800A35D0[2][2];\n")
s = sub(s, "extern s16 D_800A35D0;\n", "extern s16 D_800A35D0[2][2];\n", 2)
s = sub(s, "(s16 *)(base + 0x40), &D_800A35D0);", "(s16 *)(base + 0x40), D_800A35D0[0]);")
s = sub(s, "(&D_800A35D0) + (arg1 * 2))", "D_800A35D0[arg1])")
s = sub(s, "(&D_800A35D0) + (arg3 * 2))", "D_800A35D0[arg3])")
s = sub(s, "extern u8 D_8009BCE4;\n", "extern u8 D_8009BCE4[20];\n", 3)
s = sub(s, "(&D_8009BCE4)[arg2[SELWORK_800768DC->f38[arg3]]]", "D_8009BCE4[arg2[SELWORK_800768DC->f38[arg3]]]")
s = sub(s, "(&D_8009BCE4)[idx]", "D_8009BCE4[idx]", 4)
s = sub(s, "sym = (u8 *)&D_800A35D0;", "sym = (u8 *)D_800A35D0;", 2)
w("f1_decls_770b8_pun_kept", s)
f1 = s

# F2: F1 + func_800770B8's per-player pair through a typed row pointer,
# D_8009BD21 through its record, SelWork f1C/f20 as Q46 union word views.
s = f1
s = sub(s, """        u8 *dp;
        u8 *sym;
""", """        s16 (*sym)[2];
""")
s = sub(s, """        sym = (u8 *)D_800A35D0;
        dp = sym + (t0 * 4);
        *(s16 *)(dp + 2) = 0;
        *(s16 *)(dp + 0) = 0;
""", """        sym = D_800A35D0;
        sym[t0][1] = 0;
        sym[t0][0] = 0;
""")
s = sub(s, "                sym = (u8 *)D_800A35D0;\n", "                sym = D_800A35D0;\n")
s = sub(s, "extern u8 D_8009BD21;\n", "")
s = sub(s, "(&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2]",
        "D_8009BD20[*(u8 *)(D_800A36A0 + 0x67)][1]")
s = sub(s, """    s16 f18[2];
    u8 pad1C[0x18];
""", """    s16 f18[2];
    union {
        s16 half[2];
        s32 word;
    } f1C;
    union {
        s16 half[2];
        s32 word;
    } f20;
    u8 pad24[0x10];
""")
w("f2_full_frontier_nop1c", s)
f2a = s
s = sub(s, """        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
""", """        ((SelWork_800768DC *)p)->f20.word = 0;
        ((SelWork_800768DC *)p)->f1C.word = 0;
""")
w("f2b_full_frontier_cast_p", s)
s = sub(f2a, """        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
""", """        u8 *p = D_800A36A0;
        SELWORK_800768DC->f20.word = 0;
        SELWORK_800768DC->f1C.word = 0;
""")
w("f2c_full_frontier_macro", s)
print("ok")

# F3: F2c without the row pointer (direct subscripts; the re-set dropped)
f2c = open(f"{out}/f2c_full_frontier_macro.c", encoding="utf-8").read()
s = sub(f2c, "        s16 (*sym)[2];\n", "")
s = sub(s, """        sym = D_800A35D0;
        sym[t0][1] = 0;
        sym[t0][0] = 0;
""", """        D_800A35D0[t0][1] = 0;
        D_800A35D0[t0][0] = 0;
""")
i = s.index("                /* FAKE: same-value dead store re-establishing sym")
j = s.index("                sym = D_800A35D0;\n", i) + len("                sym = D_800A35D0;\n")
s = s[:i] + s[j:]
w("f3_direct_no_row_pointer", s)
# F3b: row pointer kept, re-set dropped
s = f2c
i = s.index("                /* FAKE: same-value dead store re-establishing sym")
j = s.index("                sym = D_800A35D0;\n", i) + len("                sym = D_800A35D0;\n")
w("f3b_row_pointer_no_reset", s[:i] + s[j:])
print("f3 ok")
