/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 23 at 51 build insns; the floor is 18 at 51.
 *
 * val split into three per-block locals, val2 and c shared.  Score 23.
 * The loaded-byte pseudos acquire an explicit `preferences: 3` ($v1) and
 * the selected value goes to $a0.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/split_val/
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 vala;
    s32 valb;
    s32 valc;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    vala = D_80106A73;
    val2 = vala | 1;
    if (!c) {
        val2 = vala;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    valb = D_80106A73;
    val2 = valb | 2;
    if (!c) {
        val2 = valb;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    valc = D_80106A73;
    val2 = valc | 4;
    if (!c) {
        val2 = valc;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
