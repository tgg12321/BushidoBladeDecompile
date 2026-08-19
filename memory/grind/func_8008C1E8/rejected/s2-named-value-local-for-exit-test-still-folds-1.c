s32 func_8008C1E8(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 s0;
    s32 s5;
    s32 s1;
    s32 (*fn)(s32, s32);

    s0 = 0;
    if (*flag != 0) return -1;
    {
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 ae2_val;
        ae2_val = *p_ae2;
        s5 = *(s16 *)((s32)D_800A3074 + ((ae2_val & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    s1 = 0;
    {
        s32 remaining;

        if (D_800F1AF4 == 0) goto return_val;
        do {
            volatile s32 *loop_flag = flag;
            while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
                fn = D_800F1AE8;
                if (fn != 0) {
                    s32 prev = s0;
                    s0 += 1;
                    if (fn(2, prev) == 0) {
                        DeliverEvent(0xF000000B, 0x100);
                        goto return_val;
                    }
                }
            }
            if (s1 == 0) {
                D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
            }
            *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
            loop_flag[1] += 1;
            loop_flag[1];
            s1 += 1;
            loop_flag[2] -= 1;
            loop_flag[2];
            if (s1 == s5) {
                if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == loop_flag[3]) {
                    volatile s32 *p_af8 = &D_800F1AF8;
                    do {
                        fn = D_800F1AE8;
                        if (fn != 0) {
                            s32 prev = s0;
                            s0 += 1;
                            if (fn(2, prev) == 0) {
                                volatile s32 *p_af4b = &D_800F1AF4;
                                DeliverEvent(0xF000000B, 0x100);
                                return (arg1 - *p_af4b) - 1;
                            }
                        }
                    } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
                }
                s1 = 0;
            }
            remaining = D_800F1AF4;
        } while (remaining != 0);
    }

return_val:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
