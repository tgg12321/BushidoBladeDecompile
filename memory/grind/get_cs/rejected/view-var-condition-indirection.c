/* func_8007C7A0 — REJECTED s12 (2026-08-10, structural): view-local
 * indirection of the X-clamp condition reads. Measured on the s6 floor-5
 * chassis, honest sandbox in display.c context.
 *
 * P1 (statement-arm form): score 5, build 50 — identical to the clean
 * baseline; `x` coalesces with arg0 and the pass-through `arg0 = x;` is
 * elided. NOT a win, NOT in the banned family — just a synonym of the
 * clean 5-form.
 *
 * P2 (ternary form, below): score 5, build 50 — the DECISIVE kill. The
 * banned published-ternary (H21, sandbox 0) reads the assignment target
 * in BOTH the conditions and the innermost else. P2 keeps the else-arm
 * target read but routes the conditions through the view local — and the
 * three-arm $v0 join + `move a3,v0` writeback does NOT materialize.
 * Mechanism (measured, not assumed): GCC 2.7.2's COND_EXPR expansion
 * spills to a join temp only when the assignment TARGET is read in the
 * CONDITION position. So there is no "conditions read a copy" escape
 * spelling: view-variable indirection lands back on the clean 5-form,
 * and the 0-reaching spelling space remains exactly the banned family.
 * Do not re-propose either shape as a route to 0. */

/* P2 — the ternary form: */
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    {
        s16 x = arg0;
        arg0 = (x < 0) ? 0 : (((D_8009BE78 - 1) < x) ? (D_8009BE78 - 1) : arg0);
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
