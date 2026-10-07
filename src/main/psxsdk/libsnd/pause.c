/* PsyQ 4.0 LIBSND PAUSE: _SsSndPause. .text 0x800848AC..0x80084948, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

void _SsSndPause(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    score->unk14 = 0;
    _ss_score[a0][a1].unk98 &= ~2;
}
