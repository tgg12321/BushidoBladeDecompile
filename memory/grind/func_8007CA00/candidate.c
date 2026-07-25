/* func_8007CA00 — candidate, sandbox --disable all floor = 11 (was 13).
 * Lever: sequence arg0[0] as the LAST load in case1 (and case2 BE77!=0) by
 * pre-subtracting arg0[2]/div from 0x400 into a temp, THEN subtracting arg0[0].
 * This makes GCC keep arg0[0] in $a0 (matching case2), fixing the case1 load
 * register/offset swap (lh v1,4(a0); lh a0,0(a0)) and removing 2 diffs.
 * Apply this body to src/display.c as the starting point next session. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    int new_var;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            s32 t = 0x400 - arg0[2];
            return t - arg0[0];
        }
        return arg0[0];
    case 2:
        if (0 != D_8009BE77) {
            s32 t = 0x400 - (((s16)(*((u16 *)(arg0 + 2)))) / 2);
            new_var = t;
            return new_var - arg0[0];
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    default:
        return arg0[0];
    }
}
