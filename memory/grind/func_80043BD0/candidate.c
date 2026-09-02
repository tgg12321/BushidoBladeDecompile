/* candidate.c -- func_80043BD0 -- s1 (2026-09-02): sandbox --disable all = 0 (43/43 insns), applied verbatim in src/text1a_c.c replacing the HEAD pin body. */
/* PsyQ LIBGPU.H POLY_FT3 (0x20 bytes) -- the only libgpu primitive with a
 * u16 clut at +0xE, u16 tpage at +0x16 and v0/v1/v2 at +0xD/+0x15/+0x1D;
 * the caller (func_80043454) walks a 0x20-stride primitive array. */
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
} POLY_FT3;

/* Shift a textured triangle's texture source: the tpage x field (64-px
 * pages) by du, the tpage y field (256-px pages) and every vertex v by dv,
 * and the clut x (16-px units) / y fields by dcx / dcy.  Each packed u16
 * field is updated in place inside its bit range. */
void func_80043BD0(POLY_FT3 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
    u16 tpage, clut;
    s32 tx, ty, cx, cy;

    tpage = p->tpage;
    tx = ((tpage & 0xF) + ((s16)du >> 6)) & 0xF;
    ty = (((tpage >> 4) & 1) + ((s16)dv >> 8)) & 1;
    p->tpage = tx | ((tpage & 0xFFE0) | (ty << 4));
    p->v0 += dv;
    p->v1 += dv;
    p->v2 += dv;
    clut = p->clut;
    cx = (((s16)dcx >> 4) + (clut & 0x3F)) & 0x3F;
    cy = (dcy + ((clut >> 6) & 0x1FF)) & 0x1FF;
    p->clut = cx | ((clut & 0x8000) | (cy << 6));
}
