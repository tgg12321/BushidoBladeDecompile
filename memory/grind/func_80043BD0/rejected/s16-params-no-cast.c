/* REJECTED s1: score 41/46 insns. s16 params with plain 'a1 >> 6': promoted-short params are not re-extended, so the sll/sra pairs vanish and the stack arg changes. */
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
void func_80043BD0(PolyFt3_43BD0 *p, s16 a1, s16 a2, s16 a3, s16 a4) {
    u16 tpage, clut;
    s32 x, y;
    tpage = p->tpage;
    x = ((tpage & 0xF) + (a1 >> 6)) & 0xF;
    y = (((tpage >> 4) & 1) + (a2 >> 8)) & 1;
    p->tpage = x | ((tpage & 0xFFE0) | (y << 4));
    p->v0 += a2;
    p->v2 += a2;
    p->v1 += a2;
    clut = p->clut;
    x = ((clut & 0x3F) + (a3 >> 4)) & 0x3F;
    y = (((clut >> 6) & 0x1FF) + a4) & 0x1FF;
    p->clut = x | ((clut & 0x8000) | (y << 6));
}
