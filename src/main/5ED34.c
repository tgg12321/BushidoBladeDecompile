/* InitDisplayStateLoadPackAndSetEntryFlags..gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer.
 * .text 0x8006E534 (ROM 0x5ED34). Start boundary: G8. Compiled -G8 (Q10, Q44,
 * Q54): the original bytes reach the small globals 0x800A32E8..0x800A35CA
 * straight off $gp, which the original compiler emits only at -G8, and no
 * function outside this range gp-accesses any of them. gpu_LoadVramBottomStrip,
 * gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip,
 * resource_RelocateNineOffsetListsAndLoadVabPack and gpu_AddPanelGridTile have
 * no gp access and compile identically at -G8 and -G0 (Q54). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
extern void AddPrim(void *, void *);

extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern RECT D_800A32EC;

/* Q65: this file's statics (.sbss, allocated per file by PSYLINK), in
 * address order. */
static s16 D_800A3530[2];
static s16 D_800A3534[2];
static s32 D_800A3538; /* not named by any code or data: size from the gap */
static s32 D_800A353C; /* not named by any code or data: size from the gap */
static s16 D_800A3540[2];
static s16 D_800A3544[2];
static s32 D_800A3548;
static s32 g_entry_menu_pressed_buttons;
static s16 g_entry_menu_darken_level;
static s16 g_entry_menu_grid_phase_extra_slot_count;
static s16 g_entry_menu_row_phase_extra_slot_count;
static s16 D_800A355C;
static Unk800A3560Slots g_entry_menu_slot_records;
static Unk8009BD24Block *g_entry_menu_working_record_ptr;
static s32 g_entry_menu_arena_cursor;
static s16 g_entry_menu_slide_displacement;
static s32 D_800A3574; /* not named by any code or data: size from the gap */
static s16 g_entry_menu_slide_phase;
static s16 D_800A357C;
static s16 g_entry_menu_handler_index;
static s16 g_entry_menu_next_handler_index;
static s16 g_entry_menu_grid_column_cursors[2];
static s16 g_entry_menu_grid_row_cursors[2];
static s16 g_entry_menu_row_phase_choice_cursors[2];
static s16 D_800A3594[2];
static s16 g_entry_menu_choice_grid_column;
static s16 g_entry_menu_choice_grid_row;
static s32 D_800A35A0;
static Unk8006E49CRec *D_800A35A4;
static Unk8006EA28Rec *g_entry_menu_resources;
static Unk8006E49CRec *D_800A35AC;
static s32 g_entry_menu_init_extra_slot_count;
static s16 D_800A35B4;
static s16 D_800A35B8;
static s32 g_entry_menu_fight_mode;
static GpuDb *g_entry_menu_gpu_db;
static Unk800A35C4Rec *g_entry_menu_work_block;
static s16 g_entry_menu_selection_offset_countdowns[2];

s32 InitDisplayStateLoadPackAndSetEntryFlags(
    s32 arg0, s32 arg1, Unk8009BD24Block *arg2, u32 arg3) {
    RECT rect;
    s16 i;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    D_800A32E8 = 0x7F;
    g_entry_menu_fight_mode = arg2->fight_mode;
    D_800A35AC = (Unk8006E49CRec *)arg0;
    g_entry_menu_working_record_ptr = arg2;
    g_entry_menu_row_phase_extra_slot_count = 0;
    g_entry_menu_grid_phase_extra_slot_count = 0;
    D_800A32E9 = 0;
    g_entry_menu_init_extra_slot_count = arg1;
    g_entry_menu_arena_cursor = arg0 + 0x58;
    g_entry_menu_resources = (Unk8006EA28Rec *)(arg0 + 0x58);
    resource_FreeSlotsAndBlocksAndInvalidateEntryCache();

    switch (g_entry_menu_fight_mode) {
    case 0:
        gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
            5, (Unk8006E950Head *)g_entry_menu_arena_cursor);
        break;
    case 4:
    case 6:
        gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
            3, (Unk8006E950Head *)g_entry_menu_arena_cursor);
        break;
    case 2:
        if (g_entry_menu_working_record_ptr->unk14_17) {
            gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
                3, (Unk8006E950Head *)g_entry_menu_arena_cursor);
        } else {
            gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
                4, (Unk8006E950Head *)g_entry_menu_arena_cursor);
        }
        break;
    case 1:
    case 3:
        gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
            4, (Unk8006E950Head *)g_entry_menu_arena_cursor);
        break;
    }

    g_entry_menu_arena_cursor = resource_RelocateNineOffsetListsAndLoadVabPack(
        (Unk8006EA28Rec *)g_entry_menu_arena_cursor);
    g_entry_menu_arena_cursor =
        gpu_InitPrimBufferPair(g_entry_menu_arena_cursor, D_800A35AC);
    g_entry_menu_slot_records.word = -1;
    g_entry_menu_slide_displacement = 0;
    g_entry_menu_slide_phase = 0;
    D_800A357C = 0;
    g_entry_menu_handler_index = 0;
    D_800A35A0 = 0;
    g_entry_menu_grid_column_cursors[0] = 0;
    g_entry_menu_grid_row_cursors[0] = 0;
    g_entry_menu_grid_column_cursors[1] = 2;
    g_entry_menu_grid_row_cursors[1] = 0;
    g_entry_menu_slot_records.rec[0].unk1 =
        g_entry_menu_grid_entries[0][0].value;
    g_entry_menu_slot_records.rec[1].unk1 =
        g_entry_menu_grid_entries[0][2].value;

    for (i = 0; i < 0x16; i++) {
        g_entry_menu_entry_flags[i] &= 0xFA;
        if (arg3 & (1 << i)) {
            g_entry_menu_entry_flags[i] |= 1;
        }
    }

    if (g_entry_menu_fight_mode < 4) {
        if (g_entry_menu_fight_mode >= 0) {
            g_entry_menu_entry_flags[g_entry_menu_grid_entries[4][1].value] |=
                4;
            g_entry_menu_entry_flags[g_entry_menu_grid_entries[4][3].value] |=
                4;
        }
    }
    D_800A35B8 = (arg3 >> 20) & 3;
    D_800A35B4 = 5;
    g_entry_menu_entry_flags
        [g_entry_menu_grid_entries[g_entry_menu_grid_row_cursors[0]]
                                  [g_entry_menu_grid_column_cursors[0]]
                                      .value] |= 4;
    if (g_entry_menu_init_extra_slot_count != 0) {
        g_entry_menu_entry_flags
            [g_entry_menu_grid_entries[g_entry_menu_grid_row_cursors[1]]
                                      [g_entry_menu_grid_column_cursors[1]]
                                          .value] |= 4;
    }

    rect = D_800A32EC;
    g_entry_menu_choice_grid_row = 0;
    g_entry_menu_choice_grid_column = 0;
    DrawSync(0);
    MoveImage(&rect, 0x3C0, 0x1FE);
    DrawSync(0);
    g_entry_menu_work_block = (Unk800A35C4Rec *)g_entry_menu_arena_cursor;
    g_entry_menu_arena_cursor += 0x14;
    g_entry_menu_work_block->animation_frame_counter = 0;
    g_entry_menu_work_block->prim_buffer_counter = 0;
    return 1;
}

Unk8006E49CRec *gpu_GetIndexedPrimBufferRecord(s32 buffer_index) {
    return &D_800A35AC[buffer_index];
}

void gpu_LoadVramBottomStrip(Unk8006E950Head *resource_root) {
    Unk8009BD24Block *p;
    s32 data;
    RECT rect;
    p = GetWorkingRecord();
    if (p->grayscale) {
        data = resource_root->unk_10;
    } else {
        data = resource_root->unk_0C;
    }
    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x280;
    rect.h = 0x20;
    DrawSync(0);
    LoadImage(&rect, (u32 *)data);
    DrawSync(0);
}

void gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
    s32 file_index_offset, Unk8006E950Head *resource_root) {
    s32 image_pixels_addr;
    RECT rect;

    cdrom_WaitIdle();
    cdrom_StartRead(
        cdrom_GetFileIndex(2, file_index_offset), (s32)resource_root);
    cdrom_WaitIdle();
    reloc_RelocateSelfOffsets((s32 *)resource_root);

    image_pixels_addr = resource_root->vram_image_pixels;

    rect.x = 0x280;
    rect.y = 0;
    rect.w = 0x180;
    rect.h = 0x1DC;
    DrawSync(0);
    LoadImage(&rect, (u32 *)image_pixels_addr);

    rect.w = 0x170;
    rect.x = 0x280;
    rect.y = 0x1DC;
    rect.h = 0x24;
    DrawSync(0);
    LoadImage(&rect, (u32 *)(image_pixels_addr + 0x59400));

    gpu_LoadVramBottomStrip(resource_root);
}

s32 resource_RelocateNineOffsetListsAndLoadVabPack(
    Unk8006EA28Rec *resource_root) {
    reloc_RelocateOffsetList(
        (s32 *)resource_root, (s32)resource_root->selected_entry_header_block);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_58);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_5C);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_60);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_64);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_68);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_6C);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_70);
    reloc_RelocateOffsetList((s32 *)resource_root, (s32)resource_root->unk_74);
    snd_LoadVabPack(resource_root->resource_head.sound_pack, 1,
                    resource_root->resource_head.free_ram_start);
    return resource_root->resource_head.free_ram_start;
}

void StepSlideProgressCommitPendingStateAndQueueSound(void);
void gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset(Unk8006EACCRec *);

