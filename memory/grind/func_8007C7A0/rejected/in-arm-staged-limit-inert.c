/* REJECTED-AS-INERT s14 (2026-08-10, permuter, driver session 9): in-arm
 * staged limit temp — the dominant attractor of the s9 RANDOM campaign
 * (weighted 450-515 class, ws permuter/c7a0_s9_from5, output-450-1 /
 * output-505-1). Honest sandbox in display.c context: 5 @ 50 — identical
 * to the clean baseline; the temp coalesces into arg0 and the COND_EXPR
 * join temp does NOT materialize. Distinct from BOTH the hoisted named-limit
 * (s13 Q1: declared+initialized before the clamp, 16 @ 51) and the banned
 * three-arm join-writeback family (the temp here holds only the LIMIT in
 * one arm, never the clamp result across arms). Not a candidate, not a
 * gradient — banked so no future session re-measures it. */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 t;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            t = D_8009BE78 - 1;
            arg0 = t;
        }
    } else {
        arg0 = 0;
    }

    if (arg1 >= 0) {
        if ((D_8009BE7A - 1) < arg1) {
            arg1 = D_8009BE7A - 1;
        }
    } else {
        arg1 = 0;
    }

    if ((u32)(D_8009BE74 - 1) < 2U) {
        s32 hi = arg1 & 0xFFF;
        s32 lo;
        hi = hi << 12;
        lo = arg0 & 0xFFF;
        lo = lo | 0xE3000000;
        return hi | lo;
    } else {
        s32 hi = arg1 & 0x3FF;
        s32 lo;
        hi = hi << 10;
        lo = arg0 & 0x3FF;
        lo = lo | 0xE3000000;
        return hi | lo;
    }
}
