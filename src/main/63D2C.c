/* 5 game functions. .text 0x8007352C (ROM 0x63D2C). Start boundary: G8 (the end
 * of 5ED34.c's -G8 unit). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

extern void AddPrim(void *, void *);
extern const u8 D_800159A0[16];
extern s32 func_80073C78();

extern const u8 D_800159A0[];

s32 func_8007352C(Unk8007352CEnv *env) {
    Unk8009B0E0Record *hdr = env->header;
    SPRT *sp = (SPRT *)env->sprt_out;
    Unk8009B400Record *e;
    s32 clut;
    s16 i;
    /* FAKE: x1 computed ahead of y1 and read once in the bounds test; at its
     * use: score 2. */
    s32 x0, y0, x1, y1;

    clut = GetClut(hdr->cx, hdr->cy);
    for (i = hdr->count - 1; i >= 0; i--) {
        e = env->table + i;
        x0 = e->x + env->x;
        y0 = e->y + env->y;
        x1 = x0 + e->w;
        y1 = y0 + e->h;
        if (x1 > 0 && x0 < 0x280 && y0 < 0xF0 && y1 > 0) {
            SetSprt(sp);
            sp->clut = clut;
            sp->x0 = x0;
            sp->y0 = y0;
            sp->u0 = e->u + hdr->ubase;
            sp->v0 = e->v + hdr->vbase;
            sp->w = e->w;
            sp->h = e->h;
            if (env->has_color) {
                SetShadeTex(sp, 0);
                sp->r0 = env->col_r;
                sp->g0 = env->col_g;
                sp->b0 = env->col_b;
            } else {
                SetShadeTex(sp, 1);
            }
            SetSemiTrans((s32)sp, env->semi);
            if (env->ot_idx >= 0x1006) {
                env->ot_idx = 1;
                ((void (*)())func_8003D52C)(D_800159A0);
            }
            AddPrim(g_gpu_ot_ptr + env->ot_idx, sp);
            sp++;
        }
    }
    return (s32)sp;
}

s32 func_80073728(Unk8007352CEnv *env, s32 mode) {
    Unk8009B400Record *e = env->table;
    Unk8009B0E0Record *hdr = env->header;
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
    p = (POLY_FT4 *)env->ft4_out;
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
            SetShadeTex(p, 0);
            p->r0 = env->col_r;
            p->g0 = env->col_g;
            p->b0 = env->col_b;
        } else {
            SetShadeTex(p, 1);
        }
        SetSemiTrans(p, env->semi);
        if (env->ot_idx >= 0x1006) {
            env->ot_idx = 1;
        }
        AddPrim(g_gpu_ot_ptr + env->ot_idx, p);
        p++;
        e++;
    }
    env->ft4_out = (s32)p;
    return (s32)p;
}

extern VECTOR D_8009BCD4;

s32 func_80073C78(env, angle, mode)
Unk8007352CEnv *env;
s16 angle;
s32 mode;
{
    s16 i;
    u32 tpage;
    u32 clut;
    /* FAKE: du0 / dv0 stay 0 in this walker (func_80073728 sets them per mirror
       mode); u / v written without them: score 33. */
    s16 du0, dv0;
    s16 du1, dv1;
    u16 ub, vb;
    Unk8009B0E0Record *hdr;
    SVECTOR vec[4];
    SVECTOR ang;
    VECTOR out[4];
    MATRIX mtx;
    POLY_FT4 *p;
    Unk8009B400Record *e;
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
    p = (POLY_FT4 *)env->ft4_out;
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
            vec[j].vx =
                (((e->x - cx) + e->w * (j & 1)) * env->scale_x >> 8) * 2;
            if (mode == 1 || mode == 3) {
                vec[j].vx = -vec[j].vx;
            }
            vec[j].vy =
                (((e->y - cy) + ((e->h * (j & 2)) >> 1)) * env->scale_y >> 8) *
                2;
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
            SetShadeTex(p, 0);
            p->r0 = env->col_r;
            p->g0 = env->col_g;
            p->b0 = env->col_b;
        } else {
            SetShadeTex(p, 1);
        }
        SetSemiTrans(p, env->semi);
        if (env->ot_idx >= 0x1006) {
            env->ot_idx = 1;
        }
        AddPrim(g_gpu_ot_ptr + env->ot_idx, p);
        p++;
        e++;
    }
    env->ft4_out = (s32)p;
    return (s32)p;
}