s32 StepSlideBuildFrameRecordAndDispatchStateHandler(
    s32 arg0, s32 pad_buttons) {
    Unk8006EACCRec draw_context;
    Unk8006E49CRec *prim_buffer_record;
    s32 temp_v1;

    g_entry_menu_gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    D_800A3548 = arg0;
    g_entry_menu_pressed_buttons = pad_buttons;
    if (g_entry_menu_fight_mode == 2) {
        g_entry_menu_pressed_buttons = pad_buttons & 0xFFFF;
    }
    StepSlideProgressCommitPendingStateAndQueueSound();
    temp_v1 = g_entry_menu_work_block->prim_buffer_counter + 1;
    g_entry_menu_work_block->animation_frame_counter =
        g_entry_menu_work_block->animation_frame_counter + 1;
    g_entry_menu_work_block->prim_buffer_counter = temp_v1;
    prim_buffer_record = gpu_GetIndexedPrimBufferRecord(temp_v1 & 1);
    draw_context.resource_root.v8006EA28 = g_entry_menu_resources;
    draw_context.primitive_cursors.textured_quad_prims =
        prim_buffer_record->textured_quad_prims;
    draw_context.primitive_cursors.gradient_quad_prims =
        prim_buffer_record->gradient_quad_prims;
    draw_context.primitive_cursors.sprite_prims =
        prim_buffer_record->sprite_prims;
    draw_context.primitive_cursors.tile_prims = prim_buffer_record->tile_prims;
    draw_context.primitive_cursors.unk_14 = prim_buffer_record->unk_14;
    draw_context.primitive_cursors.draw_area_prims =
        prim_buffer_record->draw_area_prims;
    D_800A35A4 = prim_buffer_record;
    draw_context.primitive_cursors.draw_offset_prims =
        prim_buffer_record->draw_offset_prims;
    draw_context.unk_24 = prim_buffer_record->unk_20;
    if ((u16)(g_entry_menu_handler_index - 2) >= 2U) {
        gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset(&draw_context);
    }
    D_8009BC1C[g_entry_menu_handler_index](&draw_context);
    return D_800A35A0;
}

void StepSlideProgressCommitPendingStateAndQueueSound(void) {
    switch (g_entry_menu_slide_phase & 0xFF) {
    case 1:
    case 3:
        g_entry_menu_slide_displacement += 0x20;
        if (g_entry_menu_slide_displacement >= 0x1E8) {
            g_entry_menu_slide_displacement = 0x1E8;
            g_entry_menu_handler_index = g_entry_menu_next_handler_index;
            if ((g_entry_menu_slide_phase >> 8) == 0) {
                g_entry_menu_slide_phase++;
            }
        }
        break;
    case 2:
    case 4:
        if (g_entry_menu_slide_displacement == 0x1E8) {
            snd_QueueSound(5, 0x7F, 0x7F);
        }
        g_entry_menu_slide_displacement -= 0x20;
        if (g_entry_menu_slide_displacement <= 0) {
            g_entry_menu_slide_displacement = 0;
            g_entry_menu_slide_phase = 0;
        }
        break;
    }
}

extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern RECT D_800A32F4;

void gpu_UploadAndDrawSelectedEntryQuads(Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    Unk8006ECF4Rec *header_block;
    Unk8009B0E0Record *entry_sheet_headers;
    s32 entry_value;
    s32 upload_pixels_addr;
    s16 slot_index;
    RECT rectbuf;

    s.ot_idx = 0x14;
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;

    header_block =
        draw_context->resource_root.v8006EA28->selected_entry_header_block;
    entry_sheet_headers = header_block->unk_0C;

    for (slot_index = 0;
         slot_index < g_entry_menu_init_extra_slot_count + 1 +
                          g_entry_menu_grid_phase_extra_slot_count;
         slot_index++) {
        entry_value = g_entry_menu_grid_entries
                          [g_entry_menu_grid_row_cursors[slot_index]]
                          [g_entry_menu_grid_column_cursors[slot_index]]
                              .value;
        if (g_entry_menu_entry_flags[entry_value] & 1) {
            s.semi = 0;
            if (g_entry_menu_work_block->animation_frame_counter & 4) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            s.col_r = 0x94;
            s.col_g = 0x80;
            s.col_b = 0x6E;
        } else {
            s.semi = 1;
            s.has_color = 1;
            s.col_b = 0;
            s.col_g = 0;
            s.col_r = 0;
        }

        switch (entry_value) {
        case 12:
            upload_pixels_addr = g_entry_menu_resources->entry_upload_pixels[0];
            s.header = entry_sheet_headers + 22;
            break;
        case 13:
            upload_pixels_addr = g_entry_menu_resources->entry_upload_pixels[1];
            s.header = entry_sheet_headers + 23;
            break;
        case 14:
            upload_pixels_addr = g_entry_menu_resources->entry_upload_pixels[2];
            s.header = entry_sheet_headers + 24;
            break;
        case 0:
            upload_pixels_addr = g_entry_menu_resources->entry_upload_pixels[3];
            s.header = entry_sheet_headers + 25;
            break;
        case 3:
            upload_pixels_addr = g_entry_menu_resources->entry_upload_pixels[4];
            s.header = entry_sheet_headers + 26;
            break;
        default:
            s.header = entry_sheet_headers + entry_value;
            goto draw;
        }
        if (slot_index != 0) {
            if (D_800A32E8 != entry_value ||
                D_800A32E9 != g_entry_menu_grid_phase_extra_slot_count) {
                rectbuf = D_800A32F4;
                LoadImage(&rectbuf, (u32 *)upload_pixels_addr);
                DrawSync(0);
            }
        } else {
            s.header = entry_sheet_headers + entry_value;
        }
    draw:
        if (g_entry_menu_init_extra_slot_count == 0 &&
            !(g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[slot_index]
                                           .unk1] &
              2) &&
            (g_entry_menu_fight_mode != 2 ||
             !g_entry_menu_working_record_ptr->unk14_17)) {
            s.table = header_block->unk_00[1];
        } else {
            s.table = header_block->unk_00[slot_index];
        }
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        if (slot_index != 0) {
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddScaledSpriteQuads(&s, 1);
        } else {
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddScaledSpriteQuads(&s, 0);
        }
    }
}

void gpu_AddScreenDarken(Unk8006EACCRec *draw_context) {
    TILE *tile_cursor;
    s16 v1;
    s16 v2;
    s16 v3;

    tile_cursor = draw_context->primitive_cursors.tile_prims;
    SetTile(tile_cursor);
    v1 = g_entry_menu_darken_level;
    tile_cursor->x0 = 0;
    tile_cursor->y0 = 0;
    tile_cursor->r0 = v1;
    v2 = g_entry_menu_darken_level;
    tile_cursor->w = 0x280;
    tile_cursor->g0 = v2;
    v3 = g_entry_menu_darken_level;
    tile_cursor->h = 0xF0;
    tile_cursor->b0 = v3;
    SetSemiTrans(tile_cursor, 1);
    AddPrim(g_gpu_ot_ptr, tile_cursor);
    tile_cursor++;
    draw_context->primitive_cursors.tile_prims = tile_cursor;
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
}

extern void gpu_AddGradientPanel(Unk8006EACCRec *);
extern void gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(
    Unk8006EACCRec *);

void DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection(
    Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    s32 slot_index;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 header_index;
    s16 x_offset;
    s16 y_offset;
    s16 dx;
    s16 dy;

    if (g_entry_menu_darken_level != 0) {
        g_entry_menu_darken_level += 0x10;
        if (g_entry_menu_darken_level >= 0x100) {
            g_entry_menu_darken_level = 0xFF;
            gpu_AddScreenDarken(draw_context);
            g_entry_menu_darken_level = 0;
        } else {
            gpu_AddScreenDarken(draw_context);
        }
        gpu_UploadAndDrawSelectedEntryQuads(draw_context);
        gpu_AddGradientPanel(draw_context);
        gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(
            draw_context);
        return;
    }

    if (D_800A355C >= 0x79) {
        g_entry_menu_next_handler_index = 3;
    }
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.ot_idx = 0x13;
    s.semi = 0;
    s.has_color = 0;
    base = g_entry_menu_resources->unk_58;
    for (slot_index = 0;
         slot_index < 1 + g_entry_menu_init_extra_slot_count +
                          g_entry_menu_row_phase_extra_slot_count;
         slot_index++) {
        obj = base[g_entry_menu_slot_records.rec[slot_index].unk2];
        header_index = 1;
        if (g_entry_menu_fight_mode == 2 &&
            !(g_entry_menu_working_record_ptr->unk14_17) && slot_index == 1) {
            if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0]
                                             .unk1] &
                2) {
                header_index = 1;
            } else {
                header_index = 0;
            }
        } else if (
            g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[slot_index]
                                         .unk1] &
            2) {
            header_index = 0;
        }
        s.header = &obj[header_index];
        if (slot_index != 0) {
            s.header->ubase = 0x40;
        } else {
            s.header->ubase = 0;
        }
        s.table = obj[1].cells;
        x_offset =
            -(g_entry_menu_selection_offset_countdowns[slot_index] * 800) / 20;
        {
            s32 idx = s.header->count - 1;
            dx = s.table[idx].x + s.table[idx].w - obj[1].cells[0].x;
            dy = s.table[idx].y + s.table[idx].h - obj[1].cells[0].y;
        }
        y_offset =
            -(g_entry_menu_selection_offset_countdowns[slot_index] * 664) / 20;
        if (slot_index != 0) {
            x_offset = -x_offset;
        }
        s.x = (D_8009BC94[slot_index]
                         [g_entry_menu_row_phase_choice_cursors[slot_index]]
                             .x +
               0x140) +
              x_offset - ((dx * s.scale_x >> 8) / 2);
        s.y = (D_8009BC94[slot_index]
                         [g_entry_menu_row_phase_choice_cursors[slot_index]]
                             .y +
               0x9D) +
              y_offset - ((dy * s.scale_y >> 8) / 2);
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        if (slot_index != 0) {
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddRotatedSpriteQuads(&s, 0x1C0, 1);
        } else {
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddRotatedSpriteQuads(&s, 0xE40, 0);
        }
        if (g_entry_menu_selection_offset_countdowns[slot_index] > 0) {
            g_entry_menu_selection_offset_countdowns[slot_index]--;
        }
        if (g_entry_menu_selection_offset_countdowns[slot_index] == 10) {
            snd_QueueSound(7, 0x7F, 0x7F);
        }
        if (g_entry_menu_selection_offset_countdowns[slot_index] < 0) {
            g_entry_menu_selection_offset_countdowns[slot_index] = 0;
            snd_QueueSound(8, 0x7F, 0x7F);
        }
    }
    D_800A355C++;
}

