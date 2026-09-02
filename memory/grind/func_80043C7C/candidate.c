/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes) -- u16 clut at +0xE, u16 tpage at
 * +0x16, v0/v1/v2/v3 at +0xD/+0x15/+0x1D/+0x25; the 0x28-stride quad
 * sibling of func_80043BD0 (POLY_FT3). */
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
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} POLY_FT4;

/* Quad counterpart of func_80043BD0: shift a textured quad's texture source
 * (tpage x/y by du/dv, all four vertex v by dv, clut x/y by dcx/dcy), each
 * packed u16 field updated in place inside its bit range. */
void func_80043C7C(POLY_FT4 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    p->v3 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
