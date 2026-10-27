/* 17 game functions: UI drawing (gpu_DrawModeSizedPanelWithSelectedCellOpaque,
 * gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle,
 * gpu_DrawSelectedStretchedSpritePair,
 * gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite,
 * gpu_DrawIndexedCellsDigitPairsAndGrowingBars, ...). .text 0x8005C8A8 (ROM
 * 0x4D0A8). Start boundary: SEAM (owner ruling 2026-10-17; Codex seam #136): no
 * direct call crosses the cut; the static D_800A3418 and the .sdata
 * D_800A3264..D_800A32B6 (RECTs and draw records) are used only here and
 * follow 4BC3C.c's .sbss and 4510C.c's .sdata in address order. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

/* This file's statics (.sbss, allocated per file in link order), in address
 * order (Q65). */
static s32 D_800A3418;

extern Unk8009B400Record D_8009B194[3];
extern Unk8009B400Record D_8009B1AC[2];
extern Unk8009B400Record D_8009B1BC[2];
extern Unk8009B400Record D_8009B20C[6];
extern Unk8009B400Record D_8009B23C[12];
extern Unk8009B400Record D_8009B29C[2];
extern Unk8009B400Record *D_8009B2AC[4];
/* AddPrim has no prototype here: this TU called it as an implicit-int function.
 * With PsyQ's void AddPrim(void *, void *),
 * gpu_DrawIndexedCellsDigitPairsAndGrowingBars's return value is computed after
 * its last AddPrim's dead sp1C bump and a load-delay nop is added (192 -> 193
 * insns). */
extern int AddPrim();

s32 gpu_DrawModeSizedPanelWithSelectedCellOpaque(
    s32 mode, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005C8A8Rec *var_0 = (Unk8005C8A8Rec *)arg2;
    Unk8007352CRec s;
    TILE *tile;
    s32 cur;
    s16 var_4;
    u16 y_base;
    DR_MODE *mode_off;
    s32 size;
    s16 top;
    s16 i;
    s16 j;
    s16 x;
    s16 y;

    tile = var_0->unk_00;
    cur = arg2 + sizeof(var_0->unk_00);
    y_base = 0;
    /* FAKE: the low half read from arg1's stack home, which keeps arg1 in
       memory for the in-loop `lw 0xBC($sp)` too; (s16)arg1 does not. `var_4 =
       arg1`: score 58. */
    var_4 = *(s16 *)&arg1;
    mode_off = &var_0->unk_4D8;
    /* FAKE: the record's 0x4F0 bytes (sizeof(Unk8005C8A8Rec)) spelled from
     * mode_off as an int: combine folds it with no REG_EQUAL note, so size
     * keeps the target's sp+0x70 slot to the return and the deleted temp's
     * stale count takes the untouched sp+0x78; the literal or sizeof is
     * rematerialised at the return (frame 0x10 short). sizeof: score 33 */
    size = (s32)mode_off + 0x18 - arg2;
    top = (0xF0 - g_panel_frame_sizes[mode].h) / 2;
    s.col_b = 0x40;
    s.col_g = 0x40;
    s.col_r = 0x40;
    s.has_color = 0;

    switch (mode) {
    case 2:
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        y_base = 0x33;
        s.y = top + 0x73;
        s.x = 0;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        D_8009B184[0].x = (0x280 - g_panel_frame_sizes[2].w) / 2;
        D_8009B184[1].x = (g_panel_frame_sizes[2].w + 0x280) / 2 - 0xC;
        cur = gpu_AddSpriteCells(&s);
        s.header = &D_8009B0E0[8];
        s.table = D_8009B20C;
        s.x = 0;
        s.y = 0;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        cur = gpu_AddSpriteCells(&s);
        for (j = 0; j < 3; j++) {
            if (var_4 == j + 3) {
                s.semi = 0;
            } else {
                s.semi = 1;
            }
            s.header = &D_8009B14C;
            s.table = &D_8009B23C[j * s.header->count];
            s.x = 0;
            s.y = 0;
            s.sprt_out = cur;
            s.ot_idx = arg3;
            cur = gpu_AddSpriteCells(&s);
            for (i = 0; i < 2; i++) {
                if (((arg1 >> (j + 16)) & 1) == i) {
                    s.semi = 0;
                } else {
                    s.semi = 1;
                }
                s.header = &D_8009B158;
                s.table = &D_8009B29C[i];
                s.x = 0;
                s.y = j * 15;
                s.sprt_out = cur;
                s.ot_idx = arg3;
                cur = gpu_AddSpriteCells(&s);
            }
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - g_panel_frame_sizes[mode].w) / 2;
            tile->y0 = top + 0x73 + i;
            tile->w = g_panel_frame_sizes[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
        }
        /* fallthrough */
    case 0:
        if (var_4 == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.table = D_8009B1AC;
        s.header = &D_8009B0E0[4];
        s.x = 0;
        s.sprt_out = cur;
        s.y = y_base + top;
        s.ot_idx = arg3;
        cur = gpu_AddSpriteCells(&s);
        s.header = &D_8009B0E0[3];
        s.has_color = 0;
        s.table = D_8009B194;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        cur = gpu_AddSpriteCells(&s);
        top += 0xE;
        s.header = &D_8009B0E0[2];
        s.table = D_8009B184;
        s.x = 0;
        s.y = top;
        s.semi = 0;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        D_8009B184[0].x = (0x280 - g_panel_frame_sizes[mode].w) / 2;
        D_8009B184[1].x = (g_panel_frame_sizes[mode].w + 0x280) / 2 - 0xC;
        cur = gpu_AddSpriteCells(&s);
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = (0x280 - g_panel_frame_sizes[mode].w) / 2;
            tile->y0 = top + i;
            tile->w = g_panel_frame_sizes[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
        }
        break;
    case 1:
        if (var_4 == 0) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[5];
        s.table = D_8009B1BC;
        s.x = 0;
        s.y = top;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        cur = gpu_AddSpriteCells(&s);
        break;
    }

    s.has_color = 0;
    for (j = 2; j < 4; j++) {
        if (var_4 == j - 1) {
            s.semi = 0;
        } else {
            s.semi = 1;
        }
        s.header = &D_8009B0E0[4 + j];
        s.table = D_8009B2AC[j];
        s.x = 0;
        s.y = y_base + top;
        s.sprt_out = cur;
        s.ot_idx = arg3;
        cur = gpu_AddSpriteCells(&s);
    }

    s.has_color = 0;
    s.header = &D_8009B0E0[0];
    s.table = D_8009B164[0];
    s.x = 0;
    s.y = (0xF0 - g_panel_frame_sizes[mode].h) / 2;
    s.semi = 0;
    s.sprt_out = cur;
    s.ot_idx = arg3;
    D_8009B164[0][0].x = (0x280 - g_panel_frame_sizes[mode].w) / 2;
    D_8009B164[0][1].x = (g_panel_frame_sizes[mode].w + 0x280) / 2 - 0xC;
    cur = gpu_AddSpriteCells(&s);
    s.header = &D_8009B0E0[1];
    s.table = D_8009B164[1];
    s.y = (g_panel_frame_sizes[mode].h + 0xF0) / 2;
    s.sprt_out = cur;
    D_8009B164[1][0].x = (0x280 - g_panel_frame_sizes[mode].w) / 2;
    D_8009B164[1][1].x = (g_panel_frame_sizes[mode].w + 0x280) / 2 - 0xC;
    gpu_AddSpriteCells(&s);

    for (j = 0; j < 2; j++) {
        x = (0x280 - g_panel_frame_sizes[mode].w) / 2;
        if (j != 0) {
            y = (0xF0 - g_panel_frame_sizes[mode].h) / 2;
        } else {
            y = (g_panel_frame_sizes[mode].h + 0xF0) / 2;
        }
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (i != 0) {
                tile->r0 = 0x3C;
                tile->g0 = 0x3C;
                tile->b0 = 0x3C;
            } else {
                tile->r0 = 0x52;
                tile->g0 = 0x52;
                tile->b0 = 0x52;
            }
            tile->x0 = x;
            tile->y0 = y + i;
            tile->w = g_panel_frame_sizes[mode].w;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
        }
        if (j != 0) {
            x = (0x280 - g_panel_frame_sizes[mode].w) / 2;
        } else {
            x = (g_panel_frame_sizes[mode].w + 0x280) / 2;
        }
        y = (0xF0 - g_panel_frame_sizes[mode].h) / 2;
        for (i = 0; i < 2; i++) {
            SetTile(tile);
            if (j != 0) {
                if (i != 0) {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                } else {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                }
            } else {
                if (i != 0) {
                    tile->r0 = 0x52;
                    tile->g0 = 0x52;
                    tile->b0 = 0x52;
                } else {
                    tile->r0 = 0x3C;
                    tile->g0 = 0x3C;
                    tile->b0 = 0x3C;
                }
            }
            tile->x0 = x + i * 2;
            tile->y0 = y;
            tile->w = 2;
            tile->h = g_panel_frame_sizes[mode].h + 2;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
        }
    }

    SetTile(tile);
    tile->r0 = 0;
    tile->g0 = 0;
    tile->b0 = 0;
    tile->x0 = (0x280 - g_panel_frame_sizes[mode].w) / 2;
    tile->y0 = (0xF0 - g_panel_frame_sizes[mode].h) / 2;
    tile->w = g_panel_frame_sizes[mode].w;
    tile->h = g_panel_frame_sizes[mode].h;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg3, tile);
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B0E0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return size;
}

