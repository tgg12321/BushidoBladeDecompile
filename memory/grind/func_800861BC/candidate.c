/* CANDIDATE — func_800861BC (Sony LIBSND `_SsVmDoAllocate`; psyz vm_aloc2.c analog)
 * s1 re-run (recon, 2026-09-02). BYTES PROVEN on the header-form tree banked as
 * memory/grind/func_800861BC/integration_patch.diff (include/sound.h + src/main.c):
 * verify-oracle --rebuild build_sha1 == oracle 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * (tmp/grind/func_800861BC/s1/verify_oracle_header.txt), sandbox --disable all = 18
 * at 132/132 where ALL 18 are named-symbol LO16 addend hunks (struct field addend
 * D_801027F0+N vs the INCLUDE_ASM reference's splat per-word symbol at +0;
 * engine/score.py:61 compares named-symbol addends) — filt.py real=0 noise=18
 * (s1/v9_header_form_pairdiff.txt). Zero instruction/register/order differences.
 *
 * THIS FILE IS THE src/main.c BODY ONLY. It depends on declarations that the patch
 * places in include/sound.h (which src/main.c must #include):
 *   struct struct_svm { ... } ; extern struct struct_svm D_801027F0;   (_svm_cur)
 *   typedef struct { u8 tones, mvol, prior, mode, mpan; s8 reserved0; s16 attr;
 *                    u32 reserved1; u16 reserved2; u16 reserved3; } ProgAtr;
 * (ProgAtr is BB2's pre-existing typedef, moved out of src/main.c; VagAtr
 * D_80101BC8, D_80102A78[], D_800F65E0[], D_80107898[], D_800F4E1E, D_800F66F8,
 * D_800FF6A0 (s32) are declared in src/main.c already.)
 *
 * REQUIRES the "Per-word splat symbol -> aggregate merge" family (owner ruling
 * 2026-08-17, .claude/rules/no-new-park-categories.md:238; precedent e788983a):
 * struct_svm at 0x801027F0 replaces D_801027F1/F6/F7/FC, D_80102806/08/0A/0C/0E at
 * every consumer (all in src/main.c). To reproduce the tree: git apply the patch.
 *
 * Alternative body with identical bytes: candidate_v4_dup_arms.c (literal
 * duplication of the dirty|=8 statement into both arms, FAKE-annotated
 * duplicated-statement family). This file is the ordinary-C helper form.
 */

/* Sony LIBSND `_SsVmDoAllocate` (psyz vm_aloc2.c analog): set up the
   allocated voice's SPU shadow registers (start address, ADSR) and mark the
   voice's dirty bits. BB2 deltas vs psyz: _svm_voice stride 54, ADSR indexed
   by voiceOffset through the flat s16 shadow view D_80102A78[]. */
static inline void vmSetStartAddr(u16 addr) {
    D_80102A78[D_801027F0.voiceOffset + 3] = addr;
    D_800F65E0[D_801027F0.voice] |= 8;
}

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
        vmSetStartAddr(((ProgAtr *)D_800FF6A0)[progIdx].reserved2);
    } else {
        progIdx = (D_801027F0.tone_vag_idx - 1) / 2;
        vmSetStartAddr(((ProgAtr *)D_800FF6A0)[progIdx].reserved3);
    }
    D_80102A78[D_801027F0.voiceOffset + 4] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr1;
    D_80102A78[D_801027F0.voiceOffset + 5] =
        D_80101BC8[D_801027F0.field_7_fake_program * 16 + D_801027F0.tone].adsr2 + D_800F66F8;
    D_800F65E0[D_801027F0.voice] |= 0x30;
}
