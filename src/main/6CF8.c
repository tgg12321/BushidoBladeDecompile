/* The game's boot and main loop: main(), the PCdrv host-file loaders
 * (pcdrv_LoadFile, pcdrv_LoadSectors), display setup wrappers, rng_SetSeed /
 * rng_Next and scratchpad_Save / scratchpad_Restore. .text 0x800164F8 (ROM
 * 0x6CF8). Start boundary: the first game .text object (LEGACY: a splat segment
 * edge). */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "bb2_const.h"
#include <psxsdk/libgte.h>
#include <psxsdk/libsn.h>

extern u8 g_eff_initialized;
extern s32 g_rng_state;
extern RECT g_gpu_clear_rect;
extern u32 g_scratchpad_save;
/* This file's .rodata: debug format strings, and the date string
 * ("Fri Aug  7 22:26:32 1998") that D_800A30E0 (below) points at. */
const char g_str_overflow[12] = "OVER FLOW\n";
const char g_str_eff_init[28] = "eff_init:%08x size:%08x\n";
const char g_str_limit[12] = "LIMIT:%08x\n";
const char g_str_prim_overflow[24] = "common prim over flow\n";
const char D_8001004C[28] = "Fri Aug  7 22:26:32 1998\n";

extern void func_800164F8(void);

extern u8 g_softbody_slot_high_water;
extern RECT g_gpu_cd_file_image_rect;
extern s32 g_main_prim_buffer_min_remaining;

extern void __main(void);
/* Not the bb2.h spelling: the definition takes an s16 first parameter, but
 * this call passes it unextended (an s16 prototype adds sll/sra here). */
extern s32 gpu_DrawSpriteAtPackedTablePosition(s32, s32, s32);

/* func_800164F8 -- polls the PC host 10000 times: pollhost() is PsyQ LIBSN.H's
 * `break 1024`, which ASPSX (and maspsx) encode as `break 1,0` (0x0001000D). */
void func_800164F8(void) {
    s32 i;

    for (i = 9999; i >= 0; i--) {
        pollhost();
    }
}

