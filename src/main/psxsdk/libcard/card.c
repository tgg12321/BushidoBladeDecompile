/* PsyQ 4.0 LIBCARD CARD: _card_clear. .text 0x8007A318..0x8007A350, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"

void _card_clear(s32 a0) {
    _new_card(a0);
    _card_write(a0, 0x3F, 0);
}
