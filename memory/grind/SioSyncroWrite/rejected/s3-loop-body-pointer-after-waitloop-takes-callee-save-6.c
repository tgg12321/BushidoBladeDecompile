s32 SioSyncroWrite(u8 *arg0, s32 arg1) {
    volatile s32 *flag = &D_800F1AEC;
    s32 retries;
    s32 pkt_len;
    s32 i;
    s32 (*cb)(s32, s32);
    volatile s32 *p_af4c;

    retries = 0;
    if (*flag != 0) return -1;
    {
        volatile u16 *p_ae2 = &D_800F1AE2;
        u32 mode;
        mode = *p_ae2;
        pkt_len = *(s16 *)((s32)D_800A3074 + ((mode & 0x300) >> 7));
    }
    D_800F1AF4 = arg1;
    D_800F1AF0 = (s32)arg0;
    i = 0;
    if (D_800F1AF4 != 0) do {
        volatile s32 *st = flag;

        while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 5) != 5) {
            cb = D_800F1AE8;
            if (cb != 0) {
                s32 prev = retries;
                retries += 1;
                if (cb(2, prev) == 0) {
                    DeliverEvent(0xF000000B, 0x100);
                    goto done;
                }
            }
        }
        p_af4c = &D_800F1AF4;
        if (i == 0) {
            D_800F1AF8 = (*((volatile u16 *)(((s32)D_800A3044) + 4))) & 0x80;
        }
        *((u8 *)D_800A3044) = *((u8 *)D_800F1AF0);
        st[1] += 1;
        st[1];
        i += 1;
        st[2] -= 1;
        st[2];
        if (i == pkt_len) {
            if ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == st[3]) {
                volatile s32 *p_af8 = &D_800F1AF8;
                do {
                    cb = D_800F1AE8;
                    if (cb != 0) {
                        s32 prev = retries;
                        retries += 1;
                        if (cb(2, prev) == 0) {
                            volatile s32 *p_af4b = &D_800F1AF4;
                            DeliverEvent(0xF000000B, 0x100);
                            return (arg1 - *p_af4b) - 1;
                        }
                    }
                } while ((*((volatile u16 *)(((s32)D_800A3044) + 4)) & 0x80) == *p_af8);
            }
            i = 0;
        }
    } while (*p_af4c != 0);

done:
    {
        volatile s32 *p_af4 = &D_800F1AF4;
        return arg1 - *p_af4;
    }
}
