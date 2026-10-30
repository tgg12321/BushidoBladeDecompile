/* Game functions menu_HandleSelPhase0CursorInput .. StepScrollingSpriteDisplay,
 * the character-select screen among them. .text 0x800747D8 (ROM 0x64FD8). Start
 * boundary: PHASE (site 5). Ends where the PsyQ 4.0 LIBAPI C67 module starts
 * (0x80078948, LIBSCAN, Q106 D3). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern void AddPrim(void *, void *);

/* menu_HandleSelPhase0CursorInput: the duplicated `var_7 = 4;`
 * (duplicated-statement-into-arms) carries its FAKE label inline. */
/* Q65: this file's statics (.sbss), in address order. g_sel_offset_steps: one
 * {s16, s16} pair per player, passed to pad_StepOffsetsGetDirectionEvents
 * beside SelWork direction_nudge_offsets[player]. */
static s16 g_sel_offset_steps[2][2];
static Unk8006E49CRec *g_sel_prim_buffers;
static s8 g_sel_sheet_code_column_cursor;
static s32 g_menu_pending_entry_extra_slot_count;
static s32 g_option_mode_flow_state;
static s32 g_menu_compacted_unlock_mask;
static s32 D_800A35EC; /* unreferenced: size from the gap */
static s32 g_gpu_timed_sprite_elapsed_frames;
static Unk8006E49CRec *g_gpu_timed_sprite_prim_buffers;
static Ctx77D94 *g_gpu_timed_sprite_resources;
static s32 g_gpu_timed_sprite_step_count;
static s32 g_gpu_timed_sprite_sequence_phase;
static s32 D_800A3604; /* unreferenced: size from the gap */
static s32 g_gpu_scrolling_display_frame;
static Unk8006E49CRec *g_gpu_scrolling_sprite_prim_buffers;
static Unk80078824Rec *g_gpu_scrolling_sprite_resources;
static s32 D_800A3614;

