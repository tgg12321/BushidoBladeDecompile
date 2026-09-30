typedef struct {
    u8 addr[3];
    u8 len;
    u32 code;
    u32 xy0;
    u32 uv0;
    u32 xy1;
    u32 uv1;
    u32 xy2;
    u16 uv2;
    u16 pad2;
    u32 xy3;
    u16 uv3;
    u16 pad3;
} PolyW;
u8 func_80067D14(s32 arg0, s32 arg1) {
    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3720;
    extern s32 D_800A3724;
    extern s32 D_8009BD44[];
    s32 outer = D_800A34EC;
    u32 *p_rad;
    VECTOR *p_tv;
    s16 *p_count;
    s16 *p_idx;
    u8 *p_r;
    u8 *p_g;
    u8 *p_b;
    Unk800F0C10Record **p_tgt;
    s16 *p_ot;
    s32 *p_seed;
    s32 *p_out;
    VECTOR *p_work;
    SVECTOR *p_vert;
    s16 *p_life;
    s16 *p_n;
    PolyW **p_prim;
    Unk800EFC78Record **p_ent;
    s32 seed;
    u32 sum;
    s16 *sxy;

    D_800A3724 = outer + 0x1AC;
    p_seed = (s32 *)(outer + 0x1AC);
    *p_seed = rand();
    p_rad = (u32 *)(outer + 4);
    p_out = (s32 *)(outer + 0x24);
    p_work = (VECTOR *)(outer + 0x34);
    p_vert = (SVECTOR *)(outer + 0x44);
    p_tv = (VECTOR *)(outer + 0x5C);
    p_count = (s16 *)(outer + 0x6C);
    p_idx = (s16 *)(outer + 0x6E);
    p_life = (s16 *)(outer + 0x70);
    p_r = (u8 *)(outer + 0x72);
    p_g = (u8 *)(outer + 0x73);
    p_b = (u8 *)(outer + 0x74);
    p_n = (s16 *)(outer + 0x78);
    p_prim = (PolyW **)(outer + 0x80);
    p_ent = (Unk800EFC78Record **)(outer + 0x84);
    p_tgt = (Unk800F0C10Record **)(outer + 0x88);
    p_ot = (s16 *)(outer + 0x8C);

    for (*p_idx = 0; *p_idx < *p_count; (*p_idx)++) {
        *p_ent = &D_800EFC78[arg1][*p_idx];
        if ((*p_ent)->unk10 != 1) {
            continue;
        }
        (*p_ent)->unk12++;
        *p_life = (*p_ent)->unk12;
        *p_r = (*p_life << 2) < 0xFF ? ~(*p_life << 2) : 0;
        *p_g = (*p_life << 5) < 200 ? 200 - (*p_life << 5) : 0;
        if (*p_r <= 0x50 && *p_g <= 0x20) {
            goto kill;
        }
        *p_b = (*p_life << 5) < 100 ? 100 - (*p_life << 5) : 0;

        *p_tgt = &D_800F0C10[arg1][(*p_ent)->unk6];
        p_tv->vx = (*p_tgt)->unk0;
        p_vert[0].vx = p_tv->vx - ((s32 *)D_800A3470)[0];
        p_tv->vy = (*p_tgt)->unk4;
        p_vert[0].vy = p_tv->vy - ((s32 *)D_800A3470)[1];
        p_tv->vz = (*p_tgt)->unk8;
        p_vert[0].vz = p_tv->vz - ((s32 *)D_800A3470)[2];

        /* gte_ldv0(r0) --- inline_c.h :16-20 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            :: "r"(p_vert));
        /* gte_rtv0() --- inline_c.h :499-502, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A486012\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(p_out) : "memory");
        /* gte_SetTransMatrix(r0) --- inline_c.h :360-370 */
        __asm__ volatile(
            "lw     $12, 20(%0)\n"
            "lw     $13, 24(%0)\n"
            "ctc2   $12, $5\n"
            "lw     $14, 28(%0)\n"
            "ctc2   $13, $6\n"
            "ctc2   $14, $7\n"
            :: "r"((u8 *)p_out - 0x14) : "$12", "$13", "$14");

        p_vert[1].vx = (*p_ent)->unk0;
        p_vert[1].vz = (*p_ent)->unk4;
        p_vert[1].vy = (*p_ent)->unk2;
        p_work->vx = (s16)((*p_ent)->unk8 / 30);
        (*p_ent)->unk0 += p_work->vx;
        p_vert[0].vx = (*p_ent)->unk0;
        p_work->vz = (s16)((*p_ent)->unkC / 30);
        (*p_ent)->unk4 += p_work->vz;
        p_vert[0].vz = (*p_ent)->unk4;
        p_work->vy = (s16)((*p_ent)->unkA / 30) + *p_life * 9800 / 900;
        (*p_ent)->unk2 += p_work->vy;
        p_vert[0].vy = (*p_ent)->unk2;
        p_vert[2].vx = p_vert[1].vx + (p_work->vx >> 1);
        p_vert[2].vz = p_vert[1].vz + (p_work->vz >> 1);
        p_vert[2].vy = p_vert[1].vy + (p_work->vy >> 1);

        /* gte_ldv3(r0, r1, r2) --- inline_c.h :34-42 (no clobber list) */
        __asm__ volatile(
            "lwc2   $0, 0(%0)\n"
            "lwc2   $1, 4(%0)\n"
            "lwc2   $2, 0(%1)\n"
            "lwc2   $3, 4(%1)\n"
            "lwc2   $4, 0(%2)\n"
            "lwc2   $5, 4(%2)\n"
            :: "r"(&p_vert[0]), "r"(&p_vert[1]), "r"(&p_vert[2]));
        p_work->vx = p_vert[0].vx;
        p_work->vz = p_vert[0].vz;
        p_work->vy = p_vert[0].vy;
        /* gte_ldlvl(r0) --- inline_c.h :112-117 (no clobber list) */
        __asm__ volatile(
            "lwc2   $9, 0(%0)\n"
            "lwc2   $10, 4(%0)\n"
            "lwc2   $11, 8(%0)\n"
            :: "r"(p_work));
        /* gte_sqr0() --- inline_c.h :719-722, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4AA00428\n");
        /* gte_stlvnl(r0) --- inline_c.h :1111-1117 */
        __asm__ volatile(
            "swc2   $25, 0(%0)\n"
            "swc2   $26, 4(%0)\n"
            "swc2   $27, 8(%0)\n"
            :: "r"(p_out) : "memory");

        seed = *p_seed ^ rand();
        *p_seed = seed;
        if ((u32)(p_out[0] + p_out[1] + p_out[2]) > *p_rad + seed * 3000 / 32768 * 3000) {
        kill:
            (*p_ent)->unk10 = 3;
            continue;
        }

        /* gte_rtpt() --- inline_c.h :489-492, post-DMPSX command word */
        __asm__ volatile(
            "nop\n"
            "nop\n"
            ".word 0x4A280030\n");
        /* gte_stsxy3(r0, r1, r2) --- inline_c.h :906-912 */
        __asm__ volatile(
            "swc2   $12, 0(%0)\n"
            "swc2   $13, 0(%1)\n"
            "swc2   $14, 0(%2)\n"
            :: "r"((s32 *)D_800A34B8), "r"((s32 *)D_800A34B8 + 1),
               "r"((s32 *)D_800A34B8 + 2) : "memory");
        /* gte_stsz(r0) --- inline_c.h :1042-1046 */
        __asm__ volatile(
            "swc2   $19, 0(%0)\n"
            :: "r"(D_800A34D0) : "memory");

        if (*(s32 *)D_800A34D0 < 0) {
            continue;
        }
        ((s32 *)D_800A34D0)[1] = *(s32 *)D_800A34D0 ? *(s32 *)D_800A34D0 : 1;
        *(s32 *)D_800A34D0 = func_80052C28(((s32 *)D_800A34D0)[1], 0);
        if (*(s32 *)D_800A34D0 == 0) {
            *(s32 *)D_800A34D0 = 1;
        }
        *(s32 *)D_800A34CC = 1;
        if (*(s32 *)D_800A34D0 >= 0x1005 || *(s32 *)D_800A34CC == 0) {
            continue;
        }

        if (arg0 < 2) {
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 * 200 / ((s32 *)D_800A34D0)[1];
            *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 9 : 1;
        } else if (arg0 < 4) {
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 * 200 / ((s32 *)D_800A34D0)[1];
            *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s16 *)D_800A34C0 += *(s16 *)D_800A34C0 * 20 / (*p_life * 6 + 1);
            *(s32 *)D_800A34C8 += *(s32 *)D_800A34C8 * 20 / (*p_life * 6 + 1);
        } else if (arg0 < 6) {
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 * 200 / ((s32 *)D_800A34D0)[1];
            *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s16 *)D_800A34C0 += *(s16 *)D_800A34C0 * *p_life / 2;
            *(s32 *)D_800A34C8 += *(s32 *)D_800A34C8 * *p_life / 2;
        } else if (arg0 < 8) {
            *(s32 *)D_800A34B4 = *(s32 *)D_800A34B0 * 200 / ((s32 *)D_800A34D0)[1];
            *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34A8 * *(s32 *)D_800A34B4 >> 9 : 1;
            *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 > 0x200
                                     ? *(s16 *)D_800A34AC * *(s32 *)D_800A34B4 >> 9 : 1;
            sxy = (s16 *)D_800A34B8;
            sxy[2] += rand() * 20 / 32768;
            sxy[3] += rand() * 10 / 32768;
        }
        if (*(s16 *)D_800A34C0 > *(s16 *)D_800A34A8 * 2) {
            *(s16 *)D_800A34C0 = *(s16 *)D_800A34A8 >> 1;
            *(s32 *)D_800A34C8 = *(s16 *)D_800A34AC >> 1;
        }
        *(s16 *)D_800A34BC = -*(s16 *)D_800A34C0;
        *(s32 *)D_800A34C4 = -*(s32 *)D_800A34C8;
        *(s32 *)D_800A34C8 <<= 16;
        *(s32 *)D_800A34C4 <<= 16;

        sum = p_out[0] + p_out[1] + p_out[2];
        if (*p_rad / 3 < sum) {
            *p_n = 1;
        } else if (*p_rad / 2 < sum) {
            *p_n = 0;
        } else {
            *p_n = 2;
        }
        if (*p_n < 0) {
            continue;
        }
        do {
            *(s16 *)((s32)p_ot + (*p_prim - (PolyW *)D_800A37D4) * 2) = *(s32 *)D_800A34D0;
            (*p_prim)->xy0 =
                ((s32 *)D_800A34B8)[*p_n] + *(s16 *)D_800A34BC + *(s32 *)D_800A34C4;
            (*p_prim)->xy1 =
                ((s32 *)D_800A34B8)[*p_n] + *(s16 *)D_800A34C0 + *(s32 *)D_800A34C4;
            (*p_prim)->xy2 =
                ((s32 *)D_800A34B8)[*p_n] + *(s16 *)D_800A34BC + *(s32 *)D_800A34C8;
            (*p_prim)->xy3 =
                ((s32 *)D_800A34B8)[*p_n] + *(s16 *)D_800A34C0 + *(s32 *)D_800A34C8;
            (*p_prim)->uv0 = *(u16 *)D_800A34D4 + *(s32 *)D_800A3494;
            (*p_prim)->uv1 = *(u16 *)D_800A34D8 + *(s32 *)D_800A3490;
            (*p_prim)->uv2 = *(u16 *)D_800A34DC;
            (*p_prim)->uv3 = *(u16 *)D_800A34E0;
            (*p_prim)->len = 9;
            if (arg0 < 2) {
                (*p_prim)->code = *p_r + (*p_g << 8) + 0x2E000000;
            } else if (arg0 < 4) {
                (*p_prim)->code =
                    *p_r + ((*p_r >> 2) << 8) + (*p_b << 16) + 0x2E000000;
            } else if (arg0 < 6) {
                if (*p_prim - (PolyW *)D_800A3720 >= 0x1C1) {
                    continue;
                }
                if (D_8009BD44[0] & 1) {
                    (*p_prim)++->code = 0x2C242424;
                    break;
                } else if (D_800A34F0[arg0 - 4] != 0) {
                    (*p_prim)++->code = (((rand() * 4 >> 12) + 0x3C) << 8) + 0x2C080038;
                    break;
                } else {
                    (*p_prim)++->code = 0x2C285A78;
                    break;
                }
            } else if (arg0 < 8) {
                if (*p_prim - (PolyW *)D_800A3720 >= 0x1C1) {
                    break;
                }
                (*p_prim)++->code = 0x2EFF8080;
                break;
            }
            if (*p_prim - (PolyW *)D_800A3720 < 0x1C1) {
                (*p_prim)++;
            }
        } while (--*p_n >= 0);
    }
}
