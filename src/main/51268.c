/* 99 game functions. .text 0x80060A68 (ROM 0x51268). Start boundary: PHASE
 * (rodata-align section 9), moved by the per-file gp model (Q65). Ends at
 * eff_LinkParticleQuads: the menu controller after it is 596CC.c (seam, owner
 * ruling 2026-10-17). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>
#include <psxsdk/gtemac.h>

/* eff_StartCommand .. eff_BindScratchpad belong here by the per-file gp
 * evidence (Q65). */

/* Q65: this file's statics (.sbss, allocated per file in link order by
 * PSYLINK), in address order. */
static POLY_FT4 *D_800A3420;
static POLY_FT4 *D_800A3424;
/* not named by any code or data: size from the gap */
static s32 D_800A3428[2];
/* not named by any code or data: size from the gap */
static s32 D_800A3430[2];
static s16 D_800A3438[4];
static s16 D_800A3440;
static s32 g_eff_jittered_billboard_slot_mask;
static s32 g_eff_billboard_pair_slot_mask;
static u32 g_eff_yawed_quad_slot_counters[2];
static s32 g_eff_yawed_quad_slot_masks[2];
static s16 D_800A345C[2];
static s32 g_eff_rotated_quad_cluster_slot_mask;
static s32 g_eff_point_light_color;
static Unk1F800000Unk00 *g_eff_active_command_block;
static s16 *D_800A346C;
static s32 *g_eff_view_origin;
static MATRIX *g_eff_view_rotation_matrix;
static s16 *g_eff_command_vector;
static s32 *g_eff_command_position;
static s32 *D_800A3480;
static s32 *D_800A3484;
static TexRec *g_eff_current_tex_rec;
static TexRec *g_eff_clut_tex_rec;
static s32 *g_eff_tpage_word;
static s32 *g_eff_clut_word;
static u16 *g_eff_tex_left_u;
static u16 *g_eff_tex_right_u;
static u16 *g_eff_tex_top_v;
static u16 *g_eff_tex_bottom_v;
static s16 *g_eff_base_width;
static s16 *g_eff_base_height;
static s32 *g_eff_geom_screen_work;
static s32 *g_eff_perspective_scale;
static s32 *g_eff_screen_xy;
static s16 *D_800A34BC;
static s16 *g_eff_right_x_offset;
static s32 *g_eff_upper_y_offset;
static s32 *g_eff_lower_y_offset_word;
static s32 *g_eff_rtp_flag;
static s32 *g_eff_depth_work;
static u16 *g_eff_top_left_uv;
static u16 *g_eff_top_right_uv;
static u16 *g_eff_bottom_left_uv;
static u16 *g_eff_bottom_right_uv;
static u32 *g_eff_current_ot_entry;
static u32 *g_eff_current_prim_tag;
static Unk1F8000B8Union *g_eff_work_area;
/* Two s16 slots at 0x800A34F0, indexed as one array (eff_PrepareParticles and
 * eff_UpdateParticles read the pair through one indexed access). */
static s16 D_800A34F0[2];
static s32 D_800A34F4; /* not named by any code or data: size from the gap */

/* Runs the command in the block g_eff_active_command_block points at (idx = the
 * block's low halfword, unk00.h): copies the three words at unk0C and the three
 * halfwords at unk10 into unk20 / unk18, points g_eff_command_position /
 * g_eff_command_vector at the copies, clears g_eff_command_phase[idx], calls
 * the D_8009BA7C entry D_8009BA60[idx] + g_eff_command_phase[idx], stores the
 * result through unk14, and sets g_eff_point_light_countdown to 0xA when bit 21
 * of unk00.w is set. Member reads (not
 * *(u16 *)g_eff_active_command_block) let the second idx read (the D_8009BA60
 * index) rise above the g_eff_command_position store, as in the target. */
void eff_StartCommand(void) {
    extern s32 g_eff_point_light_countdown;

    s32 result;

    g_eff_command_phase[g_eff_active_command_block->unk00.h] = 0;
    g_eff_active_command_block->unk20[0] = g_eff_active_command_block->unk0C[0];
    g_eff_active_command_block->unk20[1] = g_eff_active_command_block->unk0C[1];
    g_eff_active_command_block->unk20[2] = g_eff_active_command_block->unk0C[2];
    g_eff_active_command_block->unk18[0] = g_eff_active_command_block->unk10[0];
    g_eff_active_command_block->unk18[1] = g_eff_active_command_block->unk10[1];
    g_eff_command_vector = g_eff_active_command_block->unk18;
    g_eff_active_command_block->unk18[2] = g_eff_active_command_block->unk10[2];
    g_eff_command_position = g_eff_active_command_block->unk20;

    result = ((s32(*)(void))D_8009BA7C
                  [D_8009BA60[g_eff_active_command_block->unk00.h] +
                   g_eff_command_phase[g_eff_active_command_block->unk00.h]])();
    *g_eff_active_command_block->unk14 = result;

    if (g_eff_active_command_block->unk00.w & 0x200000) {
        g_eff_point_light_countdown = 0xA;
    }
}

void eff_StepCommand(void) {

    Unk1F800000Unk00 *src_command_block;
    s16 *rotation_out;
    s32 *position_out;
    u16 command_index;
    s32 result;

    src_command_block = g_eff_active_command_block;
    rotation_out = D_800A346C;
    rotation_out[0] = src_command_block->unk04[0];
    rotation_out[1] = src_command_block->unk04[1];
    rotation_out[2] = src_command_block->unk04[2];

    position_out = g_eff_view_origin;
    position_out[0] = src_command_block->unk08[0];
    position_out[1] = src_command_block->unk08[1];
    position_out[2] = src_command_block->unk08[2];
    eff_SetRotMatrix(rotation_out, position_out, g_eff_view_rotation_matrix);

    command_index = g_eff_active_command_block->unk00.h;
    result = ((s32(*)(void))D_8009BA7C[D_8009BA60[command_index] +
                                       g_eff_command_phase[command_index]])();

    *g_eff_active_command_block->unk14 = result;
}

extern u8 g_eff_command_status[0x1C];

void eff_ResetCommands(void) {
    s32 command_index = 0;
    s32 *p = g_eff_command_phase;
    do {
        *p = 0;
        g_eff_command_status[command_index] = 0;
        command_index++;
        p++;
    } while (command_index < 0x1C);
    D_800A345C[1] = 0;
    D_800A345C[0] = 0;
    g_eff_yawed_quad_slot_masks[1] = 0;
    g_eff_yawed_quad_slot_masks[0] = 0;
    g_eff_yawed_quad_slot_counters[1] = 0;
    g_eff_yawed_quad_slot_counters[0] = 0;
    g_eff_rotated_quad_cluster_slot_mask = 0;
    g_eff_jittered_billboard_slot_mask = 0;
    g_eff_billboard_pair_slot_mask = 0;
}

s32 eff_LoadImagesAndSetup(s32 arg0, s32 quad_storage) {
    /* FAKE: single forward-order param alias; flips the s2/s1 save+copy pair
     * order to the target's (param-local-alias-prologue-pair-flip); arg0
     * throughout: score 4 */
    unsigned int new_var;
    RECT s;
    s32 v;
    s32 second_quad_buffer_addr;
    new_var = arg0;
    cdrom_WaitIdle();
    v = g_working_record.fight_mode;
    if (v == 0) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x3C), arg0);
    } else if (v == 3) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x2F), new_var);
    } else if (v == 2) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x30), new_var);
    } else if (v == 5) {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0x31), new_var);
    } else {
        cdrom_StartRead(cdrom_GetFileIndex(2, 0), new_var);
    }
    cdrom_WaitIdle();
    s.x = 0x380;
    s.y = 0;
    s.w = 0x80;
    s.h = 0x1DC;
    DrawSync(0);
    LoadImage(&s, (u32 *)new_var);
    DrawSync(0);
    s.w = 0x70;
    s.y = 0x1DC;
    s.h = 0x24;
    LoadImage(&s, (u32 *)(new_var + 0x1DC00));
    DrawSync(0);
    eff_ResetCommands();
    srand(rand());
    second_quad_buffer_addr = quad_storage + 0x4650;
    D_800A3420 = (POLY_FT4 *)quad_storage;
    D_800A3424 = (POLY_FT4 *)second_quad_buffer_addr;
    return second_quad_buffer_addr + 0x4650;
}

extern POLY_FT4 *g_eff_quad_buffer_base;

void eff_SelectQuadBuffer(s32 select_second_buffer) {
    g_eff_quad_cursor = select_second_buffer != 0 ? D_800A3424 : D_800A3420;
    g_eff_quad_buffer_base = g_eff_quad_cursor;
}

#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)

void eff_BindScratchpad(s16 *src_rotation, s32 *src_position) {
    g_eff_active_command_block = &SPAD51268->default_command_block;
    D_800A346C = SPAD51268->default_command_block.unk18;
    g_eff_view_origin = SPAD51268->default_command_block.unk20;
    g_eff_view_rotation_matrix = &SPAD51268->view_rotation_matrix;
    g_eff_current_tex_rec = &SPAD51268->unk50;
    g_eff_tpage_word = &SPAD51268->tpage_word;
    g_eff_clut_word = &SPAD51268->clut_word;
    g_eff_tex_left_u = &SPAD51268->tex_left_u;
    g_eff_tex_right_u = &SPAD51268->tex_right_u;
    g_eff_tex_top_v = &SPAD51268->tex_top_v;
    g_eff_tex_bottom_v = &SPAD51268->tex_bottom_v;
    g_eff_base_width = &SPAD51268->base_width;
    g_eff_base_height = &SPAD51268->base_height;
    g_eff_geom_screen_work = &SPAD51268->geom_screen_work;
    g_eff_perspective_scale = &SPAD51268->perspective_scale;
    g_eff_screen_xy = SPAD51268->screen_xy;
    D_800A34BC = &SPAD51268->left_x_offset;
    g_eff_right_x_offset = &SPAD51268->right_x_offset;
    g_eff_upper_y_offset = &SPAD51268->upper_y_offset;
    g_eff_lower_y_offset_word = &SPAD51268->unk88;
    g_eff_rtp_flag = &SPAD51268->unk8C;
    g_eff_depth_work = SPAD51268->depth_work;
    g_eff_top_left_uv = &SPAD51268->top_left_uv;
    g_eff_top_right_uv = &SPAD51268->top_right_uv;
    g_eff_bottom_left_uv = &SPAD51268->bottom_left_uv;
    g_eff_bottom_right_uv = &SPAD51268->bottom_right_uv;
    g_eff_current_ot_entry = &SPAD51268->unkA0;
    g_eff_current_prim_tag = &SPAD51268->unkA4;
    D_800A3480 = &SPAD51268->yawed_quad_tex_rec_index;
    D_800A3484 = &SPAD51268->quad_rate_selector;
    g_eff_clut_tex_rec = &SPAD51268->unkB0;
    g_eff_work_area = &SPAD51268->effect_work_area;
    SPAD51268->default_command_block.unk04 = src_rotation;
    SPAD51268->default_command_block.unk08 = src_position;
}

extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
extern void AddPrim(void *, void *);
void eff_StartCommand(void);
void eff_StepCommand(void);
void eff_ResetCommands(void);
void eff_BindScratchpad(s16 *arg0, s32 *arg1);

/* the message, alignment and an empty trailing string */
const char D_800158E0[24] = "eff prim over :%d \n";

void func_80061064(s16 *src_rotation, s32 *src_position) {
    s32 quad_count;
    s32 command_index;
    eff_BindScratchpad(src_rotation, src_position);
    command_index = 0;
    do {
        g_eff_active_command_block->unk14 =
            &g_eff_command_status[command_index];
        if (g_eff_command_status[command_index] != 0) {
            g_eff_active_command_block->unk00.w = command_index;
            eff_StepCommand();
        }
        command_index += 1;
    } while (command_index < 0x1C);
    if (g_eff_point_light_countdown >= 2) {
        g_eff_point_light_countdown -= 1;
        light_SetPointLight(
            &g_eff_point_light_position, g_eff_point_light_color);
    } else if (g_eff_point_light_countdown == 1) {
        light_DisablePointLight();
        g_eff_point_light_countdown = 0;
    }
    quad_count = g_eff_quad_cursor - g_eff_quad_buffer_base;
    if (quad_count >= 0x1C2) {
        printf(&D_800158E0, quad_count - 0x1C2);
    }
}

void eff_StopCommands(void) {
    eff_ResetCommands();
    light_DisablePointLight();
    g_eff_point_light_countdown = 0;
}

extern Unk1F800000Unk00 g_eff_command_block;

void eff_StartRotatedQuadClusterWithNearWhiteLight(
    s32 *src_position, s16 *src_command_vector) {
    s16 command_vector[3];
    s32 *p;
    s16 *src_command_vector_cursor = src_command_vector;
    command_vector[0] = *src_command_vector_cursor++;
    command_vector[1] = *src_command_vector_cursor++;
    g_eff_active_command_block = &g_eff_command_block;
    command_vector[2] = *src_command_vector_cursor;
    g_eff_command_block.unk10 = command_vector;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[26];
    g_eff_command_block.unk00.w = 0x21001A;
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFFFFEF;
}

void eff_StartRecycledCommandWithRoseLight(s32 *src_position) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    if (g_eff_command_status[9] != 0 && g_eff_command_status[10] != 0) {
        g_eff_command_status[10] = 0;
        g_eff_command_status[9] = 0;
    }
    if (g_eff_command_status[9] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[9];
        g_eff_active_command_block->unk00.w = 0x210009;
    } else if (g_eff_command_status[10] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[10];
        g_eff_command_block.unk00.w = 0x21000A;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFF0060;
}

s32 eff_StartCosineScaledQuadWithPaleBlueLight(s32 *src_position) {
    s32 *p = src_position;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[4];
    g_eff_command_block.unk00.w = 0x210004;
    eff_StartCommand();
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0x8080FF;
    return 1;
}

s32 eff_StartSlowRateCosineScaledQuadWithPaleBlueLight(s32 *src_position) {
    s32 *src_position_cursor = src_position;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x21000B;
    eff_StartCommand();
    g_eff_point_light_position.vx = *src_position_cursor++;
    g_eff_point_light_position.vy = *src_position_cursor++;
    g_eff_point_light_position.vz = *src_position_cursor++;
    g_eff_point_light_color = 0x8080FF;
    return 16;
}

s32 eff_StartMediumRateCosineScaledQuadWithPaleBlueLight(s32 *src_position) {
    s32 *p = src_position;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x29000B;
    eff_StartCommand();
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0x8080FF;
    return 8;
}

s32 eff_StartFastRateCosineScaledQuadWithPaleBlueLight(s32 *src_position) {
    s32 *p = src_position;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[11];
    g_eff_command_block.unk00.w = 0x31000B;
    eff_StartCommand();
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0x8080FF;
    return 5;
}

void eff_StartRecycledCommandWithPaleRedLight(s32 *src_position) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    if (g_eff_command_status[5] != 0 && g_eff_command_status[6] != 0) {
        g_eff_command_status[6] = 0;
        g_eff_command_status[5] = 0;
    }
    if (g_eff_command_status[5] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[5];
        g_eff_active_command_block->unk00.w = 0x210005;
    } else if (g_eff_command_status[6] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[6];
        g_eff_command_block.unk00.w = 0x210006;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFF8080;
}

