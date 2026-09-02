/* candidate.c -- func_80043D34 -- s1 (2026-09-02): sandbox --disable all = 0 (43/43 insns), applied verbatim in src/text1a_c.c replacing the HEAD pin body. Direct transfer of the func_80043BD0 s1 spelling onto POLY_GT3 (+0x1A tpage, +0xD/+0x19/+0x25 v bytes, 0x28 caller stride). */
 * +0x1A, v0/v1/v2 at +0xD/+0x19/+0x25; the caller (func_80043454) walks a
 * 0x28-stride primitive array (asm/funcs/func_80043454.s:182,198). */
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    u8 r1, g1, b1, p1;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    u8 r2, g2, b2, p2;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad2;
} POLY_GT3;

/* Shift a gouraud-textured triangle's texture source: the tpage x field
 * (64-px pages) by du, the tpage y field (256-px pages) and every vertex v
 * by dv, and the clut x (16-px units) / y fields by dcx / dcy.  Same update
 * as func_80043BD0 on a POLY_GT3. */
void func_80043D34(POLY_GT3 *p, s32 du, s32 dv, s32 dcx, s32 dcy) {
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
