#ifndef GAME_H
#define GAME_H

/* Game types for the translation units in src/main/ (SLUS_006.63's game code):
 * records, tables and object layouts. Most multi-TU declarations live in
 * bb2.h, which includes this file; some shared externs are still declared
 * locally. Sony's library types come from include/psxsdk/. */

#include "common.h"
#include <psxsdk/kernel.h>
#include <psxsdk/libapi.h>
#include <psxsdk/libc.h>
#include <psxsdk/libcard.h>
#include <psxsdk/libcd.h>
#include <psxsdk/libcomb.h>
#include <psxsdk/libetc.h>
#include <psxsdk/libgpu.h>
#include <psxsdk/libgte.h>
#include <psxsdk/libsnd.h>
#include <psxsdk/libspu.h>

/* The 0x24-byte file record at 0x80106A50 (memcard_BuildSaveBlock checksums it
 * as one block; InitSavedRecordAndResetCameraDistanceOffset initialises it). */
typedef struct {
    u8 holder_character_index;
    u8 holder_weapon_id;
    s32 time_frames;
} TimeRec;

typedef struct {
    s32 unlock_mask; /* 0x80106A50 */
    u8 unk_04;       /* 0x80106A54 */
    u8 unk_05[3];
    TimeRec times[3]; /* 0x80106A58 */
    u8 frec20[3];     /* 0x80106A70 */
    /* 0x80106A73: bits 0/1/2: gpu_GetSavedGrayscaleFlag,
     * fighter_GetSavedCpuBehaviorAdjustFlag, snd_GetSavedCdAudioSuppressFlag */
    u8 flags;
} FileRecord;

/* The 0x100-byte save block after the memory-card header (D_800F34D8 = the
 * 0x200-byte card buffer + 0x100): memcard_BuildSaveBlock writes three copies
 * of g_saved_record with their byte checksums and clears the pointer / value
 * table; memcard_RestoreSaveBlock restores the first copy whose checksum holds
 * (unless its flags bit 7 is set) and writes each val[j] through ptr[j] when
 * ptr[j] is a KSEG0 RAM address. */
typedef struct Unk800F34D8Save {
    FileRecord rec[3]; /* +0x00 */
    s32 sum[3];        /* +0x6C byte sums of rec[] */
    u16 *ptr[0x16];    /* +0x78 */
    u16 val[0x16];     /* +0xD0 */
    s32 unk_FC;
} Unk800F34D8Save; /* sizeof == 0x100 */

/* The game's display double buffer at 0x800F7438: two 0x4090-byte records, one
 * per frame parity (g_gpu_presented_frame_count & 1). disp_Init /
 * gpu_BeginFullscreenImageDisplay pass +0x00 to SetDefDrawEnv and +0x5C to
 * SetDefDispEnv for each record; main clears +0x70 with ClearOTagR(ot, 0x1008)
 * (0x1008 words = 0x4020 bytes, which ends the record) and draws it from its
 * last entry, DrawOTag(+0x408C). */
typedef struct {
    DRAWENV draw;   /* +0x00 */
    DISPENV disp;   /* +0x5C */
    u32 ot[0x1008]; /* +0x70 ordering table */
} GpuDb;            /* 0x4090 */

/* The record at g_gpu_vram_scrolls (0x134 bytes). */
typedef struct {
    s32 phase; /* +0x000 */
    /* +0x004: one bank per frame parity, two packets per level */
    DR_MOVE move[2][6];
    /* +0x124: filled from g_gpu_vram_scroll_presets by gpu_InitVramScroll */
    s16 ctl[7];
} Unk800EF848Rec;

/* 3-word record table at 0x800F1198, terminated by an all-zero record
 * (eff_SetBillboardList walks it with a 12-byte stride). */
typedef struct {
    s32 packed_x;
    s32 packed_y;
    s32 z_coord;
} Unk800F1198Record;

/* 0x800A9CF8 record (0x18 bytes); node_InitFrameEntriesFromWorldTranslations
 * reaches its fields off one base register, and
 * resource_RebaseFrameAnimatedEntryBlockAndPointers bumps the frame_data /
 * frame_entries pointers together.
 */
typedef struct {
    s16 reloc_block_id;
    s16 anim_frame_count;
    /* = resource_GetLoadedSelector() (g_loaded_resource_selector) */
    s16 resource_selector;
    s16 frame_entry_count; /* entry count */
    s32 frame_data;
    s32 frame_entries; /* entry table (stride 0x68) */
    /* gpu_GetGridNodeRecordTable() table (stride 0x68) */
    s32 paired_grid_nodes;
    s32 unk14;
} Unk800A9CF8Rec;

/* Per-lane slot record table at 0x800F0EC8: 2 lanes x 10 slots x one 3-word
 * record (12 bytes; lane stride 120). eff_AllocateOrRecycleYawedQuadSlot writes
 * it, eff_DrawYawedSlotQuads reads it. The slot bitmask
 * g_eff_yawed_quad_slot_masks[n] and the sibling SVECTOR table
 * g_eff_yawed_quad_slot_rotations[n][10] index the same pair (n, slot). */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0EC8Record;

/* Effect-slot position table at 0x800F0E38: 12 slots x one 3-word {x, y, z}
 * record (12 bytes; 0x800F0E38 + 12 * 12 == 0x800F0EC8, the table above).
 * eff_SpawnBillboardPair spawns into it, eff_DrawAndAdvanceBillboardPairs draws
 * from it. The slot bitmask g_eff_billboard_pair_slot_mask and the age table
 * g_eff_billboard_pair_ages[slot] index the same slot. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0E38Record;

/* 0x8009BC0C: the eight option rows of the list menu menu_UpdateRowMenu drives,
 * one {state, mode} byte pair per row (0x8009BC0C..0x8009BC1B), indexed by
 * g_menu_packed_cursors & 0xF. state: accept / reject / return value; mode:
 * passed to the three render calls, 1/2/3 select the
 * g_row_menu_working_record_ptr bit-3 action.
 */
typedef struct {
    u8 state;
    u8 mode;
} MenuOption;

/* 0x8009BC04: the list menu's state -- the rows switched on (one bit per
 * row; menu_SetEnabledRows sets them), a second row mask menu_UpdateRowMenu
 * sets from the cursor row (menu_DrawMaskedRowList scans it for the first row
 * drawn), then the eight option rows. */
typedef struct {
    s32 rows_on;
    s32 unk4;
    MenuOption opt[8];
} MenuState;

/* 0x8009BCF8: 2 pages x 10 two-byte records (0x8009BCF8..0x8009BD1F;
 * g_sel_sheet_code_pairs follows), one page per character-select page, 2 rows x
 * 5 columns of cells (cell = row * 5 + col). menu_UpdateSelGridPicks /
 * menu_DrawSelGrid index it [page][cell]; menu_FadeOutSelAndWriteResult reads
 * unk1 flat. Data: unk1 is 0x00..0x09 (page 0) then 0x0C..0x15 (page 1). */
typedef struct {
    u8 unk0;
    u8 unk1;
} Unk8009BCF8Record;

/* The 0x2C-byte record gpu_InitPrimBufferPair fills, two at a time (596CC /
 * 5ED34 / 64FD8 keep the pair at the start of their work area; getters `base +
 * i * 44`). Each word holds the start of a region gpu_InitPrimBufferPair carves
 * from its buffer: +0x00 0x9C40 bytes, +0x04 0x5DC0, +0x08 0x438, +0x0C 0x640,
 * +0x10 0x1B58, +0x14 0x258, +0x18 0x78, +0x1C 0x78, +0x20 0x110; +0x24 / +0x28
 * it leaves alone. The 5ED34 / 64FD8 draw contexts copy them into
 * Unk800788B0Rec; 596CC's gpu_AdvanceBufferCounterAndBuildDrawContext copies
 * them into the s32 words of its context array. A pool member and the cursor
 * member of the same primitive kind share one *_prims name
 * (textured_quad_prims, flat_quad_prims, gradient_quad_prims, tile_prims,
 * sprite_prims, draw_area_prims, draw_offset_prims), but tile and sprite swap
 * places: the pool has tile_prims at +0x0C and sprite_prims at +0x10, the
 * cursor sprite_prims at +0x0C and tile_prims at +0x10. unk_14 stays unnamed:
 * the cursor's is a DR_MODE *, but 5ED34's two gpu_AddThreeGreyTilesStackedUp
 * calls pass the Unk8006EACCRec, whose +0x18 is that word, so there it also
 * receives three TILEs per call. */
typedef struct {
    s32 textured_quad_prims;
    void *flat_quad_prims;
    void *gradient_quad_prims;
    void *tile_prims;
    s32 sprite_prims;
    void *unk_14;
    void *draw_area_prims;
    void *draw_offset_prims;
    void *unk_20;
    s32 unk_24;
    s32 unk_28;
} Unk8006E49CRec;

/* The eight primitive cursors of a draw context (StepScrollingSpriteDisplay
 * fills one bare; Unk8006EACCRec embeds it as primitive_cursors). The draw
 * functions build primitives at a cursor and advance it. textured_quad_prims /
 * sprite_prims hold the s32 cursors gpu_AddScaledSpriteQuads /
 * gpu_AddRotatedSpriteQuads and gpu_AddSpriteCells return. unk_14 is typed
 * DR_MODE *, but 5ED34's two gpu_AddThreeGreyTilesStackedUp calls (which read
 * +0x18 of the Unk8006EACCRec) also build TILEs at it, so it is not a
 * single-kind cursor. */
typedef struct {
    s32 textured_quad_prims;
    POLY_F4 *flat_quad_prims;
    POLY_G4 *gradient_quad_prims;
    s32 sprite_prims;
    TILE *tile_prims;
    DR_MODE *unk_14;
    DR_AREA *draw_area_prims;
    DR_OFFSET *draw_offset_prims;
} Unk800788B0Rec;

/* The VAB pack snd_LoadVabPack loads (MOD.BIN's head sound_pack points at one;
   g_vab_rec_ptr keeps the loaded ones): four words, the first three
   file-relative offsets snd_LoadVabPack turns into addresses in place.
   key_on_table: the u32 key-event table snd_FlushSoundQueue reads; vab_header:
   the VabHdr snd_VabOpen opens; vab_body: the body snd_VabOpen transfers;
   vab_body_size: the body's size in SPU memory. */
typedef struct {
    s32 key_on_table;
    s32 vab_header;
    s32 vab_body;
    s32 vab_body_size;
} Unk8005C2A8Pack;

/* The 8-byte sprite cell: x / y, its offset from the descriptor's screen
 * position; u / v, its texel offset from the sheet's ubase / vbase; w / h, its
 * size (gpu_AddSpriteCells: x0 = x + the descriptor's x, u0 = u + ubase, w / h
 * copied). A sheet's cells follow its header(s) (Unk8009B0E0Record.cells). The
 * 0x8009B400 tables hold such cells: 0x8009B400..0x8009B44F is 10 records, one
 * per digit (gpu_DrawTimeFieldsAndDecimalCounter /
 * gpu_DrawTwoDigitCounterWithOptionalSecondSide index it with a 0..9 value),
 * and 0x8009B458..0x8009B487 6 more
 * (gpu_DrawTwoDigitCounterWithOptionalSecondSide);
 * g_gpu_time_field_line_origins and D_8009B488 follow. */
typedef struct {
    s16 x;
    s16 y;
    u8 u;
    u8 v;
    u8 w;
    u8 h;
} Unk8009B400Record;

/* The 12-byte sprite-sheet header the walkers read (Unk8007352CRec.header): tp0
 * / tp1, the texture-page bits (gpu_AddScaledSpriteQuads: (tp0 & 0xFE1F) + (tp1
 * << 7), gpu_GetSheetTPage); count, the cell count; cx / cy, the CLUT position
 * (GetClut); ubase / vbase, the texel origin the cells' u / v add to; cells,
 * the cell table that follows. A resource sheet may carry two or three headers
 * (the plain one, then one highlight per player: menu_UpdateEntryGrid,
 * menu_DrawSelGrid) ahead of one shared cell table, which then starts at the
 * last header's cells. 0x8009B0E0 is a table of 9 such headers
 * (0x8009B0E0..0x8009B14B) that gpu_DrawModeSizedPanelWithSelectedCellOpaque
 * reaches off one base; D_8009B14C and D_8009B158 are two more of the same
 * shape. */
typedef struct {
    u8 tp0;
    u8 tp1;
    u8 count;
    u8 unk3;
    u16 cx;
    u16 cy;
    u16 ubase;
    u16 vbase;
    Unk8009B400Record cells[0];
} Unk8009B0E0Record;

/* The head of the resource files
 * gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip loads. By
 * g_cd_file_table's sizes, file 2 of cdrom_GetFileIndex's group 2 is MOD.BIN, 3
 * SEL.BIN, 4 / 5 SEL1 / SEL2.BIN, 6 D_SEL.BIN, 0x32 NAR.BIN. Each file starts
 * with a list of file-relative offsets ending in -1, which
 * reloc_RelocateSelfOffsets turns into addresses. The first five mean the same
 * in every file: reloc_RelocateTwelveListsAndLoadVabPack /
 * resource_RelocateNineOffsetListsAndLoadVabPack pass sound_pack and
 * free_ram_start to the VAB loader snd_LoadVabPack (the pack exists only in
 * MOD and SEL / SEL1 / SEL2; menu_RelocateSelLists /
 * reloc_RelocateFiveOffsetListsAndGetWord1, for D_SEL / NAR, make no such call,
 * and in D_SEL, NAR and STAFF the words unk_00 (sound_pack), unk_04
 * (free_ram_start) and unk_08 (vram_image_pixels) are equal, so the pack range
 * is empty),
 * gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip loads the image at
 * vram_image_pixels into VRAM, and gpu_LoadVramBottomStrip loads the 640x32
 * strip at unk_0C or unk_10. The four per-file relocators
 * (reloc_RelocateTwelveListsAndLoadVabPack,
 * resource_RelocateNineOffsetListsAndLoadVabPack, menu_RelocateSelLists,
 * reloc_RelocateFiveOffsetListsAndGetWord1) return free_ram_start, where the
 * caller's gpu_InitPrimBufferPair buffer starts: it equals the image start, RAM
 * reused after the VRAM upload. The words after the head differ per file. */
typedef struct {
    Unk8005C2A8Pack *sound_pack;
    s32 free_ram_start;
    s32 vram_image_pixels;
    s32 unk_0C;
    s32 unk_10;
} Unk8006E950Head;

/* STAFF.BIN (resource file 0x5F), the root 64FD8's InitScrollingSpriteDisplay
 * loads at its work area + 0x58 (g_gpu_scrolling_sprite_resources). After the
 * head, one offset list (relocated with the head by reloc_RelocateSelfOffsets,
 * so it ends in -1 like the head's own list): the sprite sheets
 * gpu_DrawScrollingAndDelayedSheets draws: unk_14[10] first, then unk_14[0]
 * onward while the next entry is not -1. */
typedef struct {
    Unk8006E950Head resource_head;
    Unk8009B0E0Record *unk_14[0];
} Unk80078824Rec;

/* 12-byte rectangle rows {x, y, w, h, r, g, b}: MOD.BIN's rect_rows
 * (menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads draws them) and
 * D_SEL.BIN's row_tile_rects (menu_DrawSelRowTiles). */
typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;

/* SEL.BIN's selected_entry_header_block, gpu_UploadAndDrawSelectedEntryQuads's:
 * two cell tables (unk_00[slot_index], one per player), a 0 word, then the
 * 12-byte sheet headers the cells are drawn with (unk_0C[v] for the
 * g_entry_menu_grid_entries entry value v it reads; after its first iteration
 * the values 12, 13, 14, 0 and 3 take unk_0C[22..26]). */
