#ifndef GAME_H
#define GAME_H

/* Game state - gameplay, stage, pad, camera, animation */

#include "common.h"

/* Named globals */
extern u8 SpecialCam;
extern u8 g_disp_enable;
extern u8 g_disp_fade;
extern s16 g_game_mirror_mode;
extern s16 g_game_p2_ctrl;
extern s32 g_game_timer;
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

/* Stage/match control block at 0x800EFAE8 (0x4C bytes). Object model evidence
 * (independent of and predating any byte-chasing): the original binary
 * addresses the whole block through ONE base register -- asm/funcs/func_80054604.s
 * forms $s1 = %hi/%lo(D_800EFAE8) once in its prologue and reaches offsets
 * 0x00/0x02/0x04/0x08/0x0C/0x10/0x14/0x1C/0x1E/0x20/0x2C/0x44/0x46/0x48/0x4A as
 * displacements off that single register (`lw $v1, 0x2C($s1)`, `sh $s5, 0x44($s1)`,
 * ...), and the still-asm per-frame handler asm/funcs/func_8005490C.s addresses
 * the same block the same way. The relocator func_80054FDC bumps the 0x2C..0x40
 * word group together by one base offset. Base+offset addressing of one object,
 * not symbol adjacency. Replaces the splat per-word scalars D_800EFAE8 /
 * D_800EFB0C / D_800EFB14 / D_800EFB18 / D_800EFB1C / D_800EFB20 / D_800EFB24 /
 * D_800EFB28. */
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
    /* 0x24 */ s32 unk24;   /* returned by address from func_8005507C */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;   /* loaded data base (census g_snd_data_buf_base); relocated by func_80054FDC */
    /* 0x30 */ s32 unk30;   /* relocated by func_80054FDC */
    /* 0x34 */ s32 unk34;   /* relocated by func_80054FDC when nonzero */
    /* 0x38 */ s32 unk38;   /* relocated by func_80054FDC when nonzero */
    /* 0x3C */ s32 unk3C;   /* relocated by func_80054FDC when nonzero */
    /* 0x40 */ s32 unk40;   /* relocated by func_80054FDC when nonzero */
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
    /* 0x48 */ s16 unk48;
    /* 0x4A */ s16 unk4A;
} Unk800EFAE8Ctrl;

extern Unk800EFAE8Ctrl D_800EFAE8;

#endif /* GAME_H */
