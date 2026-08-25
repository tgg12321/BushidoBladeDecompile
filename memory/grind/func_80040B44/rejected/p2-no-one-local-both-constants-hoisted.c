/* REJECTED (s2, floor 7, WORSE than p1's 6). Deleting the `s32 one = 1;` local makes
 * loop.c hoist BOTH li 1 and li 0xffff, and move_movables emits them immediately
 * before NOTE_INSN_LOOP_BEG - i.e. at the END of the preheader, after the t4 setup.
 * Target has li t9,1 FIRST. NOTE: this rejection is chassis-relative to the p1
 * shape; once t4 became a strength-reduction giv (candidate.c) removing `one` is
 * byte-NEUTRAL and the holder is gone for good. */
void func_80040B44(s32 *arg0) {
    s32 seen[18];
    s32 *t5;
    s32 *t7;
    s32 *v1;
    s32 *t3;
    s32 *p0;
    u16 a0_val;
    s32 *t4;
    s32 i;

    t5 = (s32 *)((u8 *)arg0 + 0x10D4);
    t7 = (s32 *)((u8 *)arg0 + 0x94);
    v1 = *(s32 **)((u8 *)arg0 + 0x1C);
    p0 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));

    {
        s32 *p1;
        i = 0x11;
        p1 = &seen[17];
        do {
            *p1 = 0;
            i--;
            p1--;
        } while (i >= 0);
    }
    {
        s32 *p2;
        i = 0x13;
        p2 = (s32 *)((u8 *)arg0 + 0x4C);
        do {
            *(s32 *)((u8 *)p2 + 0x1A34) = 0;
            i--;
            p2 = (s32 *)((u8 *)p2 - 4);
        } while (i >= 0);
    }

    a0_val = *(u16 *)p0;
    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;

    {
        t4 = (s32 *)((u8 *)t5 + 0x58);
        t3 = (s32 *)((u8 *)p0 + 2);

        do {
            s32 a3 = *(u16 *)t3;
            s32 *a1;
            s32 t2;

            t3 = (s32 *)((u8 *)t3 + 2);
            a1 = (s32 *)(a3 * 4 + (s32)&seen[0]);
            t2 = a0_val & 0xFFFF;

            if (*a1 == 0) {
                s32 *slot = (s32 *)(a3 * 0x68 + (s32)t7);
                *(s16 *)((u8 *)slot + 2) = (s16)t2;
                *a1 = 1;
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)slot;
            } else {
                s32 *t0 = t5;
                s32 *a2 = (s32 *)(a3 * 0x68 + (s32)t7);
                s32 *end = (s32 *)((u8 *)a2 + 0x60);

                do {
                    *(Copy16 *)t0 = *(Copy16 *)a2;
                    a2 = (s32 *)((u8 *)a2 + 0x10);
                    t0 = (s32 *)((u8 *)t0 + 0x10);
                } while (a2 != end);

                *(Copy8 *)t0 = *(Copy8 *)a2;

                *(s16 *)((u8 *)t4 - 0x56) = (s16)t2;
                *(s32 *)t4 = (s32)((u8 *)t7 + a3 * 0x68);
                t4 = (s32 *)((u8 *)t4 + 0x68);
                *(s32 *)((t2 << 2) + (s32)arg0 + 0x1A34) = (s32)t5;
                t5 = (s32 *)((u8 *)t5 + 0x68);
            }

            a0_val = *(u16 *)t3;
            t3 = (s32 *)((u8 *)t3 + 2);
        } while (a0_val != 0xFFFF);
        t3 = (s32 *)((u8 *)t3 - 2);
    }
done:
    *(s32 *)((u8 *)t5 + 0x58) = 0;
}
