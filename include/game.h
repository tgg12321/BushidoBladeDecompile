#ifndef GAME_H
#define GAME_H

/* Game state - gameplay, stage, pad, camera, animation */

#include "common.h"

/* Named globals */
extern u8 g_cd_file_table;
extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 D_800F6658;
extern s32 D_800A3790;
extern s16 g_stage_id;
extern s16 g_stage_variant;


/* 3-word record table at 0x800F1198, terminated by an all-zero record.
 * Object model evidence (independent of and predating any byte-chasing):
 * the original binary walks this table with a 12-byte-stride induction
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
 * 3-word record (12 bytes; lane stride 120). Object model evidence
 * (independent of and predating any byte-chasing): the original binary
 * addresses all three words through ONE offset register per access site --
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

/* 0x8009BCF8: 40-byte table of 20 two-byte records (0x8009BCF8..0x8009BD1F;
 * D_8009BD20 follows). Object model evidence, independent of and predating any
 * byte-chasing session, from the original binary: func_800759D0 takes the table
 * base into a register (`lui $s6,%hi(D_8009BCF8); addiu $s6,$s6,%lo(D_8009BCF8)`)
 * and reads `lbu %lo(D_8009BCF8)($at)` through a shift-1 (2-byte-stride) index;
 * func_80075F80 reads `lbu %lo(D_8009BCF8)($at)` through a shift-1 index;
 * func_80076D74 reads byte 1 of the record (`lbu %lo(D_8009BCF9)($at)`) through a
 * shift-1 index. Data: the unk1 column is 0x00..0x09 then 0x0C..0x15 (20 entries).
 * Replaces the splat per-word scalars D_8009BCF8 / D_8009BCF9 (per-word splat
 * symbol -> aggregate merge family, owner ruling 2026-08-17). */
typedef struct {
    u8 unk0;
    u8 unk1;
} Unk8009BCF8Record;

extern Unk8009BCF8Record D_8009BCF8[20];

/* 0x8009BC94: table of {x, y} s16 position records, 6 records (24 bytes) per
 * row. Object model evidence, independent of and predating any byte-chasing
 * session, from the original binary: func_8006F100 and func_80071C4C form ONE
 * offset per access (row*24 held in a strength-reduced register plus
 * D_800A3590[row]*4) and read `lh %lo(D_8009BC94)($at)` and
 * `lh %lo(D_8009BC96)($at)` through that same offset -- record stride 4,
 * row stride 24, base+offset addressing. Replaces the splat per-word scalars
 * D_8009BC94 / D_8009BC96 (per-word splat symbol -> aggregate merge family,
 * owner ruling 2026-08-17). */
typedef struct {
    s16 x;
    s16 y;
} Unk8009BC94Record;

extern Unk8009BC94Record D_8009BC94[][6];

/* 0x8009B398: table of 4 twelve-byte records (0x8009B398..0x8009B3C7;
 * D_8009B3C8 follows, different data). Object model evidence from the original
 * binary, independent of the byte-chasing session: asm/funcs/func_8005E098.s
 * forms ONE base `lui $s7,%hi(D_8009B3B0); addiu $s7,$s7,%lo(D_8009B3B0)`
 * (record 2) and reaches record 3 as `addiu $v1,$s7,0xC` and record 0 as
 * `addiu $a0,$s7,-0x18` -- base+offset addressing of one object at a 12-byte
 * stride. Data: all four records share one shape (word 0 = 0x0001001F,
 * word 2 = 0). Replaces the splat per-word scalars D_8009B398 / D_8009B3A4 /
 * D_8009B3B0 / D_8009B3BC in C (per-word splat symbol -> aggregate merge family,
 * owner ruling 2026-08-17); the dlabels stay in asm/data for the still-asm
 * func_8005D814 / func_8005E54C. */
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk8009B398Record;

extern Unk8009B398Record D_8009B398[4];

