/* PsyQ 4.0 LIBAPI PATCH: EnablePAD, DisablePAD and _patch_pad (hand-written asm). .text
 * 0x80078F60..0x80078FF0, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

extern void (*jtbl_800A3624)(void);
/* func_80078F60 / func_80078F74: 5-insn bare tail-jump trampolines
   (lui/lw/nop/jr/nop) through the jtbl_800A3620 / jtbl_800A3624 function
   pointers that the Pad-init wrapper func_80078F88 installs at runtime. GCC
   2.7.2 has no MIPS sibling-call optimization, so no pure-C `(*fp)()` form
   emits a frameless `jr $t1` (it always builds a stack frame + jalr + jr $ra).
   Hand-coded canonical asm (owner-authorized). */
INCLUDE_ASM("asm/funcs", EnablePAD);
INCLUDE_ASM("asm/funcs", DisablePAD);
INCLUDE_ASM("asm/funcs", _patch_pad);
