# Hypothesis ledger — func_80047A90

## Session 1 (recon) — tested

### H1: pins removable, structure is right — CONFIRMED
Statement: the pinned body's structure (pointer-walk both loops) is byte-structure-correct;
the 20 is purely a register permutation.
Probe: strip pins, sandbox. Result: 20, 84/84 insns, all diffs are renames. CONFIRMED.

### H2: original loop 1 was array-indexed for-loop (loop.c produces the pointers) — CONFIRMED
Mechanism: LICM hoists Judge base (late pseudo → low alloc pressure), strength reduction
creates the walking givs, biv keeps slti.
Probe: full nested-for version. Result: loop 1 matched exactly incl. counter→t0. CONFIRMED.

### H3: original loop 2 was ALSO array-indexed for-for — KILLED
Probe: same full nested-for version. Result: 41 (87 insns) — LICM hoisted 0x7D0/0x66666667/8
which target keeps inline; sll-index addressing instead of pointers. Loop 2 in the original
was compiled WITHOUT loop notes ⇒ goto-form source. KILLED (do not re-propose for-loops or
do-while for loop 2 — do{}while(0..) also emits loop notes).

### H4: counter-init-last order sinks pt1/pt2 below a3off — CONFIRMED (floor 20→14)
Probe: `pt2; pt1; k = 1;` vs `k; pt2; pt1` (21) vs `pt2; k; pt1` (18). Result: 14 with
pt1→t1 pt2→t2 pt3→t3 pa1→a1 pa2→a2 all target. CONFIRMED, load-bearing.

### H5: both counters are one variable (target t0 in both loops) — KILLED as spelled
Probe: merged i, order pt2;pt1;i=1. Result: 17. Also merged + pt3=pa1+0x11: 17.
KILLED for these spellings — but the CONFLICT argument still favors a merged counter
(only a merged counter conflicts with BOTH a2/a3 holders forcing t0 in both loops);
the failure is priority (merged i ~11-12 refs → allocated first → a2). A merged-counter
retry must FIRST shave i's refs to ≤7 (floor_log2 drops to 2 ⇒ prio .222 < a3off .276
< pa2 .286). Do not re-run the naive merged form.

## Live frontier (for session 2)

### F1 (primary): shave ref-counts so alloc order becomes pa2 > a3off > [pt1,pt2 sunk] > counter(s)
Mechanism: global.c allocno_compare prio = floor_log2(refs)*refs/live_length. Exact current
numbers in evidence.md. The two remaining 2-cycles both need "counter allocated last".
Loop 2: k (5 refs/36 = .278) vs a3off (4/29 = .276) — shave ONE k ref or ONE k live-insn
worth of margin: candidate spellings — `k = 8` compare via a temp? `if (k == 8)` is required
semantics; try `s32 last = 8;`-free forms like comparing a3 (`a3 == 0x1C`?? NO — target
compares k reg vs 8: addiu v0,zero,8; bne t0,v0). Try: k as s16? (narrow type changes refs? no —
same). Try moving `k++` earlier/later in tail (live-length ±1-2 insns flips .278 vs .276).
Loop 1: i (7/25 = .56) must fall below judge (.12) — impossible by refs alone with 4+ refs;
instead make i live LONGER (merged counter: 7 refs/63 = .222) — see F2.

### F2 (the closing shape, likely): merged counter with exactly ≤7 total refs
i refs today when merged: init0, <17 test, ++ (loop1) + =1, ==8, ++, <9 (loop2) ≈ 11-12
(each ++ counts set+use). To reach ≤7 counted refs the loop-1 side must lose refs to
strength reduction: loop 1's biv test `i < 17` could be replaced by giv test if i were
otherwise unused — but target KEEPS slti on the counter, so i must keep exactly those refs.
Alternative: recount what lreg reports for the merged form (session 1 never dumped it —
FIRST ACTION session 2: rebuild merged form, run greg.sh, read pseudo numbers instead of
guessing). Then tune ++/test spellings (e.g. `i += 1` vs `i++`, test `i < 9` vs `i != 9`)
to hit the order pa2 > a3off > i, with pt1/pt2 sunk (init before k? — measured 14-winner
order pt2;pt1;counter-last already sinks them).

### F3 (fallback): permuter campaign seeded from the 14-floor form
The space is now small and pure-rename: permuter with register-diff scoring from the
14 base, sweeping statement order / counter spellings / init placements. Use a clean
single-function target.o (see difficult-is-not-impossible §3). Only after F1/F2 measured.

## [s1] The inherited pinned body's pointer-walk structure is byte-structure-correct; the distance is purely a register permutation
- mechanism: sandbox strips the register-asm pins; 84/84 insns match, all diffs are renames (4-cycle: counter a2->t0, judge-base a3->a2, pt1 t0->t1, a3off t1->a3)
- probe: strip all 10 pins, sandbox --disable all
- result: 20, 84/84, renames only
- verdict: CONFIRMED

