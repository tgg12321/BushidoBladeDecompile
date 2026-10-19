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
        if (((ptr->unk_00.word >> 16) & 0x1F) != a1) {
            ptr->unk_06 = -2;
        }
    }
    D_80094B88[a0] = a1;
}

s32 func_80041650(s32 a0) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        return (ptr->unk_00.word >> 16) & 0x1F;
    }
    return -1;
}

/* Sets or clears bit 0 of the flag byte in every bone record of
 * D_800A9A10[arg0], then pushes its colour (greyscaled when func_800486FC() is
 * set) through func_80041398. */
void func_80041688(s32 arg0, s32 arg1) {
    /* !FAKE: frame layout: the 0x38 frame keeps 0x20 leading bytes
       untouched (phantom-frame pad); without it: score 6 */
    volatile u32 pre_pad[8];
    Unk80045878Obj *var_1;
    s32 i;
    Unk80045878Node *p;
    u8 *q;
    s32 b, r, g, v;

    var_1 = D_800A9A10[arg0];
    if (var_1 == NULL)
        return;

    p = &var_1->unk_2C[1];
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

    q = (u8 *)var_1 + 0x10D5;
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
        v = math_Grayscale3(var_1->unk_18.byte[2], var_1->unk_18.byte[1],
                            var_1->unk_18.byte[0]);
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        r = var_1->unk_18.byte[0];
        g = var_1->unk_18.byte[1];
        b = var_1->unk_18.byte[2];
        func_80041398(b | ((r << 16) | (g << 8)));
    }
}

extern void gte_SetMatrixRotTransIRVec(void *, void *, void *);

void node_UpdateWorldMatrix(Unk80101DF0Record *a0) {
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
            node_UpdateWorldMatrix(a0->unkC);
        }
        gte_MulMatrix0ClearTrans(&a0->unkC->xf.mat, &a0->work, &a0->xf.mat);
        gte_SetMatrixRotTransIRVec(&a0->unkC->xf.mat, a0->work.t, a0->xf.mat.t);
    } else {
        a0->xf.mat = a0->work;
    }
    a0->unk6 = 1;
}

void node_SetMatrixFromNegatedRot(Unk80101DF0Record *a0) {
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

    if ((u32)a0 >= 2) {
        return;
    }
    mask_table = D_80094D40[a1];
    for (bit = 0x10, i = 0; i < 5; bit >>= 1, i++) {
        if (!(mask_table & bit) || !(a2 & bit)) {
            continue;
        }
        switch (a0) {
        case 0:
            if (func_8003E2A0() == 0) {
                gpu_LoadTimGroupSplitOffset(a3, i + 1, 0, 0, -0x140, 0xE8);
            } else {
                gpu_LoadTimGroup(a3, i + 1);
            }
            break;
        case 1:
            if (func_8003E2A0() == a0) {
                gpu_LoadTimGroupSplitOffset(a3, i + 1, 0x80, 0, -0x140, 0xE8);
            } else {
                gpu_LoadTimGroupOffset(a3, i + 1, 0x80, 0);
            }
            break;
        }
    }
}

extern s16 *D_80094DF0[6];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 g_gpu_store_buf[][16];

void func_80041AC8(Unk80045878Obj *arg0) {
    RECT rect;
    s16 *tbl;
    s32 i;
    s32 xoff;
    s32 yoff;
    s16 *id_ptr;

    if (arg0->unk_04 != 1) {
        return;
    }
    if (D_80094E08[arg0->unk_08] == 0xFF) {
        return;
    }
    /* FAKE: id_ptr spelling: the reload of arg0->unk_08 for the table
     * index stays after the D_800A9A20 store only when it is not plain
     * struct indexing; D_80094E08[arg0->unk_08]: score 2
     * (proven-spelling-class-reconstruction) */
    id_ptr = &arg0->unk_08;
    D_800A9A20 = arg0->unk_08;
    tbl = D_80094DF0[D_80094E08[*id_ptr]];
    if (func_8003E2A0() == 1) {
        xoff = -0x140;
        yoff = 0xF0;
    } else {
        xoff = 0x80;
        yoff = 0;
    }
    for (i = 0; tbl[0] >= 0; tbl += 2, i++) {
        rect.x = tbl[0] + xoff;
        rect.y = tbl[1] + yoff;
        rect.w = 0x10;
        rect.h = 1;
        StoreImage(&rect, (u32 *)g_gpu_store_buf[i]);
    }
    DrawSync(0);
}

