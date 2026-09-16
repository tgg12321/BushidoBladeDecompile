/* REJECTED — func_80056CB8, s6 (2026-09-16). KILLED (instance), not a cheat —
 * plain register-pressure regression.
 *
 * Hypothesis: our build recomputes `i * 2` twice (once for the D_8009A821
 * flags-index, once ~30 lines later for the D_8009A820 scale-index, with a
 * ratan2 call in between) where target's asm reuses ONE materialized value
 * ($s8) across both sites via `addu at,at,s8`. Factoring both sites onto a
 * single named local should let GCC do the same reuse.
 *
 * Measured: score REGRESSED 81 -> 90 (build_insns 197 -> 199) on the s6
 * chassis (D_800F6610 fix + s5 store-batching + func_80053614 s32-return
 * fix). The shared local's live range spans the ratan2 call and the
 * intervening obj/ang/sin_p/cos_p computation, which raised register
 * pressure enough to cost more than the single `sll`+`addu` it removed.
 *
 * Do not re-propose this exact spelling. Target's $s8 reuse is real (see
 * candidate.c's REMAINING RESIDUAL section) but is NOT reached by a shared
 * INDEX local — try a shared BASE POINTER local next (e.g. precompute
 * `u8 *fp = (&D_8009A821) + idx;` reused differently), or investigate
 * whether it's actually a delay-slot/scheduling artifact independent of any
 * C-level variable sharing.
 *
 * --- the only changed lines vs candidate.c, in context ---
 *
 *         s32 r1;
 *         s32 r2;
 *         s32 code;
 *         s32 idx;
 *
 *         idx = i * 2;
 *         obj = arg0;
 *         flags = (&D_8009A821)[idx] << 8;
 *         ...
 *         scale = (&D_8009A820)[idx] << 8;
 */
