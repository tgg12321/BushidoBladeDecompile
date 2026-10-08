/* 21 game functions. .text 0x8004153C (ROM 0x31D3C). Start boundary: LEGACY
 * (inside the -G8 run, no evidence either way). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

extern u8 D_80094D40[];

Unk80045878Obj *func_8004153C(s32 a0) { return D_800A9A10[a0]; }

s32 func_80041554(s32 a0) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        return ptr->unk_08;
    }
    return -1;
}

s32 func_80041584(void) {
    s32 i;
    s32 ret = -1;
    for (i = 0; i < 3; i++) {
        if (D_800A9A10[i] == 0) {
            ret = i;
            break;
        }
    }
    return ret;
}

void func_800415C4(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    D_800A9A10[a0] = 0;
}

void func_80041604(s32 a0, s32 a1) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        /* FAKE: the halfword read into an s32 local (lh); in the masked
         * expression it loads lhu (score 1) */
        s32 val = ptr->unk_00.half[1];
        if ((val & 0x1F) != a1) {
            ptr->unk_06 = -2;
        }
    }
    D_80094B88[a0] = a1;
}

s32 func_80041650(s32 a0) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        /* FAKE: the halfword read into an s32 local (lh); in the masked
         * expression it loads lhu (score 1) */
        s32 val = ptr->unk_00.half[1];
        return val & 0x1F;
    }
    return -1;
}

/* Sets or clears bit 0 of the flag byte in every bone record of player arg0,
 * then pushes the player's colour (greyscaled when func_800486FC() is set)
 * through func_80041398. */
void func_80041688(s32 arg0, s32 arg1) {
    /* !FAKE: frame layout: the 0x38 frame keeps 0x20 leading bytes
       untouched (phantom-frame pad); without it: score 6 */
    volatile u32 pre_pad[8];
    Unk80045878Obj *player;
    s32 i;
    Unk80045878Node *p;
    u8 *q;
    s32 b, r, g, v;

    player = D_800A9A10[arg0];
    if (player == NULL)
        return;

    p = &player->unk_2C[1];
    if (arg1) {
        p->node.unk1 |= 1;
    } else {
        p->node.unk1 &= ~1;
    }

    i = 1;
loop1:
    p++;
    /* FAKE: guard staged through the existing local b (dead before b's real
       colour use); the second set of b changes the scheduler's load order
       to the target's [b,r,g]; without it: score 2
       (staged-value-reused-variable) */
    b = p->node.unk2 >= 0;
    if (b) {
        if (arg1)
            p->node.unk1 |= 1;
        else
            p->node.unk1 &= ~1;
    }
    i++;
    if (i < 18)
        goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0)
        goto after2;
    if (arg1)
        *q |= 1;
    else
        *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        v = math_Grayscale3(player->unk_18.byte[2], player->unk_18.byte[1],
                            player->unk_18.byte[0]);
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        /* FAKE: r / g / b read into locals ahead of the call; read in the
         * argument expression the frame grows 0x38 -> 0x40 (score 8). */
        r = player->unk_18.byte[0];
        g = player->unk_18.byte[1];
        b = player->unk_18.byte[2];
        func_80041398(b | ((r << 16) | (g << 8)));
    }
}

extern void gte_SetMatrixRotTransIRVec(void *, void *, void *);

void func_800417D0(Unk80101DF0Record *a0) {
    RotMatrixFunc func;

    if (a0->unk6 == 1) {
        return;
    }
    if (a0->unk6 != 2) {
        func = D_800F66A0[a0->unk8];
        func(&a0->xf.rot, &a0->work);
    }
    if (a0->unkC != 0) {
        if (a0->unkC->unk6 == 0) {
            func_800417D0(a0->unkC);
        }
        gte_MulMatrix0ClearTrans(&a0->unkC->xf.mat, &a0->work, &a0->xf.mat);
        gte_SetMatrixRotTransIRVec(&a0->unkC->xf.mat, a0->work.t, a0->xf.mat.t);
    } else {
        a0->xf.mat = a0->work;
    }
    a0->unk6 = 1;
}

void func_800418D0(Unk80101DF0Record *a0) {
    SVECTOR sp10;
    sp10.vx = -(u16)a0->xf.rot.vx;
    sp10.vy = -(u16)a0->xf.rot.vy;
    sp10.vz = -(u16)a0->xf.rot.vz;
    D_800F66A0[a0->unk8](&sp10, &a0->work);
    a0->xf.mat = a0->work;
}

