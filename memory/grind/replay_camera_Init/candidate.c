/* Best-known form for replay_camera_Init — src/code6cac_b2_post.c
 * Apply this body in place of the current replay_camera_Init definition.
 *
 * Measured floor (unchanged s0 -> s1 -> s2, 2026-07-30):
 *   sandbox --disable all                  -> score 17, target 39 insns, build 36
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
 * s2 (structural modality) confirmed this exact form is a LOCAL OPTIMUM on the
 * ordering axis. Eight statement-order / declaration-order / type-narrowing
 * variants were measured in both regimes (sweep harness + results banked in
 * tmp/grind/replay_camera_Init/s2/); every neighbour is worse, including the
 * "write the statements in target's execution order" form, which scores
 * 18/37 stripped and 23/40 with the reload. The placement of
 * `D_80101E7C = a1;` BETWEEN the SpecialCam load and the D_8008EC38 load is
 * load-bearing — do not "clean it up" into the natural order.
 *
 * The 3-instruction shortfall is fully characterised in hypotheses.md:
 *   - 2 insns = the `lui/lw` reload of D_80101E70, produced ONLY by the
 *     pre-existing `extern volatile s32 D_80101E70;` at line 45 of the source
 *     file, which the cheat-invisible sandbox strips (cse.c:7329 — a volatile
 *     MEM has src_elt == 0 so its store is never recorded as a forwardable
 *     equivalence). s2's census KILLED the carve-out route: D_80101E70 has no
 *     asynchronous IRQ/callback writer, so the volatile is NOT eligible for
 *     legitimate-volatile-interrupt-touched.
 *   - 1 insn = `addu $a3,$a1,$zero`, the incoming-a1 parameter copy that target
 *     puts in the bnez delay slot. s2 CONFIRMED it materialises exactly when
 *     another live value occupies hard reg $a1 across the copy's range, and
 *     that this function's honest value set (sval / cam_val / ec_val) cannot
 *     supply that occupancy.
 *
 * Everything else in the 17 is register naming — and s2 established that our
 * naming for sval / cam_val / ec_val ($v0 / $v1 / $a0) ALREADY matches target
 * exactly. The two register-level residuals are the $t0 address cache for
 * D_80101E62 (the alias) and the $a3 copy.
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