/* 0x8009B2C8: two rows of five sprite-sheet headers
 * (gpu_DrawFixedSpritePairWithSelectedSheets draws headers 0 / 1 of
 * row idx with the cells at D_8009B340 / D_8009B358;
 * gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle headers 2 and 3 / 4 with
 * D_8009B388's), then the cells: three under header 0 (count 3), six under
 * header 1 (count 6). */
extern Unk8009B0E0Record D_8009B2C8[2][5];
extern Unk8009B400Record D_8009B340[3];
extern Unk8009B400Record D_8009B358[6];

s32 gpu_DrawFixedSpritePairWithSelectedSheets(s32 arg0, s32 arg1) {
    Unk8007352CRec s;
    s32 idx;
    idx = arg1;
    if (arg1 > 0) {
        idx = arg1 - 1;
    }
    s.has_color = 0;
    s.header = &D_8009B2C8[idx][0];
    s.table = D_8009B340;
    s.scale_y = 0x100;
    s.scale_x = 0x100;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = 1;
    s.ft4_out = arg0;
    arg0 = gpu_AddScaledSpriteQuads(&s, 0);
    s.has_color = 0;
    s.header = &D_8009B2C8[idx][1];
    s.table = D_8009B358;
    s.scale_y = 0x100;
    s.scale_x = 0x100;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = 1;
    s.ft4_out = arg0;
    return gpu_AddScaledSpriteQuads(&s, 0);
}

