/* PsyQ 4.0 LIBGTE REG11: SetFarColor. .text 0x8007EFBC..0x8007EFDC, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

void SetFarColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile ("ctc2 %0, $21" :: "r"(a0));  /* ctc2 $a0, $21 */
    __asm__ volatile ("ctc2 %0, $22" :: "r"(a1));  /* ctc2 $a1, $22 */
    __asm__ volatile ("ctc2 %0, $23" :: "r"(a2));  /* ctc2 $a2, $23 */
}
