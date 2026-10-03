#ifndef GAME_H
#define GAME_H

/* Game state - gameplay, stage, pad, camera, animation */

#include "common.h"

/* Named globals */
extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_stage_id;
extern s16 g_stage_variant;


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

extern Unk800F1198Record D_800F1198[];

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

extern Unk800A9CF8Header D_800A9CF8;

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

extern Unk800F0EC8Record D_800F0EC8[][10];

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

extern Unk800F0E38Record D_800F0E38[12];

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

extern MenuOption D_8009BC0C[8];

/* 0x8009BCC4: the three scroll pages' origins, s16 (x, y) pairs for pages 4..6
 * (0x8009BCC4..0x8009BCCF). Object model evidence from the original binary:
 * func_800720FC reads (x, y) of entry `mode` through ONE index register
 * (`sll $a2,mode,2`, then `lhu %lo(D_8009BCC4)($at)` and `lhu %lo(D_8009BCC6)($at)`
 * with the same $a2), and its scroll loop reads page p's pair at
 * 0x8009BCB4 + p * 4 for p = 4..6 (asm lines 220-233), i.e. entries 0..2 of
 * this table. Replaces the splat
 * per-word symbols D_8009BCC4 / D_8009BCC6 (per-word splat symbol -> aggregate
 * merge family, owner ruling). */
extern s16 D_8009BCC4[3][2];

/* 0x8009BCD0: the current scroll offset, one s16 per axis. func_800720FC walks
 * it with a 2-byte step bounded by &D_8009BCD0 + 4 (asm lines 220-288). Replaces
 * D_8009BCD0 / D_8009BCD2 (same merge family). */
extern s16 D_8009BCD0[2];

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

extern Unk8009BCF8Record D_8009BCF8[2][10];

/* 0x8009BD20: two 2-byte records, {0x01, 0x02} and {0x03, 0x04}
 * (asm/data/7D920.data.s:23805-23817, splat's D_8009BD20 / D_8009BD21 dlabels).
 * Object model evidence from the original binary: func_800747D8 takes the table
 * base into a register (`lui $a2,%hi(D_8009BD20); addiu $a2,$a2,%lo(D_8009BD20)`
 * at 0x800749B4) and adds the column; func_80074488 (0x800746A8-0x800746D4) and
 * func_800770B8 (0x80077338-0x80077344) index it with the row shifted left 1 (a
 * 2-byte stride); func_80074488 reads both columns (column 0 at %lo(D_8009BD20),
 * column 1 at %lo(D_8009BD21)), func_800770B8 only column 1 at
 * %lo(D_8009BD21). */
extern u8 D_8009BD20[2][2];

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
    void *f00;
    s32 *f04;
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
    void *f24;
    u8 pad28[4];
    s32 *f2C;
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

extern u8 *D_800A36A0;
#define SELWORK ((SelWork *)D_800A36A0)

/* 0x800FF558: one PsyQ MATRIX (m[3][3], pad, t[3]; 0x20 bytes). Object model
 * evidence from the original binary: func_8004A940 forms the single base
 * %hi/%lo(0x800FF558) and reads t[0..2] at +0x14/+0x18/+0x1C, loads words
 * +0x0..+0x10 into GTE control registers 0..4 (the SetRotMatrix sequence) and
 * passes the base as the MATRIX * of gte_MulMatrix0ClearTrans; func_80048BA4
 * passes +0x14 as ApplyMatrix's VECTOR * and writes the nine s16 at +0..+0x10.
 * Replaces the twelve per-word splat scalars 0x800FF558..0x800FF574 (per-word
 * splat symbol -> aggregate merge family). Declared through the struct tag so
 * this header needs no gte.h; the type is complete wherever gte.h is included. */
extern struct MATRIX D_800FF558;

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

extern Unk8009BC94Record D_8009BC94[][6];

