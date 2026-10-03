/* PsyQ 4.0 LIBGTE FGO_03: RotMatrixZYX. .text 0x8007F5EC..0x8007F87C, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007F5EC: hand-coded asm in original PSY-Q source.
 * 3-axis Euler rotation: reads X/Y/Z angles from arg0 (s16[3]),
 * looks up cos/sin for each, applies a 9-element 3D rotation chain
 * to arg1[0..0x10]. Manual signal review (scanner 3/5
 * but verified hand-coded): three INT_MIN-guard idioms, 163 insns
 * with zero spills despite 10+ live registers, hand-scheduled
 * multu/mflo where a 3-cycle gap holds a bgez+andi pair in the
 * pipeline stall window. Same cluster authorization scope as
 * func_8007F87C (RotMatrixX). */
INCLUDE_ASM("asm/funcs", RotMatrixZYX);
