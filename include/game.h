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
    u8 unk_0;
    u8 unk_1;
    s32 unk_4;
} TimeRec;

typedef struct {
    s32 unk_00; /* 0x80106A50 */
    u8 unk_04;  /* 0x80106A54 */
    u8 unk_05[3];
    TimeRec times[3]; /* 0x80106A58 */
    u8 frec20[3];     /* 0x80106A70 */
    /* 0x80106A73: bits 0/1/2: gpu_GetSavedGrayscaleFlag,
     * fighter_GetSavedCpuBehaviorAdjustFlag, snd_GetSavedCdAudioSuppressFlag */
    u8 flags;
} FileRecord;

/* The 0x100-byte save block after the memory-card header (D_800F34D8 = the
 * 0x200-byte card buffer + 0x100): memcard_BuildSaveBlock writes three copies
 * of D_80106A50 with their byte checksums and clears the pointer / value table;
 * memcard_RestoreSaveBlock restores the first copy whose checksum holds (unless
 * its flags bit 7 is set) and writes each val[j] through ptr[j] when ptr[j] is
 * a KSEG0 RAM address. */
typedef struct Unk800F34D8Save {
    FileRecord rec[3]; /* +0x00 */
    s32 sum[3];        /* +0x6C byte sums of rec[] */
    u16 *ptr[0x16];    /* +0x78 */
    u16 val[0x16];     /* +0xD0 */
    s32 unk_FC;
} Unk800F34D8Save; /* sizeof == 0x100 */

/* The game's display double buffer at 0x800F7438: two 0x4090-byte records, one
 * per frame parity (D_800A36AC & 1). disp_Init /
 * gpu_BeginFullscreenImageDisplay pass +0x00 to SetDefDrawEnv and +0x5C to
 * SetDefDispEnv for each record; main clears +0x70 with ClearOTagR(ot, 0x1008)
 * (0x1008 words = 0x4020 bytes, which ends the record) and draws it from its
 * last entry, DrawOTag(+0x408C). */
typedef struct {
    DRAWENV draw;   /* +0x00 */
    DISPENV disp;   /* +0x5C */
    u32 ot[0x1008]; /* +0x70 ordering table */
} GpuDb;            /* 0x4090 */

/* The record at D_800EF848 (0x134 bytes). */
typedef struct {
    s32 phase; /* +0x000 */
    /* +0x004: one bank per frame parity, two packets per level */
    DR_MOVE move[2][6];
    s16 ctl[7]; /* +0x124: filled from D_80099C34 by gpu_InitVramScroll */
} Unk800EF848Rec;

/* 3-word record table at 0x800F1198, terminated by an all-zero record
 * (eff_SetBillboardList walks it with a 12-byte stride). */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F1198Record;

/* 0x800A9CF8 record (0x18 bytes); node_InitFrameEntriesFromWorldTranslations
 * reaches its fields off one base register, and
 * resource_RebaseFrameAnimatedEntryBlockAndPointers bumps the unk8 / unkC
 * pointers together.
 */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4; /* = resource_GetLoadedSelector() (g_loaded_resource_selector) */
    s16 unk6; /* entry count */
    s32 unk8;
    s32 unkC;  /* entry table (stride 0x68) */
    s32 unk10; /* gpu_GetGridNodeRecordTable() table (stride 0x68) */
    s32 unk14;
} Unk800A9CF8Rec;

/* Per-lane slot record table at 0x800F0EC8: 2 lanes x 10 slots x one 3-word
 * record (12 bytes; lane stride 120). eff_AllocateOrRecycleYawedQuadSlot writes
 * it, eff_DrawYawedSlotQuads reads it. The slot bitmask D_800A3454[n] and the
 * sibling SVECTOR table D_800F1000[n][10] index the same pair (n, slot). */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0EC8Record;

/* Effect-slot position table at 0x800F0E38: 12 slots x one 3-word {x, y, z}
 * record (12 bytes; 0x800F0E38 + 12 * 12 == 0x800F0EC8, the table above).
 * eff_SpawnBillboardPair spawns into it, eff_DrawAndAdvanceBillboardPairs draws
 * from it. The slot bitmask D_800A3448 and the age table D_800F0BEC[slot] index
 * the same slot. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0E38Record;

/* 0x8009BC0C: the eight option rows of the list menu menu_UpdateRowMenu drives,
 * one {state, mode} byte pair per row (0x8009BC0C..0x8009BC1B), indexed by
 * D_800A34F8 & 0xF. state: accept / reject / return value; mode: passed to the
 * three render calls, 1/2/3 select the D_800A3524 bit-3 action. */
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

/* 0x8009BCF8: 2 pages x 10 two-byte records (0x8009BCF8..0x8009BD1F; D_8009BD20
 * follows), one page per character-select page, 2 rows x 5 columns of cells
 * (cell = row * 5 + col). menu_UpdateSelGridPicks / menu_DrawSelGrid index it
 * [page][cell]; menu_FadeOutSelAndWriteResult reads unk1 flat. Data: unk1 is
 * 0x00..0x09 (page 0) then 0x0C..0x15 (page 1). */
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
 * Unk800788B0Rec, whose members name the primitive types; 596CC's
 * gpu_AdvanceBufferCounterAndBuildDrawContext copies them into the s32 words of
 * its context array. */
typedef struct {
    s32 unk_00;
    void *unk_04;
    void *unk_08;
    void *unk_0C;
    s32 unk_10;
    void *unk_14;
    void *unk_18;
    void *unk_1C;
    void *unk_20;
    s32 unk_24;
    s32 unk_28;
} Unk8006E49CRec;

/* The eight primitive cursors of a draw context (StepScrollingSpriteDisplay
 * fills one bare; Unk8006EACCRec embeds it). The draw functions build
 * primitives at a cursor and advance it. unk_00 / unk_0C hold the s32 cursors
 * gpu_AddScaledSpriteQuads / gpu_AddRotatedSpriteQuads and gpu_AddSpriteCells
 * return. */
typedef struct {
    s32 unk_00;
    POLY_F4 *unk_04;
    POLY_G4 *unk_08;
    s32 unk_0C;
    TILE *unk_10;
    DR_MODE *unk_14;
    DR_AREA *unk_18;
    DR_OFFSET *unk_1C;
} Unk800788B0Rec;

/* The VAB pack snd_LoadVabPack loads (MOD.BIN's head unk_00 points at one;
   g_vab_rec_ptr keeps the loaded ones): four words, the first three
   file-relative offsets snd_LoadVabPack turns into addresses in place. unk_00:
   the u32 key-event table snd_FlushSoundQueue reads; unk_04: the VabHdr
   snd_VabOpen opens; unk_08: the body snd_VabOpen transfers; unk_0C: the body's
   size in SPU memory. */
typedef struct {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
} Unk8005C2A8Pack;

/* The 8-byte sprite cell: x / y, its offset from the descriptor's screen
 * position; u / v, its texel offset from the sheet's ubase / vbase; w / h, its
 * size (gpu_AddSpriteCells: x0 = x + the descriptor's x, u0 = u + ubase, w / h
 * copied). A sheet's cells follow its header(s) (Unk8009B0E0Record.cells). The
 * 0x8009B400 tables hold such cells: 0x8009B400..0x8009B44F is 10 records, one
 * per digit (gpu_DrawTimeFieldsAndDecimalCounter /
 * gpu_DrawTwoDigitCounterWithOptionalSecondSide index it with a 0..9 value),
 * and 0x8009B458..0x8009B487 6 more
 * (gpu_DrawTwoDigitCounterWithOptionalSecondSide); D_8009B450 and D_8009B488
 * follow. */
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
 * resource_RelocateNineOffsetListsAndLoadVabPack pass unk_00 and unk_04 to the
 * VAB loader snd_LoadVabPack (menu_RelocateSelLists /
 * reloc_RelocateFiveOffsetListsAndGetWord1, for D_SEL / NAR, make no such call,
 * and those files' unk_00..unk_08 are equal),
 * gpu_ReadCdFileRelocateAndUploadImagesAndBottomStrip loads the image at unk_08
 * into VRAM, and gpu_LoadVramBottomStrip loads the 640x32 strip at unk_0C or
 * unk_10. The four per-file relocators
 * (reloc_RelocateTwelveListsAndLoadVabPack,
 * resource_RelocateNineOffsetListsAndLoadVabPack, menu_RelocateSelLists,
 * reloc_RelocateFiveOffsetListsAndGetWord1) return unk_04, where the caller's
 * gpu_InitPrimBufferPair buffer starts. The words after the head differ per
 * file. */
typedef struct {
    Unk8005C2A8Pack *unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
} Unk8006E950Head;

/* STAFF.BIN (resource file 0x5F), the root 64FD8's InitScrollingSpriteDisplay
 * loads at its work area + 0x58 (D_800A3610). After the head, one offset list
 * (relocated with the head by reloc_RelocateSelfOffsets, so it ends in -1 like
 * the head's own list): the sprite sheets gpu_DrawScrollingAndDelayedSheets
 * draws: unk_14[10] first, then unk_14[0] onward while the next entry is not
 * -1. */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B0E0Record *unk_14[0];
} Unk80078824Rec;

/* 12-byte rectangle rows {x, y, w, h, r, g, b}: MOD.BIN's unk_44
 * (menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads draws them) and
 * D_SEL.BIN's unk_3C (menu_DrawSelRowTiles). */
typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;

/* SEL.BIN's unk_54 block, gpu_UploadAndDrawSelectedEntryQuads's: two cell
 * tables (unk_00[i], one per player), a 0 word, then the 12-byte sheet headers
 * the cells are drawn with (unk_0C[v] for the D_8009BC40 entry value v it
 * reads; after its first iteration the values 12, 13, 14, 0 and 3 take
 * unk_0C[22..26]). */
typedef struct {
    Unk8009B400Record *unk_00[2];
    s32 unk_08;
    Unk8009B0E0Record unk_0C[0];
} Unk8006ECF4Rec;

