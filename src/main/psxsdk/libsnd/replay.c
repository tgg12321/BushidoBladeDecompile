/* PsyQ 4.0 LIBSND REPLAY: _SsSndReplay. .text 0x80085210..0x80085270, a verbatim LIBSCAN module
 * span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "sound.h"

/* PsyQ 4.0 LIBSND replay: _SsSndReplay — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libsnd/replay.c */
void _SsSndReplay(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    score->unk14 = 1;
    _ss_score[a0][a1].unk98 &= ~8;
}
