/* PsyQ 4.0 LIBGTE SMP_00: LightColor, DpqColorLight, DpqColor3, Intpl,
 * Square12, Square0, AverageZ3, AverageZ4, OuterProduct12, OuterProduct0 and
 * Lzc. .text 0x8007F00C..0x8007F21C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", LightColor);
INCLUDE_ASM("asm/funcs", DpqColorLight);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", DpqColor3);
INCLUDE_ASM("asm/funcs", Intpl);
/* Square12 / Square0: hand-written GTE sqr leaf wrappers (lwc2 x3, sqr,
 * swc2 x3) returning $a1 from the jr delay slot, which compiled C cannot
 * place there. */
INCLUDE_ASM("asm/funcs", Square12);
INCLUDE_ASM("asm/funcs", Square0);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", AverageZ3);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", AverageZ4);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", OuterProduct12);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", OuterProduct0);
/* Original LIBGTE assembly (fixed-register ABI). */
INCLUDE_ASM("asm/funcs", Lzc);