void func_80041BF4(s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *obj;
    s32 r;
    s32 g;
    s32 b;
    s32 k;
    s32 xoff;
    s32 yoff;
    s16 *tbl;
    s32 idx;
    /* FAKE: constant holder for the trailing `== 1` test; with the literal
     * the 1 lands in $v1, not the target's $t0: score 2
     * (named-local-fake-exception) */
    s32 one = 1;
    RECT rect;

    obj = func_8004153C(1);
    if (obj == 0) {
        return;
    }
    if (obj->unk_08 != D_800A9A20) {
        return;
    }
    if (D_80094E08[obj->unk_08] == 0xFF) {
        return;
    }
    r = (a0 << 12) / 255;
    g = (a1 << 12) / 255;
    b = (a2 << 12) / 255;
    if (func_800486FC()) {
        r = g = b = math_Grayscale3(r, g, b);
    }
    for (k = 0; k < 2; k++) {
        if (k == 0) {
            xoff = -0x140;
            yoff = 0xF0;
        } else {
            xoff = 0x80;
            yoff = 0;
        }
        tbl = D_80094DF0[D_80094E08[obj->unk_08]];
        for (idx = 0; tbl[0] >= 0; tbl += 2) {
            rect.x = tbl[0] + xoff;
            rect.y = tbl[1] + yoff;
            rect.w = 0x10;
            rect.h = 1;
            LoadImage(&rect, (u32 *)g_gpu_store_buf[idx++]);
            DrawSync(0);
            gpu_TintVramRowInPlace(rect.x, rect.y, 0x10, r, g, b);
        }
    }
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
    Unk800F62E0Rec *var_0;
    s32 var_1;
    Unk800F62E0Rec *tbl;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 angle;
    s32 *ptr;

    /* FAKE: var_0 holds D_800F62E0 for the loop and the first calc_LightMatrix
     * call; naming D_800F62E0 directly re-forms the address and the registers
     * rotate (score 45). */
    var_0 = D_800F62E0;
    var_1 = 0;

    do {
        tbl = var_0;
        if (var_1 == 0) {
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
        var_1++;
    } while (var_1 < 2);

    calc_LightMatrix(var_0);
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
extern s32 D_80094E48[][6];

void func_800421C8(s32 a0) {
    s32 *p = D_80094E48[a0];
    s32 var_1;
    s32 var_2;
    func_800422BC(a0, *p++, 0, 0);
    func_800422BC(a0, *p++, 0, 1);
    func_800422BC(a0, *p++, 1, 0);
    func_800422BC(a0, *p++, 1, 1);
    var_1 = *p;
    D_800F62E0[4].light[0].yaw = var_1 & 0xFFF;
    D_800F62E0[1].light[0].yaw = var_1 & 0xFFF;
    D_800F62E0[0].light[0].yaw = var_1 & 0xFFF;
    var_2 = *p >> 16;
    D_800F62E0[4].light[0].pitch = var_2 & 0xFFF;
    D_800F62E0[1].light[0].pitch = var_2 & 0xFFF;
    D_800F62E0[0].light[0].pitch = var_2 & 0xFFF;
    func_80042478(p[1]);
}

void func_800422BC(s32 a0, s32 arg1, s32 a2, s32 a3) {
    s32 r = (arg1 >> 16) & 0xFF;
    s32 g = (arg1 >> 8) & 0xFF;
    s32 b = arg1 & 0xFF;
    s16 r2;
    s16 g2;
    s16 b2;

    if (func_800486FC()) {
        r = g = b = math_Grayscale3(r, g, b);
    }
    if (a3 == 0) {
        r2 = (r << 12) / 255;
        g2 = (g << 12) / 255;
        b2 = (b << 12) / 255;
        if (a2 == 0) {
            D_800F62E0[0].cmat.m[0][0] = r2;
            D_800F62E0[0].cmat.m[1][0] = g2;
            D_800F62E0[0].cmat.m[2][0] = b2;
            calc_LightMatrix(&D_800F62E0[0]);
            D_800F62E0[1].cmat.m[0][0] = r2;
            D_800F62E0[1].cmat.m[1][0] = g2;
            D_800F62E0[1].cmat.m[2][0] = b2;
            calc_LightMatrix(&D_800F62E0[1]);
        } else {
            D_800F62E0[4].cmat.m[0][0] = r2;
            D_800F62E0[4].cmat.m[1][0] = g2;
            D_800F62E0[4].cmat.m[2][0] = b2;
            calc_LightMatrix(&D_800F62E0[4]);
        }
    } else if (a2 == 0) {
        D_800F62E0[0].back[0] = r;
        D_800F62E0[0].back[1] = g;
        D_800F62E0[0].back[2] = b;
        D_800F62E0[1].back[0] = r;
        D_800F62E0[1].back[1] = g;
        D_800F62E0[1].back[2] = b;
    } else {
        D_800F62E0[4].back[0] = r;
        D_800F62E0[4].back[1] = g;
        D_800F62E0[4].back[2] = b;
    }
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
