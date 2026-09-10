static __inline__ s32 hlf(S46C *s, s32 ret, s32 r5, s32 r4, s32 c1,
                          s32 ba0, s32 ma0, s32 ba2, s32 ma2) {
    extern s32 rand(void);
    extern s32 D_800A3418;
    s32 a0_offset;
    s32 a2_offset;

    D_800A3418 ^= rand();
    a0_offset = r5 - ba0;
    a0_offset += ((u32)(D_800A3418 * ma0) >> 0xF);
    s->zero18 = a0_offset;
    D_800A3418 ^= rand();
    a2_offset = r4 - ba2;
    a2_offset += ((u32)(D_800A3418 * ma2) >> 0xF);
    s->zero10 = 0;
    s->one14 = c1;
    s->ret = ret;
    s->zero1C = a2_offset;
    return func_80073728((s32)s, 0);
}
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

    if (i < ((D_800A326C + 1) * 2)) {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        p_b2ec = p_b2e0 + 0xC;
        base_offset = (s32 *)(p_b2ec + stride);
        p_b388 = &D_8009B388;
        p_b390 = p_b388 + 2;
        do {
            s.byte28 = 0;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.p1 = p_b388;
            D_800A3418 ^= rand();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            ret = hlf(&s, ret, (s32)r5, (s32)r4, c1, 0x19, 0x32, 0xC, 0x19);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = p_b390;
            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            ret = hlf(&s, ret, (s32)r5, (s32)r4, c1, 0x32, 0x64, 0x19, 0x32);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
