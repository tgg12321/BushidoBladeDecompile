/* 5 game functions. .text 0x8007352C (ROM 0x63D2C). Start boundary: G8 (the end of 5ED34.c's -G8
 * unit). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* Declarations from the file this TU was split from (text1b_tu1c.c). */
extern s32 rsin();
extern s32 g_gpu_ot_ptr;
extern s32 func_80073728(s32, s32);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
extern s32 func_8006E480();
extern s32 func_8007352C();
extern s32 SetShadeTex(s32, s32);
extern void SetPolyFT4(void *);
void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2);
void func_80069A30(u8 *a0);
void func_80069A8C(u8 *a0);
extern void func_80069A8C(u8 *p);
extern const u8 D_800159A0[16];
typedef struct EnvA {
    s32 *header;
    s8  *table;
    s32  out;
    s32  pad0C;
    s32  semi;
    u32  ot_idx;
    s32  x;
    s32  y;
    s32  pad20, pad24;
    u8   has_color;
    u8   col_r;
    u8   col_g;
    u8   col_b;
} EnvA;
s32 func_8006E480(s32 a0_addr, s32 a1);
extern s32 func_80073C78();

extern void SetSprt(s32);
extern s32 SetShadeTex(s32, s32);



extern const u8 D_800159A0[];

typedef struct SprtA {
    u32 tag;
    u8  r0, g0, b0, code;
    s16 x0, y0;
    u8  u0, v0;
    u16 clut;
    s16 w, h;
} SprtA;

typedef struct SprtHdrA {
    u8  pad0, pad1;
    u8  count;
    u8  pad3;
    u16 cx, cy;
    u8  ubase;
    u8  pad9;
    u8  vbase;
} SprtHdrA;

typedef struct SprtEntA {
    s16 x, y;
    u8  u, v;
    u8  w, h;
} SprtEntA;

s32 func_8007352C(s32 env_addr) {
    EnvA *env = (EnvA *)env_addr;
    SprtHdrA *hdr = (SprtHdrA *)env->header;
    SprtA *sp = (SprtA *)env->out;
    SprtEntA *e;
    s32 clut;
    s16 i;
    s32 x0, y0, x1, y1;

    clut = GetClut(hdr->cx, hdr->cy);
    for (i = hdr->count - 1; i >= 0; i--) {
        e = (SprtEntA *)env->table + i;
        x0 = e->x + env->x;
        y0 = e->y + env->y;
        x1 = x0 + e->w;
        y1 = y0 + e->h;
        if (x1 > 0 && x0 < 0x280 && y0 < 0xF0 && y1 > 0) {
            SetSprt((s32)sp);
            sp->clut = clut;
            sp->x0 = x0;
            sp->y0 = y0;
            sp->u0 = e->u + hdr->ubase;
            sp->v0 = e->v + hdr->vbase;
            sp->w = e->w;
            sp->h = e->h;
            if (env->has_color) {
                SetShadeTex((s32)sp, 0);
                sp->r0 = env->col_r;
                sp->g0 = env->col_g;
                sp->b0 = env->col_b;
            } else {
                SetShadeTex((s32)sp, 1);
            }
            SetSemiTrans((s32)sp, env->semi);
            if (env->ot_idx >= 0x1006) {
                env->ot_idx = 1;
                ((void (*)())func_8003D52C)(D_800159A0);
            }
            AddPrim(g_gpu_ot_ptr + env->ot_idx * 4, (s32)sp);
            sp++;
        }
    }
    return (s32)sp;
}
/* END func_8007352C */

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
extern VECTOR D_8009BCD4;

