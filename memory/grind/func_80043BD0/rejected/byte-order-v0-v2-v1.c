/* REJECTED s1: score 4/43 insns. The 0-form with byte-add order v0, v2, v1 -- the target's sb order is 0xD, 0x1D, 0x15 but source order v0, v1, v2 is what the scheduler maps to it. */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} PolyFt3_43BD0;
void func_80043BD0(PolyFt3_43BD0 *p, s32 a1, s32 a2, s32 a3, s32 a4) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;
    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)a1 >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)a2 >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += a2;
    p->v2 += a2;
    p->v1 += a2;
    clut = p->clut;
    cx = (((s16)a3 >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (a4 + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
