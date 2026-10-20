/*
 * func_8004A76C — no C candidate exists.
 *
 * asm/funcs/func_8004A76C.s is a 39-insn hand software-pipelined GTE
 * (cop2) perspective-transform loop (rtpt + mfc2 of GTE result regs,
 * lwc2/swc2 vertex I/O). GTE ops have no C form (AGENTS.md inline-asm
 * policy). scan_hand_coded --single: tier=STRONG 1/8 (S7, unsaved $s0).
 * The lone caller func_8004A4E0 hands $s0 live across the call in a way
 * GCC cannot emit (its own canonical audit); the loop body's rtpt/entry-j
 * pipelining shape is hand-scheduled, not GCC-schedulable. Closed by
 * docs/audits/INLINE-ASM-AUDIT-2026-10-01.md item A6 ("CLOSED, no
 * change"); re-confirmed memory/grind/func_8004A76C/hypotheses.md s1 H1
 * (class kill). src/main/3AB48.c:30 stays
 * INCLUDE_ASM("asm/funcs", func_8004A76C);
 */
/*
 * s5 (synthesis): two honest-C syntheses measured and banked under
 * rejected/ (33 natural loop, 30 hand-pipelined goto form) -- both move the
 * vertex pointer from the target's unsaved $s0 to $a3, a false ABI claim
 * against the hand-asm caller, and the sandbox strips gte_rtpt() (and local
 * ldv3c/stsxy3c) as unpinned engine/gtemacro.py units, so neither score is a
 * shippable floor. The stub above stays the candidate.
 */
/*
 * s7 (forensics): raw instrumented cc1 -da probes (tmp/grind/func_8004A76C/s7/)
 * narrowed the residual of the closest form
 * (rejected/psyq-rtpt-pair-reorg-slot-residual.c) to four classes: $s0 base,
 * $t0-$t2 mfc2 seats, and two reorg.c delay-slot facts (beqz slot holds the
 * a0 decrement while the eligible `addiu a1` sits right before the branch;
 * `jr ra` slot left as nop after an eligible sh). The nop pad before the first
 * rtpt alone is reachable (PsyQ gte_rtpt + gte_rtpt_b). The stub stays.
 */
/*
 * s8 (rederive): fresh m2c decompile (tmp/grind/func_8004A76C/s8/m2c_out.c) independently
 * lands on the s1 ABI fact -- it emits a two-parameter signature and never gives $s0 a
 * variable, confirming no formal-parameter slot exists for the real third input. A fresh
 * decomp.me corpus similarity search (461 cached scratches) returns a null result (best
 * similarity 0.011, no GTE-shaped match). The sibling func_8004A4E0's candidate.c is
 * comments-only (nothing to transplant); Kengo is permanently deleted. No rederive-axis
 * frontier remains; the stub stays.
 */
/*
 * s11 (rederive): fresh independent m2c run (tmp/grind/func_8004A76C/s11/m2c_out.c) reconfirms
 * s8 exactly -- two-parameter signature, no $s0 slot, every cop2/GTE opcode M2C_ERROR'd. Sibling
 * func_8004A4E0's candidate.c re-read in full: unchanged, still comments-only, its own s11/s12
 * independently hit the same null rederive result for its own body. Mandated kill re-audit on the
 * closest rejected form (rejected/psyq-rtpt-pair-reorg-slot-residual.c): fake_ablate.py confirms
 * zero FAKE constructs (kill was never FAKE-propped); re-measured --diff on current chassis, 5/5
 * hunks source-level because engine/gtemacro.py still strips the unpinned gte_rtpt/lwc2/swc2/mfc2
 * block wholesale -- same operator-only engine-pin gap as s5/s7/s10, not a new residual. No
 * rederive-axis or kill-re-audit frontier remains; the stub stays.
 */
/*
 * s9 (forensics): kill re-audit -- rejected/psyq-rtpt-pair-reorg-slot-residual.c scores 32 on the current
 * stub chassis (5 source-level hunks, no FAKE construct to ablate). The jr $ra slot predicate is now
 * enumerated: reorg.c:2969 backward scan from return_internal stops only at a CODE_LABEL/jump/asm
 * (stop_search_p, reorg.c:719-745) or a volatile/conflicting store. An in-loop return
 * (rejected/return-in-loop-label-empty-jr-slot-extra-jump.c) leaves the slot empty but adds a jump and
 * inverts the exit layout. No PsyQ inline header writes mfc2 into $8-$10 or clobbers $2/$3. The stub stays.
 */
/*
 * s12 (rederive): third independent m2c run, byte-identical in structure to s8/s11 (no $s0
 * formal param, every cop2 op M2C_ERROR). Sibling func_8004A4E0's candidate.c re-read at its
 * current state (floor 163, 23 sessions) -- still pure INCLUDE_ASM, zero C, nothing to
 * transplant. No new rederive axis opened. The stub stays.
 */
/*
 * s13 (structural): mandated kill re-audit on rejected/psyq-rtpt-pair-reorg-slot-residual.c --
 * fake_ablate confirms zero FAKE constructs (again); --diff on the current chassis still shows
 * 5/5 source-level hunks, all the stripped lwc2/c2-rtpt/swc2/mfc2 GTE opcodes (engine/gtemacro.py
 * unpinned-unit strip, same operator-only gap as s5/s7/s9/s10/s11). s10's six-variant z-temp
 * sweep (s16/u16/reversed-decl/array/fused-asm/narrow-sz) already exhausts the one place a
 * structural var-split/decl-order/type-narrowing lever could move this chassis's register seats;
 * none reach the target's $t0-$t2, so no new structural probe was worth separately measuring.
 * No structural-axis frontier remains. The stub stays.
 */
