/* Best-known form for replay_camera_Init — src/code6cac_b2_post.c
 * Apply this body in place of the current replay_camera_Init definition.
 *
 * Measured floor (s3, 2026-07-30):  17 -> 13.
 *   sandbox --disable all  ->  score 13, target 39 insns, build 38
 * (s0/s1/s2 floor was 17 / 36 insns.  The 14-with-cheats number the s2 header
 * quoted is now IRRELEVANT: the D_80101E70 reload that only the stripped
 * `extern volatile` used to produce is now produced HONESTLY, so the
 * cheat-invisible sandbox sees it.)
 *
 * 0 register-asm pins, 0 __asm__ blocks, 1 residual regfix rule
 * (replay_camera_Init: fill_delay @ 26 <- 15 — cannot retire until score 0).
 *
 * ============================ REVIEWER NOTICE ============================
 * This form carries TWO pointer-to-global locals (`pe62`, `pe70`).  They are
 * the pointer-alias-fake-exception family and MUST be reviewed by a fresh
 * layer-2 cheat-reviewer before this function can be accepted as COMPLETED-C.
 * The three prerequisites that s0's FAILed proposal lacked are supplied here:
 *
 *  (1) LEVER EXHAUSTION.  s1 H1/H2/H3/H4, s2 H5/H6/H7 — the volatile
 *      carve-out census (NEGATIVE), the whole "constant-foldable pointer
 *      invalidates CSE's memory table" family, the dead-extra-parameter RA
 *      shift, and an 8-form statement-order / declaration-order /
 *      type-narrowing sweep are all measured dead and banked in
 *      hypotheses.md + rejected/.
 *
 *  (2) NAMED GCC-PASS MECHANISM.  GCC 2.7.2 `cse.c:7308-7361` records a
 *      store's destination MEM in the equivalence table keyed by the stored
 *      value, so a LATER READ OF THE SAME MEM RTX folds to the stored
 *      register (store-to-load forwarding) and the reload disappears.  The
 *      read `*pe70` is a DIFFERENT rtx — `(mem (reg))`, not
 *      `(mem (symbol_ref))` — so `exp_equiv_p` does not match it against the
 *      recorded `(mem (symbol_ref "D_80101E70"))` entry and the load survives
 *      to codegen, exactly as it does in target.  This is the mechanism the
 *      `volatile` used to supply via the `sets[i].src_elt == 0` guard at
 *      cse.c:7329; the pointer supplies it without any type qualifier.
 *      `pe62` supplies target's SECOND observable shape: one materialised
 *      address (`lui;addiu`) held in a register and reused for both the
 *      pre-branch `lh` and the post-branch `sh` — target's `$t0`.
 *
 *  (3) /* FAKE *\/ ANNOTATION.  Both declarations carry it inline below.
 *
 * NOTE the important difference from s1's KILLED H2: H2 predicted the alias
 * would invalidate ALL memory equivalences (via note_mem_written) and was
 * correctly killed — the address does constant-fold, so nothing is
 * invalidated.  What actually works is narrower and was never tested before
 * s3: the alias does not invalidate anything, it simply makes the READ a
 * non-matching rtx.  Do not confuse the two.
 * =========================================================================
 *
 * s3 measurements around this form (all `sandbox --disable all`):
 *   candidate.c (this)                                 13 / 38
 *   without pe62 (direct D_80101E62)                   14 / 38
 *   without pe70 (direct read of D_80101E70)           19 / 35   <- reload gone
 *   s0-s2 candidate (neither pointer)                  17 / 36
 *   target statement order + both pointers             19 / 39   <- 39 insns!
 *
 * The last line is the live frontier: writing the body in target's own
 * execution order (both loads, both stores, the re-read, and only THEN
 * `D_80101E7C = a1;`) produces exactly 39 instructions INCLUDING the
 * `move a3,a1` parameter home in the bnez delay slot that four sessions could
 * not materialise — but lands it in $a2 and shuffles the store order, so the
 * score rises to 19.  It is a register-naming problem now, not a
 * missing-instruction problem.  See tmp/grind/replay_camera_Init/s3/.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    s32 sval;
    /* FAKE — pointer-alias-fake-exception: materialises D_80101E62's address
     * once into a register so the guard's `lh` and the later `sh` share it,
     * reproducing target's $t0.  See the REVIEWER NOTICE above. */
    s16 *pe62 = &D_80101E62;
    /* FAKE — pointer-alias-fake-exception: makes the re-read of D_80101E70 a
     * `(mem (reg))` rtx that cse.c's store-to-load forwarding cannot fold
     * against the recorded `(mem (symbol_ref))`, so the reload survives.  See
     * the REVIEWER NOTICE above. */
    s32 *pe70 = &D_80101E70;
    s32 reloaded;

    if (*pe62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    {
        extern u8 SpecialCam;
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    *pe62 = 2;
    reloaded = *pe70;
    D_80101E9E = 0;
    D_80101E78 = (u32)(reloaded + 0x7FF) >> 11;
    return 1;
}