/* SEL.BIN / SEL1.BIN / SEL2.BIN (resource files 3-5), the root 5ED34's
 * InitDisplayStateLoadPackAndSetEntryFlags loads at its work area + 0x58
 * (D_800A35A8; StepSlideBuildFrameRecordAndDispatchStateHandler hands it to the
 * handlers as Unk8006EACCRec.unk_00). After the head:
 * - unk_14: per entry id two TIM pixel addresses, one per column,
 * menu_UpdateRowCursors loads (LoadImage).
 * - unk_54..unk_74: the nine lists
 * resource_RelocateNineOffsetListsAndLoadVabPack relocates
 * (reloc_RelocateOffsetList): unk_54, the header block
 * gpu_UploadAndDrawSelectedEntryQuads draws (Unk8006ECF4Rec); unk_58..unk_74,
 * sprite-sheet lists. unk_78 is a further offset no code reads.
 * - unk_7C: per player a row of eight VRAM RECTs menu_UpdateRowCursors hands
 * LoadImage as the destination; unk_80: the bytes menu_UpdateChoiceGrid reads.
 * - unk_84: TIM pixel addresses gpu_UploadAndDrawSelectedEntryQuads loads per
 * character case. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 unk_14[8][2];
    Unk8006ECF4Rec *unk_54;
    Unk8009B0E0Record **unk_58;
    Unk8009B0E0Record **unk_5C;
    Unk8009B0E0Record **unk_60;
    Unk8009B0E0Record **unk_64;
    Unk8009B0E0Record **unk_68;
    Unk8009B0E0Record **unk_6C;
    Unk8009B0E0Record **unk_70;
    Unk8009B0E0Record **unk_74;
    s32 *unk_78;
    RECT *unk_7C;
    u8 *unk_80;
    s32 unk_84[5];
} Unk8006EA28Rec;

/* D_SEL.BIN (resource file 6), the root 64FD8's menu_InitSelScreen loads at its
 * work area + 0x58 and keeps in SelWork.f04 (menu_StepSelScreen hands it to the
 * handlers as Unk8006EACCRec.unk_00). After the head: unk_14..unk_38, the ten
 * lists menu_RelocateSelLists relocates, each a list of sprite sheets (unk_20
 * is indexed by the round count SelWork.f65); unk_3C, the rectangle rows
 * menu_DrawSelRowTiles draws.
 */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B0E0Record **unk_14;
    Unk8009B0E0Record **unk_18;
    Unk8009B0E0Record **unk_1C;
    Unk8009B0E0Record **unk_20[3];
    Unk8009B0E0Record **unk_2C;
    Unk8009B0E0Record **unk_30;
    Unk8009B0E0Record **unk_34;
    Unk8009B0E0Record **unk_38;
    Rec_8006C21C *unk_3C;
} Unk80076FF8Rec;

typedef struct {
    s16 on, off;
} Unk80077D94Entry;

/* NAR.BIN (resource file 0x32), the root 64FD8's InitTimedSpriteSequence loads
 * at its work area + 0x58 (D_800A35F8). After the head: table, the cell table
 * gpu_DrawAndAdvanceTimedSpriteSequence draws the hdr18..hdr24 sheets with;
 * hdr18..hdr28, the five sheet lists reloc_RelocateFiveOffsetListsAndGetWord1
 * relocates; nar2C, in30 / out34 and the five portrait TIMs img38
 * gpu_DrawAndAdvanceTimedSpriteSequence uploads. */
typedef struct {
    Unk8006E950Head unk_00;
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
 * and 64FD8 menu_StepSelScreen / StepTimedSpriteSequenceWithSkip: unk_00 the
 * resource root (SEL for StepSlideBuildFrameRecordAndDispatchStateHandler's
 * handlers, D_SEL for menu_StepSelScreen's; StepTimedSpriteSequenceWithSkip
 * leaves it unset), then the cursors; unk_24 (pool word +0x20) only
 * StepSlideBuildFrameRecordAndDispatchStateHandler / menu_StepSelScreen set. */
typedef struct {
    union {
        Unk8006EA28Rec *v8006EA28;
        Unk80076FF8Rec *v80076FF8;
    } unk_00;

    Unk800788B0Rec unk_04;
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
   Unk8009BD24Block.unk00. */
typedef struct {
    u8 bd0;
    u8 unk1;
} Unk8009BD24Record;

/* One of the three 4-byte records at Unk8009BD24Block.unk21
   (fight_PackTimeRecordsAndInitPresentation and
   CopySavedFlagsButtonMappingsAndTimesToWorkingRecord store them;
   gpu_DrawIndexedCellsAndDecimalDigitPairs reads them). */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Unk8009BD45Rec;

/* 0x8009BD24..0x8009BD57: the settings record GetWorkingRecord returns and
   64FD8 hands to menu_LoadResourcesInitStateSelectFirstEnabledRow /
   InitDisplayStateLoadPackAndSetEntryFlags / menu_InitSelScreen, whose callees
   reach every field off that one base (596CC D_800A3524, 5ED34 D_800A3568,
   SelWork.f00). Bit fields are named by bit offset.
   - unk14_*: the word at +0x14. unk14_0 (`& 0xF`; menu_UpdateRowMenuStoreResult
   stores it), unk14_4 (6 bits), unk14_10 (the round count - 3; also picks the
   results-screen layout), unk14_12 (`== 2` tests), unk14_14, unk14_15 (one bit
   per player), unk14_17, unk14_18 (3 bits); no code reads bits 21-23.
   - unk17 / unk1A / unk1D: three byte triples. menu_HandleRowBitInput rebuilds
   a player's nibble of unk17 from unk1A or unk1D; 64FD8 copies unk17 and unk1D
   into each other.
   - unk20_*: the word at +0x20, bits 0-3; its upper bytes start the unk21
   records. */
typedef struct {
    Unk8009BD24Record unk00[2][5];
    u32 unk14_0 : 4;
    u32 unk14_4 : 6;
    u32 unk14_10 : 2;
    u32 unk14_12 : 2;
    u32 unk14_14 : 1;
    u32 unk14_15 : 2;
    u32 unk14_17 : 1;
    u32 unk14_18 : 3;
    u32 unk14_21 : 3;
    u8 unk17[3];
    u8 unk1A[3];
    u8 unk1D[3];
    u32 unk20_0 : 1;
    u32 unk20_1 : 1;
    u32 unk20_2 : 1;
    u32 unk20_3 : 1;
    u32 unk20_4 : 4;
    Unk8009BD45Rec unk21[3];
    u8 unk2D[3];
    u8 unk30;
} Unk8009BD24Block;

/* The select-screen work area D_800A36A0 points at (menu_InitSelScreen places
 * it at the end of the buffers gpu_InitPrimBufferPair lays out). Two-element
 * arrays are per player. Per player, f48 is a list of f60 entries with cursor
 * f5C; confirming moves the entry under the cursor to f7E[f3C++], cancelling
 * moves f7E[--f3C] back into f48 in sorted order; f6A holds the picked entries
 * menu_FadeOutSelAndWriteResult reports. f00 is the result record
 * menu_FadeOutSelAndWriteResult writes; f04 is the resource table the select
 * draw functions receive as arg0[0] (menu_StepSelScreen passes it as the first
 * word of that context). f10, f14, f1C and f20 are per-player s16 pairs the
 * code also accesses as one word (union word views, Q33/Q46):
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
 * index it [row][D_800A3590[row]]. */
typedef struct {
    s16 x;
    s16 y;
} Unk8009BC94Record;

/* 0x8009BC40: one select-grid entry; value indexes D_8009BC7C's flags and
 * picks the entry's sprite and sound. */
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
   motion_StartPlayback is handed; Unk800EFAE8Rec .unk2C): its header holds
   file-relative offsets of the parts motion_StartPlayback /
   motion_UpdatePlayback use. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10[2];
    s32 unk18[2];
} Unk800469C4Hdr;

/* Record at 0x800EFAE8 (0x4C bytes): motion_StartPlayback sets it
 * up, the per-frame handler motion_UpdatePlayback runs it (walking the
 * per-object arrays in its object loop), and motion_RebasePlayback relocates
 * the 0x2C..0x40 words.
 */
typedef struct {
    /* phase (motion_UpdatePlayback: -1 = done, 0 = init) */
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* stage flags (bit31/bit30 tests, low 6 bits = cleanup index + 1) */
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 unk22;
    /* negated camera rotation vx/vy/vz; motion_GetPlaybackViewAngles returns
     * its address */
    /* 0x24 */ s16 unk24[4];
    /* loaded data base; relocated by motion_RebasePlayback */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* relocated by motion_RebasePlayback */
    /* per player; relocated by motion_RebasePlayback when nonzero */
    /* 0x34 */ s32 unk34[2];
    /* per player; relocated by motion_RebasePlayback when nonzero */
    /* 0x3C */ s32 unk3C[2];
    /* 0x44 */ s16 unk44[2]; /* per player */
    /* 0x48 */ s16 unk48[2]; /* per player */
} Unk800EFAE8Rec;

/* 0x800A3560: two 3-byte records, one per selection slot i (slot i at
 * 0x800A3560 + i * 3; 0x800A3566/7 pad before D_800A3568). Object-model
 * evidence: DrawRotatedEntryCountdownAndRequestState3OrDrawDarkenedSelection
 * and menu_UpdateRow step one offset register by 3 per slot;
 * menu_UpdateEntryGridCursors reaches +0 / +1 / +2 through i * 3;
 * InitDisplayStateLoadPackAndSetEntryFlags stores byte +1 of both. `word` (Q33
 * union word view) is InitDisplayStateLoadPackAndSetEntryFlags's one word store
 * of -1 over bytes 0..3; every other access goes through rec[]. Every consumer
 * is in the -G8 file src/main/5ED34.c (Q44/Q54).
 */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
} Unk800A3560Record;