s32 pcdrv_LoadFile(char *a0, u8 *dest) {
    s32 fd;
    s32 total;
    s32 remaining;
    s32 chunk;

    fd = PCopen(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    total = PClseek(fd, 0, 2);
    remaining = total;
    PClseek(fd, 0, 0);
    if (total > 0) {
        do {
            chunk = 0x4000;
            if (remaining < 0x4001) {
                chunk = remaining;
            }
            if (PCread(fd, dest, chunk) != chunk) {
                close(fd);
                return -1;
            }
            remaining -= chunk;
            dest += chunk;
        } while (remaining > 0);
    }
    PCclose(fd);
    return total;
}

s32 pcdrv_LoadSectors(char *a0, u8 *dest, s32 sector_offset, s32 count) {
    s32 fd;
    s32 i;

    fd = PCopen(a0 + 4, 0, 0);
    if (fd == -1) {
        return -2;
    }
    PClseek(fd, sector_offset << 11, 0);
    for (i = 0; i < count; i++) {
        if (PCread(fd, dest, 0x800) != 0x800) {
            close(fd);
            return -1;
        }
        dest += 0x800;
    }
    PCclose(fd);
    return count << 11;
}

s32 math_FovToScreenDist(s32 fov_degrees) {
    s32 fov_angle = (fov_degrees << 12) / 360;
    s32 half_fov_angle = fov_angle / 2;
    s16 cos_val = Judge[(half_fov_angle + 0x400) & 0xFFF];
    s16 sin_val = Judge[half_fov_angle & 0xFFF];
    return (cos_val * 320) / sin_val;
}

void gpu_SetDrawEnvBg(s32 clear_bg, s32 a1, s32 a2, s32 a3) {
    s32 i;

    for (i = 0; i < 2; i++) {
        g_gpu_db[i].draw.isbg = clear_bg;
        setRGB0(&g_gpu_db[i].draw, a1, a2, a3);
    }
}

u32 gpu_GetSavedGrayscaleFlag(void) { return g_saved_record.flags & 1; }

u32 fighter_GetSavedCpuBehaviorAdjustFlag(void) {
    return (g_saved_record.flags >> 1) & 1;
}

u32 snd_GetSavedCdAudioSuppressFlag(void) {
    return (g_saved_record.flags >> 2) & 1;
}

void InitSavedRecordAndResetCameraDistanceOffset(void) {
    s32 i;
    /* FAKE: second handle to g_saved_record; using the global directly
     * everywhere: score 19; final times[0].time_frames store through var_1:
     * score 10
     */
    /* SOTN: src/st/st0/2A218.c:48 @db41b28 */
    FileRecord *var_1 = &g_saved_record;

    g_camera_distance_offset = 0;
    var_1->flags = 0;
    var_1->unlock_mask = 0x7007;
    var_1->staff_roll_character_mask = 0;
    for (i = 0; i < 3; i++) {
        var_1->times[i].holder_character_index = 0;
        var_1->times[i].holder_weapon_id = 0;
        var_1->times[i].time_frames = 0x1A5E0;
    }
    g_saved_record.times[0].time_frames = 0x6978;
    pad_SetDefaultSavedButtonMappings();
}

void gpu_ResetGraphMode1(void) { ResetGraph(1); }

void gpu_HideResetAndClear(void) {
    SetDispMask(0);
    ResetGraph(1);
    ClearImage(&g_gpu_clear_rect, 0, 0, 0);
    DrawSync(0);
}

void gpu_SetDispMaskOn(void) { SetDispMask(1); }

void func_800168F0(void) {}

void rcnt_StartCnt1Wrapper(void) { rcnt_StartCnt1(); }

extern void SetGraphDebug(s32);

void disp_Init(void) {
    ResetGraph(0);
    SetGraphDebug(0);
    SetDispMask(0);
    InitGeom();
    SetGeomOffset(0x140, 0x78);
    SetGeomScreen(math_FovToScreenDist(0x2D));
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    ClearImage(&g_gpu_clear_rect, 0, 0, 0);
    DrawSync(0);
}

extern void InitPAD(u8 *, s32, u8 *, s32);

void sys_Init(void) {
    ResetCallback();
    InitPAD((u8 *)g_pad_buf[0], 8, (u8 *)g_pad_buf[1], 8);
    StartPAD();
    ChangeClearPAD(0);
    disp_Init();
    g_local_ot_draw_mode = DISP_DISABLED;
    g_gpu_jitter_burst_enabled = 0;
    cdrom_Init();
    memcard_Init();
    rcnt_StartCnt1Wrapper();
}

void gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth(
    u8 *file_buffer, void *arg1, s32 arg2) {
    RECT rect;
    s32 i;

    rect = g_gpu_cd_file_image_rect;

    SetDispMask(0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x140, 0xF0);
    cdrom_WaitIdle();
    cdrom_StartRead(cdrom_GetFileIndex(2, 0x61), (s32)file_buffer);
    cdrom_WaitIdle();
    PutDispEnv(&g_gpu_db[1].disp);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(file_buffer + 0x14));
    DrawSync(0);
    SetDispMask(1);

    for (i = 0; i < 0x96; i++) {
        if (i >= 0x79) {
            u16 *pixel_cursor = (u16 *)(file_buffer + 0x28);
            s32 j;

            for (j = 0; j < rect.w * rect.h; j++, pixel_cursor++) {
                u16 c = *pixel_cursor;
                s32 r = c & 0x1F;

                *pixel_cursor = (r >> 1) + ((((c >> 5) & 0x1F) >> 1) << 5) +
                                ((((c >> 10) & 0x1F) >> 1) << 10);
            }
            LoadImage(&rect, (u32 *)(file_buffer + 0x14));
        }

        DrawSync(0);
        VSync(0);
    }

    SetDispMask(0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    SetDispMask(1);
}

void sys_Panic(void) {
    printf(g_str_overflow);
    while (1) {
        func_800164F8();
    }
}

void eff_ClearInitFlag(void) { g_eff_initialized = 0; }

void eff_Init(void) {
    s32 size;

    if (g_eff_initialized != 0) {
        return;
    }
    size = eff_LoadImagesAndSetup(0x801D8800, 0x8010E800);
    printf(g_str_eff_init, 0x8010E800, size);
    if (0xA000 < size) {
        sys_Panic();
    }
    g_eff_initialized = 1;
}

extern void memcpy(u32, u32, s32);
extern s32 snd_VabFakeOpen(s32, s16);

void snd_InitAndLoadCommonVab(void) {
    s32 size;

    snd_Init();
    size = snd_LoadCommonVab(0x801D8800);
    if (size >= 0xD01) {
        sys_Panic();
    }
    memcpy(0x8010DB00, 0x801D8800, size);
    snd_VabFakeOpen(0xFFF35300, 0);
    snd_StopVoicesSetMaxMasterVolumeStereoAndClearAutoKeyOffMode();
    g_snd_initialized = 1;
}

extern u32 D_800A3798;
extern u8 D_800A3744;
extern u8 D_800A3745;
extern u8 D_800A3746;
extern u8 D_800A36B0;

void func_80016D78(void) {
    printf(g_str_limit, 0x8010DB00);
    InitSavedRecordAndResetCameraDistanceOffset();
    motion_InitBuffers();
    D_800A3770[0] = 0x801D8800;
    D_800A3770[1] = 0x801EBC00;
    D_800A3798 = 0x13400;
    g_eff_initialized = 0;
    g_snd_initialized = 0;
    snd_InitAndLoadCommonVab();
    pad_ResetStateMarkValid();
    font_LoadImage();
    fight_InitSettings();
    D_800A36F9 = 0;
    g_fight_view_option = 0;
    D_800A3744 = 0;
    D_800A3745 = 0;
    D_800A3746 = 0;
    func_80046B44();
    g_main_frame_period = 2;
    g_motion_fighter_loaded_data_ids[1] = 0;
    D_800A36B0 = 0;
    g_comb_abort_request = 0;
}

void gpu_WaitDrawSync(void) { DrawSync(0); }

void menu_RunPauseMenu(GpuDb *arg0, s32 arg1) {
    u32 local_ot[2];
    GpuDb *var_1;
    s32 var_2;
    s32 has_toggle_rows;
    s32 limit;
    u32 prim_buffer_addr;
    s32 idx;
    GpuDb *var_7;

    var_2 = 0;
    has_toggle_rows = 0;
    /* FAKE: pass-through handle on the parameter, used once at DrawOTag;
       moves the prologue's $s5 save/init group from first to third. Removed:
       score 4; removed with the do-while wrapper: score 25
       (pointer-alias-fake-exception) */
    var_7 = arg0;
    if (g_fight_mode_selector == 2) {
        has_toggle_rows = D_800A389A < 1;
    }
    limit = 3;
    if (has_toggle_rows != 0) {
        limit = 6;
    }

    D_800A36B0 = 1;
    snd_QueueSound(3, 0x7F, 0x7F);
    prim_buffer_addr = D_800A3770[g_gpu_presented_frame_count & 1];

    while (1) {
        idx = g_gpu_presented_frame_count & 1;
        g_prim_buf_cursor = (u32 *)(prim_buffer_addr + (idx * 0x9A00));
        g_gpu_ot_ptr = &local_ot[idx];
        var_1 = &g_gpu_db[idx];

        ClearOTagR(g_gpu_ot_ptr, 1);
        pad_Update();
        if (has_toggle_rows != 0) {
            gpu_DrawModeSizedPanelWithSelectedCellOpaque(
                2, var_2 | (g_fight_toggle_option_bits << 16),
                (s32)g_prim_buf_cursor, 0);
        } else {
            gpu_DrawModeSizedPanelWithSelectedCellOpaque(
                0, var_2, (s32)g_prim_buf_cursor, 0);
        }
        cdrom_Update();
        snd_FlushSoundQueue();
        DrawSync(0);
        VSync(2);
        /* FAKE: do-while(0) raises var_1's loop-depth weight above var_2's,
           seating var_1 in $s0 and var_2 in $s1; unwrapped: score 21.
           Unwrapped trials: buffer-first/inline idx/pointer base/for: 21;
           cursor if/else: 23; preincrement/decrement wrap: 24;
           post-loop switch: 28; buffer-derived indices: 45; var_1 as
           g_gpu_db + idx or declared first / last: 21; set just before
           PutDispEnv: 65; no var_1 (g_gpu_db[idx] at both calls): 73
           (do-while-zero-exception) */
        do {
            PutDispEnv(&var_1->disp);
            PutDrawEnv(&var_1->draw);
        } while (0);
        DrawOTag(&var_7->ot[0x1007]);
        DrawOTag(g_gpu_ot_ptr);
        g_gpu_presented_frame_count++;

        if (g_pad_state.pressed & 0x100010) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_2 = 0;
            break;
        }
        if (g_pad_state.pressed & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            break;
        }
        if (g_pad_state.pressed & 0x10001000) {
            snd_QueueSound(0, 0x7F, 0x7F);
            var_2 = (var_2 == 0) ? limit - 1 : var_2 - 1;
        } else if (g_pad_state.pressed & 0x40004000) {
            snd_QueueSound(0, 0x7F, 0x7F);
            var_2 = (var_2 == limit - 1) ? 0 : var_2 + 1;
        }

        if ((has_toggle_rows != 0) && (var_2 >= 3)) {
            if (g_pad_state.pressed & 0x80008000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                g_fight_toggle_option_bits |= 1 << (var_2 - 3);
            } else if (g_pad_state.pressed & 0x20002000) {
                snd_QueueSound(0, 0x7F, 0x7F);
                g_fight_toggle_option_bits &= ~(1 << (var_2 - 3));
            }
        }
    }

    if (var_2 != 0) {
        if (var_2 == 1) {
            g_menu_mode_subflow_init_pending = 1;
            g_main_loop_handler_index = 8;
            cdrom_PauseAndWait();
        } else if (var_2 == 2) {
            g_main_loop_handler_index = 8;
            cdrom_PauseAndWait();
        }
    }

    DrawSync(0);
    ResetRCnt(0xF2000001);
    D_800A36B0 = 1;
}

