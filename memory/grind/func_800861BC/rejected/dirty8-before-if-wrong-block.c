/* REJECTED (s1, recon, 2026-09-02): dirty|=8 hoisted BEFORE the if/else: emitted before the branch, target has it after the join label; no pass sinks it.
   Chassis: asm-until-matched HEAD 2829a7b0, TU-local struct_svm at D_801027F0, no FAKE constructs.
   Measured: sandbox --disable all = 37 (struct-addend noise = 13-18 of that; see tmp/grind/func_800861BC/s1/v7_dirty8_before_if_pairdiff.txt). */
struct struct_svm {
    char prog_tones; char vabId; char note; char fine; char volume; char pan;
    char prog; char field_7_fake_program; char field_8_unknown; char field_0x9;
    char mvol; char mpan; char tone; char tone_vol; char tone_pan;
    char tone_prior; char tone_center; unsigned char tone_shift; char tone_min;
    char tone_max; u8 tone_mode; u8 pad; short seq_sep_no; short tone_vag_idx;
    short voice; short voiceOffset; short field_0x1e;
};
extern struct struct_svm D_801027F0;
typedef struct {
    u8 tones, mvol, prior, mode, mpan, reserved0;
    s16 attr;
    u32 reserved1;
    u32 reserved2;
} ProgAtr;
void func_800861BC(void) {
    int i;
    int progIdx;

    D_801027F0.voiceOffset = D_801027F0.voice * 8;
    D_801027F0.field_0x1e = D_801027F0.field_7_fake_program * 16 + D_801027F0.tone;
    *(s16 *)((u8 *)&D_800F4E1E + D_801027F0.voice * 54) = 0x7FFF;
    for (i = 0; i < 16; i++) {
        D_80107898[i] &= ~(1 << D_801027F0.voice);
    }
    D_800F65E0[D_801027F0.voice] |= 8;

    if ((D_801027F0.tone_vag_idx & 1) > 0) {
        progIdx = (D_801027F0.tone_vag_idx - 1) / 2;
        D_80102A78[D_801027F0.voiceOffset + 3] =
            ((u16 *)&((ProgAtr *)D_800FF6A0)[progIdx].reserved2)[0];

    } else {
        progIdx = (D_801027F0.tone_vag_idx - 1) / 2;
        D_80102A78[D_801027F0.voiceOffset + 3] =
            ((u16 *)&((ProgAtr *)D_800FF6A0)[progIdx].reserved2)[1];

    }
    D_80102A78[D_801027F0.voiceOffset + 4] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr1;
    D_80102A78[D_801027F0.voiceOffset + 5] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr2 + D_800F66F8;
    D_800F65E0[D_801027F0.voice] |= 0x30;

}
