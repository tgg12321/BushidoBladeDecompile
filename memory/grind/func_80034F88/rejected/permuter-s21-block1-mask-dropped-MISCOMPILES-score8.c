/*
 * REJECTED — s21 (permuter), seed 2, decomp-permuter find ws2/output-355-1.
 * SCORE 8 at 50 insns, lbu 176 / sb 164. DOUBLY dead; never re-credit this 8.
 *
 * 1. IT MISCOMPILES. Block 1 reads `c = p[8];` — the permuter DELETED the
 *    `& 1`. The condition then tests every bit of p[8], not bit 0, so the
 *    bit-0 flag is set whenever any of p[8]'s bits are set. This is the same
 *    failure mode as s4's only other sub-floor find
 *    (rejected/permuter-17-store-hoisted-into-arm-MISCOMPILES.c): the permuter
 *    is free to change semantics, so EVERY sub-floor find on this function has
 *    to be read for meaning before its score is believed. Two out of three
 *    sub-floor finds in the function's history have been semantic breaks.
 *
 * 2. EVEN IF IT COMPUTED THE RIGHT THING it carries `new_var2 = q;`, the
 *    BANNED second C pointer object by copy (Judge constraint: "no form may
 *    declare or assign more than ONE C pointer object aliasing D_80106A73 ...
 *    `q1 = qm;` copies ... are all out"), plus a `new_var = 0x17;` constant
 *    holder in the copy loop.
 *
 * Its admissible sibling (same copy lever, `& 1` intact) is banked at
 * rejected/permuter-s21-pointer-copy-second-object-score8-BANNED.c, which is
 * where the real measurement lives.
 */
void func_80034F88(void) {
    int new_var;
    s32 *p;
    s32 i;
    u8 *q;
    u8 *new_var2;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8];               /* MISCOMPILE: the `& 1` is gone */
        v = D_80106A73;
        new_var2 = q;           /* BANNED: second C pointer object (copy) */
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *new_var2 = c;
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
        new_var = 0x17;         /* constant holder */
        *(&D_80106A70 + i) = *((u8 *)p + i + new_var);
    }
}
