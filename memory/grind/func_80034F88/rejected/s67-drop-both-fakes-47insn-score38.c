void func_80034F88(void) {
    s32 *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block 0's own address object (a second C handle on
         * D_80106A73), mechanism: global.c allocation priority
         * floor_log2(nrefs)*nrefs*10000/live_length -- blocks 1 and 2 cannot be
         * reached from this handle because global.c:1275 assigns exactly one
         * hard register per allocno and GCC 2.7.2 does no live-range splitting.
         * lever-exhaustion: memory/grind/func_80034F88/hypotheses.md s53-s65. */
        u8 *q = &D_80106A73;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
        {
            /* FAKE: the address object for flag blocks 1 and 2, mechanism:
             * global.c:1275 assigns exactly one hard register per allocno and
             * GCC 2.7.2 does no live-range splitting, so blocks 1/2 cannot be
             * reached from the block-0 object. lever-exhaustion: as above. */
            u8 *r = &D_80106A73;

            v = *r;
            c = p[8] & 2;
            if (c) {
                c = v | 2;
            } else {
                c = v;
            }
            *r = c;

            v = *r;
            c = p[8] & 4;
            if (c) {
                c = v | 4;
            } else {
                c = v;
            }
            *r = c;
        }

        /* FAKE: the copy loop's counter is staged through q, whose pointer
         * value is dead from block 0's store above and is never read again,
         * mechanism: flow.c counts REG_N_REFS per RTL insn weighted by loop
         * depth, so the loop's eleven counter references lift this allocno from
         * 5 refs / pri 3571 to 16 refs / pri 30476 and global.c seats it in $v1
         * before block 0's value allocno (pri 17500) is considered, which sends
         * that value to $a0 as the target has it.  Both values are real and
         * used; the loop adds no instruction anywhere in the function.
         * lever-exhaustion: hypotheses.md s53-s65 -- s65's branch (A), whose
         * other byte-neutral spellings (duplicated store into arms, split
         * reads, merged mask) are all banked dead. */
        for (q = 0; (s32)q < 3; q++) {
            c = *((u8 *)p + (s32)q + 0x17);
            D_80106A70[(s32)q] = c;
        }
    }
}
