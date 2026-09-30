void func_80075830(s32 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    u8 packet[0x2C];
    s16 var_a1;
    s16 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    *(s32 *)(packet + 0x10) = arg3;
    temp_v1 = ((s32) (rsin(((*(u16 *)((s32)D_800A36A0 + 0x34) & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;
    *(s8 *)(packet + 0x2B) = temp_v1;
    *(s8 *)(packet + 0x2A) = temp_v1;
    *(s8 *)(packet + 0x29) = temp_v1;
    temp_v0 = *(s32 *)(*(s32 *)(*arg0 + 0x14) + 0x54);
    *(s32 *)(packet + 0x00) = temp_v0;
    *(s32 *)(packet + 0x04) = temp_v0 + 0xC;
    if (arg1 < 0xA) {
        var_a1 = arg1 / 5;
        var_a2 = arg1 % 5;
    } else {
        var_a1 = (arg1 - 0xA) / 5;
        var_a2 = (arg1 - 0xA) % 5;
    }
    if (SELWORK->f1C.half[arg2] == var_a1 && SELWORK->f20.half[arg2] == var_a2) {
        *(s8 *)(packet + 0x28) = 1;
    } else {
        *(s8 *)(packet + 0x28) = 0;
    }
    *(s32 *)(packet + 0x18) = arg2 * 0xF0 + var_a1 * 0x64;
    *(s32 *)(packet + 0x1C) = var_a2 * 16;
    if (arg2 != 0) {
        *(s32 *)(packet + 0x14) = 0x13;
    } else {
        *(s32 *)(packet + 0x14) = 9;
    }
    *(s32 *)(packet + 0x08) = arg0[0x10 / 4];
    arg0[0x10 / 4] = func_8007352C((s32)packet);
}
