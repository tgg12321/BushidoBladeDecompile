/* PsyQ 4.0 LIBGTE REG11: SetFarColor. .text 0x8007EFBC..0x8007EFDC, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>

void SetFarColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    gte_ldfcdir(a0, a1, a2);
}
