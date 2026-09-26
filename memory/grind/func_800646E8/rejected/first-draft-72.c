extern void ApplyRotMatrixLV(VECTOR *, VECTOR *);
/* Draw the up-to-16 effect slots func_800645B0 spawns: per live slot (bit i
   of D_800A3444) advance its counter, and while the counter's frame (/4) is
   below 7 project the slot's world position through D_800A3474 and emit one
   textured, semi-transparent POLY_FT4 billboard sized by the screen depth;
   once the frame reaches 7 the slot is retired. Finally link every new quad
   into the OT at its depth. Returns 1 when any slot is still live, else 0. */
s32 func_800646E8(void) {
    extern u16 D_8009B8E8[][4];
    extern s32 D_8009BD44;
    extern s32 D_800A3720;
    extern s32 D_800A37D4;
    u8 *base;
    u32 *zbuf;
    s16 *w;
    s16 *h;
    s32 *frame;
    VECTOR *trans;
    VECTOR *pos;
    SVECTOR *sv;
    s32 *p;
    u32 *end;
    POLY_FT4 *prim;
    u32 *zp;
    s16 i;
    s32 bit;
    s32 sw;
    s32 sh;

    base = (u8 *)D_800A34EC;
    zbuf = (u32 *)(base + 0x48);
    w = (s16 *)(base + 0x10);
    h = (s16 *)(base + 0x12);
    frame = (s32 *)(base + 0x14);
    trans = (VECTOR *)(base + 0x18);
    pos = (VECTOR *)(base + 0x28);
    sv = (SVECTOR *)(base + 0x38);
    p = (s32 *)(base + 0x40);
    end = (u32 *)(base + 0x44);
    prim = (POLY_FT4 *)D_800A37D4;
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
    sv->vx = sv->vy = sv->vz = 0;
    *(s32 *)D_800A34B0 = ReadGeomScreen() * 1000;
    *(s16 *)D_800A34A8 = 0x40;
    *(s16 *)D_800A34AC = 0x20;
    *(s32 *)D_800A3490 = 0xE;
    zp = zbuf;
    for (i = 0; i < 16; i++) {
        bit = 1 << i;
        if (!(D_800A3444 & bit)) {
            continue;
        }
        (*(s16 *)((s32)&D_800F0BCC + i * 2))++;
        *frame = *(s16 *)((s32)&D_800F0BCC + i * 2) / 4;
        if (*frame < 7) {
            pos->vx = *(s32 *)((s32)&D_800F0D78 + i * 12) - ((s32 *)D_800A3470)[0];
            pos->vy = *(s32 *)((s32)&D_800F0D7C + i * 12) - ((s32 *)D_800A3470)[1];
            pos->vz = *(s32 *)((s32)&videoDec + i * 12) - ((s32 *)D_800A3470)[2];
            ApplyRotMatrixLV(pos, trans);
            SetTransMatrix((u8 *)trans - 0x14);
            RotTransPers((s32 *)sv, (s32 *)D_800A34B8, p, (s32 *)D_800A34CC);
            /* gte_stsz(r0) --- inline_c.h :1042-1046 */
            __asm__ volatile(
                "swc2   $19, 0(%0)\n"
                :: "r"(D_800A34D0) : "memory");
            D_800A3488 = (s32)D_8009B8E8[*frame];
            *(s32 *)D_800A3494 = (u16)(((((u16 *)D_800A3488)[0] >> 4) & 0x3F) + (((u16 *)D_800A3488)[1] << 6));
            *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
            *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
            *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0x3F;
            *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0x1F;
            *zp = func_80052C28(*(s32 *)D_800A34D0, 0);
            if (*zp >= 0x1005) {
                continue;
            }
            if ((u32)((*(s32 *)D_800A34B0 / 1000) >> 4) >= *zp) {
                continue;
            }
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 / *(s32 *)D_800A34D0;
            sw = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4;
            *w = sw > 0x800 ? sw >> 8 : 8;
            sh = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4;
            *h = sh > 0x800 ? sh >> 8 : 8;
            *w += (*w * *(s16 *)((s32)&D_800F0BCC + i * 2)) >> 5;
            *h += (*h * (*(s16 *)((s32)&D_800F0BCC + i * 2) * 4)) >> 8;
            SetPolyFT4(prim);
            prim->tpage = *(s32 *)D_800A3490;
            prim->clut = *(s32 *)D_800A3494;
            if (D_8009BD44 & 1) {
                prim->r0 = 0x80;
                prim->g0 = 0x80;
                prim->b0 = 0x80;
            } else {
                prim->r0 = 0xA0;
                prim->g0 = 0x8C;
                prim->b0 = 0x50;
            }
            prim->x0 = *(s32 *)D_800A34B8 - *w / 2;
            prim->y0 = ((s16 *)D_800A34B8)[1] - *h * 28 / 32;
            prim->x1 = *(s32 *)D_800A34B8 + *w / 2;
            prim->y1 = ((s16 *)D_800A34B8)[1] - *h * 28 / 32;
            prim->x2 = *(s32 *)D_800A34B8 - *w / 2;
            prim->y2 = ((s16 *)D_800A34B8)[1] + *h / 8;
            prim->x3 = *(s32 *)D_800A34B8 + *w / 2;
            prim->y3 = ((s16 *)D_800A34B8)[1] + *h / 8;
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
            if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
                prim++;
                zp++;
            }
            (*(s16 *)((s32)&D_800F0BCC + i * 2))++;
        } else {
            D_800A3444 &= 0xFFFF - bit;
        }
    }
    if (D_800A3444 == 0) {
        return 0;
    }
    *end = (u32)prim;
    for (prim = (POLY_FT4 *)D_800A37D4; (u32)prim < *end; prim++, zbuf++) {
        D_800A34E8 = (s32)prim;
        D_800A34E4 = g_gpu_ot_ptr + *zbuf * 4;
        *(u32 *)D_800A34E8 = (*(u32 *)D_800A34E8 & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);
        *(u32 *)D_800A34E4 = (D_800A34E8 & 0xFFFFFF) | (*(u32 *)D_800A34E4 & 0xFF000000);
    }
    D_800A37D4 = *end;
    return 1;
}
