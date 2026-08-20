s32 func_8002BEA0(void) {
    s32 temp_a3;
    s32 temp_t1;
    s32 temp_v0;
    s32 temp_v1_3;
    s32 var_a0;
    u32 temp_a0;
    u32 var_t0;
    u8 *t2_base;
    u8 *t3_base;

    temp_a3 = D_80101FBC - D_80102408;
    temp_t1 = D_80101FC4 - D_80102410;
    temp_a0 = (temp_a3 * temp_a3) + (temp_t1 * temp_t1);
    t2_base = &D_80101EC8;
    t3_base = t2_base + 0x44C;
    if (temp_a0 < 0x400U) {
        var_t0 = ((u32) (*((&D_8008D118) + temp_a0))) >> 3;
    } else {
        s32 sp_tmp;
        register s32 t4_v asm("t4");
        t4_v = (s32) temp_a0;
        __asm__ volatile(".word 0x488CF000" : : "r"(t4_v));
        __asm__ volatile("nop");
        __asm__ volatile("nop");
        t4_v = (s32) (&sp_tmp);
        __asm__ volatile(".word 0xE99F0000" : : "r"(t4_v));
        {
            u32 clz = sp_tmp;
            u32 v0_m = clz & (-2);
            u32 v1_m = 0x16 - v0_m;
            u32 idx = temp_a0 >> v1_m;
            u32 hi = (u32) ((u8) (*((&D_8008D118) + idx)));
            do { v0_m = 0x13 - (v1_m >> 1); var_t0 = (hi << 16) >> v0_m; } while (0);
        }
    }
    if (((s32) var_t0) < 0x44C) {
        var_a0 = ((0x44C - ((s32) var_t0)) * 0x50) / 100;
    } else {
        s32 v1_2 = 0x44C - ((s32) var_t0);
        var_a0 = v1_2 >> 4;
        if (v1_2 < 0) {
            var_a0 = (v1_2 + 0xF) >> 4;
        }
    }
    temp_v0 = 0x3E8;
    __asm__ volatile("" : "=r"(temp_v0) : "0"(temp_v0));
    temp_v1_3 = -var_a0;
    *((s32 *) (t2_base + 0x134)) = (temp_a3 * var_a0) / temp_v0;
    *((s32 *) (t2_base + 0x13C)) = (temp_t1 * var_a0) / temp_v0;
    *((s32 *) (t3_base + 0x134)) = (temp_a3 * temp_v1_3) / temp_v0;
    *((s32 *) (t3_base + 0x13C)) = (temp_t1 * temp_v1_3) / temp_v0;
    return (s32) var_t0 - 0x44C;
}
