/* PsyQ LIBSND VM_INIT: _SsVmInit. .text 0x80086818..0x80086B38 (VM_N2P follows). Not a verbatim
 * LIBSCAN span: BB2 links an interim LIBSND build, between PsyQ 4.0 and 4.1, that no archived release
 * holds (memory/closer/libsnd-hunt-report.md). Module start (owner ruling Q109), libscan xref tier:
 * the verbatim SSINIT module's REL26 at +0x80 names _SsVmInit -> EXE jal 0x80086818
 * (docs/naming/libscan/near_manifest.csv), VM_INIT's only XDEF (+0x0, PsyQ 4.0 LIBSND.LIB). */
#include "common.h"
#include "libsnd_i.h"

/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */
extern s32 D_800F19D0[2];

void _SsVmInit(s32 a0) {
    SpuVoiceAttr attr;
    u16 i;

    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(0x20, D_800F19D0);

    i = 0;
    do {
        _svm_sreg_buf[i] = 0;
        i++;
    } while (i < 0xC0);
    i = 0;
    do {
        _svm_sreg_dirty[i] = 0;
        i++;
    } while (i < 0x18);
    _svm_vab_count = 0;
    i = 0;
    do {
        _svm_vab_used[i] = 0;
        i++;
    } while (i < 0x10);

    {
        /* FAKE: masked holds (u8)a0 as a u16; read from a0 twice the andi lands in v0 and the byte store takes s1 (score 3). */
        u16 masked = (u8)a0;
        if (masked >= 0x18) {
            _SsVmMaxVoice = 0x18;
        } else {
            _SsVmMaxVoice = masked;
        }
    }

    attr.mask = 0x60093;
    i = 0;
    attr.pitch = 0x1000;
    attr.addr = 0x1000;
    attr.adsr1 = 0x80FF;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.adsr2 = 0x4000;

    if (_SsVmMaxVoice != 0) {
        do {
            _svm_voice[i].unk2 = 0x18;
            _svm_voice[i].unke = -1;
            _svm_voice[i].unk0 = 0xFF;
            _svm_voice[i].unk1b = 0;
            _svm_voice[i].unk04 = 0;
            _svm_voice[i].unk6 = 0;
            _svm_voice[i].unk10 = 0;
            _svm_voice[i].prog = 0;
            _svm_voice[i].tone = 0xFF;
            _svm_voice[i].unk8 = 0;
            _svm_voice[i].unkc = 0;
            _svm_voice[i].unka = 0x40;
            _svm_voice[i].auto_vol = 0;
            _svm_voice[i].unk1e = 0;
            _svm_voice[i].unk20 = 0;
            _svm_voice[i].unk22 = 0;
            _svm_voice[i].auto_pan = 0;
            _svm_voice[i].unk2a = 0;
            _svm_voice[i].unk2c = 0;
            _svm_voice[i].unk2e = 0;
            _svm_voice[i].start_pan = 0;
            _svm_voice[i].start_vol = 0;
            attr.voice = 1 << i;
            func_8008B488(&attr);
            _svm_cur.voice = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < _SsVmMaxVoice);
    }

    _svm_rattr.depth.left = 0x3FFF;
    _svm_rattr.depth.right = 0x3FFF;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    D_800F1B14 = 0;
    D_800F2B68 = 0;
    _svm_rattr.mask = 0;
    _svm_rattr.mode = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