void eff_StartArgSelectedCommandWithCyanLight(
    s32 *src_position, s32 command_variant) {
    s32 *p;

    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    switch (command_variant) {
    case 0:
        g_eff_command_status[12] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[12];
        g_eff_command_block.unk00.w = 0x21000C;
        break;
    case 1:
        g_eff_command_status[13] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[13];
        g_eff_command_block.unk00.w = 0x21000D;
        break;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0x10FFFF;
}

void eff_StartArgSelectedCommandWithGreenLight(
    s32 *src_position, s32 command_variant) {
    s32 *p;

    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    switch (command_variant) {
    case 0:
        g_eff_command_status[14] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[14];
        g_eff_command_block.unk00.w = 0x21000E;
        break;
    case 1:
        g_eff_command_status[15] = 0;
        g_eff_command_block.unk14 = &g_eff_command_status[15];
        g_eff_command_block.unk00.w = 0x21000F;
        break;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0x10FF10;
}

void eff_StartRecycledCommandWithOrangeLight(s32 *src_position) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    if (g_eff_command_status[16] != 0 && g_eff_command_status[17] != 0) {
        g_eff_command_status[17] = 0;
        g_eff_command_status[16] = 0;
    }
    if (g_eff_command_status[16] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[16];
        g_eff_active_command_block->unk00.w = 0x210010;
    } else if (g_eff_command_status[17] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[17];
        g_eff_command_block.unk00.w = 0x210011;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xC06013;
}

void eff_StartRecycledVectorCommandWithRedLight(
    s32 *src_position, s16 *src_command_vector) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk10 = src_command_vector;
    if (g_eff_command_status[2] != 0 && g_eff_command_status[3] != 0) {
        g_eff_command_status[3] = 0;
        g_eff_command_status[2] = 0;
    }
    if (g_eff_command_status[2] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[2];
        g_eff_active_command_block->unk00.w = 0x210002;
    } else if (g_eff_command_status[3] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[3];
        g_eff_command_block.unk00.w = 0x210003;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFF0000;
}

void eff_StartBillboardPairCommand(s32 *src_position) {
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[8];
    g_eff_command_block.unk00.w = 0x10008;
    eff_StartCommand();
}

void eff_StartJitteredBillboardsCommand(s32 *src_position) {
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk14 = &g_eff_command_status[7];
    g_eff_command_block.unk00.w = 0x10007;
    eff_StartCommand();
}

void eff_StartYawedQuadSlotCommand(s32 *src_position, s16 command_vector_y,
                                   s32 command_word_bits, s32 use_second_slot) {
    s16 command_vector[4];

    g_eff_active_command_block = &g_eff_command_block;
    command_vector[2] = 0;
    command_vector[0] = 0;
    command_vector[1] = command_vector_y;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk10 = command_vector;
    if (use_second_slot == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[0];
        g_eff_command_block.unk00.w = command_word_bits + 0x10000;
    } else {
        g_eff_command_block.unk14 = &g_eff_command_status[1];
        g_eff_command_block.unk00.w = command_word_bits + 0x10001;
    }
    eff_StartCommand();
}

void eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight(
    s32 *src_position, s16 *src_command_vector) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    g_eff_command_block.unk10 = src_command_vector;
    if (g_eff_command_status[20] != 0 && g_eff_command_status[21] != 0) {
        g_eff_command_status[21] = 0;
        g_eff_command_status[20] = 0;
    }
    if (g_eff_command_status[20] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[20];
        g_eff_active_command_block->unk00.w = 0x210014;
        eff_StartCommand();
        g_eff_active_command_block->unk14 = &g_eff_command_status[7];
        g_eff_active_command_block->unk00.w = 0x10007;
    } else if (g_eff_command_status[21] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[21];
        g_eff_active_command_block->unk00.w = 0x210015;
        eff_StartCommand();
        g_eff_active_command_block->unk14 = &g_eff_command_status[7];
        g_eff_active_command_block->unk00.w = 0x10007;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFF8080;
}

void eff_StartRecycledYawedParticlesWithTextureSelection(
    s32 *src_position, s32 yaw_angle, s32 use_alternate_texture) {
    SVECTOR command_vector;
    SVECTOR yaw_rotation;
    VECTOR rotated_vector;
    MATRIX yaw_matrix;
    s32 gte_flag_out;

    g_eff_active_command_block = &g_eff_command_block;
    if (use_alternate_texture != 1) {
        use_alternate_texture = 0;
    }
    command_vector.vy = -0xA00;
    command_vector.vx = 0;
    command_vector.vz = 0xA00;
    rotated_vector.vz = 0;
    rotated_vector.vy = 0;
    rotated_vector.vx = 0;
    yaw_rotation.vy = yaw_angle;
    yaw_rotation.vz = 0;
    yaw_rotation.vx = 0;
    RotMatrix(&yaw_rotation, &yaw_matrix);
    yaw_matrix.t[2] = 0;
    yaw_matrix.t[1] = 0;
    yaw_matrix.t[0] = 0;
    SetRotMatrix(&yaw_matrix);
    SetTransMatrix(&yaw_matrix);
    RotTrans(&command_vector, &rotated_vector, &gte_flag_out);
    command_vector.vx = rotated_vector.vx;
    command_vector.vy = rotated_vector.vy;
    command_vector.vz = rotated_vector.vz;
    g_eff_active_command_block->unk0C = src_position;
    g_eff_active_command_block->unk10 = &command_vector.vx;
    if (g_eff_command_status[22] != 0 && g_eff_command_status[23] != 0) {
        g_eff_command_status[23] = 0;
        g_eff_command_status[22] = 0;
    }
    if (g_eff_command_status[22] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[22];
        g_eff_active_command_block->unk00.w = 0x10016;
        D_800A34F0[0] = use_alternate_texture;
    } else if (g_eff_command_status[23] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[23];
        g_eff_active_command_block->unk00.w = 0x10017;
        D_800A34F0[1] = use_alternate_texture;
    }
    eff_StartCommand();
}

void eff_StartRecycledYawedParticles(s32 *src_position, s16 yaw_angle) {
    SVECTOR command_vector;
    SVECTOR yaw_rotation;
    VECTOR rotated_vector;
    MATRIX yaw_matrix;
    s32 gte_flag_out;

    g_eff_active_command_block = &g_eff_command_block;
    command_vector.vy = -0xA00;
    command_vector.vx = 0;
    command_vector.vz = 0xA00;
    rotated_vector.vz = 0;
    rotated_vector.vy = 0;
    rotated_vector.vx = 0;
    yaw_rotation.vz = 0;
    yaw_rotation.vy = yaw_angle;
    yaw_rotation.vx = 0;
    RotMatrix(&yaw_rotation, &yaw_matrix);
    yaw_matrix.t[2] = 0;
    yaw_matrix.t[1] = 0;
    yaw_matrix.t[0] = 0;
    SetRotMatrix(&yaw_matrix);
    SetTransMatrix(&yaw_matrix);
    RotTrans(&command_vector, &rotated_vector, &gte_flag_out);
    command_vector.vx = rotated_vector.vx;
    command_vector.vy = rotated_vector.vy;
    command_vector.vz = rotated_vector.vz;
    g_eff_active_command_block->unk0C = src_position;
    g_eff_active_command_block->unk10 = &command_vector.vx;
    if (g_eff_command_status[24] != 0 && g_eff_command_status[25] != 0) {
        g_eff_command_status[25] = 0;
        g_eff_command_status[24] = 0;
    }
    if (g_eff_command_status[24] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[24];
        g_eff_active_command_block->unk00.w = 0x10018;
    } else if (g_eff_command_status[25] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[25];
        g_eff_active_command_block->unk00.w = 0x10019;
    }
    eff_StartCommand();
}

void eff_StartRecycledCommandWithMagentaLight(s32 *src_position) {
    s32 *p;
    g_eff_active_command_block = &g_eff_command_block;
    g_eff_command_block.unk0C = src_position;
    if (g_eff_command_status[18] != 0 && g_eff_command_status[19] != 0) {
        g_eff_command_status[19] = 0;
        g_eff_command_status[18] = 0;
    }
    if (g_eff_command_status[18] == 0) {
        g_eff_active_command_block->unk14 = &g_eff_command_status[18];
        g_eff_active_command_block->unk00.w = 0x210012;
    } else if (g_eff_command_status[19] == 0) {
        g_eff_command_block.unk14 = &g_eff_command_status[19];
        g_eff_command_block.unk00.w = 0x210013;
    }
    eff_StartCommand();
    p = src_position;
    g_eff_point_light_position.vx = *p++;
    g_eff_point_light_position.vy = *p++;
    g_eff_point_light_position.vz = *p;
    g_eff_point_light_color = 0xFF00FF;
}

extern VECTOR D_8009BB74;

void eff_SetRotMatrix(s16 *src_rotation, s32 *a1, MATRIX *matrix_out) {
    SVECTOR *rotation = &g_eff_work_area->v80061FAC;
    rotation->vx = src_rotation[0];
    rotation->vy = src_rotation[1];
    rotation->vz = src_rotation[2];
    RotMatrix(rotation, matrix_out);
    matrix_out->t[2] = 0;
    matrix_out->t[1] = 0;
    matrix_out->t[0] = 0;
    ScaleMatrixL(matrix_out, &D_8009BB74);
    SetRotMatrix(matrix_out);
}

extern s32 g_eff_billboard_draw_count;

void eff_SetBillboardList(Unk800F1198Record *src_billboards) {
    s32 billboard_index;

    g_eff_billboard_draw_count = 0;
    for (billboard_index = 0; src_billboards[billboard_index].unk0 & 1;
         billboard_index++) {
        g_eff_billboards[billboard_index].unk0 =
            src_billboards[billboard_index].unk0;
        g_eff_billboards[billboard_index].unk4 =
            src_billboards[billboard_index].unk4;
        g_eff_billboards[billboard_index].unk8 =
            src_billboards[billboard_index].unk8;
    }
    g_eff_billboards[billboard_index].unk0 =
        g_eff_billboards[billboard_index].unk4 =
            g_eff_billboards[billboard_index].unk8 = 0;
}

/* Draw the g_eff_billboards billboard list (see eff_SetBillboardList): per
   record until the terminator, pick a sprite size and an animation frame from
   the record's type, rotate/translate its position, project it, and when it
   lands in range emit one textured POLY_FT4 billboard and link it into the OT
   at its depth. */
void eff_DrawBillboardList(s16 *src_rotation, s32 *src_position) {
    extern s32 g_eff_billboard_draw_count;
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    extern s32 rand(void);
    Unk1F8000B8_800620B8 *base;
    s16 *half_width;
    s16 *billboard_height;
    VECTOR *var_3;
    VECTOR *v;
    SVECTOR *local_origin;
    s32 *depth_cue_out;
    u32 *ot_index;
    POLY_FT4 *prim;
    Unk1F800000Unk00 *src_command_block;
    s16 *rotation_out;
    s32 *position_out;
    MATRIX *scaled_rotation_matrix;
    s16 billboard_index;
    s32 scaled_width;
    s32 scaled_height;
    s16 width;
    s16 height;

    eff_BindScratchpad(src_rotation, src_position);
    src_command_block = g_eff_active_command_block;
    rotation_out = D_800A346C;
    rotation_out[0] = src_command_block->unk04[0];
    prim = g_eff_quad_cursor;
    rotation_out[1] = src_command_block->unk04[1];
    rotation_out[2] = src_command_block->unk04[2];
    position_out = g_eff_view_origin;
    /* the matrix eff_SetRotMatrix builds from the angles at src_rotation */
    scaled_rotation_matrix = g_eff_view_rotation_matrix;
    position_out[0] = src_command_block->unk08[0];
    base = &g_eff_work_area->v800620B8;
    position_out[1] = src_command_block->unk08[1];
    position_out[2] = src_command_block->unk08[2];
    g_eff_billboard_draw_count++;
    eff_SetRotMatrix(rotation_out, position_out, scaled_rotation_matrix);
    SetRotMatrix(g_eff_view_rotation_matrix);
    half_width = &base->billboard_screen_half_width;
    billboard_height = &base->billboard_screen_height;
    var_3 = &base->rotated_position;
    v = &base->view_relative_position;
    local_origin = &base->zero_vector;
    depth_cue_out = &base->depth_cue;
    ot_index = &base->billboard_ot_index;
    local_origin->vz = 0;
    local_origin->vy = 0;
    local_origin->vx = 0;
    *g_eff_geom_screen_work = ReadGeomScreen() << 8;
    *g_eff_tpage_word = 0x2F;
    for (billboard_index = 0; g_eff_billboards[billboard_index].unk0 & 1;
         billboard_index++) {
        switch (g_eff_billboards[billboard_index].unk4 & 7) {
        case 3:
            *g_eff_base_width = 0x151;
            *g_eff_base_height = 0xA8;
            goto sel_a;
        case 0:
            *g_eff_base_width = 0x1C2;
            *g_eff_base_height = 0xE1;
        sel_a:
            g_eff_clut_tex_rec = g_eff_current_tex_rec =
                &D_8009BA00[(u32)g_eff_billboard_draw_count % 6];
            if (g_working_record.grayscale) {
                g_eff_clut_tex_rec = &D_8009BA00[10];
            }
            *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0x1F;
            *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0x1F;
            break;
        case 2:
            *g_eff_base_width = 0x50;
            *g_eff_base_height = 0x3C;
            goto sel_b;
        case 1:
            *g_eff_base_width = 0x64;
            *g_eff_base_height = 0x78;
        sel_b:
            g_eff_clut_tex_rec = g_eff_current_tex_rec =
                &D_8009BA00[6 + (g_eff_billboard_draw_count & 3)];
            if (g_working_record.grayscale) {
                g_eff_clut_tex_rec = &D_8009BA00[11];
            }
            *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0xF;
            *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0x13;
            break;
        }
        v->vx =
            g_eff_billboards[billboard_index].unk0 / 2 - g_eff_view_origin[0];
        v->vy =
            g_eff_billboards[billboard_index].unk4 / 8 - g_eff_view_origin[1];
        v->vz = g_eff_billboards[billboard_index].unk8 - g_eff_view_origin[2];
        ApplyRotMatrixLV(v, var_3);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the address 0x14
         * below var_3, it loads var_3 as the translation (there is no whole
         * MATRIX here). Spelled (MATRIX *)base: score 65. */
        SetTransMatrix((MATRIX *)((u8 *)var_3 - 0x14));
        RotTransPers(
            local_origin, g_eff_screen_xy, depth_cue_out, g_eff_rtp_flag);
        gte_stsz(g_eff_depth_work);
        *ot_index = gpu_DepthToOtIndex(*g_eff_depth_work, 0);
        if (*ot_index < 0x1005 &&
            (*g_eff_geom_screen_work / 256 >> 4) < *ot_index) {
            *g_eff_clut_word =
                (u16)(((g_eff_clut_tex_rec->clut_x >> 4) & 0x3F) +
                      (g_eff_clut_tex_rec->clut_y << 6));
            *g_eff_tex_left_u = g_eff_current_tex_rec->u;
            *g_eff_tex_top_v = g_eff_current_tex_rec->v;
            *g_eff_depth_work = *g_eff_depth_work ? *g_eff_depth_work : 1;
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work / *g_eff_depth_work;
            scaled_width = *g_eff_base_width * *g_eff_perspective_scale;
            width = scaled_width > 0x200 ? scaled_width >> 8 : 2;
            *half_width = width / 2;
            scaled_height = *g_eff_base_height * *g_eff_perspective_scale;
            height = scaled_height > 0x200 ? scaled_height >> 8 : 2;
            *billboard_height = height;
            *billboard_height += (*billboard_height * rand() / 10) >> 14;
            SetPolyFT4(prim);
            prim->tpage = *g_eff_tpage_word;
            prim->clut = *g_eff_clut_word;
            prim->r0 = 0xFF;
            prim->g0 = 0x80;
            prim->b0 = 0x80;
            prim->x0 = *g_eff_screen_xy - *half_width;
            prim->y0 = (*g_eff_screen_xy >> 16) - *billboard_height;
            prim->x1 = *half_width + *g_eff_screen_xy;
            prim->y1 = (*g_eff_screen_xy >> 16) - *billboard_height;
            prim->x2 = *g_eff_screen_xy - *half_width;
            prim->y2 = *g_eff_screen_xy >> 16;
            prim->x3 = *half_width + *g_eff_screen_xy;
            prim->y3 = *g_eff_screen_xy >> 16;
            prim->u0 = *g_eff_tex_left_u;
            prim->v0 = *g_eff_tex_top_v;
            prim->u1 = *g_eff_tex_right_u;
            prim->v1 = *g_eff_tex_top_v;
            prim->u2 = *g_eff_tex_left_u;
            prim->v2 = *g_eff_tex_bottom_v;
            prim->u3 = *g_eff_tex_right_u;
            prim->v3 = *g_eff_tex_bottom_v;
            SetShadeTex(prim, 1);
            SetSemiTrans(prim, 1);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                g_eff_current_ot_entry = g_gpu_ot_ptr + *ot_index;
                g_eff_current_prim_tag = &prim->tag;
                *g_eff_current_prim_tag =
                    (*g_eff_current_prim_tag & 0xFF000000) |
                    (*g_eff_current_ot_entry & 0xFFFFFF);
                *g_eff_current_ot_entry =
                    ((u32)g_eff_current_prim_tag & 0xFFFFFF) |
                    (*g_eff_current_ot_entry & 0xFF000000);
                prim++;
            }
        }
    }
    g_eff_quad_cursor = prim;
}

s32 eff_SpawnRotatedQuadCluster(void) {
    s32 *pos;
    s16 *rot;
    s32 slot_index;
    s32 mask;

    g_eff_command_phase[26] = 1;
    pos = g_eff_command_position;
    rot = g_eff_command_vector;
    for (slot_index = 0; slot_index < 6; slot_index++) {
        mask = 1 << slot_index;
        if (!(g_eff_rotated_quad_cluster_slot_mask & mask)) {
            D_800F0FB8[slot_index].x = pos[0];
            D_800F0FB8[slot_index].y = pos[1];
            D_800F0FB8[slot_index].z = pos[2];
            D_800F10A0[slot_index].vx = rot[0];
            D_800F10A0[slot_index].vy = rot[1];
            g_eff_rotated_quad_cluster_slot_mask |= mask;
            D_800F10A0[slot_index].vz = rot[2];
            D_800F0C04[slot_index] = 0;
            break;
        }
    }
    return 1;
}

/* Draw the up-to-6 slots eff_SpawnRotatedQuadCluster spawns: per active slot,
   scale/rotate/ translate its matrix, then emit three textured POLY_FT4 quads,
   and finally link the new quads into the OT. Returns 1 when quads were added,
   else the live-slot mask (0 once every slot has expired). */
s32 eff_DrawAndAdvanceRotatedQuadClusters(void) {
    extern s16 D_8009BB84[];
    s32 count;
    MATRIX *composite_matrix;
    s32 *depth_cue_out;
    u16 *quad_ot_indices;
    s32 *scale;
    s32 *var_5;
    MATRIX *mats;
    MATRIX *slot_matrix;
    s32 *sxy;
    POLY_FT4 *prim;
    s32 slot_index;
    s32 quad_index;
    s32 k;
    s32 v;
    s32 c;
    s32 slot_bit;
    s16 *quad_vertices;
    POLY_FT4 *quads_end;

    /* FAKE: the work-area base (g_eff_work_area) is staged through `prim`
     * before prim becomes the quad cursor; keeps `mats = base + 0x78` out of
     * the loop's giv init (target keeps `move s4,v0`) and puts base in s1. A
     * separate `u8 *`/`s32`/`POLY_FT4 *` base, declaration/scope/setup-order
     * variants, direct global, or local struct with sub-pointers: score 37;
     * advancing mats as a cursor: 90; direct struct member accesses: 149. */
    prim = (POLY_FT4 *)g_eff_work_area;
    count = 0;
    mats = (MATRIX *)((u8 *)prim + 0x78);
    composite_matrix = (MATRIX *)((u8 *)prim + 0x138);
    sxy = (s32 *)((u8 *)prim + 0x158);
    depth_cue_out = (s32 *)((u8 *)prim + 0x168);
    quad_ot_indices = (u16 *)((u8 *)prim + 0x16C);
    scale = (s32 *)((u8 *)prim + 0x178);
    var_5 = (s32 *)((u8 *)prim + 0x188);
    prim = g_eff_quad_cursor;
    for (slot_index = 0; slot_index < 6; slot_index++) {
        slot_bit = 1 << slot_index;
        if (!(g_eff_rotated_quad_cluster_slot_mask & slot_bit)) {
            continue;
        }
        slot_matrix = &mats[slot_index];
        v = rsin(((D_800F0C04[slot_index] + 6) << 10) / 6);
        scale[0] = scale[1] = scale[2] = v + (D_800F0C04[slot_index] << 12) / 6;
        RotMatrixZYX(&D_800F10A0[slot_index].vx, (u8 *)slot_matrix);
        ScaleMatrix((u8 *)slot_matrix, scale);
        slot_matrix->t[0] = D_800F0FB8[slot_index].x - g_eff_view_origin[0];
        slot_matrix->t[1] = D_800F0FB8[slot_index].y - g_eff_view_origin[1];
        slot_matrix->t[2] = D_800F0FB8[slot_index].z - g_eff_view_origin[2];
        CompMatrix(g_eff_view_rotation_matrix, slot_matrix, composite_matrix);
        SetRotMatrix(composite_matrix);
        SetTransMatrix(composite_matrix);
        for (quad_index = 0; quad_index < 3; quad_index++) {
            if (D_800F0C04[slot_index] < 3) {
                *(s32 *)&prim->r0 = 0x808080;
            } else {
                c = ((6 - D_800F0C04[slot_index]) << 7) / 3;
                *var_5 = c;
                *(s32 *)&prim->r0 = c + (c << 8) + (c << 16);
            }
            if (D_800F0C04[slot_index] < 3) {
                g_eff_current_tex_rec = quad_index ? D_8009B960 : D_8009B958;
            } else {
                g_eff_current_tex_rec = quad_index ? D_8009B970 : D_8009B968;
            }
            if (quad_index) {
                *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0x3F;
            } else {
                *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0x1F;
            }
            *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0x1F;
            *g_eff_tpage_word = 0x2E;
            *g_eff_clut_word = (((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                                (g_eff_current_tex_rec->clut_y << 6))
                               << 16;
            *g_eff_tpage_word = *g_eff_tpage_word << 16;
            *g_eff_tex_left_u = g_eff_current_tex_rec->u;
            *g_eff_tex_top_v = g_eff_current_tex_rec->v;
            *g_eff_top_left_uv = *g_eff_tex_left_u + (*g_eff_tex_top_v << 8);
            *g_eff_top_right_uv = *g_eff_tex_right_u + (*g_eff_tex_top_v << 8);
            *g_eff_bottom_left_uv =
                *g_eff_tex_left_u + (*g_eff_tex_bottom_v << 8);
            *g_eff_bottom_right_uv =
                *g_eff_tex_right_u + (*g_eff_tex_bottom_v << 8);
            setlen(prim, 9);
            prim->code = 0x2E;
            *(s32 *)&prim->u0 = *g_eff_top_left_uv + *g_eff_clut_word;
            *(s32 *)&prim->u1 = *g_eff_top_right_uv + *g_eff_tpage_word;
            *(u16 *)&prim->u2 = *g_eff_bottom_left_uv;
            *(u16 *)&prim->u3 = *g_eff_bottom_right_uv;
            SetPolyFT4(prim);
            SetShadeTex(prim, 1);
            SetSemiTrans(prim, 1);
            quad_vertices = &D_8009BB84[quad_index * 16];
            RotTransPers4(quad_vertices, quad_vertices + 4, quad_vertices + 8,
                          quad_vertices + 12, sxy, sxy + 1, sxy + 2, sxy + 3,
                          depth_cue_out, g_eff_rtp_flag);
            gte_stsz(g_eff_depth_work);
            *g_eff_depth_work = gpu_DepthToOtIndex(*g_eff_depth_work, 0);
            if (*g_eff_depth_work == 0) {
                *g_eff_depth_work = 1;
            }
            if (*g_eff_depth_work < 0x1005) {
                quad_ot_indices[count] = *g_eff_depth_work;
                *(s32 *)&prim->x0 = sxy[0];
                *(s32 *)&prim->x1 = sxy[1];
                *(s32 *)&prim->x2 = sxy[2];
                *(s32 *)&prim->x3 = sxy[3];
                count++;
                if (prim - g_eff_quad_buffer_base < 0x1C1) {
                    prim++;
                }
            }
        }
        D_800F0C04[slot_index]++;
        if (D_800F0C04[slot_index] >= 7) {
            g_eff_rotated_quad_cluster_slot_mask &= ~(1 << slot_index);
        }
    }
    if (g_eff_quad_cursor != prim) {
        /* `quads_end` keeps the fill position; the quad cursor prim restarts at
         * g_eff_quad_cursor to link each quad, the way SOTN's menu code
         * restarts its fill cursor at the buffer to read it back (Q51) */
        /* SOTN: src/dra/menu.c:3352 @db41b28 */
        quads_end = prim;
        for (prim = g_eff_quad_cursor, k = 0; prim < quads_end; prim++, k++) {
            AddPrim(g_gpu_ot_ptr + quad_ot_indices[k], prim);
        }
        g_eff_quad_cursor = quads_end;
        return 1;
    }
    if (g_eff_rotated_quad_cluster_slot_mask == 0) {
        g_eff_command_phase[26] = 0;
        return 0;
    }
    return g_eff_rotated_quad_cluster_slot_mask;
}

/* Spawn a slot: claim the first free slot bit in g_eff_billboard_pair_slot_mask
   (of 12), copy the position at *g_eff_command_position into its D_800F0E38
   record and reset its age. With every slot taken it writes slot 12
   (0x800F0EC8, the next table)
   -- the original has no guard. */
s32 eff_SpawnBillboardPair(void) {
    s32 slot_index;
    s32 slot_bit;
    s32 *src_position;

    g_eff_command_phase[8] = 1;
    for (slot_index = 0; slot_index < 12; slot_index++) {
        slot_bit = 1 << slot_index;
        if (!(g_eff_billboard_pair_slot_mask & slot_bit)) {
            g_eff_billboard_pair_slot_mask |= slot_bit;
            break;
        }
    }
    src_position = g_eff_command_position;
    D_800F0E38[slot_index].unk0 = src_position[0];
    D_800F0E38[slot_index].unk4 = src_position[1];
    D_800F0E38[slot_index].unk8 = src_position[2];
    g_eff_billboard_pair_ages[slot_index] = 0;
    return 1;
}

/* Draw the up-to-12 slots eff_SpawnBillboardPair spawns: per live slot
   slot_index (bit slot_index of g_eff_billboard_pair_slot_mask, age
   g_eff_billboard_pair_ages[slot_index], world position D_800F0E38[slot_index])
   emit two textured POLY_FT4 billboards (billboard_index == 0, then
   billboard_index == 1 offset vertically by an rsin of its age), each
   rotated/translated relative to the camera, projected, sized from its depth
   and animated by its age, then linked into the OT at its depth. A slot whose
   age reaches 16 clears its bit. Returns nonzero while any slot is still live.
 */
s32 eff_DrawAndAdvanceBillboardPairs(void) {
    extern s32 RotTransPers(SVECTOR *, s32 *, s32 *, s32 *);
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80063084 *base;
    POLY_FT4 *prim;
    VECTOR *var_2;
    SVECTOR *relative_position;
    SVECTOR *v;
    s32 *depth_cue_out;
    s32 *var_6;
    s32 *ot_index;
    s16 slot_index;
    s16 billboard_index;
    s32 bit;
    s32 scale;
    s32 red_green_level;

    base = &g_eff_work_area->v80063084;
    prim = g_eff_quad_cursor;
    var_2 = &base->rotated_position;
    relative_position = &base->view_relative_position;
    v = &base->zero_vector;
    depth_cue_out = &base->depth_cue;
    var_6 = &base->unk38;
    ot_index = &base->billboard_ot_index;
    gte_SetRotMatrix(g_eff_view_rotation_matrix);
    v->vz = 0;
    v->vy = 0;
    v->vx = 0;
    *g_eff_geom_screen_work = ReadGeomScreen() * 1000;
    *g_eff_base_width = 0xC0;
    *g_eff_base_height = 0x60;
    *g_eff_tpage_word = 0x2E;
    *g_eff_tpage_word = *g_eff_tpage_word << 16;
    for (slot_index = 0; slot_index < 12; slot_index++) {
        bit = 1 << slot_index;
        if (!(g_eff_billboard_pair_slot_mask & bit)) {
            continue;
        }
        for (billboard_index = 0; billboard_index < 2; billboard_index++) {
            if (g_eff_billboard_pair_ages[slot_index] < 16) {
                relative_position->vx =
                    D_800F0E38[slot_index].unk0 - g_eff_view_origin[0];
                relative_position->vz =
                    D_800F0E38[slot_index].unk8 - g_eff_view_origin[2];
                if (billboard_index != 0 &&
                    g_eff_billboard_pair_ages[slot_index] >= 3) {
                    relative_position->vy =
                        D_800F0E38[slot_index].unk4 -
                        (rsin(
                             (g_eff_billboard_pair_ages[slot_index] - 3) << 8) *
                             329 / 4096 -
                         69) /
                            10 -
                        g_eff_view_origin[1];
                } else {
                    relative_position->vy =
                        D_800F0E38[slot_index].unk4 - g_eff_view_origin[1];
                }
                ApplyRotMatrix(relative_position, var_2);
                /* FAKE: SetTransMatrix reads only m->t (+0x14): handed the
                 * address 0x14 below var_2, it loads var_2 as the translation.
                 * Spelled (MATRIX *)base: score 55. */
                SetTransMatrix((MATRIX *)((u8 *)var_2 - 0x14));
                RotTransPers(v, g_eff_screen_xy, depth_cue_out, g_eff_rtp_flag);
                gte_stsz(g_eff_depth_work);
                if (billboard_index == 0) {
                    g_eff_current_tex_rec = D_8009B940;
                } else if (g_eff_billboard_pair_ages[slot_index] < 11) {
                    g_eff_current_tex_rec = D_8009B948;
                } else {
                    g_eff_current_tex_rec = D_8009B950;
                }
                *g_eff_clut_word =
                    (((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                     (g_eff_current_tex_rec->clut_y << 6))
                    << 16;
                *g_eff_tex_left_u = g_eff_current_tex_rec->u;
                *g_eff_tex_top_v = g_eff_current_tex_rec->v;
                *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0x3F;
                *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0x3F;
                *g_eff_top_left_uv =
                    *g_eff_tex_left_u + (*g_eff_tex_top_v << 8);
                *g_eff_top_right_uv =
                    *g_eff_tex_right_u + (*g_eff_tex_top_v << 8);
                *g_eff_bottom_left_uv =
                    *g_eff_tex_left_u + (*g_eff_tex_bottom_v << 8);
                *g_eff_bottom_right_uv =
                    *g_eff_tex_right_u + (*g_eff_tex_bottom_v << 8);
                *ot_index = gpu_DepthToOtIndex(*g_eff_depth_work, 0);
                if (*ot_index < 0x1005 &&
                    *g_eff_geom_screen_work / 1000 >> 4 < *ot_index) {
                    *g_eff_perspective_scale =
                        *g_eff_geom_screen_work / *g_eff_depth_work;
                    /* Sprite size = base size * depth scale, in 1/256 units,
                       at least 8. */
                    *g_eff_right_x_offset =
                        *g_eff_base_width * *g_eff_perspective_scale > 0x800
                            ? *g_eff_base_width * *g_eff_perspective_scale >> 8
                            : 8;
                    *g_eff_lower_y_offset_word =
                        *g_eff_base_height * *g_eff_perspective_scale > 0x800
                            ? *g_eff_base_height * *g_eff_perspective_scale >> 8
                            : 8;
                    if (billboard_index == 0) {
                        if (g_eff_billboard_pair_ages[slot_index] < 5) {
                            *g_eff_right_x_offset =
                                *g_eff_right_x_offset *
                                (rcos((g_eff_billboard_pair_ages[slot_index]
                                       << 10) /
                                      5) *
                                     3 / 4096 +
                                 7) /
                                10;
                        } else if (g_eff_billboard_pair_ages[slot_index] < 16) {
                            *g_eff_right_x_offset =
                                *g_eff_right_x_offset *
                                (rcos(((g_eff_billboard_pair_ages[slot_index] -
                                        5)
                                       << 10) /
                                      11) *
                                 7 / 4096) /
                                10;
                        }
                        if (g_eff_billboard_pair_ages[slot_index] < 10) {
                            *g_eff_lower_y_offset_word =
                                *g_eff_lower_y_offset_word *
                                (rsin((g_eff_billboard_pair_ages[slot_index]
                                       << 10) /
                                      10) *
                                 85 / 4096) /
                                100;
                        } else if (g_eff_billboard_pair_ages[slot_index] < 16) {
                            *g_eff_lower_y_offset_word =
                                *g_eff_lower_y_offset_word *
                                (rcos(((g_eff_billboard_pair_ages[slot_index] -
                                        10)
                                       << 10) /
                                      6) *
                                 85 / 4096) /
                                100;
                        }
                        if (g_working_record.grayscale) {
                            *(s32 *)&prim->r0 = 0x808080;
                        } else {
                            *(s32 *)&prim->r0 = 0xFF8080;
                        }
                    } else {
                        scale =
                            rsin((g_eff_billboard_pair_ages[slot_index] << 10) /
                                 12) *
                            12 / 4096;
                        *g_eff_right_x_offset =
                            *g_eff_right_x_offset * scale / 10;
                        *g_eff_lower_y_offset_word =
                            *g_eff_lower_y_offset_word * scale / 10;
                        if (g_eff_billboard_pair_ages[slot_index] >= 9) {
                            /* *var_6 = age - 15: the colour word below is built
                             * from -*var_6 * 128 / 5 and -*var_6 * 255 / 5. */
                            *var_6 = g_eff_billboard_pair_ages[slot_index] - 15;
                            red_green_level = (*var_6 * -128 / 5) & 0xFF;
                            *(s32 *)&prim->r0 =
                                red_green_level + (red_green_level << 8) +
                                ((*var_6 * -255 / 5 & 0xFF) << 16);
                        } else {
                            *(s32 *)&prim->r0 = 0;
                        }
                    }
                    setlen(prim, 9);
                    prim->code = 0x2E;
                    *g_eff_right_x_offset = *g_eff_right_x_offset >> 1;
                    *D_800A34BC = -*g_eff_right_x_offset;
                    *g_eff_upper_y_offset =
                        *g_eff_lower_y_offset_word * 60 / 64;
                    *g_eff_lower_y_offset_word = 0;
                    *(s32 *)&prim->x0 = *g_eff_screen_xy + *D_800A34BC -
                                        (*g_eff_upper_y_offset << 16);
                    *(s32 *)&prim->x1 =
                        *g_eff_screen_xy + *g_eff_right_x_offset -
                        (*g_eff_upper_y_offset << 16);
                    *(s32 *)&prim->x2 = *g_eff_screen_xy + *D_800A34BC +
                                        (*g_eff_lower_y_offset_word << 16);
                    *(s32 *)&prim->x3 =
                        *g_eff_screen_xy + *g_eff_right_x_offset +
                        (*g_eff_lower_y_offset_word << 16);
                    *(s32 *)&prim->u0 = *g_eff_top_left_uv + *g_eff_clut_word;
                    *(s32 *)&prim->u1 = *g_eff_top_right_uv + *g_eff_tpage_word;
                    *(u16 *)&prim->u2 = *g_eff_bottom_left_uv;
                    *(u16 *)&prim->u3 = *g_eff_bottom_right_uv;
                    g_eff_current_ot_entry = g_gpu_ot_ptr + *ot_index;
                    g_eff_current_prim_tag = &prim->tag;
                    *g_eff_current_prim_tag =
                        (*g_eff_current_prim_tag & 0xFF000000) |
                        (*g_eff_current_ot_entry & 0xFFFFFF);
                    *g_eff_current_ot_entry =
                        ((u32)g_eff_current_prim_tag & 0xFFFFFF) |
                        (*g_eff_current_ot_entry & 0xFF000000);
                    if (prim - g_eff_quad_buffer_base < 0x1C1) {
                        prim++;
                    }
                }
            } else {
                g_eff_billboard_pair_slot_mask &= 0xFFFF - (1 << slot_index);
            }
        }
        g_eff_billboard_pair_ages[slot_index]++;
    }
    g_eff_quad_cursor = prim;
    return g_eff_billboard_pair_slot_mask != 0;
}

u8 eff_AllocateOrRecycleYawedQuadSlot(s32);

u8 func_80063AF0(void) {
    g_eff_command_phase[0] = 1;
    D_800A345C[0] = (g_eff_active_command_block->unk00.w >> 17) & 3;
    return eff_AllocateOrRecycleYawedQuadSlot(0);
}

u8 func_80063B34(void) {
    g_eff_command_phase[1] = 1;
    D_800A345C[1] = (g_eff_active_command_block->unk00.w >> 17) & 3;
    return eff_AllocateOrRecycleYawedQuadSlot(1);
}

s32 eff_DrawYawedSlotQuads(s32);
void eff_BuildUsedSlotRotMatrices(s32 *count, MATRIX *m, s32 idx);

u8 func_80063B78(void) {
    *D_800A3480 = D_800A345C[0];
    return eff_DrawYawedSlotQuads(0);
}

u8 func_80063BA4(void) {
    *D_800A3480 = D_800A345C[1];
    return eff_DrawYawedSlotQuads(1);
}

extern SVECTOR g_eff_yawed_quad_slot_rotations[][10];

/* eff_AllocateOrRecycleYawedQuadSlot -- slot allocator for command slot
 * `command_slot_index`: g_eff_yawed_quad_slot_counters[command_slot_index]
 * counts live entries, and g_eff_yawed_quad_slot_masks[command_slot_index] is
 * the per-slot in-use bitmask. While fewer than 10 entries are live, take the
 * lowest free bit, mark it, and fill that slot's SVECTOR
 * (g_eff_yawed_quad_slot_rotations[command_slot_index][slot]) and 3-word record
 * (g_eff_yawed_quad_slot_positions[command_slot_index][slot]) from
 * g_eff_command_vector / g_eff_command_position. Once the command slot is full,
 * the counter wraps through 10..19 and slots are overwritten in rotation. */
u8 eff_AllocateOrRecycleYawedQuadSlot(s32 command_slot_index) {
    s32 slot_bit;
    s32 slot_index;

    if (g_eff_yawed_quad_slot_counters[command_slot_index] < 10) {
        g_eff_yawed_quad_slot_counters[command_slot_index]++;
        for (slot_index = 0;
             slot_index < g_eff_yawed_quad_slot_counters[command_slot_index];
             slot_index++) {
            if (!(g_eff_yawed_quad_slot_masks[command_slot_index] &
                  (slot_bit = 1 << slot_index))) {
                g_eff_yawed_quad_slot_masks[command_slot_index] |= slot_bit;
                g_eff_yawed_quad_slot_rotations[command_slot_index][slot_index]
                    .vy = g_eff_command_vector[1];
                g_eff_yawed_quad_slot_rotations[command_slot_index][slot_index]
                    .vx = g_eff_yawed_quad_slot_rotations[command_slot_index]
                                                         [slot_index]
                                                             .vz = 0;
                g_eff_yawed_quad_slot_positions[command_slot_index][slot_index]
                    .unk0 = g_eff_command_position[0];
                g_eff_yawed_quad_slot_positions[command_slot_index][slot_index]
                    .unk4 = g_eff_command_position[1];
                g_eff_yawed_quad_slot_positions[command_slot_index][slot_index]
                    .unk8 = g_eff_command_position[2];
                break;
            }
        }
    } else {
        g_eff_yawed_quad_slot_counters[command_slot_index] =
            (g_eff_yawed_quad_slot_counters[command_slot_index] + 1) % 10 + 10;
        g_eff_yawed_quad_slot_rotations
            [command_slot_index]
            [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                .vy = g_eff_command_vector[1];
        g_eff_yawed_quad_slot_rotations
            [command_slot_index]
            [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                .vx =
            g_eff_yawed_quad_slot_rotations
                [command_slot_index]
                [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                    .vz = 0;
        g_eff_yawed_quad_slot_positions
            [command_slot_index]
            [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                .unk0 = g_eff_command_position[0];
        g_eff_yawed_quad_slot_positions
            [command_slot_index]
            [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                .unk4 = g_eff_command_position[1];
        g_eff_yawed_quad_slot_positions
            [command_slot_index]
            [g_eff_yawed_quad_slot_counters[command_slot_index] - 10]
                .unk8 = g_eff_command_position[2];
    }
    return 1;
}

/* Draw the live slots (up to 10, see eff_AllocateOrRecycleYawedQuadSlot) of
 * `arg0`: per slot whose bit is set in g_eff_yawed_quad_slot_masks[arg0], emit
 * one textured POLY_FT4 billboard at the slot's position (relative to
 * *g_eff_view_origin) through the composite of g_eff_view_rotation_matrix and
 * the slot's matrix, keep it when its depth is in range, then link every new
 * quad into the OT at its depth. Returns 1. */
s32 eff_DrawYawedSlotQuads(s32 arg0) {
    extern SVECTOR D_8009BBE4;
    extern SVECTOR D_8009BBEC;
    extern SVECTOR D_8009BBF4;
    extern SVECTOR D_8009BBFC;
    s32 count;
    MATRIX *mats;
    MATRIX *var_2;
    s32 *sxy;
    u16 *quad_ot_indices;
    u16 *var_5;
    POLY_FT4 *prim;
    POLY_FT4 *quads_end;
    s32 i;
    s32 k;
    s32 bit;

    /* FAKE: the work-area base (g_eff_work_area) is staged through `prim`
     * before prim becomes the quad cursor: prim's pseudo lives across the calls
     * and gets s2, where the target holds the base. Separate u8 *base: 12;
     * inline mats argument: 9; all four other pointers after call in
     * declaration / var_5-first order: 56 / 88; direct sxy: 17;
     * direct var_2: 83; direct var_2 + sxy: 41; direct sxy with
     * s32 / void * / MATRIX *base: 17 each; direct sxy plus other pointers
     * after call: 50; direct sxy with load-then-clamp count: 47. */
    prim = (POLY_FT4 *)g_eff_work_area;
    mats = (MATRIX *)((u8 *)prim + 0x28);
    var_2 = (MATRIX *)((u8 *)prim + 0x168);
    sxy = (s32 *)((u8 *)prim + 0x188);
    quad_ot_indices = (u16 *)((u8 *)prim + 0x19C);
    var_5 = (u16 *)((u8 *)prim + 0x1B0);
    if (g_eff_yawed_quad_slot_counters[arg0] < 10) {
        count = g_eff_yawed_quad_slot_counters[arg0];
    } else {
        count = 10;
    }
    eff_BuildUsedSlotRotMatrices(&count, mats, arg0);
    g_eff_current_tex_rec = &D_8009B920[*D_800A3480];
    prim = g_eff_quad_cursor;
    *g_eff_tpage_word = 0xE;
    *g_eff_clut_word = (((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                        (g_eff_current_tex_rec->clut_y << 6))
                       << 16;
    *g_eff_tpage_word = *g_eff_tpage_word << 16;
    *g_eff_tex_left_u = g_eff_current_tex_rec->u;
    *g_eff_tex_top_v = g_eff_current_tex_rec->v;
    *g_eff_tex_right_u = g_eff_current_tex_rec->u + 7;
    *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0xF;
    *g_eff_top_left_uv = *g_eff_tex_left_u + (*g_eff_tex_top_v << 8);
    *g_eff_top_right_uv = *g_eff_tex_right_u + (*g_eff_tex_top_v << 8);
    *g_eff_bottom_left_uv = *g_eff_tex_left_u + (*g_eff_tex_bottom_v << 8);
    *g_eff_bottom_right_uv = *g_eff_tex_right_u + (*g_eff_tex_bottom_v << 8);
    *var_5 = 0;
    *g_eff_geom_screen_work = ReadGeomScreen();
    for (i = 0; i < count; i++) {
        /* FAKE: the slot's mask is named `bit` inside the test and not read
         * again; it stretches the g_eff_yawed_quad_slot_masks[arg0] address's
         * life so loop.c hoists it (target spills it to 32(sp)), and keeps
         * combine from making the test srav/andi. Without `bit`: score 43. */
        if (!(g_eff_yawed_quad_slot_masks[arg0] & (bit = 1 << i))) {
            continue;
        }
        setlen(prim, 9);
        prim->code = 0x2F;
        *(s32 *)&prim->u0 = *g_eff_top_left_uv + *g_eff_clut_word;
        *(s32 *)&prim->u1 = *g_eff_top_right_uv + *g_eff_tpage_word;
        *(u16 *)&prim->u2 = *g_eff_bottom_left_uv;
        *(u16 *)&prim->u3 = *g_eff_bottom_right_uv;
        mats[i].t[0] = g_eff_yawed_quad_slot_positions[arg0][i].unk0 -
                       g_eff_view_origin[0];
        mats[i].t[1] = g_eff_yawed_quad_slot_positions[arg0][i].unk4 -
                       g_eff_view_origin[1];
        mats[i].t[2] = g_eff_yawed_quad_slot_positions[arg0][i].unk8 -
                       g_eff_view_origin[2];
        gte_CompMatrix(g_eff_view_rotation_matrix, &mats[i], var_2);
        gte_SetRotMatrix(var_2);
        gte_SetTransMatrix(var_2);
        gte_ldv3(&D_8009BBE4, &D_8009BBEC, &D_8009BBF4);
        gte_rtpt();
        gte_stsxy3(sxy, sxy + 1, sxy + 2);
        gte_stsz(g_eff_depth_work);
        if (*g_eff_depth_work <= 0) {
            continue;
        }
        *g_eff_depth_work = gpu_DepthToOtIndex(*g_eff_depth_work - 50, 0);
        if (*g_eff_depth_work >= 0x1005) {
            continue;
        }
        if ((*g_eff_geom_screen_work >> 4) >= *g_eff_depth_work) {
            continue;
        }
        quad_ot_indices[(*var_5)++] = *g_eff_depth_work;
        gte_ldv0(&D_8009BBFC);
        gte_rtps();
        gte_stsxy(sxy + 3);
        *(s32 *)&prim->x0 = sxy[0];
        *(s32 *)&prim->x1 = sxy[1];
        *(s32 *)&prim->x2 = sxy[2];
        *(s32 *)&prim->x3 = sxy[3];
        if (prim - g_eff_quad_buffer_base < 0x1C1) {
            prim++;
        }
    }
    /* `quads_end` keeps the fill position; the quad cursor prim restarts at
     * g_eff_quad_cursor to link each quad, the way SOTN's menu code restarts
     * its fill cursor at the buffer to read it back (Q51) */
    /* SOTN: src/dra/menu.c:3352 @db41b28 */
    quads_end = prim;
    for (prim = g_eff_quad_cursor, k = 0; prim < quads_end; prim++, k++) {
        g_eff_current_prim_tag = &prim->tag;
        g_eff_current_ot_entry = g_gpu_ot_ptr + quad_ot_indices[k];
        *g_eff_current_prim_tag = (*g_eff_current_prim_tag & 0xFF000000) |
                                  (*g_eff_current_ot_entry & 0xFFFFFF);
        *g_eff_current_ot_entry = ((u32)g_eff_current_prim_tag & 0xFFFFFF) |
                                  (*g_eff_current_ot_entry & 0xFF000000);
    }
    g_eff_quad_cursor = quads_end;
    return 1;
}

/* eff_BuildUsedSlotRotMatrices -- rotates one matrix per enabled bit: for every
 * i < *slot_count whose bit is set in
 * g_eff_yawed_quad_slot_masks[command_slot_index],
 * RotMatrix(&g_eff_yawed_quad_slot_rotations[command_slot_index][i],
 * &slot_matrices[i]).
 */
void eff_BuildUsedSlotRotMatrices(
    s32 *slot_count, MATRIX *slot_matrices, s32 command_slot_index) {
    s32 i;
    s32 mask;

    for (i = 0; i < *slot_count; i++) {
        /* FAKE: setting mask twice keeps the target's `li v0,1; sllv` inside
         * the loop; `mask = 1 << i` hoists the 1 into s5: score 12 (Q129) */
        mask = 1;
        mask <<= i;
        if (g_eff_yawed_quad_slot_masks[command_slot_index] & mask) {
            RotMatrix(&g_eff_yawed_quad_slot_rotations[command_slot_index][i],
                      &slot_matrices[i]);
        }
    }
}

/* eff_SpawnJitteredBillboards -- for each group of four slots (bits of
 * g_eff_jittered_billboard_slot_mask), claim the first free slot: set its
 * position D_800F0D78[slot] to the position at g_eff_command_position jittered
 * by (rand() & 0xFF) - 0x7F per axis, give it a random 0..7 in
 * g_eff_jittered_billboard_ages, and mark its bit. */
s32 eff_SpawnJitteredBillboards(void) {
    s32 group_start_index;
    s32 group_slot_index;
    s32 slot_index;
    s32 slot_bit;

    g_eff_command_phase[7] = 1;
    for (group_start_index = 0; group_start_index < 0xF;
         group_start_index += 4) {
        for (group_slot_index = 0; group_slot_index < 4; group_slot_index++) {
            slot_index = group_start_index + group_slot_index;
            slot_bit = 1 << slot_index;
            if (!(g_eff_jittered_billboard_slot_mask & slot_bit)) {
                D_800F0D78[slot_index].x =
                    g_eff_command_position[0] + (rand() & 0xFF) - 0x7F;
                D_800F0D78[slot_index].y =
                    g_eff_command_position[1] + (rand() & 0xFF) - 0x7F;
                D_800F0D78[slot_index].z =
                    g_eff_command_position[2] + (rand() & 0xFF) - 0x7F;
                g_eff_jittered_billboard_ages[slot_index] = rand() & 7;
                g_eff_jittered_billboard_slot_mask |= slot_bit;
                break;
            }
        }
    }
    return 1;
}

/* Draw the up-to-16 effect slots eff_SpawnJitteredBillboards spawns: per live
   slot (bit slot_index of g_eff_jittered_billboard_slot_mask) step its counter
   g_eff_jittered_billboard_ages[slot_index]; while counter / 4
   is below 7, project the slot's world position through
   g_eff_view_rotation_matrix and, when its depth is in range, emit one
   textured, semi-transparent POLY_FT4 billboard sized by the screen depth and
   step the counter again; once counter / 4 reaches 7 the slot is retired.
   Finally link every new quad into the OT at its depth. Returns 1 when any slot
   is still live, else 0. */
s32 eff_DrawAndAdvanceJitteredBillboards(void) {
    Unk1F8000B8_800646E8 *base;
    u32 *quad_ot_read_cursor;
    s16 *billboard_width;
    s16 *billboard_height;
    s32 *frame_index;
    VECTOR *var_5;
    VECTOR *relative_position;
    SVECTOR *local_origin;
    s32 *p;
    POLY_FT4 **quads_end;
    POLY_FT4 *prim;
    u32 *quad_ot_write_cursor;
    s16 slot_index;
    s32 bit;

    base = &g_eff_work_area->v800646E8;
    quad_ot_read_cursor = base->quad_ot_index_table;
    billboard_width = &base->billboard_screen_width;
    billboard_height = &base->billboard_screen_height;
    frame_index = &base->billboard_frame_index;
    var_5 = &base->rotated_position;
    relative_position = &base->view_relative_position;
    local_origin = &base->unk38;
    p = &base->unk40;
    quads_end = &base->unk44;
    prim = g_eff_quad_cursor;
    gte_SetRotMatrix(g_eff_view_rotation_matrix);
    local_origin->vx = local_origin->vy = local_origin->vz = 0;
    *g_eff_geom_screen_work = ReadGeomScreen() * 1000;
    *g_eff_base_width = 0x40;
    *g_eff_base_height = 0x20;
    *g_eff_tpage_word = 0xE;
    for (slot_index = 0, quad_ot_write_cursor = quad_ot_read_cursor;
         slot_index < 16; slot_index++) {
        bit = 1 << slot_index;
        if (!(g_eff_jittered_billboard_slot_mask & bit)) {
            continue;
        }
        g_eff_jittered_billboard_ages[slot_index]++;
        *frame_index = g_eff_jittered_billboard_ages[slot_index] / 4;
        if (*frame_index < 7) {
            relative_position->vx =
                D_800F0D78[slot_index].x - g_eff_view_origin[0];
            relative_position->vy =
                D_800F0D78[slot_index].y - g_eff_view_origin[1];
            relative_position->vz =
                D_800F0D78[slot_index].z - g_eff_view_origin[2];
            ApplyRotMatrixLV(relative_position, var_5);
            /* FAKE: the MATRIX whose t[] is *var_5: SetTransMatrix reads only
             * m->t (there is no whole MATRIX here). Spelled
             * (MATRIX *)((u8 *)base + 4): score 68. */
            SetTransMatrix((MATRIX *)((u8 *)var_5 - 0x14));
            RotTransPers(local_origin, g_eff_screen_xy, p, g_eff_rtp_flag);
            gte_stsz(g_eff_depth_work);
            g_eff_current_tex_rec = &D_8009B8E8[*frame_index];
            *g_eff_clut_word =
                (u16)(((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                      (g_eff_current_tex_rec->clut_y << 6));
            *g_eff_tex_left_u = g_eff_current_tex_rec->u;
            *g_eff_tex_top_v = g_eff_current_tex_rec->v;
            *g_eff_tex_right_u = g_eff_current_tex_rec->u + 0x3F;
            *g_eff_tex_bottom_v = g_eff_current_tex_rec->v + 0x1F;
            *quad_ot_write_cursor = gpu_DepthToOtIndex(*g_eff_depth_work, 0);
            if (*quad_ot_write_cursor >= 0x1005) {
                continue;
            }
            if ((*g_eff_geom_screen_work / 1000 >> 4) >=
                *quad_ot_write_cursor) {
                continue;
            }
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work / *g_eff_depth_work;
            *billboard_width =
                *g_eff_base_width * *g_eff_perspective_scale > 0x800
                    ? (*g_eff_base_width * *g_eff_perspective_scale) >> 8
                    : 8;
            *billboard_height =
                *g_eff_base_height * *g_eff_perspective_scale > 0x800
                    ? (*g_eff_base_height * *g_eff_perspective_scale) >> 8
                    : 8;
            *billboard_width += (*billboard_width *
                                 g_eff_jittered_billboard_ages[slot_index]) >>
                                5;
            *billboard_height +=
                (*billboard_height *
                 (g_eff_jittered_billboard_ages[slot_index] << 2)) >>
                8;
            SetPolyFT4(prim);
            prim->tpage = *g_eff_tpage_word;
            prim->clut = *g_eff_clut_word;
            if (g_working_record.grayscale) {
                prim->r0 = 0x80;
                prim->g0 = 0x80;
                prim->b0 = 0x80;
            } else {
                prim->r0 = 0xA0;
                prim->g0 = 0x8C;
                prim->b0 = 0x50;
            }
            prim->x0 = *g_eff_screen_xy - *billboard_width / 2;
            prim->y0 = (*g_eff_screen_xy >> 16) - *billboard_height * 28 / 32;
            prim->x1 = *g_eff_screen_xy + *billboard_width / 2;
            prim->y1 = (*g_eff_screen_xy >> 16) - *billboard_height * 28 / 32;
            prim->x2 = *g_eff_screen_xy - *billboard_width / 2;
            prim->y2 = (*g_eff_screen_xy >> 16) + *billboard_height / 8;
            prim->x3 = *g_eff_screen_xy + *billboard_width / 2;
            prim->y3 = (*g_eff_screen_xy >> 16) + *billboard_height / 8;
            prim->u0 = *g_eff_tex_left_u;
            prim->v0 = *g_eff_tex_top_v;
            prim->u1 = *g_eff_tex_right_u;
            prim->v1 = *g_eff_tex_top_v;
            prim->u2 = *g_eff_tex_left_u;
            prim->v2 = *g_eff_tex_bottom_v;
            prim->u3 = *g_eff_tex_right_u;
            prim->v3 = *g_eff_tex_bottom_v;
            SetShadeTex(prim, 0);
            SetSemiTrans(prim, 1);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                prim++;
                quad_ot_write_cursor++;
            }
            g_eff_jittered_billboard_ages[slot_index]++;
        } else {
            g_eff_jittered_billboard_slot_mask &= 0xFFFF - bit;
        }
    }
    if (g_eff_jittered_billboard_slot_mask == 0) {
        return 0;
    }
    *quads_end = prim;
    for (prim = g_eff_quad_cursor; prim < *quads_end; prim++,
        quad_ot_read_cursor++) {
        g_eff_current_ot_entry = g_gpu_ot_ptr + *quad_ot_read_cursor;
        g_eff_current_prim_tag = &prim->tag;
        *g_eff_current_prim_tag = (*g_eff_current_prim_tag & 0xFF000000) |
                                  (*g_eff_current_ot_entry & 0xFFFFFF);
        *g_eff_current_ot_entry = ((u32)g_eff_current_prim_tag & 0xFFFFFF) |
                                  (*g_eff_current_ot_entry & 0xFF000000);
    }
    g_eff_quad_cursor = *quads_end;
    return 1;
}

void eff_InitCosineScaledQuad(void) {
    g_eff_animated_quad_positions[0].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[0].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[0].unk8 = g_eff_command_position[2];
    g_eff_command_phase[4] = 1;
    g_eff_animated_quad_counters[0] = 0;
}

void func_80064ED8(void) {
    g_eff_animated_quad_positions[1].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[1].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[1].unk8 = g_eff_command_position[2];
    g_eff_command_phase[5] = 1;
    g_eff_animated_quad_counters[1] = 0;
}

void func_80064F20(void) {
    g_eff_animated_quad_positions[2].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[2].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[2].unk8 = g_eff_command_position[2];
    g_eff_command_phase[6] = 1;
    g_eff_animated_quad_counters[2] = 0;
}

s32 func_80064F68(void) {
    g_eff_animated_quad_positions[3].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[3].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[3].unk8 = g_eff_command_position[2];
    g_eff_command_phase[9] = 1;
    g_eff_animated_quad_counters[3] = 0x40;
    return 1;
}

s32 func_80064FB4(void) {
    g_eff_animated_quad_positions[4].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[4].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[4].unk8 = g_eff_command_position[2];
    g_eff_command_phase[10] = 1;
    g_eff_animated_quad_counters[4] = 0x40;
    return 1;
}

s32 eff_InitRateSelectedCosineScaledQuad(void) {
    g_eff_animated_quad_positions[5].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[5].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[5].unk8 = g_eff_command_position[2];
    g_eff_command_phase[11] = 1;
    g_eff_animated_quad_counters[5] = 0;
    D_800A3440 = (g_eff_active_command_block->unk00.w >> 19) & 3;
    return 1;
}

void func_8006505C(void) {
    g_eff_animated_quad_positions[6].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[6].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[6].unk8 = g_eff_command_position[2];
    g_eff_command_phase[12] = 1;
    g_eff_animated_quad_counters[6] = 0;
}

void func_800650A4(void) {
    g_eff_animated_quad_positions[7].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[7].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[7].unk8 = g_eff_command_position[2];
    g_eff_command_phase[13] = 1;
    g_eff_animated_quad_counters[7] = 0;
}

void func_800650EC(void) {
    g_eff_animated_quad_positions[10].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[10].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[10].unk8 = g_eff_command_position[2];
    g_eff_command_phase[14] = 1;
    g_eff_animated_quad_counters[10] = 0;
}

void func_80065134(void) {
    g_eff_animated_quad_positions[11].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[11].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[11].unk8 = g_eff_command_position[2];
    g_eff_command_phase[15] = 1;
    g_eff_animated_quad_counters[11] = 0;
}

void func_8006517C(void) {
    g_eff_animated_quad_positions[12].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[12].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[12].unk8 = g_eff_command_position[2];
    g_eff_animated_quad_counters[12] = 0;
    g_eff_animated_quad_positions[14].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[14].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[14].unk8 = g_eff_command_position[2];
    g_eff_command_phase[16] = 1;
    g_eff_animated_quad_counters[14] = 0;
}

void func_800651F0(void) {
    g_eff_animated_quad_positions[13].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[13].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[13].unk8 = g_eff_command_position[2];
    g_eff_animated_quad_counters[13] = 0;
    g_eff_animated_quad_positions[15].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[15].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[15].unk8 = g_eff_command_position[2];
    g_eff_command_phase[17] = 1;
    g_eff_animated_quad_counters[15] = 0;
}

void func_80065264(void) {
    g_eff_animated_quad_positions[16].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[16].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[16].unk8 = g_eff_command_position[2];
    g_eff_command_phase[18] = 1;
    g_eff_animated_quad_counters[16] = 0;
}

void func_800652AC(void) {
    g_eff_animated_quad_positions[17].unk0 = g_eff_command_position[0];
    g_eff_animated_quad_positions[17].unk4 = g_eff_command_position[1];
    g_eff_animated_quad_positions[17].unk8 = g_eff_command_position[2];
    g_eff_command_phase[19] = 1;
    g_eff_animated_quad_counters[17] = 0;
}

u8 eff_DrawAnimatedQuads(s32);

u8 eff_DrawAndAdvanceCosineScaledQuad(void) {
    u8 draw_result = eff_DrawAnimatedQuads(0);
    s16 *p = &g_eff_animated_quad_counters[0];
    s16 quad_counter = *p;
    quad_counter += 0x1FF;
    *p = quad_counter;
    if ((s16)quad_counter < 0x1001) {
        return draw_result;
    }
    return 0;
}

u8 func_80065344(void) {
    u8 draw_result = eff_DrawAnimatedQuads(1);
    s16 *p = &g_eff_animated_quad_counters[1];
    u16 quad_counter = *p;
    quad_counter += 0x1C6;
    *p = quad_counter;
    if ((s16)quad_counter < 0x11C8) {
        return draw_result;
    }
    return 0;
}

u8 func_80065394(void) {
    u8 draw_result = eff_DrawAnimatedQuads(2);
    s16 *p = &g_eff_animated_quad_counters[2];
    s16 quad_counter = *p;
    quad_counter += 0x1C6;
    *p = quad_counter;
    if ((s16)quad_counter < 0x11C8) {
        return draw_result;
    }
    return 0;
}

u8 func_800653E4(void) {
    u8 draw_result = eff_DrawAnimatedQuads(3);
    s16 *p = &g_eff_animated_quad_counters[3];
    s16 quad_counter = *p;
    quad_counter += 0x19;
    *p = quad_counter;
    if ((s16)quad_counter < 0xC9) {
        return draw_result;
    }
    return 0;
}

u8 func_80065434(void) {
    u8 draw_result = eff_DrawAnimatedQuads(4);
    s16 *p = &g_eff_animated_quad_counters[4];
    u16 quad_counter = *p;
    quad_counter += 0x19;
    *p = quad_counter;
    if ((s16)quad_counter < 0xC9) {
        return draw_result;
    }
    return 0;
}

u8 eff_DrawAndAdvanceRateSelectedCosineScaledQuad(void) {
    unsigned int rate_selector;
    u8 draw_result;
    *D_800A3484 = D_800A3440;
    draw_result = eff_DrawAnimatedQuads(5);
    rate_selector = *D_800A3484;
    switch (rate_selector) {
    case 0: {
        s16 *p = &g_eff_animated_quad_counters[5];
        *p = *p + 0x1FF;
        break;
    }
    case 1: {
        s16 *p = &g_eff_animated_quad_counters[5];
        *p = *p + 0x3FE;
        break;
    }
    case 2: {
        s16 *p = &g_eff_animated_quad_counters[5];
        *p = *p + 0x5FD;
        break;
    }
    }
    if (g_eff_animated_quad_counters[5] < 0x2001) {
        return draw_result;
    }
    return 0;
}

u8 func_80065540(void) {
    u8 draw_result = eff_DrawAnimatedQuads(6);
    s16 *p = &g_eff_animated_quad_counters[6];
    u16 quad_counter = *p;
    quad_counter += 0x32;
    *p = quad_counter;
    if ((s16)quad_counter < 0x100) {
        return draw_result;
    }
    return 0;
}

u8 func_80065590(void) {
    u8 draw_result = eff_DrawAnimatedQuads(7);
    s16 *p = &g_eff_animated_quad_counters[7];
    u16 quad_counter = *p;
    quad_counter += 0x32;
    *p = quad_counter;
    if ((s16)quad_counter < 0x100) {
        return draw_result;
    }
    return 0;
}

u8 func_800655E0(void) {
    u8 draw_result = eff_DrawAnimatedQuads(0xA);
    s16 *p = &g_eff_animated_quad_counters[10];
    s16 quad_counter = *p;
    quad_counter += 0x32;
    *p = quad_counter;
    if ((s16)quad_counter < 0x100) {
        return draw_result;
    }
    return 0;
}

u8 func_80065630(void) {
    u8 draw_result = eff_DrawAnimatedQuads(0xB);
    s16 *p = &g_eff_animated_quad_counters[11];
    s16 quad_counter = *p;
    quad_counter += 0x32;
    *p = quad_counter;
    if ((s16)quad_counter < 0x100) {
        return draw_result;
    }
    return 0;
}

s32 eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires(void) {
    s16 *first_quad_counter;
    s32 return_value;
    eff_DrawAnimatedQuads(0xC);
    first_quad_counter = &g_eff_animated_quad_counters[12];
    *first_quad_counter = *first_quad_counter + 1;
    return_value = eff_DrawAnimatedQuads(0xE);
    g_eff_animated_quad_counters[14] = g_eff_animated_quad_counters[14] + 1;
    return_value &= 0xFF;
    if (g_eff_animated_quad_counters[14] >= 11) {
        return_value = 0;
    }
    return return_value;
}

s32 eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires(void) {
    s16 *first_quad_counter = &g_eff_animated_quad_counters[13];
    s32 second_draw_result;
    eff_DrawAnimatedQuads(0xD);
    *first_quad_counter = *first_quad_counter + 1;
    second_draw_result = eff_DrawAnimatedQuads(0xF);
    g_eff_animated_quad_counters[15] = g_eff_animated_quad_counters[15] + 1;
    if ((s16)*first_quad_counter < 11) {
        return second_draw_result & 0xFF;
    }
    return 0;
}

u8 func_80065760(void) {
    u8 draw_result = eff_DrawAnimatedQuads(0x10);
    s16 *p = &g_eff_animated_quad_counters[16];
    s16 quad_counter = *p;
    quad_counter += 0x1C6;
    *p = quad_counter;
    if (quad_counter < 0x11C8) {
        return draw_result;
    }
    return 0;
}

u8 func_800657B0(void) {
    u8 draw_result = eff_DrawAnimatedQuads(0x11);
    s16 *p = &g_eff_animated_quad_counters[17];
    s16 quad_counter = *p;
    quad_counter += 0x1C6;
    *p = quad_counter;
    if ((s16)quad_counter < 0x11C8) {
        return draw_result;
    }
    return 0;
}

/* Shared draw routine of the per-mode effect wrappers
 * (eff_DrawAndAdvanceCosineScaledQuad..func_800657B0, mode 0..0x11): projects
 * this mode's g_eff_animated_quad_positions position record through the current
 * camera, picks the texture/colour and quad size from the mode and its
 * g_eff_animated_quad_counters timer, shapes the four corners (switch 2) and
 * links one POLY_FT4 into the OT. Modes 6/7 re-run the body for the paired mode
 * (mode_index + 2, i.e. 8/9) via `goto again`; the re-run is guarded by
 * `mode_index < 9`, so modes 10/11 draw once. */
u8 eff_DrawAnimatedQuads(s32 mode_index) {
    extern s16 g_main_loop_handler_index;
    extern s32 ReadGeomScreen(void);
    Unk1F8000B8_80065800 *var_0;
    POLY_FT4 *prim;
    s32 *var_2;
    VECTOR *p_t;
    VECTOR *relative_position;
    SVECTOR *p_v;
    s16 *p_w;
    s16 *p_h;
    MATRIX *rotation_matrix;
    s16 *texture_width;
    s16 *texture_height;
    VECTOR *dst;
    s32 brightness;
    s32 w;
    s32 width_scale;
    s32 height_scale;
    s32 h;
    s16 *quad_counter;
    /* FAKE: pointer alias of g_eff_animated_quad_counters - case 10/11's base
     * address in its own register ahead of the index shift */
    s16 *var_18;
    s32 vertex_index;

    var_0 = &g_eff_work_area->v80065800;
    prim = g_eff_quad_cursor;
    var_2 = &var_0->depth_cue;
    p_t = var_0->unk14;
    relative_position = &var_0->view_relative_position;
    p_v = &var_0->unk64;
    p_w = &var_0->quad_extent_x;
    p_h = &var_0->quad_extent_y;
    rotation_matrix = &var_0->corner_rotation_matrix;
    texture_width = &var_0->tex_width;
    texture_height = &var_0->tex_height;

    gte_SetRotMatrix(g_eff_view_rotation_matrix);
    relative_position->vx =
        g_eff_animated_quad_positions[mode_index].unk0 - g_eff_view_origin[0];
    relative_position->vy =
        g_eff_animated_quad_positions[mode_index].unk4 - g_eff_view_origin[1];
    relative_position->vz =
        g_eff_animated_quad_positions[mode_index].unk8 - g_eff_view_origin[2];
    ApplyRotMatrixLV(relative_position, p_t);
    gte_SetTransMatrix((MATRIX *)var_0);
    p_v->vz = 0;
    p_v->vy = 0;
    p_v->vx = 0;
    gte_RotTransPers(
        p_v, g_eff_screen_xy, var_2, g_eff_rtp_flag, g_eff_depth_work);
    *g_eff_depth_work = *g_eff_depth_work ? *g_eff_depth_work << 2 : 1;

again:
    *g_eff_base_width = 0x20;
    *g_eff_base_height = 0x20;
    *texture_width = 0x40;
    *texture_height = 0x40;
    *g_eff_tpage_word = 0x2E;
    switch (mode_index) {
    case 1:
    case 2:
        if (*g_eff_depth_work > 4000) {
            g_eff_current_tex_rec = D_8009B8D0;
        } else {
            g_eff_current_tex_rec = D_8009B8D8;
        }
        prim->r0 = 0xFF;
        prim->g0 = 0x80;
        prim->b0 = 0x80;
        break;
    case 3:
    case 4:
        if (g_eff_animated_quad_counters[mode_index] < 0x96) {
            g_eff_current_tex_rec = D_8009B980;
        } else {
            g_eff_current_tex_rec = D_8009B978;
        }
        prim->r0 = 0xFF;
        prim->g0 = g_eff_animated_quad_counters[mode_index];
        prim->b0 = 0x60;
        *g_eff_base_width = 0xC0;
        *g_eff_base_height = 0xC0;
        break;
    case 5:
        if (g_eff_animated_quad_counters[4] > 0x800) {
            g_eff_current_tex_rec = D_8009B8E0;
        } else {
            g_eff_current_tex_rec = D_8009B8C8;
        }
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0xFF;
        break;
    case 0:
        g_eff_current_tex_rec = D_8009B8C8;
        prim->r0 = 0x80;
        prim->g0 = 0x80;
        prim->b0 = 0xFF;
        break;
    case 10:
    case 11:
        if (g_working_record.view_setting && g_main_loop_handler_index != 5) {
            g_eff_current_tex_rec = D_8009B9D8;
        } else {
            g_eff_current_tex_rec = D_8009B990;
        }
        var_18 = g_eff_animated_quad_counters;
        quad_counter = var_18 + mode_index;
        if (*quad_counter > 0x96) {
            prim->r0 = 0;
            prim->g0 = (0xFF - *quad_counter) * 0xFF / 105;
        } else {
            prim->r0 = 0;
            prim->g0 = 0xFF;
        }
        prim->b0 = 0x7F - *quad_counter / 2;
        goto size_sel;
    case 6:
    case 7:
        if (g_eff_animated_quad_counters[mode_index] < 0x96) {
            if (g_working_record.view_setting &&
                g_main_loop_handler_index != 5) {
                g_eff_current_tex_rec = D_8009B9D8;
            } else {
                g_eff_current_tex_rec = D_8009B988;
            }
        } else if (
            g_working_record.view_setting && g_main_loop_handler_index != 5) {
            g_eff_current_tex_rec = D_8009B9E0;
        } else {
            g_eff_current_tex_rec = D_8009B990;
        }
        prim->r0 = 0x10;
        prim->g0 = ~g_eff_animated_quad_counters[mode_index];
        prim->b0 = 0xFF;
    size_sel:
        if (g_working_record.view_setting && g_main_loop_handler_index != 5) {
            *g_eff_base_width = 0x80;
            *g_eff_base_height = 0x80;
            *g_eff_tpage_word = 0x2F;
        } else {
            *g_eff_base_width = 0x20;
            *g_eff_base_height = 0x80;
            *texture_width = 0x40;
            *texture_height = 0x10;
        }
        break;
    case 8:
    case 9:
        if (g_eff_animated_quad_counters[mode_index - 2] < 0xC8) {
            g_eff_current_tex_rec = D_8009B980;
        } else {
            g_eff_current_tex_rec = D_8009B978;
        }
        prim->r0 = g_eff_animated_quad_counters[mode_index - 2] * 2 / 3;
        prim->g0 = g_eff_animated_quad_counters[mode_index - 2] / 2;
        prim->b0 = 0xFF;
        *g_eff_base_width = 0x100;
        *g_eff_base_height = 0x100;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        prim->r0 = 0xC0;
        prim->g0 = 0x70;
        prim->b0 = 0x13;
        *g_eff_base_width *= 2;
        *g_eff_base_height *= 2;
        if (g_eff_animated_quad_counters[mode_index] >= 8) {
            g_eff_current_tex_rec = D_8009B8C8;
            brightness = 10 - g_eff_animated_quad_counters[mode_index];
            prim->r0 = brightness << 6;
            prim->g0 = brightness * 0x70 / 3;
            prim->b0 = brightness * 0x60 / 15;
            *g_eff_tpage_word = 0x2E;
        } else {
            if (g_eff_animated_quad_counters[mode_index] >= 5) {
                g_eff_current_tex_rec = D_8009B9F0;
            } else {
                g_eff_current_tex_rec = D_8009B9E8;
            }
            *g_eff_tpage_word = 0x2F;
        }
        break;
    case 16:
    case 17:
        g_eff_current_tex_rec = D_8009B8C8;
        prim->r0 = 0xFF;
        prim->g0 = 0x60;
        prim->b0 = 0xFF;
        break;
    }
    *g_eff_clut_word = (u16)(((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                             (g_eff_current_tex_rec->clut_y << 6));
    *g_eff_tex_left_u = g_eff_current_tex_rec->u;
    *g_eff_tex_top_v = g_eff_current_tex_rec->v;
    *g_eff_tex_right_u = *texture_width + g_eff_current_tex_rec->u - 1;
    *g_eff_tex_bottom_v = *texture_height + g_eff_current_tex_rec->v - 1;
    *g_eff_geom_screen_work = ReadGeomScreen() * 1000;
    *g_eff_perspective_scale = *g_eff_geom_screen_work / *g_eff_depth_work;
    *p_w = *g_eff_base_width * *g_eff_perspective_scale > 0x200
               ? *g_eff_base_width * *g_eff_perspective_scale >> 8
               : 6;
    *p_h = *g_eff_base_height * *g_eff_perspective_scale > 0x200
               ? *g_eff_base_height * *g_eff_perspective_scale >> 8
               : 6;
    SetPolyFT4(prim);
    prim->tpage = *g_eff_tpage_word;
    prim->clut = *g_eff_clut_word;
    prim->u0 = *g_eff_tex_left_u;
    prim->v0 = *g_eff_tex_top_v;
    prim->u1 = *g_eff_tex_right_u;
    prim->v1 = *g_eff_tex_top_v;
    prim->u2 = *g_eff_tex_left_u;
    prim->v2 = *g_eff_tex_bottom_v;
    prim->u3 = *g_eff_tex_right_u;
    prim->v3 = *g_eff_tex_bottom_v;
    SetShadeTex(prim, 0);
    SetSemiTrans(prim, 1);
    switch (mode_index) {
    case 0:
        *p_w = (u16)*p_w * 8 +
               (*p_w * rcos(g_eff_animated_quad_counters[mode_index] & 0xFFF) >>
                9);
        *p_h = (u16)*p_h * 2 +
               (*p_h * rcos((g_eff_animated_quad_counters[mode_index] + 0x7FF) &
                            0xFFF) >>
                11);
        prim->x0 = *g_eff_screen_xy - *p_w / 2;
        prim->y0 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        prim->x1 = *g_eff_screen_xy + *p_w / 2;
        prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        prim->x2 = *g_eff_screen_xy - *p_w / 2;
        prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 2;
        prim->x3 = *g_eff_screen_xy + *p_w / 2;
        prim->y3 = (*g_eff_screen_xy >> 16) + *p_h / 2;
        break;
    case 1:
    case 2:
    case 16:
    case 17:
        width_scale =
            (rsin((g_eff_animated_quad_counters[mode_index] << 11) / 4551 -
                  0x400) +
             0x1000) *
            25;
        *p_w = (*p_w * width_scale >> 13) / 2;
        height_scale =
            rsin((g_eff_animated_quad_counters[mode_index] << 11) / 4551) * 15;
        *p_h = (*p_h * height_scale >> 12) / 2;
        vertex_index = 0;
        p_v->vy = 0;
        p_v->vx = 0;
        p_v->vz = g_eff_animated_quad_counters[mode_index];
        RotMatrix(p_v, rotation_matrix);
        dst = p_t;
        SetRotMatrix(rotation_matrix);
        do {
            w = *p_w;
            if (!(vertex_index & 1)) {
                p_v->vx = -w;
            } else {
                p_v->vx = w;
            }
            h = *p_h;
            if (!(vertex_index & 2)) {
                p_v->vy = -h;
            } else {
                p_v->vy = h;
            }
            ApplyRotMatrix(p_v, dst);
            dst++;
        } while (++vertex_index < 4);
        prim->x0 = *g_eff_screen_xy + p_t[0].vx / 2;
        prim->y0 = (*g_eff_screen_xy >> 16) + p_t[0].vy / 4;
        prim->x1 = *g_eff_screen_xy + p_t[1].vx / 2;
        prim->y1 = (*g_eff_screen_xy >> 16) + p_t[1].vy / 4;
        prim->x2 = *g_eff_screen_xy + p_t[2].vx / 2;
        prim->y2 = (*g_eff_screen_xy >> 16) + p_t[2].vy / 4;
        prim->x3 = *g_eff_screen_xy + p_t[3].vx / 2;
        prim->y3 = (*g_eff_screen_xy >> 16) + p_t[3].vy / 4;
        break;
    case 12:
    case 13:
    case 14:
    case 15:
        *p_w =
            *p_w *
                rsin((g_eff_animated_quad_counters[mode_index] * 0x3FF / 10) &
                     0xFFF) >>
            11;
        *p_h =
            *p_h *
                rsin((g_eff_animated_quad_counters[mode_index] * 0x3FF / 10) &
                     0xFFF) >>
            11;
        goto quad;
    case 8:
    case 9:
        *p_w = *p_w * rsin((g_eff_animated_quad_counters[mode_index - 2] / 2) &
                           0xFFF) >>
               11;
        *p_h = *p_h * rsin((g_eff_animated_quad_counters[mode_index - 2] / 2) &
                           0xFFF) >>
               11;
        goto quad;
    case 3:
    case 4:
        *p_w =
            *p_w * rsin(g_eff_animated_quad_counters[mode_index] & 0xFFF) >> 11;
        *p_h =
            *p_h * rsin(g_eff_animated_quad_counters[mode_index] & 0xFFF) >> 11;
    quad:
        prim->x0 = *g_eff_screen_xy - *p_w;
        prim->y0 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        prim->x1 = *p_w + *g_eff_screen_xy;
        prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        prim->x2 = *g_eff_screen_xy - *p_w;
        prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 2;
        prim->x3 = *p_w + *g_eff_screen_xy;
        prim->y3 = (*g_eff_screen_xy >> 16) + *p_h / 2;
        break;
    case 5:
        if (g_eff_animated_quad_counters[mode_index] < 0x1000) {
            *p_w = (u16)*p_w * 8 +
                   (*p_w * rcos(g_eff_animated_quad_counters[mode_index] &
                                0xFFF) >>
                    9);
            *p_h =
                (u16)*p_h * 2 +
                (*p_h *
                     rcos((g_eff_animated_quad_counters[mode_index] + 0x7FF) &
                          0xFFF) >>
                 11);
        } else {
            *p_h = 0;
            *p_w = 0;
        }
        prim->x0 = *g_eff_screen_xy - *p_w / 4;
        prim->y0 = (*g_eff_screen_xy >> 16) - *p_h / 4;
        prim->x1 = *g_eff_screen_xy + *p_w / 4;
        prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 4;
        prim->x2 = *g_eff_screen_xy - *p_w / 4;
        prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 4;
        prim->x3 = *g_eff_screen_xy + *p_w / 4;
        prim->y3 = (*g_eff_screen_xy >> 16) + *p_h / 4;
        break;
    case 6:
    case 10:
        if (g_working_record.view_setting && g_main_loop_handler_index != 5) {
            *p_w = -*p_w;
        }
    case 7:
    case 11:
        if (g_working_record.view_setting && g_main_loop_handler_index != 5) {
            *p_w =
                *p_w * rsin(g_eff_animated_quad_counters[mode_index] & 0xFFF) >>
                11;
            *p_h =
                *p_h * rsin(g_eff_animated_quad_counters[mode_index] & 0xFFF) >>
                11;
        } else {
            *p_w =
                *p_w + (*p_w * rsin(g_eff_animated_quad_counters[mode_index] *
                                    0x300 / 255) >>
                        11);
            *p_h = *p_h * rcos((g_eff_animated_quad_counters[mode_index] << 9) /
                               255) >>
                   12;
        }
        if (g_working_record.view_setting && g_main_loop_handler_index != 5) {
            prim->x0 = *g_eff_screen_xy - *p_w;
            prim->y0 = (*g_eff_screen_xy >> 16) - *p_h / 2;
            prim->x1 = *p_w + *g_eff_screen_xy;
            prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 2;
            prim->x2 = *g_eff_screen_xy - *p_w;
            prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 2;
            prim->x3 = *p_w + *g_eff_screen_xy;
            prim->y3 = (*g_eff_screen_xy >> 16) + *p_h / 2;
        } else if (g_eff_animated_quad_counters[mode_index] < 0x96) {
            prim->x0 = *g_eff_screen_xy - *p_w / 2;
            prim->y0 = (*g_eff_screen_xy >> 16) + *p_h / 2;
            prim->x1 = *g_eff_screen_xy - *p_w / 2;
            prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 2;
            prim->x2 = *g_eff_screen_xy + *p_w / 2;
            prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 2;
            prim->x3 = *g_eff_screen_xy + *p_w / 2;
            prim->y3 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        } else {
            prim->x0 = *g_eff_screen_xy;
            prim->y0 = (*g_eff_screen_xy >> 16) + *p_h / 2;
            prim->x1 = *g_eff_screen_xy;
            prim->y1 = (*g_eff_screen_xy >> 16) - *p_h / 2;
            prim->x2 = *p_w + *g_eff_screen_xy;
            prim->y2 = (*g_eff_screen_xy >> 16) + *p_h / 2;
            prim->x3 = *p_w + *g_eff_screen_xy;
            prim->y3 = (*g_eff_screen_xy >> 16) - *p_h / 2;
        }
        if (mode_index < 9) {
            AddPrim(g_gpu_ot_ptr + 1, prim);
            if (prim - g_eff_quad_buffer_base < 0x1C1) {
                prim++;
            }
            mode_index += 2;
            goto again;
        }
        break;
    }
    AddPrim(g_gpu_ot_ptr + 1, prim);
    if (prim - g_eff_quad_buffer_base < 0x1C1) {
        prim++;
    }
    g_eff_quad_cursor = prim;
    return 1;
}

u8 eff_SpawnParticles(s32, s32, s32);

u8 func_80066EC0(void) {
    u8 spawn_result = eff_SpawnParticles(0, 0, 0);
    g_eff_command_phase[2] = 1;
    return spawn_result;
}

u8 func_80066EF4(void) {
    u8 spawn_result = eff_SpawnParticles(0, 0, 1);
    g_eff_command_phase[2] = 2;
    return spawn_result;
}

u8 func_80066F28(void) {
    u8 spawn_result = eff_SpawnParticles(1, 1, 0);
    g_eff_command_phase[3] = 1;
    return spawn_result;
}

u8 func_80066F5C(void) {
    u8 spawn_result = eff_SpawnParticles(1, 1, 1);
    g_eff_command_phase[3] = 2;
    return spawn_result;
}

u8 func_80066F90(void) {
    u8 spawn_result = eff_SpawnParticles(2, 1, 0);
    g_eff_command_phase[20] = 1;
    return spawn_result;
}

u8 func_80066FC4(void) {
    u8 spawn_result = eff_SpawnParticles(2, 1, 1);
    g_eff_command_phase[20] = 2;
    return spawn_result;
}

u8 func_80066FF8(void) {
    u8 spawn_result = eff_SpawnParticles(3, 0, 0);
    g_eff_command_phase[21] = 1;
    return spawn_result;
}

u8 func_8006702C(void) {
    u8 spawn_result = eff_SpawnParticles(3, 0, 1);
    g_eff_command_phase[21] = 2;
    return spawn_result;
}

u8 func_80067060(void) {
    u8 spawn_result = eff_SpawnParticles(4, 2, 0);
    g_eff_command_phase[22] = 1;
    return spawn_result;
}

u8 func_80067094(void) {
    u8 spawn_result = eff_SpawnParticles(4, 2, 1);
    g_eff_command_phase[22] = 2;
    return spawn_result;
}

u8 func_800670C8(void) {
    u8 spawn_result = eff_SpawnParticles(5, 3, 0);
    g_eff_command_phase[23] = 1;
    return spawn_result;
}

u8 func_800670FC(void) {
    u8 spawn_result = eff_SpawnParticles(5, 3, 1);
    g_eff_command_phase[23] = 2;
    return spawn_result;
}

u8 func_80067130(void) {
    u8 spawn_result = eff_SpawnParticles(6, 3, 0);
    g_eff_command_phase[24] = 1;
    return spawn_result;
}

u8 func_80067164(void) {
    u8 spawn_result = eff_SpawnParticles(6, 3, 1);
    g_eff_command_phase[24] = 2;
    return spawn_result;
}

u8 func_80067198(void) {
    u8 spawn_result = eff_SpawnParticles(7, 2, 0);
    g_eff_command_phase[25] = 1;
    return spawn_result;
}

u8 func_800671CC(void) {
    u8 spawn_result = eff_SpawnParticles(7, 2, 1);
    g_eff_command_phase[25] = 2;
    return spawn_result;
}

extern SVECTOR g_eff_particle_emitter_vectors[];
extern Unk800EFC78Record g_eff_particles[][48];

/* eff_SpawnParticles -- spawns half (half_index selects which) of the 0x30
 * g_eff_particles[particle_row_index] particle records: each gets the
 * g_eff_particle_emitter_vectors[particle_row_index] vector scaled per axis by
 * its own random factor and turned by a random rotation (spread chosen by
 * particle_mode). half_index == 0 first latches g_eff_command_position /
 * g_eff_command_vector into the emitter. Returns 1. */
u8 eff_SpawnParticles(
    s32 particle_mode, s32 particle_row_index, s32 half_index) {
    SVECTOR v;
    s32 random_values[6];
    MATRIX m;
    SVECTOR rotation_angles;
    VECTOR out;
    SVECTOR var_5;
    s16 base;
    s16 scale_mask;
    s16 angle_mask;
    s16 angle_offset;
    u8 particle_count;
    s32 particle_index;
    Unk800EFC78Record *p;

    if (particle_mode < 4) {
        base = 0x41;
        scale_mask = 0x3F;
    } else {
        base = 0x31;
        scale_mask = 0x1F;
    }
    D_800A3438[particle_row_index] = 0;
    gte_SetRotMatrix(g_eff_view_rotation_matrix);
    if (half_index == 0) {
        g_eff_particle_group_positions[particle_row_index][0].unk0 =
            g_eff_command_position[0];
        g_eff_particle_group_positions[particle_row_index][0].unk4 =
            g_eff_command_position[1];
        g_eff_particle_group_positions[particle_row_index][0].unk8 =
            g_eff_command_position[2];
        g_eff_particle_emitter_vectors[particle_row_index].vx =
            g_eff_command_vector[0];
        g_eff_particle_emitter_vectors[particle_row_index].vy =
            g_eff_command_vector[1];
        g_eff_particle_emitter_vectors[particle_row_index].vz =
            g_eff_command_vector[2];
    }
    particle_count = 0x30;
    random_values[5] = rand();
    if (particle_mode < 2) {
        angle_mask = 0xFF;
        angle_offset = 0x7F;
    } else if (particle_mode < 4) {
        angle_mask = 0xFFF;
        angle_offset = 0;
    } else if (particle_mode < 6) {
        angle_mask = 0x7F;
        angle_offset = 0x3F;
    } else if (particle_mode < 8) {
        angle_mask = 0x7F;
        angle_offset = 0x3F;
    }
    var_5.vx = g_eff_particle_emitter_vectors[particle_row_index].vx;
    var_5.vy = g_eff_particle_emitter_vectors[particle_row_index].vy;
    var_5.vz = g_eff_particle_emitter_vectors[particle_row_index].vz;
    for (particle_index = (particle_count >> 1) * half_index;
         particle_index < (particle_count >> 1) * (half_index + 1);
         particle_index++) {
        random_values[0] = random_values[5] ^ rand();
        random_values[1] = random_values[0] ^ rand();
        random_values[2] = random_values[1] ^ rand();
        random_values[3] = random_values[2] ^ rand();
        random_values[4] = random_values[3] ^ rand();
        random_values[5] = random_values[4] ^ rand();
        rotation_angles.vy = random_values[0] =
            (random_values[0] & angle_mask) - angle_offset;
        rotation_angles.vx = random_values[1] =
            (random_values[1] & angle_mask) - angle_offset;
        rotation_angles.vz = random_values[2] =
            (random_values[2] & angle_mask) - angle_offset;
        random_values[3] &= scale_mask;
        random_values[4] &= scale_mask;
        random_values[5] &= scale_mask;
        v.vx = var_5.vx * (base + random_values[3]) / scale_mask;
        v.vy = var_5.vy * (base + random_values[4]) / scale_mask;
        v.vz = var_5.vz * (base + random_values[5]) / scale_mask;
        p = &g_eff_particles[particle_row_index][particle_index];
        p->unk6 = particle_index / 16;
        p->unk12 = 0;
        p->unk4 = 0;
        p->unk2 = 0;
        p->unk0 = 0;
        p->unk10 = 1;
        RotMatrix(&rotation_angles, &m);
        gte_ApplyMatrix(&m, &v, &out);
        p->unk8 = out.vx;
        p->unkA = out.vy;
        p->unkC = out.vz;
    }
    return 1;
}

u8 eff_PrepareParticles(s32, s32);
void eff_UpdateParticles(s32, s32);
u8 eff_LinkParticleQuads(s32, s32);

u8 func_800676C8(void) {
    eff_PrepareParticles(0, 0);
    eff_UpdateParticles(0, 0);
    return eff_LinkParticleQuads(0, 0);
}

u8 func_80067704(void) {
    eff_PrepareParticles(1, 1);
    eff_UpdateParticles(1, 1);
    return eff_LinkParticleQuads(1, 1);
}

u8 func_80067740(void) {
    eff_PrepareParticles(2, 1);
    eff_UpdateParticles(2, 1);
    return eff_LinkParticleQuads(2, 1);
}

u8 func_8006777C(void) {
    eff_PrepareParticles(3, 0);
    eff_UpdateParticles(3, 0);
    return eff_LinkParticleQuads(3, 0);
}

u8 func_800677B8(void) {
    eff_PrepareParticles(4, 2);
    eff_UpdateParticles(4, 2);
    return eff_LinkParticleQuads(4, 2);
}

u8 func_800677F4(void) {
    eff_PrepareParticles(5, 3);
    eff_UpdateParticles(5, 3);
    return eff_LinkParticleQuads(5, 3);
}

u8 func_80067830(void) {
    eff_PrepareParticles(6, 3);
    eff_UpdateParticles(6, 3);
    return eff_LinkParticleQuads(6, 3);
}

u8 func_8006786C(void) {
    eff_PrepareParticles(7, 2);
    eff_UpdateParticles(7, 2);
    return eff_LinkParticleQuads(7, 2);
}

extern s16 D_800EFC8A[];
extern s16 D_800F0B98[];

u8 eff_PrepareParticles(s32 particle_mode, s32 particle_row_index) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &g_eff_work_area->v800678A8;
    /* FAKE: unk00 / unk02 / unk04 / unk6C / unk80 are reached through the
     * plain pointers p0 / p2 / p4 / p6C / p80: member stores are
     * MEM_IN_STRUCT_P and let scalar accesses move across them (as members, p0:
     * scores 6, p4 2, p80 2, p2 24, p6C 38; all five 49) (Q129). */
    u16 *p0 = &var_0->tex_right_u_offset;
    u16 *p2 = &var_0->tex_bottom_v_offset;
    u32 *p4 = &var_0->squared_distance_limit;
    s16 *p6C = &var_0->particle_scan_count;
    POLY_FT4 **p80 = &var_0->particle_quad_cursor;

    D_800A3724 = &var_0->random_accumulator;
    *p80 = g_eff_quad_cursor;
    *p4 = 0x895440;
    *g_eff_tpage_word = 0x2E;

    if (particle_mode < 2) {
        g_eff_current_tex_rec = D_8009B890;
        *p2 = 7;
        *p0 = 7;
        *g_eff_base_width = 0x40;
        *g_eff_base_height = 0x20;
        D_800F0B98[particle_mode] = 3;
    } else if (particle_mode < 4) {
        g_eff_current_tex_rec = D_8009B890;
        *p2 = 7;
        *p0 = 7;
        *g_eff_base_width = 0x20;
        *g_eff_base_height = 0x10;
        D_800F0B98[particle_mode] = 3;
    } else if (particle_mode < 6) {
        s16 tex_rec_index = D_800EFC8A[particle_row_index * 0x1E0] >> 3;
        var_0->unk70 = tex_rec_index;
        if (tex_rec_index >= 4) {
            var_0->unk70 = 3;
        }
        if (D_800A34F0[particle_mode - 4] != 0) {
            g_eff_current_tex_rec = &D_8009B9B8[var_0->unk70];
        } else {
            g_eff_current_tex_rec = &D_8009B998[var_0->unk70];
        }
        *p0 = 0x1F;
        *p2 = 0x20;
        *g_eff_base_width = 0xC0;
        *g_eff_base_height = 0x30;
        *g_eff_tpage_word = 0xF;
        D_800F0B98[particle_mode] = 1;
    } else if (particle_mode < 8) {
        *g_eff_tpage_word = 0x2E;
        g_eff_current_tex_rec = D_8009B8B0;
        *p2 = 0xF;
        *p0 = 0xF;
        *g_eff_base_width = 0xC0;
        *g_eff_base_height = 0x60;
        D_800F0B98[particle_mode] = 2;
    }

    *g_eff_clut_word = (((g_eff_current_tex_rec->clut_x >> 4) & 0x3F) +
                        (g_eff_current_tex_rec->clut_y << 6))
                       << 16;
    *g_eff_tpage_word <<= 16;
    *g_eff_tex_left_u = g_eff_current_tex_rec->u;
    *g_eff_tex_top_v = g_eff_current_tex_rec->v;
    *g_eff_tex_right_u = *p0 + g_eff_current_tex_rec->u;
    *g_eff_tex_bottom_v = *p2 + g_eff_current_tex_rec->v;
    *g_eff_top_left_uv = *g_eff_tex_left_u + (*g_eff_tex_top_v << 8);
    *g_eff_top_right_uv = *g_eff_tex_right_u + (*g_eff_tex_top_v << 8);
    *g_eff_bottom_left_uv = *g_eff_tex_left_u + (*g_eff_tex_bottom_v << 8);
    *g_eff_bottom_right_uv = *g_eff_tex_right_u + (*g_eff_tex_bottom_v << 8);

    gte_SetRotMatrix(g_eff_view_rotation_matrix);

    if (D_800A3438[particle_row_index] < D_800F0B98[particle_mode]) {
        g_eff_particle_group_positions[particle_row_index]
                                      [D_800A3438[particle_row_index]]
                                          .unk0 =
            g_eff_particle_group_positions[particle_row_index][0].unk0;
        g_eff_particle_group_positions[particle_row_index]
                                      [D_800A3438[particle_row_index]]
                                          .unk4 =
            g_eff_particle_group_positions[particle_row_index][0].unk4;
        g_eff_particle_group_positions[particle_row_index]
                                      [D_800A3438[particle_row_index]]
                                          .unk8 =
            g_eff_particle_group_positions[particle_row_index][0].unk8;
    }

    gte_ReadGeomScreen(g_eff_geom_screen_work);

    *p6C = (D_800A3438[particle_row_index] + 1) * 16;
    if (D_800A3438[particle_row_index] < D_800F0B98[particle_mode] - 1) {
        D_800A3438[particle_row_index]++;
    }
}

/* Per-frame update + draw of the g_eff_particles[particle_row_index] records,
 * the middle of the eff_PrepareParticles / eff_UpdateParticles /
 * eff_LinkParticleQuads trio. For each live record of
 * g_eff_particles[particle_row_index] (state 1): step its unk12 and derive
 * r/g/b from it, take its g_eff_particle_group_positions[particle_row_index]
 * entry less g_eff_view_origin through the GTE rotation, set state 3 (and skip
 * it) once r and g are both low or once the squared length p_out[0..2] passes
 * *var_1 plus a rand() term, project three vertices, size the sprite by type
 * (particle_mode), then emit up to three POLY_FT4 quads, storing
 * *g_eff_depth_work for each in var_9[]. */
void eff_UpdateParticles(s32 particle_mode, s32 particle_row_index) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *var_0 = &g_eff_work_area->v800678A8;
    /* FAKE: scratchpad members are reached through the plain pointers below.
     * Replacing p_count alone spills the unk70 address from s8 to 96(sp),
     * adding stack reloads before its halfword loads (1052 vs 1047 insns).
     * Single-pointer direct-member scores (pointer / member): var_1 / unk04
     * 101, var_2 / unk5C 112, p_count / unk6C 94, p_idx / unk6E 109,
     * p_r / unk72 119, p_g / unk73 102, p_b / unk74 104, var_8 / unk88 99,
     * var_10 / unk1AC 91, p_work / unk34 75, var_14 / unk70 95,
     * p_n / unk78 74, p_prim / unk80 43, p_ent / unk84 65;
     * all fourteen as members: 230 (984 vs 1047 insns) (Q129). */
    u32 *var_1;
    VECTOR *var_2;
    s16 *p_count;
    s16 *p_idx;
    u8 *p_r;
    u8 *p_g;
    u8 *p_b;
    Unk800F0C10Record **var_8;
    u16 *var_9;
    s32 *var_10;
    s32 *p_out;
    VECTOR *p_work;
    SVECTOR *var_13;
    s16 *var_14;
    s16 *p_n;
    POLY_FT4 **p_prim;
    Unk800EFC78Record **p_ent;
    u32 squared_length;
    s16 *sxy;

    D_800A3724 = &var_0->random_accumulator;
    var_10 = &var_0->random_accumulator;
    *var_10 = rand();
    var_1 = &var_0->squared_distance_limit;
    p_out = var_0->unk10.t;
    p_work = &var_0->unk34;
    var_13 = var_0->unk44;
    var_2 = &var_0->group_position;
    p_count = &var_0->particle_scan_count;
    p_idx = &var_0->unk6E;
    var_14 = &var_0->unk70;
    p_r = &var_0->particle_red;
    p_g = &var_0->particle_green;
    p_b = &var_0->particle_blue;
    p_n = &var_0->screen_xy_index;
    p_prim = &var_0->particle_quad_cursor;
    p_ent = &var_0->current_particle;
    var_8 = &var_0->group_position_entry;
    var_9 = var_0->quad_ot_index_table;

    for (*p_idx = 0; *p_idx < *p_count; (*p_idx)++) {
        *p_ent = &g_eff_particles[particle_row_index][*p_idx];
        if ((*p_ent)->unk10 != 1) {
            continue;
        }
        (*p_ent)->unk12++;
        *var_14 = (*p_ent)->unk12;
        *p_r = (*var_14 << 2) < 0xFF ? ~(*var_14 << 2) : 0;
        *p_g = (*var_14 << 5) < 200 ? 200 - (*var_14 << 5) : 0;
        if (*p_r <= 0x50 && *p_g <= 0x20) {
            (*p_ent)->unk10 = 3;
            continue;
        }
        *p_b = (*var_14 << 5) < 100 ? 100 - (*var_14 << 5) : 0;

        *var_8 =
            &g_eff_particle_group_positions[particle_row_index][(*p_ent)->unk6];
        var_2->vx = (*var_8)->unk0;
        var_13[0].vx = var_2->vx - g_eff_view_origin[0];
        var_2->vy = (*var_8)->unk4;
        var_13[0].vy = var_2->vy - g_eff_view_origin[1];
        var_2->vz = (*var_8)->unk8;
        var_13[0].vz = var_2->vz - g_eff_view_origin[2];

        gte_ApplyRotMatrix(var_13, p_out);
        /* FAKE: SetTransMatrix reads only m->t (+0x14): handed p_out - 0x14,
         * i.e. &var_0->unk10 reached through p_out, it loads p_out as the
         * translation. Spelled &var_0->unk10: score 82; through a MATRIX *
         * local m = &var_0->unk10 (p_out = m->t): score 58. */
        gte_SetTransMatrix((u8 *)p_out - 0x14);

        var_13[1].vx = (*p_ent)->unk0;
        var_13[1].vz = (*p_ent)->unk4;
        var_13[1].vy = (*p_ent)->unk2;
        p_work->vx = (*p_ent)->unk8 / 30;
        (*p_ent)->unk0 += p_work->vx;
        var_13[0].vx = (*p_ent)->unk0;
        p_work->vz = (*p_ent)->unkC / 30;
        (*p_ent)->unk4 += p_work->vz;
        var_13[0].vz = (*p_ent)->unk4;
        p_work->vy = (*p_ent)->unkA / 30 + *var_14 * 9800 / 900;
        (*p_ent)->unk2 += p_work->vy;
        var_13[0].vy = (*p_ent)->unk2;
        var_13[2].vx = var_13[1].vx + (p_work->vx >> 1);
        var_13[2].vz = var_13[1].vz + (p_work->vz >> 1);
        var_13[2].vy = var_13[1].vy + (p_work->vy >> 1);

        gte_ldv3(&var_13[0], &var_13[1], &var_13[2]);
        p_work->vx = var_13[0].vx;
        p_work->vz = var_13[0].vz;
        p_work->vy = var_13[0].vy;
        gte_Square0(p_work, p_out);

        *var_10 ^= rand();
        if ((u32)(p_out[0] + p_out[1] + p_out[2]) >
            *var_1 + *var_10 * 3000 / 32768 * 3000) {
            (*p_ent)->unk10 = 3;
            continue;
        }

        gte_rtpt();
        gte_stsxy3(
            &g_eff_screen_xy[0], &g_eff_screen_xy[1], &g_eff_screen_xy[2]);
        gte_stsz(g_eff_depth_work);

        if (*g_eff_depth_work < 0) {
            continue;
        }
        g_eff_depth_work[1] = *g_eff_depth_work ? *g_eff_depth_work : 1;
        *g_eff_depth_work = gpu_DepthToOtIndex(g_eff_depth_work[1], 0);
        if (*g_eff_depth_work == 0) {
            *g_eff_depth_work = 1;
        }
        *g_eff_rtp_flag = 1;
        if (*g_eff_depth_work >= 0x1005 || *g_eff_rtp_flag == 0) {
            continue;
        }

        if (particle_mode < 2) {
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work * 200 / g_eff_depth_work[1];
            *g_eff_right_x_offset =
                *g_eff_base_width * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_width * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_lower_y_offset_word =
                *g_eff_base_height * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_height * *g_eff_perspective_scale >> 9
                    : 1;
        } else if (particle_mode < 4) {
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work * 200 / g_eff_depth_work[1];
            *g_eff_right_x_offset =
                *g_eff_base_width * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_width * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_lower_y_offset_word =
                *g_eff_base_height * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_height * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_right_x_offset +=
                *g_eff_right_x_offset * 20 / (*var_14 * 6 + 1);
            *g_eff_lower_y_offset_word +=
                *g_eff_lower_y_offset_word * 20 / (*var_14 * 6 + 1);
        } else if (particle_mode < 6) {
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work * 200 / g_eff_depth_work[1];
            *g_eff_right_x_offset =
                *g_eff_base_width * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_width * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_lower_y_offset_word =
                *g_eff_base_height * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_height * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_right_x_offset += *g_eff_right_x_offset * *var_14 / 2;
            *g_eff_lower_y_offset_word +=
                *g_eff_lower_y_offset_word * *var_14 / 2;
        } else if (particle_mode < 8) {
            *g_eff_perspective_scale =
                *g_eff_geom_screen_work * 200 / g_eff_depth_work[1];
            *g_eff_right_x_offset =
                *g_eff_base_width * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_width * *g_eff_perspective_scale >> 9
                    : 1;
            *g_eff_lower_y_offset_word =
                *g_eff_base_height * *g_eff_perspective_scale > 0x200
                    ? *g_eff_base_height * *g_eff_perspective_scale >> 9
                    : 1;
            sxy = (s16 *)g_eff_screen_xy;
            sxy[2] += rand() * 20 / 32768;
            sxy[3] += rand() * 10 / 32768;
        }
        if (*g_eff_right_x_offset > *g_eff_base_width * 2) {
            *g_eff_right_x_offset = *g_eff_base_width >> 1;
            *g_eff_lower_y_offset_word = *g_eff_base_height >> 1;
        }
        *D_800A34BC = -*g_eff_right_x_offset;
        *g_eff_upper_y_offset = -*g_eff_lower_y_offset_word;
        *g_eff_lower_y_offset_word <<= 16;
        *g_eff_upper_y_offset <<= 16;

        squared_length = p_out[0] + p_out[1] + p_out[2];
        if (*var_1 / 3 < squared_length) {
            *p_n = 1;
        } else if (*var_1 / 2 < squared_length) {
            *p_n = 0;
        } else {
            *p_n = 2;
        }
        /* Packed xy / uv words written through word views of the POLY_FT4, as
           the other emitters in this file do (whole-word coordinate and colour
           writes, halfword uv writes).
           SOTN: src/dra/8BEF8.c:185 @aa53500
           SOTN: src/st/cat/e_bone_ark.c:436 @aa53500 */
        for (; *p_n >= 0; (*p_n)--) {
            var_9[*p_prim - g_eff_quad_cursor] = *g_eff_depth_work;
            *(s32 *)&(*p_prim)->x0 =
                g_eff_screen_xy[*p_n] + *D_800A34BC + *g_eff_upper_y_offset;
            *(s32 *)&(*p_prim)->x1 =
                g_eff_screen_xy[*p_n] + *g_eff_right_x_offset +
                *g_eff_upper_y_offset;
            *(s32 *)&(*p_prim)->x2 = g_eff_screen_xy[*p_n] + *D_800A34BC +
                                     *g_eff_lower_y_offset_word;
            *(s32 *)&(*p_prim)->x3 =
                g_eff_screen_xy[*p_n] + *g_eff_right_x_offset +
                *g_eff_lower_y_offset_word;
            *(s32 *)&(*p_prim)->u0 = *g_eff_top_left_uv + *g_eff_clut_word;
            *(s32 *)&(*p_prim)->u1 = *g_eff_top_right_uv + *g_eff_tpage_word;
            *(u16 *)&(*p_prim)->u2 = *g_eff_bottom_left_uv;
            *(u16 *)&(*p_prim)->u3 = *g_eff_bottom_right_uv;
            setlen(*p_prim, 9);
            if (particle_mode < 2) {
                *(s32 *)&(*p_prim)->r0 = *p_r + (*p_g << 8) + 0x2E000000;
            } else if (particle_mode < 4) {
                *(s32 *)&(*p_prim)->r0 =
                    *p_r + ((*p_r >> 2) << 8) + (*p_b << 16) + 0x2E000000;
            } else if (particle_mode < 6) {
                if (*p_prim - g_eff_quad_buffer_base >= 0x1C1) {
                    continue;
                }
                if (g_working_record.grayscale) {
                    *(s32 *)&(*p_prim)++->r0 = 0x2C242424;
                    break;
                } else if (D_800A34F0[particle_mode - 4] != 0) {
                    *(s32 *)&(*p_prim)++->r0 =
                        (((rand() * 4 >> 12) + 0x3C) << 8) + 0x2C080038;
                    break;
                } else {
                    *(s32 *)&(*p_prim)++->r0 = 0x2C285A78;
                    break;
                }
            } else if (particle_mode < 8) {
                if (*p_prim - g_eff_quad_buffer_base >= 0x1C1) {
                    break;
                }
                *(s32 *)&(*p_prim)++->r0 = 0x2EFF8080;
                break;
            }
            if (*p_prim - g_eff_quad_buffer_base < 0x1C1) {
                (*p_prim)++;
            }
        }
    }
}

u8 eff_LinkParticleQuads(s32 arg0, s32 arg1) {
    extern s32 *D_800A3724;
    Unk1F8000B8_800678A8 *work_area = &g_eff_work_area->v800678A8;
    s16 *quad_index = &work_area->unk6E;
    POLY_FT4 **quad_end = &work_area->new_quads_end;
    POLY_FT4 **quad_cursor = &work_area->particle_quad_cursor;
    u16 *quad_ot_indices = work_area->quad_ot_index_table;
    POLY_FT4 *entry_quad_cursor;
    POLY_FT4 *first_quad;
    s32 quad_count;
    s32 has_quads;

    D_800A3724 = &work_area->random_accumulator;
    first_quad = g_eff_quad_cursor;
    entry_quad_cursor = *quad_cursor;
    quad_count = entry_quad_cursor - first_quad;

    if (quad_count != 0) {
        *quad_cursor = first_quad;
        *quad_end = entry_quad_cursor;
        has_quads = 1;
        *quad_index = 0;

        if (*quad_cursor < *quad_end) {
            u32 *prim_tag;
            do {
                g_eff_current_ot_entry =
                    g_gpu_ot_ptr + quad_ot_indices[*quad_index];
                prim_tag = &(*quad_cursor)->tag;
                g_eff_current_prim_tag = prim_tag;
                *prim_tag = (*prim_tag & 0xFF000000) |
                            (*g_eff_current_ot_entry & 0xFFFFFF);

                *g_eff_current_ot_entry =
                    ((u32)g_eff_current_prim_tag & 0xFFFFFF) |
                    (*g_eff_current_ot_entry & 0xFF000000);

                (*quad_cursor)++;
                (*quad_index)++;
            } while (*quad_cursor < *quad_end);
        }
        g_eff_quad_cursor = *quad_end;
    } else {
        has_quads = 0;
    }

    return has_quads;
}

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 g_eff_billboard_draw_count = 0;
s32 g_eff_point_light_countdown = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
POLY_FT4 *g_eff_quad_buffer_base;
s32 *D_800A3724;
POLY_FT4 *g_eff_quad_cursor;
