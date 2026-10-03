/* LIBSND code between verbatim modules: _SsVmFlush and _SsVmInit. .text 0x800863DC..0x80086B38: an
 * unidentified region between verbatim LIBSCAN modules (docs/naming/libscan/matches.json;
 * memory/closer/libsnd-hunt-report.md lists the probable newer-build modules), one file per gap
 * (Q106 D3), named by its ROM offset. */
#include "common.h"
#include "sound.h"

/* Declarations from the file this module was split from (src/main/psxsdk/libspu/spu.c, ex main.c). */
extern s16 _svm_damper;
extern s16 _svm_stereo_mono;
typedef struct {
    s16 left, right;
} SpuVolume;
extern u8 _SsVmMaxVoice;
extern u8 _svm_vab_used[];
extern s32 _svm_rattr;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
extern s32 _svm_rattr_plus_0x4;
extern s16 _svm_sreg_buf[];
extern u8 _svm_sreg_dirty[];
extern s32 _svm_envx_hist[];

/* PsyQ LIBSPU SpuVoiceAttr (libspu.h), the argument of func_8008B488
   (SpuSetVoiceAttr shape). */
typedef struct {
    /* 0x00 */ u32 voice;
    /* 0x04 */ u32 mask;
    /* 0x08 */ SpuVolume volume;
    /* 0x0C */ SpuVolume volmode;
    /* 0x10 */ SpuVolume volumex;
    /* 0x14 */ u16 pitch;
    /* 0x16 */ u16 note;
    /* 0x18 */ u16 sample_note;
    /* 0x1A */ s16 envx;
    /* 0x1C */ u32 addr;
    /* 0x20 */ u32 loop_addr;
    /* 0x24 */ s32 a_mode;
    /* 0x28 */ s32 s_mode;
    /* 0x2C */ s32 r_mode;
    /* 0x30 */ u16 ar;
    /* 0x32 */ u16 dr;
    /* 0x34 */ u16 sr;
    /* 0x36 */ u16 rr;
    /* 0x38 */ u16 sl;
    /* 0x3A */ u16 adsr1;
    /* 0x3C */ u16 adsr2;
} SpuVoiceAttr;

extern s32 D_80103604;              /* psyz _svm_envx_ptr */
extern u8  _svm_auto_kof_mode;
extern void (*D_80102BF8)(s32);     /* psyz _autovol */
extern void (*D_801027E8)(s32);     /* psyz _autopan */
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 D_800F1B14;              /* psyz _svm_orev1 */
extern u16 D_800F2B68;              /* psyz _svm_orev2 */
extern void SpuGetVoiceEnvelope(s32, u16 *);
extern void SpuSetNoiseVoice(s32, s32);
extern void SpuSetKey(s32, u32);
extern void SpuSetReverbVoice(s32, s32);

/* Sony LIBSND `_SsVmFlush` (VM_F): sample every voice's envelope into
   _svm_voice[].unk6 and the 16-slot silence history ring, release the
   noise state of voices silent in history slots 0-14, run the auto-volume / auto-pan
   callbacks, flush the dirty shadow registers through SpuSetVoiceAttr
   (func_8008B488), then write key-off / key-on / reverb masks and clear the
   pending key masks. Shape follows sotn-decomp
   src/main/psxsdk/libsnd/vmanager.c SpuVmFlush (US main build, matched);
   BB2's build reads the envelope with SpuGetVoiceEnvelope and writes the SPU
   through LIBSPU calls where SOTN pokes the registers directly. */
void _SsVmFlush(void)
{
    s32 i;
    u32 env_mask;
    SpuVoiceAttr attr;

    D_80103604 = (D_80103604 + 1) & 0xF;
    _svm_envx_hist[D_80103604] = 0;

    for (i = 0; i < _SsVmMaxVoice; i++) {
        SpuGetVoiceEnvelope(i, &_svm_voice[i].unk6);
        if (_svm_voice[i].unk6 == 0) {
            _svm_envx_hist[D_80103604] |= 1 << i;
        }
    }
    if (_svm_auto_kof_mode == 0) {
        env_mask = 0xFFFFFFFF;
        for (i = 0; i < 0xF; i++) {
            env_mask &= _svm_envx_hist[i];
        }
        for (i = 0; i < _SsVmMaxVoice; i++) {
            if (env_mask & (1 << i)) {
                if (_svm_voice[i].unk1b == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                _svm_voice[i].unk1b = 0;
            }
        }
    }

    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
    for (i = 0; i < 24; i++) {
        if (_svm_voice[i].auto_vol != 0) {
            D_80102BF8(i);
        }
        if (_svm_voice[i].auto_pan != 0) {
            D_801027E8(i);
        }
    }

    for (i = 0; i < 24; i++) {
        attr.mask = 0;
        attr.voice = 1 << i;
        if (_svm_sreg_dirty[i] & 1) {
            attr.mask = 3;
            attr.volume.left = _svm_sreg_buf[i * 8 + 0];
            attr.volume.right = _svm_sreg_buf[i * 8 + 1];
        }
        if (_svm_sreg_dirty[i] & 4) {
            attr.mask |= 0x10;
            attr.pitch = _svm_sreg_buf[i * 8 + 2];
        }
        if (_svm_sreg_dirty[i] & 8) {
            attr.mask |= 0x80;
            attr.addr = (u16)_svm_sreg_buf[i * 8 + 3] << 3;
        }
        if (_svm_sreg_dirty[i] & 0x10) {
            attr.mask |= 0x60000;
            attr.adsr1 = _svm_sreg_buf[i * 8 + 4];
            attr.adsr2 = _svm_sreg_buf[i * 8 + 5];
        }
        if (attr.mask != 0) {
            func_8008B488(&attr);
        }
        _svm_sreg_dirty[i] = 0;
    }

    SpuSetKey(0, ((_svm_okof2 & 0xFF) << 16) | _svm_okof1);
    SpuSetKey(1, ((_svm_okon2 & 0xFF) << 16) | _svm_okon1);
    SpuSetReverbVoice(8, ((D_800F2B68 & 0xFF) << 16) | D_800F1B14);

    _svm_okof1 = 0;
    _svm_okof2 = 0;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
}
/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */
extern s32 D_800F19D0[2];
extern s16 kMaxPrograms;
extern u16 _svm_vab_count;

void _SsVmInit(s32 a0) {
    s32 buf[16];
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
        u16 masked = (u8)a0;
        if (masked >= 0x18) {
            _SsVmMaxVoice = 0x18;
        } else {
            _SsVmMaxVoice = masked;
        }
    }

    buf[1] = 0x60093;
    i = 0;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;

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
            buf[0] = 1 << i;
            func_8008B488(buf);
            _svm_cur.voice = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < _SsVmMaxVoice);
    }

    _svm_rattr_plus_0x8 = 0x3FFF;
    _svm_rattr_plus_0xA = 0x3FFF;
    _svm_okon1 = 0;
    _svm_okon2 = 0;
    _svm_okof1 = 0;
    D_800F1B14 = 0;
    D_800F2B68 = 0;
    _svm_rattr = 0;
    _svm_rattr_plus_0x4 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
