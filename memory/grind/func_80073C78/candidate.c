extern s32 D_8009BCD4[];
extern void ApplyRotMatrix(s16 *, s32 *);

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
    s16 vec[4][4];
    s16 ang[4];
    s32 out[4][4];
    s32 mtx[8];
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
        ang[1] = 0;
        ang[0] = 0;
        ang[2] = angle;
        RotMatrix(ang, (u8 *)mtx);
        ScaleMatrixL((u8 *)mtx, (u8 *)D_8009BCD4);
        SetRotMatrix((u8 *)mtx);
        for (j = 0; j < 4; j++) {
            vec[j][0] = (((e->x - cx) + e->w * (j & 1)) * env->scale_x >> 8) * 2;
            if (mode == 1 || mode == 3) {
                vec[j][0] = -vec[j][0];
            }
            vec[j][1] = (((e->y - cy) + ((e->h * (j & 2)) >> 1)) * env->scale_y >> 8) * 2;
            if (mode == 2 || mode == 3) {
                vec[j][1] = -vec[j][1];
            }
            ApplyRotMatrix(vec[j], out[j]);
        }
        x = env->x;
        y = env->y;
        p->x0 = cx + out[0][0] + x;
        p->y0 = cy + out[0][1] + y;
        p->x1 = cx + out[1][0] + x;
        p->y1 = cy + out[1][1] + y;
        p->x2 = cx + out[2][0] + x;
        p->y2 = cy + out[2][1] + y;
        p->x3 = cx + out[3][0] + x;
        p->y3 = cy + out[3][1] + y;
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
