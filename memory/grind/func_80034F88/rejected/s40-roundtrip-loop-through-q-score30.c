/* s40 (synthesis, 2026-09-05) DISPROVEN -- s40-roundtrip-loop-through-q-score30.c
 * Round-trip chassis with the trailing copy loop addressed off q (`q[i-3]`) to raise q above m in allocno_compare. It DOES work numerically -- q rises to nrefs=12 livelen=36 pri=10000 and the loop counter (73) is displaced from $v1 to $a0 -- but q lands in $a1, not $v1, and the loop loses its own %hi materialisation. 30 at 48. Re-measures s37-q-as-copy-loop-base on the new chassis.
 */
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
        q[i - 3] = *((u8 *)p + i + 0x17);
    }
}
