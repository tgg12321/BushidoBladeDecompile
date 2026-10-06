#ifndef GAME_H
#define GAME_H

/* Game types for the translation units in src/main/ (SLUS_006.63's game code): records, tables
 * and object layouts. bb2.h, which includes this file, holds the declarations moved out of the
 * pre-restructure headers plus the identical multi-TU declarations hoisted in 42f458af0; other
 * externs, some of them shared by several TUs, are still declared locally. Sony's library types
 * come from include/psxsdk/. */

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
#include <psxsdk/libsn.h>
#include <psxsdk/libsnd.h>
#include <psxsdk/libspu.h>

/* The 0x24-byte file record at 0x80106A50 (func_80037F40 checksums it as one
 * block; func_800167EC initialises it). */
typedef struct {
    u8 unk_0;
    u8 unk_1;
    s32 unk_4;
} FileTimeRec;

typedef struct {
    s32 unk_00;             /* 0x80106A50 */
    u8 unk_04;              /* 0x80106A54 */
    u8 unk_05[3];
    FileTimeRec times[3];   /* 0x80106A58 */
    u8 color[3];            /* 0x80106A70 */
    u8 flags;               /* 0x80106A73: bits 0/1/2 = file_GetFlag0/1/2 */
} FileRecord;

/* The game's display double buffer at 0x800F7438: two 0x4090-byte records, one per
 * frame parity (D_800A36AC & 1). disp_Init / func_8006E10C pass +0x00 to
 * SetDefDrawEnv and +0x5C to SetDefDispEnv for each record; main clears +0x70 with
 * ClearOTagR(ot, 0x1008) (0x1008 words = 0x4020 bytes, which ends the record) and
 * draws it from its last entry, DrawOTag(+0x408C). */
typedef struct {
    DRAWENV draw;   /* +0x00 */
    DISPENV disp;   /* +0x5C */
    u32 ot[0x1008]; /* +0x70 ordering table */
} GpuDb; /* 0x4090 */

/* One VRAM-scroll channel at D_800EF848 (0x134 bytes each). */
typedef struct {
    s32 phase;          /* +0x000 */
    DR_MOVE move[2][6]; /* +0x004: one bank per frame parity, two packets per level */
    s16 ctl[7];         /* +0x124: filled from D_80099C34 by func_80048F58 */
} MoveChannel;

/* 3-word record table at 0x800F1198, terminated by an all-zero record.
 * Object model evidence: the original binary walks this table with a 12-byte-stride induction
 * register (asm/funcs/func_80062020.s:.L80062038, `addiu $v1, $v1, 0xC`)
 * and addresses the record members through one base register at
 * displacements 0/4/8 (`sw $zero, 0x8($v0)` / `sw $zero, 0x4($v0)`), i.e.
 * record stride and base+offset addressing, not mere symbol adjacency.
 * Replaces the splat per-word scalars D_800F1198 / D_800F119C / D_800F11A0. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F1198Record;

/* 0x800A9CF8 header block (0x18 bytes). Evidence for the aggregate: in the
 * original binary func_8004473C forms &D_800A9D08 in a register and reads
 * D_800A9CF8 / D_800A9CFE as base-0x10 / base-0xA (`lhu 0($a3)` /
 * `lh 6($a3)` with $a3 = $a0 - 0x10) -- one base register reaching three
 * of these addresses by signed displacement, i.e. base+offset addressing
 * of one object, not symbol adjacency. func_80044C70 corroborates by
 * bumping the unk8 / unkC pointer pair together. Replaces the splat
 * per-word scalars
 * D_800A9CF8 / D_800A9CFA / D_800A9CFC / D_800A9CFE / D_800A9D00 /
 * D_800A9D04 / D_800A9D08. */
typedef struct {
    s16 unk0;
    s16 unk2;
    s16 unk4;   /* stage id (stage_GetId) */
    s16 unk6;   /* entry count */
    s32 unk8;
    s32 unkC;   /* entry table (stride 0x68) */
    s32 unk10;  /* game_GetCharData() table (stride 0x68) */
    s32 unk14;
} Unk800A9CF8Header;

/* Per-lane slot record table at 0x800F0EC8: 2 lanes x 10 slots x one
 * 3-word record (12 bytes; lane stride 120). Object model evidence:
 * the original binary addresses all three words through ONE offset register per access site --
 * asm/funcs/func_80063E10.s computes lane*120 (`sll $a0,$s7,4; subu $a0,$a0,$s7;
 * sll $a0,$a0,3`), adds the slot offset held in $s6, and reads
 * %lo(D_800F0EC8/ECC/ED0)($at) with that same $a0 added to each base; the
 * writer asm/funcs/func_80063BD0.s forms lane*120 + slot*12 the same way and
 * stores the three words at displacements 0/4/8 of that offset. Record stride
 * and base+offset addressing, not symbol adjacency. The slot bitmask
 * D_800A3454[lane] and the sibling SVECTOR table D_800F1000[lane][10] index the
 * same lane/slot pair. Replaces the splat per-word scalars D_800F0EC8 /
 * D_800F0ECC / D_800F0ED0. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0EC8Record;

/* Flare-slot position table at 0x800F0E38: 12 slots x one 3-word {x, y, z}
 * record (12 bytes; 0x800F0E38 + 12 * 12 == 0x800F0EC8, the table above).
 * Object model evidence: the original binary addresses all three words through ONE offset register per
 * access site -- the spawner asm/funcs/func_80062FEC.s forms slot*12
 * (`addu $v1,$a1,$a2; sll $v1,$v1,2` with $a1 = slot*2) and stores
 * %lo(D_800F0E38/E3C/E40)($at) with that same $v1 added to each base; the
 * drawer asm/funcs/func_80063084.s holds slot*12 in $s6 and reads
 * %lo(D_800F0E38/E3C/E40)($at) with $s6 added to each base. The slot bitmask
 * D_800A3448 and the age table D_800F0BEC[slot] index the same slot. Record
 * stride and base+offset addressing, not symbol adjacency. Replaces the splat
 * per-word scalars D_800F0E38 / D_800F0E3C / D_800F0E40. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk800F0E38Record;

/* 0x8009BC0C: the eight option rows of the list menu func_800693CC drives, one
 * {state, mode} byte pair per row (0x8009BC0C..0x8009BC1B). Object model evidence
 * from the original binary: func_800693CC indexes the row as (D_800A34F8 & 0xF) << 1
 * and reads `lbu %lo(D_8009BC0C)($at)` (state: accept / reject / return value) and
 * `lbu %lo(D_8009BC0D)($at)` (mode: passed to the three render calls, 1/2/3 select
 * the D_800A3524 bit-3 action) through that same index -- stride-2 records.
 * Replaces the splat per-word symbols D_8009BC0C / D_8009BC0D (per-word splat
 * symbol -> aggregate merge family). */
typedef struct {
    u8 state;
    u8 mode;
} MenuOption;

/* 0x8009BCF8: 2 pages x 10 two-byte records (0x8009BCF8..0x8009BD1F; D_8009BD20
 * follows), one page per character-select page, 2 rows x 5 columns of cells.
 * Object model evidence from the original binary: func_80075F80 (0x800761B4-
 * 0x800761E4) and func_800759D0 (0x80075BC4-0x80075BF0) address it as
 * page * 20 + cell * 2 -- the cell (row * 5 + col) shifted left 1, the page times
 * 5 shifted left 2, added, then `lbu %lo(D_8009BCF8)($at)` -- two-level array
 * indexing of [page][cell]; func_800759D0's cell loop steps through the records
 * flat from the table base (`lui $s6,%hi(D_8009BCF8); addiu $s6,$s6,%lo(D_8009BCF8)`,
 * a 2-byte step), and func_80076D74 reads byte 1 (`lbu %lo(D_8009BCF9)($at)`)
 * through a flat shift-1 index. Data: the unk1 column is 0x00..0x09 (page 0) then
 * 0x0C..0x15 (page 1). Replaces the splat per-word scalars D_8009BCF8 / D_8009BCF9
 * (per-word splat symbol -> aggregate merge family, owner ruling). */
typedef struct {
    u8 unk0;
    u8 unk1;
} Unk8009BCF8Record;

/* The 0x2C-byte record func_8006E49C fills, two at a time (51268 / 5ED34 / 64FD8 keep the pair at
 * the start of their work area; getters `base + i * 44`). Each word holds the start of a region
 * func_8006E49C carves from its buffer: +0x00 0x9C40 bytes, +0x04 0x5DC0, +0x08 0x438, +0x0C 0x640,
 * +0x10 0x1B58, +0x14 0x258, +0x18 0x78, +0x1C 0x78, +0x20 0x110; +0x24 / +0x28 it leaves alone.
 * The 5ED34 / 64FD8 draw contexts copy them into Unk800788B0Rec, whose members name the primitive
 * types; 51268's func_8006E390 copies them into the s32 words of its context array. */
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

/* The eight primitive cursors of a draw context (func_800788B0 fills one bare; Unk8006EACCRec
 * embeds it). The draw functions build primitives at a cursor and advance it.
 * unk_00 / unk_0C hold the s32 cursors func_80073728 / func_80073C78 and func_8007352C return. */
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

/* The VAB pack func_8005C2A8 loads (MOD.BIN's head unk_00 points at one; g_vab_rec_ptr keeps the
   loaded ones): four words, the first three file-relative offsets func_8005C2A8 turns into addresses
   in place. unk_00: the u32 key-event table func_8005C6D0 reads; unk_04: the VabHdr snd_VabOpen opens;
   unk_08: the body snd_VabOpen transfers; unk_0C: the body's size in SPU memory. */
typedef struct {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
} Unk8005C2A8Pack;

/* The 8-byte sprite cell: x / y, its offset from the descriptor's screen position; u / v, its texel
 * offset from the sheet's ubase / vbase; w / h, its size (func_8007352C: x0 = x + env x,
 * u0 = u + ubase, w / h copied). A sheet's cells follow its header(s) (Unk8009B0E0Record.cells).
 * The 0x8009B400 tables hold such cells. Object model evidence from the original binary:
 * asm/funcs/func_8005E098.s and asm/funcs/func_8005D814.s index 0x8009B400 by a
 * digit value through a shift-3 (8-byte-stride) index, and func_8005E098 stores
 * an s16 to offset 0 of the indexed record (`sh $v0,0x0($v1)`); it indexes 0x8009B458 by `sra 13` of an
 * s16 counter (i * 8) and 0x8009B468 / 0x8009B470 by counter * 16 -- pairs of
 * 8-byte records. Data: 0x8009B400..0x8009B44F is 10 records (one per digit),
 * 0x8009B458..0x8009B487 is 6 records of the same shape; D_8009B450 and
 * D_8009B488 follow. Replaces the splat per-word scalars D_8009B458 /
 * D_8009B468 / D_8009B470 in C (per-word splat symbol -> aggregate merge
 * family, owner ruling). */
typedef struct {
    s16 x;
    s16 y;
    u8 u;
    u8 v;
    u8 w;
    u8 h;
} Unk8009B400Record;

/* The 12-byte sprite-sheet header the walkers read (Unk8007352CEnv.header): tp0 / tp1, the
 * texture-page bits (func_80073728: (tp0 & 0xFE1F) + (tp1 << 7), func_8006E480); count, the cell
 * count; cx / cy, the CLUT position (GetClut); ubase / vbase, the texel origin the cells' u / v add
 * to; cells, the cell table that follows. A resource sheet may carry two or three headers (the
 * plain one, then one highlight per player: func_8006F97C, func_800759D0) ahead of one shared cell
 * table, which then starts at the last header's cells (hdr[2].cells).
 * 0x8009B0E0: table of 9 twelve-byte sprite-sheet headers (0x8009B0E0..0x8009B14B),
 * the record func_8007352C reads through Unk8007352CEnv.header. Object
 * model evidence from the original binary: asm/funcs/func_8005C8A8.s forms ONE base %hi/%lo(D_8009B0F8) in $s0
 * and reaches record 8 as `addiu $s0,$s0,0x48`; forms %hi/%lo(D_8009B110) in $s1
 * and reaches records 2 and 1 as `addiu $v1,$s1,-0xC` / `addiu $s1,$s1,-0x18`;
 * forms it again in $s2 and reaches records 0 and 1 as -0x30 / -0x24; its tail
 * loop indexes the table by `(j * 3) << 2` added to that base -- base+offset
 * and 12-byte-stride addressing of one object. Replaces the splat per-word
 * labels D_8009B0E0 / D_8009B0F8 / D_8009B110 / D_8009B11C in C (per-word splat
 * symbol -> aggregate merge family, owner ruling); the dlabels stay
 * in asm/data as data labels. D_8009B14C and D_8009B158 are two more headers of
 * the same shape (func_8005C8A8 passes &D_8009B14C as a header and reads its
 * +2 count byte, the splat label D_8009B14E, which this declaration retires). */
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

