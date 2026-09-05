/* s37 (2026-09-05) DISPROVEN -- s37-q-as-copy-loop-base-score30.c
 * candidate.c chassis with the trailing copy loop addressed off q (`q[i - 3]`, mirroring the existing src/code6cac_b.c:4044 idiom `(&D_80106A73) - 3`) to raise q's allocno reference count. 30 at 48 insns -- the loop loses its own %hi(D_80106A70) materialisation, which costs more than the priority lift buys. `*(q - 3 + i)` (h12) measures identically.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
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
        q[i - 3] = *((u8 *)p + i + 0x17);
    }
}