void rng_SetSeed(s32 seed) { g_rng_state = seed; }

s32 rng_Next(void) {
    s32 var_0 = g_rng_state;
    s32 result = var_0 * 5497 + 0x7FA9;
    var_0 = (var_0 >> 16) ^ result;
    g_rng_state = var_0;
    return var_0 & 0x7FFF;
}

void main(void) {
    s32 idx;
    GpuDb *var_1;
    u32 *frame_ot;
    /* g_main_present_skip_frames (frames left to skip presenting) as read this
     * frame; the next frame passes it to pad_Update. */
    s32 present_skip_frames;

    __main();
    SetSp(0x801FFF00);
    SetMem(2);
    sys_Init();
    func_80016D78();
    SetDispMask(1);
    gpu_ShowCdFileImageFadeByHalvingColorsAndRestoreDisplayWidth(
        (u8 *)0x80118800, var_1, idx);

    g_main_loop_handler_index = 0xF;
    g_main_present_skip_frames = 0;
    g_gpu_presented_frame_count = 0;

    for (;;) {
        idx = g_gpu_presented_frame_count & 1;
        var_1 = &g_gpu_db[idx];
        frame_ot = var_1->ot;
        ClearOTagR(frame_ot, 0x1008);
        g_gpu_ot_ptr = frame_ot;
        g_prim_buf_cursor = (u32 *)D_800A3770[idx];
        eff_SelectQuadBuffer(idx);
        font_BeginFrame();
        pad_Update(present_skip_frames);
        cdrom_Update();
        snd_FlushSoundQueue();

        if (g_comb_abort_request != 0) {
            cdrom_PauseAndWait();
            g_local_ot_draw_mode = DISP_DISABLED;
            g_comb_abort_request = 0;
            g_menu_mode_subflow_init_pending = 0;
            g_main_loop_handler_index = 8;
        }

        g_main_loop_handlers[g_main_loop_handler_index]();
        font_AddTpage();

        do {
            if (GetRCnt(0xF2000001u) >= ((g_main_frame_period - 1) << 8) + 0x80)
                break;
            rand();
        } while (1);

        VSync(1);
        DrawSync(0);
        VSync(0);
        ResetRCnt(0xF2000001u);

        present_skip_frames = g_main_present_skip_frames;
        if (present_skip_frames == 0) {
            PutDispEnv(&var_1->disp);
            PutDrawEnv(&var_1->draw);
        }

        {
            s32 remaining = D_800A3770[idx] + 0x13400 - (s32)g_prim_buf_cursor;
            if (remaining < g_main_prim_buffer_min_remaining) {
                g_main_prim_buffer_min_remaining = remaining;
            }
            if (remaining < 0) {
                printf(g_str_prim_overflow);
                while (1) {
                    func_800164F8();
                }
            }
        }

        if (g_main_present_skip_frames != 0) {
            g_main_present_skip_frames--;
        } else {
            DrawOTag(&var_1->ot[0x1007]);
            g_gpu_presented_frame_count++;
        }

        if (g_main_loop_handler_index != 1)
            continue;
        if (present_skip_frames != 0)
            continue;
        if (!(g_pad_state.pressed & 0x08000800u)) {
            if (g_fight_mode_selector != 2)
                continue;
            if (g_fight_auto_pause_countdown == 0)
                continue;
            g_fight_auto_pause_countdown--;
            if (g_fight_auto_pause_countdown != 0)
                continue;
        }
        menu_RunPauseMenu(var_1, idx);
    }
}

