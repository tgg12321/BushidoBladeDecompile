/* REJECTED — explicit goto to a single shared tail. sandbox floor = 17 (WORSE
 * than the 13 baseline / 11 candidate). Forcing structural label-sharing makes
 * GCC's layout+RA worse than letting it duplicate the tail naturally. Do NOT
 * re-propose explicit goto/label-sharing for the CA40 tail. */
s32 func_8007CA00(s16 *arg0) {
    s32 v1;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            v1 = arg0[2];
            goto sub;
        }
        return arg0[0];
    case 2:
        if (D_8009BE77 != 0) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
        sub:
            return (0x400 - v1) - arg0[0];
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    default:
        return arg0[0];
    }
}
