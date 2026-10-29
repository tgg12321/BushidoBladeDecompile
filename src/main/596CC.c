/* 48 game functions: the row menu (menu_SetEnabledRows,
 * menu_LoadResourcesInitStateSelectFirstEnabledRow,
 * menu_HandleOptionMenuChoices,
 * ..., gpu_InitPrimBufferPair). .text 0x80068ECC (ROM 0x596CC). Start boundary:
 * SEAM (owner ruling 2026-10-17; Codex seam #99): no direct call or static
 * crosses the cut; 51268.c's .sbss ends at D_800A34F4 and these functions own
 * g_menu_packed_cursors..g_menu_confirm_cursor; .rodata 0x80015988+0x28
 * (menu_HandleOptionMenuChoices's table, D_800159A0) follows 51268.o's. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>
#include <psxsdk/gtemac.h>

/* Q65: this file's statics (.sbss), in address order. */
static s32 g_menu_packed_cursors;
static Unk800A34FCRec *g_row_menu_work_block;
static s32 g_menu_arena_cursor;
/* not named by any code or data: size from the gap */
static s32 D_800A3504[2];
static s16 g_menu_draw_offset_steps[2];
static s16 D_800A3510[2];
static s32 g_row_menu_pulse_counter;
static s32 g_menu_draw_buffer_counter;
static Unk8006E49CRec *g_row_menu_prim_buffers;
static Unk8006E49CRec *D_800A3520;
static Unk8009BD24Block *g_row_menu_working_record_ptr;
static s16 g_menu_stored_choice_cursor;
static s32 g_menu_confirm_cursor;

extern void AddPrim(void *, void *);
extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
/* Declared unprototyped: the callers below pass an s32 where the definition
 * takes a pointer, and gpu_DrawWhiteTileAndSheetsTestPadMask calls
 * func_80069E18 with 2 of its 1 args.
 */
s32 reloc_RelocateTwelveListsAndLoadVabPack();
void func_80069E18();
extern void gpu_LoadVramBottomStrip();

void menu_SetEnabledRows(s32 arg0) {
    s32 *p = &g_row_menu_row_state.rows_on;
    s32 v = *p;
    v &= ~0x1;
    v |= arg0 & 0x1;
    v &= ~0x2;
    v |= arg0 & 0x2;
    v &= ~0x4;
    v |= arg0 & 0x4;
    v &= ~0x8;
    v |= (((u32)arg0 >> 4) & 1) << 3;
    v &= ~0x10;
    v |= (((u32)arg0 >> 5) & 1) << 4;
    v &= ~0x20;
    v |= (((u32)arg0 >> 6) & 1) << 5;
    v &= ~0x40;
    v |= (arg0 << 3) & 0x40;
    v &= ~0x80;
    v |= arg0 & 0x80;
    *p = v;
}

extern s32 D_800A372C;
extern u8 g_row_menu_vram_copy_source_rect[8];

s32 menu_LoadResourcesInitStateSelectFirstEnabledRow(
    s32 arg0, Unk8009BD24Block *arg1) {
    RECT buf;
    s32 temp_s0;
    s32 var_2;
    Unk800A34FCRec *work_block;

    g_menu_arena_cursor = arg0;
    g_row_menu_prim_buffers = (Unk8006E49CRec *)arg0;
    temp_s0 = arg0 + 0x58;
    g_menu_arena_cursor = temp_s0;
    resource_FreeSlotsAndBlocksAndInvalidateEntryCache();
    gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
        2, (Unk8006E950Head *)g_menu_arena_cursor);
    D_800A372C = g_menu_arena_cursor;
    var_2 = reloc_RelocateTwelveListsAndLoadVabPack(g_menu_arena_cursor);
    g_menu_arena_cursor = var_2;
    work_block = (Unk800A34FCRec *)gpu_InitPrimBufferPair(
        var_2, g_row_menu_prim_buffers);
    work_block->menu_resource_root = (Unk8006919CRec *)temp_s0;
    g_menu_arena_cursor = (s32)work_block;
    g_row_menu_work_block = work_block;
    g_menu_arena_cursor = (s32)(work_block + 1);
    {
        s32 var_4 = -0x10;
        s32 var_5;
        s32 flags = g_menu_packed_cursors;
        var_5 = g_row_menu_row_state.rows_on;

        flags &= var_4;
        g_menu_packed_cursors = flags;
        if (!((u32)var_5 & 1)) {
            u32 mask;
            s32 var_8;
            mask = (u32)-0x10;
            var_8 = g_row_menu_row_state.rows_on;
            do {
                s32 next_row;
                s32 lo;
                flags = g_menu_packed_cursors;
                lo = flags;
                lo &= 0xF;
                if (lo >= 7) {
                    g_menu_packed_cursors = flags & mask;
                    break;
                }
                next_row = lo + 1;
                flags = (flags & mask) | (next_row & 0xF);
                g_menu_packed_cursors = flags;
                flags &= 0xF;
                flags = (u32)var_8 >> flags;
                flags &= 1;
            } while (!flags);
        }
        {
            /* FAKE: one s16 * holder per stored work-block field, set before
             * the g_row_menu_working_record_ptr store; plain `*p` stores keep
             * the g_menu_stored_choice_cursor and g_menu_packed_cursors stores
             * in place. Member stores through a holder: score 28; straight
             * through g_row_menu_work_block: 39. */
            s16 *p_2A;
            s16 *p_28;
            s16 *p_0E;
            s16 *p_0C;
            s16 *p_10;
            s16 *p_12;
            s32 value;

            value = 5;
            /* FAKE: block fence keeps li v0,5 at the join-block head so
               reorg steals it into all three incoming jump delay slots; without
               it: score 4 */
            do {
            } while (0);
            p_2A = &g_row_menu_work_block->row_bit_cursors.half[1];
            p_28 = &g_row_menu_work_block->row_bit_cursors.half[0];
            p_0E = &g_row_menu_work_block->menu_draw_offsets[1];
            p_0C = &g_row_menu_work_block->menu_draw_offsets[0];
            p_10 = &g_row_menu_work_block->unk_10;
            p_12 = &g_row_menu_work_block->unk_12;
            g_row_menu_working_record_ptr = arg1;
            /* FAKE: sched fence keeps the g_row_menu_working_record_ptr store
               adjacent to the g_row_menu_work_block load instead of sinking
               below the zero-stores; without it: score 2 */
            do {
            } while (0);
            g_menu_draw_buffer_counter = 0;
            g_menu_stored_choice_cursor = 0;
            *p_2A = value;
            *p_28 = value;
            flags = g_menu_packed_cursors & ~0x1C00;
            D_800A3510[1] = 0;
            D_800A3510[0] = 0;
            g_menu_draw_offset_steps[1] = 0;
            g_menu_draw_offset_steps[0] = 0;
            flags |= 0x1000;
            g_menu_packed_cursors = flags;
            *p_12 = 0;
            *p_10 = 0;
            *p_0E = 0;
            *p_0C = 0;
            __builtin_memcpy(&buf, g_row_menu_vram_copy_source_rect, 8);
            DrawSync(0);
            MoveImage(&buf, 0x3C0, 0x1FE);
            DrawSync(0);
        }
    }
    g_row_menu_work_block->strip_refresh_snapshot =
        g_row_menu_working_record_ptr->grayscale;
    return 1;
}

Unk8006E49CRec *gpu_GetPrimBufferAndRefreshStrip(s32 buffer_index) {
    Unk8009BD24Block *v0 = g_row_menu_working_record_ptr;
    Unk800A34FCRec *v1 = g_row_menu_work_block;
    if (v1->strip_refresh_snapshot != v0->grayscale) {
        gpu_LoadVramBottomStrip(D_800A372C);
    }
    v0 = g_row_menu_working_record_ptr;
    v1 = g_row_menu_work_block;
    v1->strip_refresh_snapshot = v0->grayscale;
    return &g_row_menu_prim_buffers[buffer_index];
}

s32 reloc_RelocateTwelveListsAndLoadVabPack(Unk8006919CRec *resource_root) {
    s32 lists_relocated = 0;
    Unk8009B0E0Record ***p = &resource_root->unk_14;
    do {
        reloc_RelocateOffsetList((s32 *)resource_root, (s32)*p);
        p++;
        lists_relocated++;
    } while (lists_relocated < 12);
    snd_LoadVabPack(resource_root->resource_head.sound_pack, 1,
                    resource_root->resource_head.free_ram_start);
    return resource_root->resource_head.free_ram_start;
}

void reloc_RelocateOffsetList(s32 *base_addr, s32 offset_list_addr) {
    s32 *p = (s32 *)offset_list_addr;
    if (!*p)
        return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)base_addr;
        }
        p++;
    }
}

extern void gpu_AdvanceBufferCounterAndBuildDrawContext();

s32 gpu_DrawWhiteTileAndSheetsTestPadMask(s32 arg0, s32 arg1) {
    s32 sp10[10];
    g_row_menu_pulse_counter = 0;
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        sp10, &g_menu_draw_buffer_counter);
    func_80069E18(sp10, 0);
    if ((arg1 & 0x400040) != 0) {
        snd_QueueSound(1, 0x7F, 0x7F);
        return 1;
    }
    return 0;
}

extern u32 g_pad_direction_negative_masks[2];

s32 pad_StepOffsetsGetDirectionEvents(
    u32 *pad_buttons, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 one;
    s32 sum;
    s32 i;
    s32 a3_off;
    u32 positive_mask;
    u32 negative_mask;
    u32 v;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    /* FAKE: `one` constant holder schedules `li $t6,1` at the preheader;
     * literal 1: score 2 (also 2 with the *arg2 update before i++). */
    one = 1;
    /* FAKE: do{}while(0) around the loop seats sum in $t2 and the reduced
     * i * 16 in $t1; unwrapped: score 7 (for header 9). All three FAKEs
     * removed, plain for / while / do-while with arg2++ / arg3++: 15. */
    do {
        a3_off = 0;
        do {
            /* FAKE: byte-offset step; element-count step: score 3,
             * arg3[i]: 11 (18 with the other two FAKEs also removed). */
            arg3 = (s16 *)((u8 *)arg3 + a3_off);
            negative_mask = g_pad_direction_negative_masks[i] << arg1;
            positive_mask = g_pad_direction_positive_masks[i] << arg1;
            if (*arg2 == 0) {
                v = *pad_buttons;
                if (v & positive_mask) {
                    *arg3 = one;
                } else if (v & negative_mask) {
                    *arg3 = -1;
                }
            } else {
                v = *pad_buttons;
                if (v & positive_mask) {
                    *arg2 = 6;
                } else if (v & negative_mask) {
                    *arg3 = -6;
                }
                sval = *arg2;
                if (sval >= 6) {
                    sum += one << (i * 16);
                    *arg3 = 0;
                    *arg2 = 0;
                } else if (sval < -5) {
                    sum += 2 << (i * 16);
                    *arg3 = 0;
                    *arg2 = 0;
                }
            }
            a3_off += 2;
            i++;
            *arg2 = (u16)*arg2 + (u16)*arg3;
            arg2++;
        } while (i < 2);
    } while (0);
    return sum;
}

