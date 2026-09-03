void func_8003A728(s32 a0) {
    s32 buf8;
    s32 packed;
    s32 hi16;
    s32 vsync;
    s32 v0;
    s32 v1;
    u16 c0lo;

    if (D_800A320C != 0) {
        buf8 = *(s32 *)(a0 + 8);
        vsync = D_800A38A0;
        packed = (vsync << 31) | (D_800A3730 << 30) | ((D_800A3870 & 3) << 28)
               | (*(s16 *)a0 << 16) | (buf8 & 0xFFFF);
        hi16 = D_800A37C4 << 16;
        D_800A3698 = packed;
        D_800A369C = hi16 | ((packed ^ (packed >> 16) ^ (hi16 >> 16)) & 0xFFFF);

        if (D_800A3916 != 0) {
            if (vsync == 0) {
                func_8003A574();
            } else {
                if (((FuncBufType)func_8003A450)(&D_800A3698) == 0) {
                    func_8003A3F0();
                    return;
                }
                D_800A3908 += func_8003A6FC(buf8 & 0xFFFF);
                func_8003A574();
            }
        } else {
            if (func_8003A5A0() == 0) {
                func_8003A3F0();
                return;
            }
            if (D_800A38A0 == 1) {
                if (D_800A36C0 & 0x40000000) {
                    func_8003A39C();
                    return;
                }
                if (D_800A36D0 & 0x40000000) {
                    func_8003A39C();
                    return;
                }
            }
            if (((FuncBufType)func_8003A450)(&D_800A3698) == 0) {
                func_8003A3F0();
                return;
            }
            D_800A3908 += func_8003A6FC(buf8 & 0xFFFF);
            func_8003A574();
            if (D_800A38A0 == 0) {
                if (D_800A3730 != 0 || (D_800A36C0 & 0x40000000)) {
                    func_8003A39C();
                    return;
                }
            }
        }

        if (D_800A3916 == 0) {
            D_800A38FC += func_8003A6FC((u16)D_800A36C0);
            c0lo = D_800A36C0;
            if (D_800A38A0 == 0) {
                *(s32 *)(a0 + 8) = (c0lo << 16) | (u16)D_800A3698;
                *(s16 *)(a0 + 2) = D_800A36C2 & 0xF;
            } else {
                *(s32 *)(a0 + 8) = ((u16)D_800A36D0 << 16) | c0lo;
                *(s16 *)a0 = D_800A36C2 & 0xF;
                *(s16 *)(a0 + 2) = D_800A36D2 & 0xF;
            }
            if (D_800A38A0 == 0) {
                if (((D_800A36C0 >> 28) & 3) == 2 && D_800A3870 == 2) {
                    D_800A3870 = 0;
                }
            } else {
                if (((D_800A36C0 >> 28) & 3) == 2 && ((D_800A36D0 >> 28) & 3) == 2) {
                    D_800A3870 = 0;
                }
            }
        }
        D_800A3916 = 0;
        D_800A36D0 = D_800A3698;
        D_800A36D4 = D_800A369C;
    } else {
        D_800A3870 = 0;
    }
}
