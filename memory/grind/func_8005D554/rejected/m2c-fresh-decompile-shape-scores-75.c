s32 func_8005D554(s32 arg0, s32 arg1) {
    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 var_v0;
    s32 var_s0;
    s32 var_s1;
    s32 var_s2;
    s32 temp_s0;
    u32 temp_s4;
    u32 temp_s5;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 temp_v1_4;
    s32 temp_v1_5;
    s32 temp_v1_6;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_v0_3;
    u8 *sp40;

    var_s0 = arg1;
    var_v0 = D_800A326C;
    if (D_800A326C < 0) {
        var_v0 = D_800A326C + 3;
    }
    D_800A326C -= (var_v0 >> 2) * 4;
    var_s1 = arg0;
    if (var_s0 > 0) {
        var_s0 -= 1;
    }
    var_s2 = 0;
    temp_v1 = D_800A3418 ^ rand();
    D_800A3418 = temp_v1;
    temp_s5 = (u32)(temp_v1 * 0x260) >> 0xF;
    temp_v1_2 = D_800A3418 ^ rand();
    D_800A3418 = temp_v1_2;
    temp_s4 = (u32)(temp_v1_2 * 0xDC) >> 0xF;
    if (((D_800A326C + 1) * 2) > 0) {
        temp_s0 = var_s0 * 0x3C;
        sp40 = (u8 *)(temp_s0 + ((s32)&D_8009B2E0 + 0xC));
        do {
            s.byte28 = 0;
            s.p0 = (void *)(temp_s0 + (s32)&D_8009B2E0);
            s.p1 = &D_8009B388;
            D_800A3418 ^= rand();
            s.c24 = 0x100;
            s.c20 = 0x100;
            var_s2 += 1;
            temp_v1_3 = D_800A3418 ^ rand();
            temp_a0 = temp_s5 - 0x19 + ((u32)(temp_v1_3 * 0x32) >> 0xF);
            D_800A3418 = temp_v1_3;
            s.zero18 = temp_a0;
            temp_v1_4 = D_800A3418 ^ rand();
            temp_a2 = temp_s4 - 0xC + ((u32)(temp_v1_4 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = 1;
            s.ret = var_s1;
            D_800A3418 = temp_v1_4;
            s.zero1C = temp_a2;
            temp_v0_3 = func_80073728((s32)&s, 0);

            s.p1 = &D_8009B390;
            s.byte28 = 0;
            s.c24 = 0x100;
            s.c20 = 0x100;
            s.p0 = (void *)(sp40 + ((D_800A3418 & 1) * 0xC));
            temp_v1_5 = D_800A3418 ^ rand();
            temp_a0_2 = temp_s5 - 0x32 + ((u32)(temp_v1_5 * 0x64) >> 0xF);
            D_800A3418 = temp_v1_5;
            s.zero18 = temp_a0_2;
            temp_v1_6 = D_800A3418 ^ rand();
            temp_a2_2 = temp_s4 - 0x19 + ((u32)(temp_v1_6 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = 1;
            s.ret = temp_v0_3;
            D_800A3418 = temp_v1_6;
            s.zero1C = temp_a2_2;
            var_s1 = func_80073728((s32)&s, 0);
        } while (var_s2 < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return var_s1;
}