void func_80074220(Unk8006EACCRec *arg0, s32 arg1) {
    Unk8007352CEnv s;
    s32 i;
    Unk8009B0E0Record **temp_s2;
    TILE *t;
    POLY_F4 *q;

    if (arg1 != 0)
        goto skip_init;
    t = arg0->unk_04.unk_10;
    SetTile(t);
    func_80069A30(t);
    t->x0 = 0x3F;
    t->y0 = 0x30;
    t->w = 0x202;
    t->h = 0xB0;
    SetSemiTrans(t, 1);
    AddPrim(g_gpu_ot_ptr + 0x1E, t);
    t++;
    arg0->unk_04.unk_10 = t;
skip_init:
    s.ot_idx = 0x1F;
    s.has_color = 0;
    s.semi = 0;
    temp_s2 = arg0->unk_00.v80076FF8->unk_38;
    s.x = 0;
    s.y = 0;
    i = 0;
    do {
        s.header = temp_s2[i];
        s.table = s.header->cells;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        i++;
    } while (i < 3);

    s.header = *temp_s2;
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x1F, arg0->unk_04.unk_14);
    q = arg0->unk_04.unk_04;
    arg0->unk_04.unk_14++;
    SetPolyF4(q);
    func_80069A8C(q);
    q->x0 = 0;
    q->y0 = 0xB9;
    q->x1 = 0x122;
    q->y1 = 0;
    q->x2 = 0;
    q->y2 = 0xEF;
    q->x3 = 0x122;
    q->y3 = 0xEF;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, q);
    q++;

    SetPolyF4(q);
    func_80069A8C(q);
    q->x0 = 0x15E;
    q->y0 = 0;
    q->x1 = 0x27F;
    q->y1 = 0;
    q->x2 = 0x15E;
    q->y2 = 0xEF;
    q->x3 = 0x27F;
    q->y3 = 0x36;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, q);
    q++;

    SetPolyF4(q);
    func_80069A8C(q);
    q->x0 = 0x122;
    q->y0 = 0;
    q->x1 = 0x15E;
    q->y1 = 0;
    q->x2 = 0x122;
    q->y2 = 0xEF;
    q->x3 = 0x15E;
    q->y3 = 0xEF;
    SetSemiTrans(q, 0);
    AddPrim(g_gpu_ot_ptr + 0x20, q);
    q++;

    arg0->unk_04.unk_04 = q;
}

void func_80074488(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    s16 mask;
    s16 i;
    Unk8009B0E0Record **table;
    s16 rect[4];

    i = 0;
    mask = (1 << SELWORK->f3C[0]) + (1 << (SELWORK->f65 + 5)) +
           (1 << (SELWORK->f67 + 8)) + (1 << (SELWORK->f66 + 9));
    s.ot_idx = 2;
    table = arg0->unk_00.v80076FF8->unk_34;
    do {
        s.x = 0;
        s.y = 0;
        s.semi = 0;
        if ((mask >> i) & 1) {
            if (i < 5) {
                s.col_b =
                    ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) -
                    0x80;
                s.y = SELWORK->f40[0][1];
            } else if (i < 8) {
                if (SELWORK->f3C[0] == 0) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            } else if (i < 10) {
                if (SELWORK->f3C[0] == 1) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            } else if (i < 14) {
                if (SELWORK->f3C[0] == 2) {
                    s.col_b =
                        ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >>
                         12) -
                        0x80;
                    s.x = SELWORK->f40[0][0];
                    s.y = SELWORK->f40[0][1];
                } else {
                    s.col_b = 0x80;
                }
            }
            s.has_color = 1;
            s.col_g = s.col_b;
            s.col_r = s.col_b;
        } else {
            s.col_b = 0x40;
            s.col_g = 0x40;
            s.col_r = 0x40;
            if (i < 5) {
                s.has_color = 0;
                s.semi = 1;
            } else if (i < 14) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
        }
        if ((u16)(i - 10) >= 4 || D_8009BD20[SELWORK->f67][0] + 9 == i ||
            D_8009BD20[SELWORK->f67][1] + 9 == i) {
            s.header = table[i];
            s.table = s.header->cells;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
        i++;
    } while (i < 15);

    s.header = table[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 2, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    rect[2] = 0x108;
    rect[0] = 0xBC;
    rect[1] = 0x25;
    rect[3] = 1;
    /* The original passes its own context base here, unadjusted:
     * func_80069898 reads +0x18 as its TILE cursor, which in this context is
     * the DR_MODE cursor. */
    func_80069898((s32 *)arg0, (u16 *)rect, 2);
}

/* Tentative definitions (COMMON) of the small data this file reaches
 * gp-relative (Q65). */
u8 *D_800A36A0;