s32 gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle(s32 arg0, s32 arg1) {
    Unk8007352CRec s;
    s32 i;
    u32 base_x;
    u32 base_y;
    s32 var_4;
    s32 row_off;
    s32 x;
    s32 y;
    s32 tmp1; /* FAKE: carrier, see the y sites (without tmp1: score 4) */
    /* FAKE: carrier, see the y sites (without tmp2: score 4; without both: 15)
     */
    s32 tmp2;
    s32 var_10; /* FAKE: constant 0x100 in a local; the literal: score 15 */
    s32 var_11; /* FAKE: constant 1 in a local; the literal: score 2 */
    /* FAKE: pointer alias of D_8009B2C8[0][2]; direct use: score 41 */
    Unk8009B0E0Record *var_12;
    /* Once-written named intermediate: row header 3, base for random
     * header 3/4 selection; inline at use: score 36. */
    Unk8009B0E0Record *random_header_pair;

    D_800A326C %= 4;

    var_4 = arg0;
    if (arg1 > 0)
        arg1 -= 1;

    D_800A3418 ^= rand();
    base_x = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= rand();
    base_y = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    i = 0;
    if (i < ((D_800A326C + 1) * 2)) {
        var_10 = 0x100;
        var_11 = 1;
        row_off = arg1 * 0x3C;
        var_12 = &D_8009B2C8[0][2];
        random_header_pair = &D_8009B2C8[arg1][3];
        do {
            s.has_color = 0;
            /* FAKE: integer sum keeps `addu v0,s0,fp`; the byte-pointer sum
             * swaps it: score 1 */
            s.header = (Unk8009B0E0Record *)(row_off + (s32)var_12);
            s.table = &D_8009B388[0];
            D_800A3418 ^= rand();
            s.scale_y = var_10;
            s.scale_x = var_10;
            D_800A3418 ^= rand();
            /* Ruling 4 compound-assignment split; one expression: score 3. */
            x = (s32)base_x - 0x19;
            x += ((u32)(D_800A3418 * 0x32) >> 0xF);
            /* FAKE: tmp1 is written twice (y base, then a copy of var_4) so
             * sched1 places the y base after the argument setup; dropping
             * either write or one carrier for both halves does not match (no
             * y-base write: score 4; no var_4 copy: 35; one carrier: 30) */
            tmp1 = (s32)base_y - 0xC;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp1 + ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.semi = 0;
            s.ot_idx = var_11;
            tmp1 = var_4;
            s.ft4_out = tmp1;
            s.y = y;
            var_4 = gpu_AddScaledSpriteQuads(&s, 0);

            s.has_color = 0;
            s.scale_y = var_10;
            s.scale_x = var_10;
            s.table = &D_8009B388[1];
            s.header = random_header_pair + (D_800A3418 & 1);
            D_800A3418 ^= rand();
            /* Ruling 4 compound-assignment split; one expression: score 3. */
            x = (s32)base_x - 0x32;
            x += ((u32)(D_800A3418 * 0x64) >> 0xF);
            /* FAKE: tmp2 written twice, as tmp1 above (no y-base write: score
             * 4; no var_4 copy: 35) */
            tmp2 = (s32)base_y - 0x19;
            s.x = x;
            D_800A3418 ^= rand();
            y = tmp2 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.semi = 0;
            s.ot_idx = var_11;
            tmp2 = var_4;
            s.ft4_out = tmp2;
            s.y = y;
            var_4 = gpu_AddScaledSpriteQuads(&s, 0);
            i += 1;
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return var_4;
}

/* gpu_DrawTimeFieldsAndDecimalCounter's 7 sprite cells: [0..2] drawn with
 * header D_8009B398[0], [3..4] with [1], [5] / [6] with [2] / [3] */
extern Unk8009B400Record D_8009B3C8[7];

s32 gpu_DrawTimeFieldsAndDecimalCounter(
    Unk8001CD68Rec *arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *var_0 = (Unk8005D814Rec *)arg2;
    Unk8007352CRec s;
    s16 var_2[3];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg2); returning sizeof(Unk8005D814Rec) directly:
     * score 76 (Q129). */
    s32 end_off;
    s16 i;
    s16 j;
    s16 var_9;

    arg1--;
    tile = var_0->unk_00;
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.ot_idx = arg3;
    s.semi = 0;
    s.header = &D_8009B398[0];
    cur = arg2 + sizeof(var_0->unk_00);
    mode_off = &var_0->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    for (i = 0; i < 3; i++) {
        s.table = &D_8009B3C8[i];
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }

    s.header = &D_8009B398[1];
    for (i = 0; i < 2; i++) {
        s.table = &D_8009B3C8[3 + i];
        if (i != 0) {
            if (arg1 == 1) {
                s.table->w = 0x2D;
            } else {
                s.table->w = 0x3C;
            }
        }
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.y = 0x16;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 2; i++) {
            switch (j) {
            case 0:
                var_2[i] = arg0->unk_0;
                if (i != 0) {
                    var_2[i] = var_2[i] % 10;
                } else {
                    var_2[0] = var_2[0] / 10 % 10;
                }
                s.table = &D_8009B400[var_2[i]];
                if (var_2[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x1A2;
                break;
            case 1:
                var_2[i] = arg0->unk_2;
                if (i != 0) {
                    var_2[i] = var_2[i] % 10;
                } else {
                    var_2[0] = var_2[0] / 10 % 10;
                }
                s.table = &D_8009B400[var_2[i]];
                if (var_2[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x1D3;
                break;
            case 2:
                var_2[i] = arg0->unk_3;
                if (i != 0) {
                    var_2[i] = var_2[i] % 10;
                } else {
                    var_2[0] = var_2[0] / 10 % 10;
                }
                s.table = &D_8009B400[var_2[i]];
                if (var_2[i] == 1) {
                    s.x = i * 20 + 3;
                } else {
                    s.x = i * 20;
                }
                s.table->x = 0x209;
                break;
            }
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
    }

    s.header = &D_8009B398[0];
    var_2[0] = var_2[1] = var_2[2] = arg1;
    var_2[2] = var_2[2] % 10;
    var_2[1] = var_2[1] / 10 % 10;
    var_2[0] = var_2[0] / 100;
    var_2[1] = var_2[1] % 100;
    s.y = 0x29;
    var_9 = 0;
    for (j = 0; j < 3; j++) {
        if (var_9 || var_2[j] != 0 || j == 2) {
            s.table = &D_8009B400[var_2[j]];
            s.table->x = 0x1F3;
            var_9 = 1;
            if (var_2[j] == 1) {
                s.x = j * 21 + 3;
            } else {
                s.x = j * 21;
            }
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        SetTile(tile);
        tile->r0 = 0xFF;
        tile->g0 = 0x10;
        tile->b0 = 0x10;
        tile->x0 = D_8009B450[j].x;
        tile->y0 = D_8009B450[j].y;
        tile->w = 0x238 - D_8009B450[j].x;
        tile->h = 1;
        SetSemiTrans(tile, 0);
        AddPrim(g_gpu_ot_ptr + arg3, tile);
        tile++;
        s.y = D_8009B450[j].y;
        s.header = &D_8009B398[2];
        s.table = &D_8009B3C8[5];
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        s.header = &D_8009B398[3];
        s.table = &D_8009B3C8[6];
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B398[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

extern Unk8009B400Record D_8009B488;

s32 gpu_DrawTwoDigitCounterWithOptionalSecondSide(
    s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *var_0 = (Unk8005D814Rec *)arg2;
    Unk8007352CRec s;
    s16 d[2];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg2); returning sizeof(Unk8005D814Rec) directly:
     * score 53 (Q129). */
    s32 end_off;
    s16 i;
    s16 j;

    tile = var_0->unk_00;
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.header = &D_8009B398[1];
    cur = arg2 + sizeof(var_0->unk_00);
    mode_off = &var_0->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    s.ot_idx = arg3;
    for (i = 0; i < 2; i++) {
        if (arg0 < 0) {
            s.table = &D_8009B488;
            if (arg1 == 1) {
                s.table->w = 0x2D;
            } else {
                s.table->w = 0x3C;
            }
        } else {
            s.table = &D_8009B458[0][i];
        }
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        if (arg0 < 0) {
            break;
        }
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.semi = 0;
    s.y = 0x16;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        if (j) {
            d[0] = d[1] = arg0;
        } else {
            d[0] = d[1] = arg1;
        }
        d[1] = d[1] % 10;
        d[0] = d[0] / 10 % 10;
        for (i = 0; i < 2; i++) {
            if (d[i] == 0 && i == 0 && arg0 < 0) {
                i++;
            }
            s.table = &D_8009B400[d[i]];
            if (j != 0) {
                s.table->x = 0x50;
            } else {
                s.table->x = 0x209;
            }
            if (d[i] == 1) {
                s.x = i * 20 + 3;
            } else {
                s.x = i * 20;
            }
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
        if (arg0 < 0) {
            break;
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        SetTile(tile);
        tile->r0 = 0xFF;
        tile->g0 = 0x10;
        tile->b0 = 0x10;
        tile->x0 = 0x209 - j * 0x1C1;
        tile->y0 = 0x24;
        tile->w = 0x30;
        tile->h = 1;
        SetSemiTrans(tile, 0);
        AddPrim(g_gpu_ot_ptr + arg3, tile);
        tile++;
        s.y = 0x24;
        s.header = &D_8009B398[2];
        s.table = &D_8009B458[j + 1][0];
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        s.header = &D_8009B398[3];
        s.table = &D_8009B458[j + 1][1];
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        if (arg0 < 0) {
            break;
        }
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B398[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

s32 gpu_DrawDecrementedSingleSideCounter(s32 a0, s32 a1, s32 a2) {
    return gpu_DrawTwoDigitCounterWithOptionalSecondSide(-1, a0 - 1, a1, a2);
}

extern Unk8009B0E0Record D_8009ADB4;
extern Unk8009B400Record D_8009ADC0[3];
extern Unk8009B400Record D_8009ADD8[][3];
extern Unk8009B0E0Record D_8009B4B0;
extern Unk8009B400Record D_8009B4BC[5];
extern Unk8009B0E0Record D_8009B4E4;
extern Unk8009B0E0Record D_8009B4F0;
extern Unk8009B400Record D_8009B4FC;
extern Unk8009B400Record D_8009B504;
extern Unk8009B400Record D_8009B50C;
extern Unk8009B400Record D_8009B514;
extern Unk8009B400Record D_8009B51C;
extern Unk8009B0E0Record D_8009B524;
extern Unk8009B0E0Record D_8009B530;
extern Unk8009B0E0Record D_8009B53C;
extern Unk8009B0E0Record D_8009B548;
extern Unk8009B400Record D_8009B554[3];
extern Unk8009B400Record D_8009B56C[2];
extern Unk8009B400Record D_8009B57C[2];
extern u8 D_8009B58C[];
extern u8 D_800A3270[];

s32 gpu_DrawPackedComparisonRowsAndTotals(u32 arg0, s32 arg1, s32 arg2) {
    Unk8005E54CRec *var_0 = (Unk8005E54CRec *)arg1;
    /* An s16 pair: the round rows' values, then the totals under them. The
       target addresses both through the one frame slot sp+0x18 (a separate
       totals array does not match). */
    s16 var_1[2];
    s16 var_2[2];
    Unk8007352CRec s;
    /* FAKE: frame layout: unused; keeps the 8 untouched bytes at sp+0x58 where
     * the siblings gpu_DrawTimeFieldsAndDecimalCounter and
     * gpu_DrawPairedMarkRowsAndTimeFields keep a real s16[3] array (Q35,
     * no-new-park-categories phantom-frame-slot pad family). Removed: score 47
     */
    volatile s16 digit[3];
    TILE *tile;
    s32 cur;
    s32 var_7;
    DR_MODE *mode_off;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg1); returning sizeof(Unk8005E54CRec) directly:
     * score 75 (Q129). */
    s32 end_off;
    /* i counts the players (first loop) and then the rounds; j is the player
       and k the mark; each phase restarts them loop to loop as plain indices,
       the way SOTN's DebugCaptureScreen reuses its i for its file-search loop
       and its row countdown (Q51), as
       gpu_DrawTwoDigitCounterWithOptionalSecondSide /
       gpu_DrawPairedMarkRowsAndTimeFields do */
    /* SOTN: src/dra/42398.c:75 @aa53500 */
    s16 i;
    s16 j;
    s16 k;
    s16 c;
    s16 y;

    tile = var_0->unk_00;
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    cur = arg1 + sizeof(var_0->unk_00);
    var_7 = (s32)var_0->unk_898;
    mode_off = &var_0->unk_BB8;
    end_off = arg1 + sizeof(Unk8005E54CRec);
    s.ot_idx = arg2;
    for (i = 0; i < 2; i++) {
        if (!(g_working_record.unk14_15 >> i & 1)) {
            s.header = &D_8009B524;
        } else {
            s.header = &D_8009B53C;
        }
        s.x = i * 320;
        s.table = D_8009B554;
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        if (!(g_working_record.unk14_15 >> i & 1)) {
            s.header = &D_8009B530;
            s.table = D_8009B56C;
        } else {
            s.header = &D_8009B548;
            s.table = D_8009B57C;
        }
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }

    s.semi = 0;
    s.has_color = 0;
    for (i = 0; i < g_working_record.unk14_10 + 3; i++) {
        var_1[0] = (arg0 >> (i * 4)) & 3;
        var_1[1] = (arg0 >> (i * 4 + 2)) & 3;
        if (g_working_record.unk14_10 == 2) {
            s.y = i * 24 + 0x44;
        } else if (g_working_record.unk14_10 == 1) {
            s.y = i * 24 + 0x4F;
        } else {
            s.y = i * 34 + 0x4F;
        }
        if (var_1[0] == 3 || var_1[1] == 3) {
            s.header = &D_8009B4B0;
            s.x = 0;
            s.y += 2;
            s.table = &D_8009B4BC[D_800A3270[i]];
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        } else {
            s.header = &D_8009B4E4;
            s.x = 0;
            s.table = &D_8009B514;
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
            for (j = 0; j < 2; j++) {
                s.x = j * 70;
                if (var_1[j] > var_1[!j]) {
                    s.table = &D_8009B4FC;
                } else if (var_1[j] < var_1[!j]) {
                    s.table = &D_8009B504;
                } else {
                    s.table = &D_8009B50C;
                }
                s.sprt_out = cur;
                cur = gpu_AddSpriteCells(&s);
            }
            s.y += 5;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B4F0;
                s.table = &D_8009B51C;
                for (k = 0; k < var_1[j]; k++) {
                    if (j) {
                        s.x = k * 16 + 0x179;
                    } else {
                        s.x = (1 - k) * 16 + 0xE2;
                    }
                    s.sprt_out = cur;
                    cur = gpu_AddSpriteCells(&s);
                }
            }
        }
    }

    s.header = &D_8009B4E4;
    s.x = 0;
    if (g_working_record.unk14_10 == 2) {
        y = 0xC6;
    } else if (g_working_record.unk14_10 == 1) {
        y = 0xC2;
    } else {
        y = 0xBE;
    }
    s.y = y + 3;
    s.table = &D_8009B514;
    s.sprt_out = cur;
    cur = gpu_AddSpriteCells(&s);
    /* FAKE: one 32-bit store clears the whole pair (target `sw
     * $zero,0x18($sp)`; Q36, no-new-park-categories); `var_1[0] = var_1[1] =
     * 0`: score 2 */
    *(s32 *)var_1 = 0;
    for (i = 0; i < g_working_record.unk14_10 + 3; i++) {
        if (((arg0 >> (i * 4)) & 3) != 3) {
            var_1[0] += (arg0 >> (i * 4)) & 3;
        }
        if (((arg0 >> (i * 4 + 2)) & 3) != 3) {
            var_1[1] += (arg0 >> (i * 4 + 2)) & 3;
        }
    }
    for (j = 0; j < 2; j++) {
        s.header = &D_8009B4F0;
        s.table = &D_8009B51C;
        for (k = 0; k < var_1[j]; k++) {
            if (j) {
                s.x = (k >> 1) * 20 + 0x181;
            } else {
                s.x = 0xF2 - (k >> 1) * 20;
            }
            s.y = y + (k & 1) * 12;
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B524, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    /* The second DR_MODE goes at the record's end, past the size this function
     * returns (2B344 fight_UpdateMatchTallyPresentation advances its cursor
     * g_prim_buf_cursor by that size; 6CF8 gpu_DrawDisplayModeOnLocalOtAndSync
     * ignores it). */
    mode_off++;

    s.ot_idx = arg2;
    var_2[0] = var_2[1] = 0;
    for (i = 0; i < g_working_record.unk14_10 + 3; i++) {
        s.header = &D_8009ADB4;
        s.semi = 0;
        var_1[0] = (arg0 >> (i * 4)) & 3;
        var_1[1] = (arg0 >> (i * 4 + 2)) & 3;
        s.col_r = s.col_g = s.col_b = 0x40;
        for (j = 0; j < 2; j++) {
            if (var_1[j] <= var_1[!j]) {
                if (var_1[j] != 3) {
                    s.has_color = 1;
                } else {
                    s.has_color = 0;
                }
            } else {
                if (var_1[j] != 3) {
                    var_2[j]++;
                }
                s.has_color = 0;
            }
            c = g_working_record.unk00[j][i].bd0;
            if (c >= 12) {
                c -= 2;
            }
            s.table = D_8009ADD8[c];
            s.x = j * 320 + D_8009B58C[c];
            if (g_working_record.unk14_10 == 2) {
                s.y = i * 24 - 8;
            } else if (g_working_record.unk14_10 == 1) {
                s.y = i * 24 + 3;
            } else {
                s.y = i * 34 + 3;
            }
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
            if (g_working_record.unk00[j][i].bd0 == 8) {
                s.table = D_8009ADC0;
                s.sprt_out = cur;
                cur = gpu_AddSpriteCells(&s);
            }
        }
        s.has_color = 0;
        if (var_1[0] == 3) {
            s.y += 0x4C;
            s.scale_x = 0x100;
            s.x = 0;
            s.semi = 0;
            s.scale_y = 0x400;
            s.y += 6;
            for (j = 0; j < 2; j++) {
                s.header = &D_8009B398[j + 2];
                s.table = D_8009B490[j];
                s.ft4_out = var_7;
                var_7 = gpu_AddScaledSpriteQuads(&s, 0);
                s.table = &D_8009B490[j][1];
                s.ft4_out = var_7;
                var_7 = gpu_AddScaledSpriteQuads(&s, 0);
            }
        }
    }

    s.header = &D_8009B398[0];
    s.semi = 0;
    if (g_working_record.unk14_10 == 2) {
        s.y = 0xC9;
    } else if (g_working_record.unk14_10 == 1) {
        s.y = 0xC5;
    } else {
        s.y = 0xC1;
    }
    for (j = 0; j < 2; j++) {
        s.x = j * 70 + 0x113;
        if (var_2[j] == 1) {
            s.x += 3;
        }
        s.table = &D_8009B400[var_2[j]];
        s.table->x = s.table->y = 0;
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }

    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 8;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    tile->x0 = 0x19D;
    tile->y0 = 0x3A;
    tile->w = 0xDC;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    tile++;
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0x10;
    tile->b0 = 0x10;
    if (g_working_record.unk14_10 == 2) {
        setXY0(tile, 0x5E, 0xC1);
    } else if (g_working_record.unk14_10 == 1) {
        setXY0(tile, 0x5E, 0xBD);
    } else {
        setXY0(tile, 0x5E, 0xB9);
    }
    tile->w = 0x1C5;
    tile->h = 1;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + arg2, tile);
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009ADB4, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    return end_off - arg1;
}

extern Unk8009B0E0Record D_8009B5A0[2];
extern Unk8009B400Record D_8009B5B8[2][2];
extern Unk8009B400Record D_8009B5D8[2];
extern Unk8009B400Record D_8009B5E8;

s32 gpu_DrawPairedMarkRowsAndTimeFields(
    Unk8001CD68Rec *arg0, s32 arg1, s32 arg2, s32 arg3) {
    Unk8005D814Rec *var_0 = (Unk8005D814Rec *)arg2;
    Unk8007352CRec s;
    s16 d[3];
    TILE *tile;
    s32 cur;
    DR_MODE *mode_off;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg2); returning sizeof(Unk8005D814Rec) directly:
     * score 97 (Q129). */
    s32 end_off;
    /* i/row count the first phase's two loops; k and j are reused loop to
     * loop as plain indices by the later phases (tile strip k/j, timer j/k),
     * the way SOTN's DebugCaptureScreen reuses its i for its file-search loop
     * and its row countdown (Q51) */
    /* SOTN: src/dra/42398.c:75 @aa53500 */
    s16 i;
    s16 j;
    s16 k;
    s16 row;
    s16 var_11;
    s16 count;
    s32 x;

    s.has_color = 0;
    s.y = 0;
    s.semi = 0;
    tile = var_0->unk_00;
    cur = arg2 + sizeof(var_0->unk_00);
    mode_off = &var_0->unk_2F8;
    end_off = arg2 + sizeof(Unk8005D814Rec);
    s.ot_idx = arg3;
    for (i = 0; i < 2; i++) {
        s.header = &D_8009B5A0[i];
        var_11 = (arg1 >> (i * 8)) & 0xFF;
        if (i != 0) {
            count = 2;
        } else {
            count = g_working_record.unk14_14 + 1;
        }
        for (row = 0; row < 2; row++) {
            for (k = 0; k < count; k++) {
                if (row != 0) {
                    s.x = i * 8 + 0x1C2 - (0x1C - i * 8) * k;
                } else {
                    s.x = (0x1C - i * 8) * k;
                }
                s.table = &D_8009B5B8[i][0];
                if (k >= ((var_11 >> (row * 4)) & 0xF)) {
                    s.table++;
                }
                s.sprt_out = cur;
                cur = gpu_AddSpriteCells(&s);
            }
        }
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B5A0[0], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    /* The second DR_MODE goes at the record's end, past the size this function
     * returns (its caller, 9F9C fight_UpdateModeDisplaysAndMatchProgress,
     * advances its cursor g_prim_buf_cursor by that size). */
    mode_off++;

    s.has_color = 0;
    s.y = 0;
    s.semi = 0;
    s.header = &D_8009B398[1];
    s.ot_idx = arg3;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            s.x = j * 550;
            s.table = &D_8009B5D8[k];
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
    }

    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg3;
    for (k = 0; k < 2; k++) {
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = 0xFF;
            tile->g0 = 0x10;
            tile->b0 = 0x10;
            tile->x0 = 0x48 + j * 431 + j * (k << 4);
            tile->y0 = k * 20 + 0x24;
            tile->w = 0x42 - k * 16;
            tile->h = 1;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + arg3, tile);
            tile++;
            s.y = k * 20 + 0x24;
            s.header = &D_8009B398[2];
            s.table = &D_8009B5F0[j][0];
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
            s.header = &D_8009B398[3];
            s.table = &D_8009B5F0[j][1];
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
    }

    s.header = &D_8009B398[0];
    s.has_color = 0;
    s.y = 0x16;
    s.semi = 0;
    s.ot_idx = arg3;
    for (j = 0; j < 2; j++) {
        for (k = 0; k < 3; k++) {
            switch (j) {
            case 0:
                if (k < 2 || g_working_record.unk14_12 == 2) {
                    d[k] = arg0->unk_2;
                    if (k == 0 && g_working_record.unk14_12 == 2) {
                        d[k] = d[k] / 100;
                    } else if (
                        k == 0 || (k == 1 && g_working_record.unk14_12 == 2)) {
                        d[k] = d[k] / 10;
                    }
                    d[k] = d[k] % 10;
                    s.table = &D_8009B400[d[k]];
                    if (d[k] == 1) {
                        s.x = k * 20 + 3;
                    } else {
                        s.x = k * 20;
                    }
                }
                break;
            case 1:
                if (k < 2) {
                    d[k] = arg0->unk_3;
                    if (k != 0) {
                        d[k] = d[k] % 10;
                    } else {
                        d[k] = d[k] / 10 % 10;
                    }
                    x = (g_working_record.unk14_12 == 2) ? k * 20 + 0x48
                                                         : k * 20 + 0x34;
                    s.table = &D_8009B400[d[k]];
                    if (d[k] == 1) {
                        s.x = x + 3;
                    } else {
                        s.x = x;
                    }
                }
                break;
            }
            if (g_working_record.unk14_12 == 2) {
                s.table->x = 0x109;
            } else {
                s.table->x = 0x113;
            }
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
        s.table = &D_8009B5E8;
        if (g_working_record.unk14_12 == 2) {
            s.x = j * 6 + 0x145;
        } else {
            s.x = j * 6 + 0x13B;
        }
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(&D_8009B398[1], 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, mode_off);
    return end_off - arg2;
}

/* gpu_DrawSelectedStretchedSpritePair's sheets: three headers at 0x8009B610
 * (the second draw, header arg0, its one cell at 0x8009B634) and three at
 * 0x8009B63C (counts 2 / 1 / 4) whose cells follow at 0x8009B660 (0-1, 2, 3-6).
 */
extern Unk8009B0E0Record D_8009B610[3];
extern Unk8009B400Record D_8009B634;
extern Unk8009B0E0Record D_8009B63C[3];
extern Unk8009B400Record D_8009B660[7];

s32 gpu_DrawSelectedStretchedSpritePair(s32 arg0, s32 arg1, s32 arg2) {
    Unk8007352CRec s;
    s32 cursor = arg1;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end - arg1); returning 0x190 directly:
     * score 12 (Q129). */
    s32 end = arg1 + 0x190;

    s.scale_x = 0x200;
    s.scale_y = 0x100;
    s.header = &D_8009B63C[arg0];
    s.has_color = 0;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.ot_idx = arg2;
    switch (arg0) {
    case 0:
        s.table = &D_8009B660[0];
        break;
    case 1:
        s.table = &D_8009B660[2];
        break;
    case 2:
        s.table = &D_8009B660[3];
        break;
    }
    s.ft4_out = cursor;
    cursor = gpu_AddScaledSpriteQuads(&s, 0);
    s.header = &D_8009B610[arg0];
    s.table = &D_8009B634;
    s.ft4_out = cursor;
    gpu_AddScaledSpriteQuads(&s, 0);
    return end - arg1;
}

extern RECT D_800A327C;
extern RECT D_800A3284;
extern s32 D_800A3278;

void gpu_ReadCdFileUploadTwoRegionsAndResetSplitWipe(
    s32 arg0, u8 *file_buffer) {
    RECT r1, r2;
    s32 s0;
    s0 = cdrom_GetFileIndex(2, arg0 + 0x33);
    cdrom_StartRead(s0, (s32)file_buffer);
    cdrom_WaitIdle();
    cdrom_GetFileSize(s0);
    r1 = D_800A327C;
    r2 = D_800A3284;
    LoadImage(&r1, (u32 *)(file_buffer + 0x40));
    DrawSync(0);
    LoadImage(&r2, (u32 *)(file_buffer + 0x14));
    DrawSync(0);
    D_800A3278 = 0;
}

extern Unk8009B0E0Record D_8009B698[2];
extern Unk8009B400Record D_8009B6B0[8];

s32 gpu_DrawImageAndAdvanceSplitWipe(s32 arg0, s32 arg1) {
    Unk8005FC9CRec *var_0 = (Unk8005FC9CRec *)arg0;
    Unk8007352CRec s;
    RECT r;
    RECT *var_3;
    GpuDb *var_4;
    s32 cur;
    DR_MODE *mode_off;
    POLY_G4 *poly;
    DR_AREA *area;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg0); returning sizeof(Unk8005FC9CRec) directly:
     * score 55 (Q129). */
    s32 end_off;
    s16 j;
    s16 i;
    s16 off;
    s16 x;
    u8 c;

    cur = arg0;
    mode_off = &var_0->unk_280;
    poly = var_0->unk_28C;
    area = var_0->unk_2D4;
    end_off = arg0 + sizeof(Unk8005FC9CRec);
    j = 0;
    var_4 = &g_gpu_db[g_gpu_presented_frame_count & 1];
    r.x = var_4->draw.clip.x;
    r.y = var_4->draw.clip.y;
    r.w = var_4->draw.clip.w;
    r.h = var_4->draw.clip.h;
    var_3 = &var_4->draw.clip;
    SetDrawArea(area, &r);
    AddPrim(g_gpu_ot_ptr + arg1, area);
    area++;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.ot_idx = arg1;
    s.table = D_8009B6B0;
    off = (D_800A3278 - 0xB4) * 24;
    do {
        if (D_800A3278 >= 0xB5) {
            SetPolyG4(poly);
            SetSemiTrans(poly, 1);
            if (j != 0) {
                x = off + 0x140;
                r.x = var_3->x + x;
                r.y = var_3->y;
                r.w = var_3->w / 2 - off;
                r.h = var_3->h;
                poly->x0 = x;
                poly->y0 = 0;
                poly->x1 = x;
                poly->y1 = 0xF0;
                poly->x2 = off + 0x154;
                poly->y2 = 0;
                poly->x3 = off + 0x154;
                poly->y3 = 0xF0;
            } else {
                r.x = var_3->x;
                r.y = var_3->y;
                r.w = var_3->w / 2 - off;
                r.h = var_3->h;
                poly->x0 = 0x140 - off;
                poly->y0 = 0;
                poly->x1 = 0x140 - off;
                poly->y1 = 0xF0;
                poly->x2 = 0x12C - off;
                poly->y2 = 0;
                poly->x3 = 0x12C - off;
                poly->y3 = 0xF0;
            }
            c = ~((off * 255) / 320);
            poly->r0 = c;
            poly->g0 = 0;
            poly->b0 = 0;
            poly->r1 = c;
            poly->g1 = 0;
            poly->b1 = 0;
            poly->r2 = 0;
            poly->g2 = 0;
            poly->b2 = 0;
            poly->r3 = 0;
            poly->g3 = 0;
            poly->b3 = 0;
            AddPrim(g_gpu_ot_ptr + arg1, poly);
            poly++;
        }
        for (i = 0; i < 2; i++) {
            s.header = &D_8009B698[i];
            s.y = i << 6;
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
        if (D_800A3278 >= 0xB5) {
            SetDrawArea(area, &r);
            AddPrim(g_gpu_ot_ptr + arg1, area);
            area++;
        }
        j++;
    } while (j < 2);
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(D_8009B698, 0x20), 0);
    AddPrim(g_gpu_ot_ptr + arg1, mode_off);
    if (off <= 0x140) {
        D_800A3278++;
    }
    return end_off - arg0;
}

/* gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite's sheets: the frame
 * header (3 cells, D_8009B758) and the digit header (1 cell; one of the ten
 * digit cells D_8009B708). */
extern Unk8009B0E0Record D_8009B6F0[2];
extern Unk8009B400Record D_8009B708[10];
extern Unk8009B400Record D_8009B758[3];

s32 gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite(
    s32 arg0, s32 arg1, s32 arg2) {
    Unk8007352CRec s;
    s16 d[2];
    Unk800600C8Rec *var_2 = (Unk800600C8Rec *)arg1;
    DR_MODE *mode_off = &var_2->unk_B4;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg1); returning sizeof(Unk800600C8Rec) directly:
     * score 20 (Q129). */
    s32 end_off = arg1 + sizeof(Unk800600C8Rec);
    s32 cur = arg1;
    s32 i;

    s.header = &D_8009B6F0[0];
    s.has_color = 0;
    s.semi = 0;
    s.y = 0;
    s.ot_idx = arg2;
    if (arg0 < 0xA) {
        s.x = 0x93;
    } else {
        s.x = 0xA3;
    }
    s.table = D_8009B758;
    s.sprt_out = cur;
    cur = gpu_AddSpriteCells(&s);
    s.header = &D_8009B6F0[1];
    d[0] = d[1] = arg0;
    d[1] = d[1] / 10 % 10;
    d[0] = d[0] % 10;
    for (i = 0; i < 2; i++) {
        s.table = &D_8009B708[d[i]];
        if (arg0 < 0xA) {
            s.x = 0x64;
        } else {
            s.x = (((1 - i) << 2) << 3) + 0x54;
        }
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        if (d[1] == 0) {
            break;
        }
    }
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(D_8009B6F0, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg2, mode_off);
    return end_off - arg1;
}

extern RECT D_800A3294;
extern RECT D_800A329C;
extern RECT D_800A32A4;
extern RECT D_800A32AC;

void gpu_ReadCdFileRelocateTwoImagesAndUploadFourRegions(
    s32 arg0, s32 *file_buffer) {
    RECT r1, r2, r3, r4;
    s32 s1;
    u8 *p;
    u8 *q;
    s1 = cdrom_GetFileIndex(2, arg0 + 0x3D);
    cdrom_StartRead(s1, (s32)file_buffer);
    cdrom_WaitIdle();
    cdrom_GetFileSize(s1);
    file_buffer[0] = file_buffer[0] + (s32)file_buffer;
    file_buffer[1] = file_buffer[1] + (s32)file_buffer;
    r1 = D_800A3294;
    r2 = D_800A329C;
    p = (u8 *)file_buffer[0];
    LoadImage(&r1, (u32 *)(p + 0x40));
    DrawSync(0);
    LoadImage(&r2, (u32 *)(p + 0x14));
    DrawSync(0);
    r3 = D_800A32A4;
    r4 = D_800A32AC;
    q = (u8 *)file_buffer[1];
    LoadImage(&r3, (u32 *)(q + 0x60));
    DrawSync(0);
    LoadImage(&r4, (u32 *)(q + 0x14));
    DrawSync(0);
}

extern Unk8009B0E0Record D_8009B7AC[3];
/* packed screen positions: x = (v >> 7) + 0x37, y = (v & 0x7F) + 0x2A */
extern u16 D_8009B850[];

s32 gpu_DrawSpriteAtPackedTablePosition(s16 arg0, s32 arg1, s32 ot_index) {
    Unk8007352CRec s;
    Unk80060414Rec *sprite_chunk = (Unk80060414Rec *)arg1;
    DR_MODE *mode_off;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg1); returning sizeof(Unk80060414Rec) directly:
     * score 22 (Q129). */
    s32 end_off;
    mode_off = &sprite_chunk->unk_14;
    end_off = arg1 + sizeof(Unk80060414Rec);
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = ot_index;
    s.x = (D_8009B850[arg0 & 0x7FFF] >> 7) + 0x37;
    s.y = (D_8009B850[arg0 & 0x7FFF] & 0x7F) + 0x2A;
    if (arg0 & 0x8000) {
        s.header = &D_8009B7AC[0];
    } else if (g_working_record.unk00[0][0].bd0 < 0xC) {
        s.header = &D_8009B7AC[1];
    } else {
        s.header = &D_8009B7AC[2];
    }
    s.table = &D_800A328C;
    s.sprt_out = (s32)&sprite_chunk->unk_00;
    gpu_AddSpriteCells(&s);
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + ot_index, mode_off);
    return end_off - arg1;
}

extern Unk8009B0E0Record D_8009B770[4];
extern Unk8009B0E0Record D_8009B7A0;
extern Unk8009B400Record D_8009B7D0;
extern Unk8009B400Record D_8009B7D8[5];
extern Unk8009B400Record D_8009B800[4];
extern Unk8009B400Record D_8009B820[4];
extern Unk8009B400Record D_8009B840[2];

s32 gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells(s32 arg0, s32 arg1) {
    Unk80060544Rec *var_0 = (Unk80060544Rec *)arg0;
    Unk8007352CRec s;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg0); returning sizeof(Unk80060544Rec) directly:
     * score 25 (Q129). */
    s32 end_off;
    s32 var_3;
    s32 i;
    s32 j;
    s32 cur;
    Unk8009B0E0Record *p0;
    Unk8009B400Record *p1;
    DR_MODE *mode_off;
    cur = arg0;
    var_3 = (s32)var_0->unk_4EC;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = arg1;
    mode_off = &var_0->unk_5DC;
    end_off = arg0 + sizeof(Unk80060544Rec);
    s.scale_x = 0x200;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    i = 0;
    do {
        s.header = &D_8009B770[i];
        switch (i) {
        case 0:
            s.table = D_8009B7D8;
            break;
        case 1:
        case 2:
            s.table = D_8009B800;
            break;
        case 3:
            s.table = &D_8009B7D0;
            s.ft4_out = var_3;
            var_3 = gpu_AddScaledSpriteQuads(&s, 0);
            break;
        }
        if (i != 3) {
            s.sprt_out = cur;
            cur = gpu_AddSpriteCells(&s);
        }
        i += 1;
    } while (i < 4);
    s.header = &D_8009B7A0;
    s.table = D_8009B820;
    s.sprt_out = cur;
    cur = gpu_AddSpriteCells(&s);
    j = 0;
    p1 = D_8009B840;
    p0 = &D_8009B398[2];
    s.col_r = 0xFF;
    s.col_b = 0x10;
    s.col_g = 0x10;
    s.has_color = 1;
    do {
        s.header = p0;
        s.table = p1;
        s.sprt_out = cur;
        cur = gpu_AddSpriteCells(&s);
        p1++;
        j += 1;
        p0++;
    } while (j < 2);
    SetDrawMode(mode_off, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg1, mode_off);
    return end_off - arg0;
}

extern u16 D_800A32B6;
extern u16 D_800A32B4;

void gpu_ResetGrowingBarProgress(void) {
    D_800A32B6 = 0;
    D_800A32B4 = 0;
}

extern Unk8006D808Rec D_8009B0C0;

s32 gpu_DrawIndexedCellsDigitPairsAndGrowingBars(s32 arg0, s32 arg1, s32 arg2) {
    s32 sp18;
    s32 sp1C;
    s32 t1;
    s32 t2;
    u16 cur1;
    u16 cur2;
    /* FAKE: the returned size is computed as the chunk end minus its start
     * (end_off - arg0); returning 0xAC8 directly:
     * score 26 (Q129). */
    s32 end_off;
    s32 var_7;

    var_7 = arg0 + 0x7D0;
    end_off = arg0 + 0xAC8;
    sp18 = arg0;
    sp1C = arg0 + 0x870;
    gpu_DrawIndexedCellsAndDecimalDigitPairs(
        &sp18, &sp1C, &D_8009B0C0, arg1, arg2);
    if ((u32)arg2 < 3U) {
        SetTile((TILE *)var_7);
        *(u8 *)(arg0 + 0x7D4) = 0xFF;
        *(s16 *)(arg0 + 0x7D8) = 0x6A;
        *(u8 *)(arg0 + 0x7D5) = 0;
        *(u8 *)(arg0 + 0x7D6) = 0;
        *(s16 *)(arg0 + 0x7DA) = (s16)(arg2 * 0x1A + 0x5B);
        cur1 = D_800A32B4;
        /* FAKE: increment staged through t1 (stored next line, then reused for
         * the product): the reassignment breaks cse's mem==reg equivalence, so
         * the clamp re-read emits lh (staged-value-reused-variable); direct
         * increment: score 8 (with t2's: 16); fresh s16/u16/s32 next
         * locals or inline products at both sites: score 16 */
        t1 = cur1 + 1;
        D_800A32B4 = t1;
        t1 = (s32)((s16)cur1) * 0x1AA;
        *(s16 *)(arg0 + 0x7DE) = 2;
        *(s16 *)(arg0 + 0x7DC) = (s16)(t1 / 0x1E);
        if ((s16)D_800A32B4 >= 0x1F) {
            D_800A32B4 = 0x1E;
        }
        SetSemiTrans((TILE *)var_7, 0);
        AddPrim(g_gpu_ot_ptr + arg1, (void *)var_7);
        var_7 = arg0 + 0x7E0;
    }
    SetTile((TILE *)var_7);
    *(u8 *)(var_7 + 4) = 0xFF;
    *(s16 *)(var_7 + 8) = 0x9E;
    *(u8 *)(var_7 + 5) = 0;
    *(u8 *)(var_7 + 6) = 0;
    *(s16 *)(var_7 + 0xA) = 0xBD;
    cur2 = D_800A32B6;
    /* FAKE: increment staged through t2, as t1 above: the clamp re-read emits
     * lh (staged-value-reused-variable); direct increment: score 8 */
    t2 = cur2 + 1;
    D_800A32B6 = t2;
    t2 = (s32)((s16)cur2) * 0x144;
    *(s16 *)(var_7 + 0xE) = 2;
    *(s16 *)(var_7 + 0xC) = (s16)(t2 / 0x1E);
    if ((s16)D_800A32B6 >= 0x1F) {
        D_800A32B6 = 0x1E;
    }
    SetSemiTrans((TILE *)var_7, 0);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)var_7);
    var_7 += 0x10;

    SetTile((TILE *)var_7);
    *(s16 *)(var_7 + 8) = 0x3F;
    *(s16 *)(var_7 + 0xA) = 0x2D;
    *(s16 *)(var_7 + 0xC) = 0x202;
    *(u8 *)(var_7 + 4) = 0;
    *(u8 *)(var_7 + 5) = 0;
    *(u8 *)(var_7 + 6) = 0;
    *(s16 *)(var_7 + 0xE) = 0x6C;
    SetSemiTrans((TILE *)var_7, 1);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)var_7);
    var_7 += 0x10;

    SetTile((TILE *)var_7);
    *(s16 *)(var_7 + 8) = 0x92;
    *(s16 *)(var_7 + 0xA) = 0xAA;
    *(s16 *)(var_7 + 0xC) = 0x15C;
    *(u8 *)(var_7 + 4) = 0;
    *(u8 *)(var_7 + 5) = 0;
    *(u8 *)(var_7 + 6) = 0;
    *(s16 *)(var_7 + 0xE) = 0x1A;
    SetSemiTrans((TILE *)var_7, 1);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)var_7);

    SetDrawMode((void *)sp1C, 1, 0, 0, 0);
    AddPrim(g_gpu_ot_ptr + arg1, (void *)sp1C);
    sp1C += 0xC;
    return end_off - arg0;
}

