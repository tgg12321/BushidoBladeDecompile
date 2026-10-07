/* PsyQ 4.0 LIBGTE FGO_03: RotMatrixZYX. .text 0x8007F5EC..0x8007F87C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Original LIBGTE assembly. 3-axis Euler rotation: reads X/Y/Z angles from
 * arg0 (s16[3]), looks up cos/sin for each, applies the rotation chain to
 * arg1. */
INCLUDE_ASM("asm/funcs", RotMatrixZYX);
