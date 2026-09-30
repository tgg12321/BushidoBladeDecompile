void func_80075830(s32 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    S_80074488 s;
    s16 var_a1;
    s16 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    s.sp28 = arg3;
    temp_v1 = ((s32) (rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;
    s.sp43 = temp_v1;
    s.sp42 = temp_v1;
    s.sp41 = temp_v1;
    temp_v0 = *(s32 *)(*(s32 *)(*arg0 + 0x14) + 0x54);
    s.sp18 = temp_v0;
    s.sp1C = temp_v0 + 0xC;
    if (arg1 < 0xA) {
        var_a1 = arg1 / 5;
        var_a2 = arg1 % 5;
    } else {
        var_a1 = (arg1 - 0xA) / 5;
        var_a2 = (arg1 - 0xA) % 5;
    }
    if (SELWORK->f1C.half[arg2] == var_a1 && SELWORK->f20.half[arg2] == var_a2) {
        s.sp40 = 1;
    } else {
        s.sp40 = 0;
    }
    s.sp30 = arg2 * 0xF0 + var_a1 * 0x64;
    s.sp34 = var_a2 * 16;
    if (arg2 != 0) {
        s.sp2C = 0x13;
    } else {
        s.sp2C = 9;
    }
    s.sp20 = arg0[0x10 / 4];
    arg0[0x10 / 4] = func_8007352C((s32)&s);
}
