#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern u8 D_8008E914[][8];
extern s32 D_8008EA00[][4];
extern s32 func_8001DB58(void);





/* Extern function declarations */


















extern u16 D_80101F32;





extern void player_SetCharId(s32, s32);







extern u8 D_800A3768;
extern u8 D_800A36A8;




extern s16 *func_8004678C(void);










extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);







extern u8 D_800A384C;
extern u8 D_8008E908[][5];
extern u8 D_8008EC24[][5];
extern s32 ratan2(s32, s32);
extern s32 rand(void);
extern void RotMatrixX(s32, s32 *);
extern void RotMatrixY(s32, s32 *);
extern void RotMatrixZ(s32, s32 *);



extern void eff_Init(void);

extern s32 stage_GetDataPtr(void);











extern s16 Judge;

extern u16 D_8008EBA0;



/* P1/P2 round scores and tiebreakers (per-file declarations: owner rulings Q21-Q25,
 * .claude/rules/no-new-park-categories.md aggregate-merge exception). Declared here
 * as single u8s: every measured counting aggregate spelling of func_800340A0
 * misses the shipped code (constant subscripts put element 0 behind a base
 * register; the index-variable and regrouped-condition spellings that avoid that
 * miss its round-result stores or compares; dummy-index and pointer-alias
 * spellings that match are refused/set aside, Q22/Q23). src/code6cac.c:2135
 * declares the same bytes as D_800A3898[2] / D_800A38AA[2] for func_8001CE60,
 * which indexes them by player and does not produce those accesses from single
 * bytes. The mismatch is kept because no single declaration compiles both files
 * with a counting spelling (Q22/Q23 set-asides excluded).
 * Evidence: memory/grind/func_8001CE60/evidence.md (s3),
 * probes/calib_800340A0/ (Q24-AGREEMENT*.txt). */
extern u8 D_800A3898;
extern u8 D_800A3899;
extern u8 D_800A38AA;
extern u8 D_800A38AB;


extern u8 D_800F65F8;




extern u8 D_80106A78;


extern s32 D_80102410;
extern s32 D_80102408;
extern s32 D_80101FC4;
extern s32 D_80101FBC;
extern s16 D_800A3824;
extern s16 D_800A3876;
extern s16 D_800A38A8;
extern void func_8001F860(s16 *arg0, s32 arg1);
extern void func_8002AB08(s32 a0);

extern void func_800288C8(void);
extern s32 func_80029454(void);
extern void func_80031B24(void);
extern s32 D_801020D8;





extern s32 D_801020FC;






/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

INCLUDE_RODATA("asm/rodata", jtbl_8001042C);
void func_80026DA4(void);
INCLUDE_ASM("asm/funcs", func_80026DA4);
