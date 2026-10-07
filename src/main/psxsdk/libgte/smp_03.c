/* PsyQ 4.0 LIBGTE SMP_03: RotTransPers3. .text 0x8007F24C..0x8007F2AC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* func_8007F24C = LIBGTE SMP_03 RotTransPers3 â€” verbatim-linked Sony PsyQ 4.0
 * object. Triple perspective transform: lwc2 3 SXY0/SXY1/SXY2
 * pairs from *a0/*a1/*a2 -> rtpt -> swc2 SZ/SXY0/SXY1/SXY2 to *a3 & sp-loaded
 * pointers -> cfc2 FLAG to *(sp+0x1C) -> return mfc2 SZ3 >> 2 (folded into jr
 * delay slot). Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", RotTransPers3);