/* 8-byte sprite records {s16, s16, u8 x4}. Object model evidence from the
 * original binary, independent of the byte-chasing session:
 * asm/funcs/func_8005E098.s and asm/funcs/func_8005D814.s index 0x8009B400 by a
 * digit value through a shift-3 (8-byte-stride) index, and func_8005E098 stores
 * an s16 to offset 0 of the indexed record (`sh $v0,0x0($v1)`); it indexes 0x8009B458 by `sra 13` of an
 * s16 counter (i * 8) and 0x8009B468 / 0x8009B470 by counter * 16 -- pairs of
 * 8-byte records. Data: 0x8009B400..0x8009B44F is 10 records (one per digit),
 * 0x8009B458..0x8009B487 is 6 records of the same shape; D_8009B450 and
 * D_8009B488 follow. Replaces the splat per-word scalars D_8009B458 /
 * D_8009B468 / D_8009B470 in C (per-word splat symbol -> aggregate merge
 * family, owner ruling 2026-08-17). */
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

/* 0x8009B5F0: 2 x 2 table of 8-byte sprite records (Unk8009B400Record),
 * 0x8009B5F0..0x8009B60F. Object model evidence from the original binary,
 * independent of the byte-chasing session: asm/funcs/func_8005F1C8.s forms ONE
 * stride `sll $s0,$s0,4` (row counter * 16) and adds it to both
 * %lo(D_8009B5F0) (column 0: 0x8009B5F0 / 0x8009B600) and %lo(D_8009B5F8)
 * (column 1: 0x8009B5F8 / 0x8009B608) -- rows of two 8-byte records. Data:
 * all four records share the {s16, s16, u8 x4} shape (u8 [3] = 0x01 in each);
 * D_8009B610 (a 12-byte sheet header) follows. Replaces the splat per-word
 * scalars D_8009B5F0 / D_8009B5F8 in C (per-word splat symbol -> aggregate
 * merge family, owner ruling 2026-08-17). */
extern Unk8009B400Record D_8009B5F0[2][2];

/* Stage/match control block at 0x800EFAE8 (0x4C bytes). Object model evidence
 * (independent of and predating any byte-chasing): the original binary
 * addresses the whole block through ONE base register -- asm/funcs/func_80054604.s
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
 * (the original binary, independent of any byte-chasing): asm/funcs/func_800678A8.s
 * reads the pair through ONE indexed access, `lh %lo(sym)(base + arg0*2)` with the
 * base folded to 0x800A34F0 - 8 for arg0 = 4/5 (callers func_800677B8 /
 * func_800677F4), and asm/funcs/func_80067D14.s forms the same folded base with
 * %hi/%lo. func_80061C00 writes slot 0 or slot 1 with the same value. Replaces
 * the splat per-word scalars D_800A34F0 / D_800A34F2. */
extern s16 D_800A34F0[2];

/* Record table at 0x800F0C10 (0x90 bytes, ends at D_800F0CA0): 4 rows of 3
 * records of 3 s32 words. Object model evidence (the original binary):
 * asm/funcs/func_800678A8.s addresses it as base + arg1*36 + idx*12 (+0/+4/+8),
 * i.e. a row stride of 36 bytes and a record stride of 12; arg1 ranges 0..3
 * (callers func_800676C8..func_8006786C). The still-asm func_80067200 and
 * func_80067D14 reach the same words. Replaces the splat per-word scalars
 * D_800F0C10 / D_800F0C14 / D_800F0C18. */
typedef struct {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} Unk800F0C10Record;

extern Unk800F0C10Record D_800F0C10[4][3];

/* 0x800948BC: per-stage function pairs, indexed by stage_GetId().
 * stage_ExecInitFunc calls .init; func_8003E6D8 calls .unk4 (e.g. entry 13
 * holds camera_InitBone2 / func_800475A4). */
typedef struct {
    void (*init)(void);
    void (*unk4)(void);
} StageFuncEntry;
extern StageFuncEntry g_stage_init_tbl[];

#endif /* GAME_H */
