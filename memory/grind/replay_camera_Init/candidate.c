/* Best-known form for replay_camera_Init — src/code6cac_b2_post.c
 * Apply this body in place of the current replay_camera_Init definition.
 *
 * Measured floor (s1, 2026-07-30, unchanged from s0):
 *   sandbox --disable all                 -> score 17, target 39 insns, build 36
 *   sandbox --disable all --keep-cheat-asm -> score 14, build 38
 * 0 register-asm pins, 0 __asm__ blocks, 0 pointer aliases, 1 residual regfix
 * rule (replay_camera_Init: fill_delay @ 26 <- 15 — cannot retire until score 0).
 *
 * s0's cheat-reviewer (2026-07-05) FAILed an earlier version that carried
 * `s16 *s0 = &D_80101E62;` as an unannotated pointer alias
 * (pointer-alias-fake-exception requires lever-exhaustion + named mechanism +
 * a /* FAKE *\/ annotation, none of which were present). The alias is therefore
 * kept OUT of this form.
 *
 * s1 correction to the s0 note: the alias is score-inert only in the
 * volatile-STRIPPED regime that `--disable all` measures. With the reload
 * present it is worth -1 (14 -> 13). It is still not committable as-is, but do
 * not repeat s0's conclusion that it does nothing.
 *
 * The 3-instruction shortfall is fully characterised in hypotheses.md:
 *   - 2 insns = the `lui/lw` reload of D_80101E70, produced ONLY by the
 *     pre-existing `extern volatile s32 D_80101E70;` at line 45 of the source
 *     file, which the cheat-invisible sandbox strips (cse.c:7329 — a volatile
 *     MEM has src_elt == 0 so its store is never recorded as a forwardable
 *     equivalence).
 *   - 1 insn = `addu $a3,$a1,$zero`, the incoming-a1 parameter copy that target
 *     puts in the bnez delay slot; it exists only if RA denies the a1 pseudo
 *     both $a1 and $a2.
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
        s32 cam_val;
        s32 ec_val;
        cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
        D_80101E6C = cam_val;
        D_80101E7C = a1;
        ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
        D_80101E70 = ec_val;
    }
    D_80101E68 = 0;
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF) >> 11;
    return 1;
}