/* 0x8009B398: table of 4 twelve-byte records (0x8009B398..0x8009B3C7;
 * D_8009B3C8 follows, different data). Object model evidence from the original
 * binary: asm/funcs/func_8005E098.s
 * forms ONE base `lui $s7,%hi(D_8009B3B0); addiu $s7,$s7,%lo(D_8009B3B0)`
 * (record 2) and reaches record 3 as `addiu $v1,$s7,0xC` and record 0 as
 * `addiu $a0,$s7,-0x18` -- base+offset addressing of one object at a 12-byte
 * stride. Data: all four records share one shape (word 0 = 0x0001001F,
 * word 2 = 0). Replaces the splat per-word scalars D_8009B398 / D_8009B3A4 /
 * D_8009B3B0 / D_8009B3BC in C (per-word splat symbol -> aggregate merge family,
 * owner ruling); the dlabels stay in asm/data as data labels (no C
 * handle; func_8005E54C reaches record 2 as &D_8009B398[j + 2]). */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk8009B398Record;

extern Unk8009B398Record D_8009B398[4];

/* 8-byte sprite records {s16, s16, u8 x4}. Object model evidence from the
 * original binary:
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
    s16 unk0;
    s16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
} Unk8009B400Record;

extern Unk8009B400Record D_8009B400[10];
extern Unk8009B400Record D_8009B458[3][2];

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

extern Unk8009B450Record D_8009B450[2];

/* 0x8009B388: two adjacent 8-byte sprite cells (Unk8009B400Record),
 * 0x8009B388..0x8009B397, the cell table func_8005D554 hands func_80073728 for
 * the row's header record 2 (cell 0) and records 3/4 (cell 1); each of those
 * D_8009B2E0 header records has cell count 1. No other function or asm file
 * references either label. One object: spelled &D_8009B388[0] / [1],
 * func_8005D554 keeps the base in $s7 as the original does
 * (asm/funcs/func_8005D554.s); two separate symbols do not.
 * Replaces the splat per-cell scalars D_8009B388 / D_8009B390 in C. */
extern Unk8009B400Record D_8009B388[2];

/* 0x8009B5F0: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B5F0..0x8009B60F. Object model evidence from the original binary:
 * asm/funcs/func_8005F1C8.s forms ONE
 * stride `sll $s0,$s0,4` (row counter * 16) and adds it to both
 * %lo(D_8009B5F0) (column 0: 0x8009B5F0 / 0x8009B600) and %lo(D_8009B5F8)
 * (column 1: 0x8009B5F8 / 0x8009B608) -- rows of two 8-byte records. Data:
 * all four records share the {s16, s16, u8 x4} shape (u8 [3] = 0x01 in each);
 * D_8009B610 (a 12-byte sheet header) follows. Replaces the splat per-word
 * scalars D_8009B5F0 / D_8009B5F8 in C (per-word splat symbol -> aggregate
 * merge family, owner ruling). */
extern Unk8009B400Record D_8009B5F0[2][2];

/* 0x8009B490: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B490..0x8009B4AF. Object model evidence from the original binary:
 * asm/funcs/func_8005E54C.s forms ONE stride `sll $s0,$s0,4` (row counter * 16)
 * and adds it to both %lo(D_8009B490) (column 0) and %lo(D_8009B498) (column 1),
 * the same shape as D_8009B5F0 above. Data: all four records share the
 * {s16, s16, u8 x4} shape; D_8009B4B0 (a 12-byte sheet header) follows. Replaces
 * the splat per-word labels D_8009B490 / D_8009B498 in C (per-word splat symbol
 * -> aggregate merge family, owner ruling). */
extern Unk8009B400Record D_8009B490[2][2];

