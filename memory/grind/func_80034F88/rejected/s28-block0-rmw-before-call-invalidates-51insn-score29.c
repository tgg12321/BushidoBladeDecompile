/* s28 REJECTED -- block 0's read-modify-write hoisted ABOVE the
 * `p = func_80077D00()` call so that the CALL_INSN invalidates the flag byte's
 * cse memory entry (invalidate_memory, cse.c:7599) before block 1 reads it.
 * 51 insns, score 29.
 * MECHANISM CONFIRMED, PRICE REJECTS IT: the reload IS restored -- classify
 * reports the lbu multiset matching the target and the ONLY shape difference is
 * an extra `lw #,0x20(#)` / `sw #,0x20(#)` pair, i.e. the pointer now has to
 * live across the call. The call gate therefore costs +2 instructions, exactly
 * like the fourth address materialisation (a3/a4/a11/a12, 51 insns), and the
 * target's own stream has no call between its `sb` at 0x80034FAC and its `lbu`
 * at 0x80034FB4, so this was not the original's cse input either.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    q = &D_80106A73;
    *q &= 0xF8;
    p = func_80077D00();

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
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
