/* PsyQ LIBSND VM_NO1: vmNoiseOn. .text 0x80086CF8..0x800871D4. Not a verbatim LIBSCAN span: BB2 links
 * an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived release holds
 * (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan near tier: the
 * near-verbatim UT_KEYV's REL26 at +0x32C names vmNoiseOn in all six builds -> EXE jal 0x80086CF8
 * (docs/naming/libscan/near_manifest.csv), VM_NO1's only XDEF (+0x0, PsyQ 4.0 LIBSND.LIB). */
#include "common.h"
#include "libsnd_i.h"

/* Sony LIBSND `vmNoiseOn` (vm_no1.c): compute the noise voice's L/R volume
   (score channel volume x program volume x tone volume, then three pan
   stages and the optional mono fold), set the SPU noise clock from the
   note, queue the volume shadow registers, claim the voice for noise
   (pitch slot 0xA, noise state 2, every other voice's noise bit cleared),
   and set the key-on / reverb bits before switching the SPU noise voice on.
   Shape follows sotn-decomp src/main/psxsdk/libsnd/vmanager.c vmNoiseOn
   (US main build, matched); BB2's build calls SpuSetNoiseClock /
   SpuSetNoiseVoice where SOTN pokes the SPU registers directly.
   Symbol map: D_80102A78 <- _svm_sreg_buf (s16 view); D_800F65E0 <-
   _svm_sreg_dirty; D_800F1B14/D_800F2B68 <- _svm_orev1/2;
   D_800F1B10/12 <- _svm_okon1/2; D_801078D8/DA <- _svm_okof1/2. */
void vmNoiseOn(u8 vc) {
    struct SeqStruct *score;
    s16 voice;
    s16 bitsLower;
    s16 bitsUpper;
    u32 voll_t, volr_t;
    u32 voll, volr;
    /* SOTN-verbatim (sotn-decomp src/main/psxsdk/libsnd/vmanager.c
       vmNoiseOn): temp holds the tone pan, then the program pan, then the
       voice pan, one per pan stage below. Owner Ruling 8
       (ordinary-c-judge-decidable.md), vmNoiseOn only. */
    u32 temp;
    u32 idx;

    score = &_ss_score[_svm_cur.seq_sep_no & 0xFF]
                       [(_svm_cur.seq_sep_no & 0xFF00) >> 8];

    voll_t = score->unk58 * 0x81;
    volr_t = score->unk5A * 0x81;

    voll_t = (voll_t * _svm_cur.mvol) / 0x7F;
    volr_t = (volr_t * _svm_cur.mvol) / 0x7F;

    voll_t = (voll_t * _svm_cur.tone_vol) / 0x7F;
    volr_t = (volr_t * _svm_cur.tone_vol) / 0x7F;

    temp = _svm_cur.tone_pan;
    if (temp < 0x40) {
        voll = voll_t;
        volr = (volr_t * temp) / 0x3F;
    } else {
        voll = (voll_t * (0x7F - temp)) / 0x3F;
        volr = volr_t;
    }
    temp = _svm_cur.mpan;
    if (temp < 0x40) {
        volr = (volr * temp) / 0x3F;
    } else {
        voll = (voll * (0x7F - temp)) / 0x3F;
    }
    temp = _svm_cur.pan;
    if (temp < 0x40) {
        volr = (temp * volr) / 0x3F;
    } else {
        voll = (voll * (0x7F - temp)) / 0x3F;
    }

    if (_svm_stereo_mono == 1) {
        if (voll < volr) {
            voll = volr;
        } else {
            volr = voll;
        }
    }

    /* FAKE: named-intermediate (no-new-park-categories.md 'Named-intermediate
       declaration order', once-written per ordinary-c-judge-decidable.md
       Ruling 1) - idx is the voice index, bound before the SpuSetNoiseClock
       call so its pseudo is live across that call and global.c seats it in
       call-saved $s0 as the target does (sched1 still places the zero-extend
       after the jal: no dependence ties it to the call). Using vc at each use
       instead puts the index in $a0 and drops $s3 from the frame; idx also at
       the two _svm_voice[] uses differs too (the target zero-extends vc again
       there). */
    idx = vc;
    SpuSetNoiseClock((_svm_cur.note - _svm_cur.tone_center) & 0x3F);

    _svm_sreg_buf[idx * 8 + 0] = voll;
    _svm_sreg_buf[idx * 8 + 1] = volr;
    _svm_sreg_dirty[idx] |= 3;
    if (idx < 0x10) {
        bitsLower = 1 << idx;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (idx - 0x10);
    }
    _svm_voice[vc].unk04 = 0xA;
    for (voice = 0; voice < _SsVmMaxVoice; voice++) {
        _svm_voice[voice].unk1b &= 1;
    }
    _svm_voice[vc].unk1b = 2;

    _svm_okon1 |= bitsLower;
    _svm_okon2 |= bitsUpper;

    _svm_okof1 &= ~_svm_okon1;
    _svm_okof2 &= ~_svm_okon2;

    if (_svm_cur.tone_mode & 4) {
        D_800F1B14 |= bitsLower;
        D_800F2B68 |= bitsUpper;
    } else {
        D_800F1B14 &= ~bitsLower;
        D_800F2B68 &= ~bitsUpper;
    }

    SpuSetNoiseVoice(1, ((bitsUpper & 0xFF) << 16) | bitsLower);
}