s32 func_80073C78(env, angle, mode)
    EnvF *env;
    s16 angle;
    s32 mode;
{
    s16 i;
    u32 tpage;
    u32 clut;
    s16 du0, dv0;
    s16 du1, dv1;
    u16 ub, vb;
    Ft4Sheet *hdr;
    SVECTOR vec[4];
    SVECTOR ang;
    VECTOR out[4];
    MATRIX mtx;
    POLY_FT4 *p;
    Ft4Cell *e;
    s16 j;
    s16 minx, miny, maxx, maxy;
    s16 cx, cy;
    s32 u, v;
    s32 x, y;

    hdr = env->header;
    e = env->table;
    tpage = (hdr->tp0 & 0xFE1F) + (hdr->tp1 << 7);
    clut = GetClut(hdr->cx, hdr->cy);
    minx = e->x;
    miny = e->y;
    maxx = minx + ((e->w * env->scale_x) >> 8);
    maxy = miny + ((e->h * env->scale_y) >> 8);
    for (i = 1; i < hdr->count; i++) {
        e++;
        if (minx > e->x) {
            minx = e->x;
        }
        if (maxx < e->x + ((e->w * env->scale_x) >> 8)) {
            maxx = e->x + ((e->w * env->scale_x) >> 8);
        }
        if (miny > e->y) {
            miny = e->y;
        }
        if (maxy < e->y + ((e->h * env->scale_y) >> 8)) {
            maxy = e->y + ((e->h * env->scale_y) >> 8);
        }
    }
    cy = (miny + maxy) / 2;
    cx = (minx + maxx) / 2;
    e = env->table;
    p = env->out;
    du0 = 0;
    dv0 = 0;
    ub = hdr->ubase;
    vb = hdr->vbase;
    for (i = 0; i < hdr->count; i++) {
        du1 = e->w;
        dv1 = e->h;
        SetPolyFT4(p);
        p->tpage = tpage;
        p->clut = clut;
        ang.vy = 0;
        ang.vx = 0;
        ang.vz = angle;
        RotMatrix(&ang, &mtx);
        ScaleMatrixL(&mtx, &D_8009BCD4);
        SetRotMatrix(&mtx);
        for (j = 0; j < 4; j++) {
            vec[j].vx = (((e->x - cx) + e->w * (j & 1)) * env->scale_x >> 8) * 2;
            if (mode == 1 || mode == 3) {
                vec[j].vx = -vec[j].vx;
            }
            vec[j].vy = (((e->y - cy) + ((e->h * (j & 2)) >> 1)) * env->scale_y >> 8) * 2;
            if (mode == 2 || mode == 3) {
                vec[j].vy = -vec[j].vy;
            }
            ApplyRotMatrix(&vec[j], &out[j]);
        }
        x = env->x;
        y = env->y;
        p->x0 = cx + out[0].vx + x;
        p->y0 = cy + out[0].vy + y;
        p->x1 = cx + out[1].vx + x;
        p->y1 = cy + out[1].vy + y;
        p->x2 = cx + out[2].vx + x;
        p->y2 = cy + out[2].vy + y;
        p->x3 = cx + out[3].vx + x;
        p->y3 = cy + out[3].vy + y;
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




void func_80074220(s32 *arg0, s32 arg1) {
    s32 sp[12];
    s32 i;
    s32 *temp_s2;
    s32 v;
    s32 t;
    s32 q;
    s32 a3;

    if (arg1 != 0) goto skip_init;
    t = arg0[5];
    SetTile(t);
    func_80069A30(t);
    *(s16 *)(t + 8) = 0x3F;
    *(s16 *)(t + 0xA) = 0x30;
    *(s16 *)(t + 0xC) = 0x202;
    *(s16 *)(t + 0xE) = 0xB0;
    SetSemiTrans(t, 1);
    AddPrim(g_gpu_ot_ptr + 0x78, t);
    t += 0x10;
    arg0[5] = t;
skip_init:
    sp[5] = 0x1F;
    *(s8 *)((s32)&sp[0] + 0x28) = 0;
    sp[4] = 0;
    temp_s2 = *(s32 **)((s32)arg0[0] + 0x38);
    sp[6] = 0;
    sp[7] = 0;
    i = 0;
    do {
        v = temp_s2[i];
        sp[0] = v;
        sp[1] = v + 0xC;
        sp[2] = arg0[4];
        arg0[4] = func_8007352C(sp);
        i++;
    } while (i < 3);

    sp[0] = *temp_s2;
    a3 = func_8006E480(sp[0], 0);
    SetDrawMode(arg0[6], 1, 0, a3, 0);
    AddPrim(g_gpu_ot_ptr + 0x7C, arg0[6]);
    q = arg0[2];
    arg0[6] = arg0[6] + 0xC;
    SetPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0x8) = 0;
    *(s16 *)(q + 0xA) = 0xB9;
    *(s16 *)(q + 0xC) = 0x122;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x14) = 0x122;
    *(s16 *)(q + 0x16) = 0xEF;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x80, q);
    q += 0x18;

    SetPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0x8) = 0x15E;
    *(s16 *)(q + 0xA) = 0;
    *(s16 *)(q + 0xC) = 0x27F;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0x15E;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x14) = 0x27F;
    *(s16 *)(q + 0x16) = 0x36;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x80, q);
    q += 0x18;

    SetPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0x8) = 0x122;
    *(s16 *)(q + 0xA) = 0;
    *(s16 *)(q + 0xC) = 0x15E;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0x122;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x14) = 0x15E;
    *(s16 *)(q + 0x16) = 0xEF;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x80, q);
    q += 0x18;

    arg0[2] = q;
}

void func_80074488(s32 *arg0) {
    S_80074488 s;
    s16 mask;
    s16 i;
    s32 *table;
    s32 value;
    s32 color;
    s16 rect[4];
    SelWork *base;

    base = SELWORK;
    i = 0;
    mask = (1 << base->f3C[0])
         + (1 << (base->f65 + 5))
         + (1 << (base->f67 + 8))
         + (1 << (base->f66 + 9));
    s.sp2C = 2;
    table = *(s32 **)(arg0[0] + 0x34);
    do {
        s.sp30 = 0;
        s.sp34 = 0;
        s.sp28 = 0;
        if ((mask >> i) & 1) {
            if (i < 5) {
                color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                s.sp43 = color;
                s.sp34 = SELWORK->f40[0][1];
            } else if (i < 8) {
                if (SELWORK->f3C[0] == 0) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            } else if (i < 10) {
                if (SELWORK->f3C[0] == 1) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            } else if (i < 14) {
                if (SELWORK->f3C[0] == 2) {
                    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
                    s.sp43 = color;
                    s.sp30 = SELWORK->f40[0][0];
                    s.sp34 = SELWORK->f40[0][1];
                } else {
                    s.sp43 = 0x80;
                }
            }
            s.sp40 = 1;
            s.sp42 = s.sp43;
            s.sp41 = s.sp43;
        } else {
            s.sp43 = 0x40;
            s.sp42 = 0x40;
            s.sp41 = 0x40;
            if (i < 5) {
                s.sp40 = 0;
                s.sp28 = 1;
            } else if (i < 14) {
                s.sp40 = 1;
            } else {
                s.sp40 = 0;
            }
        }
        if ((u16)(i - 10) >= 4 ||
            D_8009BD20[SELWORK->f67][0] + 9 == i ||
            D_8009BD20[SELWORK->f67][1] + 9 == i) {
            value = table[i];
            s.sp18 = value;
            s.sp1C = value + 0xC;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s.sp18);
        }
        i++;
    } while (i < 15);

    s.sp18 = table[0];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, 0), 0);
    AddPrim(g_gpu_ot_ptr + 8, arg0[6]);
    arg0[6] += 0xC;
    rect[2] = 0x108;
    rect[0] = 0xBC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 2);
}

/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
u8 * D_800A36A0;
