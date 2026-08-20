/* func_80072CD4 - BYTE-MATCHING FORM (s5 synthesis, 2026-08-20):
 *   `sandbox func_80072CD4 --disable all` = **0**, build_insns 79 == target_insns 79,
 *   rules_dropped 0. Artifact: tmp/grind/func_80072CD4/s5/sandbox_rgbtriple_noholder.json.
 *
 * CLASSIFICATION IS PENDING A RULING - do not treat this as accepted. See the s5 outcome
 * (result: ruling-request) and hypotheses.md H-S1. The question is whether the standing
 * 2026-07-24 judge constraint ("do NOT respell the @4/@0xC common-tail stores as a
 * duplicated-into-arms store-schedule lever") reaches THIS body, which contains no lever
 * construct at all. If the ruling is NO (i.e. this body is ordinary C), the function closes
 * at 0. If the ruling is YES, fall back to fallback_floor4.c (the clean floor-4 form).
 *
 * WHAT CHANGED vs every s1-s4 form - the semantic reframe that produced the match:
 *   arg1 is not an opaque byte blob. The offsets are the canonical PSX libgpu POLY_G4 vertex
 *   colour layout: rgb0 = 0x04/0x05/0x06, rgb1 = 0x0C/0x0D/0x0E, rgb2 = 0x14/0x15/0x16,
 *   rgb3 = 0x1C/0x1D/0x1E (i.e. the four setRGB0..setRGB3 triples). Written that way - each
 *   branch assigning its OWN complete rgb0 and rgb1 triple, then the two unconditional
 *   rgb2/rgb3 triples - the body is byte-exact. No local, no holder variable, no volatile,
 *   no asm, no barrier, no dead store, no annotation: every statement is a live field write
 *   executed on its own path with the value the game displays.
 *
 * The red component of rgb0/rgb1 is 0xFC on BOTH branches, so the two arms happen to contain
 * an identical `= 0xFC` store; GCC's jump2 cross-jump tail-merges those (plus the rgb1-blue
 * store) at the join label, which is exactly how target's merge block begins
 * (`sb v1,4 / sb v1,0xC / sb v0,0xE`). That merge is a compiler tail-merge of two LIVE
 * statements on mutually exclusive paths, not a dead second copy.
 *
 * IN-REPO PRECEDENT for the spelling: the COMPLETED-C sibling in this same file,
 * func_80072BC4 (src/text1b.c:5822), is written in exactly this field-order house style and
 * itself carries an identical cross-arm duplicate store, `*(u8 *)((s32)(arg1) + 0x1D) = 0xC3;`
 * at src/text1b.c:5840 and src/text1b.c:5843, hoistable but not hoisted.
 *
 * The previously banked floor-4 form (fallback_floor4.c) is the ARTIFICIAL spelling by
 * comparison: it lifts only the two red components out of the colour assignments into a
 * shared tail behind an `int fc_const` holder, which is what left the merge-block store order
 * wrong (residual 4).
 *
 * An intermediate variant with the sibling's `int fc_const` holder retained also measures 0
 * (tmp/grind/func_80072CD4/s5/v_rgbtriple.c); this holder-free body is the cleaner of the two.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = 0xFC;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = 0xFC;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
