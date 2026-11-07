/* 4 game functions. .text 0x80040D48 (ROM 0x31548). Start boundary: PHASE
 * (rodata-align site 4). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"

extern s32 g_node_pose_rotation_indices[];

void slot_UpdateObjectFrame(
    s32 slot_index, s32 a1, s32 *position, s16 *rotation,
    s16 *decoded_motion_frame, s32 ground_y) {
    Unk80045878Obj *slot_object;
    Unk80045878Node *s5;
    Unk80045878Node *s3;
    Unk80045878Node *s2;
    s32 s0;
    s16 *s1;

    if (g_slot_objects[slot_index] == 0) {
        return;
    }
    slot_object = g_slot_objects[slot_index];

    slot_object->transform_nodes[0].node.xf.rot.vx = rotation[0];
    slot_object->transform_nodes[0].node.xf.rot.vy = rotation[1];
    slot_object->transform_nodes[0].node.xf.rot.vz = rotation[2];

    slot_object->transform_nodes[0].node.work.t[0] = position[0];
    /* FAKE: s5 holds slot_object + 0x2C here and the linked record pointer in
     * the copy loop below; a fresh loop local scores 35, dropping this first
     * role 46. */
    s5 = slot_object->transform_nodes;
    slot_object->transform_nodes[0].node.work.t[1] = position[1];
    s3 = &slot_object->transform_nodes[1];
    slot_object->transform_nodes[0].node.work.t[2] = position[2];

    s2 = &slot_object->transform_nodes[19];

    switch (a1) {
    case 0: {
        s32 *tbl;
        Unk80045878Node *p;
        /* s0 is the counter of all four loops in this function, reused loop
         * to loop the way SOTN's DebugCaptureScreen reuses its i (Q51) */
        /* SOTN: src/dra/42398.c:75 @aa53500 */
        s0 = 1;
        tbl = g_node_pose_rotation_indices;
        /* FAKE: s1 copies the parameter decoded_motion_frame; using
         * decoded_motion_frame directly scores 78. */
        s1 = decoded_motion_frame;

        s3->node.work.t[0] = 0;
        s3->node.work.t[1] = 0;
        s3->node.work.t[2] = 0;
        s3->node.xf.rot.vx = 0;
        s3->node.xf.rot.vy = 0;
        s3->node.xf.rot.vz = 0;

        do {
            Unk80045878Node *a4p;
            a4p = &s3[s0];
            a4p->node.xf.rot.vx = s1[*tbl * 3];
            a4p->node.xf.rot.vy = -s1[*tbl * 3 + 1];
            a4p->node.xf.rot.vz = -s1[*tbl * 3 + 2];
            s0++;
            tbl++;
        } while (s0 < 0x12);

        s0 = 0x11;
        p = &s3[17];
        do {
            p->node.matrix_state = NODE_MATRIX_REBUILD;
            s0--;
            p--;
        } while (s0 >= 0);

        s2->node.work.t[0] = s1[0x36];
        s2->node.work.t[1] = -s1[0x37];
        s2->node.work.t[2] = -s1[0x38];
        s2->node.xf.rot.vx = s1[0x39];
        s2->node.xf.rot.vy = -s1[0x3A];
        s2->node.xf.rot.vz = -s1[0x3B];

        g_math_rot_matrix_funcs[ROTATION_ORDER_ZYX](
            &s2->node.xf.rot, &s2->node.work);

        s2[1].node.work.t[0] = s1[0x3C];
        s2[1].node.work.t[1] = -s1[0x3D];
        s2[1].node.work.t[2] = -s1[0x3E];
        s2[1].node.xf.rot.vx = s1[0x3F];
        s2[1].node.xf.rot.vy = -s1[0x40];
        s2[1].node.xf.rot.vz = -s1[0x41];

        g_math_rot_matrix_funcs[ROTATION_ORDER_ZYX](
            &s2[1].node.xf.rot, &s2[1].node.work);
        break;
    }
    case 1:
        s3->node.xf.rot.vx = 0;
        s3->node.xf.rot.vy = 0;
        s3->node.xf.rot.vz = 0;
        s3->node.matrix_state = NODE_MATRIX_REBUILD;
        s3->node.work.t[0] = 0;
        s3->node.work.t[2] = 0;
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    }

    {
        Unk80045878Node *s1p;
        s0 = 0;
        s1p = s3;
        s3->node.work.t[1] =
            (s3->node.work.t[1] * slot_object->position_scale) >> 12;
        s5->node.matrix_state = NODE_MATRIX_REBUILD;
        do {
            node_UpdateWorldMatrix(&s1p->node);
            s0++;
            s1p++;
        } while (s0 < 0x12);
    }

    s0 = 1;
    s3->node.node_kind = 0xA;
    /* FAKE: unk58 stored through a pointer; the member store lets sched sink it
       below the g_draw_queue_cursor load (score 5). */
    {
        s32 *p58 = &s3->unk58;
        *p58 = (s32)slot_object->unk_18F4;
    }
    {
        void **list;
        Unk80045878Node *a4p;
        list = g_draw_queue_cursor;
        a4p = &s3[1];
        s3->node.reloc_entry_index = 0;
        g_draw_queue_cursor = list + 1;
        *list = s3;

        do {
            if (a4p->node.reloc_entry_index >= 0) {
                void **list2;
                list2 = g_draw_queue_cursor;
                g_draw_queue_cursor = list2 + 1;
                *list2 = a4p;
            }
            s0++;
            a4p++;
        } while (s0 < 0x12);
    }

    {
        Unk80045878Node *a2p;
        a2p = slot_object->duplicate_nodes;
        for (;;) {
            void **list3;
            s5 = (Unk80045878Node *)a2p->unk58;
            if (s5 == 0) {
                break;
            }
            a2p->node.xf.mat = s5->node.xf.mat;
            list3 = g_draw_queue_cursor;
            g_draw_queue_cursor = list3 + 1;
            *list3 = a2p;
            a2p++;
        }

        a2p = slot_object->projected_nodes;
        if (slot_object->projected_nodes[0].node.reloc_entry_index != -1) {
            do {
                void **list4;
                list4 = g_draw_queue_cursor;
                g_draw_queue_cursor = list4 + 1;
                *list4 = a2p;
                a2p++;
            } while (a2p->node.reloc_entry_index != -1);
        }
    }

    node_SetWordOnAllListedNodes(slot_object->projected_nodes, ground_y);
    slot_object->object_ground_y = ground_y;
    softbody_SetObjectGround(slot_object, ground_y);
    softbody_UpdateObject(slot_object);
    light_SetPointLightTarget(slot_index, (s32)s3->node.xf.mat.t);
}