void gpu_DrawDisplayModeOnLocalOtAndSync(void) {
    u32 local_ot[2];
    DRAWENV env;
    /* holds two values: the case-1/2 fade loop's iteration count, then the
     * case-20 g_fight_stage_list_entries[] code passed to
     * gpu_DrawSpriteAtPackedTablePosition (owner ruling 11) */
    s32 temp;
    s32 draw_chunk_addr;
    /* holds two values: the g_local_ot_draw_mode switch selector, then the
     * case-20 g_fight_stage_command_count limit (owner ruling 11) */
    s32 temp2;

    draw_chunk_addr = (s32)g_shared_buffer;
    if (g_local_ot_draw_mode == DISP_DISABLED) {
        return;
    }
    SetDefDrawEnv(
        &env, 0, (g_gpu_presented_frame_count & 1) ? 0xF0 : 0, 0x280, 0xF0);
    env.isbg = 0;
    PutDrawEnv(&env);
    g_gpu_ot_ptr = local_ot;
    ClearOTagR(local_ot, 2);
    temp2 = g_local_ot_draw_mode;
    switch (temp2) {
    case 1:
    case 2:
        draw_chunk_addr =
            gpu_DrawFixedSpritePairWithSelectedSheets(draw_chunk_addr, temp2);
        if (g_gpu_jitter_burst_enabled != 0) {
            s32 i;
            temp = (rand() & 3) + 4;
            for (i = 0; i < temp; i++) {
                draw_chunk_addr =
                    gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle(
                        draw_chunk_addr, g_local_ot_draw_mode);
            }
        } else if ((rand() & 7) == 0) {
            gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle(
                draw_chunk_addr, g_local_ot_draw_mode);
        }
        break;
    case 10:
        gpu_DrawPackedComparisonRowsAndTotals(
            g_fight_packed_match_scores, draw_chunk_addr, 0);
        break;
    case 20: {
        u8 cur;

        temp2 = g_fight_stage_command_count;
        cur = g_fight_stage_list_draw_cursor;
        if ((u32)temp2 < cur) {
            break;
        }
        if (0xF0 / (temp2 + 1) >= ++g_fight_stage_list_step_timer) {
            break;
        }
        /* FAKE: the common `g_fight_stage_list_draw_cursor = cur + 1` store
         * written in both arms (F7), as the target computes it per arm; one
         * hoisted store: score 6 (131/136 insns) (Q129). */
        if (cur == temp2) {
            g_fight_stage_list_draw_cursor = cur + 1;
        } else {
            u8 next = cur + 1;
            g_fight_stage_list_draw_cursor = next;
            g_fight_stage_list_step_timer = 0;
            temp = g_fight_stage_list_entries[cur];
            if (next == temp2) {
                temp |= 0x8000;
            }
            gpu_DrawSpriteAtPackedTablePosition(temp, draw_chunk_addr, 0);
        }
        break;
    }
    }
    DrawOTag(g_gpu_ot_ptr + 1);
    DrawSync(0);
}

