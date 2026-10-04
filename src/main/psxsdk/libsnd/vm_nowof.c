/* PsyQ LIBSND VM_NOWOF: _SsVmKeyOffNow. .text 0x800871D4..0x800872A4. Not a verbatim LIBSCAN span: BB2
 * links an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived release holds
 * (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan near tier: VM_NOWOF
 * (PsyQ 4.1) matches 50/52 words at 0x800871D4, unique, and the near-verbatim UT_KEYV names
 * _SsVmKeyOffNow -> EXE jal 0x800871D4 (docs/naming/libscan/near_manifest.csv); VM_NOWOF's only XDEF
 * (+0x0, PsyQ 4.0 LIBSND.LIB). */
#include "common.h"
#include "libsnd_i.h"

/* Sony LIBSND `_SsVmKeyOffNow` (probable): mark the current voice's pending
   key-off bit, release the voice slot, and drop the matching key-on bit.
   Body is psyz vm_nowof.c verbatim (with BB2's _svm_voice record layout).
   Symbol map: D_801078D8/DA <- _svm_okof1/_svm_okof2; D_800F1B10/12 <-
   _svm_okon1/_svm_okon2. */
void _SsVmKeyOffNow(s32 mode) {
    s32 bitsUpper;
    s32 bitsLower;
    u16 voice;

    voice = _svm_cur.voice;
    if (voice < 16) {
        bitsLower = 1 << voice;
        bitsUpper = 0;
    } else {
        bitsLower = 0;
        bitsUpper = 1 << (voice - 16);
    }
    _svm_voice[voice].unk1b = 0;
    _svm_voice[voice].unk04 = 0;
    _svm_voice[voice].unk0 = 0;
    _svm_okof1 |= bitsLower;
    _svm_okof2 |= bitsUpper;
    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
}
