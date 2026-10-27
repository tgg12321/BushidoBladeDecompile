/* 103 game functions, among them rcnt_StartCnt1, rcnt_GetCnt1,
 * math_GrayscaleRgb555 and math_Grayscale3. .text 0x800460E4 (ROM 0x368E4).
 * Start boundary: GP (a per-file gp merge, Q67); compiled -G8 (Q89). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void wave_SetCellBuffer(s32);
extern s32 wave_GetCellBufferSize(void);
extern void gpu_LoadTimGroupWithConditionalClutRowAdvance(u32 *, s32);
extern void resource_RebaseLoadedBlockSections(s32, s32);
void gpu_LoadTim();
void gpu_LoadTimClut();

#define ALIGN4(x) (((u32)(x) >> 2) << 2)
#define PTR_OFF(base, off) ((s32)((u8 *)(base) + (off)))

/* Q65: this file's statics (.sbss, allocated per file by PSYLINK), in
 * address order. */
static s32 D_800A33B0;
static s32 g_fight_position_records;
static s32 D_800A33B8; /* not named by any code or data: size from the gap */
static s32 D_800A33BC;
static s32 D_800A33C0;
static s32 D_800A33C4; /* not named by any code or data: size from the gap */
static s16 D_800A33C8[2];
static s32 D_800A33CC; /* not named by any code or data: size from the gap */
static s16 *g_wave_cell_buffer;
static s32 D_800A33D4;
static s32 D_800A33D8;
static s32 D_800A33DC; /* not named by any code or data: size from the gap */
static s32 D_800A33E0;
static s32 D_800A33E4;
static s16 D_800A33E8[2];
static s32 D_800A33EC;

void resource_LoadOrReuseSelectedBlock(s32 arg0, s32 arg1) {
    s32 *s0;
    s32 s7;
    s32 *s6, *s4, *s2;
    s32 s3;
    s32 *s1;
    s32 *tim_group;
    s32 *collision_grid, *sp18, *sp20;

    s0 = (s32 *)arena_GetBlockAddr(7);
    if (s0 != NULL) {
        if (g_loaded_resource_selector == arg0) {
            s7 = 1;
            switch (arg0) {
            case 3:
                break;
            case 4:
            case 7:
            case 18:
                s3 = s0[0];
                {
                    s32 off = ALIGN4(s0[s3 - 1]);
                    gpu_UnpackGridRecordsNodesAndCellRuns(
                        (u16 *)PTR_OFF(s0, off), 7);
                }
                break;
            case 34:
                s7 = 0;
                break;
            }
            resource_InvokeLoadedSelectorInitCallback();
            if (s7 != 0) {
                return;
            }
        }
    }

    g_loaded_resource_selector = (s16)arg0;
    /* FAKE: s7 (the early-return flag above) reused as slot id 7 for
     * arena_SetBlockEnd / arena_SetMoveCallback; the literal or a separate
     * local scores 37.
     */
    s7 = 7;
    s0 = arena_OpenBlock(7);

    if (arg1 != 0) {
        func_80044F30(arg0, arg1);
    } else {
        func_80044F30(arg0, (s32)s0);
    }

    if (arg1 != 0) {
        /* s3 is the header word count of the buffer being read (s0's in the
         * case blocks and below, arg1's here), the way SOTN's white-dragon
         * segment update reuses var_s2 for posX then posY (Q51; a separate
         * local or the inline read: score 2) */
        /* SOTN: src/st/rare/e_white_dragon.c:78 @db41b28 */
        s3 = *(s32 *)arg1;
        arena_Copy(arg1, (s32)s0, ((s32 *)arg1)[s3]);
    }

    {
        s32 off1_raw = s0[1];
        s3 = s0[0];
        s6 = (s32 *)((u8 *)s0 + ALIGN4(off1_raw));
        {
            s32 *a0_ptr = (s32 *)((s3 << 2) + (s32)s0);
            s4 = (s32 *)((u8 *)s0 + ALIGN4(a0_ptr[-1]));
            collision_grid = (s32 *)((u8 *)s0 + ALIGN4(s0[2]));
            sp18 = (s32 *)((u8 *)s0 + ALIGN4(s0[3]));
            sp20 = (s32 *)((u8 *)s0 + ALIGN4(s0[4]));

            {
                s32 off = ALIGN4(a0_ptr[0]);
                s2 = (s32 *)((u8 *)s0 + off);
            }

            if (arg1 != 0) {
                tim_group = (s32 *)((u8 *)arg1 + ALIGN4(((s32 *)arg1)[s3]));
            } else {
                tim_group = s2;
                {
                    s32 off3 = ALIGN4(a0_ptr[1]);
                    arena_UpdatePeak(PTR_OFF(s0, off3));
                }
            }
        }
    }

    D_8009947A = 0;
    /* FAKE: s1 = s4 routed through a delta-rebase detour that combine folds
       back (zero bytes); the extra refs lift s1's allocation priority above
       the s2 pointer, as in the target; plain s1 = s4: score 32 */
    s1 = (s32 *)((s32)s4 - (s32)s0);
    s1 = (s32 *)((s32)s1 + (s32)s0);
    switch (arg0) {
    case 3: {
        /* FAKE: once-written pointer to s0[s3 - 1]; it fixes the sched1
         * load/store order; s0[s3 - 1] inline: score 9 */
        s32 *hp = (s32 *)((s3 << 2) + (s32)s0) - 1;
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(*hp));
        D_8009947A = 1;
        break;
    }
    case 4:
    case 7:
    case 18:
        s1 = s2;
        reloc_RelocateBlock((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);
        s1 = (s32 *)node_BindFrameDataAndReserveEntryBuffer(
            (s16 *)PTR_OFF(s0, ALIGN4(s0[6])), 8, (s32)s1);
        break;
    case 11:
        wave_SetCellBuffer((s32)s1);
        s1 = (s32 *)((s32)s1 + wave_GetCellBufferSize());
        break;
    case 13:
        s1 = s2;
        s6 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 2]));
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[s3 - 1]));
        reloc_RelocateBlock((s32 *)PTR_OFF(s0, ALIGN4(s0[5])), 8);
        D_8009947A = 1;
        break;
    case 34:
        s1 = s2;
        D_8009947A = 1;
        s4 = (s32 *)((u8 *)s0 + ALIGN4(s0[5]));
        break;
    }

    reloc_RelocateBlock(s6, 7);
    gpu_LoadTimGroupWithConditionalClutRowAdvance((u32 *)tim_group, 0);
    gpu_UnpackGridRecordsNodesAndCellRuns((u16 *)s4, 7);
    collision_SetGrid((s32)collision_grid);
    D_800A33B0 = (s32)sp18;
    g_fight_position_records = (s32)sp20;
    DrawSync(0);
    arena_SetBlockEnd(s7, (s32)s1);
    arena_SetMoveCallback(s7, (s32)resource_RebaseLoadedBlockSections);
    resource_InvokeLoadedSelectorInitCallback();
    if (g_fight_mode_selector != 0) {
        if (arg0 != 0x22) {
            resource_SelectRetainedSubblockOrApplyHeaderSections(-1);
        }
    }
}

void resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks(void) {
    s32 *s0;
    s32 *s1;
    s32 *a0;
    s32 v0;

    if (D_8009947A == 0) {
        return;
    }
    s0 = (s32 *)arena_GetBlockAddr(7);
    v0 = ((u32)s0[1] >> 2) << 2;
    a0 = (s32 *)((u8 *)s0 + v0);
    switch (g_loaded_resource_selector) {
    case 0xD:
        v0 = ((u32)s0[6] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    case 3:
    case 0x22:
        v0 = ((u32)s0[5] >> 2) << 2;
        s1 = (s32 *)((u8 *)s0 + v0);
        break;
    }
    reloc_RelocateBlock(a0, 7);
    gpu_UnpackGridRecordsNodesAndCellRuns((u16 *)s1, 7);
    arena_Resize(7, (s32)((u8 *)s1 - (u8 *)s0));
    D_8009947A = 0;
}

void resource_SelectRetainedSubblockOrApplyHeaderSections(s32 a0) {
    s32 *s0;
    s32 *s2;
    s32 *s3;
    s32 *s4p;
    s32 *s1p;
    s32 *s0p;
    s32 v0;
    if (a0 < 0) {
        resource_ApplyHeaderSectionsResizeBlockAndClearRetainedSubblocks();
        return;
    }
    if (D_8009947A == 0) {
        return;
    }
    v0 = arena_GetBlockAddr(7);
    s0 = (s32 *)v0;
    v0 += 4;
    v0 = v0 + a0 * 20;
    s2 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s3 = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s4p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    v0 += 4;
    s1p = (s32 *)((u8 *)s0 + *(s32 *)v0);
    s0p = (s32 *)((u8 *)s0 + *(s32 *)(v0 + 4));
    reloc_UnrelocateBlock(7);
    reloc_RelocateBlock(s2, 7);
    collision_SetGrid((s32)s3);
    D_800A33B0 = (s32)s4p;
    g_fight_position_records = (s32)s1p;
    gpu_UnpackGridRecordsNodesAndCellRuns((u16 *)s0p, 7);
}

void resource_FreeLoadedBlockAndClearSelectorAndRetainedSubblocks(void) {
    arena_Free(7);
    g_loaded_resource_selector = -1;
    D_8009947A = 0;
}

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void wave_MoveCellBuffer(s32);

void resource_RebaseLoadedBlockSections(s32 a0, s32 a1) {
    s32 rebase_delta;
    reloc_RebaseBlock(7, a1);
    collision_RebaseGrid(a1);
    rebase_delta = (a1 / 4) * 4;
    D_800A33B0 += rebase_delta;
    g_fight_position_records += rebase_delta;
    switch (g_loaded_resource_selector) {
    case 4:
    case 7:
    case 18:
        resource_RebaseFrameAnimatedEntryBlockAndPointers(a1);
        break;
    case 13:
        reloc_RebaseBlock(8);
        break;
    case 11:
        wave_MoveCellBuffer(a1);
        break;
    }
}

/* 0x800153F0: the 22-halfword record light_Unpack unpacks (it walks it as
   u16). light_InitDefaults copies it whole by assignment: the copy's run-time
   alignment test in the target bytes is the halfword type's alignment. */
typedef struct {
    u16 v[22];
} Unk800153F0Record;

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void gpu_InitVramScroll(s32, s32);
extern void gpu_QueueVramScroll(s32);
extern s32 wave_Init(void);
extern void wave_Update(void);
extern void node_RebaseRetainedDrawNodeBuffer(s32);
extern void resource_LoadOrReuseSelectedBlock(s32, s32);
extern void resource_FreeLoadedBlockAndClearSelectorAndRetainedSubblocks(void);

extern s16 D_800A324A;

extern MATRIX g_math_light_plane_projection;

/* row 8's last pulls (wave_Update writes [0..16]); 18 words up to
 * D_800EF848, which wave_SampleLastRowStepsAtCachedIndex's [v1 + 1] reaches at
 * v1 == 17 */
extern s32 D_800EF800[18];
extern u8 g_fight_start_position_pairs;
extern s16 g_gpu_raised_ot_cutoff_requested;
extern MATRIX D_800EEDD0;

extern void node_InitRootNodePairWithNegatedRotationMatrices(void);
extern void light_InitDefaults(void);
extern s16 g_color_mode;
extern s16 D_800F665A;
extern s16 D_800A3248;

void resource_RebaseNodeAssetBlock(s32 a0, s32 a1);

s32 func_80046780(void) { return D_800A33B0; }

s32 fight_GetPositionRecordList(void) { return g_fight_position_records; }

s32 resource_GetLoadedSelector(void) { return g_loaded_resource_selector; }

s32 resource_HasRetainedSubblocks(void) { return D_8009947A; }

s32 *resource_LoadOrReuseGrayscaleAdjustedNodeAsset(s32 a0) {
    s32 arg = a0;
    s32 arena_block_id = 8;
    s32 *s2;

    if (gpu_RefreshGrayscaleMode()) {
        arg = arg + 0x1B;
    }
    {
        s32 *a1 = arena_FindBlock(8);
        if (a1 && D_800A3248 == arg) {
            s2 = (s32 *)a1[1];
        } else {
            s32 *s1;
            s32 *s0;
            s2 = arena_OpenBlock(arena_block_id);
            func_80044F80(arg, (s32)s2);
            {
                s32 off1 = (u32)s2[2] >> 2 << 2;
                s32 off0 = (u32)s2[1] >> 2 << 2;
                s1 = (s32 *)((u8 *)s2 + off1);
                s0 = (s32 *)((u8 *)s2 + off0);
            }
            arena_UpdatePeak((s32)s1);
            reloc_RelocateBlock(s0, 9);
            arena_SetBlockEnd(arena_block_id, (s32)s1);
            D_800A3248 = arg;
        }
    }
    arena_SetMoveCallback(arena_block_id, (s32)resource_RebaseNodeAssetBlock);
    return s2;
}

void resource_ResizeNodeAssetBlockAndUpdatePeak(s32 a0) {
    arena_Resize(8, a0);
    arena_UpdatePeak(0);
}

void resource_RebaseNodeAssetBlock(s32 a0, s32 a1) {
    node_RebaseRetainedDrawNodeBuffer(a1);
    reloc_RebaseBlock(9, a1);
}

void resource_FreeNodeAssetBlock(void) { arena_Free(8); }

void func_80046934(void) { arena_OpenBlock(9); }

void func_80046954(void) {}

void func_8004695C(s32 a0) {
    arena_UpdatePeak(a0);
    arena_SetBlockEnd(9, a0);
    arena_SetMoveCallback(9, (s32)func_80046954);
}

void func_800469A0(s32 a0) { arena_Resize(9, a0); }

void func_80046A80(s32, s32);

s32 *motion_LoadPlaybackBlock(s32 a0) {
    s32 *v0;
    s32 offset;

    v0 = arena_OpenBlock(0xA);
    ndata_ReadChecked(a0, (s32)v0);
    offset = (u32)v0[v0[0] + 1] >> 2 << 2;
    {
        s32 *s0 = (s32 *)((u8 *)v0 + offset);
        arena_UpdatePeak((s32)s0);
        arena_SetBlockEnd(0xA, (s32)s0);
    }
    D_800A324A = (s16)a0;
    arena_SetMoveCallback(0xA, (s32)func_80046A80);
    return v0;
}

void motion_FreePlaybackBlock(void) { arena_Free(0xA); }

void func_80046A80(s32 a0, s32 a1) { motion_RebasePlayback(a1); }

void resource_FreeSlotsAndBlocksAndInvalidateEntryCache(void) {
    slot_ReleaseObjectAndResources(0);
    slot_ReleaseObjectAndResources(1);
    resource_FreeLoadedBlockAndClearSelectorAndRetainedSubblocks();
    resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs();
    resource_InvalidateEntrySelectionCache();
    resource_FreeNodeAssetBlock();
}

void rcnt_StartCnt1(void) {
    SetRCnt(0xF2000001, -1, 0x2000);
    StartRCnt(0xF2000001);
}

void rcnt_GetCnt1(void) { GetRCnt(0xF2000001); }

void func_80046B44(void) {
    ndata_StartReadInf();
    arena_Reset();
    math_InitRotMatrixFuncs();
    reloc_ClearCounts();
    node_InitRootNodePairWithNegatedRotationMatrices();
    light_InitDefaults();
    gpu_InitFogDrawMoves();
    slot_ClearObjectPointersAndTextureRequests();
    softbody_FreeAllAndClearWriteOnlyWords();
    g_color_mode = 0;
    D_800F6650 = 0;
    g_gpu_raised_ot_cutoff_requested = 1;
    D_800F6656 = 0;
    D_800F6658 = 2;
    D_800F665A = 1;
    D_800F665C = 0;
    D_800A3790 = 0x23;
    D_800A33BC = 0;
}

void gpu_ResetDrawQueueAndPrepareViewAndLighting(
    Vec3i32 *a0, SVECTOR *a1, s32 a2) {
    s32 result[3];
    s32 view_offset[3];
    SVECTOR rot;
    MATRIX matrix_buf;

    g_draw_queue_cursor = g_draw_queue;
    D_800A3808 = g_gpu_ot_ptr;
    D_800A38D6++;
    D_800A378C = g_gpu_ot_ptr + 4;

    if (a0 != 0) {
        g_view_camera_root_node.xf.rot.vx = -a1->vx;
        g_view_camera_root_node.xf.rot.vy = -a1->vy;
        g_view_camera_root_node.xf.rot.vz = -a1->vz;

        view_offset[0] = 0;
        view_offset[1] = 0;
        view_offset[2] = -a2;

        rot.vx = -a1->vx;
        rot.vy = -a1->vy;
        rot.vz = -a1->vz;

        g_math_rot_matrix_funcs[0](&rot, &matrix_buf);

        ApplyMatrixLV(&matrix_buf, view_offset, result);

        {
            g_view_camera_root_node.work.t[0] = result[0] + a0->x;
            g_view_camera_root_node.work.t[1] = result[1] + a0->y;
            g_view_camera_root_node.work.t[2] = result[2] + a0->z;
        }

        node_SetMatrixFromNegatedRot(&g_view_camera_root_node);
        camera_CopyViewMatrixWithHalvedMiddleRow();
        gpu_ResetAndStampViewCenteredCellAttributes();

        D_800A33C0 = a2;
    }

    calc_LightMatrix(&g_light_setups[0]);
    calc_LightMatrix(&g_light_setups[1]);
    calc_LightMatrix(&g_light_setups[4]);
    light_ResetPointLight();
    ClearSavedWordPairsAndTwoWriteOnlyWords();
    func_8003F5CC();
}

void gpu_RunOptionalGridDrawAndFogPassApplyPointLightTargetsAndFlushDrawQueue(
    s32 a0) {
    if (a0 & 1) {
        gpu_RunGridDrawPassAndQueueFogMoves(D_800A33C0);
    }
    light_ApplyPointLightTargets();
    func_800444BC();
}

s32 func_80046DE4(void) { return 0; }

void *node_GetSlotMatrixPointerArray(s32 a0) {
    Unk80045878Obj *v0 = slot_GetObject(a0);
    if (v0) {
        return v0->node_matrix_ptrs;
    }
    return NULL;
}

void *node_GetSlotTransformNodes(s32 a0) {
    Unk80045878Obj *v0 = slot_GetObject(a0);
    if (v0) {
        return v0->transform_nodes;
    }
    return NULL;
}

void gpu_ClearRaisedOtCutoffRequest(void) {
    g_gpu_raised_ot_cutoff_requested = 0;
}

void gpu_SetRaisedOtCutoffRequested(s32 a0) {
    if (a0) {
        g_gpu_raised_ot_cutoff_requested = 1;
    } else {
        g_gpu_raised_ot_cutoff_requested = 0;
    }
}

s32 gpu_GetRaisedOtCutoffRequested(void) {
    return g_gpu_raised_ot_cutoff_requested;
}

void func_80046E8C(void) { D_800A3790 = 0x23; }

void gpu_RunGridDrawPassAndQueueFogMoves(s32 a0) {
    gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards(a0);
    {
        s32 v0 = resource_GetLoadedSelector();
        gpu_LatchFogDepthOnTransitionAndQueueMoves(v0, a0);
    }
}

void resource_LoadOrReuseSelectedBlockSetLightingAndBuildFogImageLevels(
    s32 a0, s32 a1) {
    resource_LoadOrReuseSelectedBlock(a0, a1);
    ApplyLightPresetRowBackgroundClearAndFarColor(a0);
    gpu_BuildFogRowLevels();
}

void *fight_GetStartPositionPairTable(void) {
    return &g_fight_start_position_pairs;
}

void math_SetLightPlaneProjection(void) {
    s32 num = (s32)g_light_setups[0].lmat.m[0][0] << 12;
    s32 div = g_light_setups[0].lmat.m[0][1];
    s32 v0 = num / div;
    s32 v1 = ((s32)g_light_setups[0].lmat.m[0][2] << 12) / div;
    g_math_light_plane_projection.m[0][2] = 0;
    g_math_light_plane_projection.m[1][0] = 0;
    v1 = -(s16)v1;
    g_math_light_plane_projection.m[1][1] = 0;
    g_math_light_plane_projection.m[1][2] = 0;
    g_math_light_plane_projection.m[2][0] = 0;
    g_math_light_plane_projection.m[0][0] = 0x1000;
    g_math_light_plane_projection.m[2][2] = 0x1000;
    v0 = -(s16)v0;
    g_math_light_plane_projection.m[0][1] = v0;
    g_math_light_plane_projection.m[2][1] = v1;
}

void math_ApplyLightPlaneProjection(MATRIX *a0, MATRIX *a1, s32 a2) {
    gte_MulMatrix0ClearTrans(&g_math_light_plane_projection, a0, a1);
    a1->t[0] =
        a0->t[0] +
        (((a0->t[1] - a2) * g_math_light_plane_projection.m[0][1]) >> 12);
    a1->t[1] = a2;
    a1->t[2] =
        a0->t[2] +
        (((a0->t[1] - a2) * g_math_light_plane_projection.m[2][1]) >> 12);
}

void math_LightPlaneProject(s32 light_setup_index, MATRIX *src_matrix,
                            MATRIX *projected_matrix_out, s32 plane_y) {
    MATRIX light_projection;

    light_projection.m[0][0] = 0x1000;
    light_projection.m[0][1] =
        (s16) - ((s32)(g_light_setups[light_setup_index].lmat.m[0][0] << 12) /
                 g_light_setups[light_setup_index].lmat.m[0][1]);
    light_projection.m[0][2] = 0;
    light_projection.m[1][0] = 0;
    light_projection.m[1][1] = 0;
    light_projection.m[1][2] = 0;
    light_projection.m[2][0] = 0;
    light_projection.m[2][1] =
        (s16) - ((s32)(g_light_setups[light_setup_index].lmat.m[0][2] << 12) /
                 g_light_setups[light_setup_index].lmat.m[0][1]);
    light_projection.m[2][2] = 0x1000;
    gte_MulMatrix0ClearTrans(
        &light_projection, src_matrix, projected_matrix_out);
    projected_matrix_out->t[0] =
        src_matrix->t[0] +
        (((src_matrix->t[1] - plane_y) * light_projection.m[0][1]) >> 12);
    projected_matrix_out->t[1] = plane_y;
    projected_matrix_out->t[2] =
        src_matrix->t[2] +
        (((src_matrix->t[1] - plane_y) * light_projection.m[2][1]) >> 12);
}

typedef struct {
    s32 w[8];
} Block32;

void camera_CopyViewMatrixWithHalvedMiddleRow(void) {
    D_800EEDD0 = g_view_camera_root_node.xf.mat;
    D_800EEDD0.m[1][0] >>= 1;
    D_800EEDD0.m[1][1] >>= 1;
    D_800EEDD0.m[1][2] >>= 1;
}

MATRIX *camera_GetViewMatrixWithHalvedMiddleRow(void) { return &D_800EEDD0; }

void node_InitIdentity(Unk80101DF0Record *node) {
    node->unk4 = 8;
    node->rotation_order = 0;
    node->unk2 = 0;
    node->unk0 = 0;
    node->unk1 = 0;
    node->parent_node = 0;
    node->light_setup_index = 4;
    node->xf.rot.vx = 0;
    node->xf.rot.vy = 0;
    node->xf.rot.vz = 0;
    g_math_rot_matrix_funcs[node->rotation_order](&node->xf.rot, &node->work);
    node->work.t[2] = 0;
    node->work.t[1] = 0;
    node->work.t[0] = 0;
    node->xf.mat = node->work;
}

s16 *camera_CalcAngles(void) {
    SVECTOR rot;
    VECTOR var_1;
    MATRIX mtx;
    s16 var_3;

    math_RotMatrixYXZ((u16 *)&g_view_camera_root_ptr->xf.rot, &mtx);
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x1000;
    ApplyMatrix(&mtx, &rot, &var_1);
    var_3 = ratan2(var_1.vx, var_1.vz);
    var_1.vz = ((s32)Judge[(var_3 + 0x400) & 0xFFF] * var_1.vz +
                (s32)Judge[var_3 & 0xFFF] * var_1.vx) >>
               12;
    D_800A33C8[0] = -ratan2(var_1.vy, var_1.vz);
    D_800A33C8[1] = var_3;
    return D_800A33C8;
}

void func_8004746C(void) { node_InitFrameEntriesFromWorldTranslations(); }

void func_8004748C(void) { node_AdvanceFrameAnimatedEntriesAndQueueDraws(); }

void func_800474AC(void) { gpu_InitVramScroll(0, 0); }

void func_800474D0(void) { gpu_QueueVramScroll(0); }

void func_800474F0(void) { node_InitFrameEntriesFromWorldTranslations(); }

void func_80047510(void) { node_AdvanceFrameAnimatedEntriesAndQueueDraws(); }

void wave_InitCallback(void) { wave_Init(); }

void wave_UpdateCallback(void) { wave_Update(); }

void node_InitViewOffsetNodeWithYXZRotation(void) {
    node_InitIdentity(&D_800EEDF0);
    D_800EEDF0.rotation_order = 4;
}

void node_OrientAndQueueViewOffsetNode(void) {
    SVECTOR rot;
    VECTOR result;
    MATRIX buf1;
    MATRIX buf2;
    s16 angle;
    s32 var_5;
    Unk80101DF0Record *base;

    if (resource_HasRetainedSubblocks() != 0) {
        return;
    }

    rot.vx = 0;
    rot.vy = 0;
    rot.vz = 0x6590;
    ApplyMatrix(&g_view_camera_root_node.xf.mat, &rot, &result);

    angle = ratan2(result.vx, result.vz);

    var_5 = ((s32)Judge[(angle + 0x400) & 0xFFF] * result.vz +
             (s32)Judge[angle & 0xFFF] * result.vx) >>
            12;
    result.vz = var_5;

    base = &D_800EEDF0;
    {
        /* FAKE: s16 temporary for the negated pitch; storing -ratan2()
         * straight into base->xf.rot.vx moves its negu and the addiu: score
         * 3 (Q129). */
        s16 neg = -ratan2(result.vy, var_5);
        base->xf.rot.vx = neg;
    }
    base->xf.rot.vy = angle;
    base->xf.mat.t[0] = g_view_camera_root_node.xf.mat.t[0];
    base->xf.mat.t[1] = g_view_camera_root_node.xf.mat.t[1];
    base->xf.mat.t[2] = g_view_camera_root_node.xf.mat.t[2] + 0x6590;
    g_math_rot_matrix_funcs[4](&base->xf.rot, &buf1);
    g_math_rot_matrix_funcs[0](&g_view_camera_root_node.xf.rot, &buf2);
    MulMatrix0(&buf2, &buf1, &base->xf.mat);

    {
        void **temp = g_draw_queue_cursor;
        g_draw_queue_cursor = temp + 1;
        *temp = base;
    }
}

void func_80047738(void) { gpu_InitVramScroll(1, 0); }

void func_8004775C(void) { gpu_QueueVramScroll(0); }

void func_8004777C(void) { node_InitFrameEntriesFromWorldTranslations(); }

void func_8004779C(void) { node_AdvanceFrameAnimatedEntriesAndQueueDraws(); }

void func_800477BC(void) {}

void func_800477C4(void) {}

void func_800477CC(void) {}

void func_800477D4(void) {}

void wave_SetCellBuffer(s32 a0) { g_wave_cell_buffer = (s16 *)a0; }

/* 17 phases and the 9 x 17 table wave_Update steps from them
 * (D_800EF800 takes the last row's deltas) */
extern s32 D_800EF558[17];
extern s32 D_800EF59C[9][17];

s32 wave_Init(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    u16 s1val;
    u16 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 val;

    s0 = g_wave_cell_buffer;
    s3val = GetTPage(0, 0, 0x2C0, 0x1C0);
    s2val = GetTPage(0, 0, 0x2C0, 0x180);
    s1val = GetClut(0x10, 0x1E0);
    t1val = GetClut(0x10, 0x1E0);
    a3 = 0;
    t2 = 0x2C00;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        a1 = 0;
        v1 = 1;
        do {
            if (a3 >= 5) {
                *s0 = s3val;
                s0 += 1;
                *s0 = t2;
                s0 += 1;
                *s0 = s1val;
                s0 += 1;
                v0 = -0xC1;
                *s0 = v0;
                s0 += 1;
                v0 = -0x100;
                *s0 = v0;
                s0 += 1;
                v0 = -0x3FC1;
                *s0 = v0;
                s0 += 1;
                v0 = -0x4000;
            } else {
                *s0 = s2val;
                s0 += 1;
                *s0 = t2;
                s0 += 1;
                *s0 = t1val;
                s0 += 1;
                v0 = -0x40C1;
                *s0 = v0;
                s0 += 1;
                v0 = -0x4100;
                *s0 = v0;
                s0 += 1;
                v0 = -0x7FC1;
                *s0 = v0;
                s0 += 1;
                v0 = -0x8000;
            }
            *s0 = v0;
            s0 += 1;
            if (a3 & 1) {
                v0 = a2 | t0;
                *s0 = v0;
                s0 += 1;
                v0 = v1 | a1;
            } else {
                v0 = v1 | a1;
                *s0 = v0;
                s0 += 1;
                v0 = a2 | t0;
            }
            *s0 = v0;
            s0 += 1;
            *s0 = 0;
            s0 += 1;
            *s0 = 0;
            s0 += 1;
            v0 = -0x1000;
            *s0 = v0;
            s0 += 1;
            t0 += 0x100;
            a2 += 1;
            a1 += 0x100;
            a0 += 1;
            v1 += 1;
        } while (a0 < 0x10);
        a3 += 1;
        a0 = 0;
    } while (a3 < 8);

    {
        Unk80101DF0Record *node = &D_800EF070;
        node->unk0 = 0xE;
        node->light_setup_index = 4;
        node->work.t[0] = -0x2EE0;
        node->unk1 = 0;
        node->work.t[1] = 0;
        node->work.t[2] = -0xFA0;
        node->xf.rot.vx = 0;
        node->xf.rot.vy = 0;
        node->xf.rot.vz = 0;
        node->rotation_order = 0;
        node->parent_node = 0;
        node->matrix_state = 0;
        node_UpdateWorldMatrix(node);
    }

    a3 = 0;
    w = 0;
    do {
        val = w;
        p = D_800EF59C[a3];
        for (a0 = 0x10; a0 >= 0; a0--) {
            p[a0] = val;
        }
        w += 0x7D0;
        a3 += 1;
    } while (a3 < 9);

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0)
        goto loop3;

    return (s32)s0 - (s32)g_wave_cell_buffer;
}

