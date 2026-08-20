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
    POLY_G4 *p = (POLY_G4 *)arg1;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            p->r0 = 0xFC; p->g0 = 0xC3; p->b0 = 0x1E;
            p->r1 = 0xFC; p->g1 = 0xC8; p->b1 = 0x32;
        } else {
            p->r0 = 0xFC; p->g0 = 0xC3; p->b0 = 0x50;
            p->r1 = 0xFC; p->g1 = 0xDC; p->b1 = 0x46;
        }
        p->r2 = 0xFC; p->g2 = 0x82; p->b2 = 0;
        p->r3 = 0x32; p->g3 = 0x28; p->b3 = 0xA;
    } else {
        p->r0 = 0x10; p->g0 = 0x30; p->b0 = 0x60;
        p->r1 = 0x18; p->g1 = 0; p->b1 = 0x40;
        p->r2 = 0x30; p->g2 = 0; p->b2 = 0x60;
        p->r3 = 0; p->g3 = 0; p->b3 = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
