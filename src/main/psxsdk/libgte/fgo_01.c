/* PsyQ 4.0 LIBGTE FGO_01: RotMatrix. .text 0x8007F35C..0x8007F5EC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* motutil_GetWalkDir: hand-coded asm in original PSY-Q source.
 * Cluster sibling of func_8007F5EC (jaccard=0.68): same 3-axis Euler
 * rotation skeleton, different rotation-matrix coefficient signs
 * and different output-byte layout. 163 insns, zero spills, three
 * INT_MIN-guard idioms in succession. Scanner STRONG 3/5 (S2+S3+S5);
 * manual review confirmed hand-coded. Same cluster authorization. */
INCLUDE_ASM("asm/funcs", RotMatrix);
