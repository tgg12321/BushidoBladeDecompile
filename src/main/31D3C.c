/* 21 game functions. .text 0x8004153C (ROM 0x31D3C). Start boundary: LEGACY
 * (inside the -G8 run, no evidence either way). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

extern u8 D_80094D40[];

Unk80045878Obj *slot_GetObject(s32 a0) { return D_800A9A10[a0]; }

s32 slot_GetAssetIndex(s32 a0) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        return ptr->unk_08;
    }
    return -1;
}

s32 slot_FindEmpty(void) {
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

void slot_ReleaseObjectAndResources(s32 a0) {
    softbody_FreeObjectById(a0);
    slot_StopAllVoicesCloseVabAndFreeObjectAndAssetBlocks(a0);
    D_800A9A10[a0] = 0;
}

void slot_RequestTimGroupsAndMarkReloadOnChange(s32 a0, s32 a1) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        if (((ptr->unk_00.word >> 16) & 0x1F) != a1) {
            ptr->load_status = -2;
        }
    }
    D_80094B88[a0] = a1;
}

s32 slot_GetLastLoadTimGroupRequest(s32 a0) {
    Unk80045878Obj *ptr = D_800A9A10[a0];
    if (ptr) {
        return (ptr->unk_00.word >> 16) & 0x1F;
    }
    return -1;
}

/* Sets or clears bit 0 of the flag byte in every bone record of
 * D_800A9A10[arg0], then pushes its colour (greyscaled when
 * gpu_RefreshGrayscaleMode() is set) through
 * PatchColorImmediatesAtListedCodeSites. */
void node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor(
    s32 arg0, s32 arg1) {
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

    p = &var_1->transform_nodes[1];
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

    if (gpu_RefreshGrayscaleMode()) {
        v = math_Grayscale3(var_1->unk_18.byte[2], var_1->unk_18.byte[1],
                            var_1->unk_18.byte[0]);
        PatchColorImmediatesAtListedCodeSites((v << 16) | (v << 8) | v);
    } else {
        r = var_1->unk_18.byte[0];
        g = var_1->unk_18.byte[1];
        b = var_1->unk_18.byte[2];
        PatchColorImmediatesAtListedCodeSites(b | ((r << 16) | (g << 8)));
    }
}

extern void gte_SetMatrixRotTransIRVec(void *, void *, void *);

void node_UpdateWorldMatrix(Unk80101DF0Record *a0) {
    RotMatrixFunc func;

    if (a0->matrix_state == 1) {
        return;
    }
    if (a0->matrix_state != 2) {
        func = D_800F66A0[a0->rotation_order];
        func(&a0->xf.rot, &a0->work);
    }
    if (a0->parent_node != 0) {
        if (a0->parent_node->matrix_state == 0) {
            node_UpdateWorldMatrix(a0->parent_node);
        }
        gte_MulMatrix0ClearTrans(
            &a0->parent_node->xf.mat, &a0->work, &a0->xf.mat);
        gte_SetMatrixRotTransIRVec(
            &a0->parent_node->xf.mat, a0->work.t, a0->xf.mat.t);
    } else {
        a0->xf.mat = a0->work;
    }
    a0->matrix_state = 1;
}

void node_SetMatrixFromNegatedRot(Unk80101DF0Record *a0) {
    SVECTOR sp10;
    sp10.vx = -(u16)a0->xf.rot.vx;
    sp10.vy = -(u16)a0->xf.rot.vy;
    sp10.vz = -(u16)a0->xf.rot.vz;
    D_800F66A0[a0->rotation_order](&sp10, &a0->work);
    a0->xf.mat = a0->work;
}

void gpu_LoadRequestedAvailableTimGroupsForSlot(
    s32 a0, s32 a1, s32 a2, s32 a3) {
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
            if (gpu_GetFogClutObject() == 0) {
                gpu_LoadTimGroupSplitOffset(
                    (u32 *)a3, i + 1, 0, 0, -0x140, 0xE8);
            } else {
                gpu_LoadTimGroup((u32 *)a3, i + 1);
            }
            break;
        case 1:
            if (gpu_GetFogClutObject() == a0) {
                gpu_LoadTimGroupSplitOffset(
                    (u32 *)a3, i + 1, 0x80, 0, -0x140, 0xE8);
            } else {
                gpu_LoadTimGroupOffset((u32 *)a3, i + 1, 0x80, 0);
            }
            break;
        }
    }
}

extern s16 *D_80094DF0[6];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 g_gpu_store_buf[][16];