typedef struct {
    Unk8009B400Record *unk_00[2];
    s32 unk_08;
    Unk8009B0E0Record unk_0C[0];
} Unk8006ECF4Rec;

/* SEL.BIN / SEL1.BIN / SEL2.BIN (resource files 3-5), the root 5ED34's
 * InitDisplayStateLoadPackAndSetEntryFlags loads at its work area + 0x58
 * (g_entry_menu_resources; StepSlideBuildFrameRecordAndDispatchStateHandler
 * hands it to the handlers as Unk8006EACCRec.resource_root). After the head:
 * - row_choice_pixels: indexed by the row-choice value (rec.row_pick) that
 * menu_UpdateRowCursors stores, two pixel-run addresses (headerless, not TIM),
 * one per column, loaded with LoadImage.
 * - selected_entry_header_block..unk_74: the nine lists
 * resource_RelocateNineOffsetListsAndLoadVabPack relocates
 * (reloc_RelocateOffsetList): selected_entry_header_block, the header block
 * gpu_UploadAndDrawSelectedEntryQuads draws (Unk8006ECF4Rec); unk_58..unk_74,
 * sprite-sheet lists. unk_78 is a further offset no code reads.
 * - row_choice_upload_rects: per player a row of eight VRAM RECTs
 * menu_UpdateRowCursors hands LoadImage as the destination; unk_80: the bytes
 * menu_UpdateChoiceGrid reads.
 * - entry_upload_pixels: five pixel-run addresses (headerless, not TIM)
 * gpu_UploadAndDrawSelectedEntryQuads loads with LoadImage. */
typedef struct {
    Unk8006E950Head resource_head;
    s32 row_choice_pixels[8][2];
    Unk8006ECF4Rec *selected_entry_header_block;
    Unk8009B0E0Record **unk_58;
    Unk8009B0E0Record **unk_5C;
    Unk8009B0E0Record **unk_60;
    Unk8009B0E0Record **unk_64;
    Unk8009B0E0Record **unk_68;
    Unk8009B0E0Record **unk_6C;
    Unk8009B0E0Record **unk_70;
    Unk8009B0E0Record **unk_74;
    s32 *unk_78;
    RECT *row_choice_upload_rects;
    u8 *unk_80;
    s32 entry_upload_pixels[5];
} Unk8006EA28Rec;

/* D_SEL.BIN (resource file 6), the root 64FD8's menu_InitSelScreen loads at its
 * work area + 0x58 and keeps in SelWork.f04 (menu_StepSelScreen hands it to the
 * handlers as Unk8006EACCRec.resource_root). After the head: unk_14..unk_38,
 * the ten lists menu_RelocateSelLists relocates, each a list of sprite sheets
 * (pick_slot_sheets is indexed by SelWork.f65); row_tile_rects, the rectangle
 * rows menu_DrawSelRowTiles draws.
 */
typedef struct {
    Unk8006E950Head resource_head;
    Unk8009B0E0Record **unk_14;
    Unk8009B0E0Record **sel_panel_sheets;
    Unk8009B0E0Record **unk_1C;
    Unk8009B0E0Record **pick_slot_sheets[3];
    Unk8009B0E0Record **opposed_choice_sheets;
    Unk8009B0E0Record **unk_30;
    Unk8009B0E0Record **unk_34;
    Unk8009B0E0Record **unk_38;
    Rec_8006C21C *row_tile_rects;
} Unk80076FF8Rec;

typedef struct {
    s16 on, off;
} Unk80077D94Entry;

/* NAR.BIN (resource file 0x32), the root 64FD8's InitTimedSpriteSequence loads
 * at its work area + 0x58 (g_gpu_timed_sprite_resources). After the head:
 * table, the cell table gpu_DrawAndAdvanceTimedSpriteSequence draws the
 * hdr18..hdr24 sheets with; hdr18..hdr28, the five sheet lists
 * reloc_RelocateFiveOffsetListsAndGetWord1 relocates; nar2C, in30 / out34 and
 * the five portrait TIMs img38 gpu_DrawAndAdvanceTimedSpriteSequence uploads.
 */
