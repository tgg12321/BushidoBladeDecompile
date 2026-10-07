/* PsyQ 4.0 LIBGTE SMP_03: RotTransPers3. .text 0x8007F24C..0x8007F2AC, a
 * verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "include_asm.h"

/* Hand-written GTE asm: triple perspective transform (ldv3, RTPT, store
 * SXY0-2 and FLAG), returns SZ3 >> 2. */
INCLUDE_ASM("asm/funcs", RotTransPers3);