void wave_Update(void) {
    s32 i;
    s32 j;
    s32 a0;
    s32 v1;
    void **temp;

    for (i = 0; i < 17; i++) {
        D_800EF59C[0][i] = ((s32)Judge[D_800EF558[i] & 0xFFF] * 0x271) >> 10;
        D_800EF558[i] += 0x12;
    }
    for (i = 1; i < 9; i++) {
        for (j = 0; j < 17; j++) {
            a0 = 0x7D0 - (D_800EF59C[i][j] - D_800EF59C[i - 1][j]);
            if (a0 < 0) {
                v1 = (a0 + 0xF) >> 4;
            } else {
                v1 = a0 / 10;
            }
            D_800EF59C[i][j] += v1;
            if (i == 8) {
                D_800EF800[j] = v1;
            }
        }
    }
    temp = g_draw_queue_cursor;
    g_draw_queue_cursor = temp + 1;
    *temp = &D_800EF070;
}

extern SVECTOR D_800EF0D8[17];
extern SVECTOR D_800EF168[17];

void wave_Draw(void) {
    s32 sxy0, sxy1, sxy2, var_3, flag;
    s32 sz0, sz1, sz2;
    s16 *s7val;
    s32 i, j;
    s32 *src;
    SVECTOR *base;
    SVECTOR *v;
    s32 *dst32;
    s16 *dst16;
    s16 z;

    s7val = g_wave_cell_buffer;
    i = 0;
    src = D_800EF59C[0];
    while (i < 9) {
        if (i & 1) {
            base = D_800EF168;
            dst32 = (s32 *)0x1F800068;
            dst16 = (s16 *)0x1F800134;
        } else {
            base = D_800EF0D8;
            dst32 = (s32 *)0x1F800020;
            dst16 = (s16 *)0x1F800110;
        }
        v = base;
        j = 0;
        z = -0x2EE0;
        while (j < 17) {
            v->vx = src[j] - 0xFA0;
            v->vy = 0;
            v->vz = z;
            v++;
            z += 0x7D0;
            j++;
        }
        v = base;
        j = 0;
        while (j < 6) {
            RotTransPers3(
                &v[0], &v[1], &v[2], &sxy0, &sxy1, &sxy2, &var_3, &flag);
            v += 3;
            ReadSZfifo3(&sz0, &sz1, &sz2);
            *dst32++ = sxy0;
            *dst32++ = sxy1;
            *dst32++ = sxy2;
            *dst16++ = sz0;
            *dst16++ = sz1;
            *dst16++ = sz2;
            j++;
        }
        if (i != 0) {
            s7val = gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision(
                0x10, (s16 *)base, s7val, 0);
        }
        i++;
        src += 17;
    }
}