s32 menu_HandleSelPhase0CursorInput(u32 pressed_buttons) {
    SelWork *base;
    s32 merged_pressed_buttons;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    u8 row;
    s32 var_7;

    base = SELWORK;
    result = 0;
    if (base->panel_slide_state.word == 0) {
        merged_pressed_buttons =
            (pressed_buttons & 0xFFFF) | (pressed_buttons >> 16);
        ret = pad_StepOffsetsGetDirectionEvents(
            (u32 *)&merged_pressed_buttons, 0, base->direction_nudge_offsets[0],
            g_sel_offset_steps[0]);
        switch (ret >> 16) {
        case 1:
            SELWORK->selection_pulse_frames = 0;
            SELWORK->f3C[0] += 1;
            if (SELWORK->f3C[0] >= 5) {
                SELWORK->f3C[0] = 0;
            }
            snd_QueueSound(0, 0x7F, 0x7F);
            break;
        case 2:
            SELWORK->selection_pulse_frames = 0;
            SELWORK->f3C[0] -= 1;
            if (SELWORK->f3C[0] < 0) {
                SELWORK->f3C[0] = 4;
            }
            snd_QueueSound(0, 0x7F, 0x7F);
            break;
        }

        state = SELWORK->f3C[0];
        switch (state) {
        case 0:
            switch (ret & 0xFF) {
            case 1:
                if (SELWORK->selected_match_count_index ==
                    SELWORK->maximum_match_count_index) {
                    SELWORK->selected_match_count_index = 0;
                } else {
                    SELWORK->selected_match_count_index += 1;
                }
                /* FAKE: `var_7 = 4;` in both arms instead of once after the
                 * label keeps jump.c from the branchless sltiu/sll store-flag
                 * fold (duplicated-statement-into-arms); one store after the
                 * label: score 8 (Q129) */
                var_7 = 4;
                goto selection_sound;
            case 2:
                if (SELWORK->selected_match_count_index == 0) {
                    SELWORK->selected_match_count_index =
                        SELWORK->maximum_match_count_index;
                } else {
                    SELWORK->selected_match_count_index -= 1;
                }
                /* FAKE: `var_7 = 4;` in both arms instead of once after the
                 * label keeps jump.c from the branchless sltiu/sll store-flag
                 * fold (duplicated-statement-into-arms); one store after the
                 * label: score 8 (Q129) */
                var_7 = 4;
                goto selection_sound;
            }
            goto confirm;
        selection_sound:
            if (SELWORK->maximum_match_count_index != 0) {
                var_7 = 0;
            }
            snd_QueueSound(var_7, 0x7F, 0x7F);
            goto confirm;
        case 1:
            if ((ret & 0xFF) != 0) {
                SELWORK->selected_win_target_index += 1;
                SELWORK->selected_win_target_index &= 1;
                SELWORK->selected_time_sheet_code =
                    g_sel_sheet_code_pairs[SELWORK->selected_win_target_index]
                                          [g_sel_sheet_code_column_cursor & 1];
                snd_QueueSound(0, 0x7F, 0x7F);
            }
            goto confirm;
        case 2:
            if ((ret & 0xFF) != 0) {
                row = SELWORK->selected_win_target_index;
                g_sel_sheet_code_column_cursor += 1;
                SELWORK->selected_time_sheet_code =
                    g_sel_sheet_code_pairs[row]
                                          [g_sel_sheet_code_column_cursor & 1];
                snd_QueueSound(0, 0x7F, 0x7F);
            }
        confirm:
            if (pressed_buttons & 0x400040) {
                snd_QueueSound(1, 0x7F, 0x7F);
                SELWORK->f3C[0] = 3;
            }
            break;
        case 3:
            if (pressed_buttons & 0x400040) {
                snd_QueueSound(1, 0x7F, 0x7F);
                for (i = 0; i < 2; i++) {
                    SELWORK->panel_slide_state.half[i] = 3;
                    SELWORK->pending_selection_phase[i] = 1;
                    SELWORK->pending_selection_cursor[i] = 0;
                }
            }
            break;
        case 4:
            if (pressed_buttons & 0x400040) {
                snd_QueueSound(1, 0x7F, 0x7F);
                result = -1;
            }
            break;
        }
        if (pressed_buttons & 0x100010) {
            snd_QueueSound(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}

void menu_DrawSelRowTiles(
    Unk8006EACCRec *draw_context, s32 player_index, s32 arg2) {
    TILE *p;
    Rec_8006C21C *t;
    s16 i;
    s16 j;
    s16 n;
    s32 ot_index;

    n = 5;
    if (arg2 != 0) {
        n = 8;
    }
    p = draw_context->primitive_cursors.tile_prims;
    for (i = 0; i < SELWORK->selected_match_count_index + 3; i++) {
        t = SELWORK->sel_resource_table->row_tile_rects;
        for (j = 0; j < n; j++) {
            SetTile(p);
            p->r0 = t->r;
            p->g0 = t->g;
            p->b0 = t->b;
            p->w = t->w;
            p->h = t->h;
            SetSemiTrans(p, 0);
            if (arg2 != 0) {
                p->x0 = t->x + player_index * 240;
                p->y0 = t->y + i * 34 + 0x2B;
            } else {
                p->x0 = t->x + player_index * 240;
                p->y0 = t->y + i * 17 + 0x7C;
            }
            ot_index = 0xB;
            if (player_index != 0) {
                ot_index = 0x15;
            }
            AddPrim(g_gpu_ot_ptr + ot_index, p);
            p++;
            t++;
        }
    }
    draw_context->primitive_cursors.tile_prims = p;
}

void menu_DrawSelPhaseSheet(
    Unk8006EACCRec *draw_context, s32 player_index, s32 sheet_index) {
    Unk8007352CRec s;
    s32 ot_index;

    ot_index = 0xC;
    s.semi = 0;
    s.has_color = 0;
    s.header = draw_context->resource_root.v80076FF8->unk_1C[(s16)sheet_index];
    s.x = player_index * 0xF0;
    s.y = 0;
    s.table = s.header->cells;
    if (player_index != 0) {
        ot_index = 0x16;
    }
    s.ot_idx = ot_index;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
}

void menu_DrawSelPanel(Unk8006EACCRec *draw_context, s32 player_index) {
    Unk8007352CRec s;
    RECT rect;
    u16 offset[2];
    Unk8009B0E0Record **records;
    TILE *prim;
    s32 var_5;
    s32 var_6;
    /* the sheet's cells, just past its 12-byte header, for s.table. Ruling 9:
     * one meaning (s.header + 0xC) at four writes. FAKE: the holder between
     * the header's cells and s.table; stored directly: score 12 (Q129). */
    Unk8009B400Record *cells;
    s16 i;

    prim = draw_context->primitive_cursors.tile_prims;
    SetTile(prim);
    SetSemiTrans(prim, 0);
    prim->r0 = 0xD0;
    prim->g0 = 0xC8;
    prim->b0 = 0xB8;
    prim->x0 = player_index * 0xF0 + 0x62;
    prim->y0 = 0x14;
    prim->w = 0xCC;
    prim->h = 0xC8;
    if (player_index != 0) {
        var_5 = 0xE;
    } else {
        var_5 = 4;
    }
    AddPrim(g_gpu_ot_ptr + var_5 + 9, prim);
    prim++;
    draw_context->primitive_cursors.tile_prims = prim;

    records = draw_context->resource_root.v80076FF8->sel_panel_sheets;
    s.semi = 0;
    s.has_color = 0;
    s.x = player_index * 0xF0;
    s.ot_idx = 2;
    i = 0;
    do {
        s.y = (0xD2 - SELWORK->panel_slide_amount[player_index]) * i;
        s.header = records[3];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        s.header = records[2];
        cells = s.header->cells;
        s.table = cells;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        s.table += s.header->count;
        s.scale_x = 0xCE00;
        s.scale_y = 0x100;
        s.ft4_out = draw_context->primitive_cursors.textured_quad_prims;
        draw_context->primitive_cursors.textured_quad_prims =
            gpu_AddScaledSpriteQuads(&s, 0);
        i++;
    } while (i < 2);

    s.x = player_index * 0xF0;
    s.y = 0;
    s.header = records[0];
    cells = s.header->cells;
    s.table = cells;
    if (player_index != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    s.header = records[1];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;

    /* FAKE: var_5 is set again (the same value) beside var_6; set once,
     * above: score 26. */
    if (player_index != 0) {
        var_5 = 0xE;
        var_6 = 0x14E;
    } else {
        var_5 = 4;
        var_6 = 0x5E;
    }
    rect.x = SELWORK->frame_gpu_db->draw.clip.x + var_6;
    rect.y = SELWORK->frame_gpu_db->draw.clip.y + 0x14;
    rect.w = 0xD4;
    rect.h = 0xC8 - SELWORK->panel_slide_amount[player_index];
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(g_gpu_ot_ptr + var_5 + 9,
            draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;

    rect.x = SELWORK->frame_gpu_db->draw.clip.x;
    rect.y = SELWORK->frame_gpu_db->draw.clip.y;
    rect.w = SELWORK->frame_gpu_db->draw.clip.w;
    rect.h = SELWORK->frame_gpu_db->draw.clip.h;
    SetDrawArea(draw_context->primitive_cursors.draw_area_prims, &rect);
    AddPrim(
        g_gpu_ot_ptr + var_5, draw_context->primitive_cursors.draw_area_prims);
    draw_context->primitive_cursors.draw_area_prims++;

    offset[0] = SELWORK->frame_gpu_db->draw.ofs[0];
    offset[1] = SELWORK->frame_gpu_db->draw.ofs[1] -
                SELWORK->panel_draw_offset_y[player_index];
    SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims, offset);
    AddPrim(g_gpu_ot_ptr + var_5 + 9,
            draw_context->primitive_cursors.draw_offset_prims);
    draw_context->primitive_cursors.draw_offset_prims++;

    offset[0] = SELWORK->frame_gpu_db->draw.ofs[0];
    offset[1] = SELWORK->frame_gpu_db->draw.ofs[1];
    SetDrawOffset(draw_context->primitive_cursors.draw_offset_prims, offset);
    AddPrim(g_gpu_ot_ptr + var_5,
            draw_context->primitive_cursors.draw_offset_prims);
    draw_context->primitive_cursors.draw_offset_prims++;
}

void menu_StepSelPanelSlides(void) {
    SelWork *base;
    s32 i;

    base = SELWORK;
    i = 0;
    do {
        switch ((u8)base->panel_slide_state.half[i]) {
        case 1:
            base->panel_draw_offset_y[i] += 0xA;
            base->panel_slide_amount[i] += 0xA;
            if (base->panel_slide_amount[i] >= 0xC8) {
                if (((u16)base->panel_slide_state.half[i] >> 8) == 0) {
                    base->panel_slide_state.half[i] += 1;
                }
                base->panel_draw_offset_y[i] = 0;
                base->panel_slide_amount[i] = 0xC8;
                base->selection_phase.half[i] =
                    base->pending_selection_phase[i];
                base->f3C[i] = base->pending_selection_cursor[i];
            }
            break;
        case 3:
            base->panel_slide_amount[i] += 0xA;
            if (base->panel_slide_amount[i] >= 0xC8) {
                base->panel_draw_offset_y[i] = 0xC8;
                base->panel_slide_amount[i] = 0xC8;
                base->selection_phase.half[i] =
                    base->pending_selection_phase[i];
                base->f3C[i] = base->pending_selection_cursor[i];
                if (((u16)base->panel_slide_state.half[i] >> 8) == 0) {
                    base->panel_slide_state.half[i] += 1;
                }
            }
            break;
        case 2:
            base->panel_slide_amount[i] -= 0xA;
            if (base->panel_slide_amount[i] <= 0) {
                base->panel_draw_offset_y[i] = 0;
                base->panel_slide_amount[i] = 0;
                base->panel_slide_state.half[i] = 0;
            }
            break;
        case 4:
            base->panel_draw_offset_y[i] -= 0xA;
            base->panel_slide_amount[i] -= 0xA;
            if (base->panel_slide_amount[i] <= 0) {
                base->panel_draw_offset_y[i] = 0;
                base->panel_slide_amount[i] = 0;
                base->panel_slide_state.half[i] = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}

/* Per-player draw of the opposed choice: the 0x2C table's sheet arg1 + 2 at
   y = selected_grid_page[arg1] * 90 plus the direction_nudge_offsets offset,
   two fixed sub-elements from the 0x14 table and two layered elements from the
   0x2C table, a SetDrawMode prim after each group. Player 1 sits one
   screen-half right; the first sheet's colour breathes with rsin of the 5-bit
   timer selection_pulse_frames. */
void menu_DrawSelOpposedChoice(Unk8006EACCRec *draw_context, s32 player_index) {
    Unk8007352CRec s;
    Unk8009B0E0Record **tbl;
    s16 i;
    /* FAKE: the holder between the header's cells and s.table; stored directly:
     * score 15. */
    Unk8009B400Record *var_3;
    /* FAKE: constant-holder: keeps the 0 for both gpu_GetSheetTPage calls in
       $s5 instead of `li $a1,0` (SOTN src/dra/7879C.c:2067 `s32 zero = 0;`);
       measured without holder: literal / for / while / declaration order /
       inline tail: 9; late semi: 12; header semi: 14; merged draw loop: 92. */
    s32 zero;

    zero = 0;
    s.semi = 0;
    if (player_index != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    tbl = draw_context->resource_root.v80076FF8->opposed_choice_sheets;
    s.header = tbl[player_index + 2];
    s.x = player_index * 240;
    var_3 = s.header->cells;
    s.table = var_3;
    s.y = SELWORK->selected_grid_page[player_index] * 90 +
          SELWORK->direction_nudge_offsets[player_index][1];
    s.col_r = s.col_g = s.col_b =
        ((rsin(((SELWORK->selection_pulse_frames & 0x1F) << 7) + 0x1FF) << 5) >>
         12) -
        0x80;
    s.has_color = 1;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    s.has_color = 0;
    tbl = draw_context->resource_root.v80076FF8->unk_14;
    i = 0;
    s.header = tbl[0];
    s.x = player_index * 240 + 0x9D;
    s.y = 0x36;
    var_3 = s.header->cells;
    s.table = var_3;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    s.y = 0x90;
    s.table += s.header->count;
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    tbl = draw_context->resource_root.v80076FF8->opposed_choice_sheets;
    do {
        s.header = tbl[i];
        s.x = player_index * 240;
        s.y = 0;
        var_3 = s.header->cells;
        if (player_index != 0) {
            s.ot_idx = 0x16;
        } else {
            s.ot_idx = 0xC;
        }
        s.table = var_3;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, zero), 0);
        AddPrim(
            g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
        i++;
        draw_context->primitive_cursors.unk_14++;
    } while (i < 2);
}

void menu_HandleSelOpposedChoice(s32 pressed_buttons, s32 player_index) {
    s16 i;
    SelWork *base;
    SelWork *work;

    base = SELWORK;
    if (base->panel_slide_state.word != 0) {
        return;
    }
    if ((pad_StepOffsetsGetDirectionEvents(
             &pressed_buttons, player_index,
             base->direction_nudge_offsets[player_index],
             g_sel_offset_steps[player_index]) >>
         16) != 0) {
        SELWORK->selection_pulse_frames = 0;
        SELWORK->selected_grid_page[player_index] =
            SELWORK->selected_grid_page[player_index] + 1;
        SELWORK->selected_grid_page[player_index] &= 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    work = SELWORK;
    work->f3C[player_index] = work->selected_grid_page[player_index];
    if (pressed_buttons & (0x40 << (player_index * 16))) {
        for (i = 0; i < 2; i++) {
            work->pending_selection_cursor[i] = 0;
            work->panel_slide_state.half[i] = 1;
            work->pending_selection_phase[i] = 2;
        }
        SELWORK->selected_grid_page[(player_index + 1) & 1] =
            (SELWORK->selected_grid_page[player_index] + 1) & 1;
        snd_QueueSound(1, 0x7F, 0x7F);
        return;
    }
    if (pressed_buttons & (0x10 << (player_index * 16))) {
        if (work->selection_phase.half[(player_index != 0) ? 0 : 1] == 1) {
            for (i = 0; i < 2; i++) {
                work->pending_selection_cursor[i] = 0;
                work->panel_slide_state.half[i] = 3;
                work->pending_selection_phase[i] = 0;
            }
            snd_QueueSound(2, 0x7F, 0x7F);
        } else {
            snd_QueueSound(4, 0x7F, 0x7F);
        }
    }
}

void menu_DrawSelGridCellOverlay(
    Unk8006EACCRec *draw_context, s32 cell_index, s32 player_index, s32 arg3) {
    Unk8007352CRec s;
    s16 cell_column;
    s16 cell_row;
    s.semi = arg3;
    s.col_r = s.col_g = s.col_b =
        ((s32)(rsin(((SELWORK->selection_pulse_frames & 0x1F) << 7) + 0x1FF) *
               0x30) >>
         12) -
        0x40;
    s.header = draw_context->resource_root.v80076FF8->unk_14[21];
    s.table = s.header->cells;
    if (cell_index < 0xA) {
        cell_column = cell_index / 5;
        cell_row = cell_index % 5;
    } else {
        cell_column = (cell_index - 0xA) / 5;
        cell_row = (cell_index - 0xA) % 5;
    }
    if (SELWORK->grid_cursor_column.half[player_index] == cell_column &&
        SELWORK->grid_cursor_row.half[player_index] == cell_row) {
        s.has_color = 1;
    } else {
        s.has_color = 0;
    }
    s.x = player_index * 0xF0 + cell_column * 0x64;
    s.y = cell_row * 16;
    if (player_index != 0) {
        s.ot_idx = 0x13;
    } else {
        s.ot_idx = 9;
    }
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
}

/* Character-select grid renderer (the draw half of menu_UpdateSelGridPicks),
 * once per player per frame: arg1 = page into g_sel_grid_page_cells, arg2 =
 * pick list (-1 = cleared), arg3 = player. Draws the page frame, the page's 10
 * character cells, the picks so far and the selected_match_count_index+3 slot
 * sprites. */
void menu_DrawSelGrid(Unk8006EACCRec *draw_context, s32 grid_page,
                      s16 *pick_slots, s32 player_index) {
    Unk8007352CRec s;
    /* the current sprite-sheet pointer table: the page table, then the slot
     * table in loop 3, then the page table again (Ruling 11) */
    Unk8009B0E0Record **table;
    /* FAKE: constant-holder: keeps gpu_GetSheetTPage's 0 argument in $fp across
       the calls; the literal: score 30 (named-local-fake-exception) */
    s32 var_2;
    s16 i;
    /* the sheet's cell array. The code treats each sheet it draws here as
       12-byte headers followed by 8-byte cells: one header for the table[0]
       page sheet (cells at +0xC), three in loops 1-3 (normal, then one cursor
       highlight per player; cells at +0x24). Loop 2 views the one-header 0x14
       placeholder sheet as three headers too, so +0x24 runs past it into
       unreferenced bytes. Ruling 9 (b'): one meaning (the cells after the
       sheet's header(s), s.header + 0xC or + 0x24) at four writes, for
       s.table. FAKE: the holder between the header's cells and s.table;
       stored directly at the page and loop-1 sites: score 25; at every site
       (loops 2 / 3 storing s.table before the header step): 39 (Q129). */
    Unk8009B400Record *cells;

    var_2 = 0;
    s.semi = 0;
    s.has_color = 0;
    table = draw_context->resource_root.v80076FF8->unk_14;
    s.header = table[0];
    s.x = player_index * 240 + 0x88;
    s.y = 0x33;
    cells = s.header->cells;
    s.table = cells;
    if (player_index != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    if (grid_page != 0) {
        s.table += s.header->count;
    }
    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
    draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, var_2), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;

    s.col_r = s.col_g = s.col_b =
        ((rsin(((SELWORK->selection_pulse_frames & 0x1F) << 7) + 0x1FF) << 5) >>
         12) -
        0x50;
    if (player_index != 0) {
        s.ot_idx = 0x14;
    } else {
        s.ot_idx = 0xA;
    }
    s.x = player_index * 240;
    for (i = grid_page * 10; i < grid_page * 10 + 10; i++) {
        /* i indexes the whole table flat (arg1 * 10 + cell). */
        /* SOTN: src/dra/62DEC.c:1091 @aa53500 */
        /* The flat read through the element pointer (the target adds i * 2
         * to the hoisted table base). Subscripted instead:
         * g_sel_grid_page_cells[0][i].cell_entry at all three reads -> score 44
         * (Q53); g_sel_grid_page_cells[arg1][i - arg1 * 10].cell_entry ->
         * score 51;
         * same row read once into entry, reused -> score 58.
         * Column loop (i = 0; i < 10; i++), passing arg1 * 10 + i:
         * g_sel_grid_page_cells[arg1][i].cell_entry at all three reads ->
         * score 53;
         * same column read once into entry, reused -> score 69. */
        u8 entry = ((Unk8009BCF8Record *)g_sel_grid_page_cells)[i].cell_entry;

        if (g_sel_entry_availability_and_claims[entry] & 1) {
            s.header = table[entry + 1];
            cells = s.header[2].cells;
            s.table = cells;
            if (g_sel_grid_page_cells
                    [grid_page]
                    [SELWORK->grid_cursor_column.half[player_index] * 5 +
                     SELWORK->grid_cursor_row.half[player_index]]
                        .cell_entry ==
                ((Unk8009BCF8Record *)g_sel_grid_page_cells)[i].cell_entry) {
                s.has_color = 1;
                s.header += 1 + player_index;
            } else {
                s.has_color = 0;
            }
            s.y = 0;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
            if (g_sel_entry_availability_and_claims[((
                    Unk8009BCF8Record *)g_sel_grid_page_cells)[i]
                                                        .cell_entry] &
                (4 << player_index)) {
                menu_DrawSelGridCellOverlay(draw_context, i, player_index, 1);
            }
        } else {
            menu_DrawSelGridCellOverlay(draw_context, i, player_index, 0);
        }
    }

    for (i = 0; i < SELWORK->f3C[player_index] + 1; i++) {
        if (pick_slots[i] >= 0) {
            s.header = table[pick_slots[i] + 1];
            cells = s.header[2].cells;
            if (i != SELWORK->f3C[player_index]) {
                s.has_color = 0;
            } else {
                s.has_color = 1;
                s.header += 1 + player_index;
            }
            s.y = i * 17;
            s.table = cells;
            s.table += s.header->count;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
        }
    }

    s.has_color = 0;
    for (i = 0; i < SELWORK->selected_match_count_index + 3; i++) {
        table = draw_context->resource_root.v80076FF8
                    ->pick_slot_sheets[SELWORK->selected_match_count_index];
        s.header = table[i];
        cells = s.header[2].cells;
        if (i == SELWORK->f3C[player_index]) {
            s.header += 1 + player_index;
        }
        s.table = cells;
        s.x = player_index * 240;
        s.y = i * 17;
        if (player_index != 0) {
            s.ot_idx = 0x14;
        } else {
            s.ot_idx = 0xA;
        }
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }

    table = draw_context->resource_root.v80076FF8->unk_14;
    s.header = table[1];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, var_2), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, var_2), 0);
    AddPrim(
        g_gpu_ot_ptr + s.ot_idx - 1, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
}

/* the shared tail of menu_UpdateSelGridPicks: the 0x14 placeholder into
 * the player's current pick slot, sound 4 if the pad bit 0x40 is set */
static inline void selgrid_StorePlaceholder(s32 pad, s16 *picks, s32 player) {
    picks[SELWORK->f3C[player]] = 0x14;
    if (pad & (0x40 << (player * 16))) {
        snd_QueueSound(4, 0x7F, 0x7F);
    }
}

/* Character-select cursor / pick handler, once per player per frame.
 *   arg0 = pad bits, both players packed (player N in bits N*16)
 *   arg1 = select page into g_sel_grid_page_cells (2 columns x 5 rows per page)
 *   arg2 = this player's pick list (character ids, -1 = cleared slot)
 *   arg3 = player index (0/1) */
void menu_UpdateSelGridPicks(
    s32 pressed_buttons, s32 grid_page, s16 *pick_slots, s32 player_index) {
    u8 *flag;
    s32 result;
    s32 bit;
    s32 i;

    if (SELWORK->panel_slide_state.half[player_index] != 0) {
        return;
    }

    if (pressed_buttons & (0x10 << (player_index * 16))) {
        snd_QueueSound(2, 0x7F, 0x7F);
        if (SELWORK->f3C[player_index] != 0) {
            pick_slots[SELWORK->f3C[player_index]] = -1;
            SELWORK->f3C[player_index]--;
            g_sel_entry_availability_and_claims
                [pick_slots[SELWORK->f3C[player_index]]] &=
                ~(4 << player_index);
            return;
        }
        /* Neither player has a pick yet (folded into one word test). */
        if (SELWORK->f3C[0] != 0 || SELWORK->f3C[1] != 0) {
            return;
        }
        if (SELWORK->selection_phase.half[(player_index != 0) ? 0 : 1] == 2) {
            SELWORK->panel_slide_state.half[1] = 3;
            SELWORK->panel_slide_state.half[0] = 3;
            SELWORK->pending_selection_phase[1] = 1;
            SELWORK->pending_selection_phase[0] = 1;
        }
        return;
    }

    result = pad_StepOffsetsGetDirectionEvents(
        &pressed_buttons, player_index,
        SELWORK->direction_nudge_offsets[player_index],
        g_sel_offset_steps[player_index]);
    if (pressed_buttons & (0xF000 << (player_index * 16))) {
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    {
        s32 low;

        low = result & 0xFF;
        if (low < 3 && low != 0) {
            SELWORK->grid_cursor_column.half[player_index] =
                (SELWORK->grid_cursor_column.half[player_index] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 value;

        value = SELWORK->grid_cursor_row.half[player_index];
        if (value == 4) {
            SELWORK->grid_cursor_row.half[player_index] = 0;
        } else {
            SELWORK->grid_cursor_row.half[player_index] = value + 1;
        }
    } break;
    case 2: {
        s16 value;

        value = SELWORK->grid_cursor_row.half[player_index];
        if (value == 0) {
            SELWORK->grid_cursor_row.half[player_index] = 4;
        } else {
            SELWORK->grid_cursor_row.half[player_index] = value - 1;
        }
    } break;
    }

    {
        u8 entry;

        entry =
            g_sel_grid_page_cells
                [grid_page][SELWORK->grid_cursor_column.half[player_index] * 5 +
                            SELWORK->grid_cursor_row.half[player_index]]
                    .cell_entry;
        flag = &g_sel_entry_availability_and_claims[entry];
        if ((*flag & 1) != 0) {
            bit = 4 << player_index;
            if ((*flag & bit) == 0) {
                u16 count;

                pick_slots[SELWORK->f3C[player_index]] = entry;
                if (pressed_buttons & (0x40 << (player_index * 16))) {
                    snd_QueueSound(1, 0x7F, 0x7F);
                    *flag |= bit;
                    count = SELWORK->f3C[player_index];
                    SELWORK->f3C[player_index] = count + 1;
                    if (SELWORK->f3C[player_index] ==
                        SELWORK->selected_match_count_index + 3) {
                        SELWORK->panel_slide_state.half[player_index] = 1;
                        SELWORK->f3C[player_index] = count;
                        SELWORK->pending_selection_cursor[player_index] = 0;
                        SELWORK->pending_selection_phase[player_index] = 3;
                        for (i = 0;
                             i < SELWORK->remaining_list_count[player_index];
                             i++) {
                            SELWORK->remaining_list_entries[player_index][i] =
                                i;
                        }
                        if (grid_page != 0) {
                            SELWORK->remaining_list_entries[player_index][4] =
                                5;
                        }
                    }
                }
                return;
            }
            selgrid_StorePlaceholder(pressed_buttons, pick_slots, player_index);
        } else {
            selgrid_StorePlaceholder(pressed_buttons, pick_slots, player_index);
        }
    }
}

void menu_DrawSelListPicks(
    Unk8006EACCRec *draw_context, s32 arg1, s16 *pick_slots, s32 player_index) {
    Unk8007352CRec s;
    s32 ot_index;
    Unk8009B0E0Record **table;
    /* the sheet's cell array, after one header or three (normal, then one
       cursor highlight per player). FAKE: the holder between the header's
       cells and s.table; stored directly: score 6. */
    Unk8009B400Record *var_3;
    s16 i;
    /* FAKE: constant-holder for gpu_GetSheetTPage's 0 argument; live past the
     * first loop it gives that loop's entry guard `move t0,zero; slt` instead
     * of a beqz. The literal: score 3 (named-local-fake-exception) */
    s32 var_5;
    u16 idx;

    var_5 = 0;
    ot_index = 10;
    s.semi = 0;
    if (player_index != 0) {
        ot_index = 20;
    }
    table = draw_context->resource_root.v80076FF8->unk_30;
    if (SELWORK->selection_phase.half[player_index] < 4) {
        s.header = table[12];
        s.has_color = 0;
        s.x = player_index * 240;
        var_3 = s.header->cells;
        s.table = var_3;
        s.y = SELWORK->f3C[player_index] * 34;
        s.ot_idx = ot_index;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, var_5), 0);
        AddPrim(
            g_gpu_ot_ptr + ot_index, draw_context->primitive_cursors.unk_14);
        draw_context->primitive_cursors.unk_14++;
    }

    s.has_color = 0;
    s.col_r = s.col_g = s.col_b =
        ((rsin(((SELWORK->selection_pulse_frames & 0x1F) << 7) + 0x1FF) << 5) >>
         12) -
        0x50;
    table = draw_context->resource_root.v80076FF8->unk_14;
    for (i = 0; i < SELWORK->selected_match_count_index + 3; i++) {
        s.has_color = 0;
        s.header = table[pick_slots[i] + 1];
        var_3 = s.header[2].cells;
        if (SELWORK->f3C[player_index] == i ||
            SELWORK->selection_phase.half[player_index] >= 4) {
            s.header += 1 + player_index;
        }
        s.table = var_3;
        s.table += s.header->count * 2;
        s.x = player_index * 240;
        s.y = i * 34;
        s.ot_idx = ot_index;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }

    table = draw_context->resource_root.v80076FF8->unk_30;
    for (i = 0; i < SELWORK->f3C[player_index] + 1; i++) {
        if (SELWORK->f3C[player_index] != i ||
            SELWORK->selection_phase.half[player_index] >= 4) {
            idx = SELWORK->picked_list_entries[player_index][i];
            s.has_color = 0;
        } else {
            idx = SELWORK->remaining_list_entries
                      [player_index]
                      [SELWORK->remaining_list_cursor[player_index]];
            s.has_color = 1;
        }
        s.header = table[(s16)idx * 2];
        var_3 = s.header[2].cells;
        if (SELWORK->f3C[player_index] == i ||
            SELWORK->selection_phase.half[player_index] >= 4) {
            s.header += 1 + player_index;
        }
        s.x = player_index * 240;
        s.table = var_3;
        s.y = i * 34;
        s.ot_idx = ot_index;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
        s.header = table[(s16)idx * 2 + 1];
        s.has_color = 0;
        var_3 = s.header->cells;
        s.table = var_3;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }

    table = draw_context->resource_root.v80076FF8
                ->pick_slot_sheets[SELWORK->selected_match_count_index];
    for (i = 0; i < SELWORK->selected_match_count_index + 3; i++) {
        s.has_color = 0;
        s.header = table[i];
        var_3 = s.header[2].cells;
        if (SELWORK->f3C[player_index] == i ||
            SELWORK->selection_phase.half[player_index] >= 4) {
            s.header += 1 + player_index;
        }
        s.table = var_3;
        s.table += s.header->count;
        s.x = player_index * 240;
        s.y = i * 34;
        s.sprt_out = draw_context->primitive_cursors.sprite_prims;
        draw_context->primitive_cursors.sprite_prims = gpu_AddSpriteCells(&s);
    }

    table = draw_context->resource_root.v80076FF8->unk_14;
    s.header = table[1];
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                gpu_GetSheetTPage(s.header, var_5), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
}

void menu_UpdateSelListPicks(
    s32 pressed_buttons, s32 grid_page, s16 *pick_slots, s32 player_index) {
    s16 i;
    s16 j;

    if (SELWORK->panel_slide_state.half[player_index] != 0) {
        return;
    }
    if (pressed_buttons & (0xA000 << (player_index * 16))) {
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    switch (pad_StepOffsetsGetDirectionEvents(
                (u32 *)&pressed_buttons, player_index,
                SELWORK->direction_nudge_offsets[player_index],
                g_sel_offset_steps[player_index]) &
            0xFF) {
    case 1:
        SELWORK->remaining_list_cursor[player_index]++;
        if (SELWORK->remaining_list_cursor[player_index] >=
            SELWORK->remaining_list_count[player_index]) {
            SELWORK->remaining_list_cursor[player_index] = 0;
        }
        break;
    case 2:
        SELWORK->remaining_list_cursor[player_index]--;
        if (SELWORK->remaining_list_cursor[player_index] < 0) {
            SELWORK->remaining_list_cursor[player_index] =
                SELWORK->remaining_list_count[player_index] - 1;
        }
        break;
    }

    if (pressed_buttons & (0x40 << (player_index * 16))) {
        if (SELWORK->f3C[player_index] <
            SELWORK->selected_match_count_index + 3) {
            snd_QueueSound(1, 0x7F, 0x7F);
            SELWORK->picked_list_entries[player_index]
                                        [SELWORK->f3C[player_index]] =
                SELWORK->remaining_list_entries
                    [player_index]
                    [SELWORK->remaining_list_cursor[player_index]];
            if (SELWORK->f3C[player_index] ==
                SELWORK->selected_match_count_index + 2) {
                SELWORK->selection_phase.half[player_index] = 4;
                return;
            }
            SELWORK->remaining_list_count[player_index]--;
            for (i = SELWORK->remaining_list_cursor[player_index];
                 i < SELWORK->remaining_list_count[player_index]; i++) {
                SELWORK->remaining_list_entries[player_index][i] =
                    SELWORK->remaining_list_entries[player_index][i + 1];
            }
            SELWORK->remaining_list_cursor[player_index] = 0;
            SELWORK->f3C[player_index]++;
        }
    } else if (pressed_buttons & (0x10 << (player_index * 16))) {
        snd_QueueSound(2, 0x7F, 0x7F);
        if (SELWORK->f3C[player_index] == 0) {
            SELWORK->panel_slide_state.half[player_index] = 3;
            SELWORK->pending_selection_phase[player_index] = 2;
            SELWORK->pending_selection_cursor[player_index] =
                SELWORK->selected_match_count_index + 2;
            g_sel_entry_availability_and_claims
                [pick_slots[SELWORK->pending_selection_cursor[player_index]]] &=
                ~(4 << player_index);
            SELWORK->remaining_list_count[player_index] = 5;
            for (i = 0; i < SELWORK->remaining_list_count[player_index]; i++) {
                SELWORK->remaining_list_entries[player_index][i] = i;
            }
            if (grid_page != 0) {
                SELWORK->remaining_list_entries[player_index][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK->remaining_list_count[player_index]; i++) {
            if (SELWORK->remaining_list_entries[player_index][i] >
                SELWORK->picked_list_entries[player_index]
                                            [SELWORK->f3C[player_index] - 1]) {
                for (j = SELWORK->remaining_list_count[player_index] - 1;
                     j >= i; j--) {
                    SELWORK->remaining_list_entries[player_index][j + 1] =
                        SELWORK->remaining_list_entries[player_index][j];
                }
                SELWORK->remaining_list_entries[player_index][i] =
                    SELWORK
                        ->picked_list_entries[player_index]
                                             [SELWORK->f3C[player_index] - 1];
                break;
            }
            if (i == SELWORK->remaining_list_count[player_index] - 1) {
                SELWORK->remaining_list_entries[player_index][i + 1] =
                    SELWORK
                        ->picked_list_entries[player_index]
                                             [SELWORK->f3C[player_index] - 1];
            }
        }
        SELWORK->remaining_list_count[player_index]++;
        SELWORK->f3C[player_index]--;
    }
}

/* Select-screen fade-out: raises SELWORK->selection_fade_shade by 8 per frame
 * and, once it reaches 0xFF, fills the result record at
 * SELWORK->sel_result_record. Draws the full-screen TILE with
 * selection_fade_shade as its colour. */
s32 menu_FadeOutSelAndWriteResult(Unk8006EACCRec *draw_context) {
    TILE *p;
    Unk8009BD24Block *working_record;
    u16 *cnt;
    s16 v;
    s16 i;
    s16 j;
    s32 var_6;
    s32 ret;

    ret = 0;
    cnt = &SELWORK->selection_fade_shade;
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        working_record = SELWORK->sel_result_record;
        working_record->match_count_index = SELWORK->selected_match_count_index;
        ret = 1;
        if (SELWORK->selected_time_sheet_code < 3) {
            var_6 = SELWORK->selected_time_sheet_code - 1;
        } else {
            var_6 = SELWORK->selected_time_sheet_code - 2;
        }
        working_record->time_limit_index = var_6;
        working_record->win_target_index = SELWORK->selected_win_target_index;
        working_record->unk14_15 =
            SELWORK->selected_grid_page[0] + SELWORK->selected_grid_page[1] * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < SELWORK->selected_match_count_index + 3; j++) {
                /* flat index into both pages, through row 0. */
                /* SOTN: src/st/e_grave_keeper.h:534 @aa53500 */
                working_record->match_picks[i][j].round_grid_pick =
                    g_sel_grid_page_cells[0][SELWORK->grid_pick_slots[i][j]]
                        .cell_pick_value;
                working_record->match_picks[i][j].unk1 =
                    SELWORK->picked_list_entries[i][j];
            }
        }
    }
    p = draw_context->primitive_cursors.tile_prims;
    SetTile(p);
    p->r0 = *cnt;
    p->g0 = *cnt;
    p->b0 = *cnt;
    p->x0 = 0;
    p->y0 = 0;
    p->w = 0x280;
    p->h = 0xF0;
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr, p);
    p++;
    draw_context->primitive_cursors.tile_prims = p;
    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, draw_context->primitive_cursors.unk_14);
    draw_context->primitive_cursors.unk_14++;
    return ret;
}

s32 menu_RelocateSelLists(s32 *resource_root) {
    reloc_RelocateOffsetList(resource_root, resource_root[5]);
    reloc_RelocateOffsetList(resource_root, resource_root[6]);
    reloc_RelocateOffsetList(resource_root, resource_root[7]);
    reloc_RelocateOffsetList(resource_root, resource_root[8]);
    reloc_RelocateOffsetList(resource_root, resource_root[9]);
    reloc_RelocateOffsetList(resource_root, resource_root[10]);
    reloc_RelocateOffsetList(resource_root, resource_root[11]);
    reloc_RelocateOffsetList(resource_root, resource_root[12]);
    reloc_RelocateOffsetList(resource_root, resource_root[13]);
    reloc_RelocateOffsetList(resource_root, resource_root[14]);
    return resource_root[1];
}

Unk8006E49CRec *menu_GetSelPrimBuffer(s32 buffer_index) {
    return &g_sel_prim_buffers[buffer_index];
}

extern s32 menu_RelocateSelLists(s32 *);

s32 menu_InitSelScreen(
    s32 prim_buffer_pair_addr, Unk8009BD24Block *working_record,
    s32 compacted_unlock_mask) {
    s16 sp[2] = {0, 0};
    /* work: the entry list pointer (arg0 + 0x58), then the work area
       gpu_InitPrimBufferPair returns (Ruling 11, Q78) */
    void *work;
    s32 r;
    s16 t0;
    s16 a2;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    work = (void *)(prim_buffer_pair_addr + 0x58);
    g_sel_prim_buffers = (Unk8006E49CRec *)prim_buffer_pair_addr;
    resource_FreeSlotsAndBlocksAndInvalidateEntryCache();
    gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(6, work);
    r = menu_RelocateSelLists(work);
    {
        Unk80076FF8Rec *list = work;
        work = (void *)gpu_InitPrimBufferPair(r, g_sel_prim_buffers);
        g_sel_screen_work = work;
        SELWORK->sel_resource_table = list;
        /* FAKE: dead store (never read) takes work out of the call result's
           cse class so the 0x30/0x34 clears use $v0, not $s1
           (dead-store-fake-exception, Q78); without it: score 2 */
        work = list;
        SELWORK->prim_buffer_flip_count = 0;
        SELWORK->selection_pulse_frames = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        /* FAKE: row pointer alias to g_sel_offset_steps, for the same-value
           re-set below (pointer-alias-fake-exception); direct
           g_sel_offset_steps with no re-set: score 19 */
        s16(*row)[2];
        a2 = 0;
        base->panel_slide_state.half[t0] = 0;
        base->panel_draw_offset_y[t0] = 0;
        base->panel_slide_amount[t0] = 0;
        base->selection_phase.half[t0] = 0;
        base->f3C[t0] = 0;
        row = g_sel_offset_steps;
        row[t0][1] = 0;
        row[t0][0] = 0;
        base->direction_nudge_offsets[t0][1] = 0;
        base->direction_nudge_offsets[t0][0] = 0;
        base->selected_grid_page[t0] = t0;
        do {
            SELWORK->grid_pick_slots[t0][a2] = -1;
            SELWORK->picked_list_entries[t0][a2] = 0;
            a2 = (s16)(a2 + 1);
        } while (a2 < 5);
        SELWORK->remaining_list_cursor[t0] = 0;
        SELWORK->remaining_list_count[t0] = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            g_sel_entry_availability_and_claims[idx] =
                (u8)(g_sel_entry_availability_and_claims[idx] & 0xF2);
            if ((compacted_unlock_mask & mask) != 0) {
                g_sel_entry_availability_and_claims[idx] =
                    (u8)(g_sel_entry_availability_and_claims[idx] | 1);
                sp[t0] += 1;
                /* FAKE: same-value dead store keeps g_sel_offset_steps's
                   lui/addiu inside the outer loop instead of hoisted
                   (dead-store-fake-exception); without it: score 18 */
                row = g_sel_offset_steps;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        SelWork *p = SELWORK;
        p->grid_cursor_row.word = 0;
        p->grid_cursor_column.word = 0;
        if (sp[0] < sp[1]) {
            p->maximum_match_count_index = sp[0] - 3;
        } else {
            p->maximum_match_count_index = sp[1] - 3;
        }
    }
    if (SELWORK->maximum_match_count_index >= 3) {
        SELWORK->maximum_match_count_index = 2;
    }
    {
        SelWork *q = SELWORK;
        q->sel_result_record = working_record;
        q->selected_match_count_index = 0;
    }
    SELWORK->selected_win_target_index = 1;
    SELWORK->selected_time_sheet_code =
        g_sel_sheet_code_pairs[SELWORK->selected_win_target_index][1];
    g_sel_sheet_code_column_cursor = 1;
    return 1;
}

/* Tail word after menu_HandleSelPhase0CursorInput's switch table. */
const u32 D_80015A20[1] = {0x00000000};

s32 menu_RunSelPhases(s32 pressed_buttons, Unk8006EACCRec *draw_context) {
    s32 ret;
    s16 i;

    ret = 0;
    if (SELWORK->selection_phase.word == 0x40004) {
        SELWORK->selection_phase.half[1] = 5;
        SELWORK->selection_phase.half[0] = 5;
        SELWORK->selection_fade_shade = 0;
    }
    gpu_DrawThreeSheetsWithOptionalTileAndOpaqueQuads(
        draw_context, SELWORK->selection_phase.half[0]);
    if (SELWORK->selection_phase.half[0] != 5) {
        menu_StepSelPanelSlides();
    }

    for (i = 0; i < 2; i++) {
        switch (SELWORK->selection_phase.half[i]) {
        case 0:
            if (i == 0) {
                ret = menu_HandleSelPhase0CursorInput(pressed_buttons);
                menu_DrawSheetsWithSelectedPulseOffsetDimmedOthersAndGreyTiles(
                    draw_context);
            }
            break;
        case 1:
            menu_HandleSelOpposedChoice(pressed_buttons, i);
            menu_DrawSelPhaseSheet(
                draw_context, i, (s16)(SELWORK->selection_phase.half[i] - 1));
            menu_DrawSelOpposedChoice(draw_context, i);
            menu_DrawSelPanel(draw_context, i);
            break;
        case 2:
            menu_UpdateSelGridPicks(
                pressed_buttons, SELWORK->selected_grid_page[i],
                SELWORK->grid_pick_slots[i], i);
            menu_DrawSelPhaseSheet(
                draw_context, i, (s16)(SELWORK->selection_phase.half[i] - 1));
            menu_DrawSelRowTiles(draw_context, i, 0);
            menu_DrawSelGrid(draw_context, SELWORK->selected_grid_page[i],
                             SELWORK->grid_pick_slots[i], i);
            menu_DrawSelPanel(draw_context, i);
            break;
        case 3:
            menu_UpdateSelListPicks(
                pressed_buttons, SELWORK->selected_grid_page[i],
                SELWORK->grid_pick_slots[i], i);
            menu_DrawSelPhaseSheet(
                draw_context, i, (s16)(SELWORK->selection_phase.half[i] - 1));
            menu_DrawSelRowTiles(draw_context, i, 1);
            menu_DrawSelListPicks(draw_context, SELWORK->selected_grid_page[i],
                                  SELWORK->grid_pick_slots[i], i);
            menu_DrawSelPanel(draw_context, i);
            break;
        case 4:
            menu_DrawSelPhaseSheet(draw_context, i, 2);
            menu_DrawSelRowTiles(draw_context, i, 1);
            menu_DrawSelListPicks(draw_context, SELWORK->selected_grid_page[i],
                                  SELWORK->grid_pick_slots[i], i);
            menu_DrawSelPanel(draw_context, i);
            if (pressed_buttons & (0x10 << (i * 16))) {
                snd_QueueSound(2, 0x7F, 0x7F);
                SELWORK->selection_phase.half[i] = 3;
            }
            break;
        case 5:
            menu_DrawSelPhaseSheet(draw_context, i, 2);
            menu_DrawSelRowTiles(draw_context, i, 1);
            menu_DrawSelListPicks(draw_context, SELWORK->selected_grid_page[i],
                                  SELWORK->grid_pick_slots[i], i);
            ret = menu_FadeOutSelAndWriteResult(draw_context);
            menu_DrawSelPanel(draw_context, i);
            break;
        }
    }
    return ret;
}

s32 menu_StepSelScreen(s32 arg0, s32 pressed_buttons) {
    Unk8006EACCRec s;
    Unk8006E49CRec *p;
    s32 temp_v1;
    SELWORK->frame_gpu_db = &g_gpu_db[g_gpu_presented_frame_count & 1];
    temp_v1 = SELWORK->prim_buffer_flip_count + 1;
    SELWORK->selection_pulse_frames = SELWORK->selection_pulse_frames + 1;
    SELWORK->prim_buffer_flip_count = temp_v1;
    p = menu_GetSelPrimBuffer(temp_v1 & 1);
    SELWORK->f2C = p;
    s.resource_root.v80076FF8 = SELWORK->sel_resource_table;
    s.primitive_cursors.textured_quad_prims = p->textured_quad_prims;
    s.primitive_cursors.flat_quad_prims = p->flat_quad_prims;
    s.primitive_cursors.gradient_quad_prims = p->gradient_quad_prims;
    s.primitive_cursors.sprite_prims = p->sprite_prims;
    s.primitive_cursors.tile_prims = p->tile_prims;
    s.primitive_cursors.unk_14 = p->unk_14;
    s.primitive_cursors.draw_area_prims = p->draw_area_prims;
    s.primitive_cursors.draw_offset_prims = p->draw_offset_prims;
    s.unk_24 = p->unk_20;
    return menu_RunSelPhases(pressed_buttons, &s);
}

s32 InitRowMenuBlackBgResetFlow(s32 prim_buffer_pair_addr) {
    menu_LoadResourcesInitStateSelectFirstEnabledRow(
        prim_buffer_pair_addr, &g_working_record);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    g_option_mode_flow_state = 0;
    return 1;
}

s32 DrawWhiteTileAndSheetsAndResetFlowOnPadMatch(void) {
    if (((s32(*)())gpu_DrawWhiteTileAndSheetsTestPadMask)() == 1) {
        g_option_mode_flow_state = 0;
        return 1;
    }
    return 0;
}

s32 menu_UpdateRowMenuStoreResult(s32 arg0, s32 pressed_buttons) {
    s32 ret;
    s32 result;

    ret = 0;
    result = menu_UpdateRowMenu(arg0, pressed_buttons);
    if (result >= 0) {
        ret = 1;
        g_option_mode_flow_state = 0;
        g_working_record.fight_mode = result;
    } else if (result == -2) {
        ret = -1;
    }
    return ret;
}

s32 ResetFlowAndFetchRowTablePair(void) {
    g_option_mode_flow_state = 0;
    g_menu_pending_entry_extra_slot_count =
        g_menu_mode_selector_and_extra_slot_counts[g_working_record.fight_mode]
                                                  [1];
    return g_menu_mode_selector_and_extra_slot_counts[g_working_record
                                                          .fight_mode][0];
}

void StoreCompactedBits(s32 unlock_mask) {
    g_menu_compacted_unlock_mask =
        (unlock_mask & 0x3FF) + ((u32)(unlock_mask & 0x3FF000) >> 2) +
        ((u32)(unlock_mask & 0x01000000) >> 4) +
        ((u32)(unlock_mask & 0x04000000) >> 5);
}

s32 InitWithCachedValueAndStoredBitsBlackBg(s32 prim_buffer_pair_addr) {
    InitDisplayStateLoadPackAndSetEntryFlags(
        prim_buffer_pair_addr, g_menu_pending_entry_extra_slot_count,
        &g_working_record, g_menu_compacted_unlock_mask);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

s32 StepThenCloseVab1OnNonzero(s32 arg0, s32 pressed_buttons) {
    s32 ret =
        StepSlideBuildFrameRecordAndDispatchStateHandler(arg0, pressed_buttons);
    if (ret) {
        snd_CloseVab1();
    }
    return ret;
}

s32 menu_ResetFlowThenRunUpdate(s32 a0, s32 pressed_buttons) {
    g_option_mode_flow_state = 0;
    return menu_UpdateStoredChoice(a0, pressed_buttons);
}

void menu_ResetFlowBackgroundAndConfirmCursor(void) {
    g_option_mode_flow_state = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    menu_ResetConfirmCursor();
}

s32 func_80077A60(s32 arg0, s32 arg1) {
    return menu_UpdateConfirmMenu(arg0, arg1);
}

s32 menu_InitSelScreen(s32, Unk8009BD24Block *, s32);

s32 menu_InitSelWithStoredBits(s32 prim_buffer_pair_addr) {
    menu_InitSelScreen(
        prim_buffer_pair_addr, &g_working_record, g_menu_compacted_unlock_mask);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

s32 func_80077AC0(s32 arg0, s32 arg1) { return menu_StepSelScreen(arg0, arg1); }

void func_80077AE0(void) { gpu_BeginFullscreenImageDisplay(); }

void func_80077B00(void) { gpu_EndFullscreenImageDisplay(); }

void SetFlowStateOne(void) { g_option_mode_flow_state = 1; }

/* ---- merged section (owner ruling Q67: one original file) ---- */
extern s32 column;

extern s32 rand(void);

s32 menu_RunOptionModeFlowStateWithRowBitEdit(s32 arg0, s32 pressed_buttons) {
    s32 return_value;
    s32 result;

    switch (g_option_mode_flow_state) {
    case 0:
        return_value = 0;
        result = menu_UpdateOptionMenuChoices(arg0, pressed_buttons);
        switch (result) {
        case 1:
            return_value = -1;
            break;
        case 2:
            g_option_mode_flow_state = 1;
            break;
        case 3:
            g_working_record.button_mappings[0] =
                g_working_record.committed_button_mappings[0];
            g_working_record.button_mappings[1] =
                g_working_record.committed_button_mappings[1];
            g_working_record.button_mappings[2] =
                g_working_record.committed_button_mappings[2];
            g_option_mode_flow_state = 4;
            break;
        }
        break;
    case 1:
        return_value = 0;
        result = func_8006C1FC(arg0, pressed_buttons);
        if (result == 1) {
            g_option_mode_flow_state = 0;
        } else if (result == 2) {
            g_option_mode_flow_state = result;
        } else if (result == 3) {
            g_option_mode_flow_state = result;
        }
        break;
    case 2:
        return_value = 2;
        break;
    case 3:
        return_value = 3;
        break;
    case 4:
        menu_ResetRowBitCursors();
        g_option_mode_flow_state++;
        /* fall through */
    case 5:
        return_value = 0;
        result = menu_UpdateRowBitMenu(arg0, pressed_buttons);
        if (result == 1) {
            g_option_mode_flow_state = 0;
            g_working_record.committed_button_mappings[0] =
                g_working_record.button_mappings[0];
            g_working_record.committed_button_mappings[1] =
                g_working_record.button_mappings[1];
            g_working_record.committed_button_mappings[2] =
                g_working_record.button_mappings[2];
        } else if (result == -1) {
            g_option_mode_flow_state = 0;
        }
        break;
    }
    return return_value;
}

Unk8009BD24Block *GetWorkingRecord(void) { return &g_working_record; }

s32 reloc_RelocateFiveOffsetListsAndGetWord1(s32 *resource_root) {
    reloc_RelocateOffsetList(resource_root, resource_root[6]);
    reloc_RelocateOffsetList(resource_root, resource_root[7]);
    reloc_RelocateOffsetList(resource_root, resource_root[8]);
    reloc_RelocateOffsetList(resource_root, resource_root[9]);
    reloc_RelocateOffsetList(resource_root, resource_root[10]);
    return resource_root[1];
}

Unk8006E49CRec *gpu_GetTimedSpritePrimBuffer(s32 buffer_index) {
    return &g_gpu_timed_sprite_prim_buffers[buffer_index];
}

extern RECT D_800A32FC;

void gpu_DrawAndAdvanceTimedSpriteSequence(Unk8006EACCRec *draw_context) {
    Unk8007352CRec s;
    RECT rect;
    s16 *in;
    s16 *out;
    Unk8009B400Record *table;
    /* FAKE: constant-holder for the 0x20 passed as gpu_GetSheetTPage's second
       argument; kept in $s7 instead of `li a1,0x20` per call; the literal:
       score 12 (named-local-fake-exception) */
    s32 var_5;
    Unk8009B0E0Record **var_6;
    s32 i;
    s32 x;
    s32 v;
    Unk80077D94Entry *w;
    s32 *slot;

    s.ot_idx = 2;
    s.y = 0x1E;
    s.semi = 0;
    in = g_gpu_timed_sprite_resources->in30;
    table = g_gpu_timed_sprite_resources->table;
    out = g_gpu_timed_sprite_resources->out34;
    var_5 = 0x20;
    if (g_gpu_timed_sprite_elapsed_frames <
            in[g_gpu_timed_sprite_sequence_phase] + 60 &&
        g_gpu_timed_sprite_elapsed_frames >=
            in[g_gpu_timed_sprite_sequence_phase]) {
        v = ((g_gpu_timed_sprite_elapsed_frames -
              in[g_gpu_timed_sprite_sequence_phase])
             << 7) /
            60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else if (g_gpu_timed_sprite_elapsed_frames >=
                   out[g_gpu_timed_sprite_sequence_phase] &&
               g_gpu_timed_sprite_elapsed_frames <
                   out[g_gpu_timed_sprite_sequence_phase] + 60) {
        v = ((60 - (g_gpu_timed_sprite_elapsed_frames -
                    out[g_gpu_timed_sprite_sequence_phase]))
             << 7) /
            60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else {
        s.has_color = 0;
    }

    if (g_gpu_timed_sprite_elapsed_frames < out[5] + 60) {
        switch (g_gpu_timed_sprite_sequence_phase) {
        case 0:
            if (g_gpu_timed_sprite_elapsed_frames + 1 >=
                out[g_gpu_timed_sprite_sequence_phase] + 60) {
                if (g_gpu_timed_sprite_elapsed_frames + 1 >=
                    in[g_gpu_timed_sprite_sequence_phase + 1]) {
                    g_gpu_timed_sprite_sequence_phase =
                        g_gpu_timed_sprite_sequence_phase + 1;
                }
            } else if (g_gpu_timed_sprite_elapsed_frames >=
                       in[g_gpu_timed_sprite_sequence_phase]) {
                var_6 = g_gpu_timed_sprite_resources->hdr18;
                for (i = 0; i < 5; var_6++, i++) {
                    s.header = *var_6;
                    s.table = table;
                    s.x = i << 7;
                    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                    draw_context->primitive_cursors.sprite_prims =
                        gpu_AddSpriteCells(&s);
                    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                                gpu_GetSheetTPage(s.header, var_5), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx,
                            draw_context->primitive_cursors.unk_14);
                    draw_context->primitive_cursors.unk_14++;
                }
            }
            break;
        case 2:
            if (g_gpu_timed_sprite_elapsed_frames + 1 >=
                out[g_gpu_timed_sprite_sequence_phase] + 60) {
                if (g_gpu_timed_sprite_elapsed_frames + 1 >=
                    in[g_gpu_timed_sprite_sequence_phase + 1]) {
                    g_gpu_timed_sprite_sequence_phase =
                        g_gpu_timed_sprite_sequence_phase + 1;
                }
                break;
            }
            var_6 = g_gpu_timed_sprite_resources->hdr24;
            for (i = 0, x = 0x156; i < 2; x += 0x80, var_6++, i++) {
                s.header = *var_6;
                s.x = x;
                s.table = table;
                s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(&s);
                SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                            gpu_GetSheetTPage(s.header, var_5), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx,
                        draw_context->primitive_cursors.unk_14);
                draw_context->primitive_cursors.unk_14++;
            }
            /* fall through */
        case 1:
            if (g_gpu_timed_sprite_elapsed_frames + 1 >=
                    in[g_gpu_timed_sprite_sequence_phase + 1] &&
                g_gpu_timed_sprite_sequence_phase == 1) {
                g_gpu_timed_sprite_sequence_phase = 2;
            }
            if (g_gpu_timed_sprite_elapsed_frames < out[2] &&
                g_gpu_timed_sprite_elapsed_frames >= in[2]) {
                s.has_color = 0;
            }
            var_6 = g_gpu_timed_sprite_resources->hdr20;
            for (i = 0, x = 0x2B; i < 2; x += 0x80, var_6++, i++) {
                s.header = *var_6;
                s.x = x;
                s.table = table;
                s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                draw_context->primitive_cursors.sprite_prims =
                    gpu_AddSpriteCells(&s);
                SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                            gpu_GetSheetTPage(s.header, var_5), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx,
                        draw_context->primitive_cursors.unk_14);
                draw_context->primitive_cursors.unk_14++;
            }
            break;
        case 3:
            rect = D_800A32FC;
            for (i = 0; i < 5; i++) {
                /* FAKE: named slot address lets loop.c strength-reduce it;
                 * inline `g_gpu_timed_sprite_resources->img38[i]`: score 9
                 * (Q129). */
                slot = &g_gpu_timed_sprite_resources->img38[i];
                LoadImage(&rect, (u32 *)(*slot + 0x220));
                DrawSync(0);
                rect.x += 0x40;
            }
            g_gpu_timed_sprite_sequence_phase =
                g_gpu_timed_sprite_sequence_phase + 1;
            break;
        case 4:
            if (g_gpu_timed_sprite_elapsed_frames + 1 <
                out[g_gpu_timed_sprite_sequence_phase] + 60) {
                var_6 = g_gpu_timed_sprite_resources->hdr1C;
                for (i = 0; i < 5; var_6++, i++) {
                    s.header = *var_6;
                    s.table = table;
                    s.x = i << 7;
                    s.sprt_out = draw_context->primitive_cursors.sprite_prims;
                    draw_context->primitive_cursors.sprite_prims =
                        gpu_AddSpriteCells(&s);
                    SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                                gpu_GetSheetTPage(s.header, var_5), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx,
                            draw_context->primitive_cursors.unk_14);
                    draw_context->primitive_cursors.unk_14++;
                }
            }
            break;
        }
    }

    s.ot_idx = 1;
    s.y = 0;
    s.x = 0;
    var_6 = g_gpu_timed_sprite_resources->hdr28;
    for (i = 0; i < 21; i++) {
        /* FAKE: integer-sum address gives the target's addu operand order;
           `&g_gpu_timed_sprite_resources->nar2C[i]`: score 1; `w = nar2C; w +=
           i`: 2 */
        w = (Unk80077D94Entry *)(i * 4 +
                                 (s32)g_gpu_timed_sprite_resources->nar2C);
        if (g_gpu_timed_sprite_elapsed_frames < w->off + 60 &&
            g_gpu_timed_sprite_elapsed_frames >= w->on) {
            if (g_gpu_timed_sprite_elapsed_frames < w->on + 60) {
                s.has_color = 1;
                v = ((g_gpu_timed_sprite_elapsed_frames - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (g_gpu_timed_sprite_elapsed_frames >= w->off &&
                       g_gpu_timed_sprite_elapsed_frames < w->off + 60) {
                /* FAKE: named intermediate keeps cnt - 60 computed first (fold
                 * reassociates it inline: score 16) (Q129). */
                s32 cnt_60ago = g_gpu_timed_sprite_elapsed_frames - 60;
                s.has_color = 1;
                v = ((w->off - cnt_60ago) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else {
                s.has_color = 1;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = 0x72;
            }
            s.header = var_6[i];
            s.table = s.header->cells;
            s.sprt_out = draw_context->primitive_cursors.sprite_prims;
            draw_context->primitive_cursors.sprite_prims =
                gpu_AddSpriteCells(&s);
            SetDrawMode(draw_context->primitive_cursors.unk_14, 1, 0,
                        gpu_GetSheetTPage(s.header, var_5), 0);
            AddPrim(g_gpu_ot_ptr + s.ot_idx,
                    draw_context->primitive_cursors.unk_14);
            draw_context->primitive_cursors.unk_14++;
        }
    }
}

s32 InitTimedSpriteSequence(s32 prim_buffer_pair_addr) {
    Ctx77D94 *resource_root;
    s32 prim_storage;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    resource_root = (Ctx77D94 *)(prim_buffer_pair_addr + 0x58);
    g_gpu_timed_sprite_prim_buffers = (Unk8006E49CRec *)prim_buffer_pair_addr;
    g_gpu_timed_sprite_resources = resource_root;
    gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
        0x32, &resource_root->resource_head);
    prim_storage =
        reloc_RelocateFiveOffsetListsAndGetWord1((s32 *)resource_root);
    gpu_InitPrimBufferPair(prim_storage, g_gpu_timed_sprite_prim_buffers);
    g_gpu_timed_sprite_step_count = 0;
    g_gpu_timed_sprite_elapsed_frames = 0;
    g_gpu_timed_sprite_sequence_phase = 0;
    return 1;
}

extern void gpu_DrawAndAdvanceTimedSpriteSequence(Unk8006EACCRec *);

s32 StepTimedSpriteSequenceWithSkip(s32 pressed_buttons) {
    Unk8006EACCRec draw_context;
    Unk8006E49CRec *prim_buffer;
    s32 step_count;
    step_count = g_gpu_timed_sprite_step_count + 1;
    g_gpu_timed_sprite_step_count = step_count;
    prim_buffer = gpu_GetTimedSpritePrimBuffer(step_count & 1);
    draw_context.primitive_cursors.textured_quad_prims =
        prim_buffer->textured_quad_prims;
    draw_context.primitive_cursors.gradient_quad_prims =
        prim_buffer->gradient_quad_prims;
    draw_context.primitive_cursors.sprite_prims = prim_buffer->sprite_prims;
    draw_context.primitive_cursors.tile_prims = prim_buffer->tile_prims;
    draw_context.primitive_cursors.unk_14 = prim_buffer->unk_14;
    draw_context.primitive_cursors.draw_area_prims =
        prim_buffer->draw_area_prims;
    draw_context.primitive_cursors.draw_offset_prims =
        prim_buffer->draw_offset_prims;
    if (g_gpu_timed_sprite_elapsed_frames > 0 && (pressed_buttons & 0x40)) {
        return 1;
    }
    gpu_DrawAndAdvanceTimedSpriteSequence(&draw_context);
    {
        s16 *out = g_gpu_timed_sprite_resources->out34;
        s32 new_val = g_gpu_timed_sprite_elapsed_frames + 1;
        int cond = new_val < out[5];
        g_gpu_timed_sprite_elapsed_frames = new_val;
        return cond ? 0 : 1;
    }
}

s32 GetScrollingSpriteRecordWord1(Unk80078824Rec *resource_root) {
    return resource_root->resource_head.free_ram_start;
}

Unk8006E49CRec *gpu_GetScrollingSpritePrimBuffer(s32 buffer_index) {
    return &g_gpu_scrolling_sprite_prim_buffers[buffer_index];
}

void gpu_DrawScrollingAndDelayedSheets(Unk800788B0Rec *primitive_cursors) {
    Unk8007352CRec s;
    Unk8009B0E0Record **sheet_headers;
    s32 i;
    /* FAKE: constant-holder keeps the 0 argument in a register across the
       calls; the literal: score 7 */
    s32 var_4;

    var_4 = 0;
    s.ot_idx = 2;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.header = g_gpu_scrolling_sprite_resources->sheet_header_list[10];
    s.y = 0;
    s.table = s.header->cells;
    sheet_headers = g_gpu_scrolling_sprite_resources->sheet_header_list;
    if (g_gpu_scrolling_display_frame >= 0xAAA) {
        if (g_gpu_scrolling_display_frame >= 0xB04) {
            s16 brightness;
            s.has_color = 1;
            brightness =
                0x80 - (((g_gpu_scrolling_display_frame - 0xB04) << 7) / 15);
            if (brightness < 0) {
                brightness = 0;
            }
            s.col_r = s.col_g = s.col_b = brightness;
        }
        s.sprt_out = primitive_cursors->sprite_prims;
        primitive_cursors->sprite_prims = gpu_AddSpriteCells(&s);
        SetDrawMode(primitive_cursors->unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, var_4), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, primitive_cursors->unk_14);
        primitive_cursors->unk_14++;
    }
    s.has_color = 0;
    for (i = 0; sheet_headers[i + 1] != (Unk8009B0E0Record *)-1; i++) {
        s.header = sheet_headers[i];
        s.table = s.header->cells;
        s.y = -g_gpu_scrolling_display_frame;
        s.sprt_out = primitive_cursors->sprite_prims;
        primitive_cursors->sprite_prims = gpu_AddSpriteCells(&s);
        SetDrawMode(primitive_cursors->unk_14, 1, 0,
                    gpu_GetSheetTPage(s.header, var_4), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, primitive_cursors->unk_14);
        primitive_cursors->unk_14++;
    }
}

extern s32 g_gpu_scrolling_sprite_step_count;

s32 InitScrollingSpriteDisplay(s32 prim_buffer_pair_addr) {
    Unk80078824Rec *resource_root;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    resource_root = (Unk80078824Rec *)(prim_buffer_pair_addr + 0x58);
    g_gpu_scrolling_sprite_prim_buffers =
        (Unk8006E49CRec *)prim_buffer_pair_addr;
    g_gpu_scrolling_sprite_resources = resource_root;
    gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip(
        0x5F, &resource_root->resource_head);
    gpu_InitPrimBufferPair(GetScrollingSpriteRecordWord1(resource_root),
                           g_gpu_scrolling_sprite_prim_buffers);
    g_gpu_scrolling_sprite_step_count = 0;
    g_gpu_scrolling_display_frame = 0;
    D_800A3614 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

void gpu_DrawScrollingAndDelayedSheets(Unk800788B0Rec *);

s32 StepScrollingSpriteDisplay(void) {
    Unk800788B0Rec primitive_cursors;
    Unk8006E49CRec *prim_buffer;
    g_gpu_scrolling_sprite_step_count++;
    prim_buffer =
        gpu_GetScrollingSpritePrimBuffer(g_gpu_scrolling_sprite_step_count & 1);
    primitive_cursors.textured_quad_prims = prim_buffer->textured_quad_prims;
    primitive_cursors.gradient_quad_prims = prim_buffer->gradient_quad_prims;
    primitive_cursors.sprite_prims = prim_buffer->sprite_prims;
    primitive_cursors.tile_prims = prim_buffer->tile_prims;
    primitive_cursors.unk_14 = prim_buffer->unk_14;
    primitive_cursors.draw_area_prims = prim_buffer->draw_area_prims;
    primitive_cursors.draw_offset_prims = prim_buffer->draw_offset_prims;
    gpu_DrawScrollingAndDelayedSheets(&primitive_cursors);
    g_gpu_scrolling_display_frame++;
    return g_gpu_scrolling_display_frame >= 0xB40;
}

/* Q65: this file's initialized small data (.sdata), in address order. */
RECT D_800A32FC = {640, 0, 64, 184};
s32 g_gpu_scrolling_sprite_step_count = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 *g_sel_screen_work;
