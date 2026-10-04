/* PsyQ LIBSND VM_ALOC2: _SsVmDoAllocate. .text 0x800861BC..0x800863CC (VM_DOFF follows). Not a
 * verbatim LIBSCAN span: BB2 links an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived
 * release holds (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan near
 * tier: the near-verbatim UT_KEYV's REL26 at +0x310 names _SsVmDoAllocate in all six builds -> EXE jal
 * 0x800861BC (docs/naming/libscan/near_manifest.csv), VM_ALOC2's only XDEF (+0x0, PsyQ 4.0
 * LIBSND.LIB). */
#include "common.h"
#include "libsnd_i.h"

/* Sony LIBSND `_SsVmDoAllocate` (psyz vm_aloc2.c analog): set up the
   allocated voice's SPU shadow registers (start address, ADSR) and mark the
   voice's dirty bits. BB2 deltas vs psyz: _svm_voice stride 54, ADSR indexed
   by voiceOffset through the flat s16 shadow view D_80102A78[]. */
static inline void vmSetStartAddr(u16 addr) {
    _svm_sreg_buf[_svm_cur.voiceOffset + 3] = addr;
    _svm_sreg_dirty[_svm_cur.voice] |= 8;
}

void _SsVmDoAllocate(void) {
    int i;
    int progIdx;

    _svm_cur.voiceOffset = _svm_cur.voice * 8;
    _svm_cur.field_0x1e = _svm_cur.field_7_fake_program * 16 + _svm_cur.tone;
    _svm_voice[_svm_cur.voice].unk6 = 0x7FFF;
    for (i = 0; i < 16; i++) {
        _svm_envx_hist[i] &= ~(1 << _svm_cur.voice);
    }
    if ((_svm_cur.tone_vag_idx & 1) > 0) {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved2);
    } else {
        progIdx = (_svm_cur.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)_svm_pg)[progIdx].reserved3);
    }
    _svm_sreg_buf[_svm_cur.voiceOffset + 4] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr1;
    _svm_sreg_buf[_svm_cur.voiceOffset + 5] =
        _svm_tn[_svm_cur.field_7_fake_program * 16 + _svm_cur.tone].adsr2 + _svm_damper;
    _svm_sreg_dirty[_svm_cur.voice] |= 0x30;
}
