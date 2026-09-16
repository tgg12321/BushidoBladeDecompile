/* REJECTED — func_80056CB8, s10 (2026-09-16). KILLED (instance), not a cheat —
 * register-pressure regression, same family as s6's shared-idx-local-worse.c
 * but a DIFFERENT spelling (loop-carried induction variable vs in-body local).
 *
 * MOTIVATION (from reading asm/funcs/func_80056CB8.s directly, full file,
 * this session): target's asm shows $fp initialized ONCE before the loop
 * (`sll $fp, $v1, 2` = i_initial*2) and incremented by a PLAIN `addiu $fp,
 * $fp, 0x2` at the loop's bottom (.L80056FB0), in lockstep with the real
 * loop counter $s6 (`addiu $s6, $s6, 0x1` = i++). $fp is read at BOTH the
 * D_8009A821 lookup (line ~31: `addu $at,$at,$fp`) and the D_8009A820
 * lookup (line ~68) via the SAME accumulator — i.e. target keeps `i*2` as a
 * genuine second loop-carried induction variable, not a recompute-from-i
 * expression. This looked like a different, untried spelling from s6's
 * shared-idx-local-worse.c (which computed `idx = i*2;` FRESH inside the
 * loop body each iteration, still killed as instance evidence).
 *
 * TWO VARIANTS MEASURED THIS SESSION on the current (s7-merged, floor-58)
 * chassis:
 *   (a) `s32 idx2 = i * 2;` declared fresh INSIDE the loop body (identical
 *       shape to s6's already-rejected form, re-tested here to confirm the
 *       s6 kill generalizes across the s7 flags/ang/code merge chassis
 *       change): score 58 -> 68 (build_insns 198 -> 200). WORSE.
 *   (b) `idx2` promoted to a genuine loop-carried induction variable —
 *       declared in the OUTER function scope alongside `i`/`start`,
 *       initialized in the for-statement's init-clause (`idx2 = start * 2`)
 *       and incremented in the for-statement's increment-clause
 *       (`idx2 += 2`), exactly mirroring target's asm structure: score
 *       58 -> 73 (build_insns 198 -> 201). WORSE, and worse than variant (a).
 *
 * CONCLUSION: sharing the i*2 value between the two byte-table lookups —
 * in ANY spelling tried so far (fresh in-body local, loop-carried
 * induction variable) — raises register pressure across the intervening
 * ratan2()-call/obj/flags computation more than it saves, regardless of
 * whether the shared value is a per-iteration temporary or a genuine
 * induction variable. Two spellings now killed, both instance kills, on
 * two different chassis (s6-baseline and s7-merged). Target's own actual
 * $fp reuse is real (confirmed directly in the raw asm this session) but
 * is NOT reached by naming/sharing the VALUE — the remaining unexplored
 * axis (per s6's original rejected-form note, still not tried) is a
 * shared BASE POINTER local (e.g. `u8 *tbl = &D_8009A821 + idx;`) or
 * accepting this is a scheduling/delay-slot artifact independent of any
 * C-level index-variable sharing.
 *
 * Do not re-propose either spelling of "one C variable holds i*2, shared
 * by both byte-table reads" — both directions (recompute-per-iteration,
 * loop-carried-induction) are now measured-worse on this chassis.
 * ------------------------------------------------------------------- */
