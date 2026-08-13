/*
 * REJECTED (s18, synthesis) -- single-slot AGGREGATE address holder.
 * score 10 at 49 insns, lbu 175 / sb 164 / lui 456: byte-for-byte the SAME
 * instruction stream as the scalar `u8 *q` candidate (diff of the objdumps is
 * empty). GCC 2.7.2 scalarises a single-slot, address-never-taken aggregate
 * into exactly ONE pseudo, so this is the scalar form spelled longer -- not a
 * route to the second address register the target needs. The struct spelling
 * (`struct { u8 *b; } s;`) measures identically. Closes the untested step of
 * the ceiling proof for the single-slot case.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *qa[1];

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
        qa[0] = &D_80106A73;
        c = p[8] & 2;
        v = *qa[0];
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *qa[0] = c;
    }

    {
        s32 v;
        s32 c;
        qa[0] = &D_80106A73;
        c = p[8] & 4;
        v = *qa[0];
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *qa[0] = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