typedef struct {
    Unk8006E950Head resource_head;
    Unk8009B400Record *table;
    Unk8009B0E0Record **hdr18;
    Unk8009B0E0Record **hdr1C;
    Unk8009B0E0Record **hdr20;
    Unk8009B0E0Record **hdr24;
    Unk8009B0E0Record **hdr28;
    Unk80077D94Entry *nar2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

/* Draw context filled by 5ED34 StepSlideBuildFrameRecordAndDispatchStateHandler
 * and 64FD8 menu_StepSelScreen / StepTimedSpriteSequenceWithSkip: resource_root
 * (SEL for StepSlideBuildFrameRecordAndDispatchStateHandler's
 * handlers, D_SEL for menu_StepSelScreen's; StepTimedSpriteSequenceWithSkip
 * leaves it unset), then primitive_cursors; unk_24 (pool word +0x20) only
 * StepSlideBuildFrameRecordAndDispatchStateHandler / menu_StepSelScreen set. */
typedef struct {
    union {
        Unk8006EA28Rec *v8006EA28;
        Unk80076FF8Rec *v80076FF8;
    } resource_root;

    Unk800788B0Rec primitive_cursors;
    void *unk_24;
} Unk8006EACCRec;

/* The draw chunk 4D0A8 gpu_DrawModeSizedPanelWithSelectedCellOpaque builds at
 * the address its caller passes: TILEs from +0 up to +0xF0 (15; it writes at
 * most 13), SPRTs from +0xF0 up to +0x4D8 (50; gpu_AddSpriteCells's cursor),
 * then the DR_MODE at +0x4D8. It returns the chunk size, 0x4F0; the last 0xC
 * bytes are never written. */
typedef struct {
    TILE unk_00[15];
    SPRT unk_F0[50];
    DR_MODE unk_4D8;
    u8 unk_4E4[0xC];
} Unk8005C8A8Rec;

/* The draw chunk 4D0A8 gpu_DrawTimeFieldsAndDecimalCounter,
 * gpu_DrawTwoDigitCounterWithOptionalSecondSide and
 * gpu_DrawPairedMarkRowsAndTimeFields each build at the address their caller
 * passes (one layout): TILEs from +0 up to +0xA0 (10; they write at most 2, 2
 * and 4), SPRTs from +0xA0 up to +0x2F8 (30; gpu_AddSpriteCells's cursor), then
 * the DR_MODE at +0x2F8. Each returns the chunk size, 0x304.
 * gpu_DrawPairedMarkRowsAndTimeFields also writes a second DR_MODE at the
 * chunk's end, outside the chunk. */
typedef struct {
    TILE unk_00[10];
    SPRT unk_A0[30];
    DR_MODE unk_2F8;
} Unk8005D814Rec;

/* The draw chunk 4D0A8 gpu_DrawPackedComparisonRowsAndTotals builds at the
 * address its caller passes: TILEs from +0 up to +0xA0 (10; it writes 3), SPRTs
 * from +0xA0 up to +0x898 (102; gpu_AddSpriteCells's cursor), POLY_FT4s from
 * +0x898 up to +0xBB8 (20; gpu_AddScaledSpriteQuads's cursor), then the DR_MODE
 * at +0xBB8. It returns the chunk size, 0xBC4, and also writes a second DR_MODE
 * at the chunk's end, outside the chunk. */
typedef struct {
    TILE unk_00[10];
    SPRT unk_A0[102];
    POLY_FT4 unk_898[20];
    DR_MODE unk_BB8;
} Unk8005E54CRec;

/* The draw chunk 4D0A8 gpu_DrawImageAndAdvanceSplitWipe builds at the address
 * its caller passes: SPRTs from +0 (gpu_AddSpriteCells's cursor), the DR_MODE
 * at +0x280, two POLY_G4 at +0x28C (at most one per pass) and three DR_AREA at
 * +0x2D4 (the first, then at most one per pass).
 * gpu_DrawImageAndAdvanceSplitWipe returns the chunk size, 0x2F8. The 32 SPRTs
 * are the most it writes: gpu_AddSpriteCells writes at most one per cell of the
 * sheet header it is given (cell count: the byte at +2), and each of the two
 * passes draws D_8009B698's header and the one 12 bytes after it (8 cells
 * each). */
typedef struct {
    SPRT unk_00[32];
    DR_MODE unk_280;
    POLY_G4 unk_28C[2];
    DR_AREA unk_2D4[3];
} Unk8005FC9CRec;

/* The draw chunk 4D0A8 gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite
 * builds at the address its caller passes: 0xB4 bytes of SPRTs from +0
 * (gpu_AddSpriteCells's cursor), then the DR_MODE at +0xB4.
 * gpu_DrawOneOrTwoDigitValueAndDigitCountShiftedSprite returns the chunk size,
 * 0xC0. At most 5 SPRTs are written: gpu_AddSpriteCells writes at most one per
 * cell of the sheet header it is given (cell count: the byte at +2);
 * D_8009B6F0's header holds 3, and D_8009B6FC's holds 1, drawn once per digit
 * for up to two digits. */
typedef struct {
    SPRT unk_00[9];
    DR_MODE unk_B4;
} Unk800600C8Rec;

/* The draw chunk 4D0A8 gpu_DrawSpriteAtPackedTablePosition builds at the
 * address its caller passes: one SPRT at +0 (gpu_AddSpriteCells's cursor; each
 * of the three sheet headers it may pass, D_8009B7AC / D_8009B7B8 / D_8009B7C4,
 * has one cell: the count byte at +2 is 1), then the DR_MODE at +0x14. It
 * returns the chunk size, 0x2C; the last 0xC bytes are never written. */
typedef struct {
    SPRT unk_00;
    DR_MODE unk_14;
    u8 unk_20[0xC];
} Unk80060414Rec;

/* The draw chunk 4D0A8 gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells
 * builds at the address its caller passes: SPRTs from +0 up to +0x4EC (63;
 * gpu_AddSpriteCells's cursor), POLY_FT4s from +0x4EC up to +0x5DC (6;
 * gpu_AddScaledSpriteQuads's cursor), then the DR_MODE at +0x5DC. It returns
 * the chunk size, 0x5F4; the last 0xC bytes are never written. The walkers
 * write at most one primitive per cell of the sheet header they are given (cell
 * count: the byte at +2): at most 19 SPRTs (D_8009B770[0..2] hold 5 / 4 / 4
 * cells, D_8009B7A0 4, D_8009B398[2] / [3] 1 each) and 1 POLY_FT4
 * (D_8009B770[3] holds 1). */
typedef struct {
    SPRT unk_00[63];
    POLY_FT4 unk_4EC[6];
    DR_MODE unk_5DC;
    u8 unk_5E8[0xC];
} Unk80060544Rec;

/* 0x8009BD24: two players x five rounds of 2-byte records; byte 0 is an
   index (gpu_DrawPackedComparisonRowsAndTotals reads it at
   j * 10 + i * 2 and picks D_8009ADD8 / D_8009B58C by it;
   gpu_DrawSpriteAtPackedTablePosition reads player 0 round 0).
   Unk8009BD24Block.match_picks. */
typedef struct {
    u8 bd0;
    u8 unk1;
} Unk8009BD24Record;

/* One of the three 4-byte records at Unk8009BD24Block.time_records
   (fight_PackTimeRecordsAndInitPresentation and
   CopySavedFlagsButtonMappingsAndTimesToWorkingRecord store them;
   gpu_DrawIndexedCellsAndDecimalDigitPairs reads them). */
typedef struct {
    u8 minutes;
    u8 seconds;
    u8 hundredths;
    u8 character_index;
} Unk8009BD45Rec;

/* 0x8009BD24..0x8009BD57: the settings record GetWorkingRecord returns and
   64FD8 hands to menu_LoadResourcesInitStateSelectFirstEnabledRow /
   InitDisplayStateLoadPackAndSetEntryFlags / menu_InitSelScreen, whose callees
   reach every field off that one base (596CC g_row_menu_working_record_ptr,
   5ED34 g_entry_menu_working_record_ptr, SelWork.f00). Bit fields are named by
   bit offset until named.
   - unk14_*: the word at +0x14. fight_mode (`& 0xF`;
   menu_UpdateRowMenuStoreResult stores it), stage_index (6 bits),
   match_count_index (the round count - 3; also picks the results-screen
   layout), time_limit_index (`== 2` tests), win_target_index (the win target
   - 1), unk14_15 (one bit per player), unk14_17, unk14_18 (3 bits); no code
   reads bits 21-23.
   - button_mappings / default_button_mappings / committed_button_mappings:
   three byte triples. menu_HandleRowBitInput rebuilds a player's nibble of
   button_mappings from default_button_mappings or committed_button_mappings;
   64FD8 copies button_mappings and committed_button_mappings into each other.
   - unk20_*: the word at +0x20, bits 0-3 (grayscale, unk20_1,
   cd_audio_suppressed, view_setting); its upper bytes start the time_records
   records. */
typedef struct {
    Unk8009BD24Record match_picks[2][5];
    u32 fight_mode : 4;
    u32 stage_index : 6;
    u32 match_count_index : 2;
    u32 time_limit_index : 2;
    u32 win_target_index : 1;
    u32 unk14_15 : 2;
    u32 unk14_17 : 1;
    u32 unk14_18 : 3;
    u32 unk14_21 : 3;
    u8 button_mappings[3];
    u8 default_button_mappings[3];
    u8 committed_button_mappings[3];
    u32 grayscale : 1;
    u32 unk20_1 : 1;
    u32 cd_audio_suppressed : 1;
    u32 view_setting : 1;
    u32 unk20_4 : 4;
    Unk8009BD45Rec time_records[3];
    u8 fight_time[3];
    u8 fight_character_index;
} Unk8009BD24Block;

/* The select-screen work area g_sel_screen_work points at (menu_InitSelScreen
 * places it at the end of the buffers gpu_InitPrimBufferPair lays out).
 * Two-element arrays are per player. Per player, f48 is a list of f60 entries
 * with cursor f5C; confirming moves the entry under the cursor to f7E[f3C++],
 * cancelling moves f7E[--f3C] back into f48 in sorted order; f6A holds the
 * picked entries menu_FadeOutSelAndWriteResult reports. f00 is the result
 * record menu_FadeOutSelAndWriteResult writes; f04 is the resource table the
 * select draw functions receive as draw_context[0] (menu_StepSelScreen passes
 * it as the first word of that context). f10, f14, f1C and f20 are per-player
 * s16 pairs the code also accesses as one word (union word views, Q33/Q46):
 * menu_HandleSelPhase0CursorInput and menu_HandleSelOpposedChoice read f10
 * whole, menu_RunSelPhases reads f14 whole, menu_InitSelScreen clears f1C / f20
 * whole (menu_UpdateSelGridPicks reads and writes their halves). The s32
 * members make the struct 4-aligned: sizeof is 0x94 (members end at 0x92; Q57).
 */
typedef struct {
    Unk8009BD24Block *f00;
    Unk80076FF8Rec *f04;
    s16 f08[2];
    s16 f0C[2];

    union {
        s16 half[2];
        s32 word;
    } f10;

    union {
        s16 half[2];
        s32 word;
    } f14;

    s16 f18[2];

    union {
        s16 half[2];
        s32 word;
    } f1C;

    union {
        s16 half[2];
        s32 word;
    } f20;

    GpuDb *f24;
    u8 pad28[4];
    Unk8006E49CRec *f2C;
    s32 f30;
    u16 f34;
    u16 f36;
    s16 f38[2];
    s16 f3C[2];
    s16 f40[2][2];
    s16 f48[2][5];
    s16 f5C[2];
    s16 f60[2];
    u8 f64;
    u8 f65;
    u8 f66;
    u8 f67;
    u8 f68[2];
    s16 f6A[2][5];
    s16 f7E[2][5];
} SelWork;

/* 0x8009BC94: table of {x, y} s16 position records, 6 records (24 bytes) per
 * row; DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection and
 * DrawRotatedEntriesAndOnFullCounterFinishWithResultOrNextStateAndStoreSelections
 * index it [row][g_entry_menu_row_phase_choice_cursors[row]]. */
typedef struct {
    s16 x;
    s16 y;
} Unk8009BC94Record;

/* 0x8009BC40: one select-grid entry; value indexes g_entry_menu_entry_flags's
 * flags and picks the entry's sprite and sound. */
typedef struct SelectEntryE534 {
    u8 value;
    u8 unk1;
} SelectEntryE534;

/* 0x8009B450: two {x, y} screen points (.short 0x01A2,0x0024,0x01F7,0x0037).
 * gpu_DrawTimeFieldsAndDecimalCounter uses x as a TILE's x0 (and w = 0x238 - x)
 * and y as its y0 and the text row. */
typedef struct {
    s16 x;
    s16 y;
} Unk8009B450Record;

/* The 0x2C-byte draw descriptor the sprite walkers consume: gpu_AddSpriteCells
 * (one SPRT per cell), gpu_AddScaledSpriteQuads / gpu_AddRotatedSpriteQuads
 * (one POLY_FT4 per cell, scaled / rotated). header / table: the sprite sheet
 * (Unk8009B0E0Record) and its cells; sprt_out / ft4_out: the SPRT and POLY_FT4
 * cursors the walkers advance and return; semi: their SetSemiTrans argument;
 * ot_idx: the ordering-table slot (g_gpu_ot_ptr + ot_idx); x / y: the screen
 * offset added to every cell; scale_x / scale_y: the 8.8 cell scales of the
 * POLY_FT4 walkers; has_color / col_r / col_g / col_b: the SetShadeTex switch
 * and the primitive colour. The cursors are s32 because every source of them is
 * an s32 word: the draw contexts' primitive cursors (Unk800788B0Rec, 596CC's
 * context words). */
typedef struct {
    Unk8009B0E0Record *header;
    Unk8009B400Record *table;
    s32 sprt_out;
    s32 ft4_out;
    s32 semi;
    u32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Unk8007352CRec;

/* 0x8009B2BC: three {w, h} menu-frame sizes, one per mode (0x8009B2BC..
 * 0x8009B2C7; D_8009B2C8 follows, different data), indexed by mode in
 * gpu_DrawModeSizedPanelWithSelectedCellOpaque. */
typedef struct {
    s16 w;
    s16 h;
} Unk8009B2BCRecord;

/* The playback block (motion_LoadPlaybackBlock loads it, or the buffer
   motion_StartPlayback is handed; Unk800EFAE8Rec .block_base): its header holds
   file-relative offsets of the parts motion_StartPlayback /
   motion_UpdatePlayback use. */
typedef struct {
    s32 unk0;
    s32 control_word_offset;
    s32 frame_count_offset;
    s32 camera_stream_offset;
    s32 motion_stream_offsets[2];
    s32 draw_set_track_offsets[2];
} Unk800469C4Hdr;

/* Record at 0x800EFAE8 (0x4C bytes): motion_StartPlayback sets it
 * up, the per-frame handler motion_UpdatePlayback runs it (walking the
 * per-object arrays in its object loop), and motion_RebasePlayback relocates
 * the 0x2C..0x40 words.
 */
typedef struct {
    /* frame cursor (motion_UpdatePlayback: -1 = finished, 0 = first frame) */
    /* 0x00 */ s16 current_frame;
    /* 0x02 */ s16 frame_count;
    /* packed control word (bit31/bit30 tests, low 6 bits = resource block
     * index + 1) */
    /* 0x04 */ s32 control_word;
    /* 0x08 */ s16 start_pair_index;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s32 origin_x;
    /* 0x10 */ s32 origin_y;
    /* 0x14 */ s32 origin_z;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 rotation_x;
    /* 0x1E */ s16 rotation_y;
    /* 0x20 */ s16 rotation_z;
    /* 0x22 */ s16 unk22;
    /* negated camera rotation vx/vy/vz; motion_GetPlaybackViewAngles returns
     * its address */
    /* 0x24 */ s16 view_angles[4];
    /* loaded data base; relocated by motion_RebasePlayback */
    /* 0x2C */ s32 block_base;
    /* 0x30 */ s32 camera_stream; /* relocated by motion_RebasePlayback */
    /* per player; relocated by motion_RebasePlayback when nonzero */
    /* 0x34 */ s32 motion_streams[2];
    /* per player; relocated by motion_RebasePlayback when nonzero */
    /* 0x3C */ s32 draw_set_tracks[2];
    /* 0x44 */ s16 unk44[2]; /* per player */
    /* 0x48 */ s16 unk48[2]; /* per player */
} Unk800EFAE8Rec;

/* 0x800A3560: two 3-byte records, one per selection slot i (slot i at
 * 0x800A3560 + i * 3; 0x800A3566/7 pad before g_entry_menu_working_record_ptr).
 * Object-model evidence:
 * DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection and
 * menu_UpdateRow step one offset register by 3 per slot;
 * menu_UpdateEntryGridCursors reaches grid_pick / cursor_entry_value / row_pick
 * through slot_index * 3; InitDisplayStateLoadPackAndSetEntryFlags stores
 * cursor_entry_value of both. `word` (Q33 union word view) is
 * InitDisplayStateLoadPackAndSetEntryFlags's one word store of -1 over bytes
 * 0..3; every other access goes through rec[]. Every consumer is in the -G8
 * file src/main/5ED34.c (Q44/Q54).
 */
typedef struct {
    u8 grid_pick;
    u8 cursor_entry_value;
    u8 row_pick;
} Unk800A3560Record;

typedef union {
    Unk800A3560Record rec[2];
    s32 word;
} Unk800A3560Slots;

/* Record table at 0x800F0C10 (0x90 bytes, ends at
 * g_eff_animated_quad_positions): 4 rows of 3 records of 3 s32 words.
 * eff_PrepareParticles indexes it [particle_row_index][idx]
 * (particle_row_index 0..3 from the callers func_800676C8..func_8006786C);
 * eff_SpawnParticles and eff_UpdateParticles use it too. */
typedef struct {
    /* 0x0 */ s32 world_x;
    /* 0x4 */ s32 world_y;
    /* 0x8 */ s32 world_z;
} Unk800F0C10Record;

/* 0x800948BC (g_resource_selector_callbacks): function-pointer pairs, indexed
 * by resource_GetLoadedSelector(). resource_InvokeLoadedSelectorInitCallback
 * calls .init;
 * gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards calls
 * .unk4 (e.g. entry 13 holds node_InitViewOffsetNodeWithYXZRotation /
 * node_OrientAndQueueViewOffsetNode). */
typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} Unk800948BCEntry;

/* g_motion_fighter_data_files: motion data id (the fighter records'
 * motion_data_id) + BBM file-name string, ended by a zero id
 * (0x8008DCCC..0x8008DD5B). motion_LoadFighterData matches the id and reads
 * entry n's file, CD file n + 2, into g_motion_fighter_buffers; the name is
 * never read. */
typedef struct {
    s32 id;
    const char *name;
} Unk8008DCCCEntry;

/* The file motion_LoadFighterData loads into g_motion_fighter_buffers[slot]
 * (slot = rec+0x4A, the load slot fighter_AssignMotionSlotAndScanDataIdNibbles
 * sets; fighters with the same model id share it). f14 is the modulus
 * motion_AdvanceVariantIfAtNibble4Or5Position / motion_ResolveMoveData wrap
 * rec+0x86 with. The u16 fields at +0x4E are indices into
 * g_motion_fighter_sections[slot].move_scripts as motion_ResolveMoveData reads:
 * f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4), f54[rec+0x86][t]
 * (id 0x7FF3), and f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). f16
 * (motion_GetCommonScriptAtFighterBufferOffset) and f18[character_data_index]
 * (motion_GetCommonScriptAtFighterBufferIndexedOffset;
 * Unk80101EC8Record.character_data_index, 0..26 as g_fight_config_id_data_rows
 * holds and as the [27][6] tables g_fighter_base_move_frame_steps /
 * g_fighter_base_powers are sized; 27 halfwords end exactly at f4E) are indices
 * into g_motion_common_sections.move_scripts; both readers load them with lhu.
 */
typedef struct {
    u8 pad00[3];
    /* motion_LoadFighterData: g_motion_fighter_sections[slot].move_scripts =
     * record + 0x6C + (unk_03 - 1) * 6 */
    u8 unk_03;
    /* g_motion_fighter_sections[slot].motion_entries .. motion_bitstream =
     * record + unk_04[k] (motion_LoadFighterData) */
    s32 unk_04[4];
    s16 f14;
    u16 f16;
    u16 f18[27];
    u16 f4E[3];
    u16 f54[3][3];
    u16 f66[11][3];
} Tbl800A3860Entry;

/* Five section pointers into a loaded file. g_motion_fighter_sections[i] for
 * the file loaded into g_motion_fighter_buffers[i] (motion_LoadFighterData:
 * move_scripts = record + 0x6C + (unk_03 - 1) * 6,
 * motion_entries..motion_bitstream = record + unk_04[0..3]);
 * g_motion_common_sections for the common file (motion_LoadCommonData:
 * move_scripts = file + 0x14, the others file + its header words 1 / 2 / 4;
 * cpu_script_table unset). move_scripts: halfword streams
 * motion_ResolveMoveData indexes (the f4E / f54 / f66 / f16 / f18 entries);
 * motion_entries: 4-byte entries whose second halfword offsets
 * motion_metadata_base (fighter_StartMoveScript, replay_PlayFrame);
 * cpu_script_table: fighter_InitCpuControl's table; motion_bitstream:
 * motion_InitStream's motion bitstream. */
typedef struct Unk801027B0Rec {
    u16 *move_scripts;
    u16 *motion_entries;
    u8 *motion_metadata_base;
    u16 *cpu_script_table;
    u32 *motion_bitstream;
} Unk801027B0Rec; /* sizeof == 0x14 */

/* s32 x/y/z triple.  Unk80101EC8Record's position-like triples are copied as
 * whole 12-byte objects (fighter_Init), and
 * fight_SearchRadialFlatSurfacePointOrUseStartPositionPair writes one through
 * its out parameter. */
typedef struct {
    s32 x, y, z;
} Vec3i32;

/* A position (g_snd_sequence_positions: 12-byte stride, 6 entries, one per
   g_snd_sequence_ticks slot): the same s32 x/y/z triple as Vec3i32;
   fighter_UpdateBoneWorldData copies a scratchpad point (SPAD->bone_hit_points)
   into Unk80101EC8Record.bone_hit_point_0 as one 12-byte object. */
typedef Vec3i32 Unk80107850Rec;

/* The PsyQ SVECTOR layout under a local name. */

typedef struct {
    s16 vx, vy, vz, pad;
} SVec4i16;

/* g_fight_ballistic_records[4]: the four 0x2C-byte records
 * fight_CreateBallisticRecordAndDispatchEvent claims (first with ballistic_kind
 * == 0) and fight_UpdateBallisticRecords steps each frame (flight_frames counts
 * frames; prev_ballistic_pos keeps the previous ballistic_pos;
 * ballistic_velocity is added to ballistic_pos with ballistic_velocity.y
 * growing by 0xD; the record is freed when
 * collision_TestSegmentExtendingShortStartWithWork reports a hit or
 * ballistic_pos.y passes launcher_collision_y).
 * fight_TriggerOpposingFighterEventFromNearBallisticRecords measures the
 * distance from the other player's world_pos to ballistic_pos (owner_index is
 * the owner). fight_ClearBallisticRecords clears ballistic_kind of all four. */
typedef struct Unk80104E88Rec {
    /* 0 = free; fight_CreateBallisticRecordAndDispatchEvent's type (1 / 2) */
    u8 ballistic_kind;
    u8 unk_01;
    u8 flight_frames;
    u8 owner_index; /* player index (Unk80101EC8Record.index) */
    Vec3i32 ballistic_pos;
    Vec3i32 prev_ballistic_pos;
    Vec3i32 ballistic_velocity;
    s32 launcher_collision_y;
} Unk80104E88Rec; /* sizeof == 0x2C */

/* The 10-entry block table over the 0x45000-byte buffer at g_arena
 * (arena_Reset .. arena_FindBlock; g_arena_block_count live entries).
 * arena_Reset clears id in all 10. arena_ShiftBlocks(a0, a1) hands
 * mem_MoveBlockRoundedUpToWords entry a0's unk4, unk4 + a1 and the summed amt
 * of entries a0.., then adds a1 to their unk4 and calls each fn(id, a1). */
typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} Unk800EED10Entry;

/* A hit record: one of the 22 test points of a character.
 * fighter_UpdateBoneWorldData places point i at SPAD->bone_hit_points[ch][i]
 * (`ofs` rotated by node_GetSlotMatrixPointerArray()'s matrix `bone`, plus that
 * matrix's translation); the hit tests (fighter_CastWeaponSegment,
 * fighter_TestTriangleBoneHits, object_ResolveFighterHits) take outer_radius /
 * unk_0E as the first test's limits and, when has_inner_sphere != 0,
 * inner_radius / unk_12 as a second test's. */
typedef struct BoneHitRec {
    s16 has_inner_sphere;
    s16 bone;
    SVec4i16 ofs;
    u16 outer_radius;
    u16 unk_0E;
    u16 inner_radius;
    u16 unk_12;
} BoneHitRec; /* sizeof == 0x14 */

/* Pad input record (0x18 bytes) at 0x80102788.  pad_Update fills it each
 * frame from the two pads (one u16 half per player in each word):
 * held = current bits, pressed = held & ~previous, released = ~held & previous,
 * unheld = ~held; pad_ResetState sets type[] and the four bit words to
 * (4, 4, 0, 0, 0, -1) and leaves valid[] alone.
 * fighter_BuildCpuOrMappedPadState copies the whole record to its caller's
 * buffer, and pad_ClearStateBits clears the four bit words of such a buffer. */
typedef struct PadState {
    /* per pad: InitPAD buffer byte 1 >> 4, 5 and 7 folded
       to 4, 4 when the status byte != 0 (pad_Update,
       where comb_ExchangePad can first replace them with
       nibbles from the link-cable exchange words);
       4 from pad_ResetState (both) and fighter_BuildCpuPad
       (entry arg0 of its own record) */
    s16 type[2];
    /* per pad: 1 iff the InitPAD buffer status byte == 0
       (pad_Update); 1, 1 from pad_ResetStateMarkValid */
    s16 valid[2];
    u32 held;     /* 0x08 */
    u32 pressed;  /* 0x0C */
    u32 released; /* 0x10 */
    u32 unheld;   /* 0x14 */
} PadState;       /* sizeof == 0x18 */

/* The 24-entry pending-sound pool (4BC3C): snd_QueueSound queues a request (an
 * entry of g_snd_sound_vab_entries) with its volumes in the first free entry;
 * snd_FlushSoundQueue keys each queued note on via SsUtKeyOnV(.., voll, volr)
 * and clears the entry; snd_Init / snd_ResetVoicesAndQueue reset all 24. */
typedef struct {
    u16 *req;
    u8 volr;
    u8 voll;
} Unk800EFB78Entry;

/* One 8-byte texture record: the CLUT position (PsyQ getClut(x, y) = (y << 6) |
 * ((x >> 4) & 0x3F)) and the texture u/v origin. 51268's g_eff_current_tex_rec
 * / g_eff_clut_tex_rec point at one; the D_8009B890 .. D_8009BA58 tables are
 * runs of them. */
typedef struct {
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
} TexRec;

