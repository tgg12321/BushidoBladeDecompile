/* REJECTED s23 -- defeat-licm-hoist-var-reuse applied to the 0x1F8002B8
 * scratchpad-address literal: reuse the dead `scale` pseudo (dead after
 * z computation) for the invariant literal at both func_80053614 call
 * sites, to make the pseudo multi-set and defeat loop.c's movable
 * admission for the constant.
 *
 * Measured on the s22-banked 38/198 chassis: score 38 -> 71/204,
 * build_insns UNCHANGED at 198 (pure register-identity/allocation
 * regression, not an insn-count change). WORSE. See hypotheses.md s23.
 *
 * Only the changed lines relative to candidate.c are shown; apply on
 * top of the s22-banked body.
 */

        /* ... scale = (&D_8009A820)[i * 2] << 8; x = ...; z = ... (unchanged) ... */

        scale = 0x1F8002B8;
        flags = func_80053614(pt0, pt1, (s32)hit0, (s32)work, scale);
        /* ... */
        flags = (flags | (func_80053614(pt0, pt1, (s32)hit1, (s32)work, scale) << 1)) + 1;