/* .data 0x8009ADB4..0x8009B450, this file's .data (d_8A588.c's g_vab_id_list
 * and 4BC3C.c's D_8009AD1C precede; the data-only d_8BC50.c's
 * D_8009B450 .. follow). D_8009ADB4 is the 3-cell sheet header
 * whose cells are D_8009ADC0; D_8009ADD8 is 20 rows of three cells
 * (gpu_DrawPackedComparisonRowsAndTotals picks a row by g_working_record's bd0,
 * less 2 from 12; the old D_8009AF94 label inside row 18 was only sin_1's
 * folded rsin_tbl base), D_8009AFB8 20 x offsets (D_8009B0C0.unk_1C).
 * D_8009ADD8 must follow D_8009ADC0: gpu_DrawIndexedCellsAndDecimalDigitPairs
 * reaches row idx as D_8009B0C0.unk_10 + 3 + idx * 3, so the two read as one
 * table of a 3-cell group and 20 rows. */
Unk8009B0E0Record D_8009ADB4 = {0x1F, 0x00, 3, 0x00, 0x3C0, 0x1FF, 0x0, 0x77};
Unk8009B400Record D_8009ADC0[3] = {
    {48, 80, 0xB0, 0x00, 0x10, 0x0C},
    {64, 80, 0xC0, 0x00, 0x10, 0x0C},
    {80, 80, 0xD0, 0x00, 0x30, 0x0C},
};
Unk8009B400Record D_8009ADD8[20][3] = {
    {{0, 80, 0x00, 0x00, 0x20, 0x0C},
     {32, 80, 0x20, 0x00, 0x20, 0x0C},
     {64, 80, 0x40, 0x00, 0x0B, 0x0C}},
    {{0, 80, 0x00, 0x18, 0x20, 0x0C},
     {32, 80, 0x20, 0x18, 0x20, 0x0C},
     {64, 80, 0x40, 0x18, 0x0B, 0x0C}},
    {{0, 80, 0x00, 0x0C, 0x20, 0x0C},
     {32, 80, 0x20, 0x0C, 0x20, 0x0C},
     {64, 80, 0x40, 0x0C, 0x02, 0x0C}},
    {{0, 80, 0x00, 0x24, 0x20, 0x0C},
     {32, 80, 0x20, 0x24, 0x20, 0x0C},
     {64, 80, 0x40, 0x24, 0x16, 0x0C}},
    {{0, 80, 0x00, 0x30, 0x20, 0x0C},
     {32, 80, 0x20, 0x30, 0x20, 0x0C},
     {64, 80, 0x40, 0x30, 0x35, 0x0C}},
    {{0, 80, 0x4E, 0x54, 0x20, 0x0C},
     {32, 80, 0x6E, 0x54, 0x20, 0x0C},
     {64, 80, 0x8E, 0x54, 0x12, 0x0C}},
    {{0, 80, 0x80, 0x30, 0x20, 0x0C},
     {32, 80, 0xA0, 0x30, 0x20, 0x0C},
     {64, 80, 0xC0, 0x30, 0x10, 0x0C}},
    {{0, 80, 0x00, 0x3C, 0x20, 0x0C},
     {32, 80, 0x20, 0x3C, 0x20, 0x0C},
     {64, 80, 0x40, 0x3C, 0x37, 0x0C}},
    {{0, 80, 0x80, 0x00, 0x10, 0x0C},
     {16, 80, 0x90, 0x00, 0x10, 0x0C},
     {32, 80, 0xA0, 0x00, 0x10, 0x0C}},
    {{0, 80, 0x00, 0x48, 0x20, 0x0C},
     {32, 80, 0x20, 0x48, 0x20, 0x0C},
     {64, 80, 0x40, 0x48, 0x33, 0x0C}},
    {{0, 80, 0x4E, 0x60, 0x20, 0x0C},
     {32, 80, 0x6E, 0x60, 0x20, 0x0C},
     {64, 80, 0x8E, 0x60, 0x18, 0x0C}},
    {{0, 80, 0x00, 0x54, 0x10, 0x0C},
     {16, 80, 0x10, 0x54, 0x10, 0x0C},
     {32, 80, 0x20, 0x54, 0x10, 0x0C}},
    {{0, 80, 0x80, 0x48, 0x07, 0x0C},
     {7, 80, 0x87, 0x48, 0x07, 0x0C},
     {14, 80, 0x8E, 0x48, 0x09, 0x0C}},
    {{0, 80, 0x00, 0x6C, 0x20, 0x0C},
     {32, 80, 0x20, 0x6C, 0x20, 0x0C},
     {64, 80, 0x40, 0x6C, 0x0E, 0x0C}},
    {{0, 80, 0x80, 0x24, 0x10, 0x0C},
     {16, 80, 0x90, 0x24, 0x10, 0x0C},
     {32, 80, 0xA0, 0x24, 0x15, 0x0C}},
    {{0, 80, 0x80, 0x0C, 0x20, 0x0C},
     {32, 80, 0xA0, 0x0C, 0x20, 0x0C},
     {64, 80, 0xC0, 0x0C, 0x3A, 0x0C}},
    {{0, 80, 0x00, 0x60, 0x20, 0x0C},
     {32, 80, 0x20, 0x60, 0x20, 0x0C},
     {64, 80, 0x40, 0x60, 0x08, 0x0C}},
    {{0, 80, 0x80, 0x3C, 0x20, 0x0C},
     {32, 80, 0xA0, 0x3C, 0x20, 0x0C},
     {64, 80, 0xC0, 0x3C, 0x16, 0x0C}},
    {{0, 80, 0x80, 0x18, 0x20, 0x0C},
     {32, 80, 0xA0, 0x18, 0x20, 0x0C},
     {64, 80, 0xC0, 0x18, 0x36, 0x0C}},
    {{0, 80, 0x4E, 0x6C, 0x20, 0x0C},
     {32, 80, 0x6E, 0x6C, 0x20, 0x0C},
     {64, 80, 0x8E, 0x6C, 0x12, 0x0C}},
};
s16 D_8009AFB8[20] = {
    0x1B0, 0x1B0, 0x1B4, 0x1AA, 0x19B, 0x1AC, 0x1AD, 0x19A, 0x195, 0x19C,
    0x1A9, 0x1BD, 0x1CB, 0x1AE, 0x1BB, 0x198, 0x1B1, 0x1AA, 0x19B, 0x1AC,
};

