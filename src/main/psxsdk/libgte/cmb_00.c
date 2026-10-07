/* PsyQ 4.0 LIBGTE CMB_00: RotTransPers4. .text 0x8007F2DC..0x8007F35C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* RotTransPers4: rtpt on three vertices, then rtps on the 4th (*a3); FLAGs
 * OR'd, returns SZ3 >> 2. Hand-written GTE asm. */
INCLUDE_ASM("asm/funcs", RotTransPers4);