/* The 0x2C-byte block g_eff_active_command_block points at (51268):
 * Unk1F800000Rec.default_command_block (eff_BindScratchpad's seed) or
 * g_eff_command_block, where eff_StartRotatedQuadClusterWithNearWhiteLight ..
 * eff_StartRecycledCommandWithMagentaLight point it before calling
 * eff_StartCommand. g_eff_billboards follows g_eff_command_block, so that copy
 * ends at +0x2C. Each copy sets one pointer pair:
 * - command_word: one word, stored whole and read whole (bit 21 in
 * eff_StartCommand, bits 17-18 / 19-20 in func_80063AF0 / func_80063B34 /
 * eff_InitRateSelectedCosineScaledQuad); eff_StartCommand / eff_StepCommand
 * also read its low halfword (`lhu`), their g_eff_command_phase /
 * g_eff_command_handler_bases index.
 * - bound_rotation_src / bound_position_src: the three halfwords / three words
 * eff_StepCommand and eff_DrawBillboardList copy into the scratchpad block's
 * vector_copy / position_copy (through D_800A346C / g_eff_view_origin). Only
 * the scratchpad block's are set: eff_BindScratchpad stores its two arguments
 * there.
 * - start_position_src / start_vector_src: the three words / three halfwords
 * eff_StartCommand copies into position_copy / vector_copy. Set only in the
 * g_eff_command_block block (through g_eff_active_command_block or
 * g_eff_command_block directly).
 * - status_dest: the byte eff_StartCommand / eff_StepCommand store the called
 * function's result to (`sb`).
 * - vector_copy / position_copy: the copies. D_800A346C / g_eff_view_origin
 * point at the scratchpad block's, g_eff_command_vector /
 * g_eff_command_position at the g_eff_command_block block's (eff_StartCommand).
 * vector_copy is s16 like its sources (bound_rotation_src / start_vector_src)
 * and every consumer (SVECTOR / SVec4i16 fields: eff_SetRotMatrix,
 * eff_SpawnRotatedQuadCluster, eff_AllocateOrRecycleYawedQuadSlot,
 * eff_SpawnParticles).
 * - unk1E: no access. */
typedef struct {
    union {
        s32 w;
        u16 h;
    } command_word;

    s16 *bound_rotation_src;
    s32 *bound_position_src;
    s32 *start_position_src;
    s16 *start_vector_src;
    u8 *status_dest;
    s16 vector_copy[3];
    u8 unk1E[2];
    s32 position_copy[3];
} Unk1F800000Unk00;

/* 20-byte record table at 0x800EFC78 (51268): 4 rows (particle_row_index)
 * of 48 records, filled by eff_SpawnParticles (which leaves +0xE
 * untouched). */
typedef struct {
    s16 offset_x;
    s16 offset_y;
    s16 offset_z;
    s16 group_position_index;
    s16 velocity_x;
    s16 velocity_y;
    s16 velocity_z;
    s16 unkE;
    s16 life_state;
    s16 frame_age;
} Unk800EFC78Record;

/* eff_DrawBillboardList's layout of the 51268 work area
 * (Unk1F8000B8Union.v800620B8). eff_SetRotMatrix, which eff_DrawBillboardList
 * calls before it touches the area, writes and consumes its SVECTOR at
 * +0x00..+0x07. SetTransMatrix is handed the address 0x14 below
 * rotated_position, so rotated_position is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                  /* no access here */
    s16 billboard_screen_half_width; /* half_width */
    s16 billboard_screen_height;     /* billboard_height */
    VECTOR rotated_position;         /* ApplyRotMatrixLV out */
    VECTOR view_relative_position;   /* ApplyRotMatrixLV in */
    SVECTOR zero_vector;             /* RotTransPers in */
    s32 depth_cue;                   /* RotTransPers' p */
    u8 unk40[4];                     /* no access */
    u32 billboard_ot_index; /* gpu_DepthToOtIndex's depth, the OT index */
} Unk1F8000B8_800620B8;

/* eff_DrawAndAdvanceBillboardPairs's layout (Unk1F8000B8Union.v80063084).
 * SetTransMatrix is handed the address 0x14 below rotated_position (the area's
 * base), so rotated_position is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x14];                 /* no access */
    VECTOR rotated_position;        /* ApplyRotMatrix out */
    SVECTOR view_relative_position; /* ApplyRotMatrix in */
    SVECTOR zero_vector;            /* RotTransPers in */
    s32 depth_cue;                  /* RotTransPers' p */
    /* g_eff_billboard_pair_ages[slot_index] - 15, the colour scale */
    s32 unk38;
    s32 billboard_ot_index; /* gpu_DepthToOtIndex's depth, the OT index */
} Unk1F8000B8_80063084;

/* eff_DrawAndAdvanceJitteredBillboards's layout (Unk1F8000B8Union.v800646E8).
 * SetTransMatrix is handed the address 0x14 below rotated_position, so
 * rotated_position is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                /* no access */
    s16 billboard_screen_width;    /* billboard_width */
    s16 billboard_screen_height;   /* billboard_height */
    s32 billboard_frame_index;     /* frame */
    VECTOR rotated_position;       /* ApplyRotMatrixLV out */
    VECTOR view_relative_position; /* ApplyRotMatrixLV in */
    SVECTOR unk38;                 /* RotTransPers in */
    s32 unk40;                     /* RotTransPers' p */
    POLY_FT4 *unk44;               /* the end of the quads */
    u32 quad_ot_index_table[16];   /* per kept quad, its OT index */
} Unk1F8000B8_800646E8;

/* eff_DrawAnimatedQuads's layout of the 51268 work area
 * (Unk1F8000B8Union.v80065800). Its gte_SetTransMatrix operand hands the GTE
 * the area's base as a MATRIX, whose t[] is unk14[0]. */
typedef struct {
    u8 unk00[0x10]; /* no access */
    s32 depth_cue;  /* gte_stdp out; not read */
    /* ApplyRotMatrixLV out ([0]); the corner loop's ApplyRotMatrix outs */
    VECTOR unk14[4];
    VECTOR view_relative_position; /* ApplyRotMatrixLV in */
    /* gte_ldv0 / RotMatrix / ApplyRotMatrix in */
    SVECTOR unk64;
    s16 quad_extent_x;             /* p_w */
    u8 unk6E[2];                   /* no access */
    s16 quad_extent_y;             /* p_h */
    u8 unk72[2];                   /* no access */
    MATRIX corner_rotation_matrix; /* RotMatrix out, SetRotMatrix in */
    s16 tex_width;                 /* added to the texture u */
    s16 tex_height;                /* added to the texture v */
} Unk1F8000B8_80065800;

/* The layout eff_PrepareParticles / eff_UpdateParticles / eff_LinkParticleQuads
 * share (Unk1F8000B8Union.v800678A8): each of the wrappers func_800676C8 ..
 * func_8006786C calls the three in turn, and values cross the calls
 * (squared_distance_limit, particle_scan_count, particle_quad_cursor, the
 * quad_ot_index_table; eff_PrepareParticles and eff_UpdateParticles each use
 * unk70 for their own value). eff_PrepareParticles sets it up,
 * eff_UpdateParticles builds the quads at particle_quad_cursor and records each
 * one's depth (its OT slot index) in quad_ot_index_table, eff_LinkParticleQuads
 * links them. */
typedef struct {
    u16 tex_right_u_offset;  /* eff_PrepareParticles: added to the texture u */
    u16 tex_bottom_v_offset; /* eff_PrepareParticles: added to the texture v */
    /* eff_PrepareParticles stores 0x895440; eff_UpdateParticles compares
     * against it */
    u32 squared_distance_limit;
    u8 unk08[8]; /* no access */
    /* eff_UpdateParticles: only its translation (+0x24) is used: the
       gte_ApplyRotMatrix / gte_Square0 output, loaded by gte_SetTransMatrix */
    MATRIX unk10;
    u8 unk30[4];  /* no access */
    VECTOR unk34; /* eff_UpdateParticles: gte_Square0 in */
    /* eff_UpdateParticles: gte_ApplyRotMatrix / gte_ldv3 in */
    SVECTOR unk44[3];
    VECTOR group_position; /* eff_UpdateParticles */
    /* eff_PrepareParticles stores it; eff_UpdateParticles's loop bound */
    s16 particle_scan_count;
    /* eff_UpdateParticles / eff_LinkParticleQuads loop index */
    s16 unk6E;
    /* eff_PrepareParticles: table index; eff_UpdateParticles: per entry */
    s16 unk70;
    u8 particle_red; /* eff_UpdateParticles colours */
    u8 particle_green;
    u8 particle_blue;
    u8 unk75[3];         /* no access */
    s16 screen_xy_index; /* eff_UpdateParticles */
    u8 unk7A[2];         /* no access */
    /* eff_LinkParticleQuads: the end of the quads */
    POLY_FT4 *new_quads_end;
    POLY_FT4 *particle_quad_cursor;          /* the quad cursor */
    Unk800EFC78Record *current_particle;     /* eff_UpdateParticles */
    Unk800F0C10Record *group_position_entry; /* eff_UpdateParticles */
    /* per quad, its depth, the OT slot index (eff_UpdateParticles stores,
       eff_LinkParticleQuads reads) */
    u16 quad_ot_index_table[0x90];
    /* eff_UpdateParticles stores rand() values here; D_800A3724 points at it */
    s32 random_accumulator;
} Unk1F8000B8_800678A8;

/* Unk1F800000Rec.effect_work_area, the 51268 work area g_eff_work_area points
 * at (0x1F8000B8 to the end of the scratchpad). Each user lays it out for one
 * call (or, for the eff_PrepareParticles trio, one wrapper call); raw sizes the
 * union. eff_SetRotMatrix's is one SVECTOR.
 * eff_DrawAndAdvanceRotatedQuadClusters and eff_DrawYawedSlotQuads still
 * convert g_eff_work_area (their work-area base is staged through a POLY_FT4 *
 * local). */
typedef union {
    u8 raw[0x400 - 0xB8];
    SVECTOR v80061FAC;
    Unk1F8000B8_800620B8 v800620B8;
    Unk1F8000B8_80063084 v80063084;
    Unk1F8000B8_800646E8 v800646E8;
    Unk1F8000B8_80065800 v80065800;
    Unk1F8000B8_800678A8 v800678A8;
} Unk1F8000B8Union;

/* 51268's view of the scratchpad from 0x1F800000 (SPAD51268).
 * 17AFC's view of the same memory, used at other times, is ScrPad.
 * eff_BindScratchpad points 51268's D_800A34xx globals at these members; apart
 * from its own two stores to default_command_block.bound_rotation_src /
 * bound_position_src, the code reaches them only through those globals:
 * - default_command_block: the command block g_eff_active_command_block points
 * at until a function retargets it; D_800A346C / g_eff_view_origin point at its
 * vector_copy / position_copy.
 * - unk2C: no access.
 * - unk50 / unkB0: the initial targets of g_eff_current_tex_rec /
 * g_eff_clut_tex_rec (TexRec). eff_DrawBillboardList retargets them only in its
 * switch cases 0-3 (unk4 & 7), so its later reads go through these seeded
 * values when no earlier record took one of those cases.
 * - unkA0 / unkA4: the initial targets of g_eff_current_ot_entry (u32 *) /
 * g_eff_current_prim_tag (u32
 * *), which every user retargets before use.
 * - effect_work_area: the work area g_eff_work_area points at
 * (Unk1F8000B8Union), laid out differently by its users; it runs to the end of
 * the scratchpad. The other members are one global each
 * (g_eff_view_rotation_matrix .. g_eff_bottom_right_uv,
 * g_eff_yawed_quad_tex_index / g_eff_quad_rate_selector). */
typedef struct {
    Unk1F800000Unk00 default_command_block;
    u8 unk2C[4];
    MATRIX view_rotation_matrix; /* g_eff_view_rotation_matrix */
    TexRec unk50;
    s32 tpage_word;        /* g_eff_tpage_word */
    s32 clut_word;         /* g_eff_clut_word */
    u16 tex_left_u;        /* g_eff_tex_left_u */
    u16 tex_right_u;       /* g_eff_tex_right_u */
    u16 tex_top_v;         /* g_eff_tex_top_v */
    u16 tex_bottom_v;      /* g_eff_tex_bottom_v */
    s16 base_width;        /* g_eff_base_width */
    s16 base_height;       /* g_eff_base_height */
    s32 geom_screen_work;  /* g_eff_geom_screen_work */
    s32 perspective_scale; /* g_eff_perspective_scale */
    s32 screen_xy[3];      /* g_eff_screen_xy */
    s16 left_x_offset;     /* g_eff_left_x_offset */
    s16 right_x_offset;    /* g_eff_right_x_offset */
    s32 upper_y_offset;    /* g_eff_upper_y_offset */
    s32 y_extent_work;     /* g_eff_lower_y_offset_word */
    s32 unk8C;             /* g_eff_rtp_flag */
    s32 depth_work[2];     /* g_eff_depth_work */
    u16 top_left_uv;       /* g_eff_top_left_uv */
    u16 top_right_uv;      /* g_eff_top_right_uv */
    u16 bottom_left_uv;    /* g_eff_bottom_left_uv */
    u16 bottom_right_uv;   /* g_eff_bottom_right_uv */
    u32 unkA0;
    u32 unkA4;
    s32 yawed_quad_tex_rec_index; /* g_eff_yawed_quad_tex_index */
    s32 quad_rate_selector;       /* g_eff_quad_rate_selector */
    TexRec unkB0;
    Unk1F8000B8Union effect_work_area;
} Unk1F800000Rec;

/* A cell (x, z) of the 32x32 grid of 2000-unit cells that 43500's
 * collision_WalkSegmentGrid walks (Work_80053E9C.walk_cell / end_cell). */
typedef struct {
    s16 x;
    s16 z;
} Cell_80052D00;

/* The 0xEC-byte work area of 43500's stage-collision cast
 * (collision_WalkSegmentGrid and its helpers, through
 * g_collision_segment_work). collision_TestSegmentExtendingShortStartWithWork /
 * collision_TestSegmentWithWork place it at their last argument;
 * collision_TestSegmentExtendingShortStart / collision_TestSegment at
 * g_collision_static_segment_work. 17AFC object_UpdatePool /
 * fight_UpdateBallisticRecords pass 0x1F8002F0, its place in their scratchpad
 * layout (Unk1F8002B8_8005344C.cast_work). */
typedef struct {
    s32 nearest_dist_sq;
    u16 hit_tag;
    s16 unk6;
    VECTOR seg_start;
    VECTOR seg_end;
    s32 hit_nx;
    s32 hit_ny;
    s32 hit_nz;
    s32 hit_plane_d;
    s32 hit_local_x;
    s32 hit_local_y;
    s32 hit_local_z;
    s32 unk44;
    s16 hit_cell_x;
    s16 hit_cell_z;
    s16 local_start_x;
    s16 local_start_y;
    s16 local_start_z;
    s16 unk52;
    s16 local_end_x;
    s16 local_end_y;
    s16 local_end_z;
    s16 unk5A;
    s32 (*unk5C)(s32, s32);
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 walk_minor_step;
    s32 walk_slope;
    s32 unk80;
    s32 start_cell_center_z;
    Cell_80052D00 walk_cell;
    Cell_80052D00 end_cell;
    s16 walk_steps_left;
    u8 unk92[0xA];
    s32 proj_axis_x;
    s32 proj_axis_y;
    s32 proj_axis_z;
    s32 plane_cross_x;
    s32 plane_cross_y;
    s32 plane_cross_z;
    s32 edge_from_u;
    s32 edge_from_v;
    s32 edge_to_u;
    s32 edge_to_v;
    s32 proj_cross_u;
    s32 proj_cross_v;
    s32 face_tag;
    s32 face_nx;
    s32 face_ny;
    s32 face_nz;
    s32 face_plane_d;
    s32 unkE0;
    s32 unkE4;
    s32 unkE8;
} Work_80053E9C;

