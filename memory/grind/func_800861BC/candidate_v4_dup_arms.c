/* ALTERNATE CANDIDATE (same bytes as candidate.c; sandbox 18 = addend artifact, real 0;
   tmp/grind/func_800861BC/s1/v4_dup_arms_pairdiff.txt; oracle SHA1 proven with THIS body,
   s1/verify_oracle.txt). Duplicated-statement-into-arms form (FAKE-annotated family,
   .claude/rules/duplicated-statement-into-arms.md). Prefer candidate.c (ordinary-C helper,
   no FAKE). Kept so the reviewer can choose. Struct decl is the TU-local measurement form. */
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

    if ((D_801027F0.tone_vag_idx & 1) > 0) {
        progIdx = (D_801027F0.tone_vag_idx - 1) / 2;
        D_80102A78[D_801027F0.voiceOffset + 3] =
            ((u16 *)&((ProgAtr *)D_800FF6A0)[progIdx].reserved2)[0];
    /* FAKE: dirty|=8 duplicated into both if/else arms, mechanism: cse.c use_related_value anchors the post-join block on its FIRST materialized _svm_cur address (voiceOffset) and jump2 cross-jump re-merges the copies byte-neutrally; lever-exhaustion: memory/grind/func_800861BC/hypotheses.md s1 H3-H6 */
    D_800F65E0[D_801027F0.voice] |= 8;

    } else {
        progIdx = (D_801027F0.tone_vag_idx - 1) / 2;
        D_80102A78[D_801027F0.voiceOffset + 3] =
            ((u16 *)&((ProgAtr *)D_800FF6A0)[progIdx].reserved2)[1];
    /* FAKE: dirty|=8 duplicated into both if/else arms, mechanism: cse.c use_related_value anchors the post-join block on its FIRST materialized _svm_cur address (voiceOffset) and jump2 cross-jump re-merges the copies byte-neutrally; lever-exhaustion: memory/grind/func_800861BC/hypotheses.md s1 H3-H6 */
    D_800F65E0[D_801027F0.voice] |= 8;

    }
    D_80102A78[D_801027F0.voiceOffset + 4] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr1;
    D_80102A78[D_801027F0.voiceOffset + 5] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr2 + D_800F66F8;
    D_800F65E0[D_801027F0.voice] |= 0x30;

}