void softbody_FreeAll(void) {
    s32 i;
    /* FAKE: the table walked by byte offset, the target's one induction
       variable; indexed by record: score 4 */
    for (i = 7 * sizeof(Func80017A44Output); i >= 0;
         i -= sizeof(Func80017A44Output)) {
        ((Func80017A44Output *)((u8 *)g_softbody_slots + i))->points = 0;
    }
}

s32 softbody_GetBufferSize(s32 node_count, s32 edge_count) {
    return (node_count << 6) + (edge_count << 4);
}

s32 math_Distance3D(s32 *first_point, s32 *second_point) {
    s32 in[3];
    s32 out[3];

    in[0] = (first_point[0] - second_point[0]) >> 2;
    in[1] = (first_point[1] - second_point[1]) >> 2;
    in[2] = (first_point[2] - second_point[2]) >> 2;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 2;
}

s32 math_Distance3D_16(s32 *first_point, s32 *second_point) {
    s32 in[3];
    s32 out[4];

    in[0] = (first_point[0] - second_point[0]) >> 4;
    in[1] = (first_point[1] - second_point[1]) >> 4;
    in[2] = (first_point[2] - second_point[2]) >> 4;
    Square12(in, out);
    return SquareRoot12(out[0] + out[1] + out[2]) << 4;
}