/* Unk1F8002B8Rec.point_slots, 0x60 bytes of scratch. point_slots is up to six
 * points that shape_points[0..2] aim at. fighter_CastWeaponSegment and
 * object_ResolveFighterHits use [0] / [1], for
 * math_PrepareSegmentCollisionFrame / math_TestPreparedSegmentSphere:
 * fighter_CastWeaponSegment's segment, base then tip
 * (fighter_ProcessWeaponBodyHits writes them before each call), and
 * object_ResolveFighterHits's g_object_pool object step, prev_pos then pos.
 * fighter_ProcessWeaponBodyHits uses all six: [0] / [1] are two of the other
 * character's scratchpad points (ScrPad.weapon_points or second_weapon_points),
 * [2] / [3] the same two from its record (prev_weapon_points or
 * prev_second_weapon_points), [4] / [5] the midpoints of [0] / [2] and [1] /
 * [3]; for each triangle it hands math_PrepareTriangleCollisionFrame it aims
 * shape_points[0..2] at three of them. object_UpdatePool /
 * fight_UpdateBallisticRecords lay the bytes out differently
 * (collision_TestSegmentExtendingShortStartWithWork's argument block, its work
 * area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of
 * Unk1F8002B8Union. raw sizes the union to 0x60. */
typedef union {
    u8 raw[0x60];
    Unk80107850Rec slot_points[6];
} Unk1F8002B8Unk00;

/* 17AFC's view of the last 0x148 bytes of the scratchpad,
 * 0x1F8002B8..0x1F8003FF (ScrPad.unk2B8.rec). The scratchpad is shared scratch:
 * other code puts its own data in these bytes at other times, with its own
 * views. Among others:
 * - 43500 collision_TestSegmentWithWork /
 * collision_TestSegmentExtendingShortStartWithWork keep their work area
 * (Work_80053E9C, 0xEC bytes) at the address of their last argument
 * (g_collision_segment_work); callers pass 0x1F8002B8 (the area spans record
 * +0x00..+0xEB) or 0x1F8002F0 (+0x38..+0x123, over shape_points up into
 * unk118).
 * - 9F9C fighter_UpdateBoneWorldData keeps collision_TestSegmentWithWork's
 * whole argument record at 0x1F8002B8: from, to, hit, normal, then that work
 * area.
 * - 32D04 func_800430E4 keeps a MATRIX at 0x1F8003A0.
 * - 3AB48 gpu_ProjectStridedNodeVerticesAndDispatchPackets fills, and the
 * renderers gpu_EmitOrientationAdjustedDepthBiasedFlatLitTexturedTriangles ..
 * gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256 read, an s16
 * per-vertex table at 0x1F8002B4 + 2 * i, which runs into the record for i
 * >= 2. In 17AFC, the collision code takes the record's address (`scr`) and
 * passes it on, or addresses it directly (object_UpdatePool /
 * fight_UpdateBallisticRecords use Unk1F8002B8_8005344C instead). shape_points
 * / other_shape_points hold point pointers (fighter_ProcessWeaponContacts reads
 * shape_points[0..2]); bone_hit_mask / unkC4 are fighter_TestTriangleBoneHits's
 * two hit masks (fighter_ProcessWeaponBodyHits reads them; no 17AFC code
 * accesses unkD4 as a member); rot_matrix is the matrix the RotMatrix* calls
 * build, and the GTE rotates unkA8 through it; func_8002DE20 rotates three
 * points, relative to the origin *shape_points[0], into unk118 and tests them
 * against the triangle (0,0) / unkA8 / unkB8. point_slots is scratch
 * (Unk1F8002B8Unk00): fighter_CastWeaponSegment / object_ResolveFighterHits /
 * fighter_ProcessWeaponBodyHits use its points. */
typedef struct {
    Unk1F8002B8Unk00 point_slots;
    Unk80107850Rec *shape_points[3];
    Unk80107850Rec *other_shape_points[3];
    Vec3i32 unk78;
    Vec3i32 bbox_max;
    Vec3i32 other_bbox_min;
    Vec3i32 other_bbox_max;
    Vec3i32 unkA8;
    s32 bone_hit_mask;
    Vec3i32 unkB8;
    s32 unkC4;
    Vec3i32 unkC8;
    s32 unkD4;
    MATRIX rot_matrix;
    SVECTOR unkF8;
    Vec3i32 unk100[2];
    Vec3i32 unk118[3];
    Vec3i32 contact_point;
} Unk1F8002B8Rec;

/* The layout object_UpdatePool and fight_UpdateBallisticRecords give the
 * scratchpad area at 0x1F8002B8 (Unk1F8002B8Union.v8005344C): the to point, hit
 * point and normal they pass collision_TestSegmentExtendingShortStartWithWork,
 * and its work area, which runs over Unk1F8002B8Rec's shape_points..unk118. */
typedef struct {
    /* to; collision_TestSegmentExtendingShortStartWithWork copies 16 bytes from
     * it */
    Vec3i32 cast_end;
    s32 unk0C; /* read only as the 4th word of that copy */
    /* the hit point; object_UpdatePool's turn block first keeps a
       rotated velocity here */
    Vec3i32 unk10;
    s32 unk1C; /* no access */
    /* object_UpdatePool's turn block's second rotated vector
       (fight_UpdateBallisticRecords does not access it) */
    Vec3i32 unk20;
    s32 unk2C;          /* no access */
    s16 cast_normal[3]; /* the hit normal */
    s16 unk36;          /* no access */
    /* collision_TestSegmentExtendingShortStartWithWork's work area,
     * +0x38..+0x123 */
    Work_80053E9C cast_work;
} Unk1F8002B8_8005344C;

/* ScrPad.unk2B8: the scratchpad's last 0x148 bytes, 0x1F8002B8..0x1F8003FF,
 * which 17AFC uses with two layouts. Its collision code uses rec
 * (Unk1F8002B8Rec). object_UpdatePool and fight_UpdateBallisticRecords use
 * v8005344C: they pass 0x1F8002F0 as
 * collision_TestSegmentExtendingShortStartWithWork's work-area address, so its
 * 0xEC-byte Work_80053E9C sits at +0x38..+0x123, over rec's
 * shape_points..unk118. */
typedef union {
    Unk1F8002B8Rec rec;
    Unk1F8002B8_8005344C v8005344C;
} Unk1F8002B8Union;

/* Scratchpad point tables at 0x1F800000.  weapon_points: three points per
 * character (fight_UpdateFighterInteractionsAndPointCaches copies [0][0..2] and
 * [1][0..2] to the two records' +0x210); second_weapon_points: two more per
 * character (copied to +0x234); body_overlap_points: two body points per
 * character (fighter_ResolveBodyOverlap builds them from bone_hit_points joint
 * 1 and the midpoint of joints 15 and 19); bone_hit_points: 22 points per
 * character (fighter_CastWeaponSegment reads 0x1F8000A8 + id * 0x108 + i * 0xC,
 * i < 22). fighter_Update hands fighter_UpdateBoneWorldData its character's
 * bone_hit_points / weapon_points / second_weapon_points. unk2B8: the area at
 * 0x1F8002B8 (Unk1F8002B8Union), to the end of the scratchpad; the scratchpad
 * is shared scratch that other code also uses with its own views (see
 * Unk1F8002B8Rec). */
typedef struct {
    Unk80107850Rec weapon_points[2][3];
    Unk80107850Rec second_weapon_points[2][2];
    Unk80107850Rec body_overlap_points[2][2];
    Unk80107850Rec bone_hit_points[2][22];
    Unk1F8002B8Union unk2B8;
} ScrPad;

#define SPAD ((ScrPad *)0x1F800000)

/* One CPU path waypoint (CpuRoute.node[]): fighter_BuildCpuOrbitRoute writes
 * x/z from the opponent's position and kind 2;
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput walks them back to
 * front. */
typedef struct CpuWaypoint {
    s16 x;
    s16 z;
    u8 kind;
    u8 unk5;
} CpuWaypoint;

/* A CPU route: the polygon and edge (its first vertex) of the nearest crossing
 * fighter_FindNearestNavCrossing found, the waypoint count and the waypoints.
 * Unk80101EC8Record carries one at +0x360; fighter_AppendNavDetour builds two
 * candidates of the same layout on its stack and appends the cheaper one. */
typedef struct CpuRoute {
    u8 poly;  /* index into the stage's NavPolySet.polys */
    u8 vtx;   /* vertex index in that polygon */
    u8 count; /* number of waypoints in node[] */
    CpuWaypoint node[8];
} CpuRoute; /* sizeof == 0x34 */

/* One polygon of a stage's navigation set (8 bytes): flags (0x80 = open chain,
 * the last vertex does not close back to the first), a kind byte copied into
 * the route waypoints built around it, a corner margin
 * (fighter_GetNavCornerPoint scales it by 40), the vertex count and the vertex
 * table of x/z pairs. */
typedef struct NavPoly {
    u8 flags;
    u8 kind;
    u8 margin;
    u8 nvtx;
    s16 (*vtx)[2];
} NavPoly;

/* A stage's navigation set: a g_cpu_stage_nav_sets row (12 bytes: count word,
 * polygon array, zero word). */
typedef struct NavPolySet {
    u8 npolys;
    u8 unk1[3];
    NavPoly *polys;
    u8 unk8[4];
} NavPolySet;

/* One decoded motion frame (0x84 bytes, 33 words): motion_DecodeFrame decodes
 * one stream entry into one, and fighter_Update fills two per call
 * (Unk80101EC8Record.saved_motion_frame holds the one it last used).  Only
 * 16-bit channels, so a whole-frame assignment is the halfword-aligned block
 * copy fighter_Update shows.  root_height is the root offset it negates,
 * root_offset_angle / root_distance the root heading and distance, unk_06..
 * unk_0A the three rotation angles it hands math_RotMatrixZYXAngles.
 * node_channels[] holds signed channels:
 * math_ComposeAxisRotationsAndRewriteAngles reads the triplet at +0x36 with lh,
 * and motion_DecodeFrame sign-extends the 12-bit values at +0x6C..+0x70 /
 * +0x78..+0x7C.
 */
typedef struct MotionFrame {
    s16 root_height;
    u16 root_offset_angle;
    u16 root_distance;
    u16 unk_06;
    u16 unk_08;
    u16 unk_0A;
    s16 node_channels[0x3C];
} MotionFrame; /* sizeof == 0x84 */

/* One cached decoded frame of a character's motion stream (motion_DecodeFrame):
 * the frame number
 * (-2 = empty, motion_InitStream), the decoded pose and per-channel rates (u16
 * channels, the layout of motion_DecodeFrame's `work`), and the bitstream
 * reader state after the frame. */
typedef struct Unk800F1B18Slot {
    s32 frame;      /* +0x000 */
    u16 pose[0x42]; /* +0x004 */
    u16 rate[0x42]; /* +0x088 */
    u32 *ptr;       /* +0x10C next stream word */
    s32 bits;       /* +0x110 valid bits in cur */
    u32 cur;        /* +0x114 unread bits, left-aligned */
} Unk800F1B18Slot;  /* sizeof == 0x118 */

/* g_motion_decoders[slot]: one motion-stream decoder record, slot 0..2
 * (motion_InitStream sets it up, motion_DecodeFrame decodes through it). stream
 * is the motion bitstream (a byte table of 3-byte keyframe offsets at +0x70);
 * pose / code the initial channel values and channel codes motion_InitStream
 * unpacks; ctr the round-robin counter over the four cache slots. */
typedef struct Unk800F1B18Rec {
    u32 *stream;             /* +0x000 */
    u16 pose[0x42];          /* +0x004 */
    u16 code[0x42];          /* +0x088 channel codes (2 bits each) */
    s32 ctr;                 /* +0x10C */
    Unk800F1B18Slot slot[4]; /* +0x110 */
} Unk800F1B18Rec;            /* sizeof == 0x570 */

/* Header of one move record of a character's move script (the u16 stream
 * motion_ResolveMoveData returns pointers into; Unk80101EC8Record.cur_move is
 * the current move, queued_move a buffered one).  event_move_table_id is a move
 * id that resolves to a u16 move-id table indexed by event code,
 * fallback_move_id the move installed when nothing more specific applies (past
 * last_move_frame with no queued move or 0x2000 entry, and on landing),
 * last_move_frame / last_command_frame frame bounds against
 * Unk80101EC8Record.move_frame, unk_09 flag bits (fighter_Update,
 * fighter_BuildCpuPad, fighter_UpdateCpuNavigationChooseScriptsAndHeldInput);
 * transition_commands starts the command list fighter_Update walks: (flags,
 * move id) entries of 2 halfwords, or 4 when flags has a 0xC000 bit, ended by a
 * zero flags word. The two extra halfwords are [mask lo, mask hi], except in a
 * 0x4000 paired entry (motion_FindPairedMoveEntry), whose four bytes are a
 * distance range (min, max, x 20), a target separation (x 20, stored to
 * g_fight_paired_move_separation: the distance
 * fighter_SetBasePositionSeparationCorrectionsAndGetDistance pulls the two
 * fighters toward) and a flags byte (bit 7 tested; the low 7 bits, stored to
 * D_800A38E8, are a frame threshold for
 * fight_UpdateSpacingTurnFacingWeaponBodyHitsAndQueueMoveEvents;
 * fight_UpdatePairedMoves). */
typedef struct MoveScript {
    u16 event_move_table_id;
    u16 fallback_move_id;
    /* row of the bank's 4-byte entry table (fighter_StartMoveScript) */
    u16 motion_bank_entry_index;
    /* first frame: Unk80101EC8Record.move_frame starts here
     * (fighter_StartMoveScript) */
    u8 first_move_frame;
    u8 last_move_frame;
    u8 last_command_frame;
    u8 unk_09;
    u16 transition_commands[1];
} MoveScript;

/* Per-character record table (base 0x80101EC8, stride 0x44C, 2 records;
 * schema: docs/naming/CHAR_STRUCT_SCHEMA.md; the old "practice menu" name was
 * reset by owner ruling Q103). Only the fields reached by C so far are named;
 * the rest is reserved padding. fighter_Init initializes record
 * [fighter_index]. */
