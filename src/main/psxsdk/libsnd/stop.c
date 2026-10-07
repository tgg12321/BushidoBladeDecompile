/* PsyQ 4.0 LIBSND SSSTOP (the Jun-06-1997 4.0 build): _SsSndStop, SsSeqStop and
 * SsSepStop. .text 0x80085270..0x80085448, a bit-verbatim module span
 * (memory/closer/libsnd-hunt-report.md "New verbatim result";
 * docs/naming/libscan/ambiguous_resolutions.md), Q106 D3. */
#include "common.h"
#include "libsnd_i.h"

/* PsyQ 4.0 LIBSND SSSTOP: _SsSndStop — verbatim-linked Sony object
   (bit-verbatim vs the Jun-06-1997 4.0 build, 118 words);
   C ref: sotn-decomp src/main/psxsdk/libsnd/stop.c (interim 4.0 build adds
   the ~0x400 flag clear + NotifyChannel/ResetCounter pair). */
void _SsSndStop(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    s32 i;

    _ss_score[a0][a1].unk98 &= ~1;
    _ss_score[a0][a1].unk98 &= ~2;
    _ss_score[a0][a1].unk98 &= ~8;
    _ss_score[a0][a1].unk98 &= ~0x400;
    _ss_score[a0][a1].unk98 |= 4;

    _SsVmSeqKeyOff((s16)(a0 | (a1 << 8)));
    _SsVmDamperOff();

    score->unk14 = 0;
    score->unk88 = 0;
    score->unk1C = 0;
    score->unk18 = 0;
    score->unk19 = 0;
    score->unk1E = 0;
    score->unk1A = 0;
    score->unk1B = 0;
    score->unk1F = 0;
    score->channel = 0;
    score->unk21 = 0;
    score->unk1C = 0;
    score->unk1D = 0;
    score->unk15 = 0;
    score->unk16 = 0;

    score->delta_value = score->unk84;
    score->unk94 = score->unk8C;
    score->unk54 = score->unk56;
    score->read_pos = score->next_sep_pos;
    score->loop_pos = score->next_sep_pos;

    for (i = 0; i < 16; i++) {
        score->programs[i] = i;
        score->panpot[i] = 0x40;
        score->vol[i] = 0x7F;
    }
    score->unk5C = 0x7F;
    score->unk5E = 0x7F;
}

void SsSeqStop(s16 a0) { _SsSndStop(a0, 0); }

void SsSepStop(s16 a0, s16 a1) { _SsSndStop(a0, a1); }