typedef union {
    Unk800A3560Record rec[2];
    s32 word;
} Unk800A3560Slots;

/* Record table at 0x800F0C10 (0x90 bytes, ends at D_800F0CA0): 4 rows of 3
 * records of 3 s32 words. eff_PrepareParticles indexes it [arg1][idx] (arg1
 * 0..3 from the callers func_800676C8..func_8006786C); eff_SpawnParticles and
 * eff_UpdateParticles use it too. */
typedef struct {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} Unk800F0C10Record;

/* 0x800948BC (D_800948BC): function-pointer pairs, indexed by
 * resource_GetLoadedSelector(). resource_InvokeLoadedSelectorInitCallback calls
 * .init; gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards
 * calls .unk4 (e.g. entry 13 holds node_InitViewOffsetNodeWithYXZRotation /
 * node_OrientAndQueueViewOffsetNode). */
typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} Unk800948BCEntry;

/* D_8008DCCC: model id -> BBM file name, ended by a zero id
 * (0x8008DCCC..0x8008DD5B). motion_LoadFighterData loads the model of entry n
 * from CD file n + 2. */
typedef struct {
    s32 id;
    const char *name;
} Unk8008DCCCEntry;

/* The file motion_LoadFighterData loads into g_motion_fighter_buffers[slot]
 * (slot = rec+0x4A, the load slot fighter_AssignMotionSlotAndScanDataIdNibbles
 * sets; fighters with the same model id share it). f14 is the modulus
 * motion_AdvanceVariantIfAtNibble4Or5Position / motion_ResolveMoveData wrap
 * rec+0x86 with. The u16 fields at +0x4E are indices into
 * g_motion_fighter_sections[slot].unk_00, as motion_ResolveMoveData reads them:
 * f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4), f54[rec+0x86][t]
 * (id 0x7FF3), and f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). f16
 * (motion_GetCommonScriptAtFighterBufferOffset) and f18[unk_0A]
 * (motion_GetCommonScriptAtFighterBufferIndexedOffset;
 * Unk80101EC8Record.unk_0A, 0..26 as D_8008D538 holds and as the [27][6] tables
 * D_8008DE34 / D_8008DF78 are sized; 27 halfwords end exactly at f4E) are
 * indices into g_motion_common_sections.unk_00; both readers load them with
 * lhu. */
typedef struct {
    u8 pad00[3];
    /* motion_LoadFighterData: g_motion_fighter_sections[slot].unk_00 =
     * record + 0x6C + (unk_03 - 1) * 6 */
    u8 unk_03;
    /* g_motion_fighter_sections[slot].unk_04 .. unk_10 = record + unk_04[k]
     * (motion_LoadFighterData) */
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
 * unk_00 = record + 0x6C + (unk_03 - 1) * 6, unk_04..unk_10 = record +
 * unk_04[0..3]); g_motion_common_sections for the common file
 * (motion_LoadCommonData: unk_00 = file + 0x14, the others file + its header
 * words 1 / 2 / 4; unk_0C unset). unk_00: halfword streams
 * motion_ResolveMoveData indexes (the f4E / f54 / f66 / f16 / f18 entries);
 * unk_04: 4-byte entries whose second halfword offsets unk_08
 * (fighter_StartMoveScript, replay_PlayFrame); unk_0C: fighter_InitCpuControl's
 * table; unk_10: motion_InitStream's motion bitstream. */
typedef struct Unk801027B0Rec {
    u16 *unk_00;
    u16 *unk_04;
    u8 *unk_08;
    u16 *unk_0C;
    u32 *unk_10;
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
   fighter_UpdateBoneWorldData copies a scratchpad point (SPAD->unkA8) into
   Unk80101EC8Record.unk_180 as one 12-byte object. */
typedef Vec3i32 Unk80107850Rec;

/* The PsyQ SVECTOR layout under a local name. */

typedef struct {
    s16 vx, vy, vz, pad;
} SVec4i16;

/* D_80104E88[4]: the four 0x2C-byte records
 * fight_CreateBallisticRecordAndDispatchEvent claims (first with unk_00 == 0)
 * and fight_UpdateBallisticRecords steps each frame (unk_02 counts frames;
 * unk_10 keeps the previous unk_04; unk_1C is added to unk_04 with unk_1C.y
 * growing by 0xD; the record is freed when
 * collision_TestSegmentExtendingShortStartWithWork reports a hit or unk_04.y
 * passes unk_28). fight_TriggerOpposingFighterEventFromNearBallisticRecords
 * measures the distance from the other player's world_pos to unk_04 (unk_03 is
 * the owner). fight_ClearBallisticRecords clears unk_00 of all four. */
typedef struct Unk80104E88Rec {
    /* 0 = free; fight_CreateBallisticRecordAndDispatchEvent's type (1 / 2) */
    u8 unk_00;
    u8 unk_01;
    u8 unk_02;
    u8 unk_03; /* player index (Unk80101EC8Record.index) */
    Vec3i32 unk_04;
    Vec3i32 unk_10;
    Vec3i32 unk_1C;
    s32 unk_28;
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
 * fighter_UpdateBoneWorldData places point i at SPAD->unkA8[ch][i] (`ofs`
 * rotated by node_GetSlotMatrixPointerArray()'s matrix `bone`, plus that
 * matrix's translation); the hit tests (fighter_CastWeaponSegment,
 * fighter_TestTriangleBoneHits, object_ResolveFighterHits) take unk_0C / unk_0E
 * as the first test's limits and, when unk_00 != 0, unk_10 / unk_12 as a second
 * test's. */
typedef struct BoneHitRec {
    s16 unk_00;
    s16 bone;
    SVec4i16 ofs;
    u16 unk_0C;
    u16 unk_0E;
    u16 unk_10;
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
 * entry of D_8009AA70) with its volumes in the first free entry;
 * snd_FlushSoundQueue keys each queued note on via SsUtKeyOnV(.., voll, volr)
 * and clears the entry; snd_Init / snd_ResetVoicesAndQueue reset all 24. */
typedef struct {
    u16 *req;
    u8 volr;
    u8 voll;
} Unk800EFB78Entry;

/* One 8-byte texture record: the CLUT position (PsyQ getClut(x, y) = (y << 6) |
 * ((x >> 4) & 0x3F)) and the texture u/v origin. 51268's D_800A3488 /
 * D_800A348C point at one; the D_8009B890 .. D_8009BA58 tables are runs of
 * them. */
typedef struct {
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
} TexRec;

/* The 0x2C-byte block D_800A3468 points at (51268): Unk1F800000Rec.unk00
 * (eff_BindScratchpad's seed) or g_eff_command_block, where
 * eff_StartRotatedQuadClusterWithNearWhiteLight ..
 * eff_StartRecycledCommandWithMagentaLight point it before calling
 * eff_StartCommand. g_eff_billboards follows g_eff_command_block, so that copy
 * ends at +0x2C. Each copy sets one pointer pair:
 * - unk00: one word, stored whole and read whole (bit 21 in eff_StartCommand,
 * bits 17-18 / 19-20 in func_80063AF0 / func_80063B34 /
 * eff_InitRateSelectedCosineScaledQuad); eff_StartCommand / eff_StepCommand
 * also read its low halfword (`lhu`), their g_eff_command_phase / D_8009BA60
 * index.
 * - unk04 / unk08: the three halfwords / three words eff_StepCommand and
 * eff_DrawBillboardList copy into the scratchpad block's unk18 / unk20 (through
 * D_800A346C / D_800A3470). Only the scratchpad block's are set:
 * eff_BindScratchpad stores its two arguments there.
 * - unk0C / unk10: the three words / three halfwords eff_StartCommand copies
 * into unk20 / unk18. Set only in the g_eff_command_block block (through
 * D_800A3468 or g_eff_command_block directly).
 * - unk14: the byte eff_StartCommand / eff_StepCommand store the called
 * function's result to (`sb`).
 * - unk18 / unk20: the copies. D_800A346C / D_800A3470 point at the scratchpad
 * block's, D_800A3478 / D_800A347C at the g_eff_command_block block's
 * (eff_StartCommand). unk18 is s16 like its sources (unk04 / unk10) and every
 * consumer (SVECTOR / SVec4i16 fields: eff_SetRotMatrix,
 * eff_SpawnRotatedQuadCluster, eff_AllocateOrRecycleYawedQuadSlot,
 * eff_SpawnParticles).
 * - unk1E: no access. */
typedef struct {
    union {
        s32 w;
        u16 h;
    } unk00;

    s16 *unk04;
    s32 *unk08;
    s32 *unk0C;
    s16 *unk10;
    u8 *unk14;
    s16 unk18[3];
    u8 unk1E[2];
    s32 unk20[3];
} Unk1F800000Unk00;

/* 20-byte record table at 0x800EFC78 (51268): 4 rows (arg1) of 48 records,
 * filled by eff_SpawnParticles (which leaves +0xE untouched). */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    s16 unkE;
    s16 unk10;
    s16 unk12;
} Unk800EFC78Record;

/* eff_DrawBillboardList's layout of the 51268 work area
 * (Unk1F8000B8Union.v800620B8). eff_SetRotMatrix, which eff_DrawBillboardList
 * calls before it touches the area, writes and consumes its SVECTOR at
 * +0x00..+0x07. SetTransMatrix is handed the address 0x14 below unk14, so unk14
 * is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10]; /* no access here */
    s16 unk10;      /* w */
    s16 unk12;      /* h */
    VECTOR unk14;   /* ApplyRotMatrixLV out */
    VECTOR unk24;   /* ApplyRotMatrixLV in */
    SVECTOR unk34;  /* RotTransPers in */
    s32 unk3C;      /* RotTransPers' p */
    u8 unk40[4];    /* no access */
    u32 unk44;      /* gpu_DepthToOtIndex's depth, the OT index */
} Unk1F8000B8_800620B8;

