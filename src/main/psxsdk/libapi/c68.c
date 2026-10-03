/* PsyQ 4.0 LIBAPI C68 and SENDPAD, kept in one file. C68 is FlushCache, the BIOS A(0x44)
 * trampoline (.text 0x80078FF0..0x80079000); SENDPAD is _SendPAD, _send_pad and the patch
 * stub func_800790A4 (.text 0x80079000..0x800790C0). Both are verbatim LIBSCAN module spans
 * (docs/naming/libscan/matches.json), but asm/funcs/FlushCache.s runs past C68's end into
 * SENDPAD's first function (_SendPAD at 0x80079000 is a mid-function XDEF,
 * docs/naming/libscan/boundary_fixes.md), so the boundary cannot be cut by moving source
 * lines alone; splitting that asm function is a separate decision. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", FlushCache);
INCLUDE_ASM("asm/funcs", _send_pad);
INCLUDE_ASM("asm/funcs", func_800790A4);