## [s1] Original loop 1 was an array-indexed for-loop; the walking pointers in target are loop.c strength-reduction givs, and the Judge base is a LICM-hoisted pseudo
- mechanism: loop.c creates the givs late (low pseudo pressure) and LICM hoists the invariant base; biv keeps the slti counter test
- probe: full nested-for version, sandbox + insn diff
- result: loop 1 matched target exactly including counter->t0
- verdict: CONFIRMED

## [s1] Original loop 2 was ALSO an array-indexed for-loop
- mechanism: if loop.c saw loop 2 it would LICM-hoist the 0x7D0 / 0x66666667 / 8 constants; target keeps them inline in the inner loop
- probe: same full nested-for version
- result: 41 (87 insns): constants hoisted, sll-index addressing - loop 2 must be goto-form (no loop notes), source-level pointer walk
- verdict: KILLED

## [s1] Counter-init-LAST statement order (pt2; pt1; k=1;) sinks pt1/pt2 allocno priority below a3off, landing pt1/pt2/pt3/pa1/pa2 on target registers
- mechanism: global.c allocno_compare prio = floor_log2(refs)*refs/live_length; earlier pointer inits lengthen pt1/pt2 live ranges (37/38 insns) dropping them to .270/.263, below a3off .276
- probe: three orderings measured: k-first=21, k-middle=18, k-last=14
- result: floor 14 with pa1->a1 pa2->a2 pt1->t1 pt2->t2 pt3->t3 all target
- verdict: CONFIRMED

## [s1] Both loops' counters are one merged variable (target uses t0 in both loops)
- mechanism: only a merged counter conflicts with both loops' a2/a3 holders, forcing it into t0 twice - but merged refs ~11-12 give it TOP priority so it allocates first and takes a2
- probe: merged i, winner init order; also merged + pt3=pa1+0x11
- result: 17 both - naive merge killed; merge-with-refs-shaved-to-<=7 remains the F2 frontier
- verdict: KILLED

## [s2] Loop-2 k<->a3off cycle closes via duplicated-statement-into-arms ref-lift on the inner tail
- mechanism: duplicating `pa1++; a3 += 4; pa2++;` into both arms of if(k==8) lifts
  pa2 4->6 refs (.375) and a3off 4->6 (.364) above k (.25, live 36->40); flow counts
  refs before global alloc, jump2's cross-jump re-merges after RA byte-neutrally;
  allocation order becomes pa1(a1), pa2(a2), a3off(a3), k(t0) - all target
- probe: else-arm duplication, sandbox + slot diff + final8.lreg
- result: 14 -> 8; 84/84 insns; loop-2 slots 34-74 byte-identical incl. delay slot
- verdict: CONFIRMED (sanctioned family, FAKE-annotated; layer-1/2 review pending)

## [s2] F2 (merged counter with <=7 counted refs) is arithmetically unreachable
- mechanism: merged counter's counted-ref FLOOR is 9 (loop1 init+inc+test = 4, every one
  byte-required by target insns 1/14/27; loop2 reinit+==8+inc+test = 5, immediates pinned
  by target bytes). floor_log2(9)=3 puts prio .415 above pa2/a3off; the needed window
  (.244, .364) requires 8 refs at live>=66
- probe: merged for-form measured 12 refs/61 (lreg); merged do-while pointer-walk measured
  9 refs/65 -> sandbox 17 with k->a2, a3off->t0
- result: no byte-preserving spelling sheds the 9th ref; live extension needs a dead read
- verdict: KILLED (family closed, not just spellings)

## [s2] Full-tail duplication into the a0<0 diamond arms is not byte-neutral
- mechanism: hoped cross-jump would re-merge to target's own j .L80047B68 shared tail
  while lifting pa2/a3off to tier-3 (10 refs)
- probe: whole post-diamond tail written into both arms
- result: sandbox 24, 81/84 insns - jump optimization collapses the structure
- verdict: KILLED (the only byte-neutral duplication surface is the k==8 arm pair)

## [s2] Separate-counter loop-1 i->t0 is unreachable by allocation-priority manipulation
- mechanism: i (7/25, .56) allocates first among wrong-reg pseudos and find_reg's
  REG_ALLOC_ORDER hands it a2; the only loop-1-resident pseudos that could block a2 are
  judge (1 use inherent -> prio cap ~.17, allocates last) and the givs/temps (already on
  a0/a1/v0/v1). pa2/a3off hold a2/a3 but are loop-2-only = disjoint = non-conflicting
- probe: derived from final8.lreg conflicts + confirmed by k-first/k-middle/order probes
  and both merged forms' dispositions
- result: no separate-counter spelling can block a2 during loop 1
- verdict: CONFIRMED (negative result; kills the priority axis for loop 1 entirely)

## Live frontier (for session 3)