/* eff_DrawAndAdvanceBillboardPairs's layout (Unk1F8000B8Union.v80063084).
 * SetTransMatrix is handed the address 0x14 below unk14 (the area's base), so
 * unk14 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x14]; /* no access */
    VECTOR unk14;   /* ApplyRotMatrix out */
    SVECTOR unk24;  /* ApplyRotMatrix in */
    SVECTOR unk2C;  /* RotTransPers in */
    s32 unk34;      /* RotTransPers' p */
    s32 unk38;      /* D_800F0BEC[i] - 15, the colour scale */
    s32 unk3C;      /* gpu_DepthToOtIndex's depth, the OT index */
} Unk1F8000B8_80063084;

/* eff_DrawAndAdvanceJitteredBillboards's layout (Unk1F8000B8Union.v800646E8).
 * SetTransMatrix is handed the address 0x14 below unk18, so unk18 is read as a
 * MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];  /* no access */
    s16 unk10;       /* w */
    s16 unk12;       /* h */
    s32 unk14;       /* frame */
    VECTOR unk18;    /* ApplyRotMatrixLV out */
    VECTOR unk28;    /* ApplyRotMatrixLV in */
    SVECTOR unk38;   /* RotTransPers in */
    s32 unk40;       /* RotTransPers' p */
    POLY_FT4 *unk44; /* the end of the quads */
    u32 unk48[16];   /* per kept quad, its OT index */
} Unk1F8000B8_800646E8;

/* eff_DrawAnimatedQuads's layout of the 51268 work area
 * (Unk1F8000B8Union.v80065800). Its gte_SetTransMatrix operand hands the GTE
 * the area's base as a MATRIX, whose t[] is unk14[0]. */
typedef struct {
    u8 unk00[0x10]; /* no access */
    s32 unk10;      /* gte_stdp out; not read */
    /* ApplyRotMatrixLV out ([0]); the corner loop's ApplyRotMatrix outs */
    VECTOR unk14[4];
    VECTOR unk54; /* ApplyRotMatrixLV in */
    /* gte_ldv0 / RotMatrix / ApplyRotMatrix in */
    SVECTOR unk64;
    s16 unk6C;    /* p_w */
    u8 unk6E[2];  /* no access */
    s16 unk70;    /* p_h */
    u8 unk72[2];  /* no access */
    MATRIX unk74; /* RotMatrix out, SetRotMatrix in */
    s16 unk94;    /* added to the texture u */
    s16 unk96;    /* added to the texture v */
} Unk1F8000B8_80065800;

/* The layout eff_PrepareParticles / eff_UpdateParticles / eff_LinkParticleQuads
 * share (Unk1F8000B8Union.v800678A8): each of the wrappers func_800676C8 ..
 * func_8006786C calls the three in turn, and values cross the calls (unk04,
 * unk6C, unk80, the unk8C table; eff_PrepareParticles and eff_UpdateParticles
 * each use unk70 for their own value). eff_PrepareParticles sets it up,
 * eff_UpdateParticles builds the quads at unk80 and records each one's depth
 * (its OT slot index) in unk8C, eff_LinkParticleQuads links them. */
typedef struct {
    u16 unk00; /* eff_PrepareParticles: added to the texture u */
    u16 unk02; /* eff_PrepareParticles: added to the texture v */
    /* eff_PrepareParticles stores 0x895440; eff_UpdateParticles compares
     * against it */
    u32 unk04;
    u8 unk08[8]; /* no access */
    /* eff_UpdateParticles: only its translation (+0x24) is used: the
       gte_ApplyRotMatrix / gte_Square0 output, loaded by gte_SetTransMatrix */
    MATRIX unk10;
    u8 unk30[4];  /* no access */
    VECTOR unk34; /* eff_UpdateParticles: gte_Square0 in */
    /* eff_UpdateParticles: gte_ApplyRotMatrix / gte_ldv3 in */
    SVECTOR unk44[3];
    VECTOR unk5C; /* eff_UpdateParticles */
    /* eff_PrepareParticles stores it; eff_UpdateParticles's loop bound */
    s16 unk6C;
    /* eff_UpdateParticles / eff_LinkParticleQuads loop index */
    s16 unk6E;
    /* eff_PrepareParticles: table index; eff_UpdateParticles: per entry */
    s16 unk70;
    u8 unk72; /* eff_UpdateParticles colours */
    u8 unk73;
    u8 unk74;
    u8 unk75[3];              /* no access */
    s16 unk78;                /* eff_UpdateParticles */
    u8 unk7A[2];              /* no access */
    POLY_FT4 *unk7C;          /* eff_LinkParticleQuads: the end of the quads */
    POLY_FT4 *unk80;          /* the quad cursor */
    Unk800EFC78Record *unk84; /* eff_UpdateParticles */
    Unk800F0C10Record *unk88; /* eff_UpdateParticles */
    /* per quad, its depth, the OT slot index (eff_UpdateParticles stores,
       eff_LinkParticleQuads reads) */
    u16 unk8C[0x90];
    /* eff_UpdateParticles stores rand() values here; D_800A3724 points at it */
    s32 unk1AC;
} Unk1F8000B8_800678A8;

/* Unk1F800000Rec.unkB8, the 51268 work area D_800A34EC points at (0x1F8000B8 to
 * the end of the scratchpad). Each user lays it out for one call (or, for the
 * eff_PrepareParticles trio, one wrapper call); raw sizes the union.
 * eff_SetRotMatrix's is one SVECTOR. eff_DrawAndAdvanceRotatedQuadClusters and
 * eff_DrawYawedSlotQuads still convert D_800A34EC (their work-area base is
 * staged through a POLY_FT4 * local). */
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
 * from its own two stores to unk00.unk04 / unk08, the code reaches them only
 * through those globals:
 * - unk00: the command block D_800A3468 points at until a function retargets
 * it; D_800A346C / D_800A3470 point at its unk18 / unk20.
 * - unk2C: no access.
 * - unk50 / unkB0: the initial targets of D_800A3488 / D_800A348C (TexRec).
 * eff_DrawBillboardList retargets them only in its switch cases 0-3 (unk4 & 7),
 * so its later reads go through these seeded values when no earlier record took
 * one of those cases.
 * - unkA0 / unkA4: the initial targets of D_800A34E4 (u32 *) / D_800A34E8 (u32
 * *), which every user retargets before use.
 * - unkB8: the work area D_800A34EC points at (Unk1F8000B8Union), laid out
 * differently by its users; it runs to the end of the scratchpad. The other
 * members are one global each (D_800A3474 .. D_800A34E0, D_800A3480 /
 * D_800A3484). */
typedef struct {
    Unk1F800000Unk00 unk00;
    u8 unk2C[4];
    MATRIX unk30; /* D_800A3474 */
    TexRec unk50;
    s32 unk58;    /* D_800A3490 */
    s32 unk5C;    /* D_800A3494 */
    u16 unk60;    /* D_800A3498 */
    u16 unk62;    /* D_800A349C */
    u16 unk64;    /* D_800A34A0 */
    u16 unk66;    /* D_800A34A4 */
    s16 unk68;    /* D_800A34A8 */
    s16 unk6A;    /* D_800A34AC */
    s32 unk6C;    /* D_800A34B0 */
    s32 unk70;    /* D_800A34B4 */
    s32 unk74[3]; /* D_800A34B8 */
    s16 unk80;    /* D_800A34BC */
    s16 unk82;    /* D_800A34C0 */
    s32 unk84;    /* D_800A34C4 */
    s32 unk88;    /* D_800A34C8 */
    s32 unk8C;    /* D_800A34CC */
    s32 unk90[2]; /* D_800A34D0 */
    u16 unk98;    /* D_800A34D4 */
    u16 unk9A;    /* D_800A34D8 */
    u16 unk9C;    /* D_800A34DC */
    u16 unk9E;    /* D_800A34E0 */
    u32 unkA0;
    u32 unkA4;
    s32 unkA8; /* D_800A3480 */
    s32 unkAC; /* D_800A3484 */
    TexRec unkB0;
    Unk1F8000B8Union unkB8;
} Unk1F800000Rec;

/* A cell (x, z) of the 32x32 grid of 2000-unit cells that 43500's
 * collision_WalkSegmentGrid walks (Work_80053E9C.unk88 / unk8C). */
typedef struct {
    s16 x;
    s16 z;
} Cell_80052D00;

/* The 0xEC-byte work area of 43500's stage-collision cast
 * (collision_WalkSegmentGrid and its helpers, through D_800A33F4).
 * collision_TestSegmentExtendingShortStartWithWork /
 * collision_TestSegmentWithWork place it at their last argument;
 * collision_TestSegmentExtendingShortStart / collision_TestSegment at
 * D_800EF9F8. 17AFC object_UpdatePool / fight_UpdateBallisticRecords pass
 * 0x1F8002F0, its place in their scratchpad layout
 * (Unk1F8002B8_8005344C.unk38). */
typedef struct {
    s32 unk0;
    u16 unk4;
    s16 unk6;
    VECTOR unk8;
    VECTOR unk18;
    s32 unk28;
    s32 unk2C;
    s32 unk30;
    s32 unk34;
    s32 unk38;
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    s16 unk48;
    s16 unk4A;
    s16 unk4C;
    s16 unk4E;
    s16 unk50;
    s16 unk52;
    s16 unk54;
    s16 unk56;
    s16 unk58;
    s16 unk5A;
    s32 (*unk5C)(s32, s32);
    s32 unk60;
    s32 unk64;
    s32 unk68;
    s32 unk6C;
    s32 unk70;
    s32 unk74;
    s32 unk78;
    s32 unk7C;
    s32 unk80;
    s32 unk84;
    Cell_80052D00 unk88;
    Cell_80052D00 unk8C;
    s16 unk90;
    u8 unk92[0xA];
    s32 unk9C;
    s32 unkA0;
    s32 unkA4;
    s32 unkA8;
    s32 unkAC;
    s32 unkB0;
    s32 unkB4;
    s32 unkB8;
    s32 unkBC;
    s32 unkC0;
    s32 unkC4;
    s32 unkC8;
    s32 unkCC;
    s32 unkD0;
    s32 unkD4;
    s32 unkD8;
    s32 unkDC;
    s32 unkE0;
    s32 unkE4;
    s32 unkE8;
} Work_80053E9C;

