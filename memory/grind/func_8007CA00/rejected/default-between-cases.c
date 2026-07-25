/* REJECTED (s2) — m2c's original case/default ordering (default BETWEEN case1
 * and case2, case1/BE77==0 falls through into default). sandbox floor = 25
 * (MUCH WORSE). m2c reconstructs the ORIGINAL source with this ordering, but our
 * decompals GCC 2.7.2 compiles case1->default fallthrough into a wholesale block
 * reshuffle (build_insns=44 but 25 differ). The original ordering is NOT a lever
 * under our fork. Do not re-propose default-repositioning. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            s32 t = 0x400 - arg0[2];
            return t - arg0[0];
        }
    default:
        return arg0[0];
    case 2:
        if (D_8009BE77 != 0) {
            s32 t = 0x400 - (((s16)(*((u16 *)(arg0 + 2)))) / 2);
            return t - arg0[0];
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    }
}
