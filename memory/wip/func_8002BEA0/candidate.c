s32 func_8002BEA0(void) {
    s32 temp_a3;
    s32 temp_t1;
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
        __asm__ volatile(
            "addu   $t4, %1, $zero\n"
            "mtc2   $t4, $30\n"
            "nop\n"
            "nop\n"
            "addu   $t4, $sp, $zero\n"
            "swc2   $31, 0($t4)\n"
            : "=m"(sp_tmp)
            : "r"(temp_a0)
            : "$12");
        {
            u32 v0_m = (u32)-2;
            u32 v1_m;
            u32 idx;
            u32 hi;
            v0_m &= sp_tmp;
            v1_m = 0x16 - v0_m;
            idx = temp_a0 >> v1_m;
            v1_m = v1_m >> 1;
            hi = (u32)((u8)(*((&D_8008D118) + idx)));
            var_t0 = (hi << 16) >> (0x13 - v1_m);
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
    {
        s32 temp_v0 = 0x3E8;
        s32 temp_v1_3 = -var_a0;
        *((s32 *) (t2_base + 0x134)) = (temp_a3 * var_a0) / temp_v0;
        *((s32 *) (t2_base + 0x13C)) = (temp_t1 * var_a0) / temp_v0;
        *((s32 *) (t3_base + 0x134)) = (temp_a3 * temp_v1_3) / temp_v0;
        *((s32 *) (t3_base + 0x13C)) = (temp_t1 * temp_v1_3) / temp_v0;
    }
    return (s32) var_t0 - 0x44C;
}
