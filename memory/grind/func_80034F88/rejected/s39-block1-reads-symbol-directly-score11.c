/* s39: v2 chassis, block 1 reads D_80106A73 by symbol (not through q) so cse cannot forward block 0's store. 11 at 50. Instrumented cc1: m DISAPPEARS from the global allocno table (it becomes a single-basic-block quantity again) and local-alloc re-seats it in $v1, restoring exactly the hard-reg-3 conflict on q that the v2 chassis removes. This is the second horn of the s39 dichotomy. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = D_80106A73;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = *q;
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