/* The head of the resource files func_8006E950 loads. By g_cd_file_table's sizes, file 2 of
 * func_80036EA8's group 2 is MOD.BIN, 3 SEL.BIN, 4 / 5 SEL1 / SEL2.BIN, 6 D_SEL.BIN, 0x32 NAR.BIN.
 * Each file starts with a list of file-relative offsets ending in -1, which func_8006E440 turns
 * into addresses. The first five mean the same in every file: func_8006919C / func_8006EA28 pass
 * unk_00 and unk_04 to the VAB loader func_8005C2A8 (func_80076FF8 / func_80077D10, for D_SEL /
 * NAR, make no such call, and those files' unk_00..unk_08 are equal), func_8006E950 loads the
 * image at unk_08 into VRAM, and func_8006E8CC loads the 640x32 strip at unk_0C or unk_10. The
 * four per-file relocators (func_8006919C, func_8006EA28, func_80076FF8, func_80077D10) return
 * unk_04, where the caller's func_8006E49C buffer starts. The words after the head differ per file. */
typedef struct {
    Unk8005C2A8Pack *unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
} Unk8006E950Head;

/* STAFF.BIN (resource file 0x5F), the root 64FD8's func_80078824 loads at its work area + 0x58
 * (D_800A3610). After the head, one offset list (relocated with the head by func_8006E440, so it
 * ends in -1 like the head's own list): the sprite sheets func_80078654 draws: unk_14[10] first,
 * then unk_14[0] onward while the next entry is not -1. */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B0E0Record *unk_14[0];
} Unk80078824Rec;

/* 12-byte rectangle rows {x, y, w, h, r, g, b}: MOD.BIN's unk_44 (func_8006C21C draws them) and
 * D_SEL.BIN's unk_3C (func_80074B18). */
typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;

/* SEL.BIN's unk_54 block, func_8006ECF4's: two cell tables (unk_00[i], one per player), a 0 word,
 * then the 12-byte sheet headers the cells are drawn with (unk_0C[sel], plus five special cases). */
typedef struct {
    Unk8009B400Record *unk_00[2];
    s32 unk_08;
    Unk8009B0E0Record unk_0C[0];
} Unk8006ECF4Rec;

/* SEL.BIN / SEL1.BIN / SEL2.BIN (resource files 3-5), the root 5ED34's func_8006E534 loads at its work
 * area + 0x58 (D_800A35A8; func_8006EACC hands it to the handlers as Unk8006EACCRec.unk_00). After
 * the head:
 * - unk_14: per entry id two TIM pixel addresses, one per column, func_80070F78 loads (LoadImage).
 * - unk_54..unk_74: the nine lists func_8006EA28 relocates (func_8006920C): unk_54, the
 *   header block func_8006ECF4 draws (Unk8006ECF4Rec); unk_58..unk_74, sprite-sheet lists.
 *   unk_78 is a further offset no code reads.
 * - unk_7C: per player a row of VRAM RECTs (64 bytes) func_80070F78 hands LoadImage as the
 *   destination; unk_80: the bytes func_800720FC reads.
 * - unk_84: TIM pixel addresses func_8006ECF4 loads per character case. */
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
    u8 *unk_7C;
    u8 *unk_80;
    s32 unk_84[5];
} Unk8006EA28Rec;

/* D_SEL.BIN (resource file 6), the root 64FD8's func_800770B8 loads at its work area + 0x58 and
 * keeps in SelWork.f04 (func_80077724 hands it to the handlers as Unk8006EACCRec.unk_00). After the
 * head: unk_14..unk_38, the ten lists func_80076FF8 relocates, each a list of sprite sheets
 * (unk_20 is indexed by the round count SelWork.f65); unk_3C, the rectangle rows func_80074B18
 * draws. */
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
} Win77D94;

/* NAR.BIN (resource file 0x32), the root 64FD8's func_800784E4 loads at its work area + 0x58
 * (D_800A35F8). After the head: table, the cell table func_80077D94 draws the hdr18..hdr24
 * sheets with; hdr18..hdr28, the five sheet lists func_80077D10 relocates;
 * win2C, in30 / out34 and the five portrait TIMs img38 func_80077D94 uploads. */
typedef struct {
    Unk8006E950Head unk_00;
    Unk8009B400Record *table;
    Unk8009B0E0Record **hdr18;
    Unk8009B0E0Record **hdr1C;
    Unk8009B0E0Record **hdr20;
    Unk8009B0E0Record **hdr24;
    Unk8009B0E0Record **hdr28;
    Win77D94 *win2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

/* Draw context filled by 5ED34 func_8006EACC and 64FD8 func_80077724 / func_8007855C: unk_00 the
 * resource root (SEL for func_8006EACC's handlers, D_SEL for func_80077724's; func_8007855C leaves it
 * unset), then the cursors; unk_24 (pool word +0x20) only func_8006EACC / func_80077724 set. */
typedef struct {
    union {
        Unk8006EA28Rec *v8006EA28;
        Unk80076FF8Rec *v80076FF8;
    } unk_00;
    Unk800788B0Rec unk_04;
    void *unk_24;
} Unk8006EACCRec;

/* The draw chunk 3AB48 func_8005C8A8 builds at the address its caller passes: TILEs from +0 up
 * to +0xF0 (15; it writes at most 13), SPRTs from +0xF0 up to +0x4D8 (50; func_8007352C's
 * cursor), then the DR_MODE at +0x4D8. It returns the chunk size, 0x4F0; the last 0xC bytes are
 * never written. */
typedef struct {
    TILE unk_00[15];
    SPRT unk_F0[50];
    DR_MODE unk_4D8;
    u8 unk_4E4[0xC];
} Unk8005C8A8Rec;

/* The draw chunk 3AB48 func_8005D814, func_8005E098 and func_8005F1C8 each build at the address
 * their caller passes (one layout): TILEs from +0 up to +0xA0 (10; they write at most 2, 2 and 4),
 * SPRTs from +0xA0 up to +0x2F8 (30; func_8007352C's cursor), then the DR_MODE at +0x2F8. Each
 * returns the chunk size, 0x304. func_8005F1C8 also writes a second DR_MODE at the chunk's end,
 * outside the chunk. */
typedef struct {
    TILE unk_00[10];
    SPRT unk_A0[30];
    DR_MODE unk_2F8;
} Unk8005D814Rec;

/* The draw chunk 3AB48 func_8005E54C builds at the address its caller passes: TILEs from +0 up
 * to +0xA0 (10; it writes 3), SPRTs from +0xA0 up to +0x898 (102; func_8007352C's cursor),
 * POLY_FT4s from +0x898 up to +0xBB8 (20; func_80073728's cursor), then the DR_MODE at +0xBB8.
 * It returns the chunk size, 0xBC4, and also writes a second DR_MODE at the chunk's end, outside
 * the chunk. */
typedef struct {
    TILE unk_00[10];
    SPRT unk_A0[102];
    POLY_FT4 unk_898[20];
    DR_MODE unk_BB8;
} Unk8005E54CRec;

/* The draw chunk 3AB48 func_8005FC9C builds at the address its caller passes: SPRTs from +0
 * (func_8007352C's cursor), the DR_MODE at +0x280, two POLY_G4 at +0x28C (at most one per pass)
 * and three DR_AREA at +0x2D4 (the first, then at most one per pass). func_8005FC9C returns the
 * chunk size, 0x2F8. The 32 SPRTs are the most it writes: func_8007352C writes at most one per
 * cell of the sheet header it is given (cell count: the byte at +2), and each of the two passes
 * draws D_8009B698's header and the one 12 bytes after it (8 cells each). */
typedef struct {
    SPRT unk_00[32];
    DR_MODE unk_280;
    POLY_G4 unk_28C[2];
    DR_AREA unk_2D4[3];
} Unk8005FC9CRec;

/* The draw chunk 3AB48 func_800600C8 builds at the address its caller passes: 0xB4 bytes of SPRTs
 * from +0 (func_8007352C's cursor), then the DR_MODE at +0xB4. func_800600C8 returns the chunk
 * size, 0xC0. At most 5 SPRTs are written: func_8007352C writes at most one per cell of the sheet
 * header it is given (cell count: the byte at +2); D_8009B6F0's header holds 3, and D_8009B6FC's
 * holds 1, drawn once per digit for up to two digits. */
typedef struct {
    SPRT unk_00[9];
    DR_MODE unk_B4;
} Unk800600C8Rec;

/* The draw chunk 3AB48 func_80060414 builds at the address its caller passes: one SPRT at +0
 * (func_8007352C's cursor; each of the three sheet headers it may pass, D_8009B7AC / D_8009B7B8 /
 * D_8009B7C4, has one cell: the count byte at +2 is 1), then the DR_MODE at +0x14. It returns
 * the chunk size, 0x2C; the last 0xC bytes are never written. */
typedef struct {
    SPRT unk_00;
    DR_MODE unk_14;
    u8 unk_20[0xC];
} Unk80060414Rec;

/* The draw chunk 3AB48 func_80060544 builds at the address its caller passes: SPRTs from +0 up
 * to +0x4EC (63; func_8007352C's cursor), POLY_FT4s from +0x4EC up to +0x5DC (6; func_80073728's
 * cursor), then the DR_MODE at +0x5DC. It returns the chunk size, 0x5F4; the last 0xC bytes are
 * never written. The walkers write at most one primitive per cell of the sheet header they are
 * given (cell count: the byte at +2): at most 19 SPRTs (D_8009B770[0..2] hold 5 / 4 / 4 cells,
 * D_8009B7A0 4, D_8009B398[2] / [3] 1 each) and 1 POLY_FT4 (D_8009B770[3] holds 1). */
typedef struct {
    SPRT unk_00[63];
    POLY_FT4 unk_4EC[6];
    DR_MODE unk_5DC;
    u8 unk_5E8[0xC];
} Unk80060544Rec;

/* 0x8009BD24: two players x five rounds of 2-byte records; byte 0 is the
   character the round was fought with (func_8005E54C reads it at
   j * 10 + i * 2 and picks UesrWorkDef / D_8009B58C by it; func_80060414 reads
   player 0 round 0). Unk8009BD24Block.unk00. */
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;

/* One of the three 4-byte records at Unk8009BD24Block.unk21 (func_8003C714 and func_80035280 store
   them; func_8006D808 reads them). */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Unk8009BD45Rec;

/* 0x8009BD24..0x8009BD57: the settings record func_80077D00 returns and 64FD8 hands to
   func_80068F70 / func_8006E534 / func_800770B8, whose callees reach every field off that one
   base (51268 D_800A3524, 5ED34 D_800A3568, SelWork.f00). Bit fields are named by bit offset.
   - unk14_*: the word at +0x14. unk14_0 (`& 0xF`; func_80077894 stores it), unk14_4 (6 bits),
     unk14_10 (the round count - 3; also picks the results-screen layout), unk14_12 (`== 2`
     tests), unk14_14, unk14_15 (one bit per player), unk14_17, unk14_18 (3 bits); no code
     reads bits 21-23.
   - unk17 / unk1A / unk1D: three byte triples. func_8006CCC8 rebuilds a player's nibble of unk17
     from unk1A or unk1D; 64FD8 copies unk17 and unk1D into each other.
   - unk20_*: the word at +0x20, bits 0-3; its upper bytes start the unk21 records. */
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

/* The select-screen work area D_800A36A0 points at (func_800770B8 places it at
 * the end of the buffers func_8006E49C lays out). Two-element arrays are per
 * player. Per player, f48 is a list of f60 entries with cursor f5C; confirming
 * moves the entry under the cursor to f7E[f3C++], cancelling moves f7E[--f3C]
 * back into f48 in sorted order; f6A holds the picked entries func_80076D74
 * reports. f00 is the result record func_80076D74 writes; f04 is the resource
 * table the select draw functions receive as arg0[0] (func_80077724 passes it as
 * the first word of that context). f10 and f14 are per-player s16 pairs that
 * the original code also reads as one word (union word views, owner rulings
 * Q33/Q46): `lw 0x10` at func_800747D8 0x800747F4 and func_80075670
 * 0x80075684, `lw 0x14` at func_80077374 0x800773A4; f1C and f20 likewise, cleared with one
 * `sw $zero` each by func_800770B8 (0x800772C0 / 0x800772BC), their halves read and written by
 * func_80075F80 (`lhu`/`sh` 0x1C / 0x20). The s32 members make the
 * struct 4-aligned, so sizeof is 0x94 (the members end at 0x92; owner ruling
 * Q57). */
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
 * row. Object model evidence from the original binary: func_8006F100 and func_80071C4C form ONE
 * offset per access (row*24 held in a strength-reduced register plus
 * D_800A3590[row]*4) and read `lh %lo(D_8009BC94)($at)` and
 * `lh %lo(D_8009BC96)($at)` through that same offset -- record stride 4,
 * row stride 24, base+offset addressing. Replaces the splat per-word scalars
 * D_8009BC94 / D_8009BC96 (per-word splat symbol -> aggregate merge family,
 * owner ruling). */
typedef struct {
    s16 x;
    s16 y;
} Unk8009BC94Record;

/* 0x8009B450: two {x, y} screen points (.short 0x01A2,0x0024,0x01F7,0x0037).
 * Object model evidence from the original binary: asm/funcs/func_8005D814.s
 * forms ONE index $s0 = j << 2 and reads both %lo(D_8009B450)($at) and
 * %lo(D_8009B452)($at) through it -- 4-byte records with halfword fields at
 * +0 and +2. func_8005D814 uses x as a TILE's x0 (and w = 0x238 - x) and y as
 * its y0 and the text row. Replaces the splat per-word scalars D_8009B450 /
 * D_8009B452 in C (per-word splat symbol -> aggregate merge family). */
typedef struct {
    s16 x;
    s16 y;
} Unk8009B450Record;

/* The 0x2C-byte draw descriptor the sprite walkers consume: func_8007352C (one SPRT per cell),
 * func_80073728 / func_80073C78 (one POLY_FT4 per cell, scaled / rotated). header / table: the sprite
 * sheet (Unk8009B0E0Record) and its cells; sprt_out / ft4_out: the SPRT and POLY_FT4 cursors the
 * walkers advance and return; semi: their SetSemiTrans argument; ot_idx: the ordering-table slot
 * (g_gpu_ot_ptr + ot_idx * 4); x / y: the screen offset added to every cell; scale_x / scale_y:
 * the 8.8 cell scales of the POLY_FT4 walkers; has_color / col_r / col_g / col_b: the SetShadeTex
 * switch and the primitive colour. The cursors are s32 because every source of them is an s32
 * word: the draw contexts' primitive cursors (Unk800788B0Rec, 51268's context words). */
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
} Unk8007352CEnv;

