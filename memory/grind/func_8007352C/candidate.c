/* func_8007352C candidate — s1 recon 2026-09-08 (third pass; prior pass discarded
 * on a self-vet tripwire, body unchanged). sandbox --disable all = 0 (127/127),
 * canonical verdict C distance 0. Ordinary C throughout: the sprite initializer is
 * called with its real one-pointer signature (src/gpu.c:427); the lh $a1,0($s0)
 * before that call is local-alloc's scratch choice for e->x feeding x0.
 * Also requires: src/text1b.c:409 GetClut decl -> `extern u16 GetClut(s32, s32);`
 * (sibling func_800485EC re-measured 0, 68/68). Body is in place in src/text1b.c.
 */
extern void SetSprt(s32);
extern s32 SetShadeTex(s32, s32);
extern s32 SetSemiTrans(s32, s32);
extern s32 AddPrim(s32, s32);
extern void func_8003D52C(u8 *, ...);
extern u16 GetClut(s32, s32);
extern u8 D_800159A0[];

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

s32 func_8007352C(EnvA *env) {
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
                func_8003D52C(D_800159A0);
            }
            AddPrim(D_800A374C + env->ot_idx * 4, (s32)sp);
            sp++;
        }
    }
    return (s32)sp;
}

