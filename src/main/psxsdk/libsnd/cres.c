/* PsyQ LIBSND CRES: _SsSndCrescendo. .text 0x800841E0..0x80084500. BB2 links
 * an interim LIBSND build (between PsyQ 4.0 and 4.1); module start (Q109) from
 * the verbatim SSCALL module's REL26 naming _SsSndCrescendo -> jal 0x800841E0
 * (docs/naming/libscan/near_manifest.csv). */
#include "common.h"
#include "libsnd_i.h"

void _SsSndCrescendo(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    u16 voll, volr;

    if (--(score->unkA0) < 0) {
        _ss_score[a0][a1].unk98 &= ~0x10;
    } else if (score->unk4C > 0) {
        if ((score->unkA0 % score->unk4C) == 0) {
            score->unk4A = score->unk4A - 1;
            if (score->unk4A >= 0) {
                _SsVmGetSeqVol(
                    (s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((voll + 1) <= (voll + score->unk4A))
                    func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll + 1),
                                  (u16)(volr + 1), 1);
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                _ss_score[a0][a1].unk98 &= ~0x10;
            }
            if ((score->unkA0 == 0) || (score->unk4A <= 0))
                _ss_score[a0][a1].unk98 &= ~0x10;
        }
    } else if (score->unk4C < 0) {
        score->unk4A = score->unk4A + score->unk4C;
        if (score->unk4A >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if (((voll - score->unk4C) >= 0x7F) &&
                ((volr - score->unk4C) >= 0x7F)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
                _ss_score[a0][a1].unk98 &= ~0x10;
            }
            if (((score->unk9C - score->unkA0) * -score->unk4C) < score->unk48)
                func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - score->unk4C),
                              (u16)(volr - score->unk4C), 1);
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0x7F, 0x7F, 1);
            _ss_score[a0][a1].unk98 &= ~0x10;
        }
        if ((score->unkA0 == 0) || (score->unk4A <= 0))
            _ss_score[a0][a1].unk98 &= ~0x10;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), &score->unk5C, &score->unk5E);
}