void gpu_BackUpFixedSlotVramRowsAndRecordAssetIndex(Unk80045878Obj *arg0) {
    RECT rect;
    s16 *tbl;
    s32 i;
    s32 xoff;
    s32 yoff;
    s16 *id_ptr;

    if (arg0->arena_id != 1) {
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
    if (gpu_GetFogClutObject() == 1) {
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

void gpu_RestoreTintBackedUpFixedSlotVramRowsAndFogLevels(
    s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *obj;
    s32 r;
    s32 g;
    s32 b;
    s32 k;
    s32 xoff;
    s32 yoff;
    s16 *tbl;
    s32 idx;
    s32 slot = 1;
    RECT rect;

    obj = slot_GetObject(slot);
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
    if (gpu_RefreshGrayscaleMode()) {
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
    if (gpu_GetFogClutObject() == slot) {
        gpu_BuildFogBlockLevels();
    }
}

extern s16 D_800A3238[3];
extern VECTOR D_800A9B28;

void light_SetPointLight(VECTOR *a0, s32 a1) {
    D_800A3238[0] = (s16)((((a1 >> 16) & 0xFF) << 12) / 255);
    D_800A3238[1] = (s16)((((a1 >> 8) & 0xFF) << 12) / 255);
    D_800A3238[2] = (s16)(((a1 & 0xFF) << 12) / 255);
    D_800A9B28 = *a0;
}

void light_ApplyPointLight(s32 a0, s32 a1) {
    Unk800F62E0Rec *var_0;
    s32 var_1;
    Unk800F62E0Rec *tbl;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 angle;
    s32 *ptr;

    /* FAKE: var_0 holds g_light_setups for the loop and the first
     * calc_LightMatrix call; naming g_light_setups directly re-forms the
     * address and the registers rotate (score 45). */
    var_0 = g_light_setups;
    for (var_1 = 0; var_1 < 2; var_1++) {
        tbl = var_0;
        if (var_1 == 0) {
            ptr = (s32 *)a0;
        } else {
            ptr = (s32 *)a1;
            tbl = &g_light_setups[1];
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
        continue;

    skip:
        tbl->light[1].on = 0;
    }

    calc_LightMatrix(var_0);
    calc_LightMatrix(&g_light_setups[1]);
}

extern s16 D_800A3238[3];
/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s16 D_800A3380[2];
static s32 D_800A3384[2];

void light_ResetPointLight(void) {
    D_800A3380[1] = 0;
    D_800A3380[0] = 0;
    D_800A3238[0] = -1;
}

void light_SetPointLightTarget(s32 a0, s32 a1) {
    if (a0 < 2) {
        D_800A3380[a0] = 1;
        D_800A3384[a0] = a1;
    }
}

void light_ApplyPointLightTargets(void) {
    s32 val = D_800A3380[0] * 2 + D_800A3380[1];
    switch (val) {
    case 1:
        light_ApplyPointLight(0, D_800A3384[1]);
        break;
    case 2:
        light_ApplyPointLight(D_800A3384[0], 0);
        break;
    case 3:
        light_ApplyPointLight(D_800A3384[0], D_800A3384[1]);
        break;
    }
}

extern void light_ApplyPointLight(s32, s32);

void light_DisablePointLight(void) { light_ApplyPointLight(0, 0); }

extern void light_SetLightOrBackColorOfSelectedSetups(s32, s32, s32, s32);
extern void gpu_SetBackgroundClearColorAndFarColor(s32);

/* 0x18-byte rows: four colour words, the angle word, a parameter */
extern s32 D_80094E48[][6];

void ApplyLightPresetRowBackgroundClearAndFarColor(s32 a0) {
    s32 *p = D_80094E48[a0];
    s32 var_1;
    s32 var_2;
    light_SetLightOrBackColorOfSelectedSetups(a0, *p++, 0, 0);
    light_SetLightOrBackColorOfSelectedSetups(a0, *p++, 0, 1);
    light_SetLightOrBackColorOfSelectedSetups(a0, *p++, 1, 0);
    light_SetLightOrBackColorOfSelectedSetups(a0, *p++, 1, 1);
    var_1 = *p;
    g_light_setups[4].light[0].yaw = var_1 & 0xFFF;
    g_light_setups[1].light[0].yaw = var_1 & 0xFFF;
    g_light_setups[0].light[0].yaw = var_1 & 0xFFF;
    var_2 = *p >> 16;
    g_light_setups[4].light[0].pitch = var_2 & 0xFFF;
    g_light_setups[1].light[0].pitch = var_2 & 0xFFF;
    g_light_setups[0].light[0].pitch = var_2 & 0xFFF;
    gpu_SetBackgroundClearColorAndFarColor(p[1]);
}

void light_SetLightOrBackColorOfSelectedSetups(
    s32 a0, s32 arg1, s32 a2, s32 a3) {
    s32 r = (arg1 >> 16) & 0xFF;
    s32 g = (arg1 >> 8) & 0xFF;
    s32 b = arg1 & 0xFF;
    s16 r2;
    s16 g2;
    s16 b2;

    if (gpu_RefreshGrayscaleMode()) {
        r = g = b = math_Grayscale3(r, g, b);
    }
    if (a3 == 0) {
        r2 = (r << 12) / 255;
        g2 = (g << 12) / 255;
        b2 = (b << 12) / 255;
        if (a2 == 0) {
            g_light_setups[0].cmat.m[0][0] = r2;
            g_light_setups[0].cmat.m[1][0] = g2;
            g_light_setups[0].cmat.m[2][0] = b2;
            calc_LightMatrix(&g_light_setups[0]);
            g_light_setups[1].cmat.m[0][0] = r2;
            g_light_setups[1].cmat.m[1][0] = g2;
            g_light_setups[1].cmat.m[2][0] = b2;
            calc_LightMatrix(&g_light_setups[1]);
        } else {
            g_light_setups[4].cmat.m[0][0] = r2;
            g_light_setups[4].cmat.m[1][0] = g2;
            g_light_setups[4].cmat.m[2][0] = b2;
            calc_LightMatrix(&g_light_setups[4]);
        }
    } else if (a2 == 0) {
        g_light_setups[0].back[0] = r;
        g_light_setups[0].back[1] = g;
        g_light_setups[0].back[2] = b;
        g_light_setups[1].back[0] = r;
        g_light_setups[1].back[1] = g;
        g_light_setups[1].back[2] = b;
    } else {
        g_light_setups[4].back[0] = r;
        g_light_setups[4].back[1] = g;
        g_light_setups[4].back[2] = b;
    }
}

void gpu_SetBackgroundClearColorAndFarColor(s32 a0) {
    s32 r = (a0 >> 16) & 0xFF;
    s32 g = (a0 >> 8) & 0xFF;
    s32 b = a0 & 0xFF;
    if (gpu_RefreshGrayscaleMode()) {
        b = math_Grayscale3(r, g, b);
        g = b;
        r = b;
    }
    gpu_SetDrawEnvBg(1, r, g, b);
    SetFarColor(r, g, b);
}

/* .data 0x80094D40..0x800951D8, this file's whole .data (31548.c's D_80094CFC
 * ends at 0x80094D40; 32D04.c's D_800951D8 follows), in address order:
 * gpu_LoadRequestedAvailableTimGroupsForSlot's per-index 5-bit mask table, the
 * six VRAM CLUT-row lists
 * ({x, y} pairs ending in {-1, 0}) that D_80094DF0 points at, the id ->
 * list index map D_80094E08 (0xFF: none), and
 * ApplyLightPresetRowBackgroundClearAndFarColor's 38 light rows. Defined after
 * every function, so no body sees a definition where it saw an extern. */
u8 D_80094D40[64] = {
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x0,  0x0,  0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0,  0x0,  0x1F, 0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x1F};
s16 D_80094D80[16] = {0x2B0, 0xFA, 0x290, 0xF8, 0x2A0, 0xF8, 0x280, 0xF8,
                      0x2B0, 0xF8, 0x290, 0xF9, 0x2B0, 0xF9, -1,    0x0};
s16 D_80094DA0[8] = {0x280, 0xF8, 0x2B0, 0xF9, 0x280, 0xF9, -1, 0x0};
s16 D_80094DB0[12] = {0x280, 0xF9, 0x2B0, 0xFA, 0x2B0, 0xF8,
                      0x2B0, 0xF9, 0x280, 0xF8, -1,    0x0};
s16 D_80094DC8[8] = {0x280, 0xFC, 0x290, 0xFB, 0x290, 0xFA, -1, 0x0};
s16 D_80094DD8[6] = {0x2A0, 0xF8, 0x2B0, 0xF9, -1, 0x0};
s16 D_80094DE4[6] = {0x280, 0xFB, 0x2B0, 0xFB, -1, 0x0};
s16 *D_80094DF0[6] = {D_80094D80, D_80094DA0, D_80094DB0,
                      D_80094DC8, D_80094DD8, D_80094DE4};
u8 D_80094E08[64] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x0,  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1,  0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x2,  0x3,  0x4,  0x5,  0xFF, 0xFF};
s32 D_80094E48[38][6] = {
    {0x0098A7B5, 0x005F5D71, 0x009A84A3, 0x00302846, 0x0E9F0CA2, 0x00000000},
    {0x00DEDED6, 0x001E1E14, 0x009EA9B7, 0x002B3049, 0x0D0A0C00, 0x00000008},
    {0x007FA9DC, 0x0036394E, 0x00B7A2E3, 0x001F2E46, 0x0E300000, 0x00000000},
    {0x00DEDECA, 0x00262F32, 0x00CF96AC, 0x001F2730, 0x0D0A0C00, 0x00000810},
    {0x00EACD98, 0x0018253B, 0x009CAFCC, 0x00243E64, 0x0E2E01A0, 0x00000018},
    {0x00EBE8B3, 0x0026274A, 0x008FB4D9, 0x001B2946, 0x0D3A020A, 0x00000010},
    {0x00B1D8F2, 0x003B3C25, 0x0070B2DE, 0x003B4B5C, 0x0D800B50, 0x00000000},
    {0x008FC9EE, 0x00312F26, 0x0095BCD5, 0x002F261D, 0x0D0001A0, 0x00000008},
    {0x00D2DDEC, 0x00353446, 0x008995D0, 0x001A2E46, 0x0D800604, 0x00000000},
    {0x00E2E1B5, 0x00272730, 0x00000151, 0x00797894, 0x0E490382, 0x00101000},
    {0x00E3E3C4, 0x00173146, 0x004F729B, 0x00576CAD, 0x0D800EC4, 0x00000000},
    {0x00E1CCA5, 0x00282638, 0x00B5B4D4, 0x00283649, 0x0E6602C9, 0x00000810},
    {0x0087C8E7, 0x003C2A1B, 0x004B9EF1, 0x00566F90, 0x0E0E01FC, 0x00080800},
    {0x008BB53A, 0x0037426B, 0x0076A7CF, 0x00404B65, 0x0CC002A2, 0x00000010},
    {0x00C8D99C, 0x00233149, 0x0036425C, 0x00232C2B, 0x0D3A020A, 0x00000018},
    {0x00E5D5C4, 0x001D3145, 0x0067AAFF, 0x005A6E7B, 0x0D800B28, 0x00000000},
    {0x00C9BDAB, 0x00534D44, 0x0072C4FF, 0x00324A57, 0x0D800708, 0x00000000},
    {0x00DBD5B4, 0x002D344C, 0x004F9CE2, 0x001F2837, 0x0D800A49, 0x00000000},
    {0x00D8D6CA, 0x00303046, 0x006BA8DB, 0x00272B3B, 0x0D8005C0, 0x00080810},
    {0x00A5C8F6, 0x002D404C, 0x0060B5DE, 0x002D4458, 0x0D800A30, 0x00000810},
    {0x00C5A4BB, 0x003B2448, 0x00C275E4, 0x00453A57, 0x0EF80BA8, 0x00080808},
    {0x009FD2F0, 0x00443332, 0x005DA6DC, 0x003F2A41, 0x0D80061F, 0x00080810},
    {0x00DAC29C, 0x00392B24, 0x0095C7C5, 0x0023333C, 0x0D800988, 0x00000008},
    {0x00D7D5BA, 0x002B3360, 0x008EB4D2, 0x0020429B, 0x0E4F0000, 0x00000000},
    {0x00B9DAE6, 0x00312847, 0x00B9A088, 0x006A3D86, 0x0CC0073C, 0x00000000},
    {0x0098C1E7, 0x00212530, 0x009BBDD5, 0x00273B4F, 0x0D800A7D, 0x00081010},
    {0x0086DAFA, 0x0031261B, 0x0070D3F2, 0x002D3E4D, 0x0D800708, 0x00000008},
    {0x0085CFEF, 0x003E3E48, 0x007CBFD7, 0x0022384C, 0x0CC00788, 0x00000008},
    {0x00E9D6C8, 0x00323E40, 0x005DB2DA, 0x003B3131, 0x0E31050E, 0x00000008},
    {0x00A3C8E7, 0x00362F44, 0x0071B6D6, 0x00355169, 0x0EB104FB, 0x00000000},
    {0x00CBE7EF, 0x00352F44, 0x0071B6D6, 0x00355169, 0x0CC00510, 0x00000008},
    {0x00FFE4D1, 0x0018324E, 0x00785FD7, 0x00372F70, 0x0E180275, 0x00000010},
    {0x00D7D7D7, 0x001E1E1E, 0x00B6B7AC, 0x003F3E46, 0x0D800800, 0x00000008},
    {0x00DCDCDC, 0x0026221B, 0x00DCB4B1, 0x00433D39, 0x0E500128, 0x00000008},
    {0x00D2D1D1, 0x00202020, 0x00C5C6C5, 0x00474746, 0x0D800E8E, 0x00000810},
    {0x00DBE4E2, 0x00585656, 0x009DB1B1, 0x003C3C3B, 0x0D800800, 0x00101020},
    {0x00D9D8D9, 0x00242323, 0x00BDBDBD, 0x00444343, 0x0D800800, 0x00000010},
    {0x00E6E4A7, 0x00161E28, 0x0092AFCC, 0x001F253F, 0x0E000650, 0x00000000}};

/* This file's initialized small data (.sdata), in address order (Q65). */
s16 D_800A3238[3] = {0, 0, 0};