/*
 * s15 (enumerate): tools/spelling_enum.py on the mfc2/sh/branch skeleton
 * that survives gte_rtpt/ldv3c/stsxy3c macro stripping (the `mid:` block of
 * rejected/psyq-rtpt-pair-reorg-slot-residual.c) enumerates an exhaustive
 * 6-member spelling space (8 anchors, 3 assignments, 0 named locals;
 * identical with and without the commutative-swap axis -- no swappable
 * pairs exist in this region). All 6 variants
 * (tmp/grind/func_8004A76C/s15/enum/v0..v5.c) score 32 -- zero movement
 * from the un-enumerated form. Class-killed for this region: no
 * byte-preserving statement reorder of the surviving skeleton changes the
 * score, because the dominant scored residual (hunks 1/2/4, the whole
 * lwc2/c2-rtpt/swc2 macro expansions) is stripped by name-pattern match in
 * engine/gtemacro.py's PINNED table regardless of surrounding C. The stub
 * stays; see hypotheses.md s15 for the full histogram.
 */
/*
 * s16 (enumerate): widened spelling_enum beyond s15's mid: block to the
 * prologue/loop/epilogue blocks -- all three are degenerate (1-member,
 * nothing to permute). The spelling space is now fully mapped across all
 * four blocks; mid: is the only one with real freedom and it was already
 * swept zero-hit at s15. The stub stays.
 */
/*
 * s17 (enumerate, repeat dispatch; mandated kill re-audit): fake_ablate.py
 * on rejected/psyq-rtpt-pair-reorg-slot-residual.c again reports zero FAKE
 * constructs; a fresh --diff re-measurement reproduces score 32/37, 5/5
 * hunks source-level (unpinned GTE macro strip), 0 operand-only, 0
 * not-scored -- identical in shape to every prior re-audit (s9/s13/s14).
 * No new enumerate-axis frontier exists beyond s15/s16's fully-mapped
 * search space. The stub stays; see hypotheses.md/evidence.md s17.
 */
/*
 * s18 (synthesis): mandated kill re-audit -- fake_ablate finds zero FAKE
 * constructs in both closest banked forms; sandbox --diff re-measures 32
 * (psyq-rtpt-pair) and 30 (pipelined-goto), 5/5 source-level each; stub 37.
 * New diagnostic: granting the $s0 ABI with a (wall-banned) file-scope
 * register variable still leaves mfc2 seats $2/$3/$7, addu a1 before the
 * mfc2s and a filled jr slot -- port cc1 == cc1psx
 * (rejected/diag-global-register-s0-wall-seats-v0-v1-a3-jrslot.c). The
 * three non-ABI residuals are independent of the ABI question. Stub stays.
 */
/*
 * s19 (solver): ra_solver model of the closest $a3 form is exact (7/7). inverse.py depth 3 is
 * NEGATIVE for the full target disposition and for the ABI-granted goal: the mfc2 temps
 * conflict only with $sp, so first-fit gives $v0/$v1/$t0, and $t0-$t2 never appear pre-RA to
 * be preferred. classify on the sandbox objects reports PRE-RA only because of the GTE strip.
 * The stub stays.
 */
/*
 * s20 (forensics): re-audit holds (32, 5/5 source-level, zero FAKE). Raw -da dumps of the closest
 * form show z1-z3 are global pseudos (live across blocks 1/3) with only $sp as hard conflict, so
 * local-alloc is not the deciding pass and global.c first-fit seats them $v0/$v1/$t0. label_census:
 * CFG identical to target. Residual stays ABI + seats + reorg slots; canonical grant
 * inline_asm_canonical.txt:25 stands. The stub stays.
 */
/*
 * s21 (forensics): re-audit holds (32, 5/5 source-level, zero FAKE). nrefs_census: allocno
 * order is inert for the mfc2 seats (conflicts pseudos+$sp only, first-fit $2 at any rank);
 * loop.c sees no loop in the goto form. Canonical grant inline_asm_canonical.txt:25 stands.
 * The stub stays.
 */
/*
 * s22 (rederive, mandated): re-ran decomp.me similarity search against the current
 * 461-scratch corpus (tmp/grind/func_8004A76C/s22/decomp_me_search_461.txt) -- byte-identical
 * to s8's result (best similarity 0.011, no GTE-shaped match); the corpus held 461 scratches
 * already at s8, so no growth occurred to re-search. Re-read m2c_out.c (s12) in full and
 * tools/m2c/m2c_macros.h: cop2 GTE opcodes have no C lowering under any m2c mode, a structural
 * fact about the instruction set, not a search/tool choice to route around. Sibling
 * func_8004A4E0's candidate.c re-read in full, unchanged since 2026-10-15 17:03, still
 * comments-only -- nothing to transplant. sandbox --disable all re-measured 37
 * ({"score":37,"no_c_body":true,"build_insns":0}). No new rederive-axis frontier opened;
 * the fourth independent rederive pass (s8/s11/s12/s22) lands on the identical null. The
 * stub stays.
 */