void gpu_DrawMirroredQuadsWithSlideDrawAreaAndOffset(
    Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    RECT rect;
    Unk8009B0E0Record **sheet_headers;
    TILE *prim;
    s16 state;
    /* FAKE: the holder between the header's cells and s.table; stored directly
       at all five sites: score 26 (one site at a time: 13 / 3 / 4 / 3 / 3). */
    Unk8009B400Record *p1;

    s.semi = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.has_color = 0;

    sheet_headers = g_entry_menu_resources->unk_5C;
    s.ot_idx = 0x10;
    state = g_entry_menu_slide_phase & 0xFF;
    s.header = sheet_headers[0];
    p1 = s.header->cells;
    if (state >= 3) {
        s.x = g_entry_menu_slide_displacement;
    } else {
        s.x = 0;
    }
    s.table = p1;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    s.table++;
    s.scale_y = 0x4C00;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    s.scale_y = 0x100;
    s.header = sheet_headers[1];
    p1 = s.header->cells;
    s.table = p1;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    s.header = sheet_headers[0];
    p1 = s.header->cells;
    if (state < 3) {
        s.x = -g_entry_menu_slide_displacement + 0x200;
    } else {
        s.x = 0x200;
    }
    s.table = p1;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 1);

    s.table++;
    s.scale_y = 0x4C00;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    s.scale_y = 0x100;
    s.header = sheet_headers[1];
    p1 = s.header->cells;
    s.table = p1;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 1);

    if (state >= 3) {
        rect.x = g_entry_menu_slide_displacement + 0x4C;
    } else {
        rect.x = 0x4C;
    }
    rect.y = g_entry_menu_gpu_db->draw.clip.y + 0x7C;
    rect.w = 0x1E8 - g_entry_menu_slide_displacement;
    rect.h = 0x54;
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(
        g_gpu_ot_ptr + 0xF, draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;

    rect.x = g_entry_menu_gpu_db->draw.clip.x;
    rect.y = g_entry_menu_gpu_db->draw.clip.y;
    rect.w = g_entry_menu_gpu_db->draw.clip.w;
    rect.h = g_entry_menu_gpu_db->draw.clip.h;
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(g_gpu_ot_ptr + 6, draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;

    switch (state) {
    case 2:
    case 4:
        g_entry_menu_work_block->draw_offset[0] =
            g_entry_menu_gpu_db->draw.ofs[0] +
            (state == 2 ? -g_entry_menu_slide_displacement
                        : g_entry_menu_slide_displacement);
        g_entry_menu_work_block->draw_offset[1] =
            g_entry_menu_gpu_db->draw.ofs[1];
        SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims,
                      g_entry_menu_work_block->draw_offset);
        AddPrim(g_gpu_ot_ptr + 0xF,
                draw_context->primitive_cursors.draw_offset_prims);
        draw_context->primitive_cursors.draw_offset_prims++;

        g_entry_menu_work_block->draw_offset[0] =
            g_entry_menu_gpu_db->draw.ofs[0];
        g_entry_menu_work_block->draw_offset[1] =
            g_entry_menu_gpu_db->draw.ofs[1];
        SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims,
                      g_entry_menu_work_block->draw_offset);
        AddPrim(g_gpu_ot_ptr + 6,
                draw_context->primitive_cursors.draw_offset_prims);
        draw_context->primitive_cursors.draw_offset_prims++;
        break;
    }

    prim = draw_context->primitive_cursors.tile_prims;
    SetTile(prim);
    SetSemiTrans(prim, 0);
    if (g_entry_menu_working_record_ptr->grayscale) {
        prim->r0 = 0xC8;
        prim->g0 = 0xC8;
        prim->b0 = 0xC8;
    } else {
        prim->r0 = 0xD0;
        prim->g0 = 0xC8;
        prim->b0 = 0xB8;
    }
    prim->x0 = 0x4C;
    prim->y0 = 0x80;
    prim->w = 0x215;
    prim->h = 0x4C;
    AddPrim(g_gpu_ot_ptr + 0xE, prim);
    prim++;
    draw_context->primitive_cursors.tile_prims = prim;

    s.header = sheet_headers[2];
    p1 = s.header->cells;
    s.table = p1;
    s.scale_y = 0x100;
    s.x = 0;
    s.ot_idx = 0xE;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    s.y = 0x50;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 2);
}

extern void menu_UpdateEntryGridCursors(Unk8006EACCRec *);
extern void gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(
    Unk8006EACCRec *);

void menu_UpdateEntryGrid(Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    s16 shift[2];
    u16 tile_rect[4];
    Unk8009B0E0Record **var_3;
    /* the sprite sheet's cell array (8-byte Unk8009B400Record cells), which
       starts just past the sheet's 12-byte Unk8009B0E0Record headers: three
       on var_3[0] (normal, then one highlight per player, +0x24), one on every
       other sheet (+0xC). FAKE: the holder between the header's cells and
       s.table; stored directly: score 98. */
    Unk8009B400Record *var_4;
    s16 slot_index;
    s16 row;
    s16 col;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.semi = 0;
    s.col_r = s.col_g = s.col_b = 0x70;
    var_3 = g_entry_menu_resources->unk_60;
    s.header = var_3[0];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xD, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;

    s.ot_idx = 0xC;
    s.x = 0x82;
    s.y = 0x86;
    s.header = var_3[0];
    var_4 = s.header[2].cells;
    s.has_color = 0;
    for (slot_index = 0;
         slot_index < 1 + g_entry_menu_init_extra_slot_count +
                          g_entry_menu_grid_phase_extra_slot_count;
         slot_index++) {
        if (g_entry_menu_grid_column_cursors[slot_index] == 5) {
            s.header += 1 + slot_index;
            if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0) {
                s.x += (rsin((g_entry_menu_work_block->animation_frame_counter *
                              192) &
                             0xFC0) *
                        5) >>
                       12;
                s.y += (rcos((g_entry_menu_work_block->animation_frame_counter
                              << 7) &
                             0xF80) *
                        3) >>
                       12;
            } else {
                s.has_color = 1;
            }
            if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0x1E) {
                s.y += (g_entry_menu_work_block->entry_countdowns[slot_index] *
                        rsin((g_entry_menu_work_block->animation_frame_counter *
                              288) &
                             0xFE0)) >>
                       12;
            }
            break;
        }
    }
    s.table = var_4;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);

    s.x = 0x17E;
    s.y = 0x86;
    s.header = var_3[0];
    s.has_color = 0;
    for (slot_index = 0;
         slot_index < 1 + g_entry_menu_init_extra_slot_count +
                          g_entry_menu_grid_phase_extra_slot_count;
         slot_index++) {
        if (g_entry_menu_grid_column_cursors[slot_index] == 4) {
            s.header += 1 + slot_index;
            if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0) {
                s.x += (rsin((g_entry_menu_work_block->animation_frame_counter *
                              192) &
                             0xFC0) *
                        5) >>
                       12;
                s.y += (rcos((g_entry_menu_work_block->animation_frame_counter
                              << 7) &
                             0xF80) *
                        3) >>
                       12;
            } else {
                s.has_color = 1;
            }
            if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0x1E) {
                s.y += (g_entry_menu_work_block->entry_countdowns[slot_index] *
                        rsin((g_entry_menu_work_block->animation_frame_counter *
                              288) &
                             0xFE0)) >>
                       12;
            }
            break;
        }
    }
    s.table += s.header->count;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);

    s.x = 0;
    s.y = 0;
    s.header = var_3[1];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0xB, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 5; col++) {
            if ((g_entry_menu_fight_mode == 0 || g_entry_menu_fight_mode == 1 ||
                 g_entry_menu_fight_mode == 2 ||
                 g_entry_menu_fight_mode == 3) &&
                col == 4 && (row == 1 || row == 3)) {
                continue;
            }
            if (g_entry_menu_entry_flags[g_entry_menu_grid_entries[col][row]
                                             .value] &
                1) {
                s.y = 0;
                s.x = 0;
                s.has_color = 0;
                for (slot_index = 0;
                     slot_index < 1 + g_entry_menu_init_extra_slot_count +
                                      g_entry_menu_grid_phase_extra_slot_count;
                     slot_index++) {
                    if (g_entry_menu_slot_records.rec[slot_index].unk0 ==
                        0xFF) {
                        shift[slot_index] = 7;
                    } else {
                        shift[slot_index] = 9;
                    }
                    if (col == g_entry_menu_grid_row_cursors[slot_index] &&
                        row == g_entry_menu_grid_column_cursors[slot_index] &&
                        g_entry_menu_work_block->entry_countdowns[slot_index] !=
                            0) {
                        s.has_color = 1;
                        s.col_r = s.col_g = s.col_b =
                            ((rsin(((g_entry_menu_work_block
                                         ->animation_frame_counter &
                                     0x1F)
                                    << shift[slot_index]) +
                                   slot_index * 511) *
                              63) >>
                             12) -
                            0x40;
                        break;
                    }
                }
                s.header = var_3[g_entry_menu_grid_entries[col][row].value + 1];
                var_4 = s.header->cells;
                s.table = var_4;
                s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                s.ot_idx = 0xA;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(&s);
            } else {
                s.y = col << 4;
                s.x = row * 116 + (row >> 1) * 20;
                s.has_color = 0;
                s.header = var_3[21];
                var_4 = s.header->cells;
                s.table = var_4;
                s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                s.ot_idx = 0xA;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(&s);
            }
        }
    }

    s.y = 0;
    s.x = 0;
    s.has_color = 0;
    s.header = var_3[22];
    var_4 = s.header->cells;
    s.table = var_4;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    s.ot_idx = 1;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 1, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    tile_rect[2] = 0xF3;
    tile_rect[0] = 0xC6;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * gpu_AddThreeGreyTilesStackedUp reads +0x18 as its TILE cursor, and arg0's
     * +0x18 is the DR_MODE cursor primitive_cursors.unk_14. */
    gpu_AddThreeGreyTilesStackedUp((s32 *)draw_context, tile_rect, 1);
    menu_UpdateEntryGridCursors(draw_context);
    gpu_UploadAndDrawSelectedEntryQuads(draw_context);
    D_800A32E8 = g_entry_menu_slot_records.rec[1].unk1;
    D_800A32E9 = g_entry_menu_grid_phase_extra_slot_count;
    gpu_AddGradientPanel(draw_context);
    gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(draw_context);
}