void func_80041988(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 mask_table;
    s32 bit;
    s32 i;
    s32 one;

    if ((u32)a0 >= 2) {
        return;
    }
    mask_table = D_80094D40[a1];
    bit = 0x10;
    i = 0;
    do {
        if (!(mask_table & bit) || !(a2 & bit)) {
            goto shift;
        }
        /* FAKE: constant holder (SOTN `s16 three = 3;`): a literal 1 is
         * hoisted out of the loop and rematerialized in $v1; the named local
         * stays in-loop in $v0 as in the target (named-local-fake-exception) */
        one = 1;
        if (a0 == 0) {
            goto case0;
        }
        if (a0 == one) {
            goto case1;
        }
        goto shift;
    case0:
        if (func_8003E2A0() == 0) {
            func_800480C0(a3, i + 1, 0, 0, -0x140, 0xE8);
        } else {
            func_80047EE8(a3, i + 1);
        }
        goto shift;
    case1:
        if (func_8003E2A0() == a0) {
            func_800480C0(a3, i + 1, 0x80, 0, -0x140, 0xE8);
        } else {
            func_80047FBC(a3, i + 1, 0x80, 0);
        }
    shift:
        bit >>= 1;
        i++;
    } while (i < 5);
}

extern s16 *D_80094DF0[6];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 g_gpu_store_buf[][16];

void func_80041AC8(Unk80045878Obj *arg0) {
    RECT rect;
    s16 *var_s0;
    u16(*var_s1)[16];
    s32 var_s2;
    s32 var_s3;
    u16 v1_val;
    s16 *id_ptr;
    if (arg0->unk_04 != 1) {
        return;
    }
    if (D_80094E08[arg0->unk_08] == 0xFF) {
        return;
    }
    /* FAKE: id_ptr spelling: the store to D_800A9A20 stays ahead of the
     * reload of arg0->unk_08 only when the reload is not plain struct
     * indexing; without it: score 2
     * (proven-spelling-class-reconstruction) */
    id_ptr = &arg0->unk_08;
    D_800A9A20 = arg0->unk_08;
    var_s0 = D_80094DF0[D_80094E08[*id_ptr]];
    if (func_8003E2A0() != 1) {
        goto else_lbl;
    }
    var_s3 = -0x140;
    var_s2 = 0xF0;
    goto after_if;
else_lbl:
    var_s3 = 0x80;

    var_s2 = 0;
after_if:
    /* FAKE: v1_val is the loop-rotated read of *var_s0 (here and at the loop's
     * end); read at the rect.x store the frame shrinks 0x50 -> 0x40 and the
     * head's lh / lhu pair changes (score 26). */
    v1_val = (u16)(*var_s0);

    if ((*var_s0) >= 0) {
        /* FAKE: constant holders for the 16 x 1 rect size; as literals the
         * g_gpu_store_buf address (lui/addiu s1) is scheduled ahead of li s5,16
         * / li s4,1 (score 4) */
        s32 w = 0x10;
        s32 h = 1;
        var_s1 = g_gpu_store_buf;
        do {
            /* FAKE: v0_val reads var_s0[1] ahead of the rect.w / rect.h stores;
             * read at the rect.y store, sh s5 / sh s4 move above the load
             * (score 4) */
            u16 v0_val;
            rect.x = v1_val + var_s3;
            v0_val = (u16)var_s0[1];
            rect.w = w;
            rect.h = h;
            rect.y = v0_val + var_s2;
            StoreImage(&rect, (u32 *)var_s1);
            var_s0 += 2;
            var_s1++;
            v1_val = (u16)(*var_s0);
        } while ((*var_s0) >= 0);
    }
    DrawSync(0);
}

