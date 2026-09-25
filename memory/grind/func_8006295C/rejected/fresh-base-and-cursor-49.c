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
} PolyFT4T;
extern void RotMatrixZYX(s16 *, u8 *);
extern void ScaleMatrix(u8 *, s32 *);
extern void CompMatrix(s32, u8 *, u8 *);
extern s32 SetShadeTex(s32, s32);
extern void SetPolyFT4(void *);
extern s32 RotTransPers4(s16 *, s16 *, s16 *, s16 *, s32 *, s32 *, s32 *, s32 *, s32 *, s32);
s32 func_8006295C(void) {
    extern s16 D_800F0C04[];
    extern u16 D_8009B958[];
    extern u16 D_8009B960[];
    extern u16 D_8009B968[];
    extern u16 D_8009B970[];
    extern s16 D_8009BB84[];
    extern s32 D_800A3720;
    extern s32 D_800A37D4;
    s32 count;
    u8 *base;
    MATRIX *cm;
    s32 *interp;
    u16 *zbuf;
    s32 *scale;
    s32 *shade;
    MATRIX *mats;
    MATRIX *m;
    s32 *sxy;
    PolyFT4T *prim;
    s32 i;
    s32 j;
    s32 k;
    s32 v;
    s32 c;
    s32 bit;
    s16 *sv;
    PolyFT4T *end;
    PolyFT4T *p;
    u16 *zp;

    base = (u8 *)D_800A34EC;
    count = 0;
    mats = (MATRIX *)(base + 0x78);
    cm = (MATRIX *)(base + 0x138);
    sxy = (s32 *)(base + 0x158);
    interp = (s32 *)(base + 0x168);
    zbuf = (u16 *)(base + 0x16C);
    scale = (s32 *)(base + 0x178);
    shade = (s32 *)(base + 0x188);
    prim = (PolyFT4T *)D_800A37D4;
    for (i = 0; i < 6; i++) {
        bit = 1 << i;
        if (!(D_800A3460 & bit)) {
            continue;
        }
        m = &mats[i];
        v = rsin(((D_800F0C04[i] + 6) << 10) / 6);
        scale[0] = scale[1] = scale[2] = v + (D_800F0C04[i] << 12) / 6;
        RotMatrixZYX((s16 *)((s32)&D_800F10A0 + i * 8), (u8 *)m);
        ScaleMatrix((u8 *)m, scale);
        m->t[0] = *(s32 *)((s32)&D_800F0FB8 + i * 12) - ((s32 *)D_800A3470)[0];
        m->t[1] = *(s32 *)((s32)&D_800F0FBC + i * 12) - ((s32 *)D_800A3470)[1];
        m->t[2] = *(s32 *)((s32)&D_800F0FC0 + i * 12) - ((s32 *)D_800A3470)[2];
        CompMatrix(D_800A3474, (u8 *)m, (u8 *)cm);
        SetRotMatrix((u8 *)cm);
        SetTransMatrix((u8 *)cm);
        for (j = 0; j < 3; j++) {
            if (D_800F0C04[i] < 3) {
                *(s32 *)&prim->r0 = 0x808080;
            } else {
                c = ((6 - D_800F0C04[i]) << 7) / 3;
                *shade = c;
                *(s32 *)&prim->r0 = c + (c << 8) + (c << 16);
            }
            if (D_800F0C04[i] < 3) {
                if (j) {
                    D_800A3488 = (s32)D_8009B960;
                } else {
                    D_800A3488 = (s32)D_8009B958;
                }
            } else {
                if (j) {
                    D_800A3488 = (s32)D_8009B970;
                } else {
                    D_800A3488 = (s32)D_8009B968;
                }
            }
            if (j) {
                *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0x3F;
            } else {
                *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0x1F;
            }
            *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0x1F;
            *(s32 *)D_800A3490 = 0x2E;
            *(s32 *)D_800A3494 = (((((u16 *)D_800A3488)[0] >> 4) & 0x3F) + (((u16 *)D_800A3488)[1] << 6)) << 16;
            *(s32 *)D_800A3490 = *(s32 *)D_800A3490 << 16;
            *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
            *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
            *(u16 *)D_800A34D4 = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A0 << 8);
            *(u16 *)D_800A34D8 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A0 << 8);
            *(u16 *)D_800A34DC = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A4 << 8);
            *(u16 *)D_800A34E0 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A4 << 8);
            ((u8 *)prim)[3] = 9;
            prim->code = 0x2E;
            *(s32 *)&prim->u0 = *(u16 *)D_800A34D4 + *(s32 *)D_800A3494;
            *(s32 *)&prim->u1 = *(u16 *)D_800A34D8 + *(s32 *)D_800A3490;
            *(u16 *)&prim->u2 = *(u16 *)D_800A34DC;
            *(u16 *)&prim->u3 = *(u16 *)D_800A34E0;
            SetPolyFT4(prim);
            SetShadeTex((s32)prim, 1);
            SetSemiTrans(prim, 1);
            sv = &D_8009BB84[j * 16];
            RotTransPers4(sv, sv + 4, sv + 8, sv + 12,
                          sxy, sxy + 1, sxy + 2, sxy + 3, interp, D_800A34CC);
            /* gte_stsz(r0) --- inline_c.h :1042-1046 */
            __asm__ volatile(
                "swc2   $19, 0(%0)\n"
                :: "r"(D_800A34D0) : "memory");
            *(s32 *)D_800A34D0 = func_80052C28(*(s32 *)D_800A34D0, 0);
            if (*(s32 *)D_800A34D0 == 0) {
                *(s32 *)D_800A34D0 = 1;
            }
            if (*(s32 *)D_800A34D0 < 0x1005) {
                zbuf[count] = *(s32 *)D_800A34D0;
                *(s32 *)&prim->x0 = sxy[0];
                *(s32 *)&prim->x1 = sxy[1];
                *(s32 *)&prim->x2 = sxy[2];
                *(s32 *)&prim->x3 = sxy[3];
                count++;
                if (prim - (PolyFT4T *)D_800A3720 < 0x1C1) {
                    prim++;
                }
            }
        }
        D_800F0C04[i]++;
        if (D_800F0C04[i] >= 7) {
            D_800A3460 &= ~(1 << i);
        }
    }
    if (D_800A37D4 != (s32)prim) {
        end = prim;
        for (p = (PolyFT4T *)D_800A37D4, k = 0; p < end; p++, k++) {
            AddPrim(g_gpu_ot_ptr + zbuf[k] * 4, (s32)p);
        }
        D_800A37D4 = (s32)end;
        return 1;
    }
    if (D_800A3460 == 0) {
        D_800F1138 = 0;
        return 0;
    }
}
