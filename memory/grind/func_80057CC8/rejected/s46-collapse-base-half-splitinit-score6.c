/* s46 REJECTED — collapsing the `base`/`half` named intermediates in the
 * ang_next < ang_prev arm into one expression measures score 6 (build_insns 111)
 * on the live chassis; the winning s46 form (score 0) keeps both locals.
 * A second collapse spelling, split-init accumulation on ang_mid itself
 * (`ang_mid = ang_prev + 0x800; ang_mid -= (s32)(ang_prev - ang_next) / 2;`),
 * ALSO measures score 6 — so the lever is the two separately-named intermediates,
 * not the operator association or the statement count.
 * Both variants banked here; do not re-propose either.
 */
/* variant A — single expression */
    if (ang_next < ang_prev) {
        ang_mid = (ang_prev + 0x800) - (s32)(ang_prev - ang_next) / 2;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }
/* variant B — split-init accumulation on ang_mid */
    if (ang_next < ang_prev) {
        ang_mid = ang_prev + 0x800;
        ang_mid -= (s32)(ang_prev - ang_next) / 2;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }
