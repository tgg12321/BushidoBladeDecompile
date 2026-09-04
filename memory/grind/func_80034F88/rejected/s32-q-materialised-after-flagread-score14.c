/*
 * s32 (structural) -- q's materialisation DELAYED below block 1's flag-word
 * read (`c = p[8] & 1;` first, then `q = &D_80106A73; *q &= 0xF8;`), single
 * pointer object, three assignments, no FAKE construct.
 *
 * Sandbox score 14 at 49 build insns / 49 target insns (base is 10/49).
 * .greg (tmp/grind/func_80034F88/s32/greg_v2.txt): the address allocno 74 still
 * prints `74 conflicts: ... 2 3 29` and is still seated `74 in 4` ($a0).  The
 * local-alloc seats merely PERMUTE (76 in 3 / 75 in 2, versus base's 75,76 in 3
 * / 79 in 2): block 0 always retains at least two local quantities that overlap
 * 74's live range, so the hard-reg conflict on $v1 survives the reorder.
 * Costs 4 points for no allocator movement.  DEAD.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        q = &D_80106A73;
        *q &= 0xF8;
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