void menu_UpdateEntryGridCursors(Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    Unk8009B0E0Record **sheet_headers;
    s16 *col;
    s16 *row;
    u8 *flags;
    s16 slot_index;
    s16 var_6;
    s16 var_7;

    s.semi = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    sheet_headers = g_entry_menu_resources->unk_74;
    s.header = sheet_headers[0];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 8, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    for (slot_index = 0;
         slot_index < 1 + g_entry_menu_init_extra_slot_count +
                          (var_7 = g_entry_menu_grid_phase_extra_slot_count);
         slot_index++) {
        col = &g_entry_menu_grid_column_cursors[slot_index];
        row = &g_entry_menu_grid_row_cursors[slot_index];
        flags = &g_entry_menu_entry_flags[g_entry_menu_grid_entries[*row][*col]
                                              .value];
        g_entry_menu_slot_records.rec[slot_index].unk1 =
            g_entry_menu_grid_entries[*row][*col].value;
        var_6 = slot_index - var_7;
        if (g_entry_menu_slot_records.rec[slot_index].unk0 == 0xFF) {
            g_entry_menu_work_block->entry_countdowns[slot_index] = 0x1E;
            if (g_entry_menu_pressed_buttons & (0x4000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)++;
                    if (*row >= 5) {
                        *row = 0;
                    }
                    flags = &g_entry_menu_entry_flags
                                [g_entry_menu_grid_entries[*row][*col].value];
                    g_entry_menu_slot_records.rec[slot_index].unk1 =
                        g_entry_menu_grid_entries[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            } else if (
                g_entry_menu_pressed_buttons & (0x1000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)--;
                    if (*row < 0) {
                        *row = 4;
                    }
                    flags = &g_entry_menu_entry_flags
                                [g_entry_menu_grid_entries[*row][*col].value];
                    g_entry_menu_slot_records.rec[slot_index].unk1 =
                        g_entry_menu_grid_entries[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            }
            if (g_entry_menu_pressed_buttons & (0x2000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)++;
                    if (*col > D_800A35B4) {
                        *col = 0;
                    }
                    flags = &g_entry_menu_entry_flags
                                [g_entry_menu_grid_entries[*row][*col].value];
                    g_entry_menu_slot_records.rec[slot_index].unk1 =
                        g_entry_menu_grid_entries[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            } else if (
                g_entry_menu_pressed_buttons & (0x8000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)--;
                    if (*col < 0) {
                        *col = D_800A35B4;
                    }
                    flags = &g_entry_menu_entry_flags
                                [g_entry_menu_grid_entries[*row][*col].value];
                    g_entry_menu_slot_records.rec[slot_index].unk1 =
                        g_entry_menu_grid_entries[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            }
            D_800A3530[slot_index] = 7;
            D_800A3534[slot_index] = 0;
        } else {
            D_800A3530[slot_index] = 9;
            if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0) {
                g_entry_menu_work_block->entry_countdowns[slot_index]--;
            }
            if (D_800A3534[slot_index] < 12) {
                D_800A3534[slot_index]++;
            }
            s.header = sheet_headers[3];
            s.header->count = D_800A3534[slot_index];
            s.table = s.header->cells;
            s.has_color = 0;
            s.x = *col * 116 + 0x6E + (*col >> 1) * 20;
            s.y = *row * 16 + 0x80;
            s.ot_idx = 7;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
        }
        if ((g_entry_menu_pressed_buttons & (0x40 << (var_6 * 16))) &&
            g_entry_menu_slot_records.rec[slot_index].unk0 == 0xFF) {
            if (*flags & 1) {
                g_entry_menu_slot_records.rec[slot_index].unk2 = 0xFF;
                g_entry_menu_row_phase_choice_cursors[slot_index] = 2;
                g_entry_menu_slot_records.rec[slot_index].unk0 =
                    g_entry_menu_grid_entries[*row][*col].unk1;
                snd_QueueSound(
                    g_entry_menu_grid_entries[*row][*col].value + 0xB, 0x7F,
                    0x7F);
                g_entry_menu_selection_offset_countdowns[0] = 0xF;
                g_entry_menu_selection_offset_countdowns[1] = 0x14;
                if (g_entry_menu_fight_mode == 2 &&
                    (g_entry_menu_working_record_ptr->unk14_17) &&
                    slot_index == 0) {
                    if (g_entry_menu_entry_flags
                            [g_entry_menu_slot_records.rec[0].unk1] &
                        2) {
                        g_entry_menu_grid_column_cursors[1] = 2;
                    } else {
                        g_entry_menu_grid_column_cursors[1] = 0;
                    }
                    g_entry_menu_grid_row_cursors[1] = 0;
                    if (g_entry_menu_work_block->entry_countdowns[0] == 0) {
                        g_entry_menu_slot_records.rec[1].unk1 =
                            g_entry_menu_grid_entries
                                [0][g_entry_menu_grid_column_cursors[1]]
                                    .value;
                        g_entry_menu_entry_flags
                            [g_entry_menu_grid_entries
                                 [0][g_entry_menu_grid_column_cursors[1]]
                                     .value] |= 4;
                    }
                    break;
                }
            } else {
                snd_QueueSound(0xA, 0x7F, 0x7F);
            }
        } else if (g_entry_menu_pressed_buttons & (0x10 << (var_6 * 16))) {
            if (g_entry_menu_slide_phase == 0) {
                snd_QueueSound(2, 0x7F, 0x7F);
                if (g_entry_menu_slot_records.rec[slot_index].unk0 == 0xFF &&
                    g_entry_menu_grid_phase_extra_slot_count == 0) {
                    D_800A35A0 = -1;
                } else if (g_entry_menu_grid_phase_extra_slot_count == 1) {
                    if (g_entry_menu_slot_records.rec[1].unk0 == 0xFF) {
                        g_entry_menu_grid_phase_extra_slot_count = 0;
                        g_entry_menu_slot_records.rec[0].unk0 = 0xFF;
                    } else {
                        g_entry_menu_slot_records.rec[1].unk0 = 0xFF;
                    }
                    *flags &= ~4;
                } else if (
                    g_entry_menu_slot_records.rec[slot_index].unk0 != 0xFF &&
                    (*flags & 1)) {
                    g_entry_menu_slot_records.rec[slot_index].unk0 = 0xFF;
                }
            }
        }
        if (g_entry_menu_work_block->entry_countdowns[slot_index] != 0) {
            s.has_color = 1;
        } else {
            s.has_color = 0;
        }
        if (g_entry_menu_grid_phase_extra_slot_count != 0 && slot_index != 0) {
            s.header = sheet_headers[slot_index + 1];
        } else {
            s.header = sheet_headers[slot_index];
        }
        s.table = s.header->cells;
        s.col_r = s.col_g = s.col_b =
            ((rsin(((g_entry_menu_work_block->animation_frame_counter & 0x1F)
                    << D_800A3530[slot_index]) +
                   slot_index * 511) *
              63) >>
             12) -
            0x40;
        s.x = *col * 116 + 0x4E + (*col >> 1) * 20;
        s.y = *row * 16 + 0x80;
        s.ot_idx = 7;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }
    if (g_entry_menu_handler_index == 0 &&
        g_entry_menu_slot_records.rec[0].unk0 != 0xFF &&
        g_entry_menu_work_block->entry_countdowns[0] == 0) {
        if (g_entry_menu_fight_mode == 2 &&
            (g_entry_menu_working_record_ptr->unk14_17)) {
            g_entry_menu_grid_phase_extra_slot_count = 1;
        }
        if ((g_entry_menu_slot_records.rec[1].unk0 != 0xFF &&
             g_entry_menu_work_block->entry_countdowns[1] == 0) ||
            g_entry_menu_init_extra_slot_count +
                    g_entry_menu_grid_phase_extra_slot_count ==
                0) {
            g_entry_menu_slide_phase = 1;
            g_entry_menu_row_phase_extra_slot_count = 0;
            if (g_entry_menu_fight_mode == 2) {
                g_entry_menu_slot_records.rec[1].unk2 = 0xFF;
                g_entry_menu_row_phase_choice_cursors[1] = 2;
            }
            g_entry_menu_next_handler_index = 1;
        }
    }
    snd_FlushSoundQueue();
}

extern void menu_UpdateRowCursors(Unk8006EACCRec *a0, Unk8007352CRec *s);
extern void gpu_UploadAndDrawSelectedEntryQuads(Unk8006EACCRec *);
extern void gpu_AddGradientPanel(Unk8006EACCRec *);
extern void gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(
    Unk8006EACCRec *);

typedef struct Unk80070C70Rec {
    s16 sp48;
    s16 sp4A;
    s16 sp4C;
    s16 sp4E;
} Unk80070C70Rec;

void menu_UpdateRow(Unk8006EACCRec *draw_context) {
    /* FAKE: constant-holder kept in s4 across calls; literal re-materializes
     * li a1,96: score 7. Inline tail helper (literal / parameter): 7 / 7;
     * gpu_GetSheetTPage(header, 0) + 0x60: 11; six-draw do loop: 7;
     * semi initialization moved to first use: 9. */
    s32 c60 = 0x60;
    Unk8007352CRec var_1;
    u16 tile_rect[4];
    Unk8009B0E0Record **sheet_headers;
    s32 var_s0;
    /* FAKE: the holder between the header's cells and var_1.table; stored
     * directly: score 11. */
    Unk8009B400Record *t;
    u8 code;

    var_1.semi = 0;
    var_1.x = 0;
    var_1.y = 0;
    var_1.scale_x = 0x100;
    var_1.scale_y = 0x100;
    var_1.has_color = 0;
    sheet_headers = g_entry_menu_resources->unk_64;
    /* Not a coercion: the target stores zero to both fields twice, before
     * and again after the var_3 load. */
    var_1.y = 0;
    var_1.x = 0;
    var_1.header = sheet_headers[1];
    t = var_1.header->cells;
    var_1.table = t;
    var_1.sprt_out = draw_context->primitive_cursors.sprite_prims;
    var_1.ot_idx = 1;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&var_1);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    tile_rect[2] = 0xE7;
    tile_rect[0] = 0xCC;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * gpu_AddThreeGreyTilesStackedUp reads +0x18 as its TILE cursor, and arg0's
     * +0x18 is the DR_MODE cursor primitive_cursors.unk_14. */
    gpu_AddThreeGreyTilesStackedUp((s32 *)draw_context, tile_rect, 1);
    var_1.header = sheet_headers[0];
    t = var_1.header[5].cells;
    var_1.table = t;
    for (var_s0 = 0; var_s0 < 6; var_s0++) {
        var_1.x = var_s0 << 6;
        var_1.sprt_out = draw_context->primitive_cursors.sprite_prims;
        var_1.ot_idx = 0xA;
        draw_context->primitive_cursors.sprite_prims =
            gpu_AddSpriteCells(&var_1);
        var_1.header++;
    }
    var_1.header = sheet_headers[0];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    var_1.header = sheet_headers[2];
    for (var_s0 = 0; var_s0 < 1 + g_entry_menu_init_extra_slot_count +
                                  g_entry_menu_row_phase_extra_slot_count;
         var_s0++) {
        code = g_entry_menu_slot_records.rec[var_s0].unk0;
        if ((code != 5) && (code != 16)) {
            t = var_1.header->cells;
            var_1.table = t;
            var_1.table += g_entry_menu_row_phase_choice_cursors[var_s0] * 2;
            if (((g_entry_menu_init_extra_slot_count +
                  g_entry_menu_row_phase_extra_slot_count) != 0) ||
                (g_entry_menu_fight_mode == 2)) {
                var_1.x = 0x50 + var_s0 * 0x16C;
            } else {
                var_1.x = 0x105;
            }
            var_1.sprt_out = draw_context->primitive_cursors.sprite_prims;
            var_1.ot_idx = 1;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&var_1);
        }
    }
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    menu_UpdateRowCursors(draw_context, &var_1);
    gpu_UploadAndDrawSelectedEntryQuads(draw_context);
    gpu_AddGradientPanel(draw_context);
    gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(draw_context);
}

void menu_UpdateRowCursors(Unk8006EACCRec *draw_context, Unk8007352CRec *s) {
    /* several values of one kind, a sprite-sheet header table:
     * g_entry_menu_resources->unk_74 (loaded at entry and again in the
     * selected-slot arm) and g_entry_menu_resources->unk_60 (the table the last
     * loop draws from) (Ruling 11, ordinary-c-judge-decidable). */
    Unk8009B0E0Record **var_0;
    s32 flag;
    /* the cell table following a sheet header (s->header->cells in the two
     * loop-2 draws, s->header[2].cells for the last loop). Ruling 11: several
     * values of that one kind (proof at tag pre-slim-2026-10-01). FAKE: the
     * holder between the header's cells and s->table at the two single-use
     * loop-2 sites; stored directly: score 9 (Q129). */
    Unk8009B400Record *cells;
    s16 i;
    s16 var_4;
    s16 var_5;

    flag = 0;
    var_0 = g_entry_menu_resources->unk_74;
    s->header = var_0[0];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s->header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 8, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    s->has_color = 1;
    s->scale_x = 0x100;
    s->scale_y = 0x100;
    for (i = 0; i < 1 + g_entry_menu_init_extra_slot_count +
                        g_entry_menu_row_phase_extra_slot_count;
         i++) {
        if (g_entry_menu_slide_phase == 0) {
            if (g_entry_menu_slot_records.rec[i].unk0 == 5) {
                g_entry_menu_slot_records.rec[i].unk2 = 6;
                flag = 1;
                g_entry_menu_work_block->row_countdowns[i] = 0;
            } else if (g_entry_menu_slot_records.rec[i].unk0 == 0x10) {
                g_entry_menu_slot_records.rec[i].unk2 = 7;
                flag = 1;
                g_entry_menu_work_block->row_countdowns[i] = 0;
            }
        }
    }
    for (i = 0; i < 1 + g_entry_menu_init_extra_slot_count +
                        (var_5 = g_entry_menu_row_phase_extra_slot_count);
         i++) {
        /* several values of one kind: player i's VRAM rect row,
         * g_entry_menu_resources->row_choice_upload_rects + i * 8, computed at
         * the top of the loop and again in the locked-slot arm (Ruling 11,
         * proof at tag pre-slim-2026-10-01). */
        RECT *var_6;

        /* FAKE: base and step as two statements, here and in the locked-slot
         * arm; `var_6 = g_entry_menu_resources->row_choice_upload_rects + i *
         * 8`: score 28 (Q129) */
        var_6 = g_entry_menu_resources->row_choice_upload_rects;
        var_6 += i * 8;
        var_4 = i - var_5;
        if (g_entry_menu_slot_records.rec[i].unk0 != 5 &&
            g_entry_menu_slot_records.rec[i].unk0 != 0x10) {
            s32 max;
            s32 min;

            if (g_entry_menu_fight_mode == 2 &&
                !(g_entry_menu_working_record_ptr->unk14_17) && i == 1) {
                if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0]
                                                 .unk1] &
                    2) {
                    max = 5;
                    min = 1;
                } else {
                    max = 4;
                    min = 0;
                }
            } else if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[i]
                                                    .unk1] &
                       2) {
                max = 4;
                min = 0;
            } else {
                max = 5;
                min = 1;
            }
            if (g_entry_menu_slot_records.rec[i].unk2 == 0xFF) {
                D_800A3544[i] = 7;
                D_800A3540[i] = 0;
                g_entry_menu_work_block->row_countdowns[i] = 0x1E;
                if ((g_entry_menu_pressed_buttons & (0x2000 << (var_4 * 16))) &&
                    g_entry_menu_row_phase_choice_cursors[i] < max) {
                    snd_QueueSound(0, 0x7F, 0x7F);
                    g_entry_menu_row_phase_choice_cursors[i]++;
                } else if (
                    (g_entry_menu_pressed_buttons & (0x8000 << (var_4 * 16))) &&
                    min < g_entry_menu_row_phase_choice_cursors[i]) {
                    snd_QueueSound(0, 0x7F, 0x7F);
                    g_entry_menu_row_phase_choice_cursors[i]--;
                }
                D_800A3594[i] = i;
                if (g_entry_menu_fight_mode == 3) {
                    s32 sel;
                    s32 id;
                    /* FAKE: named intermediate for the address of the image
                     * pointer LoadImage reads: under `*(...)` the sum expands
                     * as an address and the 0x14 moves into the load offset; as
                     * a local it computes id * 8 + 0x14 first, as the target
                     * does. Same at the confirm and locked sites. Written
                     * inline: score 27 (Q129). */
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
                    g_entry_menu_slot_records.rec[1].unk2 = 0;
                    g_entry_menu_slot_records.rec[0].unk2 = 0;
                    if (g_entry_menu_entry_flags
                            [g_entry_menu_slot_records.rec[i].unk1] &
                        2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = g_entry_menu_slot_records.rec[i].unk2;
                    tim = &g_entry_menu_resources->row_choice_pixels[id][sel];
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                } else if (
                    g_entry_menu_pressed_buttons & (0x40 << (var_4 * 16))) {
                    s32 sel;
                    s32 id;
                    /* FAKE: image pointer address, mechanism and score at the
                     * == 3 `tim` (Q129). */
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
                    g_entry_menu_slot_records.rec[i].unk2 =
                        D_8009BC38[g_entry_menu_row_phase_choice_cursors[i]];
                    sel = 1;
                    if (g_entry_menu_fight_mode == 2 &&
                        !(g_entry_menu_working_record_ptr->unk14_17) &&
                        i == 1) {
                        if (!(g_entry_menu_entry_flags
                                  [g_entry_menu_slot_records.rec[0].unk1] &
                              2)) {
                            sel = 0;
                        }
                    } else if (g_entry_menu_entry_flags
                                   [g_entry_menu_slot_records.rec[i].unk1] &
                               2) {
                        sel = 0;
                    }
                    id = g_entry_menu_slot_records.rec[i].unk2;
                    tim = &g_entry_menu_resources->row_choice_pixels[id][sel];
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                } else if (
                    g_entry_menu_pressed_buttons & (0x10 << (var_4 * 16))) {
                    snd_QueueSound(2, 0x7F, 0x7F);
                    if (g_entry_menu_row_phase_extra_slot_count == 1) {
                        g_entry_menu_row_phase_extra_slot_count = 0;
                        g_entry_menu_slot_records.rec[0].unk2 = 0xFF;
                    } else {
                        g_entry_menu_slot_records.rec[1].unk2 = 0xFF;
                        g_entry_menu_slot_records.rec[0].unk2 = 0xFF;
                        g_entry_menu_slide_phase = 3;
                        g_entry_menu_next_handler_index = 0;
                        snd_QueueSound(6, 0x7F, 0x7F);
                        if (g_entry_menu_grid_phase_extra_slot_count == 1) {
                            g_entry_menu_slot_records.rec[1].unk0 = 0xFF;
                        } else {
                            g_entry_menu_slot_records.rec[i].unk0 = 0xFF;
                        }
                        break;
                    }
                }
            } else {
                if ((g_entry_menu_pressed_buttons & (0x10 << (var_4 * 16))) &&
                    g_entry_menu_slide_phase == 0 &&
                    g_entry_menu_fight_mode != 3) {
                    snd_QueueSound(2, 0x7F, 0x7F);
                    g_entry_menu_selection_offset_countdowns[0] = 0xF;
                    g_entry_menu_selection_offset_countdowns[1] = 0x14;
                    g_entry_menu_slot_records.rec[i].unk2 = 0xFF;
                }
                D_800A3544[i] = 9;
                if (g_entry_menu_work_block->row_countdowns[i] != 0) {
                    g_entry_menu_work_block->row_countdowns[i]--;
                }
                /* FAKE: reused: re-stores the unk_74 table var_0 already
                 * holds; dropped: score 4; read in place, or a second local:
                 * 2. */
                var_0 = g_entry_menu_resources->unk_74;
                s->header = var_0[3];
                if (D_800A3540[i] < 12) {
                    D_800A3540[i]++;
                }
                s->header->count = D_800A3540[i];
                s->has_color = 0;
                cells = s->header->cells;
                s->table = cells;
                s->x = (g_entry_menu_row_phase_choice_cursors[i] << 6) + 0x85;
                s->y = 0xB6 - (D_800A3594[i] << 5);
                s->ot_idx = 7;
                s->sprt_out = draw_context->primitive_cursors.sprite_prims;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(s);
            }
            if (g_entry_menu_slide_phase != 3) {
                s->scale_x = 0x100;
                s->scale_y = 0x100;
                if (g_entry_menu_work_block->row_countdowns[i] != 0) {
                    s->has_color = 1;
                } else {
                    s->has_color = 0;
                }
                if (g_entry_menu_row_phase_extra_slot_count != 0 && i != 0) {
                    s->header = var_0[i + 1];
                } else {
                    s->header = var_0[i];
                }
                cells = s->header->cells;
                s->table = cells;
                s->col_r = s->col_g = s->col_b =
                    ((rsin(((g_entry_menu_work_block->animation_frame_counter &
                             0x1F)
                            << D_800A3544[i]) +
                           i * 511) *
                      63) >>
                     12) -
                    0x40;
                s->x = (g_entry_menu_row_phase_choice_cursors[i] << 6) + 0x80;
                s->y = 0xAC - (D_800A3594[i] << 5);
                s->ft4_out =
                    draw_context->primitive_cursors.textured_quad_prims;
                s->ot_idx = 7;
                draw_context->primitive_cursors.textured_quad_prims =
                    gpu_AddScaledSpriteQuads(s, 0);
            }
        } else {
            if (g_entry_menu_slide_phase == 0) {
                if (g_entry_menu_slot_records.rec[i == 0 ? 1 : 0].unk2 !=
                        0xFF &&
                    g_entry_menu_work_block->row_countdowns[i == 0 ? 1 : 0] ==
                        0) {
                    s32 sel;
                    s32 id;
                    /* FAKE: image pointer address, mechanism and score at the
                     * == 3 `tim` (Q129). */
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
                    flag = 2;
                    /* FAKE: reused: recomputes the rect row var_6 already
                     * holds; dropped: score 32; a block-scoped second local,
                     * or computing it only in the live arm: 12. */
                    var_6 = g_entry_menu_resources->row_choice_upload_rects;
                    var_6 += i * 8;
                    if (g_entry_menu_entry_flags
                            [g_entry_menu_slot_records.rec[i].unk1] &
                        2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = g_entry_menu_slot_records.rec[i].unk2;
                    tim = &g_entry_menu_resources->row_choice_pixels[id][sel];
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                }
            }
            if (g_entry_menu_pressed_buttons & (0x10 << (i * 16))) {
                snd_QueueSound(2, 0x7F, 0x7F);
                g_entry_menu_slide_phase = 3;
                snd_QueueSound(6, 0x7F, 0x7F);
                g_entry_menu_next_handler_index = 0;
                g_entry_menu_slot_records.rec[1].unk2 = 0xFF;
                g_entry_menu_slot_records.rec[0].unk2 = 0xFF;
                if (g_entry_menu_row_phase_extra_slot_count == 1) {
                    g_entry_menu_row_phase_extra_slot_count = 0;
                    g_entry_menu_slot_records.rec[1].unk0 = 0xFF;
                } else {
                    g_entry_menu_slot_records.rec[i].unk0 = 0xFF;
                }
                g_entry_menu_work_block->row_countdowns[i] = 0x1E;
            }
        }
    }
    var_0 = g_entry_menu_resources->unk_60;
    s->header = var_0[0];
    s->scale_x = 0x80;
    s->scale_y = 0x80;
    s->has_color = 0;
    s->ot_idx = 0xC;
    cells = s->header[2].cells;
    for (i = 0; i < 1 + g_entry_menu_init_extra_slot_count +
                        g_entry_menu_row_phase_extra_slot_count;
         i++) {
        if (g_entry_menu_slot_records.rec[i].unk0 != 5 &&
            g_entry_menu_slot_records.rec[i].unk0 != 0x10 &&
            g_entry_menu_slide_phase != 3) {
            s->table = cells;
            s->x = (g_entry_menu_row_phase_choice_cursors[i] << 6) + 0x80;
            s->y = 0xAC - (D_800A3594[i] << 5);
            if (g_entry_menu_fight_mode == 2 &&
                !(g_entry_menu_working_record_ptr->unk14_17) && i == 1) {
                if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0]
                                                 .unk1] &
                    2) {
                    s->table += s->header->count;
                }
            } else if (
                !(g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[i]
                                               .unk1] &
                  2)) {
                s->table += s->header->count;
            }
            s->ft4_out = draw_context->primitive_cursors.textured_quad_prims;
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddScaledSpriteQuads(s, 0);
        }
    }
    if (g_entry_menu_slot_records.rec[0].unk2 != 0xFF &&
        g_entry_menu_work_block->row_countdowns[0] == 0) {
        if (g_entry_menu_fight_mode == 2) {
            g_entry_menu_row_phase_extra_slot_count = 1;
        }
        if (((g_entry_menu_slot_records.rec[1].unk2 != 0xFF &&
              g_entry_menu_work_block->row_countdowns[1] == 0) ||
             g_entry_menu_init_extra_slot_count +
                     g_entry_menu_row_phase_extra_slot_count ==
                 0) &&
            flag != 1) {
            g_entry_menu_slide_phase = 0x101;
            g_entry_menu_next_handler_index = 2;
            g_entry_menu_darken_level = 1;
            D_800A355C = 0;
        }
    }
}

