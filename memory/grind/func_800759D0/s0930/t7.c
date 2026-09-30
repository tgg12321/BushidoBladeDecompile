extern u8 D_8009BCE4;

void func_800759D0(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    s32 *table;
    s32 color;
    s32 zero;
    s16 i;
    s32 q;

    zero = 0;
    s.sp28 = 0;
    s.sp40 = 0;
    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[0];
    s.sp30 = arg3 * 240 + 0x88;
    s.sp34 = 0x33;
    q = s.sp18 + 0xC;
    s.sp1C = q;
    if (arg3 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    if (arg1 != 0) {
        s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
    }
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;

    color = ((rsin(((*(u16 *)(D_800A36A0 + 0x34) & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    if (arg3 != 0) {
        s.sp2C = 0x14;
    } else {
        s.sp2C = 0xA;
    }
    s.sp30 = arg3 * 240;
    for (i = arg1 * 10; i < arg1 * 10 + 10; i++) {
        u8 entry = (D_8009BCF8 + i)->unk0;

        if ((&D_8009BCE4)[entry] & 1) {
            s16 *state;
            s32 index;

            s.sp18 = table[entry + 1];
            q = s.sp18 + 0x24;
            s.sp1C = q;
            state = (s16 *)(arg3 * 2 + (s32)D_800A36A0);
            index = (((state[0x1C / 2] * 5) + state[0x20 / 2]) * 2) + (arg1 * 20);
            if (((u8 *)D_8009BCF8)[index] == (D_8009BCF8 + i)->unk0) {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            } else {
                s.sp40 = 0;
            }
            s.sp34 = 0;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
            if ((&D_8009BCE4)[(D_8009BCF8 + i)->unk0] & (4 << arg3)) {
                func_80075830(arg0, i, arg3, 1);
            }
        } else {
            func_80075830(arg0, i, arg3, 0);
        }
    }

    for (i = 0; i < *(s16 *)(D_800A36A0 + arg3 * 2 + 0x3C) + 1; i++) {
        if (arg2[i] >= 0) {
            s.sp18 = table[arg2[i] + 1];
            q = s.sp18 + 0x24;
            if (i != *(s16 *)(D_800A36A0 + arg3 * 2 + 0x3C)) {
                s.sp40 = 0;
            } else {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            }
            s.sp34 = i * 17;
            s.sp1C = q;
            s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
        }
    }

    s.sp40 = 0;
    for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++) {
        table = *(s32 **)(arg0[0] + *(u8 *)(D_800A36A0 + 0x65) * 4 + 0x20);
        s.sp18 = table[i];
        q = s.sp18 + 0x24;
        if (i == *(s16 *)(D_800A36A0 + arg3 * 2 + 0x3C)) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = q;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 17;
        if (arg3 != 0) {
            s.sp2C = 0x14;
        } else {
            s.sp2C = 0xA;
        }
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4 - 4, arg0[6]);
    arg0[6] += 0xC;
}