extern s32 g_node_pose_rotation_indices[];

void slot_BlendTwoPosesIntoNodeMatrices(
    s32 slot_index, u8 *a1, u8 *a2, s32 interp_weight, MATRIX *a4) {
    s32 i = 1;
    s32 *tbl = g_node_pose_rotation_indices;
    Unk80045878Obj *slot_object = g_slot_objects[slot_index];
    SVECTOR buf;
    Unk80045878Node *ents;
    MATRIX *out2;
    s32 var_6;
    u16 *p;
    Unk80045878Node *var_9;
    ents = &slot_object->transform_nodes[1];
    do {
        var_6 = (*tbl) * 6;
        p = (u16 *)(var_6 + (s32)a1);
        buf.vx = p[0];
        buf.vy = -p[1];
        buf.vz = -p[2];
        math_RotMatrixZYX(&buf, a4);
        tbl++;
        /* FAKE: var_6 becomes the a2-side address (addu s0,s0,s2); a fresh sum
         * loads through v1 (score 4) */
        var_6 = var_6 + (s32)a2;
        p = (u16 *)var_6;
        buf.vx = p[0];
        buf.vy = -p[1];
        buf.vz = -p[2];
        math_RotMatrixZYX(&buf, a4 + 1);
        gte_LerpMatrixRowsAndRebuildOrthonormalBasis(
            a4, a4 + 1, interp_weight, &ents[i].node.work);
        ents[i].node.matrix_state = NODE_MATRIX_WORK_SET;
        i++;
    } while (i < 0x12);
    a1 += 0x6C;
    a2 += 0x6C;
    i = 0x12;
    var_9 = &ents[18];
    /* the second matrix, named for the interpolation loop; loop 1 passes
     * a4 + 1 inline (one name for both loops: score 4; inline in both: 10) */
    out2 = a4 + 1;
loop2:
    math_LerpShortVectorsNegateYZ(
        (s16 *)a1, (s16 *)a2, interp_weight, (s32)var_9->node.work.t);
    a1 += 6;
    a2 += 6;
    buf.vx = *((u16 *)a1);
    a1 += 2;
    buf.vy = -(*((u16 *)a1));
    a1 += 2;
    buf.vz = -(*((u16 *)a1));
    a1 += 2;
    math_RotMatrixZYX(&buf, a4);
    buf.vx = *((u16 *)a2);
    a2 += 2;
    buf.vy = -(*((u16 *)a2));
    a2 += 2;
    buf.vz = -(*((u16 *)a2));
    a2 += 2;
    math_RotMatrixZYX(&buf, out2);
    gte_LerpMatrixRowsAndRebuildOrthonormalBasis(
        a4, out2, interp_weight, &var_9->node.work);
    var_9->node.matrix_state = NODE_MATRIX_DONE;
    var_9++;
    i++;
    if (i < 0x14) {
        goto loop2;
    }
}

