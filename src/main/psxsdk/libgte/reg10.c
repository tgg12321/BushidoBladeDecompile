/* PsyQ 4.0 LIBGTE REG10: SetBackColor. .text 0x8007EF9C..0x8007EFBC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>

void SetBackColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile ("ctc2 %0, $13" :: "r"(a0)); /* ctc2 $a0, $13 */
    __asm__ volatile ("ctc2 %0, $14" :: "r"(a1)); /* ctc2 $a1, $14 */
    __asm__ volatile ("ctc2 %0, $15" :: "r"(a2)); /* ctc2 $a2, $15 */
}
