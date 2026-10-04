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

/* Scratchpad point tables at 0x1F800000.  unk00: three points per character
 * (func_8002C61C copies [0][0..2] and [1][0..2] to the two records' +0x210);
 * unk48: two more per character (copied to +0x234); unk78: two body points
 * per character (func_800288C8 builds them from unkA8 joint 1 and the
 * midpoint of joints 15 and 19); unkA8: 22 points per
 * character (func_8002A458 reads 0x1F8000A8 + id * 0x108 + i * 0xC, i < 22).
 * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48. */
typedef struct {
    LeafPos unk00[2][3];
    LeafPos unk48[2][2];
    LeafPos unk78[2][2];
    LeafPos unkA8[2][22];
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
 * unk_0A the three rotation angles it hands math_RotMatrixZYXAngles. */
typedef struct MotionFrame {
    s16 unk_00;
    u16 unk_02;
    u16 unk_04;
    u16 unk_06;
    u16 unk_08;
    u16 unk_0A;
    u16 unk_0C[0x3C];
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
 * treatment of 0x800F5328 at +0x00/04/08/30/32/34/38/3A/3C/40); +0x00/04/08 are the
 * Vec3 that func_8001BC70/func_8001BCF0 already copy through a struct assignment.
 * Replaces the per-word splat symbols D_800F532C..D_800F5368 / D_800F660C..D_800F6644. */
typedef struct Rec44 {
    s32 w0; s32 w4; s32 w8; s32 wC;
    s16 h10; s16 h12; s16 h14; s16 h16;
    s32 w18;
    s16 h1C; u8 b1E; u8 b1F;
    s32 w20; s32 w24; s32 w28; s32 w2C;
    s16 h30[2][4];                 /* two 4 x s16 limit vectors; func_8001A820 passes h30[p] to func_8001A67C */
    u8 b40; u8 b41; u8 b42; u8 b43;
} Rec44;

/* 0x1C-byte record: func_8003993C walks an array of these at D_800A36EC with element
 * stride 0x1C (base + i*0x38 + {0,0x1C} + D_800A3748*0x1C, asm/funcs/func_8003993C.s:57-89)
 * and passes two of them to func_8001BAE4 / func_8001BBD8. */
typedef struct Rec1C {
    s16 h0; s16 h2; s16 h4; s16 h6; s16 h8; s16 hA; s16 hC; s16 hE;
    s32 w10; s32 w14; s32 w18;
} Rec1C;

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

typedef struct { s32 f0, f1, f2, f3; } Copy16;

typedef struct {
    void *p0;
    s32 *p1;
    s32 pad08;
    s32 ret;
    s32 zero10;
    s32 one14;
    s32 zero18;
    s32 zero1C;
    s32 c20;
    s32 c24;
    s8 byte28;
    u8 byte29;
    u8 byte2A;
    u8 byte2B;
} S46C;

typedef struct {
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
    s32 sp38;
    s32 sp3C;
    s8 sp40;
    u8 sp41;
    u8 sp42;
    u8 sp43;
} S_80074488;

/* 0x8009BD24: two players x five rounds of 2-byte records; byte 0 is the
   character the round was fought with (func_8005E54C reads it at
   j * 10 + i * 2 and picks UesrWorkDef / D_8009B58C by it; func_80060414 reads
   player 0 round 0). 0x14 bytes, ending at the flag word below. */
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;

/* 0x8009BD38: the match-settings flag word, bit fields named by bit offset.
   Its C readers extract it by field: unk0 (`& 0xF`), unk10 (the round count
   - 3; also picks the results-screen layout), unk12 (`== 2` tests), unk14
   (1 bit), unk15 (one bit per player); func_80077894 stores unk0. Byte 3 is
   not named here (main/64FD8.c reads it as D_8009BD3B). */
typedef struct {
    u32 unk0 : 4;
    u32 unk4 : 6;
    u32 unk10 : 2;
    u32 unk12 : 2;
    u32 unk14 : 1;
    u32 unk15 : 2;
    u32 unk17 : 1;
    u32 unk18 : 6;
} Unk8009BD38Flags;

/* GameObj: a generic 0x100-byte layout, fields named by offset (field_XX), from the early m2c
 * context tooling (feaa560b2) rather than from the game's objects. In C the only member used by
 * name is field_18, a GPU primitive write cursor (func_80069898 and func_80070C70 build primitives
 * at it and advance it); func_80072BC4 / func_80072CD4 take a GameObj * but write a POLY_G4
 * through byte casts. Retyping these users is Phase 2 work. */
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;

#endif /* GAME_H */