typedef struct Unk80101EC8Record {
    /* the other record: [1] for record 0, else [0] (fighter_Init) */
    struct Unk80101EC8Record *other;
    s16 index; /* this record's own index (fighter_Init) */
    /* != 0: fighter_BuildCpuOrMappedPadState hands pad input to
     * fighter_BuildCpuPad */
    s16 cpu_controlled;
    s16 cpu_base_level;
    /* row of D_8008E5CC / D_8008E6A4, index of g_fighter_group_flags */
    s16 character_data_index;
    s16 character_roster_index;
    s16 weapon_id; /* column of D_8008E5CC / D_8008E6A4 */
    u8 unk_10[0x12 - 0x10];
    s16 weapon_entry_id;
    /* -1 == none, else index of g_resource_kind_entry_ids */
    s16 wielded_object_kind;
    u8 unk_16[0x1A - 0x16];
    s16 size_scale;
    s16 base_move_frame_step;
    s16 base_power;
    s16 power;
    u8 unk_22[0x24 - 0x22];
    /* this frame's pad input (fighter_Update copies it whole) */
    PadState pad_input;
    /* frames since the record started (fighter_Update) */
    s32 update_count;
    s16 move_frame;      /* current frame of the move */
    s16 move_frame_frac; /* frame fraction, 0x1000 = one frame */
    s16 move_frame_step; /* frame-fraction step */
    s16 move_frame_unchanged;
    /* motion data id (motion_LoadFighterData);
     * fighter_AssignMotionSlotAndScanDataIdNibbles finds it in
     * g_motion_fighter_loaded_data_ids */
    u16 motion_data_id;
    s16 motion_slot;
    s16 borrow_other_motion;
    u8 unk_4E[0x50 - 0x4E];
    /* current move; last_command_frame bounds move_frame
     * (fighter_UpdateCpuNavigationChooseScriptsAndHeldInput) */
    MoveScript *cur_move;
    u16 *motion_entry;
    /* byte 3 read by fighter_CalcOpponentDistanceThresholdFromTables */
    u8 *motion_metadata;
    u16 motion_entry_index;
    /* 1: the last-resolved move is in g_motion_common_sections
     * (motion_ResolveMoveData's is_common_out flag), 0: in a fighter slot (its
     * own, or the other fighter's when borrow_other_motion is set) */
    s16 move_is_common;
    u8 cur_move_is_common;
    u8 cur_move_slot;
    u8 frame_flags;
    u8 unk_63;
    u16 blend_src_frame_code;
    u16 blend_dst_frame_code;
    s16 pose_interp_weight;
    /* byte 0 of the current move's frame metadata (fighter_StartMoveScript) */
    u16 move_kind;
    s16 prev_move_kind;
    s16 unk_6E;
    s16 unk_70;
    s16 airborne_until_landing;
    s32 saved_collision_y;
    s16 blend_into_move;
    s16 move_started;
    MoveScript *queued_move; /* buffered next move (fighter_Update) */
    s16 queued_move_is_common;
    u16 queued_move_command;
    s16 default_variant_index;
    /* row of unk_3A8 / script_min_distances / script_max_distances /
     * filtered_script_max_distances */
    s16 variant_index;
    s16 object_variant_index;
    s16 object_variant_enabled;
    /* != 0: the second blade (prev_second_weapon_points / SPAD
     * second_weapon_points points) is live */
    s16 wielded_object_live_state;
    s16 unk_8E;
    s16 unk_90;
    s16 weapon_live_state;
    s16 falling_backward;
    s16 is_defeated;
    SVec4i16 horizontal_probe_normal;
    u8 unk_A0;
    u8 weapon_hit_start_frames[2];
    u8 weapon_hit_end_frames[2];
    u8 frame_window_start;
    u8 frame_window_end;
    u8 release_frame;
    u8 object_pickup_frame;
    u8 opponent_turn_frame;
    u8 paired_move_hit_frame;
    u8 opponent_transition_start_frame;
    u8 weapon_reload_frame;
    /* != 0: fight_UpdateFighterInteractionsAndPointCaches re-runs
     * fighter_ResolveWeaponContact for both records
     */
    u8 attack_hit_armed;
    u8 unk_AE;
    u8 unk_AF;
    u8 unk_B0;
    u8 collision_hit_tag;
    u8 collision_hit_class;
    u8 draw_move_set;
    u8 unk_B4;
    u8 unk_B5[0xB8 - 0xB5];
    VECTOR collision_pos;
    VECTOR prev_collision_pos;
    Vec3i32 base_pos;
    u8 unk_E4[0xE8 - 0xE4];
    Vec3i32 root_offset;
    Vec3i32 world_pos;
    u8 unk_100[0x104 - 0x100];
    VECTOR base_velocity;
    /* per blade; fighter_ProcessWeaponBodyHits indexes it with its 0/1 blade
     * flag */
    VECTOR weapon_point_delta[2];
    VECTOR decaying_xz_velocity;
    s32 unk_144;
    s32 ground_y;
    s16 remaining_turn;
    s16 forward_move_rate;
    s16 unk_150;
    s16 use_root_offset_yaw;
    s16 root_offset_yaw;
    s16 base_velocity_damping_x;
    s16 base_velocity_damping_y;
    s16 base_velocity_damping_z;
    u8 unk_15C[0x15E - 0x15C];
    s16 decaying_xz_velocity_damping_x;
    s16 decaying_xz_velocity_damping_y;
    s16 decaying_xz_velocity_damping_z;
    u8 unk_164[0x168 - 0x164];
    Vec3i32 camera_fighter_point;
    Vec3i32 bone_hit_points_4_5_midpoint;
    Vec3i32 bone_hit_point_0;
    Vec3i32 bone_hit_points_1_3_centroid;
    /* fighter_UpdateBoneWorldData: bone 17 / 14 translations,
     * y + ((size_scale * 71) >> 11) */
    Vec3i32 ground_probe_points[2];
    /* fighter_UpdateBoneWorldData: probe end y for ground_probe_points[i]:
     * the hit y of the collision_TestSegmentWithWork down-segment, else the
     * segment's lower end (probe y + 2000) */
    s32 ground_probe_end_ys[2];
    u8 unk_1B8[0x1BA - 0x1B8];
    /* fighter_UpdateBoneWorldData: ratan2 heading of bone 17's matrix column 2,
     * + 0x800 */
    s16 unk_1BA;
    u8 unk_1BC[0x1C2 - 0x1BC];
    s16 unk_1C2; /* the same for bone 14 */
    u8 unk_1C4[0x1C8 - 0x1C4];
    SVec4i16 orientation;
    SVec4i16 unk_1D0;
    s16 opponent_bearing;
    s16 bone_triangle_cross_y_positive;
    s16 downward_probe_hit;
    u8 unk_1DE[0x1E6 - 0x1DE];
    s16 eased_pose_bearing_offset;
    s16 eased_pose_elevation_offset;
    s16 opponent_elevation_offset;
    /* fighter_UpdateBoneWorldData: bone 11's matrix applied to g_unit_y_vector
     * (0, 0x1000, 0) */
    Vec3i32 bone_y_axis;
    Vec3i32 prev_root_offset;
    u8 unk_204[0x210 - 0x204];
    /* fight_UpdateFighterInteractionsAndPointCaches: copy of scratchpad points
     * 0x1F800000 + idx * 0x24 */
    Unk80107850Rec prev_weapon_points[3];
    /* fight_UpdateFighterInteractionsAndPointCaches: copy of scratchpad points
     * 0x1F800048 + idx * 0x18 */
    Unk80107850Rec prev_second_weapon_points[2];
    VECTOR unk_24C;
    /* fighter_Update: copy of scratchpad point weapon_points[idx][0] */
    Unk80107850Rec opponent_elevation_origin;
    s32 elevation_origin_from_weapon;
    s16 unk_26C;
    s16 unk_26E;
    s16 unk_270;
    s16 unk_272;
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 pending_move_event;
    /* per blade (weapon_hit_start_frames / weapon_hit_end_frames bounds),
     * fighter_Update */
    s16 weapon_hit_phases[2];
    s32 mash_contest_score;
    /* the motion frame fighter_Update last used */
    MotionFrame saved_motion_frame;
    u16 saved_frame_offset_yaw;
    u16 saved_motion_frame_id;
    s16 pose_blend_progress;
    s16 pose_blend_requests;
    s16 pose_blend_step;
    u8 unk_31E[0x320 - 0x31E];
    Vec3i32 pose_blend_base_shift;
    u8 unk_32C[0x330 - 0x32C];
    s16 carried_object_count;
    /* queued kinds, carried_object_count of them (object_InitKindQueue /
     * object_QueueKind)
     */
    s16 carried_object_kinds[12];
    u8 weapon_ammo;
    u8 weapon_refills_left;
    u8 reposition_count;
    u8 throw_charges_left;
    /* written by fighter_BuildCpuOrMappedPadState for the OTHER record */
    u8 unk_34E;
    u8 unk_34F[0x350 - 0x34F];
    s16 bone_event_counter;
    /* index into node_GetSlotMatrixPointerArray()'s MATRIX * table and
     * SPAD->bone_hit_points[] (fighter_DispatchArmedBoneVectorEventAndDisarm)
     */
    s16 bone_event_bone_index;
    /* fighter_StoreBoneLocalVectorAndArmEventTrigger's gte_stlvnl output;
     * fighter_DispatchArmedBoneVectorEventAndDisarm's gte_ldlv0 input */
    Vec3i32 bone_event_local_vector;
    CpuRoute cpu_route; /* 0x360 */
    s32 cpu_action_index;
    s16 cpu_action_countdown;
    s16 cpu_action_cooldown_frames;
    u8 cpu_action_from_opponent;
    u8 cpu_orbit_route_active;
    s16 unk_39E;
    s16 unk_3A0;
    s16 unk_3A2;
    /* script list: u16 byte offsets from its own start */
    u16 *unk_3A4;
    /* per-row cursor into unk_3A4 (fighter_InitCpuControl) */
    u16 *unk_3A8[3];
    /* script pointer (fighter_StartCpuPadScript / fighter_StepCpuPadScript) */
    u8 *cpu_pad_script_cursor;
    u8 cpu_pad_script_delay_left;
    u8 unk_3B9[0x3BC - 0x3B9];
    u8 cpu_pad_script_kind;
    u8 unk_3BD;
    u8 unk_3BE[0x3C0 - 0x3BE];
    u8 unk_3C0;
    u8 unk_3C1;
    s16 unk_3C2;
    s32 unk_3C4;
    s32 cpu_pad_script_held;
    s32 cpu_decided_pad_held;
    /* the pad record fighter_BuildCpuPad builds for
     * fighter_BuildCpuOrMappedPadState */
    PadState last_cpu_pad;
    u16 cpu_pad_build_count;
    u8 unk_3EA[0x3EE - 0x3EA];
    s16 unk_3EE;
    s16 cpu_distance_error_streak;
    u8 unk_3F2;
    u8 cpu_threat_resolved_count;
    u8 unk_3F4;
    u8 cpu_script_wait_frames;
    u8 cpu_script_wait_limit;
    u8 unk_3F7;
    /* [variant_index] distance bounds (fighter_InitCpuControl) */
    s16 script_min_distances[3];
    s16 script_max_distances[3];
    s16 filtered_script_max_distances[3];
    s16 scale_distance_offset;
    s8 last_chosen_script_index;
    s8 last_chosen_script_list_index;
    s16 chosen_script_start_x;
    s16 chosen_script_start_z;
    s16 chosen_script_radius;
    /* fighter_BuildCpuPad: 8 (move key, count) pairs */
    u8 cpu_tracked_move_tallies[8][2];
    u8 cpu_tracked_move_tally_threshold;
    u8 incoming_projectile_state;
    u8 incoming_attack_state;
    u8 opponent_snapshot_hit_start_frame;
    s16 cpu_tracked_move_key;
    s16 opponent_snapshot_x;
    s16 opponent_snapshot_z;
    s16 opponent_snapshot_radius;
    s32 cpu_state_flags;
    s32 nearest_nav_crossing_distance;
    s16 cpu_level;
    s16 opponent_angle_from_orientation;
    s16 opponent_angle_from_orientation_abs;
    s16 unk_43E;
    u8 latched_motion_metadata_nibble;
    u8 motion_metadata_nibble;
    u8 opponent_ground_y_band;
    u8 cpu_profile;
    u8 cpu_probes[8]; /* fighter_UpdateCpuProbes's per-direction results */
} Unk80101EC8Record;  /* sizeof == 0x44C */

/* 14-byte record table indexed by the kind field (+0x2) of the 0x64-byte
 * objects at g_object_pool: object_SpawnFromFighter and
 * object_ResolveFighterHits index it with stride 14, object_UpdatePool reads
 * +0x0 / +0xA of a record. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 launch_speed;
    s16 launch_vel_y;
    u16 spin_rate;
    s16 bounce_restitution;
    u8 unkC;
    u8 unkD;
} Tbl8008E194;

/* The twelve 0x64-byte object records at 0x80106A78. object_SpawnFromFighter
 * spawns one (its kind indexes g_object_kind_params /
 * g_resource_kind_entry_ids), object_UpdatePool moves them,
 * object_ResolveFighterHits tests them against both fighters,
 * object_SubmitPoolTransforms hands them to the effect calls; kind == -1 marks
 * a free record.
 */
typedef struct {
    /* 0 at spawn; object_UpdatePool adds 1 per step while is_moving != 0 */
    s16 moving_ticks;
    s16 kind; /* -1 = free */
    u8 can_hit_fighter;
    u8 unk_05;
    /* .index of the spawning record (object_SpawnFromFighter) */
    u8 owner;
    u8 drop_state;
    u8 is_attached;
    /* index into the opponent's bone-matrix table,
     * node_GetSlotMatrixPointerArray(owner < 1) (object_AttachToOpponentBone,
     * object_SubmitAttachedTransform) */
    u8 attached_bone;
    /* own index in g_object_pool once allocated; 0xFF = free (set by
     * object_ResetPool at init and by object_UpdatePool once kind == -1) */
    u8 slot;
    u8 model_entry_id;
    /* object_AttachToOpponentBone builds it (identity, RotMatrixX/Y/Z,
       MulMatrix0); object_SubmitAttachedTransform reads it */
    MATRIX mtx;
    Vec3i32 pos;      /* += vel each object_UpdatePool step */
    Vec3i32 prev_pos; /* pos before the step */
    Vec3i32 vel;
    s32 is_moving; /* != 0: moving */
    /* RotMatrixX/Y/Z angles; += rot_vel each step */
    s16 rot[3];
    u8 unk_5A[0x5C - 0x5A];
    s16 rot_vel[3];
    u8 unk_62[0x64 - 0x62];
} Obj80106A78; /* sizeof == 0x64 */

/* 2-byte {a,b} threshold pairs. g_snd_stage_sound_slot_trigger_delays: indexed
   by (type - 2), 5 entries (types 2..6); g_snd_stage_sequence_cue_ticks:
   indexed by the category snd_GetStageSoundIndex returns, 6 entries (both read
   by snd_UpdateStageSounds). */
typedef struct {
    u8 a;
    u8 b;
} Unk8008EA44Entry;

/* 6-row tables fight_UpdatePairedInputTotalsEventsAndMidpointEffects selects by
 * D_80101EC8[0].move_kind mode (row 0..5): unk0 scales the Judge sin/cos
 * offset, unk2 is added to y; g_fight_midpoint_effect_event_ids[row] is passed
 * as fight_RecordReplayEventAndDispatchSoundOrEffect's arg1. */
typedef struct {
    s16 unk0;
    s16 unk2;
} Tbl8008EB54Entry;

/* 0x18-byte record table at 0x80099D88 (behaviour flags and random-pick
 * weights), indexed by Unk80101EC8Record.cpu_profile in fighter_InitCpuControl,
 * fighter_StepCpuPadScript, fighter_BuildCpuPad and
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput. */
typedef struct Unk80099D88Rec {
    u16 flags;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 script_distance_fraction;
    u8 unk6;
    u8 unk7;
    /* +0x8: fighter_UpdateCpuNavigationChooseScriptsAndHeldInput scales a
       random 12-bit draw by an element, and compares [4] with a random 8-bit
       draw */
    u8 pick_weight[7];
    /* +0xF: fighter_UpdateCpuNavigationChooseScriptsAndHeldInput scales a
       random 12-bit draw by an element, and compares [5] >> 2 and [6] >> 2 with
       a random 8-bit draw */
    u8 script_kind_weight[8];
    u8 unk17;
} Unk80099D88Rec; /* size 0x18 */

/* Rows of eight 4-byte entries starting at 0x8009A8C8: each 0x20-byte row ends
 * with a zero entry (0x8009A8E4 / 0x8009A904 / 0x8009A924). Both readers index
 * the column 1-based, [row][g_fight_stage_command_count - 1]:
 * fighter_InitCpuControl reads unk0/unk1,
 * fighter_UpdateCpuNavigationChooseScriptsAndHeldInput the mask halfword. */
typedef struct Unk8009A8C8Entry {
    u8 unk0;
    u8 unk1;
    u16 mask;
} Unk8009A8C8Entry;

/* 0x44-byte record shared by the two camera-target objects at 0x800F5328 and
 * 0x800F6608 (camera_InitTwoFighterView / camera_InitTrackedFighterFocus
 * initialise them). +0x00 is one 12-byte vector, copied whole by
 * camera_SetFighterFocusView / camera_SetInterpolatedFighterView. +0x10 is the
 * camera rotation: camera_CalcEye builds the matrix from vx / vy / vz and
 * gpu_ResetDrawQueueAndPrepareViewAndLighting takes it with the +0x00 vector;
 * its pad (+0x16) is unused. */
