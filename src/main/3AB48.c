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
INCLUDE_ASM("asm/funcs", gpu_SubdivideAndEmitTexturedTriangle);
INCLUDE_ASM("asm/funcs", gpu_EmitOtCutoffFlatLitTexturedQuadsWithOptionalSubdivision);
INCLUDE_ASM("asm/funcs", gpu_SubdivideAndEmitTexturedQuad);
INCLUDE_ASM("asm/funcs", math_MidpointS16x3U8x2);
INCLUDE_ASM("asm/funcs", func_8004C404);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedDepthBiasedFlatLitTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedDepthBiasedFlatLitTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedDepthBiasedGouraudTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedDepthBiasedGouraudTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitOtCutoffFlatLitTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOtCutoffFlatLitTexturedQuads);
INCLUDE_ASM("asm/funcs", func_8004D634);
INCLUDE_ASM("asm/funcs", func_8004D838);
INCLUDE_ASM("asm/funcs", gpu_ProjectStridedNodeVerticesAndDispatchPackets);
INCLUDE_ASM("asm/funcs", func_8004DDB4);

void func_8004E564(void) {}

void func_8004E56C(void) {}

INCLUDE_ASM("asm/funcs", gpu_EmitAveragedTableColorTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitAveragedTableColorTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitTableColorGouraudTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitTableColorGouraudTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedFlatLitTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedFlatLitTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedGouraudTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOrientationAdjustedGouraudTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitOtCutoffRawTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitOtCutoffRawTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitFlatLitTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitFlatLitTexturedQuads);
INCLUDE_ASM("asm/funcs", gpu_EmitColorScaledFlatLitTexturedTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitColorScaledFlatLitTexturedQuads);
INCLUDE_ASM("asm/funcs", func_80050334);
INCLUDE_ASM("asm/funcs", func_80050538);
INCLUDE_ASM("asm/funcs", gpu_EmitBlackFlatTriangles);
INCLUDE_ASM("asm/funcs", gpu_EmitBlackFlatQuads);
INCLUDE_ASM("asm/funcs", func_80050AB8);
INCLUDE_ASM("asm/funcs", func_80050C68);
INCLUDE_ASM("asm/funcs", func_80050E60);
INCLUDE_ASM("asm/funcs", func_80051010);
INCLUDE_ASM("asm/funcs", gpu_EmitPatchedColorTriangleOutlinesAndLoadRecordRgbc);
INCLUDE_ASM("asm/funcs", gpu_EmitPatchedColorQuadOutlinesAndLoadRecordRgbc);
INCLUDE_ASM("asm/funcs", gpu_EmitPatchedColorTriangleOutlinesFromWideRecords);
INCLUDE_ASM("asm/funcs", gpu_EmitPatchedColorQuadOutlinesFromWideRecords);
INCLUDE_ASM("asm/funcs", gpu_WriteAndLinkEmbeddedNccsLitTrianglePackets);
INCLUDE_ASM("asm/funcs", gpu_WriteAndLinkDepthBiasedEmbeddedQuadsAndAuxiliaryPackets);
INCLUDE_ASM("asm/funcs", gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedTrianglePacketsToOt256);
INCLUDE_ASM("asm/funcs", gpu_WriteAndLinkEmbeddedNccsLitDepthBiasedQuadPacketsToOt256);
INCLUDE_ASM("asm/funcs", mem_MoveBlockRoundedUpToWords);
INCLUDE_ASM("asm/funcs", gte_LerpMatrixRowsAndRebuildOrthonormalBasis);
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
INCLUDE_ASM("asm/funcs", gte_TransformSVectorWithSeparateTranslation);
/* Reads the far colour (cfc2 RFC/GFC/BFC) >> 4 into a0[0..2]. Hand-written asm:
 * results in $t0-$t2, a nop in the jr delay slot. */
INCLUDE_ASM("asm/funcs", gte_ReadFarColor);
INCLUDE_ASM("asm/funcs", func_80052C10);
INCLUDE_ASM("asm/funcs", gpu_DepthToOtIndex);
INCLUDE_ASM("asm/funcs", gte_TransformPointDifferenceWithInheritedLastRow);
INCLUDE_ASM("asm/funcs", gte_ReadIR1IR2Sra2);
