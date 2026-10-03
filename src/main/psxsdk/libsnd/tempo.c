/* PsyQ 4.0 LIBSND TEMPO: _SsSndTempo. .text 0x800856B0..0x800858D0, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* PsyQ 4.0 LIBSND TEMPO: _SsSndTempo — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libsnd/tempo.c (interim
   4.0 build adds the counter<0 early clear-and-return). */
void _SsSndTempo(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];

    score->unkA8--;
    if (score->unkA8 < 0) {
        _ss_score[a0][a1].unk98 &= ~0x40;
        _ss_score[a0][a1].unk98 &= ~0x80;
        return;
    }

    if (score->unk4E > 0) {
        if ((score->unkA8 % score->unk4E) != 0) {
            return;
        }
        if (score->unk94 > score->unkAC || score->unk94 < score->unkAC) {
            score->unk94 = (score->unk94 > score->unkAC) ? score->unk94 - 1 : score->unk94 + 1;
        }
    } else {
        if (score->unk94 > score->unkAC) {
            score->unk94 += score->unk4E;
            if (score->unk94 < score->unkAC) {
                score->unk94 = score->unkAC;
            }
        } else if (score->unk94 < score->unkAC) {
            score->unk94 -= score->unk4E;
            if (score->unk94 > score->unkAC) {
                score->unk94 = score->unkAC;
            }
        }
    }

    score->unk54 = (score->unk50 * score->unk94 * 10) / (VBLANK_MINUS * 60);
    if (score->unk54 <= 0) {
        score->unk54 = 1;
    }
    if ((score->unkA8 == 0) || (score->unk94 == score->unkAC)) {
        _ss_score[a0][a1].unk98 &= ~0x40;
        _ss_score[a0][a1].unk98 &= ~0x80;
    }
}
