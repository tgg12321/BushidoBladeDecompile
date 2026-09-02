/* REJECTED s1: score 28/43 insns. Fully inline mask expressions (no tx/ty/cx/cy locals): mask andi scheduled before the y-chain, byte stores reordered, extract+delta add order in clut block. */
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
    p->tpage = (((p->tpage & 0xF) + ((s16)a1 >> 6)) & 0xF) | ((p->tpage & 0xFFE0) | (((((p->tpage >> 4) & 1) + ((s16)a2 >> 8)) & 1) << 4));
    p->v0 += a2;
    p->v2 += a2;
    p->v1 += a2;
    p->clut = (((p->clut & 0x3F) + ((s16)a3 >> 4)) & 0x3F) | ((p->clut & 0x8000) | (((((p->clut >> 6) & 0x1FF) + a4) & 0x1FF) << 6));
}
