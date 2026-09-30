void func_80074488(s32 *arg0) {
    S_80074488 s;
    s16 mask;
    s16 i;
    s32 *table;
    s32 value;
    s32 color;
    s16 rect[4];
    SelWork *base;

    base = SELWORK;
    i = 0;
    mask = (1 << base->f3C[0])
         + (1 << (base->f65 + 5))
         + (1 << (base->f67 + 8))
         + (1 << (base->f66 + 9));
    s.sp2C = 2;
    table = *(s32 **)(arg0[0] + 0x34);
    do {
        s.sp30 = 0;
        s.sp34 = 0;
        s.sp28 = 0;
        if ((mask >> i) & 1) {
            if (i < 5) {
                color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                s.sp43 = color;
                s.sp34 = SELWORK->f40[0][1];
            } else if (i < 8) {
                if (SELWORK->f3C[0] == 0) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            } else if (i < 10) {
                if (SELWORK->f3C[0] == 1) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            } else if (i < 14) {
                if (SELWORK->f3C[0] == 2) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            }
            s.sp40 = 1;
            s.sp42 = s.sp43;
            s.sp41 = s.sp43;
        } else {
            s.sp43 = 0x40;
            s.sp42 = 0x40;
            s.sp41 = 0x40;
            if (i < 5) {
                s.sp40 = 0;
                s.sp28 = 1;
            } else if (i < 14) {
                s.sp40 = 1;
            } else {
                s.sp40 = 0;
            }
        }
        if ((u16)(i - 10) >= 4 ||
            D_8009BD20[SELWORK->f67][0] + 9 == i ||
            D_8009BD20[SELWORK->f67][1] + 9 == i) {
            value = table[i];
            s.sp18 = value;
            s.sp1C = value + 0xC;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s.sp18);
        }
        i++;
    } while (i < 15);

    s.sp18 = table[0];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, 0), 0);
    AddPrim(g_gpu_ot_ptr + 8, arg0[6]);
    arg0[6] += 0xC;
    rect[2] = 0x108;
    rect[0] = 0xBC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 2);
}
