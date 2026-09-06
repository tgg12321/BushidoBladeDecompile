/* BEGIN func_80072E10 */
typedef struct PolyG4Xy {
    s32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} PolyG4Xy;
void func_80072E10(s32 arg0) {
    PolyG4Xy *p;
    func_80073060(arg0);
    p = *(PolyG4Xy **)((s32)arg0 + 0xC);
    p->x0 = 0x50;
    p->y0 = 0x32;
    p->x1 = 0x50;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = (PolyG4Xy *)func_80072BC4(D_800A3580, (GameObj *)p);
    p->x0 = 0x231;
    p->y0 = 0x32;
    p->x1 = 0x231;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = (PolyG4Xy *)func_80072BC4(D_800A3580, (GameObj *)p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x50;
    p->y2 = 0x52;
    p->x3 = 0x50;
    p->y3 = 0x71;
    p = (PolyG4Xy *)func_80072CD4(D_800A3580, (GameObj *)p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x231;
    p->y2 = 0x52;
    p->x3 = 0x231;
    p->y3 = 0x71;
    p = (PolyG4Xy *)func_80072CD4(D_800A3580, (GameObj *)p);
    *(PolyG4Xy **)((s32)arg0 + 0xC) = p;
}
/* END func_80072E10 */
