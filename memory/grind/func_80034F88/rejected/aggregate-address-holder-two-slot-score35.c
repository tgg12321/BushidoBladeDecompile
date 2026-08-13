/*
 * REJECTED (s18, synthesis) -- two-slot AGGREGATE address holder.
 * score 35 at 58 insns (target 49), lbu 175 / sb 164 / lui 456. Two slots DO
 * create two address pseudos, but the aggregate becomes frame-resident: the
 * prologue grows from `addiu sp,sp,-24` to `addiu sp,sp,-32`, the first
 * address is spilled with `sw v1,16(sp)` and reloaded per use, costing 9
 * instructions. Strictly worse than the two-SCALAR-object forms the Judge
 * banned (10 and 21) and nowhere near 0, so the aggregate family hides no
 * unexplored route to the target. The two-member-struct spelling measures
 * identically (35 / 58).
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *qa[2];

    p = func_80077D00();
    qa[0] = &D_80106A73;
    *qa[0] &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *qa[0];
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *qa[0] = c;
    }

    {
        s32 v;
        s32 c;
        qa[1] = &D_80106A73;
        c = p[8] & 2;
        v = *qa[1];
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *qa[1] = c;
    }

    {
        s32 v;
        s32 c;
        qa[1] = &D_80106A73;
        c = p[8] & 4;
        v = *qa[1];
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *qa[1] = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