typedef struct Rec44 {
    Vec3i32 focus_point;
    s32 wC;
    SVECTOR camera_angles;
    s32 w18;
    s16 h1C;
    u8 b1E;
    u8 b1F;
    s32 w20;
    s32 w24;
    s32 w28;
    s32 w2C;
    /* two limit vectors; camera_UpdateTwoFighterView passes h30[p] to
     * math_StoreFloorDiv2000XZOfPointStepTowards */
    s16 h30[2][4];
    u8 b40;
    u8 b41;
    u8 b42;
    u8 b43;
} Rec44;

/* The camera-target offset at 0x800FF5C8: camera_ApplyFightView /
 * camera_ApplyReplayView add its fields to a Rec44's focus_point, camera_angles
 * and w18, and camera_UpdatePadOffset zeroes them. The fields sit at Rec44's
 * offsets; +0x0C and the SVECTOR pad are never accessed. */
typedef struct Unk800FF5C8Rec {
    Vec3i32 focus_point_offset;
    u8 unk_0C[4];
    SVECTOR camera_angles_offset;
    s32 w18;
} Unk800FF5C8Rec;

/* 0x1C-byte record: one player's state in one frame of the replay buffer
 * g_replay_fighter_frames (frames of two, Rec1C[2] = 0x38 bytes;
 * replay_RecordFighter writes player a0->index's record of frame
 * g_replay_write_frame from the character record, replay_PlayFrame reads both
 * back and passes two of them to camera_UpdateReplayViewWithSineYawOffset /
 * camera_UpdateReplayViewWithConstantYawOffset,
 * motion_ApplyRecordedPoseAdjustments reads b14..b16 / b18).
 * - w0: the character's current move (Unk80101EC8Record.cur_move);
 * replay_PlayFrame reads its motion_bank_entry_index.
 * - h4 / h6 / h8: world_pos x / y / z; hA: orientation.vy & 0xFFF with
 * draw_move_set in the top four bits; hC: ground_y; hE / h10 / h12:
 * blend_src_frame_code / blend_dst_frame_code / pose_interp_weight.
 * - b14..b16: eased_pose_bearing_offset / eased_pose_elevation_offset /
 * opponent_elevation_offset >> 2, read back signed; b17: bit 0 =
 * cur_move_is_common != 0, bit 1 = cur_move_slot != 0; b18: frame_flags; b19:
 * move_frame. */
typedef struct Rec1C {
    MoveScript *w0;
    s16 h4;
    s16 h6;
    s16 h8;
    u16 hA;
    s16 hC;
    s16 hE;
    s16 h10;
    s16 h12;
    s8 b14;
    s8 b15;
    s8 b16;
    u8 b17;
    u8 b18;
    u8 b19;
    u8 pad1A[2];
} Rec1C;

/* The 0xB4 0x10-byte records of the event table g_replay_model_draws
 * (replay_Reset clears draw_age to -1, replay_BeginFrame ages them,
 * replay_RecordModelDraw adds or refreshes one, replay_PlayFrame replays them).
 * draw_age: -1 = free, else a frame counter; draw_span: frames seen;
 * model_entry_index: replay_RecordModelDraw's arg1; draw_pos: the s16 x / y / z
 * of arg2; slot_rot_x: arg3[0] & 0xFFF with arg0 in the top four bits;
 * draw_rot_y / draw_rot_z: arg3[1] / arg3[2]. */
typedef struct Unk800F68E0Rec {
    s16 draw_age;
    u8 draw_span;
    u8 model_entry_index;
    s16 draw_pos[3];
    u16 slot_rot_x;
    s16 draw_rot_y;
    s16 draw_rot_z;
} Unk800F68E0Rec;

/* The 0x20 0x10-byte records of the event table g_replay_events (replay_Reset
 * sets event_frame to 0xFF = free, replay_BeginFrame frees the current frame's,
 * replay_RecordEvent fills one, replay_PlayFrame replays those whose
 * event_frame is the frame index). event_frame: the frame
 * (g_replay_write_frame); unk_1 / event_kind: replay_RecordEvent's arg0 / arg1;
 * event_pos: the s16 x / y / z of arg2; event_vector: arg3[0..2]. */
typedef struct Unk80101BF0Rec {
    u8 event_frame;
    u8 unk_1;
    u8 event_kind;
    u8 pad3;
    s16 event_pos[3];
    u16 event_vector[3];
} Unk80101BF0Rec;

/* The 4-byte record fight_SplitFrameCounterIntoMinutesSecondsHundredths fills
 * from the frame counter g_fight_timer_frames (30 per second): time_minutes =
 * count / 1800, time_seconds = count / 30 % 60, time_hundredths = count % 30 *
 * 100 / 30 (99 / 59 / 99 past 0x2BF1F). gpu_DrawTimeFieldsAndDecimalCounter
 * draws the three as two-digit numbers;
 * fight_PackTimeRecordsAndInitPresentation copies them into its save record at
 * +0x2D..+0x2F. fight_UpdateModeDisplaysAndMatchProgress also fills only
 * time_seconds = n / 30 and time_hundredths = n % 30
 * * 100 / 30 from a frame countdown n, for gpu_DrawPairedMarkRowsAndTimeFields
 * to draw. */
typedef struct Unk8001CD68Rec {
    s16 time_minutes;
    u8 time_seconds;
    u8 time_hundredths;
} Unk8001CD68Rec;

/* Two 0x58-byte records set up side by side by
 * node_InitRootNodePairWithNegatedRotationMatrices: 0x80101DF0 (node_kind =
 * 0x64, rotation_order = 5; pointer stored to g_view_camera_root_ptr) and
 * 0x800FF638 (node_kind = 0x65, rotation_order = 2; pointer stored to
 * g_view_identity_root_ptr). node_SetMatrixFromNegatedRot passes a negated
 * stack copy of xf.rot and &work to g_math_rot_matrix_funcs[rotation_order],
 * then copies work to xf.mat; gpu_ResetDrawQueueAndPrepareViewAndLighting
 * writes work.t just before that call. node_OrientAndQueueViewOffsetNode passes
 * &xf.rot to g_math_rot_matrix_funcs[0] and &xf.mat to ApplyMatrix.
 * g_view_offset_node and g_wave_draw_node are two more records of this layout:
 * node_InitIdentity initialises the first, wave_Init sets up the second and
 * passes it to node_UpdateWorldMatrix, which reads matrix_state. rot is the
 * RotMatrix-style angle SVECTOR the rotation handlers take; mat and work are
 * libgte MATRIXes.
 */
typedef struct {
    SVECTOR rot; /* +0x00 */
    MATRIX mat;  /* +0x08 */
} Unk80101DF0Xform;

typedef struct Unk80101DF0Record {
    u8 node_kind;              /* +0x00 */
    s8 primitive_table_select; /* +0x01 */
    s16 reloc_entry_index;     /* +0x02 */
    s16 reloc_block_id;        /* +0x04 */
    s16 matrix_state;          /* +0x06 */
    /* +0x08 g_math_rot_matrix_funcs index */
    s16 rotation_order;
    s16 light_setup_index;                 /* +0x0A */
    struct Unk80101DF0Record *parent_node; /* +0x0C */
    Unk80101DF0Xform xf;                   /* +0x10 */
    MATRIX work;                           /* +0x38 */
} Unk80101DF0Record;                       /* 0x58 */

/* The 8 light-setup records at 0x800F62E0 (0x60 each; base + n*0x60 in
 * math_LightPlaneProject, light_InitDefaults).  light_Unpack fills one from a
 * 22-halfword template: the color matrix +0x38 (m[3][3]), the three lights'
 * pitch / yaw / on at +0x00 / +0x08 / +0x10, the u8 back color +0x58 and the
 * s16 +0x5C; calc_LightMatrix then turns light i's pitch / yaw into the light
 * matrix row i
 * (+0x18 + i*6, zero when off), scaled by +0x5C.  light_InitDefaults hands
 * record 0's color matrix and back color to SetColorMatrix / SetBackColor; the
 * light matrix goes to gte_MulMatrix0ClearTrans (func_8004A940);
 * math_SetLightPlaneProjection / math_LightPlaneProject read its row 0. */
typedef struct {
    s16 pitch;
    s16 yaw;
    s16 on;
    s16 pad;
} Unk800F62E0Light;

typedef struct {
    Unk800F62E0Light light[3]; /* +0x00 */
    MATRIX lmat;               /* +0x18 */
    MATRIX cmat;               /* +0x38 */
    u8 back[3];                /* +0x58 */
    s16 unk5C;                 /* +0x5C */
} Unk800F62E0Rec;              /* 0x60 */

/* 0x800F66A0: the rotation-to-matrix handlers a transform node's rotation_order
 * selects (rot -> matrix, PsyQ RotMatrix shape).  math_InitRotMatrixFuncs fills
 * [0] ZYX, [2] ZXY, [4] YXZ, [5] XYZ ([1] and [3] are never written); the nodes
 * set rotation_order to 0, 2, 4 and 5, and
 * gpu_UnpackGridRecordsNodesAndCellRuns / node_UpdateWorldMatrix /
 * node_SetMatrixFromNegatedRot / node_InitIdentity index it by rotation_order
 * (4-byte stride).  _svm_vab_vh follows at 0x800F66B8. */
typedef void (*RotMatrixFunc)(SVECTOR *, MATRIX *);

/* The 0x68-byte records of the table gpu_GetGridNodeRecordTable returns: a
 * transform node of the Unk80101DF0Record layout, then a byte flag.
 * gpu_UnpackGridRecordsNodesAndCellRuns fills them from a stream and calls
 * g_math_rot_matrix_funcs[rotation_order] on &xf.rot / &xf.mat (the angles as
 * stored, straight into xf.mat; unlike node_SetMatrixFromNegatedRot, no
 * negation and no work);
 * gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards and
 * gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes queue each
 * one on g_draw_queue at most once, guarded by unk58.
 */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    u8 unk58;               /* +0x58 */
    u8 unk59[0xF];          /* +0x59 */
} Unk800A6690Rec;           /* 0x68 */

/* The 0x68-byte entries of the g_node_frame_animation_context.frame_entries
 * table (the buffer node_BindFrameDataAndReserveEntryBuffer is handed; it
 * returns the end, base + count * 104). Each is a transform node of the
 * Unk80101DF0Record layout paired with one gpu_GetGridNodeRecordTable entry of
 * the same index (g_node_frame_animation_context.paired_grid_nodes):
 * node_InitFrameEntriesFromWorldTranslations initialises the node and copies
 * the paired entry's xf.mat.t into work.t;
 * node_StartFrameAnimatedEntryAndInitPairedNode /
 * node_AdvanceFrameAnimatedEntriesAndQueueDraws run the node from the
 * g_node_frame_animation_context.frame_data key frames and queue it on
 * g_draw_queue. unk58 is a word frame counter (lw/sw: -1 idle, 0 started by
 * node_StartFrameAnimatedEntryAndInitPairedNode, -2 done), anim_yaw the word Y
 * angle node_StartFrameAnimatedEntryAndInitPairedNode stores, unk60 an s16
 * node_AdvanceFrameAnimatedEntriesAndQueueDraws sets to 0x1000 at the last key
 * frame and then lowers by 0x40 per call to 0 (at 0, unk58 = -2 unless
 * g_node_frame_animation_context.resource_selector is 4); +0x62..0x67 are not
 * accessed. Not the Unk800A6690Rec layout: that table's +0x58 is a byte. */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 anim_yaw;           /* +0x5C */
    s16 unk60;              /* +0x60 */
    s8 pad62[6];            /* +0x62 */
} Unk800A9CF8Entry;         /* 0x68 */

/* The 21 ten-byte node-template records at 0x80094B96 (up to
 * g_object_scale_factors): node_InitObjectHierarchyFromTemplates builds
 * Unk80045878Obj.transform_nodes[i] from record i (unk_00: the parent node
 * index, -1 for none; unk_02: the position, scaled by position_scale),
 * node_BuildProjectedNodeList copies unk_08 (-1: no node) into
 * projected_nodes[]. */
typedef struct {
    s16 unk_00;
    s16 unk_02[3];
    s16 unk_08;
} Unk80094B96Rec;

/* The per-slot model object
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock builds (0x1A88
 * bytes, its arena_SetBlockEnd block) and g_slot_objects[] / slot_GetObject
 * hand out. The header (0x00..0x2B) is followed by three arrays of 0x68-byte
 * transform nodes (slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers
 * rebases them as 0x15, 0x14 and 0x14 records), then three 20-entry pointer
 * tables.
 * - load_flags: flag word; slot_FinalizeObjectLoadSoundTexturesAndArena sets
 * bit 1 and keeps the TIM-group request (g_slot_tim_group_requests[arena_id] &
 * 0x1F) in bits 16..20, which slot_RequestTimGroupsAndMarkReloadOnChange /
 * slot_GetLastLoadTimGroupRequest read as (word >> 16) & 0x1F; no code reads it
 * through half[].
 * - arena_id: slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a0,
 * the arena_OpenBlock id and the g_slot_objects index; load_status: 0, 1 after
 * a fresh load (slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock), -2
 * to request a reload (slot_RequestTimGroupsAndMarkReloadOnChange);
 * slot_FinalizeObjectLoadSoundTexturesAndArena returns at once when it is 0,
 * else clears it and runs its setup; asset_index:
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a1, indexing the
 * g_object_scale_factors / g_gpu_asset_draw_move_masks tables.
 * - node_light_setup_index / transform_block_id / projected_block_id:
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a0 / a0 / a0 + 3
 * (transform_block_id / projected_block_id are the reloc_RelocateBlock /
 * gpu_OffsetBlockTexPrimStreams ids); position_scale: the Q12 scale
 * node_InitObjectHierarchyFromTemplates sets from g_object_scale_factors.
 * - patch_color: written as the word 0x8000 by
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock, read as three
 * colour bytes by node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor.
 * - asset_block / staging_buffer: the resource blocks
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock stores;
 * softbody_block / softbody_params: the attached Unk8003F62CRec block and the
 * s16 list softbody_SetupObject walks up to its -3 end (softbody_RebaseObject
 * moves both by its a1).
 * - transform_nodes[]: node 0 is the root
 * (node_QueueEntryNodesFromTransformOrUpdatedSlotNode reads its xf.mat);
 * node_InitObjectHierarchyFromTemplates builds 0..20,
 * node_BuildProjectedNodeList fills projected_nodes[] (reloc_entry_index = -1
 * ends it), node_BuildIdToNodeMapWithDuplicates copies duplicates into
 * duplicate_nodes[] (unk58 = the source node; 0 ends it).
 * - unk_18F4[] / node_matrix_ptrs[]: &node.xf.mat of transform_nodes[] nodes
 * (node_RefreshObjectMatrixPointerTables); node_by_id[]: the node each id
 * selects (node_BuildIdToNodeMapWithDuplicates); unk_1A84: func_80040D48's
 * arg5. */
typedef struct Unk80045878Node {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 unk5C;              /* +0x5C */
    /* +0x60 softbody_Create's node/edge buffer */
    u8 *softbody_nodes;
    s32 unk64;     /* +0x64 */
} Unk80045878Node; /* 0x68 */

