/* REJECTED (s1, 2026-07-21): FAKE arg0=0 dead store WITHOUT the loop-body
 * `first` precompute — measured INERT at both positions.
 *
 * Form: single-walker prologue (p = (u32*)arg0; saved = (s32)p;) with
 * `arg0 = 0; /+ FAKE +/` placed either (a) after saved=... or (b) between
 * p=... and saved=... . Both scored sandbox=14, prologue byte-identical to
 * the no-FAKE form (move s2,a0 / addu s0,s2,a1 — no $s0 staging, 52/53).
 *
 * KILL MECHANISM: without the `first` precompute, whole-function RA never
 * allocates the arg0-copy chain to $s0, so there is no materialized copy
 * for the dead store to protect; flow deletes the dead set before cse2's
 * canonical-register substitution is ever contested. The dead store only
 * has an effect ON TOP of the staged allocation (see candidate.c lever 2:
 * first = saved + (new_var2 << 2) inside the loop), where it flips the
 * second-pointer binding addu s0,a0,v0 -> addu s0,s2,v0 (11 -> 10).
 *
 * Do NOT re-propose arg0=0 as a standalone lever for this function.
 */
void AddTbpOfst_80047EE8_rejected_fake_no_staging(s32 arg0, s32 arg1);
