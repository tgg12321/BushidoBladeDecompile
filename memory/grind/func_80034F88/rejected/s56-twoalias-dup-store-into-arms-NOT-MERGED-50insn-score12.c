/* s56 (solver, 2026-09-05) -- REJECTED: 50 insns / score 12.
 * Block-0's store duplicated into BOTH arms of the two-alias TARGET-VALUE-
 * PAIRING chassis.  jump2's cross_jump did NOT re-merge the copies: the build
 * grows to 50 insns and the branch flips (ours `sb; beqz; j` vs target `bnez`).
 * Kept as the MEASUREMENT of the duplicated-statement-into-arms ref-lift on
 * this function: one byte-costing duplication buys reg_n_refs(handle A) 5 -> 6
 * and reg_live_length 28 -> 24 (pri 3571 -> 5000).  The global.c threshold for
 * the pointer allocno to outrank the mask allocno (pri 17500) is nrefs >= 16 at
 * len 28 / >= 15 at len 24, i.e. ten more duplications -- ten more instructions.
 * Side finding worth keeping: on this shape p (pseudo 72) already reaches its
 * TARGET seat $a1 and the pointer reaches $a0, so the residual collapses from a
 * three-cycle rotation to the single 74<->75 swap.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;

    p = func_80077D00();

    {
        /* FAKE: handle A */
        u8 *t = &D_80106A73;
        s32 c;

        m = *t;
        m &= 0xF8;
        *t = m;

        c = p[8];
        m = c;
        m = *t;
        c &= 1;
        if (c) {
            c = m | 1;
            *t = c;
        } else {
            c = m;
            *t = c;
        }
    }

    {
        /* FAKE: handle B */
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        v = *q;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