/* 0x8009B2BC: three {w, h} menu-frame sizes, one per mode (0x8009B2BC..
 * 0x8009B2C7; D_8009B2C8 follows, different data). Object model evidence from
 * the original binary: asm/funcs/func_8005C8A8.s forms ONE index `sll $a1,$t0,2`
 * (mode * 4) and reads both %lo(D_8009B2BC)($at) and %lo(D_8009B2BE)($at)
 * through it -- 4-byte records with halfword fields at +0 and +2 (the
 * D_8009B450 shape); the mode-2 arm reads record 2's w through the splat label
 * D_8009B2C4. Replaces the splat per-word labels D_8009B2BC / D_8009B2BE /
 * D_8009B2C4 in C (per-word splat symbol -> aggregate merge family). */
typedef struct {
    s16 w;
    s16 h;
} Unk8009B2BCRecord;

/* The stage data a stage loads (func_800469C4, or the buffer func_80054604 is handed; Unk800EFAE8Ctrl
   .unk2C): its header holds file-relative offsets of the parts func_80054604 / func_8005490C use. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10[2];
    s32 unk18[2];
} Unk800469C4Hdr;

/* Stage/match control block at 0x800EFAE8 (0x4C bytes). Object model evidence:
 * the original binary addresses the whole block through ONE base register -- asm/funcs/func_80054604.s
 * forms $s1 = %hi/%lo(D_800EFAE8) once in its prologue and reaches offsets
 * 0x00/0x02/0x04/0x08/0x0C/0x10/0x14/0x1C/0x1E/0x20/0x2C/0x44/0x46/0x48/0x4A as
 * displacements off that single register (`lw $v1, 0x2C($s1)`, `sh $s5, 0x44($s1)`,
 * ...), and the per-frame handler func_8005490C addresses the same block the
 * same way. The relocator func_80054FDC bumps the 0x2C..0x40 word group
 * together by one base offset. Base+offset addressing of one object, not
 * symbol adjacency. Replaces the splat per-word scalars D_800EFAE8 /
 * D_800EFB0C / D_800EFB14 / D_800EFB18 / D_800EFB1C / D_800EFB20 / D_800EFB24 /
 * D_800EFB28.
 * The per-player pairs are arrays: asm/funcs/func_8005490C.s walks
 * 0x34/0x38 with one pointer (`lw 0x34($s0)`, `$s0 += 4`, i < 2), 0x34/0x3C with
 * `$s4 += 4` and 0x44/0x48 with `$s2 += 2` in its i < 2 player loop; 0x24/0x26/0x28
 * are the halfword stores of the negated camera rotation. */
typedef struct {
    /* 0x00 */ s16 unk0;    /* phase (func_8005490C: -1 = done, 0 = init) */
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;    /* stage flags (bit31/bit30 tests, low 6 bits = cleanup index + 1) */
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
    /* 0x24 */ s16 unk24[4]; /* negated camera rotation vx/vy/vz; returned by address from func_8005507C */
    /* 0x2C */ s32 unk2C;   /* loaded data base (census g_snd_data_buf_base); relocated by func_80054FDC */
    /* 0x30 */ s32 unk30;   /* relocated by func_80054FDC */
    /* 0x34 */ s32 unk34[2]; /* per player; relocated by func_80054FDC when nonzero */
    /* 0x3C */ s32 unk3C[2]; /* per player; relocated by func_80054FDC when nonzero */
    /* 0x44 */ s16 unk44[2]; /* per player */
    /* 0x48 */ s16 unk48[2]; /* per player */
} Unk800EFAE8Ctrl;

/* 0x800A3560: two 3-byte records, one per selection slot i (slot i at
 * 0x800A3560 + i * 3; 0x800A3566/7 pad before D_800A3568). Object model evidence
 * from the original binary: func_8006F100 reads
 * bytes +2 and +1 through ONE offset register stepped by 3 per slot
 * (asm/funcs/func_8006F100.s:73-75 `lbu %lo(D_800A3562)($at)` and :100-102
 * `lbu %lo(D_800A3561)($at)` with $s3, `addiu $s3,$s3,0x3` at :267);
 * func_80070C70 walks byte +0 the same way ($s2, `addiu $s2,$s2,0x3`,
 * func_80070C70.s:114-116, :166); func_80070188 forms i*3 once and reaches
 * +0, +1 and +2 through it (func_80070188.s:58-76, 386-419); func_8006E534 stores
 * byte +1 of both records (func_8006E534.s:96-97, D_800A3561 / D_800A3564).
 * Replaces the splat per-byte symbols D_800A3560..D_800A3565 in C (per-word splat
 * symbol -> aggregate merge family, owner ruling); their
 * undefined_syms_auto.txt rows are retired (no assembled referrer is left).
 * `word`: Q33 union word view (owner ruling), named only at
 * func_8006E534's one word store over bytes 0..3, `sw $v0,%gp_rel(D_800A3560)($gp)`
 * with $v0 = -1 (func_8006E534.s:85, 0x8006E668). Every other access goes
 * through rec[]. Owner rulings Q44/Q54: every consumer is in the -G8 file
 * src/main/5ED34.c. */
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
 * records of 3 s32 words. Object model evidence (the original binary):
 * asm/funcs/func_800678A8.s addresses it as base + arg1*36 + idx*12 (+0/+4/+8),
 * i.e. a row stride of 36 bytes and a record stride of 12; arg1 ranges 0..3
 * (callers func_800676C8..func_8006786C). func_80067200 and
 * func_80067D14 reach the same words. Replaces the splat per-word scalars
 * D_800F0C10 / D_800F0C14 / D_800F0C18. */
typedef struct {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} Unk800F0C10Record;

/* 0x800948BC: per-stage function pairs, indexed by stage_GetId().
 * stage_ExecInitFunc calls .init; func_8003E6D8 calls .unk4 (e.g. entry 13
 * holds camera_InitBone2 / func_800475A4). */
typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} StageFuncEntry;

/* menuDat: model id -> BBM file name, ended by a zero id (0x8008DCCC..0x8008DD5B,
 * asm/data/7D920.data.s dlabel menuDat). func_80020E74 loads the model of entry n
 * from CD file n + 2. */
typedef struct {
    s32 id;
    char *name;
} MenuDatEntry;

/* Per-character record pointed to by D_800A3860[ch] (ch = rec+0x4A). f14 is
 * the modulus func_800213A0 / func_80021424 wrap rec+0x86 with. The u16
 * fields at +0x4E are indices into D_801027B0[ch][0], as func_80021424 reads
 * them: f4E[rec+0x84] (id 0x7FF0) / f4E[rec+0x86] (ids 0x7FF1/2/4),
 * f54[rec+0x86][t] (id 0x7FF3), and
 * f66[id - 0x7FF5][rec+0x86] (ids 0x7FF5..0x7FFF). f16 (func_800219E4) and
 * f18[class] (func_80021A3C; class = Unk80101EC8Record.unk_0A, 0..26 as D_8008D538
 * holds and as the [27][6] class tables D_8008DE34 / D_8008DF78 are sized; 27
 * halfwords end exactly at f4E) are indices into D_80102760; both readers load
 * them with lhu. */
typedef struct {
    u8 pad00[3];
    u8 unk_03;                     /* D_801027B0[ch][0] = record + 0x6C + (unk_03 - 1) * 6 (func_80020E74) */
    s32 unk_04[4];                 /* D_801027B0[ch][1 + k] = record + unk_04[k] (func_80020E74) */
    s16 f14;
    u16 f16;
    u16 f18[27];
    u16 f4E[3];
    u16 f54[3][3];
    u16 f66[11][3];
} Tbl800A3860Entry;

/* s32 x/y/z triple.  Unk80101EC8Record's position-like triples are copied as
 * whole 12-byte objects (func_80022580), and func_80021DB0 writes one through
 * its out parameter. */
typedef struct { s32 x, y, z; } Vec3i32;

/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array").  The same
   s32 x/y/z triple as Vec3i32: func_800207C8 copies a scratchpad point
   (SPAD->unkA8) into Unk80101EC8Record.unk_180 as one 12-byte object. */
typedef Vec3i32 LeafPos;

/* Vec4i32 / SVec4i16 are the remaining local-name copies of the PsyQ VECTOR / SVECTOR
 * layouts (include/psxsdk/libgte.h); retyping them as the Sony types is Phase 2 work.  func_80022580 copies
 * Unk80101EC8Record's +0xB8 and +0x104 as whole 16-byte VECTORs (pad included)
 * and +0x1C8 as a whole 8-byte SVECTOR. */
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;

typedef struct { s16 vx, vy, vz, pad; } SVec4i16;

/* The 10-entry block table over the 0x45000-byte buffer at D_800A9D10
 * (main/35000.c func_800451D0 .. func_8004574C; D_800A33AC live entries).
 * func_800451D0 clears id in all 10 (offset 0x90 down to 0 in steps of 0x10,
 * asm/funcs/func_800451D0.s); the walkers index base + i*16.  func_80045294(a0,
 * a1) hands func_800520B8 entry a0's unk4, unk4 + a1 and the summed amt of
 * entries a0.., then adds a1 to their unk4 and calls each fn(id, a1).  Replaces the splat per-word scalars D_800EED14 /
 * D_800EED18 / D_800EED1C and D_800EED00 (entry -1, i.e. D_800EED10[j - 1]). */