/* The sheets D_8009B0C0 points at: three frame sheets (count 6 / 3 / 12, each
 * header followed by its cells; after D_8009AFE0's six cells come 8 zero
 * bytes, kept as a seventh, empty cell), the digit sheet D_8009B0B4 (its one
 * cell is the .sdata D_800A3264), then D_8009B0C0 itself. D_8009B0E0 is
 * gpu_DrawModeSizedPanelWithSelectedCellOpaque's table of 9 headers; D_8009B14C
 * / D_8009B158 two more. These cells sit in the header's cells[0] tail (a GNU
 * extension; .size says 12) because only the header is ever named, unlike
 * D_8009ADC0. */
Unk8009B0E0Record D_8009AFE0 = {
    0x1E,
    0x00,
    6,
    0xFF,
    0x3B0,
    0x1FE,
    0x0,
    0x0,
    {{145, 58, 0x00, 0x00, 0x40, 0x0C},
     {209, 58, 0x40, 0x00, 0x40, 0x0C},
     {273, 58, 0x80, 0x00, 0x40, 0x0C},
     {337, 58, 0xC0, 0x00, 0x40, 0x0C},
     {401, 58, 0x00, 0x0C, 0x40, 0x0C},
     {465, 58, 0x40, 0x0C, 0x1E, 0x0C},
     {0, 0, 0x00, 0x00, 0x00, 0x00}},
};
Unk8009B0E0Record D_8009B024 = {
    0x1E,
    0x00,
    3,
    0xFF,
    0x390,
    0x1FF,
    0x0,
    0x28,
    {{107, 80, 0x00, 0x00, 0x15, 0x0C},
     {107, 106, 0x15, 0x00, 0x15, 0x0C},
     {107, 132, 0x2A, 0x00, 0x15, 0x0C}},
};
Unk8009B0E0Record D_8009B048 = {
    0x1E,
    0x00,
    12,
    0xFF,
    0x380,
    0x1FF,
    0xFA,
    0x1A,
    {{230, 78, 0x00, 0x00, 0x03, 0x04},
     {286, 78, 0x00, 0x00, 0x03, 0x04},
     {291, 78, 0x00, 0x00, 0x03, 0x04},
     {230, 104, 0x00, 0x00, 0x03, 0x04},
     {286, 104, 0x00, 0x00, 0x03, 0x04},
     {291, 104, 0x00, 0x00, 0x03, 0x04},
     {230, 130, 0x00, 0x00, 0x03, 0x04},
     {286, 130, 0x00, 0x00, 0x03, 0x04},
     {291, 130, 0x00, 0x00, 0x03, 0x04},
     {209, 176, 0x00, 0x00, 0x03, 0x04},
     {265, 176, 0x00, 0x00, 0x03, 0x04},
     {270, 176, 0x00, 0x00, 0x03, 0x04}},
};
Unk8009B0E0Record D_8009B0B4 = {0x1E, 0x00, 1, 0xFF, 0x380, 0x1FF, 0x0, 0x1A};
extern Unk8009B400Record D_800A3264;
Unk8006D808Rec D_8009B0C0 = {
    {&D_8009AFE0, &D_8009B024, &D_8009B048},
    &D_8009ADB4,
    D_8009ADC0,
    &D_8009B0B4,
    &D_800A3264,
    D_8009AFB8,
};
Unk8009B0E0Record D_8009B0E0[9] = {
    {0x1F, 0x00, 2, 0x00, 0x3D0, 0x1FD, 0xA0, 0x1C},
    {0x1F, 0x00, 2, 0x00, 0x3D0, 0x1FD, 0xA0, 0x22},
    {0x1F, 0x00, 2, 0x00, 0x3D0, 0x1FD, 0xA0, 0x2A},
    {0x1F, 0x00, 3, 0x00, 0x3D0, 0x1FC, 0x0, 0x50},
    {0x1F, 0x00, 2, 0x00, 0x3E0, 0x1FC, 0x38, 0x6A},
    {0x1F, 0x00, 2, 0x00, 0x3E0, 0x1FC, 0x95, 0x50},
    {0x1F, 0x00, 4, 0x00, 0x3E0, 0x1FC, 0x0, 0x0},
    {0x1F, 0x00, 4, 0x00, 0x3E0, 0x1FC, 0x0, 0x0},
    {0x1F, 0x00, 6, 0x00, 0x3E0, 0x1FC, 0x0, 0x0},
};
Unk8009B0E0Record D_8009B14C = {0x1F, 0x00, 4, 0x00, 0x3E0, 0x1FC, 0x0, 0x0};
Unk8009B0E0Record D_8009B158 = {0x1F, 0x00, 1, 0x00, 0x3C0, 0x1FF, 0x0, 0x0};