/* 0x8009B0E0: table of 9 twelve-byte sprite-sheet headers (0x8009B0E0..0x8009B14B),
 * the record func_8007352C reads through EnvA.header (cell count at +2). Object
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
    u16 unk0;
    u8 count;
    u8 unk3;
    s32 unk4;
    s32 unk8;
} Unk8009B0E0Record;

extern Unk8009B0E0Record D_8009B0E0[9];
extern Unk8009B0E0Record D_8009B14C;
extern Unk8009B0E0Record D_8009B158;

/* 0x8009B164: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B164..0x8009B183. Object model evidence from the original binary:
 * asm/funcs/func_8005C8A8.s forms ONE base %hi/%lo(D_8009B164) in $s1 and passes
 * `addiu $v1,$s1,0x10` (row 1) as the second draw's cell table, and stores the
 * x of column 1 of each row through the splat labels D_8009B16C (row 0) and
 * D_8009B17C (row 1). Replaces the splat per-word labels D_8009B164 /
 * D_8009B16C / D_8009B17C in C (per-word splat symbol -> aggregate merge
 * family). */
extern Unk8009B400Record D_8009B164[2][2];

/* 0x8009B184: 2 x 8-byte sprite records (Unk8009B400Record), 0x8009B184..
 * 0x8009B193. Object model evidence from the original binary:
 * asm/funcs/func_8005C8A8.s forms ONE base %hi/%lo(D_8009B184) in $a2, passes it
 * as a cell table (the sheet header's count is 2) and stores record 0's x
 * through it (`sh $v0,0x0($a2)`); record 1's x is the splat label D_8009B18C.
 * Replaces the splat per-word labels D_8009B184 / D_8009B18C in C (per-word
 * splat symbol -> aggregate merge family). */
extern Unk8009B400Record D_8009B184[2];

/* 0x800A328C: one 8-byte sprite cell (Unk8009B400Record: x, y = 0, u, v = 0,
 * w 0x14, h 0x0E), the cell table func_80060414 hands func_8007352C in its
 * descriptor (func_8007352C reads it as SprtEntA {s16 x, y; u8 u, v, w, h}).
 * Defined in text1b_tu1b.c. */
extern Unk8009B400Record D_800A328C;

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

extern Unk8009B2BCRecord D_8009B2BC[3];

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

extern Unk800EFAE8Ctrl D_800EFAE8;

/* Two s16 slots at 0x800A34F0, indexed as one array. Object model evidence
 * (the original binary): asm/funcs/func_800678A8.s
 * reads the pair through ONE indexed access, `lh %lo(sym)(base + arg0*2)` with the
 * base folded to 0x800A34F0 - 8 for arg0 = 4/5 (callers func_800677B8 /
 * func_800677F4), and asm/funcs/func_80067D14.s forms the same folded base with
 * %hi/%lo. func_80061C00 writes slot 0 or slot 1 with the same value. Replaces
 * the splat per-word scalars D_800A34F0 / D_800A34F2. */

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

extern Unk800F0C10Record D_800F0C10[4][3];

/* Per-effect-mode state, modes 0..0x11 (func_80065800 and its per-mode init and step
 * functions). Object model evidence (the original binary): asm/funcs/func_80065800.s
 * :39-45 addresses the position records as base + mode*12 (sll/addu/sll, then
 * %lo(D_800F0CA0)($at)) and :147-152 the timers as base + mode*2; modes 8/9 read
 * element mode-2, base - 4 + mode*2 (:351-356). The init functions
 * func_80064E90..func_800652AC fill the records and initialise the timers; the step
 * functions func_800652F4..func_800657B0 advance the timers. Replaces the splat
 * per-word scalars D_800F0BA8..D_800F0BCA and D_800F0CA0..D_800F0D74. */
extern s16 D_800F0BA8[18];
extern Unk800F0C10Record D_800F0CA0[18];

/* 0x800948BC: per-stage function pairs, indexed by stage_GetId().
 * stage_ExecInitFunc calls .init; func_8003E6D8 calls .unk4 (e.g. entry 13
 * holds camera_InitBone2 / func_800475A4). */
typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} StageFuncEntry;
extern StageFuncEntry g_stage_init_tbl[];

/* 0x8008D090: the per-mode main-loop handlers, indexed by D_800A3834 (defined in src/main/d_7D870.c). */
extern void (*g_module_func_tbl[])(void);

#endif /* GAME_H */
