s32 func_80017848(u8 *ctx, s32 arg1, s32 slot_a, s32 slot_b) {
    u8 *temp_v1;
    u8 *temp_a0;
    u8 *temp_a0_2;
    u8 *temp_a0_3;
    u8 *temp_a0_4;
    u8 *temp_a1;
    u8 *temp_s0_2;
    u8 *temp_s1_2;
    u8 *var_a2;
    u8 *var_a3;
    u8 *var_v0;
    u8 *var_v0_2;
    s32 temp_s0;
    s32 temp_s1;
    s32 temp_s3;
    s32 temp_s4;
    s32 temp_v0;
    s32 temp_v1_2;
    s32 temp_v1_3;
    s32 var_a1;
    s32 var_v1;
    s32 var_v1_2;

    temp_s4 = slot_a;
    temp_s3 = slot_b;
    if (temp_s4 != temp_s3) {
        temp_v1 = *(u8 **)(ctx + 0xC);
        if (*(s32 *)((temp_s4 << 6) + (s32)temp_v1 + 0x18) >= 0) {
            if (*(s32 *)((temp_s3 << 6) + (s32)temp_v1 + 0x18) >= 0) {
                goto block_3;
            }
        }
        temp_a0 = *(u8 **)(ctx + 0xC);
        var_a1 = temp_s4 << 6;
        var_v1 = 0;
        if (*(s32 *)(var_a1 + (s32)temp_a0 + 0x1C) > 0) {
            var_a3 = temp_a0;
            var_a2 = *(u8 **)(ctx + 0x10);
            temp_a0_2 = (u8 *)(var_a1 + (s32)var_a3);
            var_v0 = temp_a0_2;
        loop_6:
            var_v1 += 1;
            if (*(u16 *)((*(u8 *)(var_v0 + 0x24) << 4) + (s32)var_a2 + 4) != temp_s3) {
                var_v0 = temp_a0_2 + var_v1;
                if (var_v1 >= *(s32 *)(temp_a0_2 + 0x1C)) {
                    var_a1 = temp_s4 << 6;
                    goto block_9;
                }
                goto loop_6;
            }
            goto block_3;
        }
    block_9:
        var_v1_2 = 0;
        if (*(s32 *)(var_a1 + (s32) * (u8 **)(ctx + 0xC) + 0x20) > 0) {
            var_a3 = *(u8 **)(ctx + 0xC);
            var_a2 = *(u8 **)(ctx + 0x10);
            temp_a0_3 = (u8 *)(var_a1 + (s32)var_a3);
            var_v0_2 = temp_a0_3;
        loop_11:
            var_v1_2 += 1;
            if (*(s16 *)((*(u8 *)(var_v0_2 + 0x2C) << 4) + (s32)var_a2 + 6) != temp_s3) {
                var_v0_2 = temp_a0_3 + var_v1_2;
                if (var_v1_2 >= *(s32 *)(temp_a0_3 + 0x20)) {
                    goto block_13;
                }
                goto loop_11;
            }
            goto block_3;
        }
    block_13:
        temp_s0 = temp_s4 << 6;
        temp_a1 = *(u8 **)(ctx + 0xC);
        temp_s1 = temp_s3 << 6;
        temp_v0 = math_Distance3D((s32 *)(temp_a1 + temp_s0), (s32 *)(temp_a1 + temp_s1));
        temp_a0_4 = *(u8 **)(ctx + 0x10) + (*(s16 *)(ctx + 6) << 4);
        *(s32 *)(temp_a0_4 + 0) = temp_v0;
        *(s32 *)(temp_a0_4 + 8) = temp_v0 * 3;
        *(s32 *)(temp_a0_4 + 0xC) = arg1;
        *(s32 *)(temp_a0_4 + 4) = (temp_s4 << 0x10) | temp_s3;
        temp_s0_2 = (u8 *)(temp_s0 + (s32) * (u8 **)(ctx + 0xC));
        temp_v1_2 = *(s32 *)(temp_s0_2 + 0x1C);
        *(s32 *)(temp_s0_2 + 0x1C) = temp_v1_2 + 1;
        *(u8 *)(temp_s0_2 + temp_v1_2 + 0x24) = *(u16 *)(ctx + 6);
        temp_s1_2 = (u8 *)(temp_s1 + (s32) * (u8 **)(ctx + 0xC));
        temp_v1_3 = *(s32 *)(temp_s1_2 + 0x20);
        *(s32 *)(temp_s1_2 + 0x20) = temp_v1_3 + 1;
        *(u8 *)(temp_s1_2 + temp_v1_3 + 0x2C) = *(u16 *)(ctx + 6);
        *(s16 *)(ctx + 6) = *(u16 *)(ctx + 6) + 1;
        return 1;
    }
block_3:
    return 0;
}
