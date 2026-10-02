typedef struct {
    u32 voice;
    u32 mask;
    SpuVolume volume;
    SpuVolume volmode;
    SpuVolume volumex;
    u16 pitch;
    u16 note;
    u16 sample_note;
    s16 envx;
    u32 addr;
    u32 loop_addr;
    s32 a_mode;
    s32 s_mode;
    s32 r_mode;
    u16 ar;
    u16 dr;
    u16 sr;
    u16 rr;
    u16 sl;
    u16 adsr1;
    u16 adsr2;
} TmpVoiceAttr;

extern s32 D_80103604;
extern u8 _svm_auto_kof_mode;
extern void (*D_80102BF8)(s32);
extern void (*D_801027E8)(s32);
extern u16 _svm_okon1;
extern u16 _svm_okon2;
extern u16 _svm_okof1;
extern u16 _svm_okof2;
extern u16 D_800F1B14;
extern u16 D_800F2B68;
extern u8 _SsVmMaxVoice;
extern void SpuGetVoiceEnvelope(s32, u16 *);
extern void SpuSetNoiseVoice(s32, s32);
extern void SpuSetKey(s32, u32);
extern void SpuSetReverbVoice(s32, s32);

void _SsVmFlush(void)
{
    s32 i;
    u32 env_mask;
    TmpVoiceAttr attr;

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