typedef struct Unk80045878Obj {
    union {
        s32 word;
        s16 half[2];
    } load_flags; /* +0x0000 */

    s16 arena_id;               /* +0x0004 */
    s16 load_status;            /* +0x0006 */
    s16 asset_index;            /* +0x0008 */
    u8 pad0A[6];                /* +0x000A */
    u16 node_light_setup_index; /* +0x0010 */
    s16 position_scale;         /* +0x0012 */
    s16 transform_block_id;     /* +0x0014 */
    s16 projected_block_id;     /* +0x0016 */

    union {
        s32 word;
        u8 byte[4];
    } patch_color; /* +0x0018 */

    s32 asset_block;                     /* +0x001C */
    s32 staging_buffer;                  /* +0x0020 */
    void *softbody_block;                /* +0x0024 */
    s32 softbody_params;                 /* +0x0028 */
    Unk80045878Node transform_nodes[21]; /* +0x002C */
    Unk80045878Node projected_nodes[20]; /* +0x08B4 */
    Unk80045878Node duplicate_nodes[20]; /* +0x10D4 */
    MATRIX *unk_18F4[20];                /* +0x18F4 */
    u8 pad1944[0x50];                    /* +0x1944 */
    MATRIX *node_matrix_ptrs[20];        /* +0x1994 */
    u8 pad19E4[0x50];                    /* +0x19E4 */
    Unk80045878Node *node_by_id[20];     /* +0x1A34 */
    s16 unk_1A84;                        /* +0x1A84 */
    u8 pad1A86[2];                       /* +0x1A86 */
} Unk80045878Obj;                        /* 0x1A88 */

/* The 16-byte records gpu_UnpackGridRecordsNodesAndCellRuns fills ahead of
 * those (cell_world_x / cell_world_z = the grid cell's column / row * 2000 -
 * 32000); gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards
 * and gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes set
 * unk6 and the draw_flags bits and queue them on g_draw_queue. */
typedef struct {
    s16 draw_kind;              /* +0x00 */
    s16 cell_reloc_block_id;    /* +0x02 */
    u16 cell_reloc_entry_index; /* +0x04 */
    u8 unk6;                    /* +0x06 */
    u8 draw_flags;              /* +0x07 */
    s32 cell_world_x;           /* +0x08 */
    s32 cell_world_z;           /* +0x0C */
} Unk800A4750Rec;               /* 0x10 */

/* One entry of the CD file table (all 159 entries equal the disc's ISO9660
 * directory records). */
typedef struct {
    CdlLOC loc; /* the file's start position */
    u32 size;   /* the file's length in bytes */
} CdFileEntry;

/* The CD read / CD audio words at 0x80101E60..0x80101EA7: the tail of the
 * CD state block g_cdrom_state (CdState below). Member widths follow the
 * original accesses; 0x80101E91..93 is alignment padding.
 * report_timeout..end_countdown belong to the object because cdrom_UpdateAudio
 * (compiled -G8) keeps the address of its accesses to report_timeout /
 * end_countdown in a register only for a variable larger than 8 bytes. unk3E
 * (u16) and expected_pos (s32; no access reveals its signedness) lie in that
 * span but cdrom_UpdateAudio never touches them; they are typed by their other
 * users' accesses (Q13/Q14). `pair` holds a copy of a g_cd_file_table element
 * (a CdFileEntry), copied whole by cdrom_StartRead / cdrom_StartAudio. */
typedef struct {
    s16 file_index;          /* 0x80101E60 */
    s16 io_state;            /* 0x80101E62 */
    s16 audio_hold;          /* 0x80101E64 */
    s16 unk06;               /* 0x80101E66 */
    s16 stop_request;        /* 0x80101E68 */
    s16 audio_repeat;        /* 0x80101E6A */
    CdFileEntry pair;        /* 0x80101E6C .. 0x80101E73 */
    s32 audio_end_pos;       /* 0x80101E74 */
    s32 read_sector_count;   /* 0x80101E78 */
    s32 read_dest_addr;      /* 0x80101E7C */
    s32 cd20;                /* 0x80101E80 */
    s32 dest_buffer;         /* 0x80101E84 */
    s32 audio_step_count;    /* 0x80101E88 */
    s32 command_delay_count; /* 0x80101E8C */
    u8 audio_mode;           /* 0x80101E90 */
    s32 is_xa_audio;         /* 0x80101E94 */
    s16 read_timeout;        /* 0x80101E98 */
    s16 getloc_timeout;      /* 0x80101E9A */
    s16 report_timeout;      /* 0x80101E9C */
    u16 unk3E;               /* 0x80101E9E */
    s32 expected_pos;        /* 0x80101EA0 */
    s32 end_countdown;       /* 0x80101EA4 */
} Unk80101E60Rec;

/* The CD module's state block, one 0x50-byte object at 0x80101E58 (span
 * bounded by owner ruling Q43). cdrom_StartAudio forms the CdlSetfilter
 * parameter (file, chan) as &rec.io_state - 0xA and reloads rec.file_index only
 * after its CdFileEntry copy into rec.pair (one object for the scheduler);
 * cdrom_Update forms &rec.pair off later members. */
typedef struct {
    u8 file;             /* 0x80101E58 */
    u8 chan;             /* 0x80101E59 */
    s32 sync_wait_count; /* 0x80101E5C */
    Unk80101E60Rec rec;  /* 0x80101E60 .. 0x80101EA7 */
} CdState;

/* The parameter block 0x80102778..0x80102787 (fight_InitSettings sets
 * every byte of it except 0x82/0x83): two u16 values, three per-player byte
 * pairs kept as one array (unk_4[2 * k + player], [0] = P1, [1] = P2), a
 * fourth per-player pair only other functions touch (operand_cpu_base_levels,
 * indexed by player in fighter_ConfigureCpuControl) and four single bytes. One
 * object: menu_PrintEditAndApplyConfigWithPerPadCursors reaches unk_4 and
 * pending_view_setting as offsets from the address of pending_stage_index.
 */
typedef struct {
    u16 cpu_base_levels[2];
    u8 unk_4[6];
    u8 operand_cpu_base_levels[2];
    u8 pending_stage_index;
    u8 pending_mode_selector;
    u8 pending_view_setting;
    u8 unk_F;
} Unk80102778Rec;

typedef struct {
    s32 pos[3];
    s32 vel_x;
    s32 vel_y;
    s32 vel_z;
    s32 field_18;
    s32 a_edge_count; /* entries used in a_edges */
    s32 b_edge_count; /* entries used in b_edges */
    u8 a_edges[8];    /* edges whose node a is this record */
    u8 b_edges[8];    /* edges whose node b is this record */
    s32 field_34;
    s32 field_38[2];
} Func80017A44Record;

typedef struct {
    s32 dist;

    union {
        s32 pair; /* a << 16 | b */

        struct {
            u16 b;
            s16 a;
        } node;
    } ends;

    s32 field_8;
    s32 group_id;
} Func80017848Edge;

/* 6CF8's object record (sizeof = 0x34): g_softbody_slots holds eight,
 * softbody_Create fills a free one from a Func80017A44Input and softbody_Build
 * builds its node / edge graph; 87A0 steps it (softbody_UpdateList by each
 * Unk80017FA0Rec's slot_index). points: the input's point table, 0 = free
 * (softbody_Free); count: its node count; flags: the input's flags; nodes /
 * edges: the node and edge tables (edges right after the count 0x40-byte
 * nodes); matrix: a copy of the input's matrix. */
typedef struct {
    SVECTOR *points;
    s16 count;
    s16 edge_count;
    s32 flags;
    Func80017A44Record *nodes;
    Func80017848Edge *edges;
    MATRIX matrix;
} Func80017A44Output;

/* The 16-byte object descriptor (2B344 softbody_CreateForNode fills it in each
 * Unk8003F824Rec, softbody_SetupObject copies it into
 * Unk8003F62CRec.step_descs; 87A0 softbody_UpdateList steps one object per
 * descriptor): the object's g_softbody_slots index (slot_index), its flags
 * (snap; bit 0 picks softbody_SnapToPoints over softbody_Step), the object's
 * matrix, its point table end and its collision-volume block. */
typedef struct {
    /* 0x00 */ s16 slot_index;
    /* 0x02 */ u8 snap;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ MATRIX *object_matrix;
    /* 0x08 */ u8 *unk8;
    /* 0x0C */ struct Unk8003F6D8Coll *colliders;
} Unk80017FA0Rec;

/* The collision volumes of one Unk8003F824Rec (2B344 Func8003F6D8Inner +0x10;
 * Unk80017FA0Rec.colliders points here): unk00 (softbody_SetObjectGround sets
 * it; 87A0 softbody_LoadColliders scales it into the scratchpad's ground word),
 * the volume count, each volume's two foci (gte_SetMatrixRotTransIR writes each
 * as three words) and its bound (unk68). */
typedef struct Unk8003F6D8Coll {
    /* 0x00 */ s32 unk00;
    /* 0x04 */ s32 count;
    /* 0x08 */ VECTOR foci[3][2];
    /* 0x68 */ s32 unk68[3];
} Unk8003F6D8Coll;

/* softbody_Create's argument (2B344 softbody_CreateForNode builds it on its
 * stack): the point table and its count, the flags, the group list, the
 * object's matrix and the buffer the node / edge tables go to. */
typedef struct {
    s16 count;
    s16 flags;
    SVECTOR *points;
    s16 *groups;
    MATRIX *matrix;
    u8 *buf;
} Func80017A44Input;

typedef struct Size2s16 {
    s16 x;
    s16 y;
} Size2s16;

/* The record gpu_DrawIndexedCellsAndDecimalDigitPairs reads: three frame sheets
 * (unk_00), the name sheet and the per-character name cells (character_sheet /
 * character_cells, three cells per character after a first group), the digit
 * sheet and its cells (digit_sheet / digit_cell; the walker shifts the sheet's
 * ubase per digit), and per-character x offsets (character_x_offsets). 4D0A8
 * hands it D_8009B0C0, menu_DrawConfirmSheetsAndDigitPairs MOD.BIN's unk_3C. */
typedef struct {
    Unk8009B0E0Record *unk_00[3];
    Unk8009B0E0Record *character_sheet;
    Unk8009B400Record *character_cells;
    Unk8009B0E0Record *digit_sheet;
    Unk8009B400Record *digit_cell;
    s16 *character_x_offsets;
} Unk8006D808Rec;

/* MOD.BIN's unk_3C list: gpu_DrawIndexedCellsAndDecimalDigitPairs's set, then
 * the three sheets menu_DrawConfirmSheetsAndDigitPairs draws. */
typedef struct {
    Unk8006D808Rec unk_00;
    Unk8009B0E0Record *unk_20[3];
} Unk8006DD94List;

/* MOD.BIN (resource file 2), the root 596CC's
 * menu_LoadResourcesInitStateSelectFirstEnabledRow loads at its work area +
 * 0x58; it keeps it in g_row_menu_work_block's word 9 (menu_resource_root), and
 * gpu_AdvanceBufferCounterAndBuildDrawContext copies that into word 1 of the
 * draw context. After the head:
 * - unk_14..cursor_row_sheets (+0x14..+0x40): the twelve lists
 * reloc_RelocateTwelveListsAndLoadVabPack
 * relocates (reloc_RelocateOffsetList: entries up to a 0 word, -1 entries
 * skipped). Their entries are sprite sheets (unk_20 pairs each sheet with a
 * -1), except unk_3C, menu_DrawConfirmSheetsAndDigitPairs's list
 * (Unk8006DD94List); reloc_RelocateTwelveListsAndLoadVabPack walks the twelve
 * slots as one run of list pointers.
 * - rect_rows: the Rec_8006C21C rows
 * menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads draws;
 * sheet_pair_box_sizes: the Size2s16 box sizes (x = width, y = height)
 * gpu_DrawSheetPairCenteredTileWithOptionalSelection reads. The -1 ending
 * reloc_RelocateSelfOffsets's offset list follows at +0x4C; no reader touches
 * it. */
typedef struct {
    Unk8006E950Head resource_head;
    Unk8009B0E0Record **unk_14;
    Unk8009B0E0Record **row_sheets;
    Unk8009B0E0Record **unk_1C;
    Unk8009B0E0Record **unk_20;
    Unk8009B0E0Record **unk_24;
    Unk8009B0E0Record **unk_28;
    Unk8009B0E0Record **unk_2C;
    Unk8009B0E0Record **unk_30;
    Unk8009B0E0Record **unk_34;
    Unk8009B0E0Record **unk_38;
    Unk8006DD94List *unk_3C;
    Unk8009B0E0Record **cursor_row_sheets;
    Rec_8006C21C *rect_rows;
    Size2s16 *sheet_pair_box_sizes;
} Unk8006919CRec;

/* The bit stream bitstream_ReadBits reads, most significant bit first: the next
   u32 word to load, the word being read and how many of its low bits are still
   unread. */
typedef struct {
    u32 *next;
    u32 word;
    s32 avail;
} BitStream;

/* 0x800A3D40: 24-byte records bitstream_InitDeltaAccumulator starts and
   bitstream_AccumulateDeltas advances: the bit stream bitstream_ReadBits reads
   and six s16 values bitstream_AccumulateDeltas adds the decoded deltas to and
   returns (4510C motion_UpdatePlayback reads them as an offset and a rotation).
 */
typedef struct {
    BitStream bits;
    s16 unkC[6];
} Unk800A3D40Rec;

/* 5ED34's work block: the 0x14 bytes InitDisplayStateLoadPackAndSetEntryFlags
 * keeps at the start of gpu_InitPrimBufferPair's returned space
 * (g_entry_menu_work_block; the arena cursor g_entry_menu_arena_cursor moves
 * past it), as 596CC's Unk800A34FCRec.
 * - entry_countdowns / row_countdowns: one s16 per player each (counted down
 * from 0x1E by the draw handlers).
 * - animation_frame_counter: the frame counter
 * StepSlideBuildFrameRecordAndDispatchStateHandler advances;
 * prim_buffer_counter: its buffer counter (bit 0 picks the half of the
 * Unk8006E49CRec pair).
 * - draw_offset: the draw offset the handlers hand to SetDrawOffset. */
typedef struct {
    s16 entry_countdowns[2];
    s16 row_countdowns[2];
    s32 animation_frame_counter;
    s32 prim_buffer_counter;
    u16 draw_offset[2];
} Unk800A35C4Rec;

/* 596CC's work block: the 0x34 bytes
 * menu_LoadResourcesInitStateSelectFirstEnabledRow keeps at the start of
 * gpu_InitPrimBufferPair's returned space (g_row_menu_work_block; its arena
 * cursor g_menu_arena_cursor moves past the block).
 * menu_LoadResourcesInitStateSelectFirstEnabledRow zeroes menu_draw_offsets /
 * unk_10 / unk_12, sets both row_bit_cursors halves to 5 and
 * strip_refresh_snapshot to bit 0 of g_row_menu_working_record_ptr's word 8
 * (gpu_GetPrimBufferAndRefreshStrip compares and refreshes it), and stores the
 * MOD.BIN root in menu_resource_root.
 * - menu_draw_offsets: the s16 pair pad_StepOffsetsGetDirectionEvents steps
 * (its arg2); the draw functions use menu_draw_offsets[0] / [1] as x / y
 * offsets.
 * - row_bit_cursors: one s16 per player
 * (menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads,
 * menu_SetExclusiveRowBit, menu_HandleRowBitInput); menu_HandleRowBitInput and
 * menu_DrawKeyConfigRowBitsGetEmptyRowSides also test the pair as one word (==
 * 0x50005: both 5), as SelWork's f1C / f20. No code touches the other bytes. */
typedef struct {
    u8 pad00[0xC];
    s16 menu_draw_offsets[2];
    s16 unk_10;
    s16 unk_12;
    u8 pad14[0x10];
    Unk8006919CRec *menu_resource_root;

    union {
        s16 half[2];
        s32 word;
    } row_bit_cursors;

    u8 pad2C[4];
    u8 strip_refresh_snapshot;
    u8 pad31[3];
} Unk800A34FCRec;

#endif /* GAME_H */