void func_80041BF4(s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *fp_ptr;
    s32 r;
    s32 g;
    s32 b;
    s32 outer;
    s32 xoff;
    s32 yoff;
    s16 *tbl;
    s32 idx;
    s32 x;
    /* FAKE: constant holder for the trailing `== 1` test: a literal is
       rematerialized in $v1, the live local gets $t0 as in the target;
       without it: score 12 (named-local-fake-exception) */
    int one;
    /* FAKE: frame layout: rect[0] is the live LoadImage RECT, rect[1] an
       unwritten tail for the 88-byte frame (one RECT gives 80; any 10-16
       byte size matches); without it: score 22 (dead-vars-local-array
       oversized-locals carve-out) */
    RECT rect[2];
    fp_ptr = func_8004153C(1);
    if (fp_ptr == 0) {
        return;
    }
    if ((fp_ptr->unk_08) != D_800A9A20) {
        return;
    }
    if (D_80094E08[fp_ptr->unk_08] == 0xFF) {
        return;
    }
    r = (a0 << 12) / 255;
    one = 1;
    g = (a1 << 12) / 255;
    b = (a2 << 12) / 255;
    if (func_800486FC()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    outer = 0;
oloop: {
    if (outer == 0) {
        xoff = -0x140;
        yoff = 0xF0;
    } else {
        /* FAKE: do-while(0) raises xoff/yoff's loop-depth-weighted reference
           counts so they land in $s5/$s4; without it: score 18
           (do-while-zero-exception) */
        do {
            xoff = 0x80;
            yoff = 0;
        } while (0);
    }
    tbl = D_80094DF0[D_80094E08[fp_ptr->unk_08]];
    idx = 0;
    goto test;
again: {
    rect[0].x = x + xoff;
    rect[0].y = tbl[1] + yoff;
    rect[0].w = 0x10;
    rect[0].h = 1;
    LoadImage(&rect[0], (u32 *)g_gpu_store_buf[idx++]);
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0].x, rect[0].y, 0x10, r, g, b);
}
test:
    /* FAKE: x is the row's x zero-extended (the lhu beside the test's lh); as
     * tbl[0] the pair is one lh and the frame drops 88 -> 80: score 25 */
    x = (u16)tbl[0];
    if ((s16)x >= 0)
        goto again;
    outer++;
}
    if (outer < 2)
        goto oloop;
    if (func_8003E2A0() == one) {
        func_8003E120();
    }
}

extern s16 D_800A3238[3];
extern VECTOR D_800A9B28;

void func_80041E10(VECTOR *a0, s32 a1) {
    D_800A3238[0] = (s16)((((a1 >> 16) & 0xFF) << 12) / 255);
    D_800A3238[1] = (s16)((((a1 >> 8) & 0xFF) << 12) / 255);
    D_800A3238[2] = (s16)(((a1 & 0xFF) << 12) / 255);
    D_800A9B28 = *a0;
}

void func_80041EB0(s32 a0, s32 a1) {
    Unk800F62E0Rec *fp_ptr;
    s32 outer;
    Unk800F62E0Rec *tbl;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 angle;
    s32 *ptr;

    /* FAKE: fp_ptr holds D_800F62E0 for the loop and the first calc_LightMatrix
     * call; naming D_800F62E0 directly re-forms the address and the registers
     * rotate (score 45). */
    fp_ptr = D_800F62E0;
    outer = 0;

    do {
        tbl = fp_ptr;
        if (outer == 0) {
            ptr = (s32 *)a0;
        } else {
            ptr = (s32 *)a1;
            tbl = &D_800F62E0[1];
        }

        if (ptr == 0) {
            goto skip;
        }
        if (D_800A3238[0] < 0) {
            goto skip;
        }

        dx = ptr[0] - D_800A9B28.vx;
        dy = ptr[1] - D_800A9B28.vy;
        dz = ptr[2] - D_800A9B28.vz;

        if (dx < 0)
            goto neg_dx;
        if (dx < 0x7000)
            goto check_dy;
        goto calc;
    neg_dx:
        if (-dx >= 0x7000)
            goto calc;
    check_dy:
        if (dy < 0)
            goto neg_dy;
        if (dy < 0x7000)
            goto check_dz;
        goto calc;
    neg_dy:
        if (-dy >= 0x7000)
            goto calc;
    check_dz:
        if (dz < 0)
            goto neg_dz;
        if (dz < 0x7000)
            goto dist_check;
        goto calc;
    neg_dz:
        if (-dz >= 0x7000)
            goto calc;

    dist_check:
        if (gte_SumSquares3(dx, dy, dz) > 0x17D7840) {
            goto skip;
        }

    calc:
        angle = ratan2(dx, dz);
        tbl->light[1].pitch =
            (s16)-ratan2(dy, (rcos(angle) * dz + rsin(angle) * dx) >> 12);
        tbl->light[1].yaw = (s16)angle;
        tbl->light[1].on = 1;
        tbl->cmat.m[0][1] = D_800A3238[0];
        tbl->cmat.m[1][1] = D_800A3238[1];
        tbl->cmat.m[2][1] = D_800A3238[2];
        goto end_loop;

    skip:
        tbl->light[1].on = 0;

    end_loop:
        outer++;
    } while (outer < 2);

    calc_LightMatrix(fp_ptr);
    calc_LightMatrix(&D_800F62E0[1]);
}