/* gpu_DrawModeSizedPanelWithSelectedCellOpaque's cell tables D_8009B164 /
 * D_8009B184 .. D_8009B29C; D_8009B2AC, the cell-table pointers it picks by j
 * (D_8009B1AC twice, D_8009B1CC, D_8009B1EC; the last two have no other
 * reader); g_panel_frame_sizes, the frame sizes per mode. */
Unk8009B400Record D_8009B164[2][2] = {
    {{0, 0, 0x00, 0x00, 0x10, 0x06}, {0, 0, 0x10, 0x00, 0x10, 0x06}},
    {{0, -6, 0x00, 0x00, 0x10, 0x08}, {0, -6, 0x10, 0x00, 0x10, 0x08}},
};
Unk8009B400Record D_8009B184[2] = {
    {0, -4, 0x00, 0x00, 0x10, 0x0A},
    {0, -4, 0x10, 0x00, 0x10, 0x0A},
};
Unk8009B400Record D_8009B194[3] = {
    {247, 3, 0x00, 0x00, 0x40, 0x0C},
    {311, 3, 0x40, 0x00, 0x40, 0x0C},
    {375, 3, 0x80, 0x00, 0x14, 0x0C},
};
Unk8009B400Record D_8009B1AC[2] = {
    {265, 18, 0x00, 0x00, 0x40, 0x0C},
    {329, 18, 0x40, 0x00, 0x2D, 0x0C},
};
Unk8009B400Record D_8009B1BC[2] = {
    {266, 4, 0x00, 0x00, 0x40, 0x0C},
    {330, 4, 0x40, 0x00, 0x2B, 0x0C},
};
Unk8009B400Record D_8009B1CC[4] = {
    {200, 19, 0x00, 0x5D, 0x40, 0x0C},
    {264, 19, 0x40, 0x5D, 0x35, 0x0C},
    {317, 19, 0x00, 0x6A, 0x39, 0x0C},
    {374, 19, 0xA9, 0x5D, 0x41, 0x0C},
};
Unk8009B400Record D_8009B1EC[4] = {
    {203, 34, 0x00, 0x5D, 0x40, 0x0C},
    {267, 34, 0x40, 0x5D, 0x40, 0x0C},
    {331, 34, 0x80, 0x5D, 0x40, 0x0C},
    {395, 34, 0xC0, 0x5D, 0x2A, 0x0C},
};
Unk8009B400Record D_8009B20C[6] = {
    {159, 165, 0x00, 0x9C, 0xC0, 0x0D}, {351, 165, 0xC0, 0x9C, 0x40, 0x0D},
    {415, 165, 0x00, 0xA9, 0x42, 0x0D}, {182, 178, 0x00, 0xB6, 0xC0, 0x0D},
    {374, 178, 0xC0, 0xB6, 0x40, 0x0D}, {438, 178, 0x00, 0xC3, 0x13, 0x0D},
};
Unk8009B400Record D_8009B23C[12] = {
    {180, 64, 0x00, 0x8F, 0x40, 0x0C}, {244, 64, 0x40, 0x8F, 0x18, 0x0C},
    {284, 64, 0x4E, 0xA9, 0x32, 0x0C}, {334, 64, 0x80, 0xA9, 0x1A, 0x0C},
    {180, 79, 0x5B, 0x8F, 0x40, 0x0C}, {244, 79, 0x9B, 0x8F, 0x28, 0x0C},
    {284, 79, 0x4E, 0xA9, 0x32, 0x0C}, {334, 79, 0x80, 0xA9, 0x1A, 0x0C},
    {180, 94, 0x9C, 0xA9, 0x0C, 0x0D}, {192, 94, 0xA8, 0xA9, 0x0C, 0x0D},
    {204, 94, 0xB4, 0xA9, 0x0C, 0x0D}, {216, 94, 0xC0, 0xA9, 0x1D, 0x0D},
};
Unk8009B400Record D_8009B29C[2] = {
    {430, 63, 0xDC, 0x2A, 0x1E, 0x0C},
    {380, 63, 0xC0, 0x2A, 0x1C, 0x0C},
};
Unk8009B400Record *D_8009B2AC[4] = {
    D_8009B1AC,
    D_8009B1AC,
    D_8009B1CC,
    D_8009B1EC,
};
Unk8009B2BCRecord g_panel_frame_sizes[3] = {
    {258, 65},
    {258, 51},
    {328, 155},
};

