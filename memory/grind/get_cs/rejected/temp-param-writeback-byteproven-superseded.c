/* s8 m1 — BYTE-PROVEN (sandbox 0, 51/51) but SUPERSEDED, do not propose:
 * X clamp into a natural if/else result temp `cs`, then `arg0 = cs;`
 * (no alias local — the copy's dest is the param itself, whose pre-copy
 * liveness is semantically forced as the clamp input). This reaches the
 * bytes because the writeback move survives coalescing exactly when the
 * copy dest is live across the arms (flow-conflict blocks the tie), and
 * arg0 is the only value in scope before the arms.
 *
 * Superseded because the exact published SOTN get_cs ternary text
 * (candidate.c) reaches the same bytes with NO explicit temp/writeback at
 * all — GCC 2.7.2's own COND_EXPR expansion of the `v < 0 ? 0 : (v > h ?
 * h : v)` nesting materializes the join temp internally. Adopting the
 * reference text is strictly cleaner than this hand-rolled equivalent,
 * and this form's writeback statement sits near the banned x/tx
 * live-writeback family boundary (a classification question the reference
 * text moots entirely). */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 cs;

    if (arg0 >= 0) {
        if ((D_8009BE78 - 1) < arg0) {
            cs = D_8009BE78 - 1;
        } else {
            cs = arg0;
        }
    } else {
        cs = 0;
    }
    arg0 = cs;

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
