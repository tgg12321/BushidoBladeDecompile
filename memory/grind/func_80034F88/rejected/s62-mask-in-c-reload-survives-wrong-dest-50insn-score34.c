void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 v;
    s32 c;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, mechanism: global.c:1275 assigns
         * exactly one hard register per allocno and GCC 2.7.2 does no
         * live-range splitting, so the target's two address seats ($v1 in
         * block 0, $a0 in blocks 1-2) are only reachable with two allocnos.
         * lever-exhaustion: the single-object chassis is measured flat at 10
         * across s53-s60 (memory/grind/func_80034F88/hypotheses.md), and the
         * pointer-object-free array spelling costs an instruction
         * (rejected/s59b-...-50insn-score24.c). */
        u8 *q = &D_80106A73;
        s32 u;

        c = *q;
        c = c & 0xF8;
        *q = c;
        c = p[8] & 1;
        u = *q;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
    }
    {
        /* FAKE: the address object for flag blocks 1 and 2, mechanism: as
         * above -- a second allocno is what lets $a0 carry blocks 1-2 while
         * $v1 carries block 0.  lever-exhaustion: as above. */
        u8 *r = &D_80106A73;

        v = *r;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *r = c;

        r = &D_80106A73;
        v = *r;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *r = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