s32 wave_SampleLastRowCachingIndexAndRemainderParity(s32 a0) {
    s32 a1 = (a0 + 0x7D00) / 3200;
    s32 a0_div = a0 / 3200;
    s32 remainder = a0 - a0_div * 3200;
    s32 var_3 = remainder & 1;
    D_800A33D4 = a1;
    D_800A33D8 = var_3;
    if ((u32)a1 >= 18) {
        return (s32)0xFFFE7960;
    }
    {
        /* row 8 of the table; a1 + 1 == 17 / 18 reads on into D_800EF800,
         * which follows the table */
        s32 val1 = D_800EF59C[8][a1] * var_3;
        s32 val2 = D_800EF59C[8][a1 + 1] * (0x1000 - var_3);
        return ((val1 + val2) >> 12) - 0x3F48;
    }
}

s32 wave_SampleLastRowStepsAtCachedIndex(void) {
    s32 v1 = D_800A33D4;
    if ((u32)v1 >= 18) {
        return 0;
    }
    {
        s32 v0 = D_800EF800[v1];
        s32 a0 = D_800A33D8;
        s32 val1 = v0 * a0;
        s32 v3 = D_800EF800[v1 + 1];
        s32 val2 = v3 * (0x1000 - a0);
        return (val1 + val2) >> 12;
    }
}

s32 wave_GetCellBufferSize(void) { return 0xD00; }

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);

void wave_MoveCellBuffer(s32 a0) {
    g_wave_cell_buffer = (s16 *)((u8 *)g_wave_cell_buffer + a0);
}

