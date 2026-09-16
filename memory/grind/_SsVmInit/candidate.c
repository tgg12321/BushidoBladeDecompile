/* _SsVmInit candidate — unchanged since s2 (structural); re-verified at s2
 * AND s3. Honest sandbox floor: 19 (unchanged across s2/s3 — six isolated
 * structural levers tried and killed across s2 (H7/H8/H9) and s3 (H10, the
 * shared-0xFF named-intermediate hoist), all measured WORSE than this
 * baseline; see hypotheses.md). Was 38 at end of s1 (a false floor from a
 * declaration-order bug; see hypotheses.md s2 H5). s3 PASS ATTRIBUTION
 * identified the residual as a local-alloc-death-count-class-wall on the
 * per-iteration `D_8010280A = i;` HImode store (pseudo reg 112 in
 * tmp/grind/_SsVmInit/dumps/main.greg/.lreg — "dies in 0 places; crosses 2
 * calls"); see hypotheses.md s3 PASS ATTRIBUTION entry before re-guessing
 * spellings at that store site.
 * Apply verbatim in place of `INCLUDE_ASM("asm/funcs", _SsVmInit);` in
 * src/main.c. Several of these externs duplicate declarations already in
 * scope LATER in the same TU (main.c) with identical types — harmless
 * per C extern-redeclaration rules, and REQUIRED here because this
 * function sits textually BEFORE those later declarations.
 */
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
extern u16 D_8010280A;
extern u16 D_800F1B14;
extern u16 D_800F2B68;

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
