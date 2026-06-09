/* Rodata sub-TU split out from the original ings.c family — owns the 5 debug
 * format strings + build-date string that all 5 ings.c functions reference.
 * Re-attributed from asm/data/800.rodata_pre.s 2026-06-09 (rodata-cleanup
 * project, docs/rodata-cleanup-project.md).
 *
 * The split into a sub-TU is required because the main ings.c emits ~96 bytes
 * of its own .rodata via file-scope __asm__("glabel ...") blocks for
 * func_800164AC and func_800164F8 (canonical-asm functions whose addresses
 * are fixed at 0x800164AC / 0x800164F8). Putting these strings in ings.c
 * itself would force those asm-emitted "functions" to shift to the front of
 * rodata too, breaking their fixed addresses. The sub-TU isolates the
 * strings so only they land at the 0x80010000 slot.
 *
 * Evidence (§8.1): grep across src/ shows these symbols (and their friendly
 * aliases g_str_overflow / g_str_eff_init / g_str_limit / g_str_prim_overflow)
 * are referenced ONLY from ings.c. The build-date string at D_8001004C is
 * referenced indirectly via the .data table D_800A30DC.
 *
 * Bracket-sized to match the asm/data block's exact byte content including
 * alignment padding (104 bytes total). */
#include "common.h"

const char D_80010000[12] = "OVER FLOW\n";
const char D_8001000C[28] = "eff_init:%08x size:%08x\n";
const char D_80010028[12] = "LIMIT:%08x\n";
const char D_80010034[24] = "common prim over flow\n";
const char D_8001004C[28] = "Fri Aug  7 22:26:32 1998\n";
