# Evidence bank — func_8007CA00

## Baseline (s1 recon, 2026-07-24)
- canonical: verdict **C**, 0 asm insns, 44 total, distance 13 (pure-C target).
- sandbox --disable all: **floor 13** at session start (13 regfix rules, all score-inert).
- Function: `s32 func_8007CA00(s16 *arg0)` in src/display.c:687. Reads globals
  `D_8009BE74` (switch selector, u8) and `D_8009BE77` (u8 flag).

## Semantics (from asm/funcs/func_8007CA00.s)
- switch(D_8009BE74): case1, case2, default.
- case1 & BE77!=0: return `0x400 - arg0[2] - arg0[0]`.
- case1 & BE77==0 / default: return `arg0[0]` (lh, signed).
- case2 & BE77!=0: return `0x400 - (arg0[2]/2) - arg0[0]`  (signed /2 via lhu+sll16+sra16+srl31+addu+sra1).
- case2 & BE77==0: return `((s16)arg0[0]) / 2`  (same signed /2 idiom, loaded lhu).
- **Shared tail CA40**: `subu v0,v0,v1; j CAA8; subu v0,v0,a0` = `0x400 - <val> - arg0[0]`,
  reached by case1 (fallthrough) AND case2/BE77!=0 (`j CA40`). Both keep arg0[0] in $a0.

## What the 13 regfix rules were faking (regfix.txt 2584-2591, 2708-2711)
- `lh $3,0($4)`->`lh $3,4($4)` and `lh $4,4($4)`->`lh $4,0($4)`: swap the two case1 loads
  (build put arg0[0]->v1, arg0[2]->a0; target the reverse).
- `insert j .LCA00_TAIL` + labels + `delete @28-30`: reconstruct the full 2-subu shared tail
  (build only merged the last subu, duplicating the first).
- `$2<->$3 @30-36` + `sra $2,$2,1`->`sra $2,$3,1 @38`: v0<->v1 swap in the case2/BE77==0 division.

## Measured probes (s1)
- **Variant A (explicit `goto sub` single shared tail): floor 17 — WORSE.** KILLED.
  Forcing label-sharing degrades GCC layout+RA vs natural duplication. -> rejected/goto-shared-tail-worse.c
- **Variant B (pre-subtract into temp so arg0[0] loads LAST -> stays in $a0): floor 11 — IMPROVED (-2).** CONFIRMED.
  Case1 loads now match target exactly (`lh v1,4(a0); lh a0,0(a0)`). -> candidate.c

## Remaining 11 diffs (objdump of variant B build, tmp/grind/func_8007CA00/s1/variantB_display.o)
1. **Case2/BE77==0 signed-/2 division: v0<->v1 swap (~6 diffs).** Build works the idiom in v1
   (lhu v1; sll v1; sra v0,v1; srl v1,v1; addu v0,v0,v1; sra v0,v0), target in v0 (return reg):
   lhu v0; sll v0; sra v1,v0; srl v0,v0; addu v1,v1,v0; sra v0,v1. Pure register-allocation.
2. **Tail-merge incompleteness / block placement (~5 diffs, +1 insn: build 45 vs target 44).**
   Target places shared tail CA40 right after case1 (case1 falls through), case2/BE77!=0 does
   `j CA40` (delay `li v0,0x400`). Build places the shared tail AFTER case2; case1 does `j 1394`
   with the first `subu v0,v0,v1` in the delay slot, and the first subu is DUPLICATED (1358 + 1390).

- [s1] canonical: verdict C, 0 asm insns, 44 total, distance 13 (pure-C target).

- [s1] sandbox --disable all floor = 13 at start (13 score-inert regfix rules); dropped to 11 with variant B.

- [s1] Semantics: switch(D_8009BE74); case1/BE77!=0 => 0x400 - arg0[2] - arg0[0]; case2/BE77!=0 => 0x400 - (arg0[2]/2 signed) - arg0[0]; case2/BE77==0 => ((s16)arg0[0])/2 signed; else arg0[0].

