u8 func_80065800(s32 arg0) {
    extern s32 D_800A3720;
    extern s32 D_800A3724;
    extern s32 D_8009BD44[];
    extern s16 D_800A3834;
    extern u16 D_8009B8C8[];
    extern u16 D_8009B8D0[];
    extern u16 D_8009B8D8[];
    extern u16 D_8009B8E0[];
    extern u16 D_8009B978[];
    extern u16 D_8009B980[];
    extern u16 D_8009B988[];
    extern u16 D_8009B990[];
    extern u16 D_8009B9D8[];
    extern u16 D_8009B9E0[];
    extern u16 D_8009B9E8[];
    extern u16 D_8009B9F0[];
    extern void ApplyRotMatrix(SVECTOR *, VECTOR *);
    extern s32 ReadGeomScreen(void);
    s32 outer;
    POLY_FT4 *prim;
    s32 *p_dp;
    VECTOR *p_t;
    VECTOR *p_in;
    SVECTOR *p_v;
    s16 *p_w;
    s16 *p_h;
    MATRIX *p_mat;
    s16 *p_tw;
    s16 *p_th;
    VECTOR *dst;
    s32 n;
    s32 w;
    s32 sw;
    s32 sh;
    s32 h;
    s16 *t;
    s16 *tbl;
    s32 i;

    outer = D_800A34EC;
    prim = (POLY_FT4 *)D_800A37D4;
    p_dp = (s32 *)(outer + 0x10);
    p_t = (VECTOR *)(outer + 0x14);
    p_in = (VECTOR *)(outer + 0x54);
    p_v = (SVECTOR *)(outer + 0x64);
    p_w = (s16 *)(outer + 0x6C);
    p_h = (s16 *)(outer + 0x70);
    p_mat = (MATRIX *)(outer + 0x74);
    p_tw = (s16 *)(outer + 0x94);
    p_th = (s16 *)(outer + 0x96);

    /* gte_SetRotMatrix(r0) --- inline_c.h :297-310 */
    __asm__ volatile(
        "lw     $12, 0(%0)\n"
        "lw     $13, 4(%0)\n"
        "ctc2   $12, $0\n"
        "ctc2   $13, $1\n"
        "lw     $12, 8(%0)\n"
        "lw     $13, 12(%0)\n"
        "lw     $14, 16(%0)\n"
        "ctc2   $12, $2\n"
        "ctc2   $13, $3\n"
        "ctc2   $14, $4\n"
        :: "r"(D_800A3474) : "$12", "$13", "$14");
    p_in->vx = D_800F0CA0[arg0].unk0 - ((s32 *)D_800A3470)[0];
    p_in->vy = D_800F0CA0[arg0].unk4 - ((s32 *)D_800A3470)[1];
    p_in->vz = D_800F0CA0[arg0].unk8 - ((s32 *)D_800A3470)[2];
    ApplyRotMatrixLV(p_in, p_t);
    /* gte_SetTransMatrix(r0) --- inline_c.h :360-369 */
    __asm__ volatile(
        "lw     $12, 20(%0)\n"
        "lw     $13, 24(%0)\n"
        "ctc2   $12, $5\n"
        "lw     $14, 28(%0)\n"
        "ctc2   $13, $6\n"
        "ctc2   $14, $7\n"
        :: "r"((MATRIX *)outer) : "$12", "$13", "$14");
    p_v->vz = 0;
    p_v->vy = 0;
    p_v->vx = 0;
    /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
    __asm__ volatile(
        "lwc2   $0, 0(%0)\n"
        "lwc2   $1, 4(%0)\n"
        :: "r"(p_v));
    /* gte_rtps() --- inline_c.h :484-487, post-DMPSX command word */
    __asm__ volatile(
        "nop\n"
        "nop\n"
        ".word 0x4A180001\n");
    /* gte_stsxy(r0) --- inline_c.h :900-904 */
    __asm__ volatile(
        "swc2   $14, 0(%0)\n"
        :: "r"(D_800A34B8) : "memory");
    /* gte_stdp(r0) --- inline_c.h :1018-1022 */
    __asm__ volatile(
        "swc2   $8, 0(%0)\n"
        :: "r"(p_dp) : "memory");
    /* gte_stflg(r0) --- inline_c.h :1024-1030 */
    __asm__ volatile(
        "cfc2   $12, $31\n"
        "nop\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34CC) : "$12", "memory");
    /* gte_stszotz(r0) --- inline_c.h :1082-1089 */
    __asm__ volatile(
        "mfc2   $12, $19\n"
        "nop\n"
        "sra    $12, $12, 2\n"
        "sw     $12, 0(%0)\n"
        :: "r"(D_800A34D0) : "$12", "memory");
    *(s32 *)D_800A34D0 = *(s32 *)D_800A34D0 ? *(s32 *)D_800A34D0 << 2 : 1;

again:
    *(s16 *)D_800A34A8 = 0x20;
    *(s16 *)D_800A34AC = 0x20;
    *p_tw = 0x40;
    *p_th = 0x40;
    *(s32 *)D_800A3490 = 0x2E;
    switch (arg0) {
    case 1:
    case 2:
        if (*(s32 *)D_800A34D0 > 4000) {
            D_800A3488 = (s32)D_8009B8D0;
        } else {
            D_800A3488 = (s32)D_8009B8D8;
        }
        prim->r0 = 0xFF;
        prim->g0 = 0x80;
        prim->b0 = 0x80;
        break;
    case 3:
    case 4:
        if (D_800F0BA8[arg0] < 0x96) {
            D_800A3488 = (s32)D_8009B980;
        } else {
            D_800A3488 = (s32)D_8009B978;
        }
        prim->r0 = 0xFF;
        prim->g0 = D_800F0BA8[arg0];
        prim->b0 = 0x60;
        *(s16 *)D_800A34A8 = 0xC0;
        *(s16 *)D_800A34AC = 0xC0;
        break;
    case 5:
        if (D_800F0BA8[4] > 0x800) {
            D_800A3488 = (s32)D_8009B8E0;
        } else {
    case 0:
            D_800A3488 = (s32)D_8009B8C8;
        }
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0xFF;
        break;
    case 10:
    case 11:
        if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            D_800A3488 = (s32)D_8009B9D8;
        } else {
            D_800A3488 = (s32)D_8009B990;
        }
        tbl = D_800F0BA8;
        t = tbl + arg0;
        if (*t > 0x96) {
            prim->r0 = 0;
            prim->g0 = (0xFF - *t) * 0xFF / 105;
        } else {
            prim->r0 = 0;
            prim->g0 = 0xFF;
        }
        prim->b0 = 0x7F - *t / 2;
        goto size_sel;
    case 6:
    case 7:
        if (D_800F0BA8[arg0] < 0x96) {
            if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
                D_800A3488 = (s32)D_8009B9D8;
            } else {
                D_800A3488 = (s32)D_8009B988;
            }
        } else if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            D_800A3488 = (s32)D_8009B9E0;
        } else {
            D_800A3488 = (s32)D_8009B990;
        }
        prim->r0 = 0x10;
        prim->g0 = ~D_800F0BA8[arg0];
        prim->b0 = 0xFF;
    size_sel:
        if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            *(s16 *)D_800A34A8 = 0x80;
            *(s16 *)D_800A34AC = 0x80;
            *(s32 *)D_800A3490 = 0x2F;
        } else {
            *(s16 *)D_800A34A8 = 0x20;
            *(s16 *)D_800A34AC = 0x80;
            *p_tw = 0x40;
            *p_th = 0x10;
        }
        break;
    case 8:
    case 9:
        if (D_800F0BA8[arg0 - 2] < 0xC8) {
            D_800A3488 = (s32)D_8009B980;
        } else {
            D_800A3488 = (s32)D_8009B978;
        }
        prim->r0 = D_800F0BA8[arg0 - 2] * 2 / 3;
        prim->g0 = D_800F0BA8[arg0 - 2] / 2;
        prim->b0 = 0xFF;
        *(s16 *)D_800A34A8 = 0x100;
        *(s16 *)D_800A34AC = 0x100;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        prim->r0 = 0xC0;
        prim->g0 = 0x70;
        prim->b0 = 0x13;
        *(s16 *)D_800A34A8 *= 2;
        *(s16 *)D_800A34AC *= 2;
        if (D_800F0BA8[arg0] >= 8) {
            D_800A3488 = (s32)D_8009B8C8;
            n = 10 - D_800F0BA8[arg0];
            prim->r0 = n << 6;
            prim->g0 = n * 0x70 / 3;
            prim->b0 = n * 0x60 / 15;
            *(s32 *)D_800A3490 = 0x2E;
        } else {
            if (D_800F0BA8[arg0] >= 5) {
                D_800A3488 = (s32)D_8009B9F0;
            } else {
                D_800A3488 = (s32)D_8009B9E8;
            }
            *(s32 *)D_800A3490 = 0x2F;
        }
        break;
    case 16:
    case 17:
        D_800A3488 = (s32)D_8009B8C8;
        prim->r0 = 0xFF;
        prim->g0 = 0x60;
        prim->b0 = 0xFF;
        break;
    }
    *(s32 *)D_800A3494 = (u16)(((((u16 *)D_800A3488)[0] >> 4) & 0x3F) + (((u16 *)D_800A3488)[1] << 6));
    *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
    *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
    *(u16 *)D_800A349C = *p_tw + ((u16 *)D_800A3488)[2] - 1;
    *(u16 *)D_800A34A4 = *p_th + ((u16 *)D_800A3488)[3] - 1;
    *(s32 *)D_800A34B0 = ReadGeomScreen() * 1000;
    *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 / *(s32 *)D_800A34D0;
    *p_w = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x200
               ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 8 : 6;
    *p_h = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x200
               ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 8 : 6;
    SetPolyFT4(prim);
    prim->tpage = *(s32 *)D_800A3490;
    prim->clut = *(s32 *)D_800A3494;
    prim->u0 = *(u16 *)D_800A3498;
    prim->v0 = *(u16 *)D_800A34A0;
    prim->u1 = *(u16 *)D_800A349C;
    prim->v1 = *(u16 *)D_800A34A0;
    prim->u2 = *(u16 *)D_800A3498;
    prim->v2 = *(u16 *)D_800A34A4;
    prim->u3 = *(u16 *)D_800A349C;
    prim->v3 = *(u16 *)D_800A34A4;
    SetShadeTex((s32)prim, 0);
    SetSemiTrans(prim, 1);
    switch (arg0) {
    case 0:
        *p_w = (u16)*p_w * 8 + (*p_w * rcos(D_800F0BA8[arg0] & 0xFFF) >> 9);
        *p_h = (u16)*p_h * 2 + (*p_h * rcos((D_800F0BA8[arg0] + 0x7FF) & 0xFFF) >> 11);
        prim->x0 = *(s32 *)D_800A34B8 - *p_w / 2;
        prim->y0 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        prim->x1 = *(s32 *)D_800A34B8 + *p_w / 2;
        prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        prim->x2 = *(s32 *)D_800A34B8 - *p_w / 2;
        prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
        prim->x3 = *(s32 *)D_800A34B8 + *p_w / 2;
        prim->y3 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
        break;
    case 1:
    case 2:
    case 16:
    case 17:
        *p_w = (*p_w * ((rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25) >> 13) / 2;
        sh = rsin((D_800F0BA8[arg0] << 11) / 4551) * 15;
        *p_h = (*p_h * sh >> 12) / 2;
        i = 0;
        p_v->vy = 0;
        p_v->vx = 0;
        p_v->vz = D_800F0BA8[arg0];
        RotMatrix((s16 *)p_v, (u8 *)p_mat);
        dst = p_t;
        SetRotMatrix((u8 *)p_mat);
        do {
            w = *p_w;
            if (!(i & 1)) {
                p_v->vx = -w;
            } else {
                p_v->vx = w;
            }
            h = *p_h;
            if (!(i & 2)) {
                p_v->vy = -h;
            } else {
                p_v->vy = h;
            }
            ApplyRotMatrix(p_v, dst);
            dst++;
        } while (++i < 4);
        prim->x0 = *(s32 *)D_800A34B8 + p_t[0].vx / 2;
        prim->y0 = (*(s32 *)D_800A34B8 >> 16) + p_t[0].vy / 4;
        prim->x1 = *(s32 *)D_800A34B8 + p_t[1].vx / 2;
        prim->y1 = (*(s32 *)D_800A34B8 >> 16) + p_t[1].vy / 4;
        prim->x2 = *(s32 *)D_800A34B8 + p_t[2].vx / 2;
        prim->y2 = (*(s32 *)D_800A34B8 >> 16) + p_t[2].vy / 4;
        prim->x3 = *(s32 *)D_800A34B8 + p_t[3].vx / 2;
        prim->y3 = (*(s32 *)D_800A34B8 >> 16) + p_t[3].vy / 4;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        *p_w = *p_w * rsin((D_800F0BA8[arg0] * 0x3FF / 10) & 0xFFF) >> 11;
        *p_h = *p_h * rsin((D_800F0BA8[arg0] * 0x3FF / 10) & 0xFFF) >> 11;
        goto quad;
    case 8:
    case 9:
        *p_w = *p_w * rsin((D_800F0BA8[arg0 - 2] / 2) & 0xFFF) >> 11;
        *p_h = *p_h * rsin((D_800F0BA8[arg0 - 2] / 2) & 0xFFF) >> 11;
        goto quad;
    case 3:
    case 4:
        *p_w = *p_w * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
        *p_h = *p_h * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
    quad:
        prim->x0 = *(s32 *)D_800A34B8 - *p_w;
        prim->y0 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        prim->x1 = *p_w + *(s32 *)D_800A34B8;
        prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        prim->x2 = *(s32 *)D_800A34B8 - *p_w;
        prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
        prim->x3 = *p_w + *(s32 *)D_800A34B8;
        prim->y3 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
        break;
    case 5:
        if (D_800F0BA8[arg0] < 0x1000) {
            *p_w = (u16)*p_w * 8 + (*p_w * rcos(D_800F0BA8[arg0] & 0xFFF) >> 9);
            *p_h = (u16)*p_h * 2 + (*p_h * rcos((D_800F0BA8[arg0] + 0x7FF) & 0xFFF) >> 11);
        } else {
            *p_h = 0;
            *p_w = 0;
        }
        prim->x0 = *(s32 *)D_800A34B8 - *p_w / 4;
        prim->y0 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 4;
        prim->x1 = *(s32 *)D_800A34B8 + *p_w / 4;
        prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 4;
        prim->x2 = *(s32 *)D_800A34B8 - *p_w / 4;
        prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 4;
        prim->x3 = *(s32 *)D_800A34B8 + *p_w / 4;
        prim->y3 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 4;
        break;
    case 6:
    case 10:
        if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            *p_w = -*p_w;
        }
    case 7:
    case 11:
        if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            *p_w = *p_w * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
            *p_h = *p_h * rsin(D_800F0BA8[arg0] & 0xFFF) >> 11;
        } else {
            *p_w = *p_w + (*p_w * rsin(D_800F0BA8[arg0] * 0x300 / 255) >> 11);
            *p_h = *p_h * rcos((D_800F0BA8[arg0] << 9) / 255) >> 12;
        }
        if ((D_8009BD44[0] & 8) && D_800A3834 != 5) {
            prim->x0 = *(s32 *)D_800A34B8 - *p_w;
            prim->y0 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
            prim->x1 = *p_w + *(s32 *)D_800A34B8;
            prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *(s32 *)D_800A34B8 - *p_w;
            prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *p_w + *(s32 *)D_800A34B8;
            prim->y3 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
        } else if (D_800F0BA8[arg0] < 0x96) {
            prim->x0 = *(s32 *)D_800A34B8 - *p_w / 2;
            prim->y0 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
            prim->x1 = *(s32 *)D_800A34B8 - *p_w / 2;
            prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *(s32 *)D_800A34B8 + *p_w / 2;
            prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *(s32 *)D_800A34B8 + *p_w / 2;
            prim->y3 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        } else {
            prim->x0 = *(s32 *)D_800A34B8;
            prim->y0 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
            prim->x1 = *(s32 *)D_800A34B8;
            prim->y1 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
            prim->x2 = *p_w + *(s32 *)D_800A34B8;
            prim->y2 = (*(s32 *)D_800A34B8 >> 16) + *p_h / 2;
            prim->x3 = *p_w + *(s32 *)D_800A34B8;
            prim->y3 = (*(s32 *)D_800A34B8 >> 16) - *p_h / 2;
        }
        if (arg0 < 9) {
            AddPrim(g_gpu_ot_ptr + 4, (s32)prim);
            if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
                prim++;
            }
            arg0 += 2;
            goto again;
        }
        break;
    }
    AddPrim(g_gpu_ot_ptr + 4, (s32)prim);
    if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
        prim++;
    }
    D_800A37D4 = (s32)prim;
    return 1;
}