/* Adds an edge between records a and b unless a == b, both records have a
 * non-negative field_18, or an edge a->b or b->a already exists. Returns 1
 * when an edge was added. */
s32 softbody_AddSpring(Func80017A44Output *softbody, s32 arg1, s32 a, s32 b) {
    s32 i;
    s32 dist;
    Func80017848Edge *edge;

    if (a == b) {
        return 0;
    }
    if (softbody->nodes[a].field_18 >= 0 && softbody->nodes[b].field_18 >= 0) {
        return 0;
    }
    for (i = 0; i < softbody->nodes[a].a_edge_count; i++) {
        if (softbody->edges[softbody->nodes[a].a_edges[i]].ends.node.b == b) {
            return 0;
        }
    }
    for (i = 0; i < softbody->nodes[a].b_edge_count; i++) {
        if (softbody->edges[softbody->nodes[a].b_edges[i]].ends.node.a == b) {
            return 0;
        }
    }
    dist = math_Distance3D(softbody->nodes[a].pos, softbody->nodes[b].pos);
    edge = &softbody->edges[softbody->edge_count];
    edge->dist = dist;
    edge->field_8 = dist * 3;
    edge->field_C = arg1;
    edge->ends.pair = (a << 16) | b;
    softbody->nodes[a].a_edges[softbody->nodes[a].a_edge_count++] =
        softbody->edge_count;
    softbody->nodes[b].b_edges[softbody->nodes[b].b_edge_count++] =
        softbody->edge_count;
    softbody->edge_count++;
    return 1;
}

void softbody_Build(Func80017A44Input *input, Func80017A44Output *softbody) {
    VECTOR pos;
    VECTOR var_1;
    s32 flag;
    Func80017A44Record *record_base;
    s16 *var_4;
    s32 var_5;
    s32 i;
    s32 j;
    s32 group_node_count;
    s32 var_9;
    s32 dist;
    s32 value;

    var_1.vx = 0;
    var_1.vy = 0;
    var_1.vz = 0;
    var_5 = 0;
    SetRotMatrix(input->matrix);
    SetTransMatrix(input->matrix);

    record_base = softbody->nodes;
    for (i = 0; i < input->count; i++) {
        record_base[i].field_18 = input->points[i].pad;
        RotTrans(&input->points[i], &pos, &flag);
        pos.vx <<= 7;
        pos.vy <<= 7;
        pos.vz <<= 7;
        if (record_base[i].field_18 >= 0) {
            var_5++;
            var_1.vx += pos.vx;
            var_1.vy += pos.vy;
            var_1.vz += pos.vz;
        }
        record_base[i].pos[0] = pos.vx;
        record_base[i].pos[1] = pos.vy;
        record_base[i].pos[2] = pos.vz;
        record_base[i].vel_x = 0;
        record_base[i].vel_y = 0;
        record_base[i].vel_z = 0;
        record_base[i].a_edge_count = 0;
        record_base[i].b_edge_count = 0;
    }

    var_1.vx /= var_5;
    var_1.vy /= var_5;
    var_1.vz /= var_5;

    for (i = 0; i < input->count; i++) {
        value = math_Distance3D_16((s32 *)&var_1, record_base[i].pos) >> 8;
        if (value > 0x100) {
            value = 0x100;
        }
        record_base[i].field_34 = value;
    }

    var_4 = input->groups;
    while ((group_node_count = *var_4++) != 0) {
        var_9 = *var_4++;
        for (i = 0; i < group_node_count - 1; i++) {
            dist = math_Distance3D_16((s32 *)&var_1, record_base[var_4[i]].pos);
            for (j = i + 1; j < group_node_count; j++) {
                if (dist < math_Distance3D_16(
                               (s32 *)&var_1, record_base[var_4[j]].pos)) {
                    softbody_AddSpring(softbody, var_9, var_4[i], var_4[j]);
                } else {
                    softbody_AddSpring(softbody, var_9, var_4[j], var_4[i]);
                }
            }
        }
        var_4 += group_node_count;
    }
}

