/* KILLED s11 — three sub-variants of arm-tail / arm-head label steering all INERT (score=2, byte-identical to baseline).
 *
 * The last unmeasured structural axis from the s10 frontier — using explicit `goto`/label
 * placement in the FALSE arm (or in both arms) to steer jump2/find_cross_jump's decision
 * without triggering a shared-tail merge. Hypothesis: a labeled point BETWEEN the color-lbu
 * triple and the gnd_load_tex call would shift LUID assignment or block cross-jump merging,
 * potentially perturbing sched1's tiebreak.
 *
 * Measurements (all sandbox --disable all):
 *   Variant A — goto+label AFTER color triple, BEFORE gnd_load_tex:
 *       ... b = ...; goto call_tex_false; call_tex_false: gnd_load_tex(...);
 *     score=2, build_insns=82 — byte-identical to baseline.
 *   Variant B — labeled entry BEFORE color triple:
 *       else { goto load_rgb_false; load_rgb_false: r=...; g=...; b=...; gnd_load_tex(...); }
 *     score=2, build_insns=82 — byte-identical to baseline.
 *   Variant C — distinct trailing labels in BOTH arms (attempt to break cross-jump symmetry):
 *       ... gnd_load_tex(...); goto tex_done_true; tex_done_true:;  (mirror on FALSE arm)
 *     score=2, build_insns=82 — byte-identical to baseline.
 *
 * Mechanism (KILLED): GCC's jump.c pass strips redundant labels (label with no cross-referencing
 * jump) and elides `goto NEXT_STMT;` forms before any of jump2/reorg/find_cross_jump sees them.
 * The label-placement axis is therefore INERT for this function — no LUID shift, no jump2
 * steering effect, no sched1 perturbation.
 *
 * Closes the "arm-tail label placement steering" frontier from s10's frontier list as measured
 * dead. Combined with s8/s9 shared-v shape work (which produced regressions 2->11/12 via
 * genuine tail merges), the cross-jump structural axis surface for this function is now
 * complete: shared-tail-merged forms regress via insn-collapse, and no non-merged label form
 * has any effect at all.
 */

/* Example — Variant A (representative): */
void gnd_init_80041688_rej(s32 arg0, s32 arg1) {
    /* ... loop1/loop2 identical to baseline ... */
    /* FALSE arm: */
    /* r = *((u8*)player + 0x18); */
    /* g = *((u8*)player + 0x19); */
    /* b = *((u8*)player + 0x1A); */
    /* goto call_tex_false; */
    /* call_tex_false: */
    /* gnd_load_tex(b | ((r << 16) | (g << 8))); */
}
