/* _SsVmInit candidate — session s1 (recon, drilled to progress).
 * Honest sandbox floor at end of session: 37 (started at 200, no-C-body).
 * Apply verbatim in place of the old `INCLUDE_ASM("asm/funcs", _SsVmInit);`
 * line in src/main.c (declarations block included — several are shared
 * with sibling functions already in the TU; de-dupe against whatever's
 * there when re-applying to a later HEAD).
 */
extern s32 MarioCam_str[2];
extern u8 _SsVmMaxVoice;
extern u8 _svm_vab_used[];
extern u16 _svm_vab_count;
extern s16 D_80102A78[];
extern u8 D_800F65E0[];
extern s16 D_800F4E1A;
extern s16 D_800F4E1C;
extern s16 D_800F4E1E;
extern s16 D_800F4E20;
extern s8  D_800F4E22;
extern s16 D_800F4E24;
extern s16 D_800F4E28[];
extern s16 D_800F4E2A;
extern s16 D_800F4E2C;
extern s16 D_800F4E2E;
extern s8  D_800F4E35;
extern s16 D_800F4E36;
extern s16 D_800F4E38;
extern s16 D_800F4E3A;
extern s16 D_800F4E3C;
extern s16 D_800F4E3E;
extern s16 D_800F4E42;
extern s16 D_800F4E44;
extern s16 D_800F4E46;
extern s16 D_800F4E48;
extern s16 D_800F4E4A;
extern u16 D_8010280A;
extern s32 _svm_rattr;
extern s32 _svm_rattr_plus_0x4;
extern s16 _svm_rattr_plus_0x8;
extern s16 _svm_rattr_plus_0xA;
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern u16 D_801078D8;
extern u16 D_800F1B14;
extern u16 D_800F2B68;
extern u8 _svm_auto_kof_mode;
extern s16 _svm_stereo_mono;
extern s16 kMaxPrograms;

void _SsVmInit(s32 a0) {
    s32 buf[16];
    s32 i;
    s32 offset;
    s32 maxVoice;

    _spu_setInTransfer(0);
    D_800F66F8 = 0;
    SpuInitMalloc(0x20, MarioCam_str);

    i = 0;
    do {
        *(s16 *)((u8 *)D_80102A78 + i * 2) = 0;
        i++;
    } while ((u16)i < 0xC0);
    i = 0;
    do {
        D_800F65E0[i] = 0;
        i++;
    } while ((u16)i < 0x18);
    _svm_vab_count = 0;
    i = 0;
    do {
        _svm_vab_used[i] = 0;
        i++;
    } while ((u16)i < 0x10);

    if ((u8)a0 < 0x18) {
        _SsVmMaxVoice = (u8)a0;
    } else {
        _SsVmMaxVoice = 0x18;
    }

    buf[1] = 0x60093;
    *(s16 *)((u8 *)buf + 0x14) = 0x1000;
    *(s32 *)((u8 *)buf + 0x1C) = 0x1000;
    *(u16 *)((u8 *)buf + 0x3A) = 0x80FF;
    *(s16 *)((u8 *)buf + 0x08) = 0;
    *(s16 *)((u8 *)buf + 0x0A) = 0;
    *(s16 *)((u8 *)buf + 0x3C) = 0x4000;

    maxVoice = _SsVmMaxVoice;
    if (maxVoice != 0) {
        i = 0;
        do {
            s32 idx = (u8)i;
            offset = ((idx * 8 - idx) * 4 - idx) * 2;
            *(s16 *)((u8 *)&D_800F4E1A + offset) = 0x18;
            *(s16 *)((u8 *)D_800F4E28 + offset) = -1;
            *(s16 *)((u8 *)&D_800F4E18 + offset) = 0xFF;
            *(s8  *)((u8 *)&D_800F4E35 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E1C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E1E + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E2E + offset) = 0xFF;
            *(s16 *)((u8 *)&D_800F4E20 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E24 + offset) = 0;
            *(s8  *)((u8 *)&D_800F4E22 + offset) = 0x40;
            *(s16 *)((u8 *)&D_800F4E36 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E38 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3C + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E42 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E44 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E46 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E48 + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E4A + offset) = 0;
            *(s16 *)((u8 *)&D_800F4E3E + offset) = 0;
            offset = 1;
            buf[0] = offset << i;
            func_8008B488(buf);
            D_8010280A = i;
            _SsVmKeyOffNow(1);
            i = i + 1;
        } while (i < maxVoice);
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
