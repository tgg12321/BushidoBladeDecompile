/* PsyQ 4.0 LIBAPI PATCH: EnablePAD, DisablePAD and _patch_pad (hand-written
 * asm). .text 0x80078F60..0x80078FF0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

extern void (*jtbl_800A3624)(void);
/* EnablePAD / DisablePAD: bare tail-jump trampolines through the
   jtbl_800A3620 / jtbl_800A3624 function pointers that _patch_pad installs.
   Hand-written asm: GCC 2.7.2 has no sibling calls, so C cannot emit the
   frameless `jr`. */
INCLUDE_ASM("asm/funcs", EnablePAD);
INCLUDE_ASM("asm/funcs", DisablePAD);
INCLUDE_ASM("asm/funcs", _patch_pad);
