/* PsyQ 4.0 LIBGTE SMP_00: LightColor, DpqColorLight, DpqColor3, Intpl,
 * Square12, Square0, AverageZ3, AverageZ4, OuterProduct12, OuterProduct0 and
 * Lzc. .text 0x8007F00C..0x8007F21C, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", LightColor);
INCLUDE_ASM("asm/funcs", DpqColorLight);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", DpqColor3);
INCLUDE_ASM("asm/funcs", Intpl);
/* func_8007F0BC / func_8007F0E4 â€” hand-written GTE sqr leaf wrappers
 * (8007Fxxx cluster, same shape as ApplyRotMatrix / func_8007E8AC,
 * f980d67b): lwc2 x3 -> GTE delay nop -> sqr -> swc2 x3 -> jr with
 * hand-pinned `addu $v0,$a1,$zero` return in the delay slot. The return
 * pin is unreachable from compiled C (local-alloc copy-suggestion scan is
 * ascending, so the arg copy always wins the qty and the return copy
 * materializes at function head â€” measured across volatile/return-local
 * variants). swc2 ops splat-tagged "handwritten instruction". Owner-authorized.
 */
INCLUDE_ASM("asm/funcs", Square12);
INCLUDE_ASM("asm/funcs", Square0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", AverageZ3);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", AverageZ4);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", OuterProduct12);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", OuterProduct0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly
 * body. */
INCLUDE_ASM("asm/funcs", Lzc);
