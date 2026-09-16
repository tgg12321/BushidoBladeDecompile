/* _SsVmInit candidate - s8 (synthesis modality): MATCH, honest sandbox
 * distance 0 (target_insns 200 == build_insns 200).
 *
 * The closing lever (found s8): target's clamp asm is
 *     andi $a0,$s1,0xFF ; sltiu $v0,$a0,0x18 ; ... ; sb $a0,%lo(_SsVmMaxVoice)($at)
 * i.e. ONE masked value feeds BOTH the compare and the else-arm store. Every
 * prior session (s4/s6 hand-tried forms, s7's 7-spelling enumeration) varied
 * only the COMPARE side and always left the else arm storing the raw
 * parameter (`_SsVmMaxVoice = a0;`), so the masked pseudo died at the compare
 * and the store came from the parameter pseudo - the a0-vs-v0 residual.
 * Reading one masked local in both the condition and the else-arm store closes
 * it; the local must be wider than the store (`u16 masked = (u8)a0;` scores 0,
 * `u8 masked` scores 3 at 201 insns, and dropping the `(u8)` cast scores 3).
 *
 * The per-voice loop's current-voice store is spelled through the TU's shipped
 * object model, `_svm_cur.voice = i;` (struct struct_svm, include/sound.h:28,
 * base 0x801027F0; .voice sits at +0x1A = 0x8010280A), matching the identical
 * store in the bytes-proven sibling at src/main.c:993. The body carries NO
 * FAKE constructs.
 *
 * Apply verbatim in place of `INCLUDE_ASM("asm/funcs", _SsVmInit);` in
 * src/main.c.
 */
/* _SsVmInit - libsnd voice-manager init (SLUS-00663). */
extern s32 MarioCam_str[2];
extern s16 D_800F4E18;
extern s16 D_800F4E1C;
extern s8  D_800F4E35;
extern u16 D_800F1B10;
extern u16 D_800F1B12;
extern u16 D_801078D8;
extern u8  _svm_auto_kof_mode;
extern s16 kMaxPrograms;
extern u16 _svm_vab_count;
extern s16 D_800F4E20;
extern s8  D_800F4E22;
extern s16 D_800F4E24;
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
extern u16 D_800F1B14;
extern u16 D_800F2B68;

void _SsVmInit(s32 a0) {
    s32 buf[16];
    u16 i;
    s32 offset;

    _spu_setInTransfer(0);
    D_800F66F8 = 0;
    SpuInitMalloc(0x20, MarioCam_str);

    i = 0;
    do {
        *(s16 *)((u8 *)D_80102A78 + i * 2) = 0;
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
            offset = i * 54;
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