s32 menu_UpdateRowMenu(s32 arg0, s32 arg1) {
    extern void gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(
        s32 *, s32, GpuDb *);
    extern void menu_DrawMaskedRowList(u8 *, u16 *, s32);
    extern void menu_DrawModeBit1Sheets(s32 *, s32);
    extern void menu_DrawModeBit0Sheets(s32 *, s32);
    /* FAKE: frame layout. gpu_AdvanceBufferCounterAndBuildDrawContext fills
     * words 1..9 (siblings: s32 sp10[10]); the folded `arg1` word sits at word
     * 12 and the saved row mask at word 14 only to reproduce the target's 0x60
     * frame (sp+0x40 / sp+0x48); separate locals give the wrong frame:
     * score 11. */
    s32 var_0[15];
    s32 direction_events;
    GpuDb *gpu_db;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        var_0, &g_menu_draw_buffer_counter);
    var_0[12] = (arg1 & 0xFFFF) | ((u32)arg1 >> 16);
    direction_events = pad_StepOffsetsGetDirectionEvents(
        (u32 *)&var_0[12], 0, g_row_menu_work_block->menu_draw_offsets,
        g_menu_draw_offset_steps);
    switch (direction_events >> 16) {
    case 1:
        do {
            s32 flags = g_menu_packed_cursors;
            s32 index;
            g_menu_packed_cursors =
                (flags & ~0xF) | (index = ((flags & 0xF) + 1) & 0xF);
            if (index >= 8) {
                g_menu_packed_cursors &= ~0xF;
            }
        } while (!(((u32)g_row_menu_row_state.rows_on >>
                    (g_menu_packed_cursors & 0xF)) &
                   1));
        g_row_menu_pulse_counter = 0;
        g_row_menu_working_record_ptr->view_setting = 0;
        break;
    case 2:
        do {
            s32 flags = g_menu_packed_cursors;
            s32 index = flags & 0xF;
            if (index == 0) {
                g_menu_packed_cursors = (flags & ~0xF) | 7;
            } else {
                g_menu_packed_cursors = (flags & ~0xF) | ((index - 1) & 0xF);
            }
        } while (!(((u32)g_row_menu_row_state.rows_on >>
                    (g_menu_packed_cursors & 0xF)) &
                   1));
        g_row_menu_pulse_counter = 0;
        g_row_menu_working_record_ptr->view_setting = 0;
        break;
    }
    if (((direction_events & 0xFF) < 3) && ((direction_events & 0xFF) != 0) &&
        (g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode == 3)) {
        g_row_menu_working_record_ptr->view_setting ^= 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    if (g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode == 2) {
        g_row_menu_working_record_ptr->view_setting = 1;
    } else if (
        g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode == 1) {
        g_row_menu_working_record_ptr->view_setting = 0;
    }
    if ((g_menu_packed_cursors & 0xF) != 7) {
        if (((g_menu_packed_cursors & 0xF) - 6) >= 0) {
            var_0[14] = g_row_menu_row_state.scroll_row_mask & 0x80;
            g_row_menu_row_state.scroll_row_mask =
                var_0[14] | (0x1F << ((g_menu_packed_cursors & 0xF) - 5));
        } else {
            g_row_menu_row_state.scroll_row_mask = 0x9F;
        }
    }
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(var_0, 0, gpu_db);
    menu_DrawMaskedRowList(
        (u8 *)var_0, (u16 *)gpu_db,
        g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode);
    menu_DrawModeBit1Sheets(
        var_0, g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode);
    menu_DrawModeBit0Sheets(
        var_0, g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].mode);
    if (arg1 & 0x40) {
        switch (g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].state) {
        case 4:
        case 5:
            if (g_pad_state.valid[1] == 0) {
                snd_QueueSound(2, 0x7F, 0x7F);
                snd_FlushSoundQueue();
                goto cancel;
            }
        }
        goto accept;
    }
    if (arg1 & 0x400000) {
        switch (g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].state) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            goto reject;
        }
        if (g_pad_state.valid[0] == 0) {
        reject:
            snd_QueueSound(2, 0x7F, 0x7F);
            goto cancel;
        }
    accept:
        snd_QueueSound(1, 0x7F, 0x7F);
        if (g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].state != 7) {
            snd_CloseVab1();
        }
        return g_row_menu_row_state.opt[g_menu_packed_cursors & 0xF].state;
    }
    if (arg1 & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (arg1 & 0x50005000) {
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    return -1;
}

void gpu_AddThreeGreyTilesStackedUp(
    s32 *draw_context, u16 *tile_rect, s32 ot_index) {
    TILE *p = (TILE *)draw_context[6];

    SetTile(p);
    p->r0 = 0xFF;
    p->g0 = 0xFF;
    p->b0 = 0xFF;
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = tile_rect[0];
    p->y0 = tile_rect[1];
    p->w = tile_rect[2];
    p->h = tile_rect[3];
    SetSemiTrans(p, 0);
    AddPrim(g_gpu_ot_ptr + ot_index, p);
    p++;

    SetTile(p);
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = tile_rect[0];
    p->y0 = tile_rect[1] - 1;
    p->w = tile_rect[2];
    p->h = tile_rect[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + ot_index, p);
    p++;

    SetTile(p);
    p->r0 = 0x40;
    p->g0 = 0x40;
    p->b0 = 0x40;
    p->x0 = tile_rect[0];
    p->y0 = tile_rect[1] - 2;
    p->w = tile_rect[2];
    p->h = tile_rect[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + ot_index, p);
    p++;

    draw_context[6] = (s32)p;
}

void gpu_SetTileRgbFromBlockFlag(void *tile_prim) {
    TILE *a0 = tile_prim;
    Unk8009BD24Block *p = GetWorkingRecord();
    s32 v0;
    if (p->grayscale) {
        v0 = 0x22;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x4C;
        a0->r0 = v0;
        a0->g0 = v0;
        v0 = 0x6C;
    }
    a0->b0 = v0;
}

void gpu_SetPolyF4RgbFromBlockFlag(void *flat_quad_prim) {
    POLY_F4 *a0 = flat_quad_prim;
    Unk8009BD24Block *p = GetWorkingRecord();
    s32 v0;
    if (p->grayscale) {
        v0 = 8;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x31;
        a0->r0 = 0;
        a0->g0 = 0;
    }
    a0->b0 = v0;
}

void gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(
    s32 *draw_context, s32 mode, GpuDb *arg2) {
    TILE *p;
    POLY_F4 *poly;
    Unk8009B0E0Record **sheet_headers;
    Unk8009B0E0Record **sheet_header_cursor;
    s32 sheets_drawn;
    Unk8007352CRec s;

    p = (TILE *)draw_context[6];

    if (mode == 2) {
        SetTile(p);
        gpu_SetTileRgbFromBlockFlag(p);
        p->x0 = 0x4E;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
        SetTile(p);
        gpu_SetTileRgbFromBlockFlag(p);
        p->x0 = 0x166;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else if (mode == 1) {
        SetTile(p);
        gpu_SetTileRgbFromBlockFlag(p);
        p->x0 = 0x3F;
        p->y0 = 0x30;
        p->w = 0x202;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else {
        SetTile(p);
        gpu_SetTileRgbFromBlockFlag(p);
        p->x0 = 0x130;
        p->y0 = 0x3A;
        p->w = 0x126;
        p->h = 0xAB;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    }
    draw_context[6] = (s32)p;
    s.ot_idx = 0x12;
    s.has_color = 0;
    s.semi = 0;
    sheet_headers = ((Unk8006919CRec *)draw_context[1])->unk_34;
    s.x = 0;
    s.y = 0;
    sheet_header_cursor = sheet_headers;
    sheets_drawn = 0;
    do {
        s.header = *sheet_header_cursor;
        s.table = s.header->cells;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        sheet_header_cursor++;
        sheets_drawn++;
    } while (sheets_drawn < 3);

    {
        s.header = sheet_headers[0];
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                    gpu_GetSheetTPage(s.header, 0), 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x12, (DR_MODE *)draw_context[7]);

    poly = (POLY_F4 *)draw_context[3];
    draw_context[7] += 0xC;
    SetPolyF4(poly);
    gpu_SetPolyF4RgbFromBlockFlag(poly);
    poly->x0 = 0;
    poly->y0 = 0xB9;
    poly->x1 = 0x122;
    poly->y1 = 0;
    poly->x2 = 0;
    poly->y2 = 0xEF;
    poly->x3 = 0x122;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    gpu_SetPolyF4RgbFromBlockFlag(poly);
    poly->x0 = 0x15E;
    poly->x1 = 0x27F;
    poly->y0 = 0;
    poly->y1 = 0;
    poly->x2 = 0x15E;
    poly->y2 = 0xEF;
    poly->x3 = 0x27F;
    poly->y3 = 0x36;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    gpu_SetPolyF4RgbFromBlockFlag(poly);
    poly->x0 = 0x122;
    poly->y0 = 0;
    poly->x1 = 0x15E;
    poly->y1 = 0;
    poly->x2 = 0x122;
    poly->y2 = 0xEF;
    poly->x3 = 0x15E;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    draw_context[3] = (s32)poly;
}

void func_80069E18(s32 draw_context_addr) {
    TILE *tile;
    Unk8009B0E0Record **ptr;
    Unk8007352CRec s;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *p1;

    tile = (TILE *)*(s32 *)(draw_context_addr + 0x18);
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0xFF;
    tile->b0 = 0xFF;
    tile->w = 0x280;
    tile->x0 = 0;
    tile->y0 = 0;
    tile->h = 0xF0;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + 0x14, tile);
    *(s32 *)(draw_context_addr + 0x18) = (s32)(tile + 1);

    ptr = (*(Unk8006919CRec **)(draw_context_addr + 4))->unk_14;
    s.ot_idx = 0x10;
    s.semi = 0;
    s.x = 0;
    s.y = 0;
    s.has_color = 0;

    s.header = ptr[0];
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context_addr + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x11, (DR_MODE *)*(s32 *)(draw_context_addr + 0x1C));
    *(s32 *)(draw_context_addr + 0x1C) =
        *(s32 *)(draw_context_addr + 0x1C) + 0xC;

    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(draw_context_addr + 0x14);
    *(s32 *)(draw_context_addr + 0x14) = gpu_AddSpriteCells(&s);

    s.header = ptr[1];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(draw_context_addr + 0x14);
    *(s32 *)(draw_context_addr + 0x14) = gpu_AddSpriteCells(&s);

    s.header = ptr[2];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(draw_context_addr + 0x14);
    *(s32 *)(draw_context_addr + 0x14) = gpu_AddSpriteCells(&s);
}

/* FAKE: frame layout (oversized live object): the descriptor plus an
 * unwritten tail, so the locals region reaches the target's frame. The plain
 * descriptor scores 12 (menu_DrawModeBit1Sheets) / 14
 * (menu_DrawModeBit0Sheets). */
typedef struct {
    Unk8007352CRec fr00;
    s32 tail[4];
} Unk80069F80Rec;

void menu_DrawModeBit1Sheets(s32 *draw_context, s32 cursor_row_mode) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddScaledSpriteQuads / gpu_AddSpriteCells); tail[0..3] is unwritten
     * padding that brings the locals to the target's 0x70 frame (the 0x2C
     * descriptor alone gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The
     * plain descriptor: score 12. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *var_4;

    if (cursor_row_mode & 2) {
        ptr = ((Unk8006919CRec *)draw_context[1])->unk_1C;
        s.fr00.header = ptr[0];
        if (cursor_row_mode & 1) {
            s.fr00.x = 0x9C;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (g_row_menu_working_record_ptr->view_setting) {
            if (cursor_row_mode & 1) {
                s.fr00.x = x0 + g_row_menu_work_block->menu_draw_offsets[0];
                c = ((rsin((g_row_menu_pulse_counter & 0x1F) << 7) * 47) >>
                     12) -
                    0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 3;
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.ft4_out = draw_context[2];
        draw_context[2] = gpu_AddScaledSpriteQuads(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[1];
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s.fr00);
        if (cursor_row_mode & 1) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            s.fr00.table = &s.fr00.header->cells[1];
            s.fr00.sprt_out = draw_context[5];
            draw_context[5] = gpu_AddSpriteCells(&s.fr00);
        }
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                    gpu_GetSheetTPage(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 3, (DR_MODE *)draw_context[7]);
        draw_context[7] += 0xC;
    }
}

void menu_DrawModeBit0Sheets(s32 *draw_context, s32 cursor_row_mode) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddScaledSpriteQuads / gpu_AddSpriteCells); tail[0..3] is unwritten
     * padding that brings the locals to the target's 0x70 frame (the 0x2C
     * descriptor alone gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The
     * plain descriptor: score 14. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *var_4;

    if (cursor_row_mode & 1) {
        ptr = ((Unk8006919CRec *)draw_context[1])->unk_1C;
        s.fr00.header = ptr[2];
        if (cursor_row_mode & 2) {
            s.fr00.x = -1;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (!(g_row_menu_working_record_ptr->view_setting)) {
            if (cursor_row_mode & 2) {
                s.fr00.x = x0 + g_row_menu_work_block->menu_draw_offsets[0];
                c = ((rsin((g_row_menu_pulse_counter & 0x1F) << 7) * 47) >>
                     12) -
                    0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.x = x0 + 0x32;
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 2;
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.ft4_out = draw_context[2];
        draw_context[2] = gpu_AddScaledSpriteQuads(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[3];
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s.fr00);
        if (cursor_row_mode & 2) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            var_4 = s.fr00.header->cells;
            s.fr00.table = var_4;
            s.fr00.sprt_out = draw_context[5];
            draw_context[5] = gpu_AddSpriteCells(&s.fr00);
        }
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                    gpu_GetSheetTPage(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 2, (DR_MODE *)draw_context[7]);
        draw_context[7] += 0xC;
    }
}

void gpu_DrawList1CEntry4SpriteCellsAtOrigin(
    s32 *draw_context, Unk8007352CRec *draw_desc) {
    draw_desc->header = ((Unk8006919CRec *)draw_context[1])->unk_1C[4];
    draw_desc->x = 0;
    draw_desc->y = 0;
    draw_desc->has_color = 0;
    draw_desc->semi = 0;
    draw_desc->ot_idx = 1;
    draw_desc->table = draw_desc->header->cells;
    draw_desc->sprt_out = draw_context[5];
    draw_context[5] = gpu_AddSpriteCells(draw_desc);
    SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                gpu_GetSheetTPage(draw_desc->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;
}

void menu_DrawList1CEntry9UnitScaleQuadsAtOrigin(
    s32 *draw_context, Unk8007352CRec *draw_desc) {
    draw_desc->header = ((Unk8006919CRec *)draw_context[1])->unk_1C[9];
    draw_desc->x = 0;
    draw_desc->y = 0;
    draw_desc->has_color = 0;
    draw_desc->semi = 0;
    draw_desc->ot_idx = 1;
    draw_desc->scale_x = 0x100;
    draw_desc->scale_y = 0x100;
    draw_desc->table = draw_desc->header->cells;
    draw_desc->ft4_out = draw_context[2];
    draw_context[2] = gpu_AddScaledSpriteQuads(draw_desc, 0);
    SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                gpu_GetSheetTPage(draw_desc->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;
}

void menu_DrawRowThreeThinTilesAndTwoSheets(
    u8 *draw_context, Unk8007352CRec *arg1, s32 row_index) {
    TILE *tile;
    Unk8009B0E0Record **sheet_headers;
    s32 s4;

    tile = (TILE *)*(s32 *)(draw_context + 0x18);
    SetTile(tile);
    if ((g_menu_packed_cursors & 0xF) == row_index) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
        s4 = 0;
    } else {
        setRGB0(tile, 0x50, 0x50, 0x50);
        s4 = 0x20;
    }
    tile->x0 = 0x5F;
    tile->y0 = arg1->y + 0xF;
    tile->w = arg1->x + 0x19;
    tile->h = 1;
    SetSemiTrans(tile, arg1->semi);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((g_menu_packed_cursors & 0xF) == row_index) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
    } else {
        setRGB0(tile, 0x20, 0x20, 0x20);
    }
    tile->x0 = arg1->x;
    tile->y0 = arg1->y + 0xE;
    tile->w = 0x78;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((g_menu_packed_cursors & 0xF) == row_index) {
        setRGB0(tile, arg1->col_r >> 1, 0, arg1->col_b >> 1);
    } else {
        setRGB0(tile, 0x10, 0x10, 0x10);
    }
    tile->x0 = arg1->x + 0x40;
    tile->y0 = arg1->y + 0xD;
    tile->w = 0x38;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);

    tile = tile + 1;
    *(s32 *)(draw_context + 0x18) = (s32)tile;
    sheet_headers = (*(Unk8006919CRec **)(draw_context + 4))->unk_1C;
    {
        /* FAKE: single-use holder of the cell table ahead of the descriptor's
         * table store; stored directly: score 7. */
        Unk8009B400Record *v1;
        /* FAKE: the header store through a pointer to the member: stored as
           arg1->header the member store is scheduled past the
           g_menu_packed_cursors load (MEM_IN_STRUCT_P); score 7. */
        Unk8009B0E0Record **var_7 = &arg1->header;

        *var_7 = sheet_headers[10];
        if ((g_menu_packed_cursors & 0xF) == row_index) {
            arg1->col_g = 0;
            arg1->col_r = arg1->col_r >> 1;
            arg1->col_b = arg1->col_b >> 1;
        } else {
            arg1->col_b = 0x28;
            arg1->col_g = 0x28;
            arg1->col_r = 0x28;
        }

        arg1->x = 0;
        v1 = arg1->header->cells;
        arg1->y += 0xF;
        arg1->table = v1;

        arg1->sprt_out = *(s32 *)(draw_context + 0x14);
        *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(arg1);

        arg1->header = sheet_headers[11];
        v1 = arg1->header->cells;
        arg1->table = v1;
        arg1->sprt_out = *(s32 *)(draw_context + 0x14);
        *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(arg1);
    }

    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(arg1->header, s4), 0);
    AddPrim(
        g_gpu_ot_ptr + arg1->ot_idx, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) = *(s32 *)(draw_context + 0x1C) + 0xC;
}

