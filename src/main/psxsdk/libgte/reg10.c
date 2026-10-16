/* PsyQ 4.0 LIBGTE REG10: SetBackColor. .text 0x8007EF9C..0x8007EFBC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>

void SetBackColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    gte_ldbkdir(a0, a1, a2);
}
