s32 func_8003800C(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;
        u32 j;

        j = 0;
        bp = base + offset;
        sum = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;
        s32 k = 0;
        s32 *ap;
        u8 *a2p;

        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            do {
                *dst = *sp2;
                sp2++;
                dst++;
            } while (sp2 != end);
            k = 0;
            *(s32 *)dst = *(s32 *)sp2;
        }

        ap = (s32 *)base;
        a2p = base;
        do {
            u16 *ptr = *(u16 **)((u8 *)ap + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(a2p + 0xD0);
            }
            a2p += 2;
            k++;
            ap++;
        } while (k < 0x16);
    }

    return 1;
}