s32 softbody_Create(Func80017A44Input *input) {
    Func80017A44Output *softbody;
    s32 i;
    Func80017A44Record *c;

    softbody = g_softbody_slots;
    for (i = 0; i < 8; i++) {
        if (softbody->points == 0)
            break;
        softbody++;
    }
    if (i == 8)
        return -1;
    if (g_softbody_slot_high_water < i)
        g_softbody_slot_high_water = i;
    softbody->count = input->count;
    softbody->points = input->points;
    softbody->matrix = *input->matrix;
    softbody->a44out_08 = input->field_2;
    c = (Func80017A44Record *)input->buf;
    softbody->edge_count = 0;
    softbody->nodes = c;
    softbody->edges = (Func80017848Edge *)(c + softbody->count);
    softbody_Build(input, softbody);
    return i;
}

void softbody_Free(s32 slot_index) { g_softbody_slots[slot_index].points = 0; }

void softbody_RebaseBuffer(s32 slot_index, s32 byte_offset) {
    Func80017A44Output *ptr = &g_softbody_slots[slot_index];

    ptr->nodes = (Func80017A44Record *)((u8 *)ptr->nodes + byte_offset);
    ptr->edges = (Func80017848Edge *)(ptr->nodes + ptr->count);
}

void softbody_RebasePoints(s32 slot_index, s32 byte_offset) {
    Func80017A44Output *ptr = &g_softbody_slots[slot_index];
    ptr->points = (SVECTOR *)((u8 *)ptr->points + byte_offset);
}

void scratchpad_Save(void) {
    vu32 *src = (vu32 *)0x1F800000;
    u32 *dst = (u32 *)&g_scratchpad_save;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void scratchpad_Restore(void) {
    u32 *src = (u32 *)&g_scratchpad_save;
    vu32 *dst = (vu32 *)0x1F800000;
    u32 i;
    for (i = 0; i < 0xF8; i++) {
        *dst++ = *src++;
    }
}

void func_80017F90(void) {}

void func_80017F98(s32 arg0, s32 arg1, s32 arg2) {}

/* Q65: this file's initialized small data (.sdata), in address order. */
/* 0x800A30CC: the full 640x480 display area ClearImage clears */
RECT g_gpu_clear_rect = {0, 0, 640, 480};
/* 0x800A30D4: copied whole into a local RECT (rect = g_gpu_cd_file_image_rect)
 */
RECT g_gpu_cd_file_image_rect = {32, 96, 256, 48};
s32 g_main_prim_buffer_min_remaining = 0x13400;
/* not named by any code or data: size from the gap */
s32 D_800A30E0[2] = {(s32)D_8001004C, 0x190};
u8 g_softbody_slot_high_water = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
u8 g_fight_view_option;
u8 g_gpu_jitter_burst_enabled;
s32 g_gpu_presented_frame_count;
u8 D_800A36B0;
u8 g_main_frame_period;
u8 D_800A36F9;
s16 g_camera_distance_offset;
u8 g_fight_auto_pause_countdown;
u8 g_eff_initialized;
u8 D_800A3744;
u32 *g_gpu_ot_ptr;
u8 g_local_ot_draw_mode;
u32 D_800A3770[2];
s32 g_fight_packed_match_scores;
u8 g_fight_toggle_option_bits;
u32 D_800A3798;
u8 g_fight_stage_command_count;
s32 g_fight_stage_list_step_timer;
s16 g_main_loop_handler_index;
u8 D_800A389A;
u32 *g_prim_buf_cursor;
s32 g_rng_state;
s16 g_fight_mode_selector;
u8 g_fight_stage_list_draw_cursor;
u8 g_snd_initialized;
u8 g_main_present_skip_frames;
u8 g_comb_abort_request;
