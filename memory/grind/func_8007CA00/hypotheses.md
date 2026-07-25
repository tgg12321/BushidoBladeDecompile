# Hypothesis ledger — func_8007CA00

## Floor history
- s1 recon (2026-07-24): 13 -> **11** (variant B, sequencing-temp lever). candidate.c saved.

## CONFIRMED
- **H-B: sequencing arg0[0] as the last subtrahend keeps it in $a0, fixing case1 load swap.**
  Mechanism: `t = 0x400 - arg0[2]; return t - arg0[0];` makes arg0[0] the last load from the
  base ptr, so GCC reuses $a0 for it (matching case2 where a0=arg0[0]). floor 13->11. -> candidate.c

## KILLED
- **H-A: explicit `goto` to a single shared tail (CA40).** Probe scored 17 (worse than 13).
  Forcing label-sharing degrades layout/RA; GCC's natural tail duplication is better.
  Do NOT re-propose explicit goto/label-sharing for the CA40 tail. -> rejected/goto-shared-tail-worse.c

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
