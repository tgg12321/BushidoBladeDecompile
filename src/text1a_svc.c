/* text1a_svc.c -- save_vc_ctrl's own translation unit, linked by bb2.ld between
 * text1a_pre_tu2 and text1a_post (the slot the raw asm object held since
 * 4fd7a997a). One function per TU, so the -G8 TARGET_FILE_SWITCHING float that
 * forced the raw-asm object cannot reorder anything here.
 *
 * INCOMPLETE (asm-until-matched): de-authorized 2026-10-01 by owner ruling
 * (docs/grind/decisions.md, "inline-asm audit"); the frame shape once cited as
 * hand-coded evidence is emitted by cc1 in COMPLETED-C functions (func_8001F938).
 */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"

INCLUDE_ASM("asm/funcs", save_vc_ctrl);
