/* PsyQ LIBSND DECRES: _SsSndDecrescendo. .text 0x80084500..0x800848AC, from an
 * interim LIBSND build (between PsyQ 4.0 and 4.1). Module start (Q109): the
 * verbatim SSCALL module's REL26 names _SsSndDecrescendo, DECRES's only XDEF
 * (docs/naming/libscan/near_manifest.csv). */
#include "common.h"
#include "libsnd_i.h"

void _SsSndDecrescendo(s16 a0, s16 a1) {
    struct SeqStruct *score = &_ss_score[a0][a1];
    u16 voll, volr;

    if (--(score->unkA0) < 0) {
        _ss_score[a0][a1].unk98 &= ~0x20;
    } else if (score->unk4C > 0) {
        if ((score->unkA0 % score->unk4C) == 0) {
            score->unk4A = score->unk4A - 1;
            if (score->unk4A >= 0) {
                _SsVmGetSeqVol(
                    (s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
                if ((((u16)voll - 1) >= ((u16)voll - score->unk4A)) ||
                    (((u16)volr - 1) >= ((u16)volr - score->unk4A))) {
                    if ((voll == 0) || (volr == 0)) {
                        _ss_score[a0][a1].unk98 &= ~0x20;
                    } else {
                        func_80087770((s16)(a0 | (a1 << 8)), (u16)(voll - 1),
                                      (u16)(volr - 1), 1);
                    }
                }
            } else {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                _ss_score[a0][a1].unk98 &= ~0x20;
            }
            if ((score->unkA0 == 0) || (score->unk4A <= 0))
                _ss_score[a0][a1].unk98 &= ~0x20;
        }
    } else if (score->unk4C < 0) {
        score->unk4A = score->unk4A + score->unk4C;
        if (score->unk4A >= 0) {
            _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), (s16 *)&voll, (s16 *)&volr);
            if ((((u16)voll + score->unk4C) <= 0) &&
                (((u16)volr + score->unk4C) <= 0)) {
                func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
                _ss_score[a0][a1].unk98 &= ~0x20;
            }
            if (((score->unk9C - score->unkA0) * -score->unk4C) <
                score->unk48) {
                if ((voll == 0) || (volr == 0)) {
                    _ss_score[a0][a1].unk98 &= ~0x20;
                } else {
                    func_80087770(
                        (s16)(a0 | (a1 << 8)), (u16)(voll + score->unk4C),
                        (u16)(volr + score->unk4C), 1);
                }
            }
        } else {
            func_80087770((s16)(a0 | (a1 << 8)), 0, 0, 1);
            _ss_score[a0][a1].unk98 &= ~0x20;
        }
        if ((score->unkA0 == 0) || (score->unk4A <= 0))
            _ss_score[a0][a1].unk98 &= ~0x20;
    }
    _SsVmGetSeqVol((s16)(a0 | (a1 << 8)), &score->unk5C, &score->unk5E);
}