extern s16 D_800A3238[3];
/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s16 D_800A3380[2];
static s32 D_800A3384[2];

void func_800420D0(void) {
    D_800A3380[1] = 0;
    D_800A3380[0] = 0;
    D_800A3238[0] = -1;
}

void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        D_800A3380[a0] = 1;
        D_800A3384[a0] = a1;
    }
}

void func_8004211C(void) {
    s32 val = D_800A3380[0] * 2 + D_800A3380[1];
    switch (val) {
    case 1:
        func_80041EB0(0, D_800A3384[1]);
        break;
    case 2:
        func_80041EB0(D_800A3384[0], 0);
        break;
    case 3:
        func_80041EB0(D_800A3384[0], D_800A3384[1]);
        break;
    }
}

extern void func_80041EB0(s32, s32);

void func_800421A4(void) { func_80041EB0(0, 0); }

extern void func_800422BC(s32, s32, s32, s32);
extern void func_80042478(s32);

/* 0x18-byte rows: four colour words, the angle word, a parameter */
extern s32 StageLight[][6];

void func_800421C8(s32 a0) {
    s32 *p = StageLight[a0];
    s32 yaw;
    s32 pitch;
    func_800422BC(a0, *p++, 0, 0);
    func_800422BC(a0, *p++, 0, 1);
    func_800422BC(a0, *p++, 1, 0);
    func_800422BC(a0, *p++, 1, 1);
    yaw = *p;
    D_800F62E0[4].light[0].yaw = yaw & 0xFFF;
    D_800F62E0[1].light[0].yaw = yaw & 0xFFF;
    D_800F62E0[0].light[0].yaw = yaw & 0xFFF;
    pitch = *p >> 16;
    D_800F62E0[4].light[0].pitch = pitch & 0xFFF;
    D_800F62E0[1].light[0].pitch = pitch & 0xFFF;
    D_800F62E0[0].light[0].pitch = pitch & 0xFFF;
    func_80042478(p[1]);
}

void func_800422BC(s32 a0, s32 packed, s32 a2, s32 a3) {
    s32 r = (packed >> 16) & 0xFF;
    s32 g = (packed >> 8) & 0xFF;
    s32 b = packed & 0xFF;
    s16 r2;
    s16 g2;
    s16 b2;
    if (func_800486FC()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    if (a3 != 0) {
        goto raw;
    }
    r2 = (r << 12) / 255;
    g2 = (g << 12) / 255;
    b2 = (b << 12) / 255;
    if (a2 != 0) {
        goto alt_scale;
    }
    D_800F62E0[0].cmat.m[0][0] = r2;
    D_800F62E0[0].cmat.m[1][0] = g2;
    D_800F62E0[0].cmat.m[2][0] = b2;
    calc_LightMatrix(&D_800F62E0[0]);
    D_800F62E0[1].cmat.m[0][0] = r2;
    D_800F62E0[1].cmat.m[1][0] = g2;
    D_800F62E0[1].cmat.m[2][0] = b2;
    calc_LightMatrix(&D_800F62E0[1]);
    goto out;
alt_scale:
    D_800F62E0[4].cmat.m[0][0] = r2;
    D_800F62E0[4].cmat.m[1][0] = g2;
    D_800F62E0[4].cmat.m[2][0] = b2;
    calc_LightMatrix(&D_800F62E0[4]);
    goto out;
raw:
    if (a2 != 0) {
        goto alt_raw;
    }
    D_800F62E0[0].back[0] = r;
    D_800F62E0[0].back[1] = g;
    D_800F62E0[0].back[2] = b;
    D_800F62E0[1].back[0] = r;
    D_800F62E0[1].back[1] = g;
    D_800F62E0[1].back[2] = b;
    goto out;
alt_raw:
    D_800F62E0[4].back[0] = r;
    D_800F62E0[4].back[1] = g;
    D_800F62E0[4].back[2] = b;
out:;
}

void func_80042478(s32 a0) {
    s32 r = (a0 >> 16) & 0xFF;
    s32 g = (a0 >> 8) & 0xFF;
    s32 b = a0 & 0xFF;
    if (func_800486FC()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    gpu_SetDrawEnvBg(1, r, g, b);
    SetFarColor(r, g, b);
}

/* This file's initialized small data (.sdata), in address order (Q65). */
s16 D_800A3238[3] = {0, 0, 0};
