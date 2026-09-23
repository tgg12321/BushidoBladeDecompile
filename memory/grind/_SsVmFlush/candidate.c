/* Sony PsyQ LIBSND VM_F: flush pending voice-manager state to LIBSPU. */
typedef struct {
    s16 left;
    s16 right;
} VmSpuVolume;

typedef struct {
    u32 voice;
    u32 mask;
    VmSpuVolume volume;
    VmSpuVolume volmode;
    VmSpuVolume volumex;
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
} VmSpuVoiceAttr;

typedef struct {
    s16 field_00;
    s16 field_02;
    s16 field_04;
    u16 envx;
    s16 field_08;
    u8 field_0a;
    u8 field_0b;
    s16 note;
    s16 field_0e;
    s16 field_10;
    s16 program;
    s16 tone;
    s16 vab_id;
    s16 field_18;
    u8 field_1a;
    u8 noise_mode;
    s16 auto_vol;
    s16 field_1e;
    s16 field_20;
    s16 field_22;
    s16 start_vol;
    s16 end_vol;
    s16 auto_pan;
    s16 field_2a;
    s16 field_2c;
    s16 field_2e;
    s16 start_pan;
    s16 end_pan;
    s16 field_34;
} VmVoice;

extern s32 D_80103604;
extern s32 D_80107898[];
extern u8 _SsVmMaxVoice;
extern u8 _svm_auto_kof_mode;
extern s8 D_800F4E35;
extern s16 D_800F4E36;
extern s16 D_800F4E42;
extern s16 D_800F4E18;
extern void (*D_80102BF8)(s32);
extern void (*D_801027E8)(s32);
extern u16 D_801078D8;
extern u16 D_801078DA;
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern u16 D_800F1B14;
extern u16 D_800F2B68;
extern void func_8008B488(VmSpuVoiceAttr *);

void _SsVmFlush(void)
{
    s32 i;
    u32 silent;
    s32 offset;
    VmVoice *voices;
    VmSpuVoiceAttr attr;

    D_80103604 = (D_80103604 + 1) & 0xF;
    D_80107898[D_80103604] = 0;

    /* FAKE: typed local alias tests address-materialization/CSE separation
       between the call output pointer and the post-call global reload. */
    voices = (VmVoice *)&D_800F4E18;
    for (i = 0; i < _SsVmMaxVoice; i++) {
        SpuGetVoiceEnvelope(i, &voices[i].envx);
        if (((VmVoice *)&D_800F4E18)[i].envx == 0) {
            D_80107898[D_80103604] |= 1 << i;
        }
    }

    if (_svm_auto_kof_mode == 0) {
        silent = -1;
        for (i = 0; i < 15; i++) {
            silent &= D_80107898[i];
        }
        for (i = 0; i < _SsVmMaxVoice; i++) {
            offset = i * 54;
            if (silent & (1 << i)) {
                if (*(u8 *)((u8 *)&D_800F4E35 + offset) == 2) {
                    SpuSetNoiseVoice(0, 0xFFFFFF);
                }
                *(u8 *)((u8 *)&D_800F4E35 + offset) = 0;
            }
        }
    }

    D_800F1B10 &= ~D_801078D8;
    D_800F1B12 &= ~D_801078DA;

    for (i = 0; i < 24; i++) {
        offset = i * 54;
        if (*(s16 *)((u8 *)&D_800F4E36 + offset) != 0) {
            D_80102BF8(i);
        }
        if (*(s16 *)((u8 *)&D_800F4E42 + offset) != 0) {
            D_801027E8(i);
        }
    }

    for (i = 0; i < 24; i++) {
        attr.mask = 0;
        attr.voice = 1 << i;
        if (D_800F65E0[i] & 1) {
            attr.mask = 3;
            attr.volume.left = D_80102A78[i * 8];
            attr.volume.right = D_80102A78[i * 8 + 1];
        }
        if (D_800F65E0[i] & 4) {
            attr.mask |= 0x10;
            attr.pitch = D_80102A78[i * 8 + 2];
        }
        if (D_800F65E0[i] & 8) {
            attr.mask |= 0x80;
            attr.addr = (u16)D_80102A78[i * 8 + 3] << 3;
        }
        if (D_800F65E0[i] & 0x10) {
            attr.mask |= 0x60000;
            attr.adsr1 = D_80102A78[i * 8 + 4];
            attr.adsr2 = D_80102A78[i * 8 + 5];
        }
        if (attr.mask != 0) {
            func_8008B488(&attr);
        }
        D_800F65E0[i] = 0;
    }

    SpuSetKey(0, ((u32)(u8)D_801078DA << 16) | D_801078D8);
    SpuSetKey(1, ((u32)(u8)D_800F1B12 << 16) | D_800F1B10);
    SpuSetReverbVoice(8, ((u32)(u8)D_800F2B68 << 16) | D_800F1B14);

    D_801078D8 = 0;
    D_801078DA = 0;
    D_800F1B10 = 0;
    D_800F1B12 = 0;
}
