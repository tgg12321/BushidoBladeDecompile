/* REJECTED s1: score 13/43 insns. Same as the 0-form except left-assoc or-tree '(w & M) | (y << s) | x' -- the target's final or has x as op0 and the mask|shift as a sub-tree. */
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
    p->tpage = (tpage & 0xFFE0) | (ty << 4) | tx;
    p->v0 += a2;
    p->v2 += a2;
    p->v1 += a2;
    clut = p->clut;
    cx = (((s16)a3 >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (a4 + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = (clut & 0x8000) | (cy << 6) | cx;
}
