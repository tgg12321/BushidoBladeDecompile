/* s11 (rederive, 2026-09-09, HEAD main @ e4c60089).  SCORE 60/180 (control 6/176).
 * Transplant of the UNSPENT in-file sibling func_8005FA98 (src/text1b.c:2710, matched, same
 * S46C, same callee func_80073728): its store order is c20,c24,p0,byte28,zero1C,zero18,
 * zero10,one14,p1,ret -- zero1C early, ret last.  To keep the rand sequence intact the three
 * rand calls and both offset computations were floated ahead of all ten stores (half 2's p0
 * store stays before its rands because it READS D_800A3418).  Cost: +4 instructions.  Both
 * a0_offset and a2_offset are now live across the third/fifth rand call simultaneously, so
 * the allocator needs two more callee-saved seats and the prologue/epilogue grows.  DEAD.
 */
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
            D_800A3418 ^= rand();
            i += 1;
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x19;
            a0_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            D_800A3418 ^= rand();
            a2_offset = (s32)r4 - 0xC;
            a2_offset += ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.c20 = c100;
            s.c24 = c100;
            s.p0 = (void *)(stride + (s32)p_b2e0);
            s.byte28 = 0;
            s.zero1C = a2_offset;
            s.zero18 = a0_offset;
            s.zero10 = 0;
            s.one14 = c1;
            s.p1 = p_b388;
            s.ret = ret;
            ret = func_80073728((s32)&s, 0);

            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= rand();
            a0_offset = (s32)r5 - 0x32;
            a0_offset += ((u32)(D_800A3418 * 0x64) >> 0xF);
            D_800A3418 ^= rand();
            a2_offset = (s32)r4 - 0x19;
            a2_offset += ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.c20 = c100;
            s.c24 = c100;
            s.byte28 = 0;
            s.zero1C = a2_offset;
            s.zero18 = a0_offset;
            s.zero10 = 0;
            s.one14 = c1;
            s.p1 = p_b390;
            s.ret = ret;
            ret = func_80073728((s32)&s, 0);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