s32 func_80071C20(void) {
    s32 stage_index;
    stage_index = 3;
    if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0].unk1] & 2) {
        stage_index = 9;
    }
    return stage_index;
}

void DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections(
    Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    s32 slot_index;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 header_index;
    s16 dx;
    s16 dy;

    base = g_entry_menu_resources->unk_58;
    for (slot_index = 0;
         slot_index < 1 + g_entry_menu_init_extra_slot_count +
                          g_entry_menu_row_phase_extra_slot_count;
         slot_index++) {
        if (g_entry_menu_slot_records.rec[slot_index].unk0 != 5 &&
            g_entry_menu_slot_records.rec[slot_index].unk0 != 16) {
            s.ot_idx = 1;
            s.semi = 0;
            s.scale_x = 0x100;
            s.scale_y = 0x100;
            s.has_color = 0;
            obj = base[g_entry_menu_slot_records.rec[slot_index].unk2];
            header_index = 1;
            if (g_entry_menu_fight_mode == 2 &&
                !(g_entry_menu_working_record_ptr->unk14_17) &&
                slot_index == 1) {
                if (g_entry_menu_entry_flags[g_entry_menu_slot_records.rec[0]
                                                 .unk1] &
                    2) {
                    header_index = 1;
                } else {
                    header_index = 0;
                }
            } else if (g_entry_menu_entry_flags
                           [g_entry_menu_slot_records.rec[slot_index].unk1] &
                       2) {
                header_index = 0;
            }
            s.header = &obj[header_index];
            if (slot_index != 0) {
                s.header->ubase = 0x40;
            } else {
                s.header->ubase = 0;
            }
            s.table = obj[1].cells;
            {
                s32 idx = s.header->count - 1;
                dx = s.table[idx].x + s.table[idx].w - obj[1].cells[0].x;
                dy = s.table[idx].y + s.table[idx].h - obj[1].cells[0].y;
            }
            s.x = D_8009BC94[slot_index]
                            [g_entry_menu_row_phase_choice_cursors[slot_index]]
                                .x +
                  0x140 - ((dx * s.scale_x >> 8) / 2);
            s.y = D_8009BC94[slot_index]
                            [g_entry_menu_row_phase_choice_cursors[slot_index]]
                                .y +
                  0x9D - ((dy * s.scale_y >> 8) / 2);
            s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
            if (slot_index != 0) {
                draw_context->primitive_cursors.textured_quad_prims =
                    gpu_AddRotatedSpriteQuads(&s, 0x1C0, 1);
            } else {
                draw_context->primitive_cursors.textured_quad_prims =
                    gpu_AddRotatedSpriteQuads(&s, 0xE40, 0);
            }
        }
    }

    g_entry_menu_darken_level += 8;
    if (g_entry_menu_darken_level >= 0xFF) {
        g_entry_menu_darken_level = 0xFF;
        g_entry_menu_slide_phase = 1;
        snd_QueueSound(6, 0x7F, 0x7F);
        if ((u32)g_entry_menu_fight_mode < 2) {
            g_entry_menu_working_record_ptr->stage_index = func_80071C20();
            D_800A35A0 = 1;
        } else if (g_entry_menu_fight_mode == 4) {
            g_entry_menu_next_handler_index = 5;
        } else if (g_entry_menu_fight_mode == 6) {
            g_entry_menu_next_handler_index = 6;
            g_entry_menu_choice_grid_row = 1;
            g_entry_menu_choice_grid_column = 1;
        } else {
            D_800A35A0 = 1;
        }
        for (slot_index = 0;
             slot_index < 1 + g_entry_menu_init_extra_slot_count +
                              g_entry_menu_grid_phase_extra_slot_count;
             slot_index++) {
            g_entry_menu_working_record_ptr->match_picks[slot_index][0].bd0 =
                g_entry_menu_slot_records.rec[slot_index].unk0;
        }
        for (slot_index = 0;
             slot_index < 1 + g_entry_menu_init_extra_slot_count +
                              g_entry_menu_row_phase_extra_slot_count;
             slot_index++) {
            g_entry_menu_working_record_ptr->match_picks[slot_index][0].unk1 =
                g_entry_menu_slot_records.rec[slot_index].unk2;
        }
    }
    gpu_AddScreenDarken(draw_context);
}

