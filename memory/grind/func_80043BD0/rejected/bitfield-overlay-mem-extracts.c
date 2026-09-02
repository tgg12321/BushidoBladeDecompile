/* REJECTED s1: score 34/48 insns. Bitfield overlay struct with inserts via memory-read extracts (p->tpage.y) -- extra lhu + zext traffic. */
typedef struct { u16 x : 6; u16 y : 9; u16 r : 1; } Clut_43BD0;
typedef struct { u16 x : 4; u16 y : 1; u16 r : 11; } Tpage_43BD0;
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    Clut_43BD0 clut;
    s16 x1, y1;
    u8 u1, v1;
    Tpage_43BD0 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
} PolyFt3_43BD0;
void func_80043BD0(PolyFt3_43BD0 *p, s32 a1, s32 a2, s32 a3, s32 a4) {
    Tpage_43BD0 t;
    Clut_43BD0 c;
    t = p->tpage;
    t.y = p->tpage.y + ((s16)a2 >> 8);
    t.x = p->tpage.x + ((s16)a1 >> 6);
    p->tpage = t;
    p->v0 += a2;
    p->v2 += a2;
    p->v1 += a2;
    c = p->clut;
    c.y = p->clut.y + a4;
    c.x = p->clut.x + ((s16)a3 >> 4);
    p->clut = c;
}
