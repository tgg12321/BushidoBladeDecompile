/*
 * func_80034F88 — REJECTED (grind session s6, forensics modality).
 * Honest sandbox score 29 at 49 build insns; the floor is 18 at 51.
 *
 * val AND val2 both split per block, c shared.  Score 29.  Together with
 * split_val2/split_val this brackets s4's 30 and shows the whole
 * live-range-surgery axis is monotonically worse than the shared form.
 *
 * RTL dumps for this form: tmp/grind/func_80034F88/s6/rtl/split_valval2/
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    s32 vala;
    s32 valb;
    s32 valc;
    u8 val2a;
    u8 val2b;
    u8 val2c;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    vala = D_80106A73;
    val2a = vala | 1;
    if (!c) {
        val2a = vala;
    }
    D_80106A73 = val2a;

    c = p[8] & 2;
    valb = D_80106A73;
    val2b = valb | 2;
    if (!c) {
        val2b = valb;
    }
    D_80106A73 = val2b;

    c = p[8] & 4;
    valc = D_80106A73;
    val2c = valc | 4;
    if (!c) {
        val2c = valc;
    }
    D_80106A73 = val2c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