void menu_UpdateChoiceGrid(Unk8006EACCRec *, Unk8009B0E0Record **, s32);

void func_80072084(Unk8006EACCRec *draw_context) {
    menu_UpdateChoiceGrid(draw_context, g_entry_menu_resources->unk_68, 0);
}

void func_800720AC(Unk8006EACCRec *draw_context) {
    menu_UpdateChoiceGrid(draw_context, g_entry_menu_resources->unk_6C, 1);
}

void func_800720D4(Unk8006EACCRec *draw_context) {
    menu_UpdateChoiceGrid(draw_context, g_entry_menu_resources->unk_70, 2);
}

void menu_UpdateChoiceGrid(
    Unk8006EACCRec *draw_context, Unk8009B0E0Record **sheet_headers, s32 mode) {
    Unk8007352CRec s;
    u16 tile_rect[4];
    RECT rect;
    u8 *choice_table;
    Unk8009B0E0Record **var_4;
    /* the cell table following the header(s) of each sheet drawn
     * (s.header[1].cells for the two-header sheet, s.header->cells for the
     * others). Ruling 11: several values of that one kind. FAKE: the holder
     * between the header's cells and s.table at the single-use sites (all
     * but the first value, read twice); stored directly: score 123 (Q129). */
    Unk8009B400Record *cells;
    s32 i;
    s32 column_index;
    s32 d;
    u8 code;
    u8 chosen_stage_index;

    choice_table = g_entry_menu_resources->unk_80;
    s.semi = 0;
    s.has_color = 0;
    s.ot_idx = 0x11;
    rect.x = g_entry_menu_gpu_db->draw.clip.x;
    rect.y = g_entry_menu_gpu_db->draw.clip.y;
    rect.w = g_entry_menu_gpu_db->draw.clip.w;
    rect.h = g_entry_menu_gpu_db->draw.clip.h;
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx,
            draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;
    g_entry_menu_work_block->draw_offset[0] = g_entry_menu_gpu_db->draw.ofs[0];
    g_entry_menu_work_block->draw_offset[1] = g_entry_menu_gpu_db->draw.ofs[1];
    SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims,
                  g_entry_menu_work_block->draw_offset);
    AddPrim(g_gpu_ot_ptr + s.ot_idx,
            draw_context->primitive_cursors.draw_offset_prims);
    draw_context->primitive_cursors.draw_offset_prims++;

    if ((g_entry_menu_slide_phase & 0xFF) == 0) {
        s.x = -10;
        s.y = -5;
        var_4 = g_entry_menu_resources->unk_74;
        s.header = var_4[4];
        cells = s.header[1].cells;
        if (!(g_entry_menu_choice_grid_row * 2 +
                      g_entry_menu_choice_grid_column ==
                  3 &&
              g_entry_menu_fight_mode == 6 && mode == 2)) {
            s.table = cells;
            s.table += (g_entry_menu_choice_grid_row + mode * 3) * 2 +
                       g_entry_menu_choice_grid_column;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
        }
        s.header++;
        for (i = 0; i < 6; i++) {
            if (!(i == 3 && g_entry_menu_fight_mode == 6 && mode == 2) &&
                g_entry_menu_choice_grid_row * 2 +
                        g_entry_menu_choice_grid_column !=
                    i) {
                s.table = cells + (mode * 6 + i);
                s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(&s);
            }
        }
        s.header = var_4[4];
        SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, 0), 0);
        AddPrim(
            g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
        draw_context->primitive_cursors.unk_14++;
    }

    s.x = 0x90;
    s.y = 0x28;
    s.header = sheet_headers[0];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    g_entry_menu_work_block->draw_offset[0] =
        g_entry_menu_gpu_db->draw.ofs[0] - D_8009BCC4[mode][0];
    g_entry_menu_work_block->draw_offset[1] =
        g_entry_menu_gpu_db->draw.ofs[1] - D_8009BCC4[mode][1];
    if (((g_entry_menu_slide_phase & 0xFF) == 1 ||
         (g_entry_menu_slide_phase & 0xFF) == 3) &&
        g_entry_menu_next_handler_index >= 4 &&
        g_entry_menu_next_handler_index < 7 &&
        g_entry_menu_handler_index >= 4 && g_entry_menu_handler_index < 7) {
        for (i = 0; i < 2; i++) {
            d = D_8009BCC4[g_entry_menu_next_handler_index - 4][i] -
                D_8009BCC4[g_entry_menu_handler_index - 4][i];
            d *= 30;
            if ((d >= 0 ? d : -d) >
                (g_entry_menu_transition_offsets[i] >= 0
                     ? g_entry_menu_transition_offsets[i]
                     : -g_entry_menu_transition_offsets[i])) {
                g_entry_menu_transition_offsets[i] += d * 32 / 488;
            } else {
                g_entry_menu_transition_offsets[i] = d;
            }
            g_entry_menu_work_block->draw_offset[i] -=
                g_entry_menu_transition_offsets[i] / 30;
        }
    } else {
        g_entry_menu_transition_offsets[1] = 0;
        g_entry_menu_transition_offsets[0] = 0;
    }
    SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims,
                  g_entry_menu_work_block->draw_offset);
    AddPrim(g_gpu_ot_ptr + s.ot_idx,
            draw_context->primitive_cursors.draw_offset_prims);
    draw_context->primitive_cursors.draw_offset_prims++;
    rect.x = s.x;
    rect.y = g_entry_menu_gpu_db->draw.clip.y + s.y;
    rect.w = 0x160;
    rect.h = 0x4A;
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx,
            draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xB;
    s.header = sheet_headers[1];
    s.scale_x += 0x80;
    s.scale_y += 0x20;
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);
    s.table += s.header->count;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 1);

    if (g_entry_menu_pressed_buttons & 0xA000A000) {
        if ((g_entry_menu_slide_phase & 0xFF) == 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (((g_entry_menu_pressed_buttons & 0x20002000) &&
                 g_entry_menu_choice_grid_column != 0) ||
                ((g_entry_menu_pressed_buttons & 0x80008000) &&
                 g_entry_menu_choice_grid_column == 0)) {
                code = choice_table[(g_entry_menu_handler_index - 4) * 8 +
                                    3 * 2 + g_entry_menu_choice_grid_column];
                g_entry_menu_next_handler_index = code & 0xF;
                if (g_entry_menu_next_handler_index != 0xF) {
                    g_entry_menu_slide_phase = code >> 4;
                    snd_QueueSound(6, 0x7F, 0x7F);
                }
            }
            g_entry_menu_choice_grid_column =
                g_entry_menu_choice_grid_column == 0;
        }
    }
    if (g_entry_menu_pressed_buttons & 0x40004000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        g_entry_menu_choice_grid_row++;
    } else if (g_entry_menu_pressed_buttons & 0x10001000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        g_entry_menu_choice_grid_row--;
    }
    if (g_entry_menu_choice_grid_row >= 3) {
        g_entry_menu_choice_grid_row = 0;
    } else if (g_entry_menu_choice_grid_row < 0) {
        g_entry_menu_choice_grid_row = 2;
    }

    s.col_r = s.col_g = s.col_b =
        ((rsin((g_entry_menu_work_block->animation_frame_counter & 0x1F) * 128 +
               0x1FF) *
          63) >>
         12) -
        0x40;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 1;
    s.header = sheet_headers[2];
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);
    tile_rect[2] = 0x111;
    tile_rect[0] = 0xB7;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * gpu_AddThreeGreyTilesStackedUp reads +0x18 as its TILE cursor, and arg0's
     * +0x18 is the DR_MODE cursor primitive_cursors.unk_14. */
    gpu_AddThreeGreyTilesStackedUp((s32 *)draw_context, tile_rect, 1);
    s.ot_idx = 0xA;
    for (i = 0; i < 4; i++) {
        for (column_index = 0; column_index < 2; column_index++) {
            if (i == g_entry_menu_choice_grid_row &&
                column_index == g_entry_menu_choice_grid_column &&
                (g_entry_menu_slide_phase & 0xFF) == 0) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            if (i + i + column_index != 3 || g_entry_menu_fight_mode != 6 ||
                mode != 2) {
                s.header = (sheet_headers + (i + i))[column_index + 3];
            } else {
                s.header = sheet_headers[12];
            }
            cells = s.header->cells;
            s.table = cells;
            s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
            draw_context->primitive_cursors.textured_quad_prims =
                gpu_AddScaledSpriteQuads(&s, 0);
        }
    }
    s.has_color = 0;
    s.header = sheet_headers[11];
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
    draw_context->primitive_cursors.textured_quad_prims =
        gpu_AddScaledSpriteQuads(&s, 0);

    if (g_entry_menu_slide_phase == 0) {
        for (i = 0; i < g_entry_menu_init_extra_slot_count + 1; i++) {
            if (g_entry_menu_pressed_buttons & (0x10 << (i * 16))) {
                s32 var_11;

                snd_QueueSound(2, 0x7F, 0x7F);
                var_11 = g_entry_menu_slot_records.rec[i].unk0;
                g_entry_menu_slot_records.rec[i].unk2 = 0xFF;
                if (var_11 == 5 || var_11 == 16) {
                    g_entry_menu_handler_index = 0;
                    g_entry_menu_slot_records.rec[i == 0 ? 1 : 0].unk2 = 0xFF;
                    g_entry_menu_slot_records.rec[i].unk0 = 0xFF;
                } else {
                    g_entry_menu_handler_index = 1;
                }
                g_entry_menu_selection_offset_countdowns[0] = 0xF;
                g_entry_menu_selection_offset_countdowns[1] = 0x14;
                g_entry_menu_work_block->row_countdowns[1] = 0;
                g_entry_menu_work_block->row_countdowns[0] = 0;
                goto end;
            }
        }
        if (g_entry_menu_pressed_buttons & 0x400040) {
            chosen_stage_index =
                choice_table[(g_entry_menu_handler_index - 4) * 8 +
                             g_entry_menu_choice_grid_row * 2 +
                             g_entry_menu_choice_grid_column];
            snd_QueueSound(1, 0x7F, 0x7F);
            if (chosen_stage_index != 0xD || g_entry_menu_fight_mode != 6) {
                g_entry_menu_working_record_ptr->stage_index =
                    chosen_stage_index;
            } else {
                g_entry_menu_working_record_ptr->stage_index = 0x25;
            }
            D_800A35A0 = 1;
        }
    }
