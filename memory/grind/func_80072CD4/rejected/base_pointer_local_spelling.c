s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int fc_const;
    u8 *p;

    p = (u8 *)arg1;
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        fc_const = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            p[5] = 0xC3;
            p[6] = 0x1E;
            p[0xD] = 0xC8;
            p[0xE] = 0x32;
        } else {
            p[5] = 0xC3;
            p[6] = 0x50;
            p[0xD] = 0xDC;
            p[0xE] = 0x46;
        }
        p[4] = fc_const;
        p[0xC] = fc_const;
        p[0x14] = 0xFC;
        p[0x15] = 0x82;
        p[0x1C] = 0x32;
        p[0x1D] = 0x28;
        p[0x16] = 0;
        p[0x1E] = 0xA;
    } else {
        p[4] = 0x10;
        p[5] = 0x30;
        p[6] = 0x60;
        p[0xC] = 0x18;
        p[0xD] = 0;
        p[0xE] = 0x40;
        p[0x14] = 0x30;
        p[0x15] = 0;
        p[0x16] = 0x60;
        p[0x1C] = 0;
        p[0x1D] = 0;
        p[0x1E] = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