typedef struct {
    s16 id;
    s16 unk2;
    s32 unk4;
    s32 amt;
    void (*fn)(s16, s32);
} Unk800EED10Entry;

/* A hit record: one of the 22 test points of a character.  func_800207C8 places
 * point i at SPAD->unkA8[ch][i] (`ofs` rotated by game_GetPlayerData()'s matrix
 * `bone`, plus that matrix's translation); the hit tests (func_8002A458,
 * func_8002CA8C, func_80031B24) take unk_0C / unk_0E as the first test's limits
 * and, when unk_00 != 0, unk_10 / unk_12 as a second test's. */
typedef struct BoneHitRec {
    s16 unk_00;
    s16 bone;
    SVec4i16 ofs;
    u16 unk_0C;
    u16 unk_0E;
    u16 unk_10;
    u16 unk_12;
} BoneHitRec;                      /* sizeof == 0x14 */

/* Pad input record (0x18 bytes) at 0x80102788.  func_80019568 fills it each
 * frame from the two pads (one u16 half per player in each word):
 * held = current bits, pressed = held & ~previous, released = ~held & previous,
 * unheld = ~held; pad_ResetState sets type[] and the four bit words to
 * (4, 4, 0, 0, 0, -1) and leaves valid[] alone.  func_8001BE20
 * copies the whole record to its caller's buffer, and pad_ClearStateBits clears the
 * four bit words of such a buffer. */
typedef struct PadState {
    s16 type[2];                   /* per pad: InitPAD buffer byte 1 >> 4, 5 and 7 folded
                                      to 4, 4 when the status byte != 0 (func_80019568,
                                      where func_8003A728 can first replace them with
                                      nibbles from the link-cable exchange words);
                                      4 from pad_ResetState (both) and func_80055B60
                                      (entry arg0 of its own record) */
    s16 valid[2];                  /* per pad: 1 iff the InitPAD buffer status byte == 0
                                      (func_80019568); 1, 1 from pad_ResetStateMarkValid */
    u32 held;                      /* 0x08 */
    u32 pressed;                   /* 0x0C */
    u32 released;                  /* 0x10 */
    u32 unheld;                    /* 0x14 */
} PadState;                        /* sizeof == 0x18 */

/* The 24-entry pending-sound pool (main/3AB48.c): func_8005C650 queues a
 * request (an entry of D_8009AA70) with its volumes in the first free entry;
 * func_8005C6D0 keys each queued note on via SsUtKeyOnV(.., voll, volr) and
 * clears the entry; snd_Init / func_8005B5AC reset all 24.  Every user indexes
 * base + i*8 (asm/funcs/func_8005C650.s, snd_Init.s: 0xC0 / 8 entries).
 * Replaces the splat per-word scalars D_800EFB7C / D_800EFB7D. */
typedef struct {
    u16 *req;
    u8 volr;
    u8 voll;
} Unk800EFB78Entry;

/* One 8-byte texture record: the CLUT position (PsyQ getClut(x, y) = (y << 6) | ((x >> 4) & 0x3F))
 * and the texture u/v origin. 51268's D_800A3488 / D_800A348C point at one; the D_8009B890 ..
 * D_8009BA58 tables (asm/data/7D920.data.s) are runs of them. */
typedef struct {
    u16 clut_x;
    u16 clut_y;
    u16 u;
    u16 v;
} TexRec;

/* The 0x2C-byte block D_800A3468 points at (51268.c): Unk1F800000Rec.unk00 (func_80060E38's seed)
 * or D_800F116C, where func_800611A4 .. func_80061EC0 point it before calling func_80060A68.
 * D_800F1198 follows D_800F116C, so that copy ends at +0x2C. Each copy sets one pointer pair:
 * - unk00: one word, stored whole and read whole (bit 21 in func_80060A68, bits 17-18 / 19-20 in
 *   func_80063AF0 / func_80063B34 / func_80065000); func_80060A68 / func_80060B70 also read its
 *   low halfword (`lhu`), their D_800F10D0 / D_8009BA60 index.
 * - unk04 / unk08: the three halfwords / three words func_80060B70 and func_800620B8 copy into the
 *   scratchpad block's unk18 / unk20 (through D_800A346C / D_800A3470). Only the scratchpad block's
 *   are set: func_80060E38 stores its two arguments there.
 * - unk0C / unk10: the three words / three halfwords func_80060A68 copies into unk20 / unk18. Set
 *   only in the D_800F116C block (through D_800A3468, or D_800F1178 / D_800F117C, separate symbols
 *   at its +0x0C / +0x10).
 * - unk14: the byte func_80060A68 / func_80060B70 store the called function's result to (`sb`);
 *   D_800F1180 is the D_800F116C block's +0x14.
 * - unk18 / unk20: the copies. D_800A346C / D_800A3470 point at the scratchpad block's,
 *   D_800A3478 / D_800A347C at the D_800F116C block's (func_80060A68). unk18 is s16 like
 *   its sources (unk04 / unk10) and every consumer (SVECTOR / SVec4i16 fields: func_80061FAC,
 *   func_8006288C, func_80063BD0, func_80067200).
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

/* 20-byte record table at 0x800EFC78 (51268.c): 4 rows (arg1) of 48 records. Object model evidence:
 * asm/funcs/func_80067200.s addresses it as base + arg1*0x3C0 + i*20 with halfword stores at
 * +0..+0xC, +0x10, +0x12 (+0xE untouched there); 0x3C0 / 20 = 48 = the loop's record count. */
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

/* func_800620B8's layout of the 51268 work area (Unk1F8000B8Union.v800620B8). func_80061FAC, which
 * func_800620B8 calls before it touches the area, writes and consumes its SVECTOR at +0x00..+0x07.
 * SetTransMatrix is handed the address 0x14 below unk14, so unk14 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                /* no access here */
    s16 unk10;                     /* w */
    s16 unk12;                     /* h */
    VECTOR unk14;                  /* ApplyRotMatrixLV out */
    VECTOR unk24;                  /* ApplyRotMatrixLV in */
    SVECTOR unk34;                 /* RotTransPers in */
    s32 unk3C;                     /* RotTransPers' p */
    u8 unk40[4];                   /* no access */
    u32 unk44;                     /* func_80052C28's depth, the OT index */
} Unk1F8000B8_800620B8;

/* func_80063084's layout (Unk1F8000B8Union.v80063084). SetTransMatrix is handed the address 0x14
 * below unk14 (the area's base), so unk14 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x14];                /* no access */
    VECTOR unk14;                  /* ApplyRotMatrix out */
    SVECTOR unk24;                 /* ApplyRotMatrix in */
    SVECTOR unk2C;                 /* RotTransPers in */
    s32 unk34;                     /* RotTransPers' p */
    s32 unk38;                     /* fade */
    s32 unk3C;                     /* func_80052C28's depth, the OT index */
} Unk1F8000B8_80063084;

/* func_800646E8's layout (Unk1F8000B8Union.v800646E8). SetTransMatrix is handed the address 0x14
 * below unk18, so unk18 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                /* no access */
    s16 unk10;                     /* w */
    s16 unk12;                     /* h */
    s32 unk14;                     /* frame */
    VECTOR unk18;                  /* ApplyRotMatrixLV out */
    VECTOR unk28;                  /* ApplyRotMatrixLV in */
    SVECTOR unk38;                 /* RotTransPers in */
    s32 unk40;                     /* RotTransPers' p */
    POLY_FT4 *unk44;               /* the end of the quads */
    u32 unk48[16];                 /* per kept quad, its OT index */
} Unk1F8000B8_800646E8;

/* func_80065800's layout of the 51268 work area (Unk1F8000B8Union.v80065800). Its gte_SetTransMatrix
 * operand hands the GTE the area's base as a MATRIX, whose t[] is unk14[0]. */
typedef struct {
    u8 unk00[0x10];                /* no access */
    s32 unk10;                     /* gte_stdp out; not read */
    VECTOR unk14[4];               /* ApplyRotMatrixLV out ([0]); the corner loop's ApplyRotMatrix outs */
    VECTOR unk54;                  /* ApplyRotMatrixLV in */
    SVECTOR unk64;                 /* gte_ldv0 / RotMatrix / ApplyRotMatrix in */
    s16 unk6C;                     /* p_w */
    u8 unk6E[2];                   /* no access */
    s16 unk70;                     /* p_h */
    u8 unk72[2];                   /* no access */
    MATRIX unk74;                  /* RotMatrix out, SetRotMatrix in */
    s16 unk94;                     /* added to the texture u */
    s16 unk96;                     /* added to the texture v */
} Unk1F8000B8_80065800;

/* The layout func_800678A8 / func_80067D14 / func_80068D88 share (Unk1F8000B8Union.v800678A8): each of
 * the wrappers func_800676C8 .. func_8006786C calls the three in turn, and values cross the calls
 * (unk04, unk6C, unk80, the unk8C table; func_800678A8 and func_80067D14 each use unk70 for their own
 * value). func_800678A8 sets it up, func_80067D14 builds the quads at unk80 and records each one's
 * depth (its OT slot index) in unk8C, func_80068D88 links them. */
typedef struct {
    u16 unk00;                     /* func_800678A8: added to the texture u */
    u16 unk02;                     /* func_800678A8: added to the texture v */
    u32 unk04;                     /* func_800678A8 stores 0x895440; func_80067D14 compares against it */
    u8 unk08[0x1C];                /* no access */
    s32 unk24[3];                  /* func_80067D14: gte_stlvnl out */
    u8 unk30[4];                   /* no access */
    VECTOR unk34;                  /* func_80067D14: gte_ldlvl in */
    SVECTOR unk44[3];              /* func_80067D14: gte_ldv0 / gte_ldv3 in */
    VECTOR unk5C;                  /* func_80067D14 */
    s16 unk6C;                     /* func_800678A8 stores it; func_80067D14's loop bound */
    s16 unk6E;                     /* func_80067D14 / func_80068D88 loop index */
    s16 unk70;                     /* func_800678A8: table index; func_80067D14: per entry */
    u8 unk72;                      /* func_80067D14 colours */
    u8 unk73;
    u8 unk74;
    u8 unk75[3];                   /* no access */
    s16 unk78;                     /* func_80067D14 */
    u8 unk7A[2];                   /* no access */
    POLY_FT4 *unk7C;               /* func_80068D88: the end of the quads */
    POLY_FT4 *unk80;               /* the quad cursor */
    Unk800EFC78Record *unk84;      /* func_80067D14 */
    Unk800F0C10Record *unk88;      /* func_80067D14 */
    u16 unk8C[0x90];               /* per quad, its depth, the OT slot index (func_80067D14 stores,
                                      func_80068D88 reads) */
    s32 unk1AC;                    /* func_80067D14 stores rand() values here; D_800A3724 points at it */
} Unk1F8000B8_800678A8;

/* Unk1F800000Rec.unkB8, the 51268 work area D_800A34EC points at (0x1F8000B8 to the end of the
 * scratchpad). Each user lays it out for one call (or, for the func_800678A8 trio, one wrapper call);
 * raw sizes the union. func_80061FAC's is one SVECTOR. func_8006295C and func_80063E10 still convert
 * D_800A34EC (their work-area base is staged through a POLY_FT4 * local). */
typedef union {
    u8 raw[0x400 - 0xB8];
    SVECTOR v80061FAC;
    Unk1F8000B8_800620B8 v800620B8;
    Unk1F8000B8_80063084 v80063084;
    Unk1F8000B8_800646E8 v800646E8;
    Unk1F8000B8_80065800 v80065800;
    Unk1F8000B8_800678A8 v800678A8;
} Unk1F8000B8Union;

