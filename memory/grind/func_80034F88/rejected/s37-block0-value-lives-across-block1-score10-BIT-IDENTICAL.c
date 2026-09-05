/* s37 (2026-09-05) DISPROVEN -- s37-block0-value-lives-across-block1-score10-BIT-IDENTICAL.c
 * Block 0's masked value kept in a local and merged by block 1 instead of being re-read, to lengthen the VALUE allocno's live range and demote it below the pointer in allocno_compare (global.c:635). 10 at 49 -- and the objdump is BIT-IDENTICAL to the base body's, so the lever is byte-inert: cse normalises the read/forward whatever the spelling.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
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
