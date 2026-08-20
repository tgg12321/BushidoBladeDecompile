typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x1E;
            ((POLY_G4 *)arg1)->g1 = 0xC8; ((POLY_G4 *)arg1)->b1 = 0x32;
        } else {
            ((POLY_G4 *)arg1)->r0 = 0xFC; ((POLY_G4 *)arg1)->g0 = 0xC3; ((POLY_G4 *)arg1)->b0 = 0x50;
            ((POLY_G4 *)arg1)->g1 = 0xDC; ((POLY_G4 *)arg1)->b1 = 0x46;
        }
        ((POLY_G4 *)arg1)->r1 = 0xFC;
        ((POLY_G4 *)arg1)->r2 = 0xFC; ((POLY_G4 *)arg1)->g2 = 0x82; ((POLY_G4 *)arg1)->b2 = 0;
        ((POLY_G4 *)arg1)->r3 = 0x32; ((POLY_G4 *)arg1)->g3 = 0x28; ((POLY_G4 *)arg1)->b3 = 0xA;
    } else {
        ((POLY_G4 *)arg1)->r0 = 0x10; ((POLY_G4 *)arg1)->g0 = 0x30; ((POLY_G4 *)arg1)->b0 = 0x60;
        ((POLY_G4 *)arg1)->r1 = 0x18; ((POLY_G4 *)arg1)->g1 = 0; ((POLY_G4 *)arg1)->b1 = 0x40;
        ((POLY_G4 *)arg1)->r2 = 0x30; ((POLY_G4 *)arg1)->g2 = 0; ((POLY_G4 *)arg1)->b2 = 0x60;
        ((POLY_G4 *)arg1)->r3 = 0; ((POLY_G4 *)arg1)->g3 = 0; ((POLY_G4 *)arg1)->b3 = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