/* 51268's view of the scratchpad from 0x1F800000 (SPAD51268 in 51268.c). 17AFC's view of the same
 * memory, used at other times, is ScrPad. func_80060E38 points the file's D_800A34xx globals at these
 * members (51268.c); apart from its own two stores to unk00.unk04 / unk08, the code reaches them only
 * through those globals:
 * - unk00: the command block D_800A3468 points at until a function retargets it; D_800A346C /
 *   D_800A3470 point at its unk18 / unk20.
 * - unk2C: no access.
 * - unk50 / unkB0: the initial targets of D_800A3488 / D_800A348C (TexRec). func_800620B8 retargets
 *   them only in its switch cases 0-3 (unk4 & 7), so its reads at 51268.c:958-960 go through these
 *   seeded values when no earlier record took one of those cases.
 * - unkA0 / unkA4: the initial targets of D_800A34E4 (u32 *) / D_800A34E8 (u32 *), which every
 *   user retargets before use.
 * - unkB8: the work area D_800A34EC points at (Unk1F8000B8Union), laid out differently by its
 *   users; it runs to the end of the scratchpad.
 * The other members are one global each (D_800A3474 .. D_800A34E0, D_800A3480 / D_800A3484). */
typedef struct {
    Unk1F800000Unk00 unk00;
    u8 unk2C[4];
    MATRIX unk30;      /* D_800A3474 */
    TexRec unk50;
    s32 unk58;         /* D_800A3490 */
    s32 unk5C;         /* D_800A3494 */
    u16 unk60;         /* D_800A3498 */
    u16 unk62;         /* D_800A349C */
    u16 unk64;         /* D_800A34A0 */
    u16 unk66;         /* D_800A34A4 */
    s16 unk68;         /* D_800A34A8 */
    s16 unk6A;         /* D_800A34AC */
    s32 unk6C;         /* D_800A34B0 */
    s32 unk70;         /* D_800A34B4 */
    s32 unk74[3];      /* D_800A34B8 */
    s16 unk80;         /* D_800A34BC */
    s16 unk82;         /* D_800A34C0 */
    s32 unk84;         /* D_800A34C4 */
    s32 unk88;         /* D_800A34C8 */
    s32 unk8C;         /* D_800A34CC */
    s32 unk90[2];      /* D_800A34D0 */
    u16 unk98;         /* D_800A34D4 */
    u16 unk9A;         /* D_800A34D8 */
    u16 unk9C;         /* D_800A34DC */
    u16 unk9E;         /* D_800A34E0 */
    u32 unkA0;
    u32 unkA4;
    s32 unkA8;         /* D_800A3480 */
    s32 unkAC;         /* D_800A3484 */
    TexRec unkB0;
    Unk1F8000B8Union unkB8;
} Unk1F800000Rec;

/* A cell (x, z) of the 32x32 grid of 2000-unit cells that 3AB48's func_80052D00 walks
 * (Work_80053E9C.unk88 / unk8C). */
typedef struct {
    s16 x;
    s16 z;
} Cell_80052D00;

/* The 0xEC-byte work area of 3AB48's stage-collision cast (func_80052D00 and its helpers, through
 * D_800A33F4). func_8005344C / func_80053614 place it at their last
 * argument; func_80053304 / func_80053584 at D_800EF9F8. 17AFC func_80030D7C / func_800321E8 pass
 * 0x1F8002F0, its place in their scratchpad layout (Unk1F8002B8_8005344C.unk38). */
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

/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch. unk00 is up to six points that unk60[0..2] aim at.
 * func_8002A458 and func_80031B24 use [0] / [1], for func_8002E838 / func_8002EA24:
 * func_8002A458's segment, base then tip (func_8002AB08 writes them before each call), and
 * func_80031B24's D_80106A78 object step, prev_pos then pos. func_8002AB08 uses all six: [0] / [1]
 * are two of the other character's scratchpad points (ScrPad.unk00 or unk48), [2] / [3] the same
 * two from its record (unk_210 or unk_234), [4] / [5] the midpoints of [0] / [2] and [1] / [3];
 * for each triangle it hands func_8002CD58 it aims unk60[0..2] at three of them.
 * func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument block, its
 * work area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of
 * Unk1F8002B8Union. raw sizes the union to 0x60. */
typedef union {
    u8 raw[0x60];
    LeafPos unk00[6];
} Unk1F8002B8Unk00;

/* 17AFC's view of the last 0x148 bytes of the scratchpad, 0x1F8002B8..0x1F8003FF
 * (ScrPad.unk2B8.rec).
 * The scratchpad is shared scratch: other code puts its own data in these bytes at other times,
 * with its own views. Among others:
 * - 3AB48 func_80053614 / func_8005344C keep their work area (Work_80053E9C, 0xEC bytes) at the
 *   address of their last argument (D_800A33F4). 3AB48 func_80056CB8 / func_800571C0 and 9F9C
 *   func_80021DB0 / func_8002304C / func_800233AC / func_800238C4 / func_80023D28 /
 *   func_80023DB8 / func_80023E40 pass 0x1F8002B8 (the area spans record +0x00..+0xEB);
 *   9F9C func_800207C8 and 17AFC func_80030D7C / func_800321E8 pass 0x1F8002F0 (+0x38..+0x123,
 *   over unk60 up into unk118).
 * - 9F9C func_800207C8 keeps a whole probe record at 0x1F8002B8: from, to, hit, normal, then
 *   that work area.
 * - 32D04 func_800430E4 keeps a MATRIX at 0x1F8003A0.
 * - 3AB48 func_8004DA74 fills, and 30 renderers in func_8004C994 .. func_80051ED4 read, an s16
 *   per-vertex table at 0x1F8002B4 + 2 * i (all INCLUDE_ASM), which runs into the record for
 *   i >= 2.
 * In 17AFC, func_8002A458 / func_8002AB08 / func_8002CA8C / func_80029454 / func_80031B24 take its
 * address (`scr`) and pass it to func_8002E838 / func_8002EA24 / func_8002D320 /
 * func_8002D780 / func_8002CD58 / func_8002DAD0 / func_8002DE20 / func_80031890; func_800290B8 /
 * func_8002C22C / func_8002C61C / func_8002EBDC / func_8002F2D0 / func_8002F770 / func_8002FC80 /
 * func_8002FDB0 address it directly (func_80030D7C / func_800321E8 use Unk1F8002B8_8005344C,
 * Unk1F8002B8Union). unk60 / unk6C hold point pointers (func_80029454 reads unk60[0..2]); unkB4 /
 * unkC4 are func_8002CA8C's two hit masks (func_8002AB08 reads them; no 17AFC code accesses unkD4
 * as a member); unkD8 is the matrix the RotMatrix* calls build, and the GTE rotates unkA8 through
 * it; func_8002DE20 rotates three points, relative to the origin *unk60[0], into unk118 and tests
 * them against the triangle (0,0) / unkA8 / unkB8. unk00 is scratch (Unk1F8002B8Unk00):
 * func_8002A458 / func_80031B24 / func_8002AB08 use its points. */
typedef struct {
    Unk1F8002B8Unk00 unk00;
    LeafPos *unk60[3];
    LeafPos *unk6C[3];
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

/* The layout func_80030D7C and func_800321E8 give the scratchpad area at 0x1F8002B8
 * (Unk1F8002B8Union.v8005344C): the to point, hit point and normal they pass func_8005344C, and
 * its work area, which runs over Unk1F8002B8Rec's unk60..unk118. */
typedef struct {
    Vec3i32 unk00;                 /* to; func_8005344C copies 16 bytes from it */
    s32 unk0C;                     /* read only as the 4th word of that copy */
    Vec3i32 unk10;                 /* the hit point; func_80030D7C's turn block first keeps a
                                      rotated velocity here */
    s32 unk1C;                     /* no access */
    Vec3i32 unk20;                 /* func_80030D7C's turn block's second rotated vector
                                      (func_800321E8 does not access it) */
    s32 unk2C;                     /* no access */
    s16 unk30[3];                  /* the hit normal */
    s16 unk36;                     /* no access */
    Work_80053E9C unk38;           /* func_8005344C's work area, +0x38..+0x123 */
} Unk1F8002B8_8005344C;

/* ScrPad.unk2B8: the scratchpad's last 0x148 bytes, 0x1F8002B8..0x1F8003FF, which 17AFC uses with
 * two layouts. Its collision code uses rec (Unk1F8002B8Rec). func_80030D7C and func_800321E8 use
 * v8005344C: they pass 0x1F8002F0 as func_8005344C's work-area address, so its 0xEC-byte
 * Work_80053E9C sits at +0x38..+0x123, over rec's unk60..unk118. */
typedef union {
    Unk1F8002B8Rec rec;
    Unk1F8002B8_8005344C v8005344C;
} Unk1F8002B8Union;

/* Scratchpad point tables at 0x1F800000.  unk00: three points per character
 * (func_8002C61C copies [0][0..2] and [1][0..2] to the two records' +0x210);
 * unk48: two more per character (copied to +0x234); unk78: two body points
 * per character (func_800288C8 builds them from unkA8 joint 1 and the
 * midpoint of joints 15 and 19); unkA8: 22 points per
 * character (func_8002A458 reads 0x1F8000A8 + id * 0x108 + i * 0xC, i < 22).
 * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48.  unk2B8: the area at
 * 0x1F8002B8 (Unk1F8002B8Union), to the end of the scratchpad; the scratchpad is shared scratch
 * that other code also uses with its own views (see Unk1F8002B8Rec). */
typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    LeafPos unk78[2][2];
    LeafPos unkA8[2][22];
    Unk1F8002B8Union unk2B8;
} ScrPad;

#define SPAD ((ScrPad *)0x1F800000)

/* One CPU path waypoint (CpuRoute.node[]): func_800571C0 writes x/z from the
 * opponent's position and kind 2; func_80058580 walks them back to front. */
typedef struct CpuWaypoint {
    s16 x;
    s16 z;
    u8 kind;
    u8 unk5;
} CpuWaypoint;

/* A CPU route: the polygon and vertex the walker stands at (func_80057ACC), the waypoint
 * count and the waypoints. Unk80101EC8Record carries one at +0x360; func_80057E84 builds two
 * candidates of the same layout on its stack and appends the cheaper one. */
typedef struct CpuRoute {
    u8 poly;                       /* index into the stage's NavPolySet.polys */
    u8 vtx;                        /* vertex index in that polygon */
    u8 count;                      /* number of waypoints in node[] */
    CpuWaypoint node[8];
} CpuRoute;                        /* sizeof == 0x34 */

/* One polygon of a stage's navigation set (8 bytes): flags (0x80 = open chain, the last vertex
 * does not close back to the first), a kind byte copied into the route waypoints built around
 * it, a corner margin (func_80057CC8 scales it by 40), the vertex count and the vertex table of
 * x/z pairs. */
typedef struct NavPoly {
    u8 flags;
    u8 kind;
    u8 margin;
    u8 nvtx;
    s16 (*vtx)[2];
} NavPoly;

/* A stage's navigation set: a D_8009A658 row (12 bytes: count word, polygon array, zero word). */
typedef struct NavPolySet {
    u8 npolys;
    u8 unk1[3];
    NavPoly *polys;
    u8 unk8[4];
} NavPolySet;

/* One decoded motion frame (0x84 bytes, 33 words): func_800198D0 decodes
 * frame `frame` of a character's motion into one, and func_80023F08 keeps the
 * current and next frames of its record (Unk80101EC8Record.unk_290 holds the
 * frame it last used).  Only 16-bit channels, so a whole-frame assignment is
 * the halfword-aligned block copy func_80023F08 shows.  unk_00 is the root
 * offset it negates, unk_02 / unk_04 the root heading and distance, unk_06..
 * unk_0A the three rotation angles it hands math_RotMatrixZYXAngles.  unk_0C[]
 * holds signed channels: func_8002F770 reads the triplet at +0x36 with lh, and
 * func_800198D0 sign-extends the 12-bit values at +0x6C..+0x70 / +0x78..+0x7C. */
typedef struct MotionFrame {
    s16 unk_00;
    u16 unk_02;
    u16 unk_04;
    u16 unk_06;
    u16 unk_08;
    u16 unk_0A;
    s16 unk_0C[0x3C];
} MotionFrame;                     /* sizeof == 0x84 */

/* Header of one move record of a character's move script (the u16 stream
 * func_80021424 returns pointers into; Unk80101EC8Record.unk_50 is the current
 * move, unk_7C a buffered one).  unk_00 / unk_02 are follow-up move ids,
 * unk_07 / unk_08 frame bounds against Unk80101EC8Record.unk_40, unk_09 flag
 * bits (func_80023F08, func_80055B60, func_80058580); unk_0A starts the
 * command list func_80023F08 walks: (flags, move id[, mask lo, mask hi])
 * entries of 2 or 4 halfwords, ended by a zero flags word. */
