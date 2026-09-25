/* PsyQ LIBGPU.H POLY_FT4 (0x28 bytes), same layout as text1a_c.c's. */
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

extern void SetPolyFT4(void *);

/* Sprite-sheet header and 8-byte cell record read by func_80073728 (the
   scaled POLY_FT4 sibling of func_8007352C's SPRT walker). */
typedef struct Ft4Sheet {
    u8  tp0, tp1;
    u8  count;
    u8  pad3;
    u16 cx, cy;
    u16 ubase;
    u16 vbase;
} Ft4Sheet;

typedef struct Ft4Cell {
    s16 x, y;
    u8  u, v;
    u8  w, h;
} Ft4Cell;

/* The same 0x2C-byte draw descriptor as EnvA (func_80073200 builds one on
   its stack as S73200 and passes it to both walkers): +0x8 is the SPRT
   cursor func_8007352C advances, +0xC the POLY_FT4 cursor this one does,
   and +0x20/+0x24 are 8.8 fixed-point scales (func_80073200 stores 0x100). */
typedef struct EnvF {
    Ft4Sheet *header;
    Ft4Cell  *table;
    s32       sprt_out;
    POLY_FT4 *out;
    s32       semi;
    u32       ot_idx;
    s32       x;
    s32       y;
    s32       scale_x;
    s32       scale_y;
    u8        has_color;
    u8        col_r;
    u8        col_g;
    u8        col_b;
} EnvF;

s32 func_80073728(s32 env_addr, s32 mode) {
    EnvF *env = (EnvF *)env_addr;
    Ft4Cell *e = env->table;
    Ft4Sheet *hdr = env->header;
    POLY_FT4 *p;
    s16 i;
    u32 tpage;
    u32 clut;
    s16 du0, du1, dv0, dv1;
    u16 ub, vb;
    s16 ox = e->x, oy = e->y;
    s32 sx, sy;
    s32 u, v;

    if (mode == 4) {
        tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7) + 0x20;
    } else if (mode == 5) {
        tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7) + 0x40;
    } else if (mode == 6) {
        tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7) + 0x60;
    } else {
        tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7);
    }
    clut = GetClut(hdr->cx, hdr->cy);
    ub = hdr->ubase;
    vb = hdr->vbase;
    if (mode == 1) {
        du1 = 1;
        dv0 = 0;
    } else if (mode == 2) {
        du0 = 0;
        dv1 = -1;
    } else if (mode == 3) {
        du1 = 1;
        dv1 = -1;
    } else {
        du0 = 0;
        dv0 = 0;
    }
    p = env->out;
    for (i = 0; i < hdr->count; i++) {
        if (mode == 1) {
            du0 = e->w - 1;
            dv1 = e->h;
        } else if (mode == 2) {
            du1 = e->w;
            dv0 = e->h - 1;
        } else if (mode == 3) {
            du0 = e->w - 1;
            dv0 = e->h - 1;
        } else {
            dv1 = e->h;
            du1 = e->w;
            if (dv1 == 1) {
                dv1 = 0;
            }
        }
        SetPolyFT4(p);
        p->tpage = tpage;
        p->clut = clut;
        sx = env->scale_x;
        sy = env->scale_y;
        p->x0 = ox + env->x + (((e->x - ox) * sx) >> 8);
        p->y0 = oy + env->y + (((e->y - oy) * sy) >> 8);
        p->x1 = ox + env->x + (((e->x - ox) * sx) >> 8) + ((e->w * sx) >> 8);
        p->y1 = oy + env->y + (((e->y - oy) * sy) >> 8);
        p->x2 = ox + env->x + (((e->x - ox) * sx) >> 8);
        p->y2 = oy + env->y + (((e->y - oy) * sy) >> 8) + ((e->h * sy) >> 8);
        p->x3 = ox + env->x + (((e->x - ox) * sx) >> 8) + ((e->w * sx) >> 8);
        p->y3 = oy + env->y + (((e->y - oy) * sy) >> 8) + ((e->h * sy) >> 8);
        u = ub + e->u;
        v = vb + e->v;
        p->u0 = u + du0;
        p->v0 = v + dv0;
        p->u1 = u + du1;
        p->v1 = v + dv0;
        p->u2 = u + du0;
        p->v2 = v + dv1;
        p->u3 = u + du1;
        p->v3 = v + dv1;
        if (env->has_color) {
            SetShadeTex((s32)p, 0);
            p->r0 = env->col_r;
            p->g0 = env->col_g;
            p->b0 = env->col_b;
        } else {
            SetShadeTex((s32)p, 1);
        }
        SetSemiTrans(p, env->semi);
        if (env->ot_idx >= 0x1006) {
            env->ot_idx = 1;
        }
        AddPrim(g_gpu_ot_ptr + env->ot_idx * 4, (s32)p);
        p++;
        e++;
    }
    env->out = p;
    return (s32)p;
}
