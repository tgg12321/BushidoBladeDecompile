s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;
    int t;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            t = 0xC3;
            *(u8 *)((s32)(arg1) + 5) = t;
            t = 0x1E;
            *(u8 *)((s32)(arg1) + 6) = t;
            t = 0xC8;
            *(u8 *)((s32)(arg1) + 0xD) = t;
            t = 0x32;
        } else {
            t = 0xC3;
            *(u8 *)((s32)(arg1) + 5) = t;
            t = 0x50;
            *(u8 *)((s32)(arg1) + 6) = t;
            t = 0xDC;
            *(u8 *)((s32)(arg1) + 0xD) = t;
            t = 0x46;
        }
        *(u8 *)((s32)(arg1) + 4) = red;
        *(u8 *)((s32)(arg1) + 0xC) = red;
        *(u8 *)((s32)(arg1) + 0xE) = t;
        red = 0xFC;
        *(u8 *)((s32)(arg1) + 0x14) = red;
        red = 0x82;
        *(u8 *)((s32)(arg1) + 0x15) = red;
        red = 0x32;
        *(u8 *)((s32)(arg1) + 0x1C) = red;
        red = 0x28;
        *(u8 *)((s32)(arg1) + 0x1D) = red;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        red = 0xA;
        *(u8 *)((s32)(arg1) + 0x1E) = red;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
