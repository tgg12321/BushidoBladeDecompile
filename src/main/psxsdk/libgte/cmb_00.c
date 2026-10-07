/* PsyQ 4.0 LIBGTE CMB_00: RotTransPers4. .text 0x8007F2DC..0x8007F35C, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007F2DC = LIBGTE CMB_00 RotTransPers4 â€” verbatim-linked Sony PsyQ 4.0
 * object. Triple perspective transform PLUS a 4th vertex
 * via rtps: rtpt on 3 SXY pairs, then rtps on the 4th (*a3). Combined FLAGs
 * OR'd; returns SZ3 >> 2. Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", RotTransPers4);
