/* REJECTED — func_80049584, session 1 (2026-08-17).
 *
 * WHY IT IS DEAD: identical to the accepted candidate in every respect except
 * that the computed total gets its own `s32 total;` instead of sharing the
 * loop-counter variable. Measured honest floor: 12 (accepted form: 0).
 *
 * The entire 12-insn residual is one fact: with a separate variable, the loop
 * counter never crosses a CALL, so GCC 2.7.2's global.c will not consider it
 * for a call-saved hard reg and it takes $a2; target keeps the counter in $s0
 * across both loops. The prologue's s0-save then reschedules, dragging the
 * lui/addiu pointer setup out of position. Declaration-order and init-order
 * permutations cannot fix this — call-saved eligibility comes from liveness
 * (reg_crosses_call), not from allocation order.
 *
 * Do NOT re-propose this form. It is the control that PROVES the accepted
 * form's single-variable reuse is load-bearing rather than decorative.
 */
void func_80049584(s32 arg0) {
    s16 *dst;
    s16 *src;
    s16 *p;
    s32 i;
    s32 unchanged;
    s32 rank;
    s32 step;
    s32 lo;
    s32 hi;
    s32 total;

    unchanged = 1;
    i = 0;
    dst = D_80099C50;
    src = D_800EF980;
    do {
        s16 v = *src;
        if ((v >= 0) != ((*dst) >= 0)) {
            unchanged = 0;
        }
        *dst = v;
        dst++;
        i++;
        src++;
    } while (i < 0x3A);
    rank = 0;
    i = 0;
    p = D_800EF980;
    do {
        if ((*p) >= 0) {
            *p = (s16) rank;
            rank++;
        }
        i++;
        p++;
    } while (i < 0x3A);
    step = 8;
    if (D_800A33EC == 0) {
        step = 0x10;
    }
    hi = D_800A33E8;
    if (hi == -1) {
        lo = D_800A33EA;
        if (lo == hi) {
            total = 0x24;
            if (D_800A33EC == 0) {
                total = 0x88;
            }
            goto end;
        }
        hi = lo;
    } else {
        lo = hi;
        if ((D_800A33EA != (-1)) && (hi != D_800A33EA)) {
            if (hi < D_800A33EA) {
                hi = D_800A33EA;
            } else {
                lo = D_800A33EA;
            }
        }
    }
    total = func_8004954C(step, lo, hi);
end:
    if (D_800A324C != total) {
        D_800A324C = total;
        unchanged = 0;
    }
    if (unchanged == 0) {
        func_80046020();
        func_80045B68(D_800A33EC, total, D_800EF980, arg0);
        func_8003E120();
    }
}
