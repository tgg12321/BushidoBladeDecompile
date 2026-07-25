# Hypothesis ledger — func_8007CA00

## Floor history
- s1 recon (2026-07-24): 13 -> **11** (variant B, sequencing-temp lever). candidate.c saved.
- s2 structural (2026-07-24): floor stays **11**. Both clusters' root causes proven (greg + jump2
  dumps); the structural axis (spelling, var-splits, decl order, type narrowing, case/branch
  reordering, goto both directions) is measured DEAD for both. Handoff -> permuter modality.

## CONFIRMED
- **H-B: sequencing arg0[0] as the last subtrahend keeps it in $a0, fixing case1 load swap.**
  Mechanism: `t = 0x400 - arg0[2]; return t - arg0[0];` makes arg0[0] the last load from the
  base ptr, so GCC reuses $a0 for it (matching case2 where a0=arg0[0]). floor 13->11. -> candidate.c

## KILLED
- **H-A: explicit `goto` to a single shared tail (CA40).** Probe scored 17 (worse than 13).
  Forcing label-sharing degrades layout/RA; GCC's natural tail duplication is better.
  Do NOT re-propose explicit goto/label-sharing for the CA40 tail. -> rejected/goto-shared-tail-worse.c

## KILLED (s2, structural)
- **H1 not addressable by division spelling.** ==0 load->v1 is a local-alloc tie driven by the
  return-reg backward constraint through the fixed /2 RTL. 6 spellings measured (see evidence s2),
  all stay 11 (disasm confirms swap persists) or worse. RTL canonicalizes identically.
- **H1 independent of H2** (swap persists in the goto variant). s1 "!=0 biases ==0 RA" DISPROVED.
- **H2 reverse-goto tail-share** (case1 fall-through, case2 goto up): 14. Fixes build_insns to 44
  (dup gone) but over-shares loads -> RA cascade. -> rejected/reverse-goto-tail-share.c
- **H2 default-between-cases** (m2c original ordering): 25 under our fork. -> rejected/default-between-cases.c
- **case2 branch-sense invert** (==0 first): 17 (breaks dispatch beqz->bnez).
- H2 +1 dup PROVEN a reorg delay-slot-fill artifact (case1 jumps to the tail; reorg copies the
  first subu into the delay slot). Only case1 fall-through avoids it, and that needs the tail
  between the cases (goto-only, which degrades RA both directions).

## LIVE FRONTIER (from variant B floor 11)
- **H1 — case2/BE77==0 division v0<->v1 swap (~6 diffs, biggest cluster).**
  Build works the signed `/2` idiom through $v1; target through $v0 (the return reg).
  Next probe: steer the loaded value into $v0. Try spelling the return so the load result
  is used directly as the return chain (avoid the intermediate that GCC parks in v1); a
  `-da` greg dump on this branch will show why pseudo lands in v1. Consider that the
  BE77!=0 division (which DOES want v1, to feed the tail) may be biasing the shared idiom's RA.
- **H2 — full tail merge + block placement (~5 diffs, +1 insn).**
  Target: shared tail after case1 (fallthrough) + case2 `j CA40`; build duplicates the first
  subu and jumps case1 down. This is GCC basic-block layout; goto (H-A) made it worse.
  Next probe: permuter (directed) on variant B — block ordering is not directly C-controllable;
  let the permuter search reorderings. Also try making case2/BE77!=0 present its value in the
  same register/shape as case1 so jump2 can extend the merge upward without a forced goto.

## [s1] Explicit goto to a single shared CA40 tail matches target's structural sharing.
- mechanism: Force label-sharing so GCC emits one shared 2-subu tail like target instead of duplicating it.
- probe: Rewrote case1/case2 to `goto sub` a shared `return (0x400 - v1) - arg0[0];`; sandbox --disable all.
- result: floor 17, worse than 13 baseline; forced sharing degrades GCC layout/RA.
- verdict: KILLED

## [s1] Sequencing arg0[0] as the last subtrahend keeps it in $a0, fixing the case1 load register/offset swap.
- mechanism: `t = 0x400 - arg0[2]; return t - arg0[0];` makes arg0[0] the final load from the base ptr so GCC reuses $a0 for it, matching case2 (a0=arg0[0]) and the shared tail's operand layout.
- probe: Rewrote case1 and case2/BE77!=0 to pre-subtract into a temp, then subtract arg0[0]; sandbox --disable all + objdump.
- result: floor 13->11; case1 loads now `lh v1,4(a0); lh a0,0(a0)` exactly matching target.
- verdict: CONFIRMED

## [s2] The case2/BE77==0 division v0<->v1 swap (6 diffs) can be fixed by re-spelling the division / adding named intermediates / explicit shifts.
- mechanism: The ==0 arg0[0] load pseudo gets v1 (no reg preference); the return-reg (v0) constraint propagates backward through the fixed signed-/2 idiom RTL and squeezes the load out of v0. The sibling !=0 load gets v0 because its result must be v1 (feeds the shared subu). Target keeps n+sign in v1 and does a cross-register final sra v0,v1,1.
- probe: cc1 -da greg dump + 6 C spellings measured: /2 direct=11, named-intermediate=11, explicit-shift=11 (disasm confirms swap persists), reused-var=11, m2c manual-halving unshared=13, reuse-new_var=14. RTL canonicalizes identically across all spellings.
- result: All spellings stay 11 (or worse); the swap is a deterministic local-alloc tie invariant to the ==0 expression.
- verdict: KILLED

## [s2] The BE77!=0 division biases the ==0 idiom's register allocation (s1 frontier hypothesis).
- mechanism: Shared idiom shape across the two case-2 sub-branches was thought to couple their RA.
- probe: greg dump shows the two loads are separate basic blocks / separate qtys; the ==0 swap is driven solely by the return-reg backward constraint. The swap also persists unchanged in the reverse-goto variant that restructures case1/case2.
- result: H1 is independent of both the !=0 branch and of H2.
- verdict: KILLED

## [s2] The tail-merge +1 duplicate subu (H2, 5 diffs) can be removed by restructuring the shared tail (reverse goto so case1 falls through / matching m2c's default-between-cases ordering).
- mechanism: jump2 already merges the full 2-subu suffix (label before the first subu in case2); case1 REACHES it via a jump, so reorg fills the jump delay slot by copying the first subu = the dup. Fall-through would avoid it but needs the tail physically between the cases.
- probe: reverse-goto (case1 fall-through, case2 goto up)=14 (build_insns 44, dup gone, but over-shares the loads -> RA cascade); default-between-cases (m2c order)=25; case2 branch-sense invert=17; forward-goto (H-A, s1)=17.
- result: Every explicit tail-sharing / reordering degrades RA; natural cross-jump cannot place the kept copy in case1. +1 dup proven a reorg delay-slot-fill artifact.
- verdict: KILLED
