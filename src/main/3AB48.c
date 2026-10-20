/* 72 game functions: GTE and math helpers (gte_SetRotTransMatrix,
 * math_SquareRoot0, math_LerpMatrix3x3, ...). .text 0x8004A348 (ROM 0x3AB48).
 * Start boundary: G8 (the Q89 cut). Ends at gte_ReadIR1IR2Sra2: the collision
 * code after it is 43500.c (seam, owner ruling 2026-10-17). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "bb2.h"
#include "include_asm.h"
#include <psxsdk/libgte.h>

extern void func_80052C10(void);

INCLUDE_ASM("asm/funcs", math_RotMatrixZYX);
INCLUDE_ASM("asm/funcs", func_8004A4E0);
INCLUDE_ASM("asm/funcs", gte_ProjectSVectorTriples);
INCLUDE_ASM("asm/funcs", gte_ProjectPackedVertices);

void func_8004A938(void) {}

INCLUDE_ASM("asm/funcs", func_8004A940);
INCLUDE_ASM("asm/funcs", func_8004BB68);
INCLUDE_ASM("asm/funcs", func_8004BCC0);
INCLUDE_ASM("asm/funcs", func_8004C1F4);
INCLUDE_ASM("asm/funcs", math_MidpointS16x3U8x2);
INCLUDE_ASM("asm/funcs", func_8004C404);
INCLUDE_ASM("asm/funcs", func_8004C994);
INCLUDE_ASM("asm/funcs", func_8004CB8C);
INCLUDE_ASM("asm/funcs", func_8004CDB0);
INCLUDE_ASM("asm/funcs", func_8004CFE0);
INCLUDE_ASM("asm/funcs", func_8004D244);
INCLUDE_ASM("asm/funcs", func_8004D424);
INCLUDE_ASM("asm/funcs", func_8004D634);
INCLUDE_ASM("asm/funcs", func_8004D838);
INCLUDE_ASM("asm/funcs", func_8004DA74);
INCLUDE_ASM("asm/funcs", func_8004DDB4);

void func_8004E564(void) {}

void func_8004E56C(void) {}

INCLUDE_ASM("asm/funcs", func_8004E574);
INCLUDE_ASM("asm/funcs", func_8004E7E4);
INCLUDE_ASM("asm/funcs", func_8004EAC8);
INCLUDE_ASM("asm/funcs", func_8004ECC8);
INCLUDE_ASM("asm/funcs", func_8004EF10);
INCLUDE_ASM("asm/funcs", func_8004F0FC);
INCLUDE_ASM("asm/funcs", func_8004F314);
INCLUDE_ASM("asm/funcs", func_8004F53C);
INCLUDE_ASM("asm/funcs", func_8004F798);
INCLUDE_ASM("asm/funcs", func_8004F970);
INCLUDE_ASM("asm/funcs", func_8004FB74);
INCLUDE_ASM("asm/funcs", func_8004FD40);
INCLUDE_ASM("asm/funcs", func_8004FF40);
INCLUDE_ASM("asm/funcs", func_80050120);
INCLUDE_ASM("asm/funcs", func_80050334);
INCLUDE_ASM("asm/funcs", func_80050538);
INCLUDE_ASM("asm/funcs", func_80050774);
INCLUDE_ASM("asm/funcs", func_80050908);
INCLUDE_ASM("asm/funcs", func_80050AB8);
INCLUDE_ASM("asm/funcs", func_80050C68);
INCLUDE_ASM("asm/funcs", func_80050E60);
INCLUDE_ASM("asm/funcs", func_80051010);
INCLUDE_ASM("asm/funcs", func_80051208);
INCLUDE_ASM("asm/funcs", func_800513B0);
INCLUDE_ASM("asm/funcs", func_800515AC);
INCLUDE_ASM("asm/funcs", func_80051754);
INCLUDE_ASM("asm/funcs", func_80051944);
INCLUDE_ASM("asm/funcs", func_80051B04);
INCLUDE_ASM("asm/funcs", func_80051D08);
INCLUDE_ASM("asm/funcs", func_80051ED4);
INCLUDE_ASM("asm/funcs", func_800520B8);
INCLUDE_ASM("asm/funcs", func_800523E0);
INCLUDE_ASM("asm/funcs", gte_NormalizeIRVector);
/* Hand-written asm in the PsyQ source: trapping add/addi/sub ops GCC cannot
 * emit, a dead delay-slot init, three separate jr $ra exits; GTE LZCS/LZCR
 * leading-zero count. */
INCLUDE_ASM("asm/funcs", math_SquareRoot0);
/* GTE SQR of IR1-3, sum MAC1-3, then a frameless tail jump to math_SquareRoot0.
 * Hand-written asm: trapping add ops, mfc2 results in $t0-$t2, hand-scheduled
 * GTE nops. */
INCLUDE_ASM("asm/funcs", math_Length3D);
/* GTE SQR (squared vector length) leaf: mtc2 IR1-3, sqr, sum MAC1-3.
 * Hand-written asm: mfc2 results in $t0-$t2. */
INCLUDE_ASM("asm/funcs", gte_SumSquares3);
INCLUDE_ASM("asm/funcs", math_LerpSVector);
INCLUDE_ASM("asm/funcs", math_LerpMatrix3x3);
/* Loads the rotation matrix at *a0 (five lw, zero translation), reads the s16
 * 3x3 matrix at *a1 (five lw), runs three MVMVA cycles, one per column of *a1,
 * and stores the 3x3 product to *a2 with nine sh. Hand-scheduled asm: each
 * column's packing interleaved with the previous cycle's GTE latency, last
 * store in the jr delay slot. */
INCLUDE_ASM("asm/funcs", gte_MulMatrix0ClearTrans);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIR);
INCLUDE_ASM("asm/funcs", gte_SetMatrixRotTransIRVec);
INCLUDE_ASM("asm/funcs", gte_SetRotTransMatrix);
/* Loads a packed 3x3 rotation matrix from *a0 into CR0-CR4 and zeroes the
 * translation CR5-CR7 (last ctc2 in the jr delay slot). Hand-written GTE asm.
 */
INCLUDE_ASM("asm/funcs", gte_SetRotMatrixClearTrans);
INCLUDE_ASM("asm/funcs", func_80052B7C);
/* Reads the far colour (cfc2 RFC/GFC/BFC) >> 4 into a0[0..2]. Hand-written asm:
 * results in $t0-$t2, a nop in the jr delay slot. */
INCLUDE_ASM("asm/funcs", gte_ReadFarColor);
INCLUDE_ASM("asm/funcs", func_80052C10);
INCLUDE_ASM("asm/funcs", gpu_DepthToOtIndex);
INCLUDE_ASM("asm/funcs", func_80052C4C);
INCLUDE_ASM("asm/funcs", gte_ReadIR1IR2Sra2);
