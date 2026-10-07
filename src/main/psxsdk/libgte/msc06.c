/* PsyQ 4.0 LIBGTE MSC06: LoadAverage12, LoadAverage0, LoadAverageShort12,
 * LoadAverageShort0, LoadAverageByte and LoadAverageCol. .text
 * 0x8007E1AC..0x8007E43C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007E1AC = LIBGTE MSC06 LoadAverage12 â€” verbatim-linked Sony PsyQ 4.0
 * object. Hand-written GTE asm; disassembler tags every
 * cop2 op "handwritten instruction". No pure-C form (mtc2/lwc2/gpf/gpl/mfc2/
 * swc2 have no C analog). Canonical body (cluster corroboration: siblings
 * MulMatrix2 and ApplyRotMatrix are hand-written asm too). */
INCLUDE_ASM("asm/funcs", LoadAverage12);
/* func_8007E1FC = LIBGTE MSC06 LoadAverage0 â€” verbatim-linked Sony PsyQ 4.0
 * object. Twin of func_8007E1AC differing only in the
 * gpf/gpl sf parameter (0 vs 1). Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", LoadAverage0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", LoadAverageShort12);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", LoadAverageShort0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", LoadAverageByte);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", LoadAverageCol);
