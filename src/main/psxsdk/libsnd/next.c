/* PsyQ 4.0 LIBSND NEXT: _SsSndNextSep. .text 0x80085114..0x80085210, a verbatim
 * LIBSCAN module span (docs/naming/libscan/matches.json), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* PsyQ LIBSND next.c: _SsSndNextSep — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libsnd/next.c (mixed
   score-pointer / full-index accesses are the original's spelling) */
void _SsSndNextSep(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    score->unk20 = 1;
    score->unk21 = 0;
    _ss_score[a0][a1].unk98 &= ~0x100;
    _ss_score[a0][a1].unk98 &= ~8;
    _ss_score[a0][a1].unk98 &= ~2;
    _ss_score[a0][a1].unk98 &= ~4;
    _ss_score[a0][a1].unk98 &= ~0x200;
    score->read_pos = score->next_sep_pos;
    score->unk14 = 1;
    _ss_score[a0][a1].unk98 |= 1;
}