extern s32 *g_color_immediate_patch_sites[];
extern s32 func_800545F4;
extern s32 g_color_immediate_patch_first_ori_template;
extern s32 g_color_immediate_patch_second_lui_template;
extern s32 g_color_immediate_patch_second_ori_template;

void PatchColorImmediatesAtListedCodeSites(s32 color) {
    s32 **site_cursor = g_color_immediate_patch_sites;
    s32 t1 = 0;
    s32 mask8 = ~0xFF;
    s32 a1 = (color >> 16) & 0xFF;
    s32 mask16 = (s32)0xFFFF0000;
    s32 color_low_half = color & 0xFFFF;
    s32 first_lui_insn = (func_800545F4 & mask8) | a1;
    s32 first_ori_insn =
        (g_color_immediate_patch_first_ori_template & mask16) | color_low_half;
    s32 second_lui_insn =
        (g_color_immediate_patch_second_lui_template & mask8) | a1;
    s32 second_ori_insn =
        (g_color_immediate_patch_second_ori_template & mask16) | color_low_half;
    do {
        s32 *first_site = *site_cursor;
        site_cursor++;
        t1++;
        first_site[0] = first_lui_insn;
        first_site[1] = first_ori_insn;
        {
            s32 *second_site = *site_cursor;
            second_site[0] = second_lui_insn;
            second_site[1] = second_ori_insn;
        }
        site_cursor++;
    } while (t1 < 4);
}

void slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers(
    s32 slot_index, s32 byte_offset) {
    Unk80045878Obj **slot_entry;
    Unk80045878Obj *slot_object;
    s32 i;
    slot_entry = &g_slot_objects[slot_index];
    slot_object = (Unk80045878Obj *)((u8 *)*slot_entry + byte_offset);
    *slot_entry = slot_object;
    node_RebaseParents(byte_offset, slot_object->transform_nodes, 0x15);
    node_RebaseParents(byte_offset, slot_object->projected_nodes, 0x14);
    node_RebaseParents(byte_offset, slot_object->duplicate_nodes, 0x14);
    {
        Unk80045878Node *c = slot_object->duplicate_nodes;
        do {
            s32 val = c->unk58;
            if (val) {
                c->unk58 = val + byte_offset;
            } else {
                break;
            }
            c++;
        } while (1);
    }
    i = 0;
    do {
        Unk80045878Node *val = slot_object->node_by_id[i];
        if (val) {
            slot_object->node_by_id[i] =
                (Unk80045878Node *)((u8 *)val + byte_offset);
        }
        i++;
    } while (i < 0x14);
    node_RefreshObjectMatrixPointerTables(slot_object);
}

/* .data 0x80094CFC..0x80094D40, this file's whole .data
 * (g_object_scale_factors, read by 309CC.c and 9F9C.c, ends at 0x80094CFC;
 * 31D3C.c's g_gpu_asset_tim_group_load_masks follows): 17 node indices, a
 * permutation of 1..17, that slot_UpdateObjectFrame and
 * slot_BlendTwoPosesIntoNodeMatrices step through to index per-node rotation
 * triples. Defined after every function, so no body sees a definition where it
 * saw an extern. */
s32 g_node_pose_rotation_indices[17] = {
    8, 9, 14, 15, 16, 17, 10, 11, 12, 13, 1, 2, 3, 4, 5, 6, 7};
