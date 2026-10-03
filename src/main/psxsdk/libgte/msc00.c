/* PsyQ 4.0 LIBGTE MSC00: InitGeom, after the module's two leading data words D_8007E08C. .text
 * 0x8007E08C..0x8007E11C, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#include "include_asm.h"

/* D_8007E08C: the two leading words of LIBGTE module MSC00, whose code starts at InitGeom */
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .include \"asm/funcs/D_8007E08C.s\"\n"
    "    .set reorder\n"
    "    .set at\n"
);

INCLUDE_ASM("asm/funcs", InitGeom);
