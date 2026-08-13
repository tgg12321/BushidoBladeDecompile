/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 24 at 49 build insns; the floor is 18 at 51.
 *
 * val2 split into three per-block locals, val and c left shared.  Never
 * isolated before (s4 split all three temporaries at once = 30; s5
 * isolated only c = 19).  Score 24.  The three val2 pseudos each STILL
 * conflict with hard reg 2 and the selected value moves to $a0, not $v0.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/split_val2/
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2a;
    u8 val2b;
    u8 val2c;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2a = val | 1;
    if (!c) {
        val2a = val;
    }
    D_80106A73 = val2a;

    c = p[8] & 2;
    val = D_80106A73;
    val2b = val | 2;
    if (!c) {
        val2b = val;
    }
    D_80106A73 = val2b;

    c = p[8] & 4;
    val = D_80106A73;
    val2c = val | 4;
    if (!c) {
        val2c = val;
    }
    D_80106A73 = val2c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
