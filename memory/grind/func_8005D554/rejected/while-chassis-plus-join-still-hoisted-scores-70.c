s32 func_8005D554(s32 arg0, s32 arg1) {
    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 v3;
    s32 v0;
    s32 i;
    u32 r5;
    u32 r4;
    s32 ret;
    s32 stride;
    s32 a0_offset;
    s32 a2_offset;
    s32 base1;
    s32 base2;
    s32 c100;
    s32 c1;
    s32 *p_b388;
    s32 *p_b390;
    u8 *p_b2e0;
    u8 *p_b2ec;
    s32 *base_offset;

    v3 = D_800A326C;
    v0 = v3;
    if (v3 < 0) v0 = v3 + 3;
    D_800A326C -= (v0 >> 2) * 4;

    ret = arg0;
    if (arg1 > 0) arg1 -= 1;
    i = 0;

    D_800A3418 ^= rand();
    r5 = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    r4 = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        p_b2ec = p_b2e0 + 0xC;
        base_offset = (s32 *)(p_b2ec + stride);
        p_b388 = &D_8009B388;
        p_b390 = p_b388 + 2;
        while (i < ((D_800A326C + 1) * 2)) {
            s.byte28 = 0;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.p1 = p_b388;
            D_800A3418 ^= rand();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x19;
            a0_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            base1 = (s32)r4 - 0xC;
            a2_offset = base1 + ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = p_b390;
            if (D_800A3418 & 1) {
                s.p0 = (void *)((u8 *)base_offset + 0xC);
            } else {
                s.p0 = (void *)base_offset;
            }
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x32;
            a0_offset += ((u32)(D_800A3418 * 0x64) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= rand();
            base2 = base1 - 0xD;
            a2_offset = base2 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((s32)&s, 0);
        }
    }
    D_800A326C += 1;
    return ret;
}
