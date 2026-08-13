/*
 * s16 REDERIVE. m2c's fresh reconstruction of the target computes the OR first
 * and lets the else-arm override (`v = *q | bit; if (!(p[8] & bit)) v = *q;`)
 * instead of a symmetric if/else. Carried onto the single-pointer chassis this
 * scores 21 at 45 build insns (target 49): GCC folds the two reads of *q into
 * one and then collapses the diamond, LOSING four instructions the target has.
 * The m2c value shape is therefore NOT the original's; the symmetric if/else in
 * candidate.c is. lbu 173 vs the target's 176.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;
    {
        u8 v;

        v = *q | 1;
        if (!(p[8] & 1)) {
            v = *q;
        }
        *q = v;
    }
    q = &D_80106A73;
    {
        u8 v;

        v = *q | 2;
        if (!(p[8] & 2)) {
            v = *q;
        }
        *q = v;
    }
    q = &D_80106A73;
    {
        u8 v;

        v = *q | 4;
        if (!(p[8] & 4)) {
            v = *q;
        }
        *q = v;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