/* Draws the eight-row list screen from the MOD.BIN resource (ctx+4): the row
 * sheets (cursor row flashing, rows switched on in g_row_menu_row_state.rows_on
 * lit), a counter frame, row 7, the frame icons and a TILE. draw_context = the
 * words gpu_AdvanceBufferCounterAndBuildDrawContext fills (+0x8 POLY_FT4
 * cursor, +0x14 SPRT cursor, +0x18 TILE cursor, +0x1C DR_MODE cursor, +0x20
 * DR_AREA cursor, +0x24 DR_OFFSET cursor), accessed as byte offsets like
 * menu_DrawRowThreeThinTilesAndTwoSheets; draw_env = the drawing environment's
 * clip RECT and ofs[2]; arg2 (passed by menu_UpdateRowMenu) is unused. */
void menu_DrawMaskedRowList(u8 *draw_context, u16 *draw_env, s32 arg2) {
    Unk8007352CRec s;
    RECT rect;
    s16 ofs[2];
    /* Ruling 11 (ordinary-c-judge-decidable): holds five values, each a
     * sprite-sheet list of the MOD.BIN
     * root: row_sheets (option rows; first sheet, the row loop and row 7),
     * cursor_row_sheets (a sheet list indexed by the menu cursor row), unk_24
     * twice (icon frames at [8 + frame], then FT4 frames at [frame]). */
    Unk8009B0E0Record **var_3;
    /* Ruling 11: holds two values, each a mask with one bit per option row:
       g_row_menu_row_state.scroll_row_mask (scanned for the first row drawn)
       and .rows_on (rows switched on). */
    u32 row_mask;
    /* the sheet's cells, just past its 12-byte header, for s.table. Ruling 9:
     * one meaning (s.header + 0xC) at five writes. FAKE: single-use holder of
     * the cell table ahead of the descriptor's table store; stored directly:
     * score 15 (Q129). */
    Unk8009B400Record *cells;
    s32 yofs;
    s32 bit;
    s32 row_index;

    var_3 = (*(Unk8006919CRec **)(draw_context + 4))->row_sheets;
    row_mask = g_row_menu_row_state.scroll_row_mask;
    bit = 0;
    s.header = var_3[0];
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;
    rect.x = draw_env[0];
    rect.y = draw_env[1] + 0x3A;
    rect.w = 0xF6;
    rect.h = 0x92;
    SetDrawArea((DR_AREA *)*(s32 *)(draw_context + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_AREA *)*(s32 *)(draw_context + 0x20));
    *(s32 *)(draw_context + 0x20) += 0xC;
    yofs = 0;
    for (; !(row_mask & (1 << bit)); bit++) {
        yofs -= 0x18;
    }
    ofs[0] = draw_env[4];
    ofs[1] = draw_env[5] + yofs;
    SetDrawOffset((DR_OFFSET *)*(s32 *)(draw_context + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_OFFSET *)*(s32 *)(draw_context + 0x24));
    *(s32 *)(draw_context + 0x24) += 0xC;

    row_mask = g_row_menu_row_state.rows_on;
    s.ot_idx = 0xB;
    for (row_index = 0; row_index < 7; row_index++) {
        s32 y = row_index * 0x18 + 0x3F;

        s.x = 0x76;
        s.has_color = 1;
        if ((g_menu_packed_cursors & 0xF) == row_index) {
            s.semi = 0;
            s.col_r = ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                        63) >>
                       12) -
                      0x61;
            s.col_g = ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                        45) >>
                       12) -
                      0x7D;
            s.col_b = 0x30;
            s.y = g_row_menu_work_block->menu_draw_offsets[1] + y;
            g_row_menu_pulse_counter++;
        } else if ((row_mask >> row_index) & 1) {
            s.col_r = 0x80;
            s.col_g = 0x6C;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
            if (g_row_menu_working_record_ptr->grayscale) {
                s.has_color = 0;
            }
        } else {
            s.col_r = 0x30;
            s.col_g = 0x30;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
        }
        s.header = var_3[row_index];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = *(s32 *)(draw_context + 0x14);
        *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(&s);
        SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                    gpu_GetSheetTPage(s.header, 0), 0);
        AddPrim(
            g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
        *(s32 *)(draw_context + 0x1C) += 0xC;
        s.has_color = 1;
        menu_DrawRowThreeThinTilesAndTwoSheets(draw_context, &s, row_index);
    }

    var_3 = (*(Unk8006919CRec **)(draw_context + 4))->cursor_row_sheets;
    s.ot_idx = 0xA;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.y = 0x41;
    s.header = var_3[g_menu_packed_cursors & 0xF];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = *(s32 *)(draw_context + 0x14);
    *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(draw_context + 4))->row_sheets;
    rect.x = draw_env[0];
    rect.y = draw_env[1];
    rect.w = draw_env[2];
    rect.h = draw_env[3];
    SetDrawArea((DR_AREA *)*(s32 *)(draw_context + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_AREA *)*(s32 *)(draw_context + 0x20));
    *(s32 *)(draw_context + 0x20) += 0xC;
    ofs[0] = draw_env[4];
    ofs[1] = draw_env[5];
    SetDrawOffset((DR_OFFSET *)*(s32 *)(draw_context + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_OFFSET *)*(s32 *)(draw_context + 0x24));
    *(s32 *)(draw_context + 0x24) += 0xC;

    s.x = 0x76;
    s.has_color = 1;
    if ((g_menu_packed_cursors & 0xF) == 7) {
        s.semi = 0;
        s.col_r =
            ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) * 63) >>
             12) -
            0x61;
        s.col_g =
            ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) * 45) >>
             12) -
            0x7D;
        s.col_b = 0x30;
        s.y = g_row_menu_work_block->menu_draw_offsets[1] + 0xCF;
        g_row_menu_pulse_counter++;
    } else if ((row_mask >> 7) & 1) {
        s.col_r = 0x80;
        s.col_g = 0x6C;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
        if (g_row_menu_working_record_ptr->grayscale) {
            s.has_color = 0;
        }
    } else {
        s.col_r = 0x30;
        s.col_g = 0x30;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
    }
    s.ot_idx = 9;
    s.header = var_3[7];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = *(s32 *)(draw_context + 0x14);
    *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;
    s.has_color = 1;
    menu_DrawRowThreeThinTilesAndTwoSheets(draw_context, &s, 7);
    menu_DrawList1CEntry9UnitScaleQuadsAtOrigin((s32 *)draw_context, &s);

    s.has_color = 0;
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(draw_context + 4))->unk_24;
    s.header = var_3[(g_menu_packed_cursors & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = *(s32 *)(draw_context + 0x14);
    *(s32 *)(draw_context + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;

    /* FAKE: reused: re-stores the unk_24 list var_3 already holds; dropped:
     * score 3; read in place, or a second local: 2. */
    var_3 = (*(Unk8006919CRec **)(draw_context + 4))->unk_24;
    s.header = var_3[g_menu_packed_cursors & 0xF];
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = *(s32 *)(draw_context + 8);
    *(s32 *)(draw_context + 8) = gpu_AddScaledSpriteQuads(&s, 0);

    SetTile((TILE *)*(s32 *)(draw_context + 0x18));
    ((TILE *)*(s32 *)(draw_context + 0x18))->r0 = 0;
    ((TILE *)*(s32 *)(draw_context + 0x18))->g0 = 0;
    ((TILE *)*(s32 *)(draw_context + 0x18))->b0 = 0;
    ((TILE *)*(s32 *)(draw_context + 0x18))->x0 = 0x142;
    ((TILE *)*(s32 *)(draw_context + 0x18))->y0 = 0x51;
    ((TILE *)*(s32 *)(draw_context + 0x18))->w = 0x100;
    ((TILE *)*(s32 *)(draw_context + 0x18))->h = 0x59;
    SetSemiTrans((TILE *)*(s32 *)(draw_context + 0x18), 0);
    AddPrim(g_gpu_ot_ptr + 1, (TILE *)*(s32 *)(draw_context + 0x18));
    *(s32 *)(draw_context + 0x18) += 0x10;

    SetDrawMode((DR_MODE *)*(s32 *)(draw_context + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(draw_context + 0x1C));
    *(s32 *)(draw_context + 0x1C) += 0xC;
}

/* One argument: the caller's s32[10] (asm reads a0 only). */
void menu_DrawOptionMenuChoices(s32 *arg0);

void menu_DrawOptionMenuChoices(s32 *draw_context) {
    Unk8007352CRec s;
    u16 tile_rect[4];
    Unk8009B0E0Record **tbl;
    s32 i;
    /* the current sheet's cells, just past its 12-byte header, for s.table:
     * one role per sheet drawn (Ruling 5 extension: s.header is set from tbl
     * before each write). FAKE: single-use holder of the cell table ahead of
     * the descriptor's table store; stored directly: score 11 (Q129). */
    Unk8009B400Record *table;

    s.has_color = 0;
    s.ot_idx = 10;
    tbl = ((Unk8006919CRec *)draw_context[1])->unk_28;
    s.header = tbl[0];
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    table = s.header->cells;
    s.table = table;
    s.sprt_out = draw_context[5];
    draw_context[5] = gpu_AddSpriteCells(&s);
    SetDrawMode(
        (DR_MODE *)draw_context[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;
    s.x = 0;
    i = 0;

    do {
        s.header = tbl[i + 1];
        if ((((u32)g_menu_packed_cursors >> 10) & 7) == i) {
            s.has_color = 1;
            s.y = g_row_menu_work_block->menu_draw_offsets[1];
            s.semi = 0;
            s.col_r = s.col_g = s.col_b =
                ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                  47) >>
                 12) -
                0x80;
        } else {
            s.has_color = 0;
            s.y = 0;
            s.semi = 1;
        }
        table = s.header->cells;
        s.table = table;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 6);

    s.y = 0;
    i = 0;
    do {
        s.header = tbl[i + 7];
        s.x = 0;
        s.has_color = 0;
        if (g_row_menu_working_record_ptr->grayscale == i) {
            s.semi = 0;
            if (!(g_menu_packed_cursors & 0x1C00)) {
                s32 c;

                s.has_color = 1;
                s.x = g_row_menu_work_block->menu_draw_offsets[0];
                c = ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                      32) >>
                     12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        table = s.header->cells;
        s.table = table;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 9];
        s.x = 0;
        s.has_color = 0;
        if (g_row_menu_working_record_ptr->unk20_1 == i) {
            s.semi = 0;
            if ((g_menu_packed_cursors & 0x1C00) == 0x400) {
                s32 c;

                s.has_color = 1;
                s.x = g_row_menu_work_block->menu_draw_offsets[0];
                c = ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                      32) >>
                     12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        table = s.header->cells;
        s.table = table;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 11];
        s.x = 0;
        s.has_color = 0;
        if (g_row_menu_working_record_ptr->cd_audio_suppressed == i) {
            s.semi = 0;
            if ((g_menu_packed_cursors & 0x1C00) == 0x800) {
                s32 c;

                s.has_color = 1;
                s.x = g_row_menu_work_block->menu_draw_offsets[0];
                c = ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF) *
                      32) >>
                     12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        table = s.header->cells;
        s.table = table;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    /* FAKE: same-value re-store of `tbl` (it already holds this list): the
     * reload shares tbl's pseudo with the loops' base; without it: score 23;
     * a second local or the list read in place: score 20 */
    tbl = ((Unk8006919CRec *)draw_context[1])->unk_28;
    s.header = tbl[1];
    SetDrawMode(
        (DR_MODE *)draw_context[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;
    tile_rect[2] = 0xAF;
    tile_rect[0] = 0xE8;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    gpu_AddThreeGreyTilesStackedUp(draw_context, tile_rect, 0x11);
}

/* menu_HandleOptionMenuChoices — menu/config input dispatch. Its second
 * switch's 6-entry jump table lands at 0x80015988 in this TU's .rodata. */
s32 menu_HandleOptionMenuChoices(s32 *arg0, u32 *pressed_buttons) {
    u32 v;
    s32 merged_pressed_buttons;
    s32 direction_events;
    s32 result = 0;

    v = *pressed_buttons;
    merged_pressed_buttons = (v & 0xFFFF) | (v >> 16);
    direction_events = pad_StepOffsetsGetDirectionEvents(
        (u32 *)&merged_pressed_buttons, 0,
        g_row_menu_work_block->menu_draw_offsets, g_menu_draw_offset_steps);
    switch (direction_events >> 16) {
    case 1:
        if ((g_menu_packed_cursors & 0x1C00) == 0x1400) {
            g_menu_packed_cursors &= ~0x1C00;
        } else {
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                (((((u32)g_menu_packed_cursors >> 10) & 7) + 1) & 7) << 10;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        g_row_menu_pulse_counter = 0;
        break;
    case 2:
        if ((g_menu_packed_cursors & 0x1C00) == 0) {
            g_menu_packed_cursors = (g_menu_packed_cursors & ~0x1C00) | 0x1400;
        } else {
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                (((((u32)g_menu_packed_cursors >> 10) & 7) - 1) & 7) << 10;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        g_row_menu_pulse_counter = 0;
        break;
    }

    switch ((u32)g_menu_packed_cursors >> 10 & 7) {
    case 0:
        if ((direction_events & 0xFF) != 0) {
            g_row_menu_working_record_ptr->grayscale ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                (((((u32)g_menu_packed_cursors >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 1:
        if ((direction_events & 0xFF) != 0) {
            g_row_menu_working_record_ptr->unk20_1 ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                (((((u32)g_menu_packed_cursors >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 2:
        if ((direction_events & 0xFF) != 0) {
            g_row_menu_working_record_ptr->cd_audio_suppressed ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                (((((u32)g_menu_packed_cursors >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 3:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            g_menu_packed_cursors =
                (g_menu_packed_cursors & 0xFFFF1FFF) | 0x4000;
            result = 2;
        }
        break;
    case 4:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            result = 3;
        }
        break;
    case 5:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            result = 1;
        }
        break;
    }

    if (*pressed_buttons & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = 1;
    }
    return result;
}

/* Keeps the original .rodata run contiguous around the compiler-generated
 * switch tables. */
const u8 D_800159A0[16] = "warning\n";

s32 menu_UpdateOptionMenuChoices(s32 arg0, u32 pressed_buttons) {
    s32 sp10[10];
    GpuDb *gpu_db;
    g_row_menu_pulse_counter += 1;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        sp10, &g_menu_draw_buffer_counter);
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(sp10, 1, gpu_db);
    menu_DrawOptionMenuChoices(sp10);
    return menu_HandleOptionMenuChoices(&arg0, &pressed_buttons);
}

s32 menu_HandleMemCardChoices(s32 *arg0, u32 *pressed_buttons) {
    u32 v;
    s32 merged_pressed_buttons;
    s32 direction_events;
    s32 choice_index;
    s32 result = 0;

    v = *pressed_buttons;
    merged_pressed_buttons = (v & 0xFFFF) | (v >> 16);
    direction_events = pad_StepOffsetsGetDirectionEvents(
        &merged_pressed_buttons, 0, g_row_menu_work_block->menu_draw_offsets,
        g_menu_draw_offset_steps);
    switch (direction_events >> 16) {
    case 1:
        if ((g_menu_packed_cursors & 0xE000) == 0x4000) {
            g_menu_packed_cursors &= 0xFFFF1FFF;
        } else {
            g_menu_packed_cursors =
                (g_menu_packed_cursors & 0xFFFF1FFF) |
                (((((u32)g_menu_packed_cursors >> 13) & 7) + 1) & 7) << 13;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        break;
    case 2:
        if ((g_menu_packed_cursors & 0xE000) == 0) {
            g_menu_packed_cursors =
                (g_menu_packed_cursors & 0xFFFF1FFF) | 0x4000;
        } else {
            g_menu_packed_cursors =
                (g_menu_packed_cursors & 0xFFFF1FFF) |
                (((((u32)g_menu_packed_cursors >> 13) & 7) - 1) & 7) << 13;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        break;
    }

    choice_index = ((u32)g_menu_packed_cursors >> 13) & 7;
    switch (choice_index) {
    case 0:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            result = 2;
        }
        break;
    case 1:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            result = 3;
        }
        break;
    case 2:
        if (*pressed_buttons & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            result = 1;
            g_menu_packed_cursors =
                (g_menu_packed_cursors & ~0x1C00) |
                ((((((u32)g_menu_packed_cursors >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*pressed_buttons & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = 1;
    }
    return result;
}

void menu_DrawMemCardChoices(s32 *draw_context) {
    Unk8007352CRec s;
    u16 tile_rect[4];
    s32 choice_index;
    Unk8009B0E0Record **q;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 11. */
    Unk8009B400Record *p1;

    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xA;
    s.has_color = 0;
    q = ((Unk8006919CRec *)draw_context[1])->memcard_choice_sheets;
    {
        s.header = q[0];
        s.semi = 0;
        p1 = s.header->cells;
        s.table = p1;
    }
    s.sprt_out = draw_context[5];
    draw_context[5] = gpu_AddSpriteCells(&s);
    SetDrawMode(
        (DR_MODE *)draw_context[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    for (choice_index = 0; choice_index < 3; choice_index++) {
        s.header = q[1];
        p1 = s.header->cells;
        s.table = p1;
        if ((((u32)g_menu_packed_cursors >> 13) & 7) == choice_index) {
            s.y = g_row_menu_work_block->menu_draw_offsets[1];
            s.semi = 0;
        } else {
            s.y = 0;
            s.semi = 1;
        }
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        q++;
    }

    q = ((Unk8006919CRec *)draw_context[1])->unk_28;
    s.header = q[1];
    SetDrawMode(
        (DR_MODE *)draw_context[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    tile_rect[2] = 0xAF;
    tile_rect[0] = 0xE8;
    tile_rect[1] = 0x25;
    tile_rect[3] = 1;
    gpu_AddThreeGreyTilesStackedUp(draw_context, tile_rect, 0x11);
}

extern DR_MODE *g_sheet_pair_draw_mode_cursor;
extern u8 *g_sheet_pair_sprite_cursor;

void gpu_DrawIndexedSheetPair(s32 arg0, s32 y_offset, Unk8007352CRec *draw_desc,
                              s32 highlighted_cell_index) {
    /* sprite-sheet header pointers, two per arg0: the sheet_pairs list of the
       MOD.BIN root (g_row_menu_work_block->menu_resource_root) */
    Unk8009B0E0Record **var_0;
    /* the sheet's 8-byte cells, just past its 12-byte header */
    Unk8009B400Record *cells;
    s32 i, cell_index, cell_count;

    var_0 = g_row_menu_work_block->menu_resource_root->sheet_pairs;

    draw_desc->col_b = 0x30;
    draw_desc->col_g = 0x30;
    draw_desc->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        /* FAKE: operand grouping (or-tree-shape-shift carve-out): with
         * (var_0 + i) + arg0 * 2, arg0 * 8 is hoisted alone as in the target;
         * var_0[arg0 * 2 + i]: score 33. */
        draw_desc->header = *(var_0 + i + arg0 * 2);
        if (draw_desc->header == (Unk8009B0E0Record *)-1)
            return;

        cell_count = (arg0 == 0x12) ? 3 : 1;

        for (cell_index = 0; cell_index < cell_count; cell_index++) {
            draw_desc->x = 0;
            draw_desc->y = y_offset;
            draw_desc->ot_idx = 8;
            if (arg0 != 0x12 || cell_index == highlighted_cell_index ||
                cell_index == 2) {
                draw_desc->has_color = 0;
            } else {
                draw_desc->has_color = 1;
            }
            draw_desc->semi = 0;
            draw_desc->sprt_out = (s32)g_sheet_pair_sprite_cursor;
            cells = draw_desc->header->cells;
            draw_desc->table = cells + cell_index;
            g_sheet_pair_sprite_cursor = (u8 *)gpu_AddSpriteCells(draw_desc);
        }

        SetDrawMode(g_sheet_pair_draw_mode_cursor, 1, 0,
                    gpu_GetSheetTPage(draw_desc->header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 8, g_sheet_pair_draw_mode_cursor);
        g_sheet_pair_draw_mode_cursor++;
    }
}

extern s32 g_sheet_pair_selection_y_offset;
extern TILE *g_sheet_pair_panel_tile_cursor;
extern u8 g_sheet_pair_sprite_buffers[];
extern DR_MODE g_sheet_pair_draw_mode_buffers[];
extern TILE g_sheet_pair_panel_tile_buffers[];

void gpu_DrawSheetPairCenteredTileWithOptionalSelection(
    s32 sheet_pair_index, s32 highlighted_cell_index) {
    Unk8007352CRec draw_desc;
    s32 buffer_index;
    Size2s16 *box_size;
    s16 tile_index;
    s32 red;
    s32 w;
    s32 h;
    s32 x0;

    g_sheet_pair_selection_y_offset = 0;
    buffer_index = g_gpu_presented_frame_count & 1;
    g_sheet_pair_sprite_cursor =
        g_sheet_pair_sprite_buffers + buffer_index * 0x12C;
    g_sheet_pair_draw_mode_cursor =
        g_sheet_pair_draw_mode_buffers + buffer_index * 4;
    g_sheet_pair_panel_tile_cursor =
        g_sheet_pair_panel_tile_buffers + buffer_index * 4;
    gpu_DrawIndexedSheetPair(sheet_pair_index, 0, &draw_desc, 0);
    h = 0;
    box_size = g_row_menu_work_block->menu_resource_root->sheet_pair_box_sizes;
    box_size += sheet_pair_index;
    if (highlighted_cell_index != -1) {
        w = highlighted_cell_index ? 0x1F : 0x28;
        h = 0x10;
        g_sheet_pair_selection_y_offset = (box_size->y + 0x10) >> 1;
        gpu_DrawIndexedSheetPair(0x12, g_sheet_pair_selection_y_offset,
                                 &draw_desc, highlighted_cell_index);
        x0 = (highlighted_cell_index & 1) * 0x3A + 0x113;
        for (tile_index = 0; tile_index < 3; tile_index++) {
            SetTile(g_sheet_pair_panel_tile_cursor);
            if (tile_index == 0) {
                red = 0xFF;
                SetSemiTrans(g_sheet_pair_panel_tile_cursor, 0);
            } else {
                red = 0xFF - ((tile_index - 1) << 7);
                SetSemiTrans(g_sheet_pair_panel_tile_cursor, 1);
            }
            g_sheet_pair_panel_tile_cursor->r0 = red;
            g_sheet_pair_panel_tile_cursor->g0 = 0;
            g_sheet_pair_panel_tile_cursor->b0 = 0;
            g_sheet_pair_panel_tile_cursor->y0 =
                g_sheet_pair_selection_y_offset + 0x7C - tile_index;
            g_sheet_pair_panel_tile_cursor->x0 = x0;
            g_sheet_pair_panel_tile_cursor->w = w;
            g_sheet_pair_panel_tile_cursor->h = 1;
            AddPrim(g_gpu_ot_ptr + 8, g_sheet_pair_panel_tile_cursor);
            g_sheet_pair_panel_tile_cursor++;
        }
    }
    SetTile(g_sheet_pair_panel_tile_cursor);
    g_sheet_pair_panel_tile_cursor->r0 = 0;
    g_sheet_pair_panel_tile_cursor->g0 = 0;
    g_sheet_pair_panel_tile_cursor->b0 = 0;
    g_sheet_pair_panel_tile_cursor->x0 = 0x140 - (box_size->x >> 1);
    g_sheet_pair_panel_tile_cursor->y0 = 0x78 - (box_size->y >> 1);
    g_sheet_pair_panel_tile_cursor->w = box_size->x;
    g_sheet_pair_panel_tile_cursor->h = box_size->y + h;
    SetSemiTrans(g_sheet_pair_panel_tile_cursor, 1);
    AddPrim(g_gpu_ot_ptr + 8, g_sheet_pair_panel_tile_cursor);
    g_sheet_pair_panel_tile_cursor++;
}

extern s32 menu_HandleMemCardChoices();

s32 menu_UpdateMemCardChoices(s32 arg0, s32 pressed_buttons) {
    s32 draw_context[22];
    GpuDb *gpu_db;
    g_row_menu_pulse_counter += 1;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        draw_context, &g_menu_draw_buffer_counter);
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(draw_context, 1, gpu_db);
    menu_DrawMemCardChoices(draw_context);
    return menu_HandleMemCardChoices(&arg0, &pressed_buttons);
}

s32 func_8006C1FC(s32 a0, s32 pressed_buttons) {
    return menu_UpdateMemCardChoices(a0, pressed_buttons);
}

#define setXYWH(p, _x0, _y0, _w, _h)                                           \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0), \
    (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h), (p)->x3 = (_x0) + (_w),           \
    (p)->y3 = (_y0) + (_h)

void menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads(s32 *draw_context) {
    Unk8007352CRec s;
    s32 player_index;
    /* FAKE: reused: re-stored with the sheet list it already holds at the
     * start of phases 2, 3 and 4 (one pseudo across the function); each
     * re-store dropped: score 8 / 15 / 18, all three 28; a once-written local
     * per phase: 15 */
    Unk8009B0E0Record **table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *rect_row;
    TILE *tile;
    POLY_G4 *poly;
    /* FAKE: constant-holder (named-local-fake-exception) for the 0 passed as
     * gpu_GetSheetTPage's second argument; held in $s5 as in the target. A
     * literal 0 at every call: score 3. */
    s32 var_8;
    /* `var_9` counts the phase-2 option-bit, phase-4 sprite and phase-6
     * rectangle loops, then holds each player's menu row cursor
     * (row_bit_cursors.half[player_index], tested against 5 and used as the
     * recs subscript), the way SOTN's AluInit reuses its loop counter i for the
     * weapon id (Q51); one local per role: score 90 */
    /* SOTN: src/dra/692E8.c:299 @db41b28 */
    s32 var_9;
    /* FAKE: per-branch constant holder (named-local-fake-exception, Q27 (B)) --
     * the bar's far-corner red, 0 in two arms and 0x80 in the other two; set in
     * each arm, it is not hoisted out of the row loop. The literals: score 8.
     */
    s32 var_10;
    /* FAKE: always-zero narrow locals (named-local-fake-exception, Q27 (A)) --
     * SetDrawMode's dither and texture-window arguments and the descriptor
     * position, read up to the phase-4 head; their dead extension temps take
     * the target's four untouched frame slots (0x60-0x78). `var_14` reaches
     * SetDrawMode's RECT *tw through a
     * `(RECT *)(s32)var_14` conversion (Q113). Literals at every read:
     * score 37. */
    s16 var_11;
    s16 xpos;
    s16 ypos;
    s16 var_14;
    s32 var_15;
    s32 x;
    s32 row;
    s32 pulse_brightness;
    /* the sprite sheet's 8-byte cells, just past its 12-byte header, for
     * s.table. Ruling 9: one meaning, header + 0xC at every write. FAKE:
     * single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 12 (Q129). */
    Unk8009B400Record *cells;

    var_8 = 0;
    var_11 = 0;
    xpos = 0;
    ypos = 0;
    var_14 = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)draw_context[1])->key_config_sheets;
    s.y = ypos;
    s.x = xpos;
    s.header = table[0];
    s.semi = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = draw_context[5];
    draw_context[5] = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)draw_context[7], 1, var_11,
                gpu_GetSheetTPage(s.header, var_8), (RECT *)(s32)var_14);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    s.ot_idx = 9;
    s.has_color = 0;
    table = ((Unk8006919CRec *)draw_context[1])->key_config_sheets;
    s.y = ypos;
    for (var_15 = 0; var_15 < 2; var_15++) {
        s.x = var_15 * 280;
        if (g_row_menu_work_block->row_bit_cursors.half[var_15] < 3) {
            for (var_9 = 0; var_9 < 4; var_9++) {
                if (g_row_menu_working_record_ptr->button_mappings
                        [g_row_menu_work_block->row_bit_cursors.half[var_15]] &
                    ((1 << var_9) << (var_15 * 4))) {
                    s.header = table[var_9 + 13];
                    s.semi = 0;
                    cells = s.header->cells;
                    s.table = cells;
                    s.sprt_out = draw_context[5];
                    draw_context[5] = gpu_AddSpriteCells(&s);
                }
            }
        }
    }
    s.header = table[13];
    SetDrawMode((DR_MODE *)draw_context[7], 1, var_11,
                gpu_GetSheetTPage(s.header, var_8), (RECT *)(s32)var_14);
    AddPrim(g_gpu_ot_ptr + 9, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    s.x = xpos;
    s.y = ypos;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)draw_context[1])->key_config_sheets;
    s.header = table[1];
    s.semi = 0;
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = draw_context[5];
    draw_context[5] = gpu_AddSpriteCells(&s);
    for (var_9 = 0; var_9 < 6; var_9++) {
        s.header = table[var_9 + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        cells = s.header->cells;
        s.table = cells;
        for (player_index = 0; player_index < 2; player_index++) {
            s.x = player_index ? 280 : 0;
            s.sprt_out = draw_context[5];
            draw_context[5] = gpu_AddSpriteCells(&s);
        }
    }
    table = ((Unk8006919CRec *)draw_context[1])->key_config_sheets;
    s.header = table[1];
    SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                gpu_GetSheetTPage(s.header, var_8), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    recs = g_row_menu_work_block->menu_resource_root->rect_rows;
    tile = (TILE *)draw_context[6];
    for (var_9 = 0; var_9 < 11; var_9++) {
        rect_row = &recs[var_9];
        for (player_index = 0; player_index < 2; player_index++) {
            SetTile(tile);
            tile->r0 = rect_row->r;
            tile->g0 = rect_row->g;
            tile->b0 = rect_row->b;
            tile->x0 = rect_row->x + player_index * 280;
            tile->y0 = rect_row->y;
            tile->w = rect_row->w;
            tile->h = rect_row->h;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + 0xC, tile);
            tile++;
        }
    }
    draw_context[6] = (s32)tile;

    pulse_brightness =
        ((rcos((g_menu_draw_buffer_counter << 7) & 0xF80) * 32) >> 12) + 0xD0;
    poly = (POLY_G4 *)draw_context[4];
    x = 0;
    for (player_index = 0; player_index < 2; player_index++) {
        var_9 = g_row_menu_work_block->row_bit_cursors.half[player_index];
        rec = &recs[var_9 + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (var_9 == 5) {
                SetSemiTrans(poly, 1);
                var_10 = 0;
                poly->r0 = pulse_brightness;
                poly->g0 = pulse_brightness;
                poly->b0 = pulse_brightness;
                poly->r1 = pulse_brightness;
                poly->g1 = pulse_brightness;
                poly->b1 = pulse_brightness;
                poly->r2 = var_10;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -4 * row + 2);
            } else {
                SetSemiTrans(poly, 0);
                var_10 = 0x80;
                poly->r0 = pulse_brightness;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = pulse_brightness;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = var_10;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -2 * row + 1);
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
            SetPolyG4(poly);
            if (var_9 == 5) {
                SetSemiTrans(poly, 1);
                var_10 = 0;
                poly->r0 = pulse_brightness;
                poly->g0 = pulse_brightness;
                poly->b0 = pulse_brightness;
                poly->r2 = pulse_brightness;
                poly->g2 = pulse_brightness;
                poly->b2 = pulse_brightness;
                poly->r1 = var_10;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y3 = rec[1].y - 1;
            } else {
                SetSemiTrans(poly, 0);
                var_10 = 0x80;
                poly->r0 = pulse_brightness;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = pulse_brightness;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = var_10;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y3 = rec[1].y - 1;
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
        }
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0, 0x40, 0);
        AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)draw_context[7]);
        draw_context[7] += 0xC;
        x += 280;
    }
    draw_context[4] = (s32)poly;
}

void menu_SetExclusiveRowBit(s32 player_index, s32 pressed_buttons) {
    s32 code;
    s16 bit_row_index;
    s32 mask;

    if (pressed_buttons & (0x10 << (player_index * 16))) {
        code = 1;
    } else if (pressed_buttons & (0x40 << (player_index * 16))) {
        code = 2;
    } else if (pressed_buttons & (0x80 << (player_index * 16))) {
        code = 3;
    } else if (pressed_buttons & (0x20 << (player_index * 16))) {
        code = 0;
    }

    mask = (1 << code) << (player_index * 4);

    for (bit_row_index = 0; bit_row_index < 3; bit_row_index++) {
        if (bit_row_index ==
            g_row_menu_work_block->row_bit_cursors.half[player_index]) {
            g_row_menu_working_record_ptr->button_mappings[bit_row_index] |=
                mask;
        } else {
            g_row_menu_working_record_ptr->button_mappings[bit_row_index] &=
                ~mask;
        }
    }
}

s32 menu_HandleRowBitInput(
    s32 *arg0, s32 *pressed_buttons, s16 empty_row_sides) {
    s32 player_index;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 player_nibble_shift;
    s32 bit_row_index;
    u8 *rec;
    s32 result;
    s32 player_nibble;
    s16 row_bit_cursor;

    result = 0;
    if ((g_row_menu_work_block->row_bit_cursors.word == 0x50005) &&
        (*pressed_buttons & 0x400040)) {
        snd_QueueSound(1, 0x7F, 0x7F);
        result = 1;
    }

    player_index = 0;
    nib = 0xF;
    player_nibble_shift = 0;
    shift = 0;
    for (; player_index < 2; shift += 0x10, player_index++) {
        lim = ((empty_row_sides >> player_index) & 1) ? 4 : 5;

        if (*pressed_buttons & (0x1000 << shift)) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_row_menu_work_block->row_bit_cursors.half[player_index] <=
                0) {
                g_row_menu_work_block->row_bit_cursors.half[player_index] = lim;
            } else {
                g_row_menu_work_block->row_bit_cursors.half[player_index] =
                    g_row_menu_work_block->row_bit_cursors.half[player_index] -
                    1;
            }
        } else if (*pressed_buttons & (0x4000 << shift)) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (g_row_menu_work_block->row_bit_cursors.half[player_index] >=
                lim) {
                g_row_menu_work_block->row_bit_cursors.half[player_index] = 0;
            } else {
                g_row_menu_work_block->row_bit_cursors.half[player_index] =
                    g_row_menu_work_block->row_bit_cursors.half[player_index] +
                    1;
            }
        }

        row_bit_cursor =
            g_row_menu_work_block->row_bit_cursors.half[player_index];
        switch (row_bit_cursor) {
        case 0:
        case 1:
        case 2:
            if (*pressed_buttons & (0xF0 << shift)) {
                snd_QueueSound(0, 0x7F, 0x7F);
                menu_SetExclusiveRowBit(player_index, *pressed_buttons);
            }
            break;
        case 3:
            if (*pressed_buttons & (0x40 << shift)) {
                snd_QueueSound(1, 0x7F, 0x7F);
                for (bit_row_index = 0; bit_row_index < 3; bit_row_index++) {
                    rec = (u8 *)g_row_menu_working_record_ptr + bit_row_index;
                    player_nibble =
                        *(rec + 0x1A) & (nib << player_nibble_shift);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) &
                                          ((player_index == 0) ? 0xF0 : 0xF)) +
                                         player_nibble);
                }
            }
            break;
        case 4:
            if (*pressed_buttons & (0x40 << shift)) {
                snd_QueueSound(1, 0x7F, 0x7F);
                for (bit_row_index = 0; bit_row_index < 3; bit_row_index++) {
                    rec = (u8 *)g_row_menu_working_record_ptr + bit_row_index;
                    player_nibble =
                        *(rec + 0x1D) & (nib << player_nibble_shift);
                    *(rec + 0x17) = (u8)((*(rec + 0x17) &
                                          ((player_index == 0) ? 0xF0 : 0xF)) +
                                         player_nibble);
                }
            }
            break;
        }
        player_nibble_shift += 4;
    }
    return result;
}

/* The two per-player sprite counters: the target clears both with one word
 * store, so the storage is the counter array plus one word view (Q33); every
 * other access goes through count[]. */
typedef union {
    s16 count[2];
    s32 word;
} Counts_8006CFBC;

s32 menu_DrawKeyConfigRowBitsGetEmptyRowSides(s32 *draw_context) {
    Unk8007352CRec s;
    Counts_8006CFBC counts;
    Unk8009B0E0Record **table;
    s16 bit_row_index;
    s16 column;
    s16 player_index;
    s16 result;
    /* the sheet's cells, just past its 12-byte header, for s.table. Ruling 11:
     * holds three values, each s.header + 0xC (for table[column + 8],
     * table[12] and table[17..19]). FAKE: single-use holder of the cell
     * table ahead of the descriptor's table store; stored directly: score 8
     * (Q129). */
    Unk8009B400Record *cells;

    result = 0;
    table = ((Unk8006919CRec *)draw_context[1])->key_config_sheets;
    s.ot_idx = 8;
    s.has_color = 0;
    s.semi = 0;

    bit_row_index = 0;
    do {
        counts.word = 0;
        s.y = bit_row_index * 16;
        column = 0;
        do {
            s.header = table[column + 8];
            cells = s.header->cells;
            s.table = cells;
            for (player_index = 0; player_index < 2; player_index++) {
                if (g_row_menu_working_record_ptr
                        ->button_mappings[bit_row_index] &
                    ((1 << (player_index * 4)) << column)) {
                    s.x = player_index * 280 + counts.count[player_index] * 23;
                    s.sprt_out = draw_context[5];
                    draw_context[5] = gpu_AddSpriteCells(&s);
                    counts.count[player_index]++;
                }
            }
            column++;
        } while (column < 4);

        player_index = 0;
        do {
            if (counts.count[player_index] == 0) {
                s.x = player_index * 280;
                result |= 1 << player_index;
                s.header = table[12];
                cells = s.header->cells;
                s.table = cells;
                s.sprt_out = draw_context[5];
                draw_context[5] = gpu_AddSpriteCells(&s);
            }
            player_index++;
        } while (player_index < 2);
        bit_row_index++;
    } while (bit_row_index < 3);

    player_index = 0;
    do {
        if (g_row_menu_work_block->row_bit_cursors.word == 0x50005) {
            s.header = table[17];
        } else if ((result >> player_index) & 1) {
            s.header = table[19];
        } else {
            s.header = table[18];
        }
        s.x = player_index * 280;
        s.y = 0;
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        player_index++;
    } while (player_index < 2);

    s.header = table[14];
    SetDrawMode(
        (DR_MODE *)draw_context[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)draw_context[7]);
    draw_context[7] += 0xC;

    {
        u16 tile_rect[4];
        tile_rect[2] = 0xE1;
        tile_rect[0] = 0xCF;
        tile_rect[1] = 0x25;
        tile_rect[3] = 1;
        gpu_AddThreeGreyTilesStackedUp(draw_context, tile_rect, 0x11);
    }
    return result;
}

void menu_ResetRowBitCursors(void) {
    g_row_menu_work_block->row_bit_cursors.half[1] = 5;
    g_row_menu_work_block->row_bit_cursors.half[0] = 5;
}

extern void menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads(s32 *);
extern s32 menu_DrawKeyConfigRowBitsGetEmptyRowSides(s32 *);

s32 menu_UpdateRowBitMenu(s32 arg0, s32 pressed_buttons) {
    s32 draw_context[22];
    GpuDb *gpu_db;
    s32 empty_row_sides;
    g_row_menu_pulse_counter += 1;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        draw_context, &g_menu_draw_buffer_counter);
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(draw_context, 2, gpu_db);
    menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads(draw_context);
    empty_row_sides = menu_DrawKeyConfigRowBitsGetEmptyRowSides(draw_context);
    return menu_HandleRowBitInput(
        &arg0, &pressed_buttons, (s32)((empty_row_sides << 16) >> 16));
}

void menu_DrawStoredChoice(s32 *draw_context) {
    Unk8007352CRec s;
    u16 tile_rect[4];
    Unk8009B0E0Record **sheet_headers;
    /* FAKE: constant holder for gpu_GetSheetTPage's 0; the literal: score 8. */
    s32 var_3 = 0;
    s16 sheet_index = 0;
    /* FAKE: constant holder for s.col_r / g / b; the literal: score 25. */
    u8 var_5 = 0x40;

    s.ot_idx = 0xA;
    sheet_headers = ((Unk8006919CRec *)draw_context[1])->stored_choice_sheets;
    s.x = 0;
    s.has_color = 1;

    for (; sheet_index < 6; sheet_index++) {
        s.has_color = 1;
        if (sheet_index == 0) {
            s.y = 0;
            s.has_color = 0;
            s.semi = 0;
        } else if (sheet_index == g_menu_stored_choice_cursor + 1) {
            s.y = g_row_menu_work_block->menu_draw_offsets[1];
            s.col_r = s.col_g = s.col_b =
                ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF)
                  << 5) >>
                 12) -
                0x80;
            s.semi = 0;
        } else if (sheet_index != 1 && sheet_index != 2 && sheet_index != 3 &&
                   sheet_index == g_menu_stored_choice_cursor + 4) {
            s.col_r = s.col_g = s.col_b = 0x80;
            s.y = 0;
            s.semi = 0;
        } else {
            s.col_r = s.col_g = s.col_b = var_5;
            s.y = 0;
            s.semi = 1;
        }
        s.header = sheet_headers[sheet_index];
        s.table = s.header->cells;
        s.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s);
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                    gpu_GetSheetTPage(s.header, var_3), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
        draw_context[7] += 0xC;
    }

    tile_rect[0] = 0xDA;
    tile_rect[1] = 0x25;
    tile_rect[2] = 0xCB;
    tile_rect[3] = 1;
    gpu_AddThreeGreyTilesStackedUp(draw_context, tile_rect, 0x11);
}

s32 menu_HandleStoredChoice(s32 arg0, u32 pressed_buttons) {
    s32 merged_pressed_buttons;
    s32 result = 0;
    s32 direction_events;
    s32 sval;

    merged_pressed_buttons =
        (pressed_buttons & 0xFFFF) | (pressed_buttons >> 16);
    direction_events = pad_StepOffsetsGetDirectionEvents(
        &merged_pressed_buttons, 0, g_row_menu_work_block->menu_draw_offsets,
        g_menu_draw_offset_steps);
    if ((direction_events >> 16) == 1) {
        g_menu_stored_choice_cursor = g_menu_stored_choice_cursor + 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    } else if ((direction_events >> 16) == 2) {
        g_menu_stored_choice_cursor = g_menu_stored_choice_cursor - 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    if ((s16)g_menu_stored_choice_cursor < 0) {
        g_menu_stored_choice_cursor = 2;
    }
    g_menu_stored_choice_cursor = (s16)g_menu_stored_choice_cursor % 3;
    if (pressed_buttons & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = -1;
    } else if (pressed_buttons & 0x400040) {
        snd_QueueSound(1, 0x7F, 0x7F);
        sval = (s16)g_menu_stored_choice_cursor;
        if (sval == 2) {
            result = -1;
        } else {
            g_row_menu_working_record_ptr->unk14_17 = sval;
            result = 1;
        }
    }
    return result;
}

extern s32 menu_HandleStoredChoice(s32, u32);

s32 menu_UpdateStoredChoice(s32 arg0, s32 pressed_buttons) {
    s32 draw_context[22];
    s32 result;
    GpuDb *gpu_db;
    g_row_menu_pulse_counter += 1;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        (s32)&draw_context[0], (s32)&g_menu_draw_buffer_counter);
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(&draw_context[0], 1, gpu_db);
    menu_DrawStoredChoice(&draw_context[0]);
    result = menu_HandleStoredChoice(arg0, pressed_buttons);
    snd_FlushSoundQueue();
    return result;
}

s32 menu_ResetConfirmCursor(void) {
    g_menu_confirm_cursor = 0;
    return 1;
}

void gpu_DrawIndexedCellsAndDecimalDigitPairs(
    s32 *sprt_out_addr, s32 *dr_mode_out_addr, Unk8006D808Rec *arg2,
    s32 ot_index, s32 highlighted_row_index) {
    Unk8007352CRec s;
    /* FAKE: oversized digit array (dead-vars-local-array OVERSIZED-LOCALS):
     * only d[0]/d[1] are used; the unwritten tail brings the locals to the
     * target's 0x88 frame (s16 d[2] gives 0x80): score 26. */
    s16 d[5];
    s16 i;
    s16 k;
    s16 idx;
    s16 row_count;

    s.ot_idx = ot_index;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.has_color = 0;
    s.col_r = s.col_g = s.col_b = 0xA0;
    s.header = arg2->unk_00[2];
    for (i = 0; i < 3; i++) {
        s.header = arg2->unk_00[i];
        s.table = s.header->cells;
        s.sprt_out = *sprt_out_addr;
        *sprt_out_addr = gpu_AddSpriteCells(&s);
    }
    s.header = arg2->unk_00[0];
    SetDrawMode(
        (DR_MODE *)*dr_mode_out_addr, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, (DR_MODE *)*dr_mode_out_addr);
    *dr_mode_out_addr += 0xC;

    row_count = 4;
    if (highlighted_row_index == -1) {
        row_count = 3;
    }
    for (i = 0; i < row_count; i++) {
        idx = g_row_menu_working_record_ptr->time_records[i].character_index;
        if (idx >= 12 && idx < 22) {
            idx -= 2;
        }
        s.header = arg2->character_sheet;
        s.table = arg2->character_cells + 3 + idx * 3;
        s.x = arg2->character_x_offsets[idx];
        s.y = i * 26;
        if (i == 3) {
            s.x -= 50;
            s.y = 98;
        }
        s.sprt_out = *sprt_out_addr;
        *sprt_out_addr = gpu_AddSpriteCells(&s);
        if (idx == 8) {
            s.table = arg2->character_cells;
            s.sprt_out = *sprt_out_addr;
            *sprt_out_addr = gpu_AddSpriteCells(&s);
        }
    }

    s.header = arg2->character_sheet;
    SetDrawMode(
        (DR_MODE *)*dr_mode_out_addr, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, (DR_MODE *)*dr_mode_out_addr);
    *dr_mode_out_addr += 0xC;

    for (i = 0; i < 3; i++) {
        for (k = 0; k < row_count; k++) {
            if ((g_gpu_presented_frame_count & 1) &&
                ((k == highlighted_row_index && k < 3) || k == 3)) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            switch (i) {
            case 0:
                d[0] = d[1] =
                    g_row_menu_working_record_ptr->time_records[k].minutes;
                break;
            case 1:
                d[0] = d[1] =
                    g_row_menu_working_record_ptr->time_records[k].seconds;
                break;
            case 2:
                d[0] = d[1] =
                    g_row_menu_working_record_ptr->time_records[k].hundredths;
                break;
            }
            d[0] = d[0] % 10;
            d[1] = d[1] / 10 % 10;
            s.header = arg2->digit_sheet;
            s.header->ubase = d[0] * 24;
            s.table = arg2->digit_cell;
            s.x = i * 58 + 24;
            s.y = k * 26 + 2;
            if (k == 3) {
                s.x = i * 58 + 3;
                s.y = 100;
            }
            s.sprt_out = *sprt_out_addr;
            *sprt_out_addr = gpu_AddSpriteCells(&s);
            s.header->ubase = d[1] * 24;
            s.x -= 24;
            s.sprt_out = *sprt_out_addr;
            *sprt_out_addr = gpu_AddSpriteCells(&s);
        }
    }

    s.header = arg2->digit_sheet;
    SetDrawMode(
        (DR_MODE *)*dr_mode_out_addr, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, (DR_MODE *)*dr_mode_out_addr);
    *dr_mode_out_addr += 0xC;
}

/* FAKE: frame layout (oversized live object), as Unk80069F80Rec; the plain
 * descriptor: score 21. */
typedef struct {
    Unk8007352CRec dd94_00;
    s32 tail[2];
} Unk8006DD94Rec;

void menu_DrawConfirmSheetsAndDigitPairs(s32 *draw_context) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddSpriteCells); tail[0..1] is unwritten padding that brings the
     * locals to the target's 0x78 frame with var_1 at sp+0x50 (an 8-aligned
     * slot, BIGGEST_ALIGNMENT -- mips.h:1082) (dead-vars-local-array
     * OVERSIZED-LOCALS). The plain descriptor: score 21. */
    Unk8006DD94Rec s;
    u16 var_1[4];
    s16 sheet_index;
    Unk8006DD94List *q;
    /* FAKE: constant holder for s.dd94_00.semi and gpu_GetSheetTPage's 0; the
     * literal: score 8. */
    s32 var_4 = 0;

    s.dd94_00.ot_idx = 0xA;
    q = ((Unk8006919CRec *)draw_context[1])->confirm_list;
    s.dd94_00.x = 0;
    s.dd94_00.semi = var_4;

    for (sheet_index = 0; sheet_index < 3; sheet_index++) {
        s.dd94_00.has_color = 1;
        if (sheet_index == g_menu_confirm_cursor + 1) {
            s.dd94_00.y = g_row_menu_work_block->menu_draw_offsets[1];
            s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b =
                ((rsin(((g_row_menu_pulse_counter & 0x1F) << 7) + 0x1FF)
                  << 5) >>
                 12) -
                0x80;
        } else {
            if (sheet_index == 0) {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x80;
            } else {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x40;
            }
            s.dd94_00.y = 0;
        }
        s.dd94_00.header = q->confirm_sheets[sheet_index];
        s.dd94_00.table = s.dd94_00.header->cells;
        s.dd94_00.sprt_out = draw_context[5];
        draw_context[5] = gpu_AddSpriteCells(&s.dd94_00);
        SetDrawMode((DR_MODE *)draw_context[7], 1, 0,
                    gpu_GetSheetTPage(s.dd94_00.header, var_4), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)draw_context[7]);
        draw_context[7] += 0xC;
    }

    gpu_DrawIndexedCellsAndDecimalDigitPairs(
        &draw_context[5], &draw_context[7], &q->cell_digit_set,
        s.dd94_00.ot_idx, -1);

    var_1[2] = 0x96;
    var_1[0] = 0xF5;
    var_1[1] = 0x25;
    var_1[3] = 1;
    gpu_AddThreeGreyTilesStackedUp(draw_context, var_1, 0x11);
}

s32 menu_HandleConfirmInput(s32 arg0, u32 pressed_buttons) {
    s32 merged_pressed_buttons;
    s32 direction_events;
    s32 result = 0;

    merged_pressed_buttons =
        (pressed_buttons & 0xFFFF) | (pressed_buttons >> 16);
    direction_events = pad_StepOffsetsGetDirectionEvents(
        &merged_pressed_buttons, 0, g_row_menu_work_block->menu_draw_offsets,
        g_menu_draw_offset_steps);
    if (((direction_events >> 16) & 0xFF) != 0) {
        g_menu_confirm_cursor += 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    g_menu_confirm_cursor &= 1;
    if ((pressed_buttons & 0x100010) != 0) {
        result = -1;
        snd_QueueSound(2, 0x7F, 0x7F);
    } else if ((pressed_buttons & 0x400040) != 0) {
        snd_QueueSound(1, 0x7F, 0x7F);
        if (g_menu_confirm_cursor != 0) {
            result = -1;
        } else {
            result = 1;
        }
    }
    snd_FlushSoundQueue();
    return result;
}

extern s32 menu_HandleConfirmInput();

s32 menu_UpdateConfirmMenu(s32 arg0, s32 pressed_buttons) {
    s32 draw_context[22];
    GpuDb *gpu_db;
    g_row_menu_pulse_counter += 1;
    gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    gpu_AdvanceBufferCounterAndBuildDrawContext(
        draw_context, &g_menu_draw_buffer_counter);
    gpu_DrawModeTilesThreeSheetsAndThreeFlatQuads(draw_context, 1, gpu_db);
    menu_DrawConfirmSheetsAndDigitPairs(draw_context);
    return menu_HandleConfirmInput(arg0, pressed_buttons);
}

extern RECT g_gpu_fullscreen_image_rect;

s32 gpu_BeginFullscreenImageDisplay(void) {
    /* FAKE: read at entry and held in $s3 across the calls; read at the
     * LoadImage call: score 22 (Q129). */
    s32 temp_s3 = g_menu_arena_cursor;
    RECT rect;
    s32 file_index;

    rect = g_gpu_fullscreen_image_rect;
    if (g_row_menu_working_record_ptr->grayscale) {
        file_index = cdrom_GetFileIndex(2, 0x60);
    } else {
        file_index = cdrom_GetFileIndex(2, 7);
    }
    cdrom_StartRead(file_index, g_menu_arena_cursor);
    cdrom_WaitIdle();
    cdrom_GetFileSize(file_index);
    SetDispMask(0);
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    g_gpu_db[0].disp.isinter = 0;
    g_gpu_db[1].disp.isinter = 0;
    g_gpu_db[0].disp.isrgb24 = 0;
    g_gpu_db[1].disp.isrgb24 = 0;
    DrawSync(0);
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(temp_s3 + 0x14));
    DrawSync(0);
    PutDrawEnv(&g_gpu_db[0].draw);
    PutDispEnv(&g_gpu_db[1].disp);
    SetDispMask(1);
    return 1;
}

extern RECT g_gpu_fullscreen_image_end_clear_rect;

s32 gpu_EndFullscreenImageDisplay(void) {
    RECT rect;
    SetDispMask(0);
    PutDrawEnv(&g_gpu_db[g_menu_draw_buffer_counter & 1].draw);
    PutDispEnv(&g_gpu_db[g_menu_draw_buffer_counter & 1].disp);
    DrawSync(0);
    rect = g_gpu_fullscreen_image_end_clear_rect;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    SetDispMask(1);
    return 1;
}

Unk8006E49CRec *gpu_GetPrimBufferAndRefreshStrip(s32);

void gpu_AdvanceBufferCounterAndBuildDrawContext(
    s32 *draw_context, s32 *draw_buffer_counter) {
    s32 *s0 = draw_context;
    Unk8006E49CRec *prim_buffer;
    draw_buffer_counter[0]++;
    prim_buffer = gpu_GetPrimBufferAndRefreshStrip(draw_buffer_counter[0] & 1);
    s0[1] = (s32)g_row_menu_work_block->menu_resource_root;
    s0[3] = (s32)prim_buffer->flat_quad_prims;
    s0[2] = prim_buffer->textured_quad_prims;
    s0[4] = (s32)prim_buffer->gradient_quad_prims;
    s0[5] = prim_buffer->sprite_prims;
    s0[6] = (s32)prim_buffer->tile_prims;
    s0[7] = (s32)prim_buffer->unk_14;
    s0[8] = (s32)prim_buffer->draw_area_prims;
    D_800A3520 = prim_buffer;
    s0[9] = (s32)prim_buffer->draw_offset_prims;
}

void reloc_RelocateSelfOffsets(s32 *offset_list) {
    s32 *p = offset_list;
    if (*p == -1)
        return;
    while (*p != -1) {
        *p = *p + (s32)offset_list;
        p++;
    }
}

s32 gpu_GetSheetTPage(Unk8009B0E0Record *sheet_header, s32 a1) {
    return (sheet_header->tp0 & 0xFE1F) + (sheet_header->tp1 << 7) + a1;
}

s32 gpu_InitPrimBufferPair(s32 prim_storage, Unk8006E49CRec *prim_buffer_pair) {
    u8 *base1;
    u8 *base2;
    u8 *base3;
    u8 *base4;
    prim_buffer_pair[0].textured_quad_prims = prim_storage;
    base1 = (u8 *)prim_buffer_pair[0].textured_quad_prims + 0x9C40;
    prim_buffer_pair[0].gradient_quad_prims = base1 + 0x5DC0;
    prim_buffer_pair[0].flat_quad_prims = base1;
    base2 = base1 + 0x6838;
    prim_buffer_pair[0].tile_prims = base1 + 0x61F8;
    prim_buffer_pair[0].unk_14 = base2 + 0x1B58;
    prim_buffer_pair[0].draw_area_prims = base2 + 0x1DB0;
    prim_buffer_pair[0].draw_offset_prims = base2 + 0x1E28;
    prim_buffer_pair[0].unk_20 = base2 + 0x1EA0;
    prim_buffer_pair[0].sprite_prims = (s32)base2;
    prim_buffer_pair[1].textured_quad_prims = (s32)(base2 + 0x1FB0);
    base3 = (u8 *)prim_buffer_pair[1].textured_quad_prims + 0x9C40;
    prim_buffer_pair[1].gradient_quad_prims = base3 + 0x5DC0;
    prim_buffer_pair[1].flat_quad_prims = base3;
    base4 = base3 + 0x6838;
    prim_buffer_pair[1].tile_prims = base3 + 0x61F8;
    prim_buffer_pair[1].unk_14 = base4 + 0x1B58;
    prim_buffer_pair[1].draw_area_prims = base4 + 0x1DB0;
    prim_buffer_pair[1].draw_offset_prims = base4 + 0x1E28;
    prim_buffer_pair[1].unk_20 = base4 + 0x1EA0;
    prim_buffer_pair[1].sprite_prims = (s32)base4;
    return (s32)(base4 + 0x1FB0);
}

/* Q65: this file's initialized small data (.sdata, 0x800A32C0..0x800A32E8),
 * in address order; values from the original EXE. */
/* copied whole into a local 8-byte buffer */
u8 g_row_menu_vram_copy_source_rect[8] = {
    0x70, 0x00, 0xE1, 0x01, 0x30, 0x00, 0x01, 0x00};
u32 g_pad_direction_positive_masks[2] = {0x2000, 0x4000};
u32 g_pad_direction_negative_masks[2] = {0x8000, 0x1000};
RECT g_gpu_fullscreen_image_rect = {0, 0, 640, 240};
RECT g_gpu_fullscreen_image_end_clear_rect = {0, 0, 640, 480};

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
TILE *g_sheet_pair_panel_tile_cursor;
DR_MODE *g_sheet_pair_draw_mode_cursor;
u8 *g_sheet_pair_sprite_cursor;
s32 D_800A372C;
s32 g_sheet_pair_selection_y_offset;

/* .data 0x8009BC04..0x8009BC1C, this file's whole .data (51268.c's
 * D_8009BBFC ends at 0x8009BC04, d_8C41C.c's g_entry_menu_handlers follows):
 * the list menu's state, rows 0-7 switched on, row mask 0xBF, then the eight
 * option rows {state, mode}. Defined after every function, so no body sees a
 * definition where it saw an extern. */
MenuState g_row_menu_row_state = {
    0xFF,
    0xBF,
    {{0, 3}, {1, 3}, {2, 3}, {4, 1}, {5, 1}, {6, 2}, {3, 3}, {7, 0}},
};
