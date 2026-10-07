/* PsyQ 4.0 LIBAPI SENDPAD: _SendPAD, _send_pad and func_800790A4 (data-as-code:
 * the 4 patch words _send_pad writes into the BIOS pad code at B0[0x5B] +0x3D8
 * and +0x4E0, then 3 pad words). .text 0x80079000..0x800790C0, a verbatim
 * LIBSCAN module span, Q106 D3, Q108. Sony hand-written asm (sendpad.s); all
 * three bodies are canonical asm (_SendPAD by Q112). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", _SendPAD);
INCLUDE_ASM("asm/funcs", _send_pad);
INCLUDE_ASM("asm/funcs", func_800790A4);
