/* s38 (synthesis, 2026-09-05) — DISPROVEN, score 12 at 49 insns.
 * Array-declared D_80106A70 (extern u8 D_80106A70[3];) with the copy loop's
 * SOURCE byte also spelled as an index, `((u8 *)p)[i + 0x17]`, instead of
 * `*((u8 *)p + i + 0x17)`.  The destination-side array spelling is
 * codegen-neutral (10/49, bit-identical — see candidate_arraydecl_pun_free.c);
 * the source-side one is not, costing 2 points at the same instruction count.
 * Keep the pointer-arithmetic spelling for the source operand.
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
        v = (*q);
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        (*q) = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = (*q);
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        (*q) = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 4;
        v = (*q);
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        (*q) = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = ((u8 *)p)[i + 0x17];
    }
}