end:
    gpu_AddGradientPanel(draw_context);
    gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(draw_context);
    snd_FlushSoundQueue();
}

POLY_G4 *gpu_AddPanelUpperQuad(s32 handler_index, POLY_G4 *gradient_quad) {
    SetPolyG4(gradient_quad);
    SetSemiTrans(gradient_quad, 0);
    if (handler_index < 4) {
        gradient_quad->r0 = 0;
        gradient_quad->g0 = 0;
        gradient_quad->b0 = 0;
        gradient_quad->r1 = 0xFC;
        gradient_quad->g1 = 0x82;
        gradient_quad->b1 = 0;
        gradient_quad->r2 = 0xFC;
        gradient_quad->g2 = 0x82;
        gradient_quad->b2 = 0;
        if (g_entry_menu_work_block->animation_frame_counter & 4) {
            gradient_quad->r3 = 0xFC;
            gradient_quad->g3 = 0xC3;
            gradient_quad->b3 = 0x1E;
        } else {
            gradient_quad->r3 = 0xFC;
            gradient_quad->g3 = 0xC3;
            gradient_quad->b3 = 0x50;
        }
    } else {
        gradient_quad->r0 = 0;
        gradient_quad->g0 = 0;
        gradient_quad->b0 = 0;
        gradient_quad->r1 = 0x40;
        gradient_quad->g1 = 0;
        gradient_quad->b1 = 0x80;
        gradient_quad->r2 = 0x50;
        gradient_quad->g2 = 0xA0;
        gradient_quad->b2 = 0x40;
        gradient_quad->r3 = 0x10;
        gradient_quad->g3 = 0x40;
        gradient_quad->b3 = 0x80;
    }
    AddPrim(g_gpu_ot_ptr + 0x18, gradient_quad);
    return gradient_quad + 1;
}

