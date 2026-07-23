/* func_80045878 — REJECTED Gap-B structural probes (s3, 2026-07-23).
 * Two disproven tail spellings, both leave floor >= 10:
 *
 * (1) s1[11] = s3;  (reuse the live s3 = a0+3 instead of recomputing a0+3)
 *     sandbox --disable all: build_insns 107 -> 105, score stays 10.
 *     WRONG BYTES: target RECOMPUTES a0+3 in the tail (`addiu v1,s2,3`),
 *     it does NOT reuse s3 (reg 19 is REG_DEAD before the tail, greg insn
 *     193). Reusing s3 drops 2 insns (no fresh recompute + no reload) and
 *     mis-aligns; not a match path. Purpose: probe whether keeping s3 live
 *     into the join raises pressure enough to trigger the s1->v0 base-copy
 *     split. It does not (build got SHORTER, not the extra copy).
 *
 * (2) Tail-store reorder (interleave a0 / a0+3 / word stores):
 *       s1[2]=a0; s1[11]=a0+3; s1[10]=a0; *(s1+0x18)=0x8000; s1[8]=a0; s1[4]=a1;
 *     sandbox --disable all: build_insns 107, score 10 (IDENTICAL to HEAD).
 *     cse re-clusters the three (subreg:HI s2) a0 stores and re-materializes
 *     the shared (HI)s2 temp (greg insn 213) regardless of source order =>
 *     the `move v0,s2` + base-in-s1 outcome is order-insensitive. Source
 *     statement order is NOT a lever for the Gap-B (HI)s2 CSE.
 *
 * Gap B root (confirmed at RTL, greg dump tmp/grind/func_80045878/s2 +
 * this session): our fork's cse materializes (subreg:HI s2) into reg2/v0
 * (greg insn 213, REG_DEAD s2) and reuses it for the three s1[N]=a0 HI
 * stores, so a0 dies mid-tail and the base stays in s1 (`sh v0,off(s1)` +
 * `move v0,s2`). TARGET stores s2 DIRECTLY (`sh s2,off(v0)`, s2 stays live)
 * and copies the base s1->v0 (`addu v0,s1,zero`). Same ONE callee->caller
 * copy, opposite choice of WHICH value gets it. A pure local-alloc/cse
 * coin-flip; no clean structural C spelling flips it (p=s1 copy-props, s3
 * reuse changes bytes, reorder is inert, no legitimate pressure hook at the
 * 2-pred join). -> directed permuter axis.
 */