- [s1] Target shares tail CA40 (subu v0,v0,v1; j CAA8; subu v0,v0,a0) between case1 (fallthrough) and case2/BE77!=0 (j CA40); both keep arg0[0] in $a0.

- [s1] regfix rules decode to: (a) case1 load-offset/register swap, (b) full 2-subu tail-merge reconstruction, (c) v0<->v1 swap in case2/BE77==0 division.

- [s1] Variant B remaining 11 diffs = ~6 (case2/BE77==0 division v0<->v1 swap, pure RA) + ~5 (tail-merge incompleteness/block placement, +1 duplicated subu, build 45 vs target 44 insns).

## s2 (structural modality, 2026-07-24) — floor stays 11; both clusters' root causes proven; structural axis exhausted

### H1 (case2/BE77==0 division v0<->v1 swap, 6 diffs) — ROOT CAUSE PROVEN via cc1 -da greg dump
- The ==0 arg0[0] load pseudo (107 in dump) gets hard reg **v1**; it has NO register
  preference. The !=0 sibling load (pseudo 93) gets **v0**. Asymmetry is because the two
  branches have OPPOSITE output constraints: !=0's div result must be v1 (feeds `subu v0,v0,v1`
  in the shared tail); ==0's result must be v0 (direct return). The return-reg (v0) constraint
  propagates backward through the FIXED /2 idiom RTL and squeezes the load out of v0 -> v1.
  Target instead keeps n+sign in v1 and does a cross-register final shift `sra v0,v1,1`,
  leaving the load in v0.
