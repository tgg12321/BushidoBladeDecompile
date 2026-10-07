/* PsyQ 4.0 LIBGTE MSC06: LoadAverage12, LoadAverage0, LoadAverageShort12,
 * LoadAverageShort0, LoadAverageByte and LoadAverageCol. .text
 * 0x8007E1AC..0x8007E43C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Original LIBGTE assembly: hand-written GTE (mtc2/lwc2/gpf/gpl/mfc2/swc2). */
INCLUDE_ASM("asm/funcs", LoadAverage12);
/* Twin of LoadAverage12 differing only in the gpf/gpl sf bit (0 vs 1). */
INCLUDE_ASM("asm/funcs", LoadAverage0);
/* Original LIBGTE assembly. */
INCLUDE_ASM("asm/funcs", LoadAverageShort12);
/* Original LIBGTE assembly. */
INCLUDE_ASM("asm/funcs", LoadAverageShort0);
/* Original LIBGTE assembly. */
INCLUDE_ASM("asm/funcs", LoadAverageByte);
/* Original LIBGTE assembly. */
INCLUDE_ASM("asm/funcs", LoadAverageCol);
