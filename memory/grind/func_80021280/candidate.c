void func_80021280(s32 a0) {
    s32 a1 = 0;
    u8 *a2 = (u8 *)&D_80101EC8 + a0 * 1100;
    u16 a3 = *(u16 *)(a2 + 0x48);
    u16 *v1 = (u16 *)&D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = *(u16 *)(a2 + 0x48);
        *(s16 *)(a2 + 0x4A) = a1;
        *(s16 *)(a2 + 0x4C) = 0;

        if ((u32)(val >> 12) < 2) {
            u16 t1 = val;
            s32 t4 = 4;
            s32 t3 = 3;
            s32 t2 = 1;
            s32 mode = D_800A38DC;
            u8 t0 = D_800A384C;

            a1 = 0;
        loop2_21280:
            {
                u16 nibble = (t1 >> (a1 << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                *(s16 *)(a2 + 0x88) = a1;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                *(u16 *)(a2 + 0x8A) = *(u16 *)(a2 + 0x26C);
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                *(s16 *)(a2 + 0x8E) = a1;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) goto next_21280;
            store5_21280:
                *(u16 *)(a2 + 0x90) = *(u16 *)(a2 + 0x26C);
            }
        next_21280:
            a1++;
            if (a1 < 3) goto loop2_21280;
        }
    }
}
