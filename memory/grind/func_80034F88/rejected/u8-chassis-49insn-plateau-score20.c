/*
 * REJECTED (s10, synthesis) — from u0_u8_chassis.c
 *
 * s10 wave U — the 49-instruction `u8 val` chassis, which s9's frontier named as
 * the one axis no session since s3 had re-explored, is DEAD at 20.  Thirteen
 * forms crossed it with every lever found in s4-s9 (two-reads-per-block,
 * outer-symbol/arm-pointer reads, staged pointer bases, pointer/symbol mixes per
 * access, explicit-RMW mask, s32 selected value, per-block read pointers):
 * u0 20/49, u1 20/49, u9 20/49, u10 20/49, u4 23/48, u7 24/48, u3 24/48,
 * u5 24/48, u8 24/49, u6 25/49, u11 25/49, u12 25/49, u2 29/47.
 * Nothing on this chassis reaches the then-floor of 18, let alone s10's 9.
 * The chassis is not the axis; the value/condition variable-reuse spelling is
 * (see candidate.c).
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val = D_80106A73;
    val2 = val | 2;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val = D_80106A73;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    D_80106A73 = val2;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
