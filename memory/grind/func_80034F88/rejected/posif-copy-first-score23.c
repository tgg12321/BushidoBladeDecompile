/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 23 at 49 build insns; the floor is 18 at 51.
 *
 * Positive-sense select (`val2 = val; if (c) val2 = val|K;`).  Score 23.
 * Confirms s5's wave-A finding (positive-sense costs points) on the
 * forensic chassis; adds nothing to the register picture.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/posif/
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val;
    if (c) {
        val2 = val | 1;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val;
    if (c) {
        val2 = val | 2;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = val;
    if (c) {
        val2 = val | 4;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
