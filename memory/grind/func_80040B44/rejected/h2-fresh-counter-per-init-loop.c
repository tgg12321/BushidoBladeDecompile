/* REJECTED (s2, floor 27 -> 24, partial only). Fresh counter AND fresh pointer per
 * init loop fixes loop-2's pointer seat ($v0) but leaves both counters in the wrong
 * seats (a1 / v1 vs target's a0 for BOTH). Target reuses ONE counter variable across
 * both init loops and uses fresh POINTERS - see candidate.c. */
void func_80040B44(s32 *arg0) {
    s32 seen[18];
    s32 *t5;
    s32 *t7;
    s32 *v1;
    s32 *t3;
    u16 a0_val;
    s32 *t4;

    t5 = (s32 *)((u8 *)arg0 + 0x10D4);
    t7 = (s32 *)((u8 *)arg0 + 0x94);
    v1 = *(s32 **)((u8 *)arg0 + 0x1C);
    t3 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));

    {
        s32 i1 = 0x11;
        s32 *p1 = &seen[17];
        do {
            *p1 = 0;
            i1--;
            p1--;
        } while (i1 >= 0);
    }
    {
        s32 i2 = 0x13;
        s32 *p2 = (s32 *)((u8 *)arg0 + 0x4C);
        do {
            *(s32 *)((u8 *)p2 + 0x1A34) = 0;
            i2--;
            p2 = (s32 *)((u8 *)p2 - 4);
        } while (i2 >= 0);
    }

    a0_val = *(u16 *)t3;
    if ((a0_val & 0xFFFF) == 0xFFFF) goto done;

    {
        s32 one = 1;
        t4 = (s32 *)((u8 *)t5 + 0x58);
        t3 = (s32 *)((u8 *)t3 + 2);

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
                *a1 = one;
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
