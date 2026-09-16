/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */
typedef struct {
/* 0x00 */ s16 unk00;
/* 0x02 */ s16 unk02;
/* 0x04 */ s16 unk04;
/* 0x06 */ s16 unk06;
/* 0x08 */ s16 unk08;
/* 0x0A */ s8  unk0A;
/* 0x0B */ s8  unk0B;
/* 0x0C */ s16 unk0C;
/* 0x0E */ s16 unk0E;
/* 0x10 */ s16 unk10;
/* 0x12 */ s16 unk12;
/* 0x14 */ s16 unk14;
/* 0x16 */ s16 unk16;
/* 0x18 */ s8  unk18;
/* 0x19 */ s8  unk19;
/* 0x1A */ s8  unk1A;
/* 0x1B */ s8  unk1B;
/* 0x1C */ s8  unk1C;
/* 0x1D */ s8  unk1D;
/* 0x1E */ s16 unk1E;
/* 0x20 */ s16 unk20;
/* 0x22 */ s16 unk22;
/* 0x24 */ s16 unk24;
/* 0x26 */ s16 unk26;
/* 0x28 */ s16 unk28;
/* 0x2A */ s16 unk2A;
/* 0x2C */ s16 unk2C;
/* 0x2E */ s16 unk2E;
/* 0x30 */ s16 unk30;
/* 0x32 */ s16 unk32;
} SvmVoice; /* 0x36 = 54 bytes */
extern SvmVoice D_800F4E18[];
extern s32 MarioCam_str[2];
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern u16 D_801078D8;
extern u8  _svm_auto_kof_mode;
extern s16 kMaxPrograms;
extern u16 _svm_vab_count;
extern u16 D_800F1B14;
extern u16 D_800F2B68;

void _SsVmInit(s32 a0) {
    s32 buf[16];
    s32 idx;
    u16 i;

    _spu_setInTransfer(0);
    D_800F66F8 = 0;
    SpuInitMalloc(0x20, MarioCam_str);

    i = 0;
    do {
        D_80102A78[i] = 0;
        i++;
    } while (i < 0xC0);
    i = 0;
    do {
        D_800F65E0[i] = 0;
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
            idx = i;
            D_800F4E18[idx].unk02 = 0x18;
            D_800F4E18[idx].unk10 = -1;
            D_800F4E18[idx].unk00 = 0xFF;
            D_800F4E18[idx].unk1D = 0;
            D_800F4E18[idx].unk04 = 0;
            D_800F4E18[idx].unk06 = 0;
            D_800F4E18[idx].unk12 = 0;
            D_800F4E18[idx].unk14 = 0;
            D_800F4E18[idx].unk16 = 0xFF;
            D_800F4E18[idx].unk08 = 0;
            D_800F4E18[idx].unk0C = 0;
            D_800F4E18[idx].unk0A = 0x40;
            D_800F4E18[idx].unk1E = 0;
            D_800F4E18[idx].unk20 = 0;
            D_800F4E18[idx].unk22 = 0;
            D_800F4E18[idx].unk24 = 0;
            D_800F4E18[idx].unk2A = 0;
            D_800F4E18[idx].unk2C = 0;
            D_800F4E18[idx].unk2E = 0;
            D_800F4E18[idx].unk30 = 0;
            D_800F4E18[idx].unk32 = 0;
            D_800F4E18[idx].unk26 = 0;
            buf[0] = 1 << i;
            func_8008B488(buf);
            _svm_cur.voice = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < _SsVmMaxVoice);
    }

    _svm_rattr_plus_0x8 = 0x3FFF;
    _svm_rattr_plus_0xA = 0x3FFF;
    D_800F1B10 = 0;
    D_800F1B12 = 0;
    D_801078D8 = 0;
    D_800F1B14 = 0;
    D_800F2B68 = 0;
    _svm_rattr = 0;
    _svm_rattr_plus_0x4 = 0;
    _svm_auto_kof_mode = 0;
    _svm_stereo_mono = 0;
    kMaxPrograms = 0x80;
    _SsVmFlush();
}