void gpu_LoadTimGroup(u32 *p, s32 arg1) {
    /* FAKE: unwritten volatile pad reproduces the target's untouched 32-byte
     * locals region (sp+0x18..0x37) (phantom-frame pad; without it: score
     * 10). SOTN precedent:
     * `volatile u32 pad; // !FAKE:` (src/st/sel/2C048.c:564), `volatile u32
     * pad[4]; // FAKE` (src/st/sel/stream.c:80). */
    volatile u32 pre_pad[8];
    s32 saved;
    s16 new_var;
    s32 count;
    u32 v_off;
    unsigned int new_var2;
    saved = (s32)p;
    p += (s16)arg1;
    v_off = *p;
    p = (u32 *)(saved + ((v_off >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 first;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            first = saved + (new_var2 << 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            gpu_LoadTim(first, new_var, a2v, a3v, v0v);
        } while ((count--) != 0);
    }
}

void gpu_LoadTimGroupOffset(u32 *p, s32 arg1, s16 arg2, s16 arg3) {
    // !FAKE: phantom-frame-slot pad, without it: score 14
    // (no-new-park-categories): target reserves 32 locals bytes at
    // sp+0x18..sp+0x37 that no instruction touches
    volatile u32 pre_pad[8];
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = (s32)p;
    p += (s16)arg1;
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            gpu_LoadTim(new_var, (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                        (s32)a3v + sx_arg2, (s32)v0v + sx_arg3);
        } while ((count--) != 0);
    }
}

void gpu_LoadTimGroupSplitOffset(
    u32 *p, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5) {
    // !FAKE: phantom-frame-slot pad (without it: score 20): target
    // reserves 32 locals bytes at sp+0x18..sp+0x37 that no instruction touches
    volatile u32 pre_pad[8];
    s32 base_addr;
    s32 count;
    s32 new_var;
    base_addr = (s32)p;
    p += (s16)arg1;
    p = (u32 *)(base_addr + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        s32 sx_arg4;
        s32 sx_arg5;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            new_var = base_addr + (((u32)word >> 2) << 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            gpu_LoadTim(new_var, (s32)a1v + sx_arg2, (s32)a2v + sx_arg3,
                        (s32)a3v + sx_arg4, (s32)v0v + sx_arg5);
        } while ((count--) != 0);
    }
}

void gpu_LoadTimGroupWithConditionalClutRowAdvance(u32 *p, s32 arg1) {
    /* FAKE: unwritten volatile pad reproduces the target's untouched 32-byte
     * locals region (sp+0x18..0x37), as the FAKE pads of gpu_LoadTimGroup /
     * gpu_LoadTimGroupOffset (phantom-frame pad; without it: score 10). SOTN
     * precedent: `volatile u32 pad;
     * // !FAKE:` (src/st/sel/2C048.c:564), `volatile u32 pad[4]; // FAKE`
     * (src/st/sel/stream.c:80). */
    volatile u32 pre_pad[8];
    u32 *base;
    s32 count;
    s32 var_4;
    base = p;
    p += (s16)arg1;
    p = (u32 *)((s32)base + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            /* compute early (target sched) */
            var_4 = (s32)base + (new_var2 << 2);
            p = (u32 *)(((s32)p) + 2); /* always advance (target delay slot) */
            if ((s32)a3v < 0x280) {
                v0v += 1;
            }
            gpu_LoadTim(var_4, (s32)a1v, (s32)a2v, (s32)a3v, (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}

void gpu_LoadTim(arg0, arg1, arg2, arg3, arg4) u8 *arg0;
s16 arg1;
s16 arg2;
s16 arg3;
u16 arg4;
{
    RECT rect;
    u16 buf[512];
    u8 *var_2;
    s32 flags;
    u32 var_4;
    u32 var_5;
    s32 var_6 = arg0[0];
    arg0 += 4;
    if (var_6 != 0x10)
        return;
    flags = *(s32 *)arg0 & 8;
    arg0 += 4;
    if (flags != 0) {
        var_2 = arg0;
        arg0 = var_2 + (((u32) * (u32 *)var_2 >> 2) << 2);
        var_2 += 4;
    }
    arg0 += 8;
    rect.x = arg1;
    rect.y = arg2;
    var_4 = *(u32 *)arg0;
    rect.h = var_4 >> 16;
    rect.w = var_4;
    LoadImage(&rect, (u32 *)(arg0 + 4));
    if (flags == 0)
        return;
    var_2 += 4;
    rect.x = arg3;
    rect.y = arg4;
    var_5 = *(u32 *)var_2;
    var_2 += 4;
    rect.h = var_5 >> 16;
    rect.w = var_5;
    if (gpu_RefreshGrayscaleMode() != 0) {
        math_GrayscaleRgb555((u16 *)var_2, rect.w, buf);
        LoadImage(&rect, (u32 *)buf);
        DrawSync(0);
        return;
    }
    LoadImage(&rect, (u32 *)var_2);
}

void gpu_LoadTimGroupClutsOffset(s32 arg0, s32 arg1, s16 arg2, s16 arg3) {
    s32 base;
    s32 count;
    s32 off;
    arg1 = (((s32)(arg1 << 16)) >> 14) + arg0;
    base = arg0;
    off = *(s32 *)arg1;
    arg0 += off;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            s32 entry;
            u32 dx_u;
            u32 dy_u;
            s32 dx;
            s32 dy;
            entry = base + *(s32 *)arg0;
            arg0 += 8;
            dx_u = *(u16 *)arg0;
            arg0 += 2;
            dy_u = *(u16 *)arg0;
            arg0 += 2;
            dx = ((s32)(dx_u << 16)) >> 16;
            dy = ((s32)(dy_u << 16)) >> 16;
            gpu_LoadTimClut(entry, dx + sx_arg2, dy + sx_arg3);
        } while ((count--) != 0);
    }
}

void gpu_LoadTimClut(arg0, arg1, arg2) u8 *arg0;
s16 arg1;
s16 arg2;
{
    RECT rect;
    u16 buf[512];
    u32 var_2;
    s32 flags;
    if (arg0[0] != 0x10)
        return;
    arg0 += 4;
    flags = *(s32 *)arg0;
    arg0 += 4;
    if ((flags & 8) == 0)
        return;
    arg0 += 8;
    rect.x = arg1;
    rect.y = arg2;
    var_2 = *(u32 *)arg0;
    arg0 += 4;
    rect.h = var_2 >> 16;
    rect.w = var_2;
    if (gpu_RefreshGrayscaleMode() != 0) {
        math_GrayscaleRgb555((u16 *)arg0, rect.w, buf);
        LoadImage(&rect, (u32 *)buf);
        return;
    }
    LoadImage(&rect, (u32 *)arg0);
}

extern void gpu_ReadTim();

s32 gpu_ReadTimGroupEntry(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    s32 base, count, entry, a, b, c, d, off;
    off = ((s32 *)arg0)[arg1];
    base = arg0;
    /* the group's offset relocated against the file base, written as
     * `entry += base` relocates the entry's offset below */
    arg0 = off + base;
    count = *(s32 *)arg0;
    arg0 += 4;
    if (arg2 >= (u32)count)
        return -1;
    arg0 += arg2 * 0xC;
    entry = *(s32 *)arg0;
    arg0 += 4;
    a = (s32) * (u16 *)arg0;
    arg0 += 2;
    b = (s32) * (u16 *)arg0;
    arg0 += 2;
    c = (s32) * (u16 *)arg0;
    arg0 += 2;
    d = (s32) * (u16 *)arg0;
    entry += base;
    gpu_ReadTim(entry, arg3, (s16)a, (s16)b, (s16)c, (s16)d);
    return count;
}

typedef struct {
    /* 0x00 */ s16 mode;
    /* 0x02 */ s16 x;
    /* 0x04 */ s16 y;
    /* 0x06 */ s16 w;
    /* 0x08 */ s16 h;
    /* 0x0A */ s16 cx;
    /* 0x0C */ s16 cy;
    /* 0x0E */ s16 cw;
    /* 0x10 */ s16 ch;
    /* 0x12 */ u16 tpage;
    /* 0x14 */ u16 clut;
    /* 0x18 */ u32 *pixdata;
    /* 0x1C */ u32 *clutdata;
} TimImageRec;

void gpu_ReadTim(tim, spr, x, y, cx, cy) u32 *tim;
TimImageRec *spr;
s16 x, y;
u16 cx, cy;
{
    u32 flag;
    u32 *p;

    if (*(u8 *)tim++ == 0x10) {
        flag = *tim++;
        spr->mode = flag & 7;
        if (flag & 8) {
            u32 bnum;

            p = tim;
            bnum = *p;
            spr->cx = cx;
            spr->cy = cy;
            tim = p + (bnum >> 2);
            p += 2;
            spr->ch = ((u16 *)p)[1];
            spr->cw = *p++;
            spr->clutdata = p;
            spr->clut = GetClut(spr->cx, spr->cy);
        } else {
            spr->clut = 0;
        }
        /* FAKE: the pixel-block cursor set to tim, then stepped past x / y
         * after the spr->x / spr->y stores; `p = tim + 2` (or &tim[2]) after
         * the stores: score 7 (Q129) */
        p = tim;
        spr->x = x;
        spr->y = y;
        p += 2;
        spr->h = ((u16 *)p)[1];
        spr->w = *p++;
        spr->pixdata = p;
        spr->tpage = GetTPage(spr->mode, 0, spr->x & 0xFFC0, spr->y & 0xFF00);
    }
}

s32 gpu_RefreshGrayscaleMode(void) {
    if (gpu_GetSavedGrayscaleFlag()) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
    return g_color_mode;
}

void func_80048744(s32 a0) {
    if (a0) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
}

void math_GrayscaleRgb555(u16 *src_pixels, s32 pixel_count, u16 *write_cursor) {
    s32 temp_a3;
    s32 temp_v1;
    s32 var_t0;
    u16 *read_cursor;
    s32 source_pixel;
    read_cursor = src_pixels;
    var_t0 = pixel_count - 1;
    if (var_t0 != -1) {
        do {
            source_pixel = *read_cursor;
            read_cursor += 1;
            var_t0 -= 1;
            temp_v1 = source_pixel << 0x10;
            {
                s32 b;
                s32 g;
                s32 weighted_blue;
                temp_a3 = source_pixel & 0x1F;
                temp_a3 = temp_a3 * 0x547;
                b = (temp_v1 >> 0x1A) & 0x1F;
                g = (temp_v1 >> 0xA) & 0xF800;
                weighted_blue = b * 0x2B8;
                temp_a3 = ((temp_a3 + g + weighted_blue) >> 0xC) & 0x1F;
            }
            *write_cursor =
                (u16)((((source_pixel & (~0x7FFF)) + (temp_a3 << 0xA)) +
                       (temp_a3 << 5)) +
                      temp_a3);
            write_cursor += 1;
        } while (var_t0 != (-1));
    }
}

s32 math_Grayscale3(s32 arg0, s32 arg1, s32 arg2) {
    arg0 = arg0 * 0x547;
    arg1 = arg1 << 11;
    arg2 = arg2 * 0x2B8;
    return (arg0 + arg1 + arg2) >> 12;
}

void gpu_TintVramRow(
    s32 mode, s32 sx, s32 sy, s32 w, s32 arg4, s32 mg, s32 mb, s32 dx, s32 dy) {
    u16 buf[256];
    u16 out[256];
    RECT rect;
    u16 *src;
    u16 *dst;
    s32 i;
    u16 p;
    s32 r, g, b, a;

    DrawSync(0);
    rect.x = sx;
    rect.y = sy;
    rect.w = w;
    rect.h = 1;
    StoreImage(&rect, (u32 *)buf);
    DrawSync(0);
    src = buf;
    dst = out;
    for (i = 0; i < w; i++) {
        p = *src;
        if (p == 0) {
            *dst++ = *src++;
            continue;
        }
        r = (p & 0x1F) << 3;
        g = ((p >> 5) & 0x1F) << 3;
        b = ((p >> 10) & 0x1F) << 3;
        a = p & 0x8000;
        src++;
        switch (mode) {
        case 0:
            r = (r * arg4) >> 15;
            g = (g * mg) >> 15;
            b = (b * mb) >> 15;
            break;
        case 1:
            r = r * 0x547;
            g = g << 11;
            b = b * 0x2B8;
            r = (r + g + b) >> 15;
            r = (r * arg4) >> 12;
            g = (r * mg) >> 12;
            b = (r * mb) >> 12;
            break;
        }
        r &= 0x1F;
        g &= 0x1F;
        b &= 0x1F;
        *dst++ = a | r | (g << 5) | (b << 10);
    }
    rect.x = dx;
    rect.y = dy;
    LoadImage(&rect, (u32 *)out);
    DrawSync(0);
}

void gpu_TintVramRowInPlace(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    gpu_TintVramRow(0, arg0, arg1, arg2, arg3, arg4, arg5, arg0, arg1);
}

/* matches the definition above */
extern s32 *resource_LoadOrReuseGrayscaleAdjustedNodeAsset(s32);
extern void resource_ResizeNodeAssetBlockAndUpdatePeak(s32);
extern u8 D_80099BCC;

s32 node_LoadSlotMappedAssetAndInitRetainedDrawNodes(s32 arg0) {
    Unk80045878Obj *temp_v0;
    s32 var_1;
    u8 *base;
    s32 delta;
    u8 *p;
    u8 *q;

    temp_v0 = slot_GetObject(arg0);
    if (temp_v0 == 0)
        return 0;
    D_800A33E0 = arg0;
    var_1 = (&D_80099BCC)[temp_v0->unk_08];
    if (var_1 == 0xFF)
        return 0;
    base = (u8 *)resource_LoadOrReuseGrayscaleAdjustedNodeAsset(var_1);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    D_800A33E4 = (s32)p;
    /* FAKE: q = p + 0xA, the target's second cursor (addiu a1,v1,10; stores at
     * -8 / -6 / -9 / 0 from it). As Unk80045878Node stores (unk0 / unk1 /
     * unk2 / unk4 / light_setup_index), no q: p[var_1] subscripts 11 (delta
     * taken as the byte offset first 14, D_800A33E4 stored first 11), p++
     * walk in the for header 12 (delta first 9, field order 0/1/2/4/A 12, p
     * read back from D_800A33E4 9), delta first with p++ in the body 7 */
    q = p + 0xA;
    /* FAKE: the record counter reuses `var_1` (the slot id, tested and
       passed to resource_LoadOrReuseGrayscaleAdjustedNodeAsset above),
       inheriting its $a0 preference; a separate counter swaps $a0/$a2 with
       delta: score 6 (Q129) */
    /* SOTN: src/dra/692E8.c:693 @db41b28 */
    for (var_1 = 0; var_1 < 0x11; var_1++) {
        *(s16 *)(q - 8 + var_1 * 0x68) = var_1;
        *(s16 *)(q - 6 + var_1 * 0x68) = 9;
        p[var_1 * 0x68] = 0xF;
        *(s8 *)(q - 9 + var_1 * 0x68) = 0;
        *(s16 *)(q + var_1 * 0x68) = (s16)arg0;
    }
    resource_ResizeNodeAssetBlockAndUpdatePeak(delta + 0x6E8);
    return 1;
}

void node_RebaseRetainedDrawNodeBuffer(s32 a0) { D_800A33E4 += a0; }

extern u32 *g_gpu_ot256_ptr;
extern u32 g_gpu_ot256_db[][256];
extern s16 D_80099C14[];

void node_SetSlotRelativeMatrixQueueRetainedNodesAndLinkOrderingTable(
    s32 arg0, s32 arg1, s32 arg2) {
    MATRIX mtx;
    SVECTOR rot;
    s32 index;
    s32 old;
    s16 *indices;
    void **list;
    MATRIX **mat_tbl;
    Unk80045878Node *node;

    mat_tbl = node_GetSlotMatrixPointerArray(D_800A33E0);
    if (mat_tbl == 0) {
        return;
    }
    if (arg1 >= 6) {
        arg1 = -1;
    }

    rot.vx = 0x1770;
    rot.vy = 0;
    rot.vz = 0;
    rot.vz = ((s32)Judge[arg0 & 0xFFF] * rot.vx) >> 12;
    rot.vx = ((s32)Judge[(arg0 + 0x400) & 0xFFF] * rot.vx) >> 12;
    node = (Unk80045878Node *)D_800A33E4;
    ApplyMatrix(mat_tbl[0], &rot, (VECTOR *)D_800FF558.t);
    D_800FF558.t[0] += mat_tbl[0]->t[0];
    D_800FF558.t[1] += mat_tbl[1]->t[1];
    D_800FF558.t[2] += mat_tbl[2]->t[2];

    rot.vx = 0;
    rot.vy = 0xC00 - arg0;
    rot.vz = 0;
    indices = D_80099C14;
    math_RotMatrixZYX(&rot, &mtx);
    gte_MulMatrix0ClearTrans(mat_tbl[0], &mtx, &mtx);
    D_800FF558.m[0][0] = mtx.m[0][0];
    D_800FF558.m[0][1] = mtx.m[1][0];
    D_800FF558.m[0][2] = mtx.m[2][0];
    D_800FF558.m[1][0] = mtx.m[0][1];
    D_800FF558.m[1][1] = mtx.m[1][1];
    D_800FF558.m[1][2] = mtx.m[2][1];
    D_800FF558.m[2][0] = mtx.m[0][2];
    D_800FF558.m[2][1] = mtx.m[1][2];
    D_800FF558.m[2][2] = mtx.m[2][2];

    while ((index = *indices++) >= 0) {
        node->node.xf.mat = *mat_tbl[index];
        *g_draw_queue_cursor++ = node;
        node++;
    }
    if (arg1 >= 0) {
        node->node.xf.mat = *mat_tbl[18];
        list = g_draw_queue_cursor;
        node->node.unk2 = arg1 + 0xF;
        g_draw_queue_cursor = list + 1;
        *list = node;
        node++;
    }
    if (arg2 != 0) {
        node->node.xf.mat = *mat_tbl[19];
        list = g_draw_queue_cursor;
        node->node.unk2 = 0x15;
        g_draw_queue_cursor = list + 1;
        *list = node;
    }

    g_gpu_ot256_ptr = g_gpu_ot256_db[g_gpu_presented_frame_count & 1];
    ClearOTagR(g_gpu_ot256_ptr, 0x100);
    old = D_800A378C[0];
    D_800A378C[0] = (u32)(g_gpu_ot256_ptr + 0xFF) & 0xFFFFFF;
    *g_gpu_ot256_ptr = old;
}

void gpu_InitVramScroll(s32 a0, s32 a1) {
    s32 i;
    u16 *src;
    s16 *dst;
    Unk800EF848Rec *base;
    if (a1 > 0) {
        func_80052C10();
    }
    base = &D_800EF848[a1];
    base->phase = 0;
    src = D_80099C34[a0];
    dst = base->ctl;
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}

void gpu_QueueVramScroll(s32 arg0) {
    RECT rect;
    s16 *var_1;
    s32 x;
    s32 y;
    s32 var_4;
    s32 var_5;
    s32 dx;
    s32 dy;
    s32 var_8;
    s32 var_9;
    s32 w;
    s32 var_11;
    s32 i;
    Unk800EF848Rec *var_13;
    DR_MOVE *p;
    s32 var_15;

    var_13 = &D_800EF848[arg0];
    var_15 = var_13->phase;
    var_1 = var_13->ctl;
    p = var_13->move[g_gpu_presented_frame_count & 1];
    x = var_1[0] & ~0x3F;
    y = var_1[1] & ~0xFF;
    var_4 = var_1[0] % 64;
    var_5 = var_1[1] % 256;
    w = var_1[2];
    var_11 = var_1[3];
    dx = var_1[4] & ~0x3F;
    dy = var_1[5] & ~0xFF;
    var_8 = var_1[4] % 64;
    var_9 = var_1[5] % 256;
    for (i = 0; i < 3; i++) {
        s32 var_16 = dx + var_8;
        s32 var_17 = dy + var_9;
        s32 sy = y + var_5;
        s32 h1 = var_11 - var_15;
        /* FAKE: strip two's height taken as an s16 at the top of the level;
         * rect.h = var_15 directly, an s32 copy or a (s16) cast drops the t1
         * copy: score 57 */
        s16 h2 = var_15;
        rect.x = x + var_4;
        rect.y = sy;
        rect.w = w;
        rect.h = h1;
        SetDrawMove(p, &rect, var_16, var_17 + var_15);
        addPrim(&D_800A378C[0xFFF], p);
        p++;
        rect.y = sy + h1;
        rect.h = h2;
        SetDrawMove(p, &rect, var_16, var_17);
        addPrim(&D_800A378C[0xFFF], p);
        p++;
        var_4 >>= 1;
        var_5 >>= 1;
        var_8 >>= 1;
        var_9 >>= 1;
        w >>= 1;
        var_11 >>= 1;
        var_15 >>= 1;
    }
    var_13->phase += var_1[6];
    if (var_13->phase >= var_1[3]) {
        var_13->phase %= var_1[3];
    }
}

extern s16 D_800EF9F2;
extern s16 D_800EF9F4;

void ResetEntrySelectionState(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_800EF9F2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800EF9F4 = -2;
    D_800A33E8[1] = -1;
    D_800A33E8[0] = -1;
    D_800A33EC = -1;
}

extern s16 D_800EF980[];

void resource_RequestEntryWithLinkedEntriesAndLatchCategory(s32 arg0) {
    u8 temp_v1;

    D_800EF980[arg0] = 1;
    if (D_800A33EC == -1) {
        if (arg0 >= 0x33) {
            D_800A33EC = 1;
        } else {
            D_800A33EC = 0;
        }
    } else {
        if (D_800A33EC == 0 && !(arg0 < 0x33)) {
            func_80052C10();
        }
        if (D_800A33EC == 1 && arg0 < 0x33) {
            func_80052C10();
        }
    }
    temp_v1 = D_80099CC8[arg0][0];
    if (temp_v1 != 0xFF) {
        D_800EF980[temp_v1] = 1;
        D_800EF980[D_80099CC8[arg0][1]] = 1;
    }
}

void resource_SetPairSelectionSlotWithBoundChecks(s32 idx, s32 val) {
    s32 cond;
    if (D_800A33EC == 0) {
        cond = val < 16;
    } else {
        cond = val < 8;
    }
    if (!cond) {
        func_80052C10();
    }
    if (((u32)idx) >= 2U) {
        func_80052C10();
    }
    D_800A33E8[idx] = (s16)val;
}

s32 math_UnorderedPairIndex(s32 arg0, s32 arg1, s32 arg2) {
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        sum += arg0;
        arg0--;
    }
    sum += arg2 - arg1;
    return sum;
}

extern s16 D_80099C50[];
extern s32 D_800A324C;
extern s32 math_UnorderedPairIndex(s32, s32, s32);

void resource_RankRequestedEntriesAndReloadOnChange(s32 arg0) {
    s16 *dst;
    s16 *src;
    s16 *p;
    /* FAKE: `i` counts both loops, then holds the pair index (a constant or
       math_UnorderedPairIndex's result); sharing keeps the counter in $s0
       as in the target; a separate `total`: score 12 (Q129) */
    /* SOTN: src/dra/692E8.c:299 @db41b28 */
    s32 i;
    s32 var_4;
    s32 var_5;
    s32 step;
    s32 lo;
    s32 hi;

    var_4 = 1;
    i = 0;
    dst = D_80099C50;
    src = D_800EF980;
    do {
        s16 v = *src;
        if ((v >= 0) != ((*dst) >= 0)) {
            var_4 = 0;
        }
        *dst = v;
        dst++;
        i++;
        src++;
    } while (i < 0x3A);
    var_5 = 0;
    i = 0;
    p = D_800EF980;
    do {
        if ((*p) >= 0) {
            *p = (s16)var_5;
            var_5++;
        }
        i++;
        p++;
    } while (i < 0x3A);
    step = 8;
    if (D_800A33EC == 0) {
        step = 0x10;
    }
    hi = D_800A33E8[0];
    if (hi == -1) {
        lo = D_800A33E8[1];
        if (lo == hi) {
            i = 0x24;
            if (D_800A33EC == 0) {
                i = 0x88;
            }
            goto end;
        }
        hi = lo;
    } else {
        lo = hi;
        if ((D_800A33E8[1] != (-1)) && (hi != D_800A33E8[1])) {
            if (hi < D_800A33E8[1]) {
                hi = D_800A33E8[1];
            } else {
                lo = D_800A33E8[1];
            }
        }
    }
    i = math_UnorderedPairIndex(step, lo, hi);
end:
    if (D_800A324C != i) {
        D_800A324C = i;
        var_4 = 0;
    }
    if (var_4 == 0) {
        resource_FreeSelectedEntryBlockStopAllVoicesAndCloseVabs();
        resource_LoadSelectedEntriesTexturesAndSound(
            D_800A33EC, i, D_800EF980, arg0);
        gpu_BuildFogBlockLevels();
    }
}

void func_80049710(void) {}

/* Appends one or two 0x68-byte draw nodes at g_prim_buf_cursor for animation
 * entry arg0 and queues each on g_draw_queue. The first
 * node (type 0) takes its rotation and position from rot_in / pos (flags == 1)
 * or from node 19 + (flags & 1) of the object in g_slot_objects slot flags >> 1
 * (slot_GetObject), whose offset it scales in place by the object's
 * position_scale and into which it copies the new matrix back; the second (type
 * 3, parent = the first) follows unless flags is still 1. */
void node_QueueEntryNodesFromTransformOrUpdatedSlotNode(
    s32 arg0, s32 flags, s32 *pos, s16 *rot_in) {
    SVECTOR ofs;
    s32 val58;
    Unk80045878Obj *slot_obj;
    Unk80045878Node *obj;
    Unk80045878Node *var_4;
    s32 node_bit;
    if (D_800EF980[arg0] < 0) {
        func_80052C10();
    }
    obj = (Unk80045878Node *)g_prim_buf_cursor;
    /* FAKE: dead store: the 0 is never read (flags == 1 skips the second
     * node) but gives the target's `move s5,zero`: score 1
     * (dead-store-fake-exception) */
    val58 = 0;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    obj->node.unk2 = D_800EF980[arg0] * 2;
    obj->node.unk4 = 6;
    obj->node.rotation_order = 0;
    obj->node.parent_node = 0;
    obj->node.light_setup_index = 4;
    if (flags != 0) {
        if (flags == 1) {
            obj->node.xf.rot.vx = rot_in[0];
            obj->node.xf.rot.vy = rot_in[1];
            obj->node.xf.rot.vz = rot_in[2];
            g_math_rot_matrix_funcs[0](&obj->node.xf.rot, &obj->node.xf.mat);
            obj->node.xf.mat.t[0] = pos[0];
            obj->node.xf.mat.t[1] = pos[1];
            obj->node.xf.mat.t[2] = pos[2];
        } else {
            /* FAKE: flags rewritten in place (bit 15 cleared here, set
             * below); a local copy flips the $s0 allocation order: score 63
             * (Q129) */
            /* SOTN: src/st/lib/e_shop.c:4621 @aa53500 */
            flags &= 0x7FFF;
            node_bit = flags & 1;
            slot_obj = slot_GetObject(flags >> 1);
            var_4 = &slot_obj->transform_nodes[19 + node_bit];
            var_4->node.work.t[0] =
                (var_4->node.work.t[0] * slot_obj->position_scale) >> 12;
            var_4->node.work.t[1] =
                (var_4->node.work.t[1] * slot_obj->position_scale) >> 12;
            var_4->node.work.t[2] =
                (var_4->node.work.t[2] * slot_obj->position_scale) >> 12;
            MulMatrix0(&slot_obj->transform_nodes[0].node.xf.mat,
                       &var_4->node.work, &obj->node.xf.mat);
            ofs.vx = var_4->node.work.t[0];
            ofs.vy = var_4->node.work.t[1];
            ofs.vz = var_4->node.work.t[2];
            ApplyMatrix(&var_4->node.parent_node->xf.mat, &ofs,
                        (VECTOR *)obj->node.xf.mat.t);
            obj->node.xf.mat.t[0] =
                obj->node.xf.mat.t[0] + var_4->node.parent_node->xf.mat.t[0];
            obj->node.xf.mat.t[1] =
                obj->node.xf.mat.t[1] + var_4->node.parent_node->xf.mat.t[1];
            obj->node.xf.mat.t[2] =
                obj->node.xf.mat.t[2] + var_4->node.parent_node->xf.mat.t[2];
            /* SOTN: src/st/lib/e_shop.c:4625 @aa53500 */
            flags |= 0x8000;
            var_4->node.xf.mat = obj->node.xf.mat;
            val58 = slot_obj->unk_1A84;
        }
        {
            void **list = g_draw_queue_cursor;
            g_draw_queue_cursor = list + 1;
            *list = obj;
        }
        obj++;
        if (flags != 1) {
            s32 entry = D_800EF980[arg0];
            void **list;
            obj->node.unk0 = 3;
            obj->node.unk1 = 0;
            /* FAKE: unk58 stored through a pointer; the member store lets sched
             * sink it below the g_draw_queue_cursor load (score 2). */
            {
                s32 *p58 = &obj->unk58;
                *p58 = val58;
            }
            list = g_draw_queue_cursor;
            obj->node.parent_node = &obj[-1].node;
            obj->node.matrix_state = 1;
            obj->node.rotation_order = 0;
            obj->node.light_setup_index = 0;
            obj->node.unk4 = 6;
            obj->node.unk2 = entry * 2 + 1;
            g_draw_queue_cursor = list + 1;
            *list = obj;
            obj++;
        }
        g_prim_buf_cursor = (u32 *)obj;
    }
}

extern s16 D_80099D3C[];

/* node_QueueTableEntryNodePairUnderSlotNode: `volatile u32 pre_pad[2]` is a
 * labelled FAKE (phantom-frame pad) giving the target's frame signature (.frame
 * $sp,48, vars= 8, regs= 5). The only fold-capable symbol (D_80099D3C) cannot
 * host the vars region without costing a sixth callee-saved register, and the
 * function is loopless, so no back-edge carrier exists; hence the FAKE pad.
 * SOTN precedent: `volatile char pad[8] //! FAKE`. */
void node_QueueTableEntryNodePairUnderSlotNode(s32 arg0, s32 arg1, s32 arg2) {
    // !FAKE: phantom-frame-slot pad, without it: score 12
    // (no-new-park-categories): target reserves 8 locals bytes at
    // sp+0x10..sp+0x17 that no instruction touches
    volatile u32 pre_pad[2];
    u8 temp_v1;
    s16 rot_z;
    s16 *src;
    Unk80045878Node *obj;
    Unk80045878Obj *slot_obj;
    void **list;

    temp_v1 = D_80099CC8[arg0][arg2];
    if (temp_v1 == 0xFF) {
        return;
    }
    if (D_800EF980[temp_v1] < 0) {
        func_80052C10();
    }
    slot_obj = slot_GetObject(arg1 >> 1);
    obj = (Unk80045878Node *)g_prim_buf_cursor;
    obj->node.unk0 = 0;
    obj->node.unk1 = 0;
    obj->node.unk2 = D_800EF980[temp_v1] * 2;
    obj->node.unk4 = 6;
    obj->node.rotation_order = 0;
    obj->node.light_setup_index = 4;
    src = &D_80099D3C[(arg1 & 1) * 6];
    obj->node.work.t[0] = ((*src) * slot_obj->position_scale) >> 12;
    src++;
    obj->node.work.t[1] = ((*src) * slot_obj->position_scale) >> 12;
    src++;
    obj->node.work.t[2] = ((*src) * slot_obj->position_scale) >> 12;
    src++;
    obj->node.xf.rot.vx = *src;
    src++;
    obj->node.xf.rot.vy = *src;
    /* FAKE: rot.vz's value read into rot_z ahead of the parent store; read at
     * its own store it scores 8 (Q129). */
    rot_z = src[1];
    obj->node.parent_node = &slot_obj->transform_nodes[12].node;
    obj->node.matrix_state = 0;
    obj->node.xf.rot.vz = rot_z;
    node_UpdateWorldMatrix(&obj->node);
    list = g_draw_queue_cursor;
    g_draw_queue_cursor = list + 1;
    *list = obj;
    obj++;
    obj->node.unk2 = D_800EF980[temp_v1] * 2 + 1;
    obj->node.unk0 = 3;
    obj->node.parent_node = &obj[-1].node;
    obj->node.unk1 = 0;
    obj->node.rotation_order = 0;
    obj->node.matrix_state = 1;
    obj->node.light_setup_index = 0;
    obj->node.unk4 = 6;
    /* FAKE: unk58 stored through a pointer; the member store lets sched lift
       the g_draw_queue_cursor reload above the node's stores (score 17). */
    {
        s32 *p58 = &obj->unk58;
        *p58 = slot_obj->unk_1A84;
    }
    list = g_draw_queue_cursor;
    g_draw_queue_cursor = list + 1;
    *list = obj;
    g_prim_buf_cursor = (u32 *)(obj + 1);
}

s32 snd_CopySelectedPackSectionsAndLoadVabPacks(s32 arg0, s32 arg1) {
    s32 count;
    s32 tail_off;
    s32 temp_a2;
    s32 var_s7;
    s32 v0;
    s32 var_fp;
    s32 var_s5;
    s32 var_s6;
    s32 var_s4;
    s32 var_s0;
    s32 var_s2;
    s32 var_s1;
    s32 var_s3;
    s32 *count_slot;
    u32 var_14;
    s32 a0_arg;
    s32 *tbl = (s32 *)arg0;

    count = tbl[0];
    var_s3 = arg1;
    tail_off = tbl[count + 1];
    temp_a2 = tbl[2];
    /* FAKE: var_s7 built in two steps; `var_s7 = arg0 + tail_off` swaps
     * s7 / s8: score 6 (Q129) */
    var_s7 = arg0;
    var_s7 += tail_off;
    v0 = tbl[1];
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = tbl[3] - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8[0];
    var_s2 = D_800A33E8[1];
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            /* FAKE: a no-op copy (both are -1); the target's `move s0,s2`;
             * without it the test inverts (score 4). */
            var_s0 = var_s2;
        } else {
            var_s2 = 0;
        }
    } else if (var_s2 == -1) {
        var_s0 = 0;
    } else if (var_s0 == var_s2) {
        var_s0 = 0;
        var_s2 = 0;
    } else if (var_s0 < var_s2) {
        var_s0 = 0;
        var_s2 = 1;
    } else if (var_s2 < var_s0) {
        var_s0 = 1;
        var_s2 = 0;
    } else {
        func_80052C10();
    }

    /* the entry count's header slot is reserved before the entries are
     * written; the count is (var_s0 >= 0) + (var_s2 >= 0), its first term
     * built as ~var_s0 >> 31 (Ruling 4 split; one expression: score 2) */
    var_14 = ~var_s0;
    count_slot = (s32 *)var_s3;
    var_s3 += 4;
    var_14 >>= 31;
    if (var_s2 >= 0) {
        var_14 += 1;
    }
    *count_slot = var_14;

    if (var_s0 >= 0) {
        *(s32 *)var_s3 = 2;
        var_s3 += 4;
        if (var_s0 == 0) {
            mem_MoveBlockRoundedUpToWords(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            mem_MoveBlockRoundedUpToWords(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        arena_UpdatePeak(a0_arg);
        var_s1 += snd_LoadVabPack((Unk8005C2A8Pack *)var_s1, 2, var_s7);
    }

    if (var_s2 >= 0) {
        *(s32 *)var_s3 = 5;
        if (var_s2 == 0) {
            mem_MoveBlockRoundedUpToWords(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            mem_MoveBlockRoundedUpToWords(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        arena_UpdatePeak(a0_arg);
        var_s1 += snd_LoadVabPack((Unk8005C2A8Pack *)var_s1, 5, var_s7);
    }
    return var_s1;
}

void resource_InvalidateEntrySelectionCache(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_80099C50[0x39];
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800A324C = -1;
}

void node_InitRootNodePairWithNegatedRotationMatrices(void) {
    Unk80101DF0Record *p1 = &g_view_camera_root_node;
    Unk80101DF0Record *p2 = &g_view_identity_root_node;
    p1->unk0 = 0x64;
    g_view_camera_root_node.unk1 = 0;
    g_view_camera_root_node.xf.rot.vx = 0;
    g_view_camera_root_node.xf.rot.vy = 0;
    g_view_camera_root_node.xf.rot.vz = 0;
    g_view_camera_root_node.work.t[0] = 0;
    g_view_camera_root_node.work.t[1] = 0;
    g_view_camera_root_node.work.t[2] = 0;
    g_view_camera_root_node.parent_node = 0;
    g_view_camera_root_node.rotation_order = 5;
    node_SetMatrixFromNegatedRot(p1);
    p2->unk0 = 0x65;
    g_view_identity_root_node.unk1 = 0;
    g_view_identity_root_node.xf.rot.vx = 0;
    g_view_identity_root_node.xf.rot.vy = 0;
    g_view_identity_root_node.xf.rot.vz = 0;
    g_view_identity_root_node.work.t[0] = 0;
    g_view_identity_root_node.work.t[1] = 0;
    g_view_identity_root_node.work.t[2] = 0;
    g_view_identity_root_node.parent_node = 0;
    g_view_identity_root_node.rotation_order = 2;
    node_SetMatrixFromNegatedRot(p2);
    g_view_camera_root_ptr = p1;
    g_view_identity_root_ptr = p2;
}

/* 0x800153F0: the 22-halfword record light_Unpack unpacks. */
extern const Unk800153F0Record g_light_default_template;
extern void light_Unpack(Unk800F62E0Rec *, u16 *);

void light_InitDefaults(void) {
    Unk800153F0Record default_template_copy;
    s32 i;
    default_template_copy = g_light_default_template;
    for (i = 0; i < 8; i++) {
        light_Unpack(&g_light_setups[i], default_template_copy.v);
    }
    SetColorMatrix(&g_light_setups[0].cmat);
    SetBackColor(g_light_setups[0].back[0], g_light_setups[0].back[1],
                 g_light_setups[0].back[2]);
}

void light_Unpack(Unk800F62E0Rec *light_setup, u16 *packed_cursor) {
    light_setup->cmat.m[0][0] = *packed_cursor++;
    light_setup->cmat.m[1][0] = *packed_cursor++;
    light_setup->cmat.m[2][0] = *packed_cursor++;
    light_setup->cmat.m[0][1] = *packed_cursor++;
    light_setup->cmat.m[1][1] = *packed_cursor++;
    light_setup->cmat.m[2][1] = *packed_cursor++;
    light_setup->cmat.m[0][2] = *packed_cursor++;
    light_setup->cmat.m[1][2] = *packed_cursor++;
    {
        s16 v48 = *packed_cursor++;
        light_setup->lmat.m[0][0] = 0;
        light_setup->lmat.m[0][1] = 0;
        light_setup->lmat.m[0][2] = 0;
        light_setup->lmat.m[1][0] = 0;
        light_setup->lmat.m[1][1] = 0;
        light_setup->lmat.m[1][2] = 0;
        light_setup->lmat.m[2][0] = 0;
        light_setup->lmat.m[2][1] = 0;
        light_setup->lmat.m[2][2] = 0;
        light_setup->cmat.m[2][2] = v48;
    }
    light_setup->light[0].pitch = *packed_cursor++;
    light_setup->light[0].yaw = *packed_cursor++;
    light_setup->light[0].on = *packed_cursor++;
    light_setup->light[1].pitch = *packed_cursor++;
    light_setup->light[1].yaw = *packed_cursor++;
    light_setup->light[1].on = *packed_cursor++;
    light_setup->light[2].pitch = *packed_cursor++;
    light_setup->light[2].yaw = *packed_cursor++;
    light_setup->light[2].on = *packed_cursor++;
    calc_LightMatrix(light_setup);
    light_setup->back[0] = *packed_cursor++;
    light_setup->back[1] = *packed_cursor++;
    light_setup->back[2] = *packed_cursor;
    light_setup->unk5C = *(packed_cursor + 1);
}

void calc_LightMatrix(arg0) Unk800F62E0Rec *arg0;
{
    s16 i;
    Unk800F62E0Light *p;
    s16 c0;
    s32 t;

    i = 0;
    do {
        p = &arg0->light[i];
        if (p->on != 0) {
            c0 = rcos(p->pitch);
            t = ((rsin(p->yaw) * c0) >> 12) * arg0->unk5C;
            arg0->lmat.m[i][0] = -t >> 12;
            t = rsin(p->pitch) * arg0->unk5C;
            arg0->lmat.m[i][1] = t >> 12;
            t = ((rcos(p->yaw) * c0) >> 12) * arg0->unk5C;
            arg0->lmat.m[i][2] = -t >> 12;
        } else {
            arg0->lmat.m[i][0] = 0;
            arg0->lmat.m[i][1] = 0;
            arg0->lmat.m[i][2] = 0;
        }
        i++;
    } while (i < 3);
}

/* Q65: this file's initialized small data (.sdata), in address order. */
s16 D_800A3248 = -1;
s16 D_800A324A = -1;
s32 D_800A324C = -1;
/* Q65: tentative definitions (COMMON) of small data reached gp-relative. */
u32 *g_gpu_ot256_ptr;