typedef struct MoveScript {
    u16 unk_00;
    u16 unk_02;
    u16 unk_04;                    /* row of the bank's 4-byte entry table (func_80021A98) */
    u8  unk_06;                    /* first frame: Unk80101EC8Record.unk_40 starts here (func_80021A98) */
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;
    u16 unk_0A[1];
} MoveScript;

/* Per-character record table (base 0x80101EC8, stride 0x44C, 2 records); the
 * old "practice menu" name was RESET by owner ruling Q103.  Schema:
 * docs/naming/CHAR_STRUCT_SCHEMA.md; base symbol: named_syms.txt (D_80101EC8).  Only the fields reached by C so
 * far are named; the rest is reserved padding.  func_80022580 initializes
 * record [idx] (every field it writes is declared at its offset). */
typedef struct Unk80101EC8Record {
    struct Unk80101EC8Record *other; /* the other record: [1] for record 0, else [0] (func_80022580) */
    s16 index;                     /* this record's own index (func_80022580) */
    s16 unk_06;                    /* != 0: func_8001BE20 hands pad input to func_80055B60 */
    s16 unk_08;
    s16 unk_0A;                    /* class idx: row of D_8008E5CC / D_8008E6A4, index of D_8008D9EC */
    s16 unk_0C;
    s16 unk_0E;                    /* column of D_8008E5CC / D_8008E6A4 */
    u8  unk_10[0x12 - 0x10];
    s16 unk_12;
    s16 unk_14;                    /* -1 == none, else index of D_8008EB80 */
    u8  unk_16[0x1A - 0x16];
    s16 unk_1A;
    s16 unk_1C;
    s16 unk_1E;
    s16 unk_20;
    u8  unk_22[0x24 - 0x22];
    PadState unk_24;               /* this frame's pad input (func_80023F08 copies it whole) */
    s32 unk_3C;                    /* frames since the record started (func_80023F08) */
    s16 unk_40;                    /* current frame of the move */
    s16 unk_42;                    /* frame fraction, 0x1000 = one frame */
    s16 unk_44;                    /* frame-fraction step */
    s16 unk_46;
    u16 unk_48;                    /* model id (func_80020E74); func_80021280 finds it in D_800A38C4 */
    s16 unk_4A;
    s16 unk_4C;
    u8  unk_4E[0x50 - 0x4E];
    MoveScript *unk_50;            /* current move; unk_08 bounds unk_40 (func_80058580) */
    u16 *unk_54;
    u8 *unk_58;                    /* byte 3 read by func_80056FE8 */
    u16 unk_5C;
    s16 unk_5E;                    /* 0/1, set alongside func_80021A98 */
    u8  unk_60;
    u8  unk_61;
    u8  unk_62;
    u8  unk_63;
    u16 unk_64;
    u16 unk_66;
    s16 unk_68;
    u16 unk_6A;                    /* SEQ state code; CHAR_STRUCT_SCHEMA.md +0x06A */
    s16 unk_6C;
    s16 unk_6E;
    s16 unk_70;
    s16 unk_72;
    s32 unk_74;
    s16 unk_78;
    s16 unk_7A;
    MoveScript *unk_7C;            /* buffered next move (func_80023F08) */
    s16 unk_80;
    u16 unk_82;
    s16 unk_84;
    s16 unk_86;                    /* row of unk_3A8 / unk_3F8 / unk_3FE / unk_404 */
    s16 unk_88;
    s16 unk_8A;
    s16 unk_8C;                    /* != 0: the second blade (unk_234 / SPAD unk48 points) is live */
    s16 unk_8E;
    s16 unk_90;
    s16 unk_92;
    s16 unk_94;
    s16 unk_96;
    SVec4i16 unk_98;
    u8  unk_A0;
    u8  unk_A1[2];
    u8  unk_A3[2];
    u8  unk_A5;
    u8  unk_A6;
    u8  unk_A7;
    u8  unk_A8;
    u8  unk_A9;
    u8  unk_AA;
    u8  unk_AB;
    u8  unk_AC;
    u8  unk_AD;                    /* != 0: func_8002C61C re-runs func_800283D0 for both records */
    u8  unk_AE;
    u8  unk_AF;
    u8  unk_B0;
    u8  unk_B1;
    u8  unk_B2;
    u8  unk_B3;
    u8  unk_B4;
    u8  unk_B5[0xB8 - 0xB5];
    Vec4i32 unk_B8;
    Vec4i32 unk_C8;
    Vec3i32 unk_D8;
    u8  unk_E4[0xE8 - 0xE4];
    Vec3i32 unk_E8;
    Vec3i32 unk_F4;
    u8  unk_100[0x104 - 0x100];
    Vec4i32 unk_104;
    Vec4i32 unk_114[2];            /* per blade; func_8002AB08 indexes it with its 0/1 blade flag */
    Vec4i32 unk_134;
    s32 unk_144;
    s32 unk_148;
    s16 unk_14C;
    s16 unk_14E;
    s16 unk_150;
    s16 unk_152;
    s16 unk_154;
    s16 unk_156;
    s16 unk_158;
    s16 unk_15A;
    u8  unk_15C[0x15E - 0x15C];
    s16 unk_15E;
    s16 unk_160;
    s16 unk_162;
    u8  unk_164[0x168 - 0x164];
    Vec3i32 unk_168;
    Vec3i32 unk_174;
    Vec3i32 unk_180;
    Vec3i32 unk_18C;
    Vec3i32 unk_198[2];            /* func_800207C8: translations of bones 17 / 14, y + ((unk_1A * 71) >> 11) */
    s32 unk_1B0[2];                /* func_800207C8: floor y under unk_198[i] (func_80053614 probe) */
    u8  unk_1B8[0x1BA - 0x1B8];
    s16 unk_1BA;                   /* func_800207C8: ratan2 heading of bone 17's matrix column 2, + 0x800 */
    u8  unk_1BC[0x1C2 - 0x1BC];
    s16 unk_1C2;                   /* the same for bone 14 */
    u8  unk_1C4[0x1C8 - 0x1C4];
    SVec4i16 unk_1C8;
    SVec4i16 unk_1D0;
    s16 unk_1D8;
    s16 unk_1DA;
    s16 unk_1DC;
    u8  unk_1DE[0x1E6 - 0x1DE];
    s16 unk_1E6;
    s16 unk_1E8;
    s16 unk_1EA;
    Vec3i32 unk_1EC;               /* func_800207C8: bone 11's matrix applied to D_800A3138 (0, 0x1000, 0) */
    Vec3i32 unk_1F8;
    u8  unk_204[0x210 - 0x204];
    LeafPos unk_210[3];            /* func_8002C61C: copy of scratchpad points 0x1F800000 + idx * 0x24 */
    LeafPos unk_234[2];            /* func_8002C61C: copy of scratchpad points 0x1F800048 + idx * 0x18 */
    Vec4i32 unk_24C;
    LeafPos unk_25C;               /* func_80023F08: copy of scratchpad point unk00[idx][0] */
    s32 unk_268;
    s16 unk_26C;
    s16 unk_26E;
    s16 unk_270;
    s16 unk_272;
    s16 unk_274;
    s16 unk_276[4];
    s16 unk_27E[4];
    s16 unk_286;
    u16 unk_288[2];                /* per blade (unk_A1 / unk_A3 bounds), func_80023F08 */
    s32 unk_28C;
    MotionFrame unk_290;           /* the motion frame func_80023F08 last used */
    u16 unk_314;
    u16 unk_316;
    s16 unk_318;
    s16 unk_31A;
    s16 unk_31C;
    u8  unk_31E[0x320 - 0x31E];
    Vec3i32 unk_320;
    u8  unk_32C[0x330 - 0x32C];
    s16 unk_330;
    s16 unk_332[12];               /* queued kinds, unk_330 of them (func_8003047C / func_80030B10) */
    u8  unk_34A;
    u8  unk_34B;
    u8  unk_34C;
    u8  unk_34D;
    u8  unk_34E;                   /* written by func_8001BE20 for the OTHER record */
    u8  unk_34F[0x350 - 0x34F];
    s16 unk_350;
    s16 unk_352;                   /* index into game_GetPlayerData()'s MATRIX * table and SPAD->unkA8[] (func_800204C0) */
    Vec3i32 unk_354;               /* func_800203B4's gte_stlvnl output; func_800204C0's gte_ldlv0 input */
    CpuRoute cpu_route;            /* 0x360 */
    s32 unk_394;
    s16 unk_398;
    s16 unk_39A;
    u8  unk_39C;
    u8  unk_39D;
    s16 unk_39E;
    s16 unk_3A0;
    s16 unk_3A2;
    u16 *unk_3A4;                  /* script list: u16 byte offsets from its own start */
    u16 *unk_3A8[3];               /* per-row cursor into unk_3A4 (func_80055138) */
    u8 *unk_3B4;                   /* script pointer (func_80055B44 / func_80055948) */
    u8  unk_3B8;
    u8  unk_3B9[0x3BC - 0x3B9];
    u8  unk_3BC;
    u8  unk_3BD;
    u8  unk_3BE[0x3C0 - 0x3BE];
    u8  unk_3C0;
    u8  unk_3C1;
    s16 unk_3C2;
    s32 unk_3C4;
    s32 unk_3C8;
    s32 unk_3CC;
    PadState unk_3D0;              /* the pad record func_80055B60 builds for func_8001BE20 */
    u16 unk_3E8;
    u8  unk_3EA[0x3EE - 0x3EA];
    s16 unk_3EE;
    s16 unk_3F0;
    u8  unk_3F2;
    u8  unk_3F3;
    u8  unk_3F4;
    u8  unk_3F5;
    u8  unk_3F6;
    u8  unk_3F7;
    s16 unk_3F8[3];                /* [unk_86] distance bounds (func_80055138) */
    s16 unk_3FE[3];
    s16 unk_404[3];
    s16 unk_40A;
    s8  unk_40C;
    s8  unk_40D;
    s16 unk_40E;
    s16 unk_410;
    s16 unk_412;
    u8  unk_414[8][2];             /* func_80055B60: 8 (target id, count) pairs */
    u8  unk_424;
    u8  unk_425;
    u8  unk_426;
    u8  unk_427;
    s16 unk_428;
    s16 unk_42A;
    s16 unk_42C;
    s16 unk_42E;
    s32 unk_430;
    s32 unk_434;
    s16 unk_438;
    s16 unk_43A;
    s16 unk_43C;
    s16 unk_43E;
    u8  unk_440;
    u8  unk_441;
    u8  unk_442;
    u8  unk_443;
    u8  unk_444[8];                /* func_80056CB8's per-direction results */
} Unk80101EC8Record;                 /* sizeof == 0x44C */

/* 14-byte record table indexed by the kind field (+0x2) of the 0x64-byte
 * objects at D_80106A78: func_80030580 and func_80031B24 index it with
 * stride 14, func_80030D7C reads +0x0 / +0xA of a record. */
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

/* The twelve 0x64-byte object records at 0x80106A78. func_80030580 spawns one
 * (its kind indexes D_8008E194 / D_8008EB80), func_80030D7C moves them,
 * func_80031B24 tests them against both fighters, func_80030208 hands them to
 * the effect calls; kind == -1 marks a free record. Field widths are the
 * consumers' loads and stores (asm/funcs/func_80030580.s, func_80030D7C.s). */
typedef struct {
    s16 unk_00;                    /* 0 at spawn; func_80030D7C adds 1 per step while unk_50 != 0 */
    s16 kind;                      /* -1 = free */
    u8  unk_04;
    u8  unk_05;
    u8  owner;                     /* .index of the spawning record (func_80030580) */
    u8  unk_07;
    u8  unk_08;
    u8  unk_09;                    /* index into the owner's matrix table (func_800300B4) */
    u8  slot;                      /* own index in D_80106A78 once allocated; 0xFF = free (set by func_8003043C at init and by func_80030D7C once kind == -1) */
    u8  unk_0B;
    MATRIX mtx;                    /* func_8002FF20 builds it (identity, RotMatrixX/Y/Z,
                                      MulMatrix0); func_800300B4 reads it */
    Vec3i32 pos;                   /* += vel each func_80030D7C step */
    Vec3i32 prev_pos;              /* pos before the step */
    Vec3i32 vel;
    s32 unk_50;                    /* != 0: moving */
    s16 rot[3];                    /* RotMatrixX/Y/Z angles; += rot_vel each step */
    u8  unk_5A[0x5C - 0x5A];
    s16 rot_vel[3];
    u8  unk_62[0x64 - 0x62];
} Obj80106A78;                     /* sizeof == 0x64 */