/* gpu_DrawFixedSpritePairWithSelectedSheets /
 * gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle's header rows D_8009B2C8 and
 * their cells D_8009B340 / D_8009B358 / D_8009B388; the sheet headers
 * D_8009B398 that gpu_DrawTimeFieldsAndDecimalCounter ..
 * gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells draw with, the cells
 * D_8009B3C8, and the ten digit cells D_8009B400. */
Unk8009B0E0Record D_8009B2C8[2][5] = {
    {
        {0x1F, 0x00, 3, 0x00, 0x3C0, 0x1FE, 0x0, 0x64},
        {0x1F, 0x00, 6, 0x00, 0x3D0, 0x1FE, 0x0, 0x92},
        {0x1F, 0x00, 1, 0x00, 0x3E0, 0x1FE, 0x90, 0x92},
        {0x1F, 0x00, 1, 0x00, 0x3E0, 0x1FE, 0x90, 0xA6},
        {0x1F, 0x00, 1, 0x00, 0x3E0, 0x1FE, 0x9E, 0xA6},
    },
    {
        {0x1E, 0x00, 3, 0x00, 0x380, 0x1FE, 0x0, 0x64},
        {0x1E, 0x00, 6, 0x00, 0x390, 0x1FE, 0x0, 0x92},
        {0x1E, 0x00, 1, 0x00, 0x3A0, 0x1FE, 0x90, 0x92},
        {0x1E, 0x00, 1, 0x00, 0x3A0, 0x1FE, 0x90, 0xA6},
        {0x1E, 0x00, 1, 0x00, 0x3A0, 0x1FE, 0x9E, 0xA6},
    },
};
Unk8009B400Record D_8009B340[3] = {
    {329, 170, 0x00, 0x00, 0x40, 0x2E},
    {393, 170, 0x40, 0x00, 0x40, 0x2E},
    {457, 170, 0x80, 0x00, 0x34, 0x2E},
};
Unk8009B400Record D_8009B358[6] = {
    {476, 134, 0x00, 0x00, 0x10, 0x40}, {492, 134, 0x10, 0x00, 0x40, 0x40},
    {556, 134, 0x50, 0x00, 0x40, 0x40}, {476, 198, 0x00, 0x40, 0x10, 0x18},
    {492, 198, 0x10, 0x40, 0x40, 0x18}, {556, 198, 0x50, 0x40, 0x40, 0x18},
};
Unk8009B400Record D_8009B388[2] = {
    {0, 0, 0x00, 0x00, 0x24, 0x14},
    {0, 0, 0x00, 0x00, 0x0E, 0x06},
};
Unk8009B0E0Record D_8009B398[4] = {
    {0x1F, 0x00, 1, 0x00, 0x3C0, 0x1FD, 0x0, 0x0},
    {0x1F, 0x00, 1, 0x00, 0x3E0, 0x1FD, 0x0, 0x0},
    {0x1F, 0x00, 1, 0x00, 0x3C0, 0x1FE, 0x0, 0x0},
    {0x1F, 0x00, 1, 0x00, 0x3D0, 0x1FE, 0x0, 0x0},
};
Unk8009B400Record D_8009B3C8[7] = {
    {459, 22, 0xFC, 0x00, 0x04, 0x05}, {513, 22, 0xFC, 0x00, 0x04, 0x05},
    {508, 22, 0xFC, 0x00, 0x04, 0x05}, {565, 23, 0xC0, 0x1C, 0x3C, 0x0E},
    {565, 42, 0xC0, 0x2A, 0x3C, 0x0E}, {568, 0, 0xC0, 0xDB, 0x23, 0x01},
    {603, 0, 0xE3, 0xDB, 0x1D, 0x01},
};
Unk8009B400Record D_8009B400[10] = {
    {0, 0, 0xA0, 0x00, 0x13, 0x0E}, {0, 0, 0xB3, 0x00, 0x0F, 0x0E},
    {0, 0, 0xC2, 0x00, 0x13, 0x0E}, {0, 0, 0xD5, 0x00, 0x13, 0x0E},
    {0, 0, 0xE8, 0x00, 0x13, 0x0E}, {0, 0, 0xA0, 0x0E, 0x13, 0x0E},
    {0, 0, 0xB3, 0x0E, 0x13, 0x0E}, {0, 0, 0xC6, 0x0E, 0x13, 0x0E},
    {0, 0, 0xD9, 0x0E, 0x13, 0x0E}, {0, 0, 0xEC, 0x0E, 0x13, 0x0E},
};

/* This file's initialized small data (.sdata), in address order (Q65). */
/* D_8009B0C0.unk_18, the one cell of the digit sheet D_8009B0B4; this file's
 * by layout, between its gp-reached objects (Q80) */
Unk8009B400Record D_800A3264 = {0xB3, 0x4E, 0x00, 0x00, 0x18, 0x0C};
s32 D_800A326C = 0;
u8 D_800A3270[8] = {0, 1, 2, 3, 4, 0, 0, 0};
s32 D_800A3278 = 0;
RECT D_800A327C = {0x380, 0, 0x40, 0x100};
RECT D_800A3284 = {0x3E0, 0x1FF, 0x10, 1};
Unk8009B400Record D_800A328C = {0, 0, 0, 0, 0x14, 0xE};
RECT D_800A3294 = {0x3C0, 0x180, 0x40, 0x16};
RECT D_800A329C = {0x3C0, 0x1FF, 0x10, 1};
RECT D_800A32A4 = {0x380, 0x17F, 0x40, 0x7F};
RECT D_800A32AC = {0x380, 0x1FF, 0x20, 1};
u16 D_800A32B4 = 0;
u16 D_800A32B6 = 0;