/* gpu_AddPanelLowerQuad - colours a Gouraud quad (POLY_G4) by mode: warm
 * per-vertex RGB triples (two variants on g_entry_menu_work_block's flag 4) for
 * modes < 4, a fixed dark set otherwise; then adds it to the OT and returns the
 * next primitive slot. */
POLY_G4 *gpu_AddPanelLowerQuad(s32 handler_index, POLY_G4 *gradient_quad) {
    SetPolyG4(gradient_quad);
    SetSemiTrans(gradient_quad, 0);
    if (handler_index < 4) {
        if (g_entry_menu_work_block->animation_frame_counter & 4) {
            gradient_quad->r0 = 0xFC;
            gradient_quad->g0 = 0xC3;
            gradient_quad->b0 = 0x1E;
            gradient_quad->r1 = 0xFC;
            gradient_quad->g1 = 0xC8;
            gradient_quad->b1 = 0x32;
        } else {
            gradient_quad->r0 = 0xFC;
            gradient_quad->g0 = 0xC3;
            gradient_quad->b0 = 0x50;
            gradient_quad->r1 = 0xFC;
            gradient_quad->g1 = 0xDC;
            gradient_quad->b1 = 0x46;
        }
        gradient_quad->r2 = 0xFC;
        gradient_quad->g2 = 0x82;
        gradient_quad->r3 = 0x32;
        gradient_quad->g3 = 0x28;
        gradient_quad->b2 = 0;
        gradient_quad->b3 = 0xA;
    } else {
        gradient_quad->r0 = 0x10;
        gradient_quad->g0 = 0x30;
        gradient_quad->b0 = 0x60;
        gradient_quad->r1 = 0x18;
        gradient_quad->g1 = 0;
        gradient_quad->b1 = 0x40;
        gradient_quad->r2 = 0x30;
        gradient_quad->g2 = 0;
        gradient_quad->b2 = 0x60;
        gradient_quad->r3 = 0;
        gradient_quad->g3 = 0;
        gradient_quad->b3 = 0;
    }
    AddPrim(g_gpu_ot_ptr + 0x18, gradient_quad);
    return gradient_quad + 1;
}

void gpu_AddGradientPanel(Unk8006EACCRec *draw_context) {
    POLY_G4 *p;
    gpu_AddPanelGrid(draw_context);
    p = draw_context->primitive_cursors.gradient_quad_prims;
    p->x0 = 0x50;
    p->y0 = 0x32;
    p->x1 = 0x50;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = gpu_AddPanelUpperQuad(g_entry_menu_handler_index, p);
    p->x0 = 0x231;
    p->y0 = 0x32;
    p->x1 = 0x231;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = gpu_AddPanelUpperQuad(g_entry_menu_handler_index, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x50;
    p->y2 = 0x52;
    p->x3 = 0x50;
    p->y3 = 0x71;
    p = gpu_AddPanelLowerQuad(g_entry_menu_handler_index, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x231;
    p->y2 = 0x52;
    p->x3 = 0x231;
    p->y3 = 0x71;
    p = gpu_AddPanelLowerQuad(g_entry_menu_handler_index, p);
    draw_context->primitive_cursors.gradient_quad_prims = p;
}

TILE *gpu_AddPanelGridTile(s32 handler_index, TILE *tile) {
    SetTile(tile);
    if (handler_index < 4) {
        tile->r0 = 0x9E;
        tile->g0 = 0x64;
        tile->b0 = 0;
        SetSemiTrans(tile, 1);
    } else {
        tile->r0 = 0x28;
        tile->g0 = 0x28;
        tile->b0 = 0x18;
        SetSemiTrans(tile, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x17, tile);
    return tile + 1;
}

TILE *gpu_AddBrownOrBlackPanelGridTile(s32 arg0, TILE *tile) {
    SetTile(tile);
    if (g_entry_menu_handler_index < 4) {
        tile->r0 = 0x46;
        tile->g0 = 0x24;
        tile->b0 = 0x0A;
        SetSemiTrans(tile, 1);
    } else {
        tile->r0 = 0;
        tile->g0 = 0;
        tile->b0 = 0;
        SetSemiTrans(tile, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x17, tile);
    return tile + 1;
}

void gpu_AddPanelGrid(Unk8006EACCRec *draw_context) {
    TILE *p;
    s32 i;
    p = draw_context->primitive_cursors.tile_prims;
    for (i = 0x6F; i < 0x210; i += 0x20) {
        p->x0 = i;
        p->y0 = 0x32;
        p->w = 2;
        p->h = 0x3F;
        p = gpu_AddPanelGridTile(g_entry_menu_handler_index, p);
    }
    p->x0 = 0x51;
    p->y0 = 0x41;
    p->w = 0x1E0;
    p->h = 1;
    p = gpu_AddPanelGridTile(g_entry_menu_handler_index, p);
    p->x0 = 0x51;
    p->y0 = 0x61;
    p->w = 0x1E0;
    p->h = 1;
    p = gpu_AddPanelGridTile(g_entry_menu_handler_index, p);
    p->x0 = 0x51;
    p->y0 = 0x50;
    p->w = 0x1E0;
    p->h = 2;
    p = gpu_AddPanelGridTile(g_entry_menu_handler_index, p);
    for (i = 0; i < 5; i++) {
        p->x0 = 0x6A + i * 0x21;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = gpu_AddBrownOrBlackPanelGridTile(g_entry_menu_handler_index, p);
    }
    for (i = 0; i < 5; i++) {
        p->x0 = 0x211 - i * 0x20;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = gpu_AddBrownOrBlackPanelGridTile(g_entry_menu_handler_index, p);
    }
    draw_context->primitive_cursors.tile_prims = p;
}

/* gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer - draws three
 * sprite-sheet layers: in modes g_entry_menu_handler_index < 4, four cells of
 * var_1[4]'s sheet through gpu_AddScaledSpriteQuads; then var_1[5]'s sheet and,
 * for g_entry_menu_handler_index < 2, the var_1[10..13] sheet picked by the
 * frame counter at g_entry_menu_work_block->animation_frame_counter
 * (incremented in StepSlideBuildFrameRecordAndDispatchStateHandler), each
 * through gpu_AddSpriteCells, with a draw-mode primitive per layer. */
void gpu_DrawTintedFlippedQuadsFixedCellLayerAndFrameCycledLayer(
    Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    Unk8009B0E0Record **sheet_headers;
    /* FAKE: the holder between the header's cells and s.table; stored directly:
     * score 15. */
    Unk8009B400Record *s1;

    s.x = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    sheet_headers = g_entry_menu_resources->unk_5C;
    s.header = sheet_headers[3];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x1C, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    s.has_color = 1;
    s.header = sheet_headers[4];
    s1 = s.header->cells;
    if (g_entry_menu_handler_index < 4) {
        s.semi = 1;
        if (g_entry_menu_work_block->animation_frame_counter & 4) {
            s.col_r = 0xBC;
            s.col_g = 0x78;
            s.col_b = 0x14;
        } else {
            s.col_r = 0xA8;
            s.col_g = 0x6E;
            s.col_b = 0x14;
        }
        s.ot_idx = 0x14;
        s.table = s1;
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        draw_context->primitive_cursors.textured_quad_prims =
            gpu_AddScaledSpriteQuads(&s, 0);
        s.table = s1 + 1;
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        draw_context->primitive_cursors.textured_quad_prims =
            gpu_AddScaledSpriteQuads(&s, 1);
        s.table = s1 + 2;
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        draw_context->primitive_cursors.textured_quad_prims =
            gpu_AddScaledSpriteQuads(&s, 2);
        s.table = s1 + 3;
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        draw_context->primitive_cursors.textured_quad_prims =
            gpu_AddScaledSpriteQuads(&s, 3);
        SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0, 0x60, 0);
        AddPrim(
            g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
        draw_context->primitive_cursors.unk_14++;
    } else {
        s.col_r = 0x32;
        s.col_g = 0x32;
        s.col_b = 0x5A;
    }
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0x12;
    s.semi = 0;
    s.header = sheet_headers[5];
    s1 = s.header->cells;
    s.table = s1;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    if (g_entry_menu_handler_index < 2) {
        s.ot_idx = 0x12;
        s.semi = 1;
        s.header =
            sheet_headers[10 +
                          g_entry_menu_work_block->animation_frame_counter % 4];
        s1 = s.header->cells;
        s.table = s1;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, 0x20), 0);
        AddPrim(
            g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
        draw_context->primitive_cursors.unk_14++;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order. */
u8 D_800A32E8 = 0;
u8 D_800A32E9 = 0;
RECT D_800A32EC = {144, 507, 48, 1};
RECT D_800A32F4 = {896, 384, 64, 64};
