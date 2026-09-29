/* Draw the up-to-12 flare slots func_80062FEC spawns: per live slot i (bit i
   of D_800A3448, age D_800F0BEC[i], world position D_800F0E38[i]) emit two
   textured POLY_FT4 billboards -- j == 0 the flare itself, j == 1 a halo that
   bobs above it -- each rotated/translated relative to the camera, projected,
   sized from its depth and animated by its age, then linked into the OT at its
   depth. A slot whose age reaches 16 clears its bit. Returns nonzero while any
   slot is still live. */
s32 func_80063084(void) {
    extern s32 D_8009BD44[];
    extern u16 D_8009B940[];
    extern u16 D_8009B948[];
    extern u16 D_8009B950[];
    extern void ApplyRotMatrix(s16 *, s32 *);
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    u8 *base;
    POLY_FT4 *prim;
    VECTOR *tv;
    SVECTOR *sv;
    SVECTOR *v;
    s32 *interp;
    s32 *fade;
    s32 *z;
    s16 i;
    s16 j;
    s32 bit;
    s32 scale;
    s32 level;

    base = (u8 *)D_800A34EC;
    prim = (POLY_FT4 *)D_800A37D4;
    tv = (VECTOR *)(base + 0x14);
    sv = (SVECTOR *)(base + 0x24);
    v = (SVECTOR *)(base + 0x2C);
    interp = (s32 *)(base + 0x34);
    fade = (s32 *)(base + 0x38);
    z = (s32 *)(base + 0x3C);
    /* PsyQ libgte inline macro gte_SetRotMatrix(r0) --- PsyQ Run-time Library
     * Release 4.3 inline_c.h (DMPSX v3) :297-310,
     * verbatim body, operand and clobbers. */
        v->vz = 0;
    v->vy = 0;
    v->vx = 0;
    *(s32 *)D_800A34B0 = ReadGeomScreen() * 1000;
    *(s16 *)D_800A34A8 = 0xC0;
    *(s16 *)D_800A34AC = 0x60;
    *(s32 *)D_800A3490 = 0x2E;
    *(s32 *)D_800A3490 = *(s32 *)D_800A3490 << 16;
    for (i = 0; i < 12; i++) {
        bit = 1 << i;
        if (!(D_800A3448 & bit)) {
            continue;
        }
        for (j = 0; j < 2; j++) {
            if (D_800F0BEC[i] < 16) {
                sv->vx = D_800F0E38[i].unk0 - ((s32 *)D_800A3470)[0];
                sv->vz = D_800F0E38[i].unk8 - ((s32 *)D_800A3470)[2];
                if (j != 0 && D_800F0BEC[i] >= 3) {
                    sv->vy = D_800F0E38[i].unk4
                        - (rsin((D_800F0BEC[i] - 3) << 8) * 329 / 4096 - 69) / 10
                        - ((s32 *)D_800A3470)[1];
                } else {
                    sv->vy = D_800F0E38[i].unk4 - ((s32 *)D_800A3470)[1];
                }
                ApplyRotMatrix(&sv->vx, &tv->vx);
                /* SetTransMatrix reads only m->t (+0x14): hand it the address
                   0x14 below tv so tv is loaded as the translation. */
                SetTransMatrix((u8 *)tv - 0x14);
                RotTransPers(v, (s32 *)D_800A34B8, interp, (s32 *)D_800A34CC);
                /* PsyQ libgte inline macro gte_stsz(r0) --- PsyQ Run-time
                 * Library Release 4.3 inline_c.h (DMPSX v3) :1042-1046,
                 * verbatim body, operand and clobbers. */
                                if (j == 0) {
                    D_800A3488 = (s32)D_8009B940;
                } else if (D_800F0BEC[i] < 11) {
                    D_800A3488 = (s32)D_8009B948;
                } else {
                    D_800A3488 = (s32)D_8009B950;
                }
                *(s32 *)D_800A3494 = (((((u16 *)D_800A3488)[0] >> 4) & 0x3F) + (((u16 *)D_800A3488)[1] << 6)) << 16;
                *(u16 *)D_800A3498 = ((u16 *)D_800A3488)[2];
                *(u16 *)D_800A34A0 = ((u16 *)D_800A3488)[3];
                *(u16 *)D_800A349C = ((u16 *)D_800A3488)[2] + 0x3F;
                *(u16 *)D_800A34A4 = ((u16 *)D_800A3488)[3] + 0x3F;
                *(u16 *)D_800A34D4 = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A0 << 8);
                *(u16 *)D_800A34D8 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A0 << 8);
                *(u16 *)D_800A34DC = *(u16 *)D_800A3498 + (*(u16 *)D_800A34A4 << 8);
                *(u16 *)D_800A34E0 = *(u16 *)D_800A349C + (*(u16 *)D_800A34A4 << 8);
                *z = func_80052C28(*(s32 *)D_800A34D0, 0);
                if (*z < 0x1005 && *(s32 *)D_800A34B0 / 1000 >> 4 < *z) {
                    *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 / *(s32 *)D_800A34D0;
                    /* Sprite size = base size * depth scale, in 1/256 units,
                       at least 8. */
                    *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x800
                        ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 8 : 8;
                    *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x800
                        ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 8 : 8;
                    if (j == 0) {
                        if (D_800F0BEC[i] < 5) {
                            *(s16 *)D_800A34C0 = *(s16 *)D_800A34C0 * (rcos((D_800F0BEC[i] << 10) / 5) * 3 / 4096 + 7) / 10;
                        } else if (D_800F0BEC[i] < 16) {
                            *(s16 *)D_800A34C0 = *(s16 *)D_800A34C0 * (rcos(((D_800F0BEC[i] - 5) << 10) / 11) * 7 / 4096) / 10;
                        }
                        if (D_800F0BEC[i] < 10) {
                            *(s32 *)D_800A34C8 = *(s32 *)D_800A34C8 * (rsin((D_800F0BEC[i] << 10) / 10) * 85 / 4096) / 100;
                        } else if (D_800F0BEC[i] < 16) {
                            *(s32 *)D_800A34C8 = *(s32 *)D_800A34C8 * (rcos(((D_800F0BEC[i] - 10) << 10) / 6) * 85 / 4096) / 100;
                        }
                        if (D_8009BD44[0] & 1) {
                            *(s32 *)&prim->r0 = 0x808080;
                        } else {
                            *(s32 *)&prim->r0 = 0xFF8080;
                        }
                    } else {
                        scale = rsin((D_800F0BEC[i] << 10) / 12) * 12 / 4096;
                        *(s16 *)D_800A34C0 = *(s16 *)D_800A34C0 * scale / 10;
                        *(s32 *)D_800A34C8 = *(s32 *)D_800A34C8 * scale / 10;
                        if (D_800F0BEC[i] >= 9) {
                            /* fade = age - 15 <= 0: the halo fades out over ages 9..15. */
                            *fade = D_800F0BEC[i] - 15;
                            level = (*fade * -128 / 5) & 0xFF;
                            *(s32 *)&prim->r0 = level + (level << 8) + ((*fade * -255 / 5 & 0xFF) << 16);
                        } else {
                            *(s32 *)&prim->r0 = 0;
                        }
                    }
                    ((u8 *)prim)[3] = 9;
                    prim->code = 0x2E;
                    *(s16 *)D_800A34C0 = *(s16 *)D_800A34C0 >> 1;
                    *(s16 *)D_800A34BC = -*(s16 *)D_800A34C0;
                    *(s32 *)D_800A34C4 = *(s32 *)D_800A34C8 * 60 / 64;
                    *(s32 *)D_800A34C8 = 0;
                    *(s32 *)&prim->x0 = *(s32 *)D_800A34B8 + *(s16 *)D_800A34BC - (*(s32 *)D_800A34C4 << 16);
                    *(s32 *)&prim->x1 = *(s32 *)D_800A34B8 + *(s16 *)D_800A34C0 - (*(s32 *)D_800A34C4 << 16);
                    *(s32 *)&prim->x2 = *(s32 *)D_800A34B8 + *(s16 *)D_800A34BC + (*(s32 *)D_800A34C8 << 16);
                    *(s32 *)&prim->x3 = *(s32 *)D_800A34B8 + *(s16 *)D_800A34C0 + (*(s32 *)D_800A34C8 << 16);
                    *(s32 *)&prim->u0 = *(u16 *)D_800A34D4 + *(s32 *)D_800A3494;
                    *(s32 *)&prim->u1 = *(u16 *)D_800A34D8 + *(s32 *)D_800A3490;
                    *(u16 *)&prim->u2 = *(u16 *)D_800A34DC;
                    *(u16 *)&prim->u3 = *(u16 *)D_800A34E0;
                    D_800A34E4 = g_gpu_ot_ptr + *z * 4;
                    D_800A34E8 = (s32)prim;
                    *(u32 *)prim = (*(u32 *)prim & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);
                    *(u32 *)D_800A34E4 = (D_800A34E8 & 0xFFFFFF) | (*(u32 *)D_800A34E4 & 0xFF000000);
                    if (prim - (POLY_FT4 *)D_800A3720 < 0x1C1) {
                        prim++;
                    }
                }
            } else {
                D_800A3448 &= 0xFFFF - (1 << i);
            }
        }
        D_800F0BEC[i]++;
    }
    D_800A37D4 = (s32)prim;
    return D_800A3448 != 0;
}