/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch. unk00 is up to six points that
 * unk60[0..2] aim at. fighter_CastWeaponSegment and object_ResolveFighterHits
 * use [0] / [1], for math_PrepareSegmentCollisionFrame /
 * math_TestPreparedSegmentSphere: fighter_CastWeaponSegment's segment, base
 * then tip (fighter_ProcessWeaponBodyHits writes them before each call), and
 * object_ResolveFighterHits's g_object_pool object step, prev_pos then pos.
 * fighter_ProcessWeaponBodyHits uses all six: [0] / [1] are two of the other
 * character's scratchpad points (ScrPad.unk00 or unk48), [2] / [3] the same two
 * from its record (unk_210 or unk_234), [4] / [5] the midpoints of [0] / [2]
 * and [1] / [3]; for each triangle it hands math_PrepareTriangleCollisionFrame
 * it aims unk60[0..2] at three of them. object_UpdatePool /
 * fight_UpdateBallisticRecords lay the bytes out differently
 * (collision_TestSegmentExtendingShortStartWithWork's argument block, its work
 * area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of
 * Unk1F8002B8Union. raw sizes the union to 0x60. */
typedef union {
    u8 raw[0x60];
    Unk80107850Rec unk00[6];
} Unk1F8002B8Unk00;

/* 17AFC's view of the last 0x148 bytes of the scratchpad,
 * 0x1F8002B8..0x1F8003FF (ScrPad.unk2B8.rec). The scratchpad is shared scratch:
 * other code puts its own data in these bytes at other times, with its own
 * views. Among others:
 * - 43500 collision_TestSegmentWithWork /
 * collision_TestSegmentExtendingShortStartWithWork keep their work area
 * (Work_80053E9C, 0xEC bytes) at the address of their last argument
 * (D_800A33F4); callers pass 0x1F8002B8 (the area spans record +0x00..+0xEB) or
 * 0x1F8002F0 (+0x38..+0x123, over unk60 up into unk118).
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
 * fight_UpdateBallisticRecords use Unk1F8002B8_8005344C instead). unk60 / unk6C
 * hold point pointers (fighter_ProcessWeaponContacts reads unk60[0..2]); unkB4
 * / unkC4 are fighter_TestTriangleBoneHits's two hit masks
 * (fighter_ProcessWeaponBodyHits reads them; no 17AFC code accesses unkD4 as a
 * member); unkD8 is the matrix the RotMatrix* calls build, and the GTE rotates
 * unkA8 through it; func_8002DE20 rotates three points, relative to the origin
 * *unk60[0], into unk118 and tests them against the triangle (0,0) / unkA8 /
 * unkB8. unk00 is scratch (Unk1F8002B8Unk00): fighter_CastWeaponSegment /
 * object_ResolveFighterHits / fighter_ProcessWeaponBodyHits use its points. */
typedef struct {
    Unk1F8002B8Unk00 unk00;
    Unk80107850Rec *unk60[3];
    Unk80107850Rec *unk6C[3];
    Vec3i32 unk78;
    Vec3i32 unk84;
    Vec3i32 unk90;
    Vec3i32 unk9C;
    Vec3i32 unkA8;
    s32 unkB4;
    Vec3i32 unkB8;
    s32 unkC4;
    Vec3i32 unkC8;
    s32 unkD4;
    MATRIX unkD8;
    SVECTOR unkF8;
    Vec3i32 unk100[2];
    Vec3i32 unk118[3];
    Vec3i32 unk13C;
} Unk1F8002B8Rec;

/* The layout object_UpdatePool and fight_UpdateBallisticRecords give the
 * scratchpad area at 0x1F8002B8 (Unk1F8002B8Union.v8005344C): the to point, hit
 * point and normal they pass collision_TestSegmentExtendingShortStartWithWork,
 * and its work area, which runs over Unk1F8002B8Rec's unk60..unk118. */
typedef struct {
    /* to; collision_TestSegmentExtendingShortStartWithWork copies 16 bytes from
     * it */
    Vec3i32 unk00;
    s32 unk0C; /* read only as the 4th word of that copy */
    /* the hit point; object_UpdatePool's turn block first keeps a
       rotated velocity here */
    Vec3i32 unk10;
    s32 unk1C; /* no access */
    /* object_UpdatePool's turn block's second rotated vector
       (fight_UpdateBallisticRecords does not access it) */
    Vec3i32 unk20;
    s32 unk2C;    /* no access */
    s16 unk30[3]; /* the hit normal */
    s16 unk36;    /* no access */
    /* collision_TestSegmentExtendingShortStartWithWork's work area,
     * +0x38..+0x123 */
    Work_80053E9C unk38;
} Unk1F8002B8_8005344C;

/* ScrPad.unk2B8: the scratchpad's last 0x148 bytes, 0x1F8002B8..0x1F8003FF,
 * which 17AFC uses with two layouts. Its collision code uses rec
 * (Unk1F8002B8Rec). object_UpdatePool and fight_UpdateBallisticRecords use
 * v8005344C: they pass 0x1F8002F0 as
 * collision_TestSegmentExtendingShortStartWithWork's work-area address, so its
 * 0xEC-byte Work_80053E9C sits at +0x38..+0x123, over rec's unk60..unk118. */
typedef union {
    Unk1F8002B8Rec rec;
    Unk1F8002B8_8005344C v8005344C;
} Unk1F8002B8Union;

/* Scratchpad point tables at 0x1F800000.  unk00: three points per character
 * (fight_UpdateFighterInteractionsAndPointCaches copies [0][0..2] and [1][0..2]
 * to the two records' +0x210); unk48: two more per character (copied to
 * +0x234); unk78: two body points per character (fighter_ResolveBodyOverlap
 * builds them from unkA8 joint 1 and the midpoint of joints 15 and 19); unkA8:
 * 22 points per character (fighter_CastWeaponSegment reads 0x1F8000A8 + id *
 * 0x108 + i * 0xC, i < 22). fighter_Update hands fighter_UpdateBoneWorldData
 * its character's unkA8 / unk00 / unk48. unk2B8: the area at 0x1F8002B8
 * (Unk1F8002B8Union), to the end of the scratchpad; the scratchpad is shared
 * scratch that other code also uses with its own views (see Unk1F8002B8Rec). */
typedef struct {
    Unk80107850Rec unk00[2][3];
    Unk80107850Rec unk48[2][2];
    Unk80107850Rec unk78[2][2];
    Unk80107850Rec unkA8[2][22];
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

/* A stage's navigation set: a D_8009A658 row (12 bytes: count word, polygon
 * array, zero word). */
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
 * copy fighter_Update shows.  unk_00 is the root offset it negates, unk_02 /
 * unk_04 the root heading and distance, unk_06.. unk_0A the three rotation
 * angles it hands math_RotMatrixZYXAngles.  unk_0C[] holds signed channels:
 * math_ComposeAxisRotationsAndRewriteAngles reads the triplet at +0x36 with lh,
 * and motion_DecodeFrame sign-extends the 12-bit values at +0x6C..+0x70 /
 * +0x78..+0x7C.
 */
typedef struct MotionFrame {
    s16 unk_00;
    u16 unk_02;
    u16 unk_04;
    u16 unk_06;
    u16 unk_08;
    u16 unk_0A;
    s16 unk_0C[0x3C];
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
 * D_800A371C: the distance
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
 * the rest is reserved padding. fighter_Init initializes record [idx]. */
typedef struct Unk80101EC8Record {
    /* the other record: [1] for record 0, else [0] (fighter_Init) */
    struct Unk80101EC8Record *other;
    s16 index; /* this record's own index (fighter_Init) */
    /* != 0: fighter_BuildCpuOrMappedPadState hands pad input to
     * fighter_BuildCpuPad */
    s16 cpu_controlled;
    s16 unk_08;
    /* row of D_8008E5CC / D_8008E6A4, index of D_8008D9EC */
    s16 unk_0A;
    s16 unk_0C;
    s16 unk_0E; /* column of D_8008E5CC / D_8008E6A4 */
    u8 unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14; /* -1 == none, else index of D_8008EB80 */
    u8 unk_16[0x1A - 0x16];
    s16 size_scale;
    s16 base_move_frame_step;
    s16 unk_1E;
    s16 unk_20;
    u8 unk_22[0x24 - 0x22];
    /* this frame's pad input (fighter_Update copies it whole) */
    PadState pad_input;
    /* frames since the record started (fighter_Update) */
    s32 update_count;
    s16 move_frame;      /* current frame of the move */
    s16 move_frame_frac; /* frame fraction, 0x1000 = one frame */
    s16 move_frame_step; /* frame-fraction step */
    s16 unk_46;
    /* motion data id (motion_LoadFighterData);
     * fighter_AssignMotionSlotAndScanDataIdNibbles finds it in D_800A38C4 */
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
     * (motion_ResolveMoveData's out flag), 0: in a fighter slot (its own, or
     * the other fighter's when borrow_other_motion is set) */
    s16 move_is_common;
    u8 cur_move_is_common;
    u8 cur_move_slot;
    u8 unk_62;
    u8 unk_63;
    u16 unk_64;
    u16 unk_66;
    s16 pose_interp_weight;
    /* byte 0 of the current move's frame metadata (fighter_StartMoveScript) */
    u16 move_kind;
    s16 prev_move_kind;
    s16 unk_6E;
    s16 unk_70;
    s16 unk_72;
    s32 saved_collision_y;
    s16 unk_78;
    s16 move_started;
    MoveScript *queued_move; /* buffered next move (fighter_Update) */
    s16 queued_move_is_common;
    u16 unk_82;
    s16 unk_84;
    /* row of unk_3A8 / unk_3F8 / unk_3FE / unk_404 */
    s16 unk_86;
    s16 unk_88;
    s16 unk_8A;
    /* != 0: the second blade (unk_234 / SPAD unk48 points) is live */
    s16 unk_8C;
    s16 unk_8E;
    s16 unk_90;
    s16 unk_92;
    s16 unk_94;
    s16 is_defeated;
    SVec4i16 horizontal_probe_normal;
    u8 unk_A0;
    u8 weapon_hit_start_frames[2];
    u8 weapon_hit_end_frames[2];
    u8 unk_A5;
    u8 unk_A6;
    u8 unk_A7;
    u8 object_pickup_frame;
    u8 opponent_turn_frame;
    u8 unk_AA;
    u8 unk_AB;
    u8 unk_AC;
    /* != 0: fight_UpdateFighterInteractionsAndPointCaches re-runs
     * fighter_ResolveWeaponContact for both records
     */
    u8 unk_AD;
    u8 unk_AE;
    u8 unk_AF;
    u8 unk_B0;
    u8 unk_B1;
    u8 unk_B2;
    u8 unk_B3;
    u8 unk_B4;
    u8 unk_B5[0xB8 - 0xB5];
    VECTOR collision_pos;
    VECTOR prev_collision_pos;
    Vec3i32 base_pos;
    u8 unk_E4[0xE8 - 0xE4];
    Vec3i32 root_offset;
    Vec3i32 world_pos;
    u8 unk_100[0x104 - 0x100];
    VECTOR unk_104;
    /* per blade; fighter_ProcessWeaponBodyHits indexes it with its 0/1 blade
     * flag */
    VECTOR weapon_point_delta[2];
    VECTOR unk_134;
    s32 unk_144;
    s32 ground_y;
    s16 remaining_turn;
    s16 forward_move_rate;
    s16 unk_150;
    s16 use_root_offset_yaw;
    s16 root_offset_yaw;
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8 unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8 unk_164[0x168 - 0x164];
    Vec3i32 camera_fighter_point;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    /* fighter_UpdateBoneWorldData: bone 17 / 14 translations,
     * y + ((size_scale * 71) >> 11) */
    Vec3i32 unk_198[2];
    /* fighter_UpdateBoneWorldData: floor y under unk_198[i]
     * (collision_TestSegmentWithWork segment test) */
    s32 unk_1B0[2];
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
    s16 unk_1DA;
    s16 downward_probe_hit;
    u8 unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    /* fighter_UpdateBoneWorldData: bone 11's matrix applied to D_800A3138 (0,
     * 0x1000, 0) */
    Vec3i32 unk_1EC;
    Vec3i32 unk_1F8;
    u8 unk_204[0x210 - 0x204];
    /* fight_UpdateFighterInteractionsAndPointCaches: copy of scratchpad points
     * 0x1F800000 + idx * 0x24 */
    Unk80107850Rec unk_210[3];
    /* fight_UpdateFighterInteractionsAndPointCaches: copy of scratchpad points
     * 0x1F800048 + idx * 0x18 */
    Unk80107850Rec unk_234[2];
    VECTOR unk_24C;
    /* fighter_Update: copy of scratchpad point unk00[idx][0] */
    Unk80107850Rec unk_25C;
    s32 unk_268;
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
    s16 unk_288[2];
    s32 unk_28C;
    /* the motion frame fighter_Update last used */
    MotionFrame saved_motion_frame;
    u16 saved_frame_offset_yaw;
    u16 unk_316;
    s16 pose_blend_progress;
    s16 unk_31A;
    s16 pose_blend_step;
    u8 unk_31E[0x320 - 0x31E];
    Vec3i32 pose_blend_base_shift;
    u8 unk_32C[0x330 - 0x32C];
    s16 carried_object_count;
    /* queued kinds, carried_object_count of them (object_InitKindQueue /
     * object_QueueKind)
     */
    s16 carried_object_kinds[12];
    u8 unk_34A;
    u8 unk_34B;
    u8 unk_34C;
    u8 unk_34D;
    /* written by fighter_BuildCpuOrMappedPadState for the OTHER record */
    u8 unk_34E;
    u8 unk_34F[0x350 - 0x34F];
    s16 unk_350;
    /* index into node_GetSlotMatrixPointerArray()'s MATRIX * table and
     * SPAD->unkA8[] (fighter_DispatchArmedBoneVectorEventAndDisarm) */
    s16 unk_352;
    /* fighter_StoreBoneLocalVectorAndArmEventTrigger's gte_stlvnl output;
     * fighter_DispatchArmedBoneVectorEventAndDisarm's gte_ldlv0 input */
    Vec3i32 unk_354;
    CpuRoute cpu_route; /* 0x360 */
    s32 unk_394;
    s16 unk_398;
    s16 unk_39A;
    u8 unk_39C;
    u8 unk_39D;
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
    u8 unk_3BC;
    u8 unk_3BD;
    u8 unk_3BE[0x3C0 - 0x3BE];
    u8 unk_3C0;
    u8 unk_3C1;
    s16 unk_3C2;
    s32 unk_3C4;
    s32 cpu_pad_script_held;
    s32 unk_3CC;
    /* the pad record fighter_BuildCpuPad builds for
     * fighter_BuildCpuOrMappedPadState */
    PadState last_cpu_pad;
    u16 cpu_pad_build_count;
    u8 unk_3EA[0x3EE - 0x3EA];
    s16 unk_3EE;
    s16 unk_3F0;
    u8 unk_3F2;
    u8 unk_3F3;
    u8 unk_3F4;
    u8 unk_3F5;
    u8 unk_3F6;
    u8 unk_3F7;
    /* [unk_86] distance bounds (fighter_InitCpuControl) */
    s16 unk_3F8[3];
    s16 unk_3FE[3];
    s16 unk_404[3];
    s16 scale_distance_offset;
    s8 last_chosen_script_index;
    s8 last_chosen_script_list_index;
    s16 chosen_script_start_x;
    s16 chosen_script_start_z;
    s16 chosen_script_radius;
    /* fighter_BuildCpuPad: 8 (target id, count) pairs */
    u8 unk_414[8][2];
    u8 unk_424;
    u8 unk_425;
    u8 unk_426;
    u8 unk_427;
    s16 unk_428;
    s16 opponent_snapshot_x;
    s16 opponent_snapshot_z;
    s16 opponent_snapshot_radius;
    s32 unk_430;
    s32 nearest_nav_crossing_distance;
    s16 unk_438;
    s16 opponent_angle_from_orientation;
    s16 opponent_angle_from_orientation_abs;
    s16 unk_43E;
    u8 unk_440;
    u8 unk_441;
    u8 unk_442;
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
    s16 unk4;
    s16 unk6;
    u16 unk8;
    s16 unkA;
    u8 unkC;
    u8 unkD;
} Tbl8008E194;

/* The twelve 0x64-byte object records at 0x80106A78. object_SpawnFromFighter
 * spawns one (its kind indexes D_8008E194 / D_8008EB80), object_UpdatePool
 * moves them, object_ResolveFighterHits tests them against both fighters,
 * object_SubmitPoolTransforms hands them to the effect calls; kind == -1 marks
 * a free record.
 */
typedef struct {
    /* 0 at spawn; object_UpdatePool adds 1 per step while is_moving != 0 */
    s16 moving_ticks;
    s16 kind; /* -1 = free */
    u8 unk_04;
    u8 unk_05;
    /* .index of the spawning record (object_SpawnFromFighter) */
    u8 owner;
    u8 unk_07;
    u8 is_attached;
    /* index into the opponent's bone-matrix table,
     * node_GetSlotMatrixPointerArray(owner < 1) (object_AttachToOpponentBone,
     * object_SubmitAttachedTransform) */
    u8 attached_bone;
    /* own index in g_object_pool once allocated; 0xFF = free (set by
     * object_ResetPool at init and by object_UpdatePool once kind == -1) */
    u8 slot;
    u8 unk_0B;
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

/* 2-byte {a,b} threshold pairs. D_8008EA44: indexed by (type - 2), 5 entries
   (types 2..6); D_8008EBFC: indexed by the category snd_GetStageSoundIndex
   returns, 6 entries (both read by snd_UpdateStageSounds). */
typedef struct {
    u8 a;
    u8 b;
} Unk8008EA44Entry;

/* 6-row tables fight_UpdatePairedInputTotalsEventsAndMidpointEffects selects by
 * D_80101EC8[0].move_kind mode (row 0..5): unk0 scales the Judge sin/cos
 * offset, unk2 is added to y; D_8008EB6C[row] is passed as
 * fight_RecordReplayEventAndDispatchSoundOrEffect's arg1. */
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
    u8 unk5;
    u8 unk6;
    u8 unk7;
    /* +0x8: fighter_UpdateCpuNavigationChooseScriptsAndHeldInput scales a
       random 12-bit draw by an element, and compares [4] with a random 8-bit
       draw */
    u8 pick_weight[7];
    /* +0xF: fighter_UpdateCpuNavigationChooseScriptsAndHeldInput scales a
       random 12-bit draw by an element, and compares [5] >> 2 and [6] >> 2 with
       a random 8-bit draw */
    u8 unkF[8];
    u8 unk17;
} Unk80099D88Rec; /* size 0x18 */

/* Rows of eight 4-byte entries starting at 0x8009A8C8: each 0x20-byte row ends
 * with a zero entry (0x8009A8E4 / 0x8009A904 / 0x8009A924). Both readers index
 * the column 1-based, [row][D_800A37A0 - 1]: fighter_InitCpuControl reads
 * unk0/unk1, fighter_UpdateCpuNavigationChooseScriptsAndHeldInput the mask
 * halfword. */
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
    Vec3i32 unk_00;
    s32 wC;
    SVECTOR unk_10;
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
 * camera_ApplyReplayView add its fields to a Rec44's unk_00, unk_10 and w18,
 * and camera_UpdatePadOffset zeroes them. The fields sit at Rec44's offsets;
 * +0x0C and the SVECTOR pad are never accessed. */
typedef struct Unk800FF5C8Rec {
    Vec3i32 unk_00;
    u8 unk_0C[4];
    SVECTOR unk_10;
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
 * - h4 / h6 / h8: world_pos x / y / z; hA: orientation.vy & 0xFFF with unk_B3
 * in the top four bits; hC: ground_y; hE / h10 / h12: unk_64 / unk_66 /
 * pose_interp_weight.
 * - b14..b16: unk_1E6 / unk_1E8 / unk_1EA >> 2, read back signed; b17: bit 0 =
 * cur_move_is_common != 0, bit 1 = cur_move_slot != 0; b18: unk_62; b19:
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
 * (replay_Reset clears unk_0 to -1, replay_BeginFrame ages them,
 * replay_RecordModelDraw adds or refreshes one, replay_PlayFrame replays them).
 * unk_0: -1 = free, else a frame counter; unk_2: frames seen; unk_3:
 * replay_RecordModelDraw's arg1; unk_4: the s16 x / y / z of arg2; unk_A:
 * arg3[0] & 0xFFF with arg0 in the top four bits; unk_C / unk_E: arg3[1] /
 * arg3[2]. */
typedef struct Unk800F68E0Rec {
    s16 unk_0;
    u8 unk_2;
    u8 unk_3;
    s16 unk_4[3];
    u16 unk_A;
    s16 unk_C;
    s16 unk_E;
} Unk800F68E0Rec;

/* The 0x20 0x10-byte records of the event table g_replay_events (replay_Reset
 * sets unk_0 to 0xFF = free, replay_BeginFrame frees the current frame's,
 * replay_RecordEvent fills one, replay_PlayFrame replays those whose unk_0 is
 * the frame index). unk_0: the frame (g_replay_write_frame); unk_1 / unk_2:
 * replay_RecordEvent's arg0 / arg1; unk_4: the s16 x / y / z of arg2; unk_A:
 * arg3[0..2]. */
typedef struct Unk80101BF0Rec {
    u8 unk_0;
    u8 unk_1;
    u8 unk_2;
    u8 pad3;
    s16 unk_4[3];
    u16 unk_A[3];
} Unk80101BF0Rec;

/* The 4-byte record fight_SplitFrameCounterIntoMinutesSecondsHundredths fills
 * from the frame counter D_800A3858 (30 per second): unk_0 = count / 1800,
 * unk_2 = count / 30 % 60, unk_3 = count % 30 * 100 / 30 (99 / 59 / 99 past
 * 0x2BF1F). gpu_DrawTimeFieldsAndDecimalCounter draws the three as two-digit
 * numbers; fight_PackTimeRecordsAndInitPresentation copies them into its save
 * record at +0x2D..+0x2F. fight_UpdateModeDisplaysAndMatchProgress also fills
 * only unk_2 = n / 30 and unk_3 = n % 30
 * * 100 / 30 from a frame countdown n, for gpu_DrawPairedMarkRowsAndTimeFields
 * to draw. */
typedef struct Unk8001CD68Rec {
    s16 unk_0;
    u8 unk_2;
    u8 unk_3;
} Unk8001CD68Rec;

/* Two 0x58-byte records set up side by side by
 * node_InitRootNodePairWithNegatedRotationMatrices: 0x80101DF0 (unk0 = 0x64,
 * rotation_order = 5; pointer stored to D_800A3708) and 0x800FF638 (unk0 =
 * 0x65, rotation_order = 2; pointer stored to D_800A370C).
 * node_SetMatrixFromNegatedRot passes a negated stack copy of xf.rot and &work
 * to D_800F66A0[rotation_order], then copies work to xf.mat;
 * gpu_ResetDrawQueueAndPrepareViewAndLighting writes work.t just before that
 * call. node_OrientAndQueueViewOffsetNode passes &xf.rot to D_800F66A0[0] and
 * &xf.mat to ApplyMatrix. D_800EEDF0 and D_800EF070 are two more records of
 * this layout: node_InitIdentity initialises the first, wave_Init sets up the
 * second and passes it to node_UpdateWorldMatrix, which reads matrix_state. rot
 * is the RotMatrix-style angle SVECTOR the rotation handlers take; mat and work
 * are libgte MATRIXes. */
typedef struct {
    SVECTOR rot; /* +0x00 */
    MATRIX mat;  /* +0x08 */
} Unk80101DF0Xform;

typedef struct Unk80101DF0Record {
    u8 unk0;                               /* +0x00 */
    s8 unk1;                               /* +0x01 */
    s16 unk2;                              /* +0x02 */
    s16 unk4;                              /* +0x04 */
    s16 matrix_state;                      /* +0x06 */
    s16 rotation_order;                    /* +0x08 D_800F66A0 index */
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
 * D_800F66A0[rotation_order] on &xf.rot / &xf.mat (the angles as stored,
 * straight into xf.mat; unlike node_SetMatrixFromNegatedRot, no negation and no
 * work); gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards
 * and gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes queue
 * each one on g_draw_queue at most once, guarded by unk58.
 */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    u8 unk58;               /* +0x58 */
    u8 unk59[0xF];          /* +0x59 */
} Unk800A6690Rec;           /* 0x68 */

/* The 0x68-byte entries of the D_800A9CF8.unkC table (the buffer
 * node_BindFrameDataAndReserveEntryBuffer is handed; it returns the end, base +
 * count * 104). Each is a transform node of the Unk80101DF0Record layout paired
 * with one gpu_GetGridNodeRecordTable entry of the same index
 * (D_800A9CF8.unk10): node_InitFrameEntriesFromWorldTranslations initialises
 * the node and copies the paired entry's xf.mat.t into work.t;
 * node_StartFrameAnimatedEntryAndInitPairedNode /
 * node_AdvanceFrameAnimatedEntriesAndQueueDraws run the node from the
 * D_800A9CF8.unk8 key frames and queue it on g_draw_queue. unk58 is a word
 * frame counter (lw/sw: -1 idle, 0 started by
 * node_StartFrameAnimatedEntryAndInitPairedNode, -2 done), unk5C the word Y
 * angle node_StartFrameAnimatedEntryAndInitPairedNode stores, unk60 an s16
 * node_AdvanceFrameAnimatedEntriesAndQueueDraws sets to 0x1000 at the last key
 * frame and then lowers by 0x40 per call to 0 (at 0, unk58 = -2 unless
 * D_800A9CF8.unk4 is 4); +0x62..0x67 are not accessed. Not the Unk800A6690Rec
 * layout: that table's +0x58 is a byte. */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 unk5C;              /* +0x5C */
    s16 unk60;              /* +0x60 */
    s8 pad62[6];            /* +0x62 */
} Unk800A9CF8Entry;         /* 0x68 */

/* The 21 ten-byte node-template records at 0x80094B96 (up to D_80094C68):
 * node_InitObjectHierarchyFromTemplates builds
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
 * bytes, its arena_SetBlockEnd block) and D_800A9A10[] / slot_GetObject hand
 * out. The header (0x00..0x2B) is followed by three arrays of 0x68-byte
 * transform nodes (slot_RebaseObjectPointerNodeLinksAndRefreshMatrixPointers
 * rebases them as 0x15, 0x14 and 0x14 records), then three 20-entry pointer
 * tables.
 * - unk_00: flag word; slot_FinalizeObjectLoadSoundTexturesAndArena sets bit 1
 * and keeps the TIM-group request (D_80094B88[arena_id] & 0x1F) in bits 16..20,
 * which slot_RequestTimGroupsAndMarkReloadOnChange /
 * slot_GetLastLoadTimGroupRequest read as (word >> 16) & 0x1F; no code reads it
 * through half[].
 * - arena_id: slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a0,
 * the arena_OpenBlock id and the D_800A9A10 index; load_status: 0, 1 after a
 * fresh load (slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock), -2 to
 * request a reload (slot_RequestTimGroupsAndMarkReloadOnChange);
 * slot_FinalizeObjectLoadSoundTexturesAndArena returns at once when it is 0,
 * else clears it and runs its setup; unk_08:
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a1, indexing the
 * D_80094C68 / D_80094B48 tables.
 * - unk_10 / unk_14 / unk_16:
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock's a0 / a0 / a0 + 3
 * (unk_14 / unk_16 are the reloc_RelocateBlock / gpu_OffsetBlockTexPrimStreams
 * ids); position_scale: the Q12 scale node_InitObjectHierarchyFromTemplates
 * sets from D_80094C68.
 * - unk_18: written as the word 0x8000 by
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock, read as three
 * colour bytes by node_SetPrimitiveTableSelectOnObjectNodesAndPatchColor.
 * - unk_1C / unk_20: the resource blocks
 * slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock stores; unk_24 /
 * softbody_params: the attached Unk8003F62CRec block and the s16 list
 * softbody_SetupObject walks up to its -3 end (softbody_RebaseObject moves both
 * by its a1).
 * - transform_nodes[]: node 0 is the root
 * (node_QueueEntryNodesFromTransformOrUpdatedSlotNode reads its xf.mat);
 * node_InitObjectHierarchyFromTemplates builds 0..20,
 * node_BuildProjectedNodeList fills projected_nodes[] (unk2 = -1 ends it),
 * node_BuildIdToNodeMapWithDuplicates copies duplicates into duplicate_nodes[]
 * (unk58 = the source node; 0 ends it).
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
    } unk_00; /* +0x0000 */

    s16 arena_id;       /* +0x0004 */
    s16 load_status;    /* +0x0006 */
    s16 unk_08;         /* +0x0008 */
    u8 pad0A[6];        /* +0x000A */
    u16 unk_10;         /* +0x0010 */
    s16 position_scale; /* +0x0012 */
    s16 unk_14;         /* +0x0014 */
    s16 unk_16;         /* +0x0016 */

    union {
        s32 word;
        u8 byte[4];
    } unk_18; /* +0x0018 */

    s32 unk_1C;                          /* +0x001C */
    s32 unk_20;                          /* +0x0020 */
    void *unk_24;                        /* +0x0024 */
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
 * those (unk8 / unkC = the grid cell's column / row * 2000 - 32000);
 * gpu_CollectGridDrawablesInvokeLoadedSelectorCallbackAndDrawBillboards and
 * gpu_QueueMaskedGridRecordsWithCellAttributesAndCollectUniqueNodes set unk6
 * and the unk7 bits and queue them on g_draw_queue. */
typedef struct {
    s16 unk0;     /* +0x00 */
    s16 unk2;     /* +0x02 */
    u16 unk4;     /* +0x04 */
    u8 unk6;      /* +0x06 */
    u8 unk7;      /* +0x07 */
    s32 unk8;     /* +0x08 */
    s32 unkC;     /* +0x0C */
} Unk800A4750Rec; /* 0x10 */

/* One entry of the CD file table (all 159 entries equal the disc's ISO9660
 * directory records). */
typedef struct {
    CdlLOC loc; /* the file's start position */
    u32 size;   /* the file's length in bytes */
} CdFileEntry;

/* The CD read / CD audio words at 0x80101E60..0x80101EA7: the tail of the
 * CD state block D_80101E58 (CdState below). Member widths follow the original
 * accesses; 0x80101E91..93 is alignment padding. unk3C..unk44 belong to the
 * object because cdrom_UpdateAudio (compiled -G8) keeps the address of its
 * accesses to unk3C / unk44 in a register only for a variable larger than 8
 * bytes. unk3E (u16) and expected_pos (s32; no access reveals its signedness)
 * lie in that span but cdrom_UpdateAudio never touches them; they are typed by
 * their other users' accesses (Q13/Q14). `pair` holds a copy of a
 * g_cd_file_table element (a CdFileEntry), copied whole by cdrom_StartRead /
 * cdrom_StartAudio. */
typedef struct {
    s16 unk00;        /* 0x80101E60 */
    s16 unk02;        /* 0x80101E62 */
    s16 unk04;        /* 0x80101E64 */
    s16 unk06;        /* 0x80101E66 */
    s16 unk08;        /* 0x80101E68 */
    s16 unk0A;        /* 0x80101E6A */
    CdFileEntry pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14;        /* 0x80101E74 */
    s32 unk18;        /* 0x80101E78 */
    s32 unk1C;        /* 0x80101E7C */
    s32 cd20;         /* 0x80101E80 */
    s32 dest_buffer;  /* 0x80101E84 */
    s32 unk28;        /* 0x80101E88 */
    s32 unk2C;        /* 0x80101E8C */
    u8 unk30;         /* 0x80101E90 */
    s32 unk34;        /* 0x80101E94 */
    s16 unk38;        /* 0x80101E98 */
    s16 unk3A;        /* 0x80101E9A */
    s16 unk3C;        /* 0x80101E9C */
    u16 unk3E;        /* 0x80101E9E */
    s32 expected_pos; /* 0x80101EA0 */
    s32 unk44;        /* 0x80101EA4 */
} Unk80101E60Rec;

/* The CD module's state block, one 0x50-byte object at 0x80101E58 (span
 * bounded by owner ruling Q43). cdrom_StartAudio forms the CdlSetfilter
 * parameter (file, chan) as &rec.unk02 - 0xA and reloads rec.unk00 only after
 * its CdFileEntry copy into rec.pair (one object for the scheduler);
 * cdrom_Update forms &rec.pair off later members. */
typedef struct {
    u8 file;            /* 0x80101E58 */
    u8 chan;            /* 0x80101E59 */
    s32 unk04;          /* 0x80101E5C */
    Unk80101E60Rec rec; /* 0x80101E60 .. 0x80101EA7 */
} CdState;

/* The parameter block 0x80102778..0x80102787 (fight_InitSettings sets
 * every byte of it except 0x82/0x83): two u16 values, three per-player byte
 * pairs kept as one array (unk_4[2 * k + player], [0] = P1, [1] = P2), a
 * fourth per-player pair only other functions touch (unk_A, indexed by player
 * in fighter_ConfigureCpuControl) and four single bytes. One object:
 * menu_PrintEditAndApplyConfigWithPerPadCursors reaches unk_4 and unk_E as
 * offsets from the address of unk_C.
 */
typedef struct {
    u16 unk_0[2];
    u8 unk_4[6];
    u8 unk_A[2];
    u8 unk_C;
    u8 unk_D;
    u8 unk_E;
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
    /* 0x04 */ MATRIX *unk4;
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
 * (unk_00), the name sheet and the per-character name cells (unk_0C / unk_10,
 * three cells per character after a first group), the digit sheet and its cells
 * (unk_14 / unk_18; the walker shifts the sheet's ubase per digit), and
 * per-character x offsets (unk_1C). 4D0A8 hands it D_8009B0C0,
 * menu_DrawConfirmSheetsAndDigitPairs MOD.BIN's unk_3C. */
typedef struct {
    Unk8009B0E0Record *unk_00[3];
    Unk8009B0E0Record *unk_0C;
    Unk8009B400Record *unk_10;
    Unk8009B0E0Record *unk_14;
    Unk8009B400Record *unk_18;
    s16 *unk_1C;
} Unk8006D808Rec;

/* MOD.BIN's unk_3C list: gpu_DrawIndexedCellsAndDecimalDigitPairs's set, then
 * the three sheets menu_DrawConfirmSheetsAndDigitPairs draws. */
typedef struct {
    Unk8006D808Rec unk_00;
    Unk8009B0E0Record *unk_20[3];
} Unk8006DD94List;

/* MOD.BIN (resource file 2), the root 596CC's
 * menu_LoadResourcesInitStateSelectFirstEnabledRow loads at its work area +
 * 0x58; it keeps it in D_800A34FC's word 9, and
 * gpu_AdvanceBufferCounterAndBuildDrawContext copies that into word 1 of the
 * draw context. After the head:
 * - unk_14..unk_40: the twelve lists reloc_RelocateTwelveListsAndLoadVabPack
 * relocates (reloc_RelocateOffsetList: entries up to a 0 word, -1 entries
 * skipped). Their entries are sprite sheets (unk_20 pairs each sheet with a
 * -1), except unk_3C, menu_DrawConfirmSheetsAndDigitPairs's list
 * (Unk8006DD94List); reloc_RelocateTwelveListsAndLoadVabPack walks the twelve
 * slots as one run of list pointers.
 * - unk_44: the Rec_8006C21C rows
 * menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads draws; unk_48: the
 * Size2s16 box sizes (x = width, y = height)
 * gpu_DrawSheetPairCenteredTileWithOptionalSelection reads. The -1 ending
 * reloc_RelocateSelfOffsets's offset list follows at +0x4C; no reader touches
 * it. */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B0E0Record **unk_14;
    Unk8009B0E0Record **unk_18;
    Unk8009B0E0Record **unk_1C;
    Unk8009B0E0Record **unk_20;
    Unk8009B0E0Record **unk_24;
    Unk8009B0E0Record **unk_28;
    Unk8009B0E0Record **unk_2C;
    Unk8009B0E0Record **unk_30;
    Unk8009B0E0Record **unk_34;
    Unk8009B0E0Record **unk_38;
    Unk8006DD94List *unk_3C;
    Unk8009B0E0Record **unk_40;
    Rec_8006C21C *unk_44;
    Size2s16 *unk_48;
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
 * keeps at the start of gpu_InitPrimBufferPair's returned space (D_800A35C4;
 * the arena cursor D_800A356C moves past it), as 596CC's Unk800A34FCRec.
 * - unk_00 / unk_04: one s16 per player each (counted down from 0x1E by the
 * draw handlers).
 * - unk_08: the frame counter StepSlideBuildFrameRecordAndDispatchStateHandler
 * advances; unk_0C: its buffer counter (bit 0 picks the half of the
 * Unk8006E49CRec pair).
 * - unk_10: the draw offset the handlers hand to SetDrawOffset. */
typedef struct {
    s16 unk_00[2];
    s16 unk_04[2];
    s32 unk_08;
    s32 unk_0C;
    u16 unk_10[2];
} Unk800A35C4Rec;

/* 596CC's work block: the 0x34 bytes
 * menu_LoadResourcesInitStateSelectFirstEnabledRow keeps at the start of
 * gpu_InitPrimBufferPair's returned space (D_800A34FC; its arena cursor
 * D_800A3500 moves past the block).
 * menu_LoadResourcesInitStateSelectFirstEnabledRow zeroes unk_0C / unk_10 /
 * unk_12, sets both unk_28 halves to 5 and unk_30 to bit 0 of D_800A3524's word
 * 8 (gpu_GetPrimBufferAndRefreshStrip compares and refreshes it), and stores
 * the MOD.BIN root in unk_24.
 * - unk_0C: the s16 pair pad_StepOffsetsGetDirectionEvents steps (its arg2);
 * the draw functions use unk_0C[0] / [1] as x / y offsets.
 * - unk_28: one s16 per player
 * (menu_DrawKeyConfigRowBitSheetsTilesAndGradientQuads,
 * menu_SetExclusiveRowBit, menu_HandleRowBitInput); menu_HandleRowBitInput and
 * menu_DrawKeyConfigRowBitsGetEmptyRowSides also test the pair as one word (==
 * 0x50005: both 5), as SelWork's f1C / f20. No code touches the other bytes. */
typedef struct {
    u8 pad00[0xC];
    s16 unk_0C[2];
    s16 unk_10;
    s16 unk_12;
    u8 pad14[0x10];
    Unk8006919CRec *unk_24;

    union {
        s16 half[2];
        s32 word;
    } unk_28;

    u8 pad2C[4];
    u8 unk_30;
    u8 pad31[3];
} Unk800A34FCRec;

#endif /* GAME_H */