/* 2-byte {a,b} threshold pairs. D_8008EA44: indexed by (type - 2), 5 entries (types 2..6);
   D_8008EBFC: indexed by leaf category, 6 entries. The original binary indexes both at a
   2-byte stride and reads both bytes at the same index (asm/funcs/func_800335D8.s). */
typedef struct {
    u8 a;
    u8 b;
} LeafThreshold;

/* 6-row tables func_80026DA4 selects by D_80101EC8[0].unk_6A mode (row 0..5): unk0
 * scales the Judge sin/cos offset, unk2 is added to y; D_8008EB6C[row] is
 * passed as func_80032854's arg1. */
typedef struct {
    s16 unk0;
    s16 unk2;
} Tbl8008EB54Entry;

/* 0x18-byte per-character status record table (named_syms.txt:
 * g_status_flag_record_table_80099D88). The original binary indexes it by character id
 * with stride 0x18 (id*3<<3) in func_80055138, func_80055948, func_80055B60 and
 * func_80058580: flags halfword at +0, bytes at +3..+8, +0xC, +0xF, +0x14, +0x15. The
 * splat symbols D_80099D8B..D_80099D9D are fields of record 0 (alias rows in
 * undefined_syms_auto.txt, retired with func_80055B60 / func_80058580). */
typedef struct StatusFlagRec {
    u16 flags;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 pick_weight[7];   /* +0x8: func_80058580's 7-entry random pick, indexed by pick */
    u8 script_weight[8]; /* +0xF: func_80058580's script-entry types 0..7 (et), indexed by type */
    u8 unk17;
} StatusFlagRec; /* size 0x18 */

/* Rows of eight 4-byte entries starting at 0x8009A8C8: each 0x20-byte row ends with a zero
 * entry (0x8009A8E4 / 0x8009A904 / 0x8009A924, asm/data/7D920.data.s). Both readers index the
 * column 1-based, [row][D_800A37A0 - 1]: func_80055138 reads unk0/unk1 (GCC folds the -1
 * into the address, %lo(0x8009A8C4)), func_80058580 the mask halfword (%lo(0x8009A8CA)). */
typedef struct CpuLevelEntry {
    u8 unk0;
    u8 unk1;
    u16 mask;
} CpuLevelEntry;

/* 0x44-byte record shared by the two camera-target objects at 0x800F5328 and
 * 0x800F6608. Field span evidenced by func_8001B294 (initialises +0x00/04/08/10/12/14/
 * 18/1E/30/32/34/38/3A/3C on the 0x800F6608 object) and func_8001B3C0 (the same
 * treatment of 0x800F5328 at +0x00/04/08/30/32/34/38/3A/3C/40); +0x00..+0x0B is one
 * 12-byte vector: func_8001BC70 / func_8001BCF0 copy it as a struct (three lw, then three
 * sw through one base register); member-by-member copies compile differently.
 * Replaces the per-word splat symbols D_800F532C..D_800F5368 / D_800F660C..D_800F6644. */
typedef struct Rec44 {
    Vec3i32 unk_00;
    s32 wC;
    s16 h10; s16 h12; s16 h14; s16 h16;
    s32 w18;
    s16 h1C; u8 b1E; u8 b1F;
    s32 w20; s32 w24; s32 w28; s32 w2C;
    s16 h30[2][4];                 /* two 4 x s16 limit vectors; func_8001A820 passes h30[p] to func_8001A67C */
    u8 b40; u8 b41; u8 b42; u8 b43;
} Rec44;

/* 0x1C-byte record: one player's state in one frame of the replay buffer D_800A36EC (frames
 * of two, Rec1C[2] = 0x38 bytes; func_80039680 writes player a0->index's record of frame
 * D_800A36F8 from the character record, func_8003993C reads both back and passes two of them
 * to func_8001BAE4 / func_8001BBD8, func_8001F1C4 reads b14..b16 / b18).
 * - w0: the character's current move (Unk80101EC8Record.unk_50); func_8003993C reads its unk_04.
 * - h4 / h6 / h8: unk_F4 x / y / z; hA: unk_1C8.vy & 0xFFF with unk_B3 in the top four bits;
 *   hC: unk_148; hE / h10 / h12: unk_64 / unk_66 / unk_68.
 * - b14..b16: unk_1E6 / unk_1E8 / unk_1EA >> 2, read back signed; b17: bit 0 = unk_60 != 0,
 *   bit 1 = unk_61 != 0; b18: unk_62; b19: unk_40. */
typedef struct Rec1C {
    MoveScript *w0;
    s16 h4; s16 h6; s16 h8;
    u16 hA;
    s16 hC; s16 hE; s16 h10; s16 h12;
    s8 b14; s8 b15; s8 b16;
    u8 b17; u8 b18; u8 b19;
    u8 pad1A[2];
} Rec1C;

/* The 0xB4 0x10-byte records of the event table D_800F68E0 (func_800392C8 clears unk_0 to -1,
 * func_80039320 ages them, func_800393C8 adds or refreshes one, func_8003993C replays them).
 * unk_0: -1 = free, else a frame counter; unk_2: frames seen; unk_3: func_800393C8's arg1;
 * unk_4: the s16 x / y / z of arg2; unk_A: arg3[0] & 0xFFF with arg0 in the top four bits;
 * unk_C / unk_E: arg3[1] / arg3[2]. */
typedef struct Unk800F68E0Rec {
    s16 unk_0;
    u8 unk_2;
    u8 unk_3;
    s16 unk_4[3];
    u16 unk_A;
    s16 unk_C;
    s16 unk_E;
} Unk800F68E0Rec;

/* The 0x20 0x10-byte records of the event table D_80101BF0 (func_800392C8 sets unk_0 to 0xFF =
 * free, func_80039320 frees the current frame's, func_800395B4 fills one, func_8003993C replays
 * those whose unk_0 is the frame index). unk_0: the frame (D_800A36F8); unk_1 / unk_2:
 * func_800395B4's arg0 / arg1; unk_4: the s16 x / y / z of arg2; unk_A: arg3[0..2]. */
typedef struct Unk80101BF0Rec {
    u8 unk_0;
    u8 unk_1;
    u8 unk_2;
    u8 pad3;
    s16 unk_4[3];
    u16 unk_A[3];
} Unk80101BF0Rec;

/* The 4-byte record func_8001CD68 fills from the frame counter D_800A3858 (30 per
 * second): unk_0 = count / 1800, unk_2 = count / 30 % 60, unk_3 = count % 30 * 100 / 30
 * (99 / 59 / 99 past 0x2BF1F). func_8005D814 draws the three as two-digit numbers;
 * func_8003C714 copies them into its save record at +0x2D..+0x2F. func_8001CE60 also
 * fills only unk_2 = n / 30 and unk_3 = n % 30 * 100 / 30 from a frame countdown n, for
 * func_8005F1C8 to draw. */
typedef struct Unk8001CD68Rec {
    s16 unk_0;
    u8 unk_2;
    u8 unk_3;
} Unk8001CD68Rec;

/* Two 0x58-byte records set up side by side by func_80049E4C: 0x80101DF0
 * (unk0 = 0x64, unk8 = 5; pointer stored to D_800A3708) and 0x800FF638
 * (unk0 = 0x65, unk8 = 2; pointer stored to D_800A370C). func_800418D0
 * passes a negated stack copy of xf.rot (+0x10..0x14; the record is not
 * written) and &work (+0x38) to g_anim_func_table[unk8], then copies work
 * to xf.mat (+0x18);
 * func_80046BF4 writes work.t (+0x4C/+0x50/+0x54) just before that call.
 * func_800475A4 passes &xf.rot to g_anim_func_table[0] and &xf.mat to
 * ApplyMatrix; func_8003E6D8 forms one base at &xf (0x80101E00) and passes
 * base+0x1C (xf.mat.t). g_cam_bone_data2 (0x800EEDF0) and D_800EF070 are
 * two more records of this layout: camera_InitRotation initialises the
 * first (u8/s8 stores at +0x00/+0x01, s16 stores at +0x02/+0x04/+0x08/+0x0A,
 * unkC = 0 at +0x0C, rot, work.t, then xf.mat = work; it does not write +0x06),
 * func_800477E8 sets up the second (+0x06 as s16) and passes it to
 * func_800417D0, which reads +0x06 as an s16 state.
 * rot is the RotMatrix-style angle SVECTOR the rotation handlers take; mat and
 * work are libgte MATRIXes (MulMatrix0 / ApplyMatrix / MulMatrix2 operands). */
typedef struct {
    SVECTOR rot; /* +0x00 */
    MATRIX mat; /* +0x08 */
} Unk80101DF0Xform;

typedef struct Unk80101DF0Record {
    u8 unk0;               /* +0x00 */
    s8 unk1;               /* +0x01 */
    s16 unk2;              /* +0x02 */
    s16 unk4;              /* +0x04 */
    s16 unk6;              /* +0x06 */
    s16 unk8;              /* +0x08 g_anim_func_table index */
    s16 unkA;              /* +0x0A */
    struct Unk80101DF0Record *unkC; /* +0x0C */
    Unk80101DF0Xform xf;   /* +0x10 */
    MATRIX work;   /* +0x38 */
} Unk80101DF0Record;      /* 0x58 */

/* The 8 light-setup records at 0x800F62E0 (0x60 each; base + n*0x60 in
 * func_800470B0, func_80049F4C).  func_8004A09C fills one from a 22-halfword
 * template: the color matrix +0x38 (m[3][3]), the three lights' pitch / yaw /
 * on at +0x00 / +0x08 / +0x10, the u8 back color +0x58 and the s16 +0x5C;
 * func_8004A1FC then turns light i's pitch / yaw into the light matrix row i
 * (+0x18 + i*6, zero when off), scaled by +0x5C.  func_80049F4C hands record
 * 0's color matrix and back color to SetColorMatrix / SetBackColor; the light
 * matrix goes to gte_MulMatrix0ClearTrans (asm/funcs/func_8004A940.s:70-72);
 * func_80046F24 / func_800470B0 read its row 0.  Replaces the splat per-field
 * scalars D_800F62E2..D_800F64BA, g_gte_color_matrix_data and
 * g_gte_back_color_r/g/b. */
typedef struct {
    s16 pitch;
    s16 yaw;
    s16 on;
    s16 pad;
} Unk800F62E0Light;

typedef struct {
    Unk800F62E0Light light[3]; /* +0x00 */
    MATRIX lmat;       /* +0x18 */
    MATRIX cmat;       /* +0x38 */
    u8 back[3];                /* +0x58 */
    s16 unk5C;                 /* +0x5C */
} Unk800F62E0Rec;              /* 0x60 */

/* 0x800F66A0: the rotation-to-matrix handlers a transform node's unk8 selects
 * (rot -> matrix, PsyQ RotMatrix shape).  func_80042E90 fills [0] ZYX, [2] ZXY,
 * [4] YXZ, [5] XYZ ([1] and [3] are never written); the nodes set unk8 to 0, 2,
 * 4 and 5, and func_8003EDC0 / func_800417D0 / func_800418D0 / camera_InitRotation
 * index it by unk8 (4-byte stride).  _svm_vab_vh follows at 0x800F66B8. */
typedef void (*AnimRotFunc)(SVECTOR *, MATRIX *);

/* The 0x68-byte records of the table game_GetCharData returns: a transform
 * node of the Unk80101DF0Record layout, then a byte flag. func_8003EDC0
 * fills them from a stream and calls g_anim_func_table[unk8] on &xf.rot /
 * &xf.mat (func_800418D0's call); func_8003E6D8 and func_8003EB84 queue each
 * one on the D_800A3820 list at most once, guarded by unk58. */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    u8 unk58;               /* +0x58 */
    u8 unk59[0xF];          /* +0x59 */
} Unk800A6690Rec;           /* 0x68 */

