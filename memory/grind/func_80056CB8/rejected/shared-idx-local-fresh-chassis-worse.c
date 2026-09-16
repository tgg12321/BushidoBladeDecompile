/* REJECTED — func_80056CB8, s32 (2026-09-16, forensics modality). KILLED
 * (instance), not a cheat — plain register-pressure regression, RE-MEASURED
 * on the CURRENT 38/204 chassis (the original shared-idx-local-worse.c kill
 * was measured on a stale s6 chassis, 81/197 baseline; this is the fresh
 * re-audit the ledger's KILL RE-AUDIT REQUIRED section asked for).
 *
 * Change from the s22-s31-banked 38/204 body: added `s32 idx;` declared in
 * the loop body, `idx = i * 2;` computed once right after `obj = arg0;`,
 * and both `(&D_8009A821)[i * 2]` / `(&D_8009A820)[i * 2]` reads rewritten
 * to `[idx]`. Nothing else changed.
 *
 * Measured: score REGRESSED 38 -> 51 (build_insns 198 -> 200) via
 * `sandbox func_80056CB8 --disable all`.
 *
 * Mechanism, PROVEN with a fresh .loop dump this session (not inferred):
 * this DOES flip loop.c:3823's giv-worth test for the i*2 giv — with `idx`
 * as an explicit shared pseudo, its own lifetime balloons to 42 (dump:
 * "Insn 41: giv reg 89 src reg 75 benefit 2 used 1 lifetime 42 replaceable
 * mult 2 add 0"), because `idx`'s own def-to-last-use LUID span now covers
 * the entire loop body (both index sites plus everything between them,
 * including the ratan2 call). loop.c's strength_reduce DOES promote it
 * this time — "giv at 130 combined with giv at 41" / "giv at 48 combined
 * with giv at 41" / "giv at 41 reduced to (reg:SI 217)" and NO "not worth
 * while" rejection line for insn 41 at all (contrast the un-shared 38/204
 * baseline, which rejects the merged giv at "124 vs 163" — see the s31 and
 * this-session's exact-arithmetic decomposition below). Loop real-insn
 * count even DROPS from 163 to 158 (the two `sll #,2` recomputations are
 * eliminated, replaced with one base-register add). But build_insns still
 * RISES (198 -> 200) and score regresses (38 -> 51): promoting idx into a
 * real accumulator forces it to live in a register across the whole loop
 * body INCLUDING the `ratan2` call, i.e. it becomes a 9th call-spanning
 * value competing for the 8 already-full callee-saved slots (obj, obj2,
 * i, flags/code, sin_p, cos_p, x, z — see s29's conflict-map audit),
 * which costs more in spill/reload/allocation churn than the 2 insns the
 * strength-reduction saved. This is the SAME mechanism s6's stale-chassis
 * kill identified, now independently reconfirmed on the CURRENT chassis
 * with a precise dump-level mechanism instead of a score-only measurement.
 *
 * CLASS-LEVEL IMPLICATION (still an instance kill per the ledger's kill-
 * scope discipline, since only ONE spelling of "shared idx" was tested):
 * any C spelling that merges the two i*2 reads into a single shared
 * variable whose live range spans the ratan2 call will hit this same
 * register-pressure wall, because the call site sits structurally between
 * the two reads (flags-table read at line ~1111, scale-table read at line
 * ~1124, with the flags/obj dispatch + ratan2 call in between) and cannot
 * be moved without changing the loop's real semantics/byte output.
 */
