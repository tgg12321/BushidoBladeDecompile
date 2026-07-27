# Hypothesis ledger — func_8007CA00

## Floor history
- s1 recon (2026-07-24): 13 -> **11** (variant B, sequencing-temp lever). candidate.c saved.
- s2 structural (2026-07-24): floor stays **11**. Both clusters' root causes proven (greg + jump2
  dumps); the structural axis (spelling, var-splits, decl order, type narrowing, case/branch
  reordering, goto both directions) is measured DEAD for both. Handoff -> permuter modality.
- s3 structural (2026-07-27): 11 -> **4** (candidate.c = partial return-funnel + label-shared
  2-subu tail with block-local 0x400). **H1 SOLVED** (==0 division byte-exact via the ret-funnel
  BB split). H2 reduced to a single li-placement cluster with the blocking GCC mechanisms
  fully identified (M1-M4 in evidence s3). s2's "goto both directions dead" verdict was
  OVERTURNED for finer-grained sharing: the kill only covered tails that absorb the loads/li.

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

## CONFIRMED (s3)
- **H-C: label-sharing ONLY the 2-subu tail (loads + li per-branch/block-local) beats both
  natural cross-jump and coarse goto-sharing.** p7=10, p12=4. The s1/s2 goto kills applied only
  to tails that absorbed the loads/li.
- **H-D: a `ret:` return-funnel BB fixes H1.** The ==0 division's last insn stays a pseudo-dest
  in its own BB; combine can't fold across the label; the backward $v0 squeeze disappears;
  allocation = target exactly. (p11 discovered; p12/p13 confirm.)
- **H-E: block-local vs global pseudo asymmetry for $v0** (M1): tail constant must be
  block-local; v1/a may cross blocks (their target regs aren't $v0).

## KILLED (s3)
- **Single shared t carrying 0x400 from both branches into the tail** (p9/p10 = 12): sched.c
  backward-float (no birthing promotion for multi-set t) + global.c sets-before-deaths hard-$v0
  conflict at the combined return subu. Dead by mechanism, not just measurement.
- **Full funnel through one t** (p11 = 9): same t->$a1 taxes every site. Partial funnel (p12) wins.
- **sub0 li-stub for case2** (p13 = 6, build 47): reorg own-thread fill (sra1) preempts the
  target-thread li steal; stub survives +3 insns.
- **Single-expression tail `(0x400 - v1) - a`** (p6 = 12): reassociates to (0x400-a)-v1;
  two-statement split mandatory.

## LIVE FRONTIER (s3, from floor 4)
- **H2-final (4 diffs): both branches need their OWN li v0,0x400 with the `sub` label after
  case1's li** — the p9 shape, blocked purely by M3 (multi-set t never birthing-promoted).
  Next probes: (a) single-set constant-holder local per branch feeding t via a
  greg-coalesced copy (`s32 c2 = 0x400; t = c2;`) — reg_n_sets(c2)==1 triggers the sched
  promotion, copy should coalesce away; BUT this is a constant-holder local =
  [[named-local-fake-exception]] territory (FAKE annotation + documented lever-exhaustion +
  layer-2). Evaluate exhaustion honestly first. (b) Directed permuter from the floor-4
  candidate (vastly better seed than s2's 11) sweeping tail/branch orderings for a pure
  spelling that dodges M3/M4. (c) Any spelling that leaves case2's j slot with NO eligible
  own-thread insn so reorg steals the tail li + advances the label (M4) — none found yet.

## LIVE FRONTIER (superseded s2 items below — kept for history)
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

## [s3] Label-sharing ONLY the 2-subu tail (loads + 0x400 kept per-branch/block-local) avoids the s1/s2 goto-sharing RA cascade.
- mechanism: The old kills shared tails that absorbed the loads/li; sharing just the subtractions leaves v1->$v1 and a->$a0 as the only cross-block pseudos (neither needs $v0), and a block-local c takes $v0 via local.c's tick exemption.
- probe: p5 (share-nothing-in-tail)=11, p6 (single-expr tail)=12 (reassociates), p7 (split-statement tail, block-local c)=10, p12 (p7 + partial funnel)=4; disasms banked.
- result: Floor dropped 11->10->4; tail subu order and $v0 chain now match target.
- verdict: CONFIRMED

## [s3] A shared `ret: return t;` funnel in its own basic block fixes H1 (the ==0 division v0<->v1 swap).
- mechanism: Per-path direct returns let combine fold the division tail into (set $v0 ...), and the return-reg constraint squeezes the load out of $v0. With the return copy in a separate BB, combine cannot reach it, the ==0 block's last insn keeps a pseudo dest, and natural qty allocation produces target's lhu v0/sll v0/sra v1/srl v0/addu v1/sra v0,v1,1 exactly.
- probe: p11 full funnel: ==0 block flipped to byte-exact target form (build_disasm_p11_funnel_floor9.txt); retained in p12/p13.
- result: H1's 6 diffs eliminated; ==0 block byte-identical to target.
- verdict: CONFIRMED

## [s3] One shared variable t can carry 0x400 from both branches into the tail with t=$v0 (the p9 shape = target's structural source form).
- mechanism: Two independent walls: (1) sched.c list-schedules backward and only birthing_insn_p (reg_n_sets==1) insns get the max-priority promotion that anchors an isolated constant-set late; branch-shared t has 2+ sets, so case2's li always hoists above the div chain, making t overlap the div's local-$v0 range. (2) global.c records sets-before-deaths, so t dying at the combine-folded (set v0 (minus t a)) conflicts with hard $v0 (greg: '75 conflicts: ... 2 3'); plain copies are exempt (proven by p12's t=$v0).
- probe: p9/p10 measured 12 (t->$a1, accumulator respelling byte-identical); p11 greg RTL shows insn 80 (li) hoisted above insn 63 (lhu); sched.c:2496 birthing_insn_p and adjust_priority read and cited.
- result: Dead by mechanism: no pure spelling can make a branch-shared t single-set.
- verdict: KILLED

## [s3] A stub block `sub0: t = 0x400; goto sub;` lets case2 acquire its li via reorg target-thread steal (li into j delay slot + label advance), completing the p9 shape without sched hoisting.
- mechanism: Sched cannot hoist across BBs, and reorg's fill_slots_from_thread can copy a target-thread li into the jump's delay slot and advance the target past it -- but fill_simple_delay_slots consults the jump's OWN thread first, and sra v1,v1,1 (or the a-load) is always an eligible own-thread candidate, so the steal never runs.
- probe: p13: sandbox 6, build 47; t=$v0 achieved, case1 + ==0 byte-exact, but stub survives as [j sub; li-delay] and ret needs a jump around it.
- result: Stub form dead as a final form; proves t=$v0 is reachable and that M4 (own-thread-first) is the last blocking mechanism.
- verdict: KILLED