/* The 0x68-byte entries of the D_800A9CF8.unkC table (the buffer
 * func_80044670 is handed; it returns the end, base + count * 104). Each
 * is a transform node of the Unk80101DF0Record layout paired with one
 * game_GetCharData entry of the same index (D_800A9CF8.unk10):
 * func_8004473C initialises the node and copies the paired entry's
 * xf.mat.t into work.t; func_80044B30 / func_80044800 run the node from
 * the D_800A9CF8.unk8 key frames and queue it on the D_800A3820 list.
 * unk58 is a word frame counter (lw/sw: -1 idle, 0 started by
 * func_80044B30, -2 done), unk5C the word Y angle func_80044B30 stores,
 * unk60 an s16 fade level (lh/sh in func_80044800); +0x62..0x67 are not
 * accessed. Not the Unk800A6690Rec layout: that table's +0x58 is a byte
 * (lbu/sb in func_8003E6D8 / func_8003EB84). */
typedef struct {
    Unk80101DF0Record node; /* +0x00 */
    s32 unk58;              /* +0x58 */
    s32 unk5C;              /* +0x5C */
    s16 unk60;              /* +0x60 */
    s8 pad62[6];            /* +0x62 */
} Unk800A9CF8Entry;         /* 0x68 */

/* The 16-byte records func_8003EDC0 fills ahead of those (unk8 / unkC = the
 * grid cell's column / row * 2000 - 32000); func_8003E6D8 and func_8003EB84
 * set unk6 and the unk7 bits and queue them on the D_800A3820 list. */
typedef struct {
    s16 unk0;  /* +0x00 */
    s16 unk2;  /* +0x02 */
    u16 unk4;  /* +0x04 */
    u8 unk6;   /* +0x06 */
    u8 unk7;   /* +0x07 */
    s32 unk8;  /* +0x08 */
    s32 unkC;  /* +0x0C */
} Unk800A4750Rec;          /* 0x10 */

/* One entry of the CD file table (sweep-2026-09-24 data_manifest.csv:18-19, CONFIRM:
 * 159/159 entries equal the disc's ISO9660 directory records). */
typedef struct {
    CdlLOC loc;                    /* the file's start position */
    u32 size;                      /* the file's length in bytes */
} CdFileEntry;

/* The replay-camera / CD-read words at 0x80101E60..0x80101EA7: the tail of
 * CD state block D_80101E58 (type CdState below), where the evidence that they are one object with
 * its head is set out.  Member widths follow the original accesses;
 * 0x80101E91..93 is the compiler's alignment padding.  unk3A (0x80101E9A)
 * lies inside the proven span's object (its size is a multiple of 4) and is
 * func_80036140's halfword (sh 80036548, lhu 80036778 / sh 80036788).
 *
 * unk3C..unk44 (0x80101E9C..0x80101EA7) are in the object by compiler
 * necessity (aggregate-merge prong (a), (a1)/(a2)/(a4')): func_80036140 is
 * compiled -G8, and there its read-modify-writes of 0x80101E9C and
 * 0x80101EA4 keep the address in a register (la; lX 0(r); sX 0(r)) only for
 * a variable larger than 8 bytes -- a smaller one is small data, and cse
 * folds any pointer back to the symbol; the original compiler agrees.
 * Separate variables, block- or function-scope pointers, an object ending at
 * 0x80101E9F or 0x80101EA3, and a separate 12-byte record at 0x80101E9C all
 * miss func_80036140's bytes; only this object matches.  unk3E and expected_pos
 * lie inside that span but func_80036140 never touches them: they are typed
 * by their other users' original accesses (owner rulings Q13/Q14): unk3E by
 * game_FrameLoop / cdrom_StartRead (u16, the lhu at 80036F9C); expected_pos
 * by cdrom_ReadyCallback / func_80036940 (s32: no access reveals its
 * signedness, and s32 / u32 build byte-identical).
 * Evidence: pre-slim-2026-10-01:memory/grind/func_80036140/evidence.md.
 *
 * The 8-byte `pair` is also one CdFileEntry by the table it is copied from: the
 * source is an element of g_cd_file_table, copied as a whole CdFileEntry
 * aggregate (cdrom_StartRead, cdrom_StartAudio).  The CdPosToInt/CdIntToPos calls on it evidence
 * only `pair.loc`: CdIntToPos (src/main/psxsdk/libcd/sys.c) writes just its minute / second /
 * sector. */
typedef struct {
    s16 unk00; /* 0x80101E60 */
    s16 unk02; /* 0x80101E62 */
    s16 unk04; /* 0x80101E64 */
    s16 unk06; /* 0x80101E66 */
    s16 unk08; /* 0x80101E68 */
    s16 unk0A; /* 0x80101E6A */
    CdFileEntry pair; /* 0x80101E6C .. 0x80101E73 */
    s32 unk14; /* 0x80101E74 */
    s32 unk18; /* 0x80101E78 */
    s32 unk1C; /* 0x80101E7C */
    s32 sectors_remaining; /* 0x80101E80 */
    s32 dest_buffer; /* 0x80101E84 */
    s32 unk28; /* 0x80101E88 */
    s32 unk2C; /* 0x80101E8C */
    u8 unk30; /* 0x80101E90 */
    s32 unk34; /* 0x80101E94 */
    s16 unk38; /* 0x80101E98 */
    s16 unk3A; /* 0x80101E9A */
    s16 unk3C; /* 0x80101E9C */
    u16 unk3E; /* 0x80101E9E */
    s32 expected_pos; /* 0x80101EA0 */
    s32 unk44; /* 0x80101EA4 */
} ReplayCamRec;

/* The CD module's state block, ONE object of 0x50 bytes at 0x80101E58.  Owner
 * ruling Q43 bounds it to the span proven: 0x80101E58..0x80101E99 by the
 * original binary's addressing, through 0x80101EA7 by func_80036140's compiler
 * necessity (ReplayCamRec above; aggregate-merge prong (a)).  The first span
 * is proven by three links:
 *   - 0x80101E58..0x80101E62 is one object: cdrom_StartAudio forms the
 *     CdlSetfilter parameter (file, chan) at 0x80101E58 as &0x80101E62 - 0xA
 *     (800370AC addiu a1,s0,-0xA; 800370B0 sb v0,-0xA(s0)).  Only those two
 *     bytes are accessed; 0x80101E5A..5B is the compiler's alignment padding
 *     before unk04.  cse relates two constant addresses only when
 *     they are offsets of one symbol.
 *   - 0x80101E6C..0x80101E99 is one object: func_80036940 forms &rec.pair
 *     (0x80101E6C) as &0x80101E8C - 0x20 (80036A74) and as &0x80101E98 - 0x2C
 *     (80036B08).
 *   - 0x80101E60..62 and 0x80101E6C are in one object (aggregate-merge prong
 *     (a), (a1)/(a2)): cdrom_StartAudio reloads rec.unk00 (lh at 8003703C)
 *     only after its CdFileEntry copy into rec.pair (sw at 8003702C/80037034).
 *     sched1 orders the two through true_dependence (sched.c:817) ->
 *     memrefs_conflict_p (sched.c:614): SIZE_FOR_MODE(BLKmode) is 0, so the
 *     aggregate store conflicts with the halfword load only when both
 *     addresses share one base symbol_ref (sched.c:777).  As separate objects
 *     there is no dependence and the load is hoisted above the copy.  One
 *     object matches cdrom_StartAudio under both cc1 and the original cc1psx;
 *     a cut at 0x80101E64, 0x80101E68 or 0x80101E6C misses under both.
 *     Evidence: pre-slim-2026-10-01:memory/grind/cdrom_StartAudio/evidence.md.
 */
typedef struct {
    u8 file; /* 0x80101E58 */
    u8 chan; /* 0x80101E59 */
    s32 unk04; /* 0x80101E5C */
    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */
} CdState;

/* Practice-lesson parameter block 0x80102778..0x80102787 (func_8001C444 sets
 * every byte of it except 0x82/0x83): two u16 values, three per-player byte
 * pairs kept as one array (unk_4[2 * k + player], [0] = P1, [1] = P2), a
 * fourth per-player pair only other functions touch (unk_A, indexed by player
 * in func_80022F34) and four single bytes. One object: func_80034708 reaches
 * unk_4 and unk_E as offsets from the address of unk_C (layout evidence:
 * pre-slim-2026-10-01:memory/grind/func_80034708/evidence.md). */
typedef struct {
    u16 unk_0[2];
    u8 unk_4[6];
    u8 unk_A[2];
    u8 unk_C;
    u8 unk_D;
    u8 unk_E;
    u8 unk_F;
} PracticeParams;

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;

/* MOD.BIN (resource file 2), the root 51268's func_80068F70 loads at its work area + 0x58; it keeps
 * it in D_800A34FC's word 9, and func_8006E390 copies that into word 1 of the draw context. After
 * the head:
 * - unk_14..unk_40: the twelve lists func_8006919C relocates (func_8006920C: entries up to a 0
 *   word, -1 entries skipped). Most entries are sprite sheets, a 12-byte Unk8009B0E0Record header
 *   followed by its 8-byte Unk8009B400Record cells (hence the readers' `+ 0xC`). The lists stay
 *   s32 *: their entries go into the s32 header words of the draw descriptors, which keep that
 *   type until the descriptors are unified.
 * - unk_44: the Rec_8006C21C rows func_8006C21C draws; unk_48: the points func_8006BEC4 reads.
 * The -1 ending func_8006E440's offset list follows at +0x4C; no reader touches it. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 *unk_14;
    s32 *unk_18;
    s32 *unk_1C;
    s32 *unk_20;
    s32 *unk_24;
    s32 *unk_28;
    s32 *unk_2C;
    s32 *unk_30;
    s32 *unk_34;
    s32 *unk_38;
    s32 *unk_3C;
    s32 *unk_40;
    Rec_8006C21C *unk_44;
    Vec2s16 *unk_48;
} Unk8006919CRec;

/* 0x800A3D40: 24-byte records func_8003D774 starts and func_8003D7B4 advances: the bit stream
   bitstream_ReadBits reads (unk0, its u32 words) and six s16 values func_8003D7B4 adds the decoded
   deltas to and returns (3AB48 func_8005490C reads them as an offset and a rotation). */
typedef struct {
    u32 unk0[3];
    s16 unkC[6];
} Unk800A3D40Rec;

/* 5ED34's work block: the 0x14 bytes func_8006E534 keeps at the start of func_8006E49C's returned
 * space (D_800A35C4; the arena cursor D_800A356C moves past it), as 51268's Unk800A34FCRec.
 * - unk_00 / unk_04: one s16 per player each (counted down from 0x1E by the draw handlers).
 * - unk_08: the frame counter func_8006EACC advances; unk_0C: its buffer counter (bit 0 picks the
 *   half of the Unk8006E49CRec pair).
 * - unk_10: the draw offset the handlers hand to SetDrawOffset. */
typedef struct {
    s16 unk_00[2];
    s16 unk_04[2];
    s32 unk_08;
    s32 unk_0C;
    u16 unk_10[2];
} Unk800A35C4Rec;

/* 51268's work block: the 0x34 bytes func_80068F70 keeps at the start of func_8006E49C's
 * returned space (D_800A34FC; its arena cursor D_800A3500 moves past the block). func_80068F70
 * zeroes unk_0C / unk_10 / unk_12, sets both unk_28 halves to 5 and unk_30 to bit 0 of D_800A3524's
 * word 8 (func_80069120 compares and refreshes it), and stores the MOD.BIN root in unk_24.
 * - unk_0C: the s16 pair func_800692C0 steps (its arg2); the draw functions use unk_0C[0] / [1] as
 *   x / y offsets.
 * - unk_28: one s16 per player (func_8006C21C, func_8006CBD4, func_8006CCC8); func_8006CCC8 and
 *   func_8006CFBC also test the pair as one word (== 0x50005: both 5), as SelWork's f1C / f20.
 * No code touches the other bytes. */
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

typedef struct { s32 f0, f1, f2, f3; } Copy16;

#endif /* GAME_H */