- **The division SPELLING does not move H1** — every form canonicalizes to the same post-CSE
  RTL, and local-alloc deterministically picks v1. Measured (all floor 11 unless noted):
  `/2` direct=11; `s32 n=(s16)x; return n/2`=11; explicit `((sh>>16)+(u32)sh>>31)>>1`=11
  (disasm confirmed STILL swapped); reused-var `r=(s16)x; r=r/2; return r`=11;
  m2c manual-halving UNSHARED `(s16)t + ((u32)(t<<16)>>31)`=13 (extra insn, sll not shared);
  reuse function-scope `new_var` for the div=14 (WORSE, conflicts with !=0's new_var).
- m2c reconstructs the ORIGINAL ==0 body as the manual round-toward-zero halving
  `(s32)((s16)temp + ((u32)(temp<<16)>>31)) >> 1` — but our fork compiles it to the SAME
  swapped RA. So the original spelling is not the H1 lever.
- H1 is INDEPENDENT of H2 (the ==0 swap persisted unchanged in the reverse-goto variant).
- Frontier's s1 hypothesis "the BE77!=0 division biases the ==0 idiom's RA" is DISPROVED:
  greg shows the ==0 swap comes from the return-reg backward constraint, not from !=0
  (separate basic blocks / separate qtys).

### H2 (tail placement / +1 duplicate subu, 5 diffs) — ROOT CAUSE PROVEN via jump2 dump
- The +1 dup is a REORG delay-slot-fill artifact, NOT cross-jump incompleteness. jump2 already
  merges the FULL 2-subu suffix: cross-jump inserts `code_label 150` right before the first
  `subu` inside CASE2's block and redirects case1's jump to it (earlier->later, GCC 2.7.2's
  fixed cross-jump direction). Because case1 REACHES the tail via a jump, reorg fills that
  jump's delay slot by copying the first subu (target addr advances past it) = the duplicate.
- Target has case1 FALL THROUGH to the shared tail (tail physically between case1 and case2,
  case2 jumps UP), so no delay slot -> no dup. Achieving fall-through requires the tail to sit
  between the cases, which only an explicit goto produces — and goto degrades RA:
  reverse-goto (case2->up into case1) = build_insns 44 (dup GONE) but RA worse -> **14**;
  forward-goto (H-A, s1) = 17. Both KILLED. Natural cross-jump can't put the kept copy in case1.
- Inverting case2 branch sense (`if(BE77==0) return div;` first) = 17 (flips beqz->bnez, breaks
  the dispatch match). KILLED.

### Artifacts (tmp/grind/func_8007CA00/s2/)
- display.i.greg / .lreg / .jump2 (RTL dumps for the two root-cause proofs), display.i.* full pass set
- build_disasm via dis.sh; m2c.sh output (original structure reconstruction)

- [s2] Floor confirmed 11 with candidate.c applied to src (build_insns 45 vs target 44).

- [s2] H1 root cause (greg dump): ==0 load pseudo -> hard reg v1 with NO register preference; return-reg v0 backward-constraint through the fixed /2 RTL squeezes it out of v0. Sibling !=0 load -> v0 (its result must be v1 for the shared subu).

- [s2] H1 invariant to spelling: /2, named intermediate, explicit (x<<16)>>16 shifts, reused-var staging all = floor 11 with the swap intact (disasm verified); m2c-manual-halving unshared=13, reuse-new_var=14.

- [s2] m2c reconstructs the ORIGINAL ==0 body as manual round-toward-zero halving (s32)((s16)t + ((u32)(t<<16)>>31))>>1, and the original case/default ordering has default BETWEEN case1 and case2 - but our fork compiles both to worse or identical RA.

- [s2] H2 root cause (jump2 dump): cross-jump merges the full 2-subu suffix into case2's block (code_label 150 before the first subu) and redirects case1's jump to it; reorg then copies the first subu into case1's jump delay slot = the +1 duplicate. Target has case1 fall-through (no delay slot -> no dup).

- [s2] H1 and H2 are independent (H1 swap persists in the reverse-goto variant).

- [s2] Structural modality (spelling, var-splits, decl order, type narrowing, case/default reorder, branch-sense invert, goto both directions) is exhausted for both clusters.

## s3 (structural modality, 2026-07-27) — floor 11 -> **4**; H1 SOLVED; H2 reduced to one li-placement cluster; four GCC mechanisms proven

### Probe ladder (all sandbox --disable all; disasms in tmp/grind/func_8007CA00/s3/)
- p5 share-only-subus reverse goto (c,v1,a all cross-block): **11** (build 43) — structure right but c->$a1.
- p6 li-in-tail single-expression `(0x400 - v1) - a`: **12** (44) — GCC REASSOCIATES to (0x400-a)-v1; the s1 two-statement split is required inside the tail too.
- p7 li-in-tail split (`c=0x400; c=c-v1; return c-a;` c BLOCK-LOCAL): **10** (44) — first floor drop; c takes $v0 (local.c tick exemption).
- p8/p9/p10 per-branch t=0x400 + mid-block label (target's structural shape): **12** (43) — t->$a1, two proven walls (see mechanisms). -> rejected/per-branch-li-shared-t.c
- p11 full return-funnel (`ret: return t;` all paths): **9** (43) — DISCOVERED the H1 cure (==0 division flips to target exactly) but t->$a1 costs every t site. -> rejected/full-funnel-single-t.c
- p12 = p7 tail + funnel for ==0/case1==0/default only: **4** (44 == target). -> candidate.c
- p13 sub0 li-stub for case2: **6** (47) — t=$v0 and case1+==0 byte-exact, but reorg own-thread fill (sra1) blocks the target-thread li steal; stub survives. -> rejected/sub0-li-stub.c

### Remaining 4 diffs (p12/candidate, vs target)
1. case1 beqz delay slot: nop vs `li v0,0x400` (li is beyond the `sub` label -> not in the owned thread -> not eager-stealable).
2-4. tail cluster: build [label][li][subu v1][j][subu a0-delay] vs target [li in case1's slot][label][subu v1][j][subu a0-delay]; case2's j delay = sra v1,v1,1 (own-thread fill) vs target `li v0,0x400`.
All four are ONE configuration: both branches need their OWN li with the label after case1's li (p9 shape) — blocked by the mechanisms below.

### Proven GCC 2.7.2 mechanisms (dump- or source-verified)
- **M1 (greg dumps, s3):** global.c records conflicts SETS-BEFORE-DEATHS: a GLOBAL pseudo dying in an insn whose dest is hard $v0 (e.g. combine-folded `(set v0 (minus t a))` return) conflicts with $v0 ("75 conflicts: ... 2"). Plain copies `(set v0 t)` ARE exempt (p12: t=$v0 with the ret-copy). local.c (block-local qtys) is tick-granular — dying srcs never conflict with the dest -> block-local temps CAN take $v0. Consequence: the shared tail's 0x400 must be block-local; cross-block carriers may only be values whose target reg is not $v0 (v1->$v1, a->$a0 fine).
- **M2 (H1 root cause + cure):** the ==0 division swap was the return-reg backward squeeze THROUGH the combine-folded (set v0 (sra P 1)). Putting the return copy in its own BB (`ret:` funnel) leaves the block's last insn a pseudo-dest -> combine can't reach across the BB -> natural qty allocation = target (lhu v0, sll v0, sra v1, srl v0, addu v1, sra v0,v1,1). Verified byte-exact in p11/p12/p13.
- **M3 (sched.c, source-read + p11 greg RTL order insn 80 < 63):** GCC 2.7.2 list-schedules BACKWARD; an isolated constant-set with no in-block consumer floats to the BLOCK TOP unless adjust_priority promotes it — birthing_insn_p (sched.c:2496) requires reg_n_sets[dest]==1. A branch-shared t (2+ sets) is never promoted -> case2's `t=0x400` always hoists above the div chain -> t crosses the div's local-$v0 range -> M1 conflict. A PURE spelling cannot make t single-set (both branches must init it).
- **M4 (reorg, p13):** fill_simple_delay_slots fills an unconditional j from its OWN thread first; sra1/lh are always eligible -> the target-thread steal (copy tail-li + advance label past it, which would produce target's exact slot layout) never runs while any own-thread candidate exists.
- Eager (conditional-branch) fill can steal a trap-free insn PAST loads (pass-over needs only resource-disjointness) but never across a label (non-owned thread); loads are never stolen cross-path (may trap). Explains every beqz-slot li/nop in the probe ladder.

### Artifacts
tmp/grind/func_8007CA00/s3/: build_disasm_baseline.txt, build_disasm_p5_subonly_goto.txt, build_disasm_p6_li_in_tail.txt, build_disasm_p7_split_tail_floor10.txt, build_disasm_p9_lifirst.txt, build_disasm_p10_accum.txt, build_disasm_p11_funnel_floor9.txt, build_disasm_p12_fusion_floor4.txt, build_disasm_p13_sub0.txt, display.i.greg + full -da dump set (p11 form), dis.sh, greg.sh.

- [s3] sandbox --disable all floor at s3 start = 11 (candidate.c applied); at s3 end = 4 with build_insns 44 == target 44 (p12 form in src/display.c and candidate.c).

- [s3] Remaining 4 diffs are ONE configuration: case1 beqz slot nop-vs-li, tail li at the label instead of pre-label, case2 j delay sra-vs-li; all resolve iff both branches own their li with the label after case1's (p9 shape).

- [s3] M1: GCC 2.7.2 global.c marks sets-before-deaths -> global pseudo dying at a hard-$v0-dest insn conflicts with $v0; plain (set v0 t) copies are exempt; local.c is tick-granular so block-local temps can take $v0.

- [s3] M2: ret-funnel BB split prevents combine folding the ==0 division into (set v0 ...), eliminating the backward return-reg squeeze (H1 root cause from s2) -- ==0 block now byte-exact.

- [s3] M3: sched.c backward list scheduler floats isolated constant-sets to block top; adjust_priority's birthing_insn_p promotion requires reg_n_sets==1 (sched.c:2496-2517,2534-2575); branch-shared t (>=2 sets) is never anchored.

- [s3] M4: reorg fill_simple_delay_slots fills unconditional jumps from the own thread first (sra1/lh always eligible), so the target-thread li steal + label advance never fires (p13, build 47).

- [s3] p6 shows single-expression (0x400 - v1) - a reassociates to (0x400 - a) - v1: the two-statement split is load-bearing inside the tail as well.

- [s3] s2's 'goto both directions KILLED' verdict was overly broad: it covered only tails absorbing loads/li; finer-grained sharing (subus only) improves the floor.

## s4 (permuter modality, 2026-07-27) — floor 4 -> **0**; H2-final CLOSED via sanctioned do-while(0) fence; candidate-ready

### Campaign A (directed-case2-orderings, tmp/perm_ca00_s4, weighted base 320)
- Workspace: minimal-TU base.c (globals are lui/%hi, not gp-rel, so no context needed),
  full pipeline compile.sh (cc1|prologue_fix|maspsx|multu_pad, NO regfix — cheat-free by
  construction), target.o at offset 0. Validated: plain seed = 44/44 insns, the known 4-diff cluster.
- 8 finds in ~3 min (scores 170-300), ALL ONE ATTRACTOR CLASS = semantics-breaking: hoist the
  0x400 init into case1's block only, leaving it UNINITIALIZED on case2's goto-sub path
  (variants: double-subtract v1 in tail, `goto ret` deleted so case1 falls into case2,
  `a = arg0[0]` moved after the return). The scorer is semantics-blind; every scoring
  improvement in this basin required breaking case2's init or control flow. ZERO legal
  sub-base finds in 8.7k iterations. Harvested + stopped (telemetry in metrics/events.jsonl).
- Evidence value: the basin's attractors all converge on "case1 owns the li" = independent
  confirmation of the p9-shape diagnosis; the permuter could only reach it illegally.

### Chassis B (full-dup split-c) — measured dead without a campaign
- 46 insns (target 44), 40 diff lines: `move a1,a0` lands in the dispatch delay slot, whole
  register file cascades (loads from a1, div idiom in a0/v1). Both li DO land in delay-slot
  positions — placement right, RA cost fatal. -> rejected/full-dup-split-c.c

### Directed 6-variant matrix (p9+funnel hybrid × {plain li, do-while(0) li, c2-holder li} × {direct tail, block-local-c-copy tail})
- v_plain_*: 24/22 diff lines (the known M3/M1 walls; p9 re-confirmed).
- v_c2hold_*: 24/22 — **s3 frontier-1 (single-set constant-holder) KILLED by measurement**:
  `{ s32 c2 = 0x400; t = c2; }` changes NOTHING vs plain (copy does not become an anchored li).
  -> rejected/c2-single-set-constant-holder.c
- v_dowhile_*: **0 diff lines, BOTH tail forms** (44 == 44).

### The closing form (in src/display.c; sandbox --disable all = 0, rules_dropped 13)
- p9 shape + ret-funnel + single-level FAKE-annotated `do { t = 0x400; } while (0);` in case2.
- Mechanism: the do-while loop notes keep the isolated constant-set BELOW the div chain
  (defeats M3's backward-float, which requires sched to hoist it to block top); the li is then
  the last insn before the j, so reorg own-thread fill (M4) drops it into the delay slot =
  target. case1's li (multi-set t, no birthing promotion) floats to case1's block top per M3 —
  exactly where target wants it; eager fill steals it past the two lh loads into the beqz slot.
  With the p9 shape achieved, the direct tail `t = t - v1; return t - a;` also matches (the M1
  $v0 exclusion no longer bites; block-local-c-copy tail equally 0 — direct chosen as simpler).
- Sanction: do-while-zero-exception (owner ruling 2026-07-06 FINAL) — any codegen effect,
  single-level needs only the inline FAKE annotation; verified on-disk, not from memory.
- Layer-1 cheat-reviewer: **PASS** (independently re-ran sandbox = 0, re-read the rule file,
  ran volatile_cheats detector = no hits, verified semantics path-by-path vs target asm).

- [s4] sandbox --disable all floor: 4 at s4 start (candidate re-applied to src) -> **0** at s4 end (do-while form in src/display.c; build 44 == target 44).
- [s4] Permuter basin (floor-4 chassis): saturated by semantics-breaking attractors only; no legal pure spelling exists in that neighborhood (8 finds, 1 class, 8.7k iters).
- [s4] c2 single-set constant-holder: measured identical to plain multi-set (22/24 diff lines) — birthing/coalescing bet disproven.
- [s4] do-while(0) fence on case2's li closes ALL 4 remaining diffs; both tail forms 0; layer-1 PASS.
