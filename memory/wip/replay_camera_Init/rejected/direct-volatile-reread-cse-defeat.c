/* REJECTED 2026-07-06 — cheat-reviewer FAIL (pass 1, session 2).
 * Reads D_80101E70 back from the global (instead of reusing the
 * already-computed value) to compute D_80101E78. Nothing writes D_80101E70
 * in between the store and this read, so this exploits the pre-existing
 * (and unjustified) `extern volatile s32 D_80101E70;` declaration at
 * src/code6cac_b2_post.c:45 purely to force a CSE-defeating reload instead
 * of reusing the register — same forbidden mechanism as the
 * *(volatile s32*)&D_80101E70 cast and the `volatile s32 *ecp = &D_80101E70`
 * pointer-indirection variants already rejected in meta.json's
 * rejected_forms (no IRQ-writer citation exists for D_80101E70; it is NOT
 * in volatile_extern_allowlist.txt; 5 of 7 files declaring this symbol use
 * plain non-volatile s32). Measured distance: 17 (IDENTICAL to the fixed
 * form using a local `ec_val` instead — the volatile re-read bought
 * nothing, confirming it was cheat-shaped rather than load-bearing).
 *
 * DO NOT resurrect this form. Use the `ec_val` local-reuse form in
 * candidate.c instead — same score, no cheat.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        D_80101E6C = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E7C = a1;
        D_80101E70 = *(s32 *)((u8 *)&D_8008EC38 + sval);
    }
    D_80101E68 = 0;
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