### F1 (primary): loop-1 residue needs a NON-priority mechanism - forensics modality
The 6-slot i<->judge cycle is closed to every allocno_compare spelling (see s2 kills).
Remaining mechanisms to investigate with the instrumented cc1 (BB2_ALLOC_DEBUG,
tools/gcc-2.7.2/cc1 per [[instrumented-cc1-location]]):
  (a) find_reg's actual hard-reg walk for pseudo 72 - is a preference/prefclass term
      steering it, and does ANY conflict/pref state make it skip a2?
  (b) local_alloc: in one merged dump the loop-1 givs were LOCAL-alloc'd (139/140) vs
      GLOBAL (138/139) in the 8-form - what flips giv local/global status, and could a
      form make I single-block (local-alloc'd) so global.c never places it?
  (c) target may have compiled loop 1 with a DIFFERENT block structure (the k=1-first
      emission at insn 30 hints the loop-2 preheader was scheduled/laid out differently).
next probe: BB2_ALLOC_DEBUG dump of find_reg for pseudo 72 in the 8-form; compare
local/global giv status across the banked dumps (final8 vs nested/nested2).

### F2: the 2-slot k=1-vs-lui order at insns 30-33
k-first/k-middle source orders break registers (18/15 measured s2). sched1 does not hoist
k=1 past the lui pair in our build but did in the original. Investigate with sched-solver
(tools/sched_solver, [[sched-solver-campaign-2026-08-05]]): what INSN_PRIORITY/dependence
state puts addiu t0,zero,1 first? Possibly falls out automatically if loop-1 registers
close first (i->t0 changes the dependence picture at the boundary).
next probe: sched_solver on the loop-2 init block of the 8-form vs target order.

### F3 (fallback): permuter campaign from the 8-floor base
Space is small (2 residues). Clean single-function target.o at offset 0, seed =
candidate.c, PERM sweep on loop-1 spellings (counter forms, giv orderings) - the permuter
may find the a2-blocker construction analysis missed. Fresh-seed discipline per
[[permuter-fresh-seed-discipline]].

## [s2] Duplicating the inner-loop tail (pa1++; a3+=4; pa2++;) into both arms of if(k==8) lifts pa2/a3off allocno priority above k and lands the whole loop-2 register set on target
- mechanism: flow.c counts reg_n_refs before global alloc (pa2 4->6 refs .375, a3off 4->6 .364, k live 36->40 .25 => order pa1(a1) pa2(a2) a3off(a3) k(t0) pt1(t1) pt2(t2)); jump2 cross-jump re-merges the duplicate AFTER RA, byte-neutrally - sanctioned family duplicated-statement-into-arms (owner 2026-07-01 + 2026-08-06 tail clarification)
- probe: else-arm duplication on the s1 14-form; sandbox + slot diff + final8.lreg dump
- result: 14 -> 8, 84/84 insns, loop-2 slots 34-74 byte-identical to target including the delay-slot addiu a2,a2,4
- verdict: CONFIRMED

## [s2] F2 (merged counter with <=7 counted refs, s1's primary frontier) is arithmetically unreachable
- mechanism: merged counter ref FLOOR is 9 (loop1 init+inc+test=4, loop2 reinit+==8+inc+test=5, every ref byte-required); floor_log2(9)=3 => prio .415 above pa2 .375/a3off .364; the needed window (.244,.364) requires 8 refs at live>=66 which no byte-preserving spelling reaches
- probe: merged for-form lreg (12 refs/61); merged do-while pointer-walk built and measured (9 refs/65)
- result: merged do-while = sandbox 17 with k->a2/a3off->t0/jb->a3; ref floor derivation banked in evidence.md
- verdict: KILLED

## [s2] Duplicating the full post-diamond tail into the a0<0 arms lifts pa2/a3off to tier-3 while cross-jump re-merges to target's j-shared-tail shape
- mechanism: same reg_n_refs lift, larger surface; requires jump2 to reconstruct the exact then-arm j+delay pattern
- probe: whole tail duplicated into both diamond arms, sandbox
- result: 24 with 81/84 insns - structure collapses; byte-neutrality prerequisite fails; only the k==8 arm pair is a valid duplication surface in this function
- verdict: KILLED

## [s2] Separate-counter loop-1 can reach i->t0 by any allocation-priority spelling
- mechanism: i (7/25, prio .56) allocates first and REG_ALLOC_ORDER hands it a2; blocking a2 requires an earlier-allocated CONFLICTING pseudo, but the only loop-1-resident candidates are judge (1 inherent use, prio cap ~.17, allocates last) and givs/temps already on a0/a1/v0/v1; pa2/a3off hold a2/a3 but are loop-2-disjoint hence non-conflicting
- probe: final8.lreg conflict analysis + k-first(18)/k-middle(15)/outer-head-order(17,18) probes + both merged forms' greg dispositions
- result: no a2-blocker is constructible; the 6-slot i<->judge residue is closed to the entire priority axis
- verdict: KILLED
