# Hypothesis ledger — func_800770B8

## Session 1 (recon, 2026-09-01) — floor 175 → 26 → 20 → 15 → 14

CONFIRMED:
- H1 "function returns 1, not void" — target's pre-epilogue `addiu $v0,$zero,1`
  is a live return value; `return 1;` + caller prototype fix is load-bearing
  (part of the 26-floor clean chassis; the chassis-era tail rules @2173-2187
  largely evaporated).
- H2 "the srav+andi vs sllv+and diff is the tree-level single-bit fold" —
  `s32 mask = 1 << idx;` named intermediate keeps the shift; 26 → 20. The
  hoisted `$t1 = 1` invariant appears exactly as target. (Ordinary once-written
  once-read local holding a real consumed value; semantically truthful name.)
- H3 "target does NOT hoist &D_800A35D0 because the symbol materializes into a
  multi-set reused pseudo" — two-statement spelling into the reused `ptr`
  (`ptr = (u8*)&D_800A35D0; ptr = (t0*4) + ptr;`) defeats scan_loop admission;
  20 → 15. Single-statement forms hoist (fresh single-set temp).
- H4 "row-43 addu operand order follows int-vs-pointer domain of the address
  tree" — `(u8*)((t0*2) + (s32)base)` flips to target's (offset,base) order;
  15 → 14. Pointer-domain swaps are canonicalized away (byte-neutral).

KILLED (measured, forms banked in rejected/):
- K1 q-split second handle for the 0x30/0x34 stores → 28, insn collapse (cse
  forwards the handle, deletes the loop-preheader reload).
- K2 tail cached-pointer statement groups → 29 (cse merges the repeated
  D_800A36A0 reads; direct per-statement global derefs already match the tail).
- K3 common `row` pointer for p_6a/p_7e → 38 (restructures CSE of (t0*10));
  association-order spelling of the same expression → byte-neutral.

## Live frontier (for s2)

1. **reorg fill choice on the inner-loop-2 beqz** (residual class 4, ~2 diffs +
   phase). Mechanism: reorg.c picks our `ori` (fall-through steal, dest dead on
   taken path) where target steals `addiu a2+1` from the branch-target thread and
   retargets past it. Probe: read tmp/grind/func_800770B8/dumps/text1b.dbr for
   the beqz's candidate list/eligibility; then find the C shape that makes the
   ori ineligible or the thread-steal preferred (e.g. if-arm first statement
   writing a register live at the join; `for` vs `do-while` respelling; ONLY
   as a measured last resort the sanctioned duplicated-statement-into-arms
   family with its full prerequisites). This is the rule-era insert_label
   device residual — main's ledger has the mechanism background.
2. **0x30/0x34 store handle + a2=0 placement** (residual class 2, ~4 diffs).
   Target keeps the func_8006E49C result in $v0 for two stores while $s1 holds
   the copy. Probe: .lreg/.greg for the call-result pseudo's death; C levers
   that split the last two stores' addresses from p_old WITHOUT a cse-visible
   second handle (e.g. reorder the four post-call statements; store 0x30/0x34
   BEFORE D_800A36A0/+4; sub-expression shapes). K1 bounds the naive spelling.
3. **prologue schedule / ClearOTagR arg order + p_6a base coalesce** (classes
   1+3, ~8 diffs). Class 3 probe: .lreg death counts for the base vs offset
   pseudos at the row-62 addu ([[local-alloc-death-count-class-wall]] gate
   first — if the multi-death pseudo is punted, spelling levers are closed).
   Class 1 probe: .sched dump of block 0 — see whether a1-first arg evaluation
   (target) is reachable by arg-expression shape (e.g. hoisting D_800A374C into
   a named local) before touching anything else; prologue save order may be
   pure cascade from the body's register demand, so re-measure AFTER frontier
   items 1-2 land.

## [s1] Function returns 1 (not void); chassis tail rules were void-artifacts
- mechanism: live $v0=1 before epilogue; caller ignores result
- probe: s32 signature + return 1 + caller prototype fix, sandbox
- result: 26-floor clean chassis, tail byte-clean
- verdict: CONFIRMED

## [s1] srav+andi vs target sllv+and is the tree-level single-bit fold of (x & (1<<k))!=0
- mechanism: fold-const bit-test transform; separate named local hides the shift from the fold
- probe: s32 mask = 1 << idx; if (arg2 & mask)
- result: 26 -> 20; hoisted $t1=1 appears exactly as target
- verdict: CONFIRMED

## [s1] Target leaves lui/addiu D_800A35D0 in-loop because the symbol materializes into a multi-set reused pseudo
- mechanism: loop.c scan_loop movable admission requires n_times_set==1; expand_assignment targets ptr's pseudo directly only in the two-statement spelling
- probe: ptr = (u8*)&D_800A35D0; ptr = (t0*4) + ptr;
- result: 20 -> 15; rows 49-51 byte-clean; single-statement form still hoists
- verdict: CONFIRMED

## [s1] row-43 addu operand order follows int-vs-pointer domain of the address expression
- mechanism: pointer-domain PLUS is canonicalized (base,off); int-domain preserves written order
- probe: ptr = (u8*)((t0*2) + (s32)base)
- result: 15 -> 14
- verdict: CONFIRMED

## [s1] Second C handle q for the call result reproduces target's $v0-handle stores at 0x30/0x34
- mechanism: two live pseudos for one value
- probe: u8 *q = func_8006E49C(...); stores via q
- result: 28, build collapses 176->171 (cse forwards q, deletes preheader reload)
- verdict: KILLED

## [s1] Tail needs cached-pointer statement groups to share gp loads
- mechanism: one load per group
- probe: p = D_800A36A0; grouped stores
- result: 29; cse merges the repeated reads below target's count; direct derefs already match
- verdict: KILLED

## [s1] Common row pointer coalesces the p_6a base addu into the offset register
- mechanism: RA coalesce with offset pseudo
- probe: u8 *row = (u8*)((t0*10)+(s32)D_800A36A0)
- result: 38 isolated; association-order swap byte-neutral
- verdict: KILLED

## Session 2 (structural, 2026-09-01) - floor 14 -> 12 -> 10

CONFIRMED:
- H5 "the target's tail reads D_800A36A0 once for the {+0, +0x65} store pair" -
  a local `u8 *q = D_800A36A0;` scoped to just those two stores removes our
  redundant reload (lw + load-delay nop); 14 -> 12. s1's K2 stays dead: the rest
  of the tail must keep its per-statement global derefs.
- H6 "a2's initialiser is a statement INSIDE the outer loop body, not a preheader
  init plus a tail reset" - moving it to the first statement of the do-body sinks
  `addu $a2,$zero,$zero` from sched1 slot 30 to slot 37 (target's slot) and drops
  the duplicate; 12 -> 10; rows 30-34 and 37 byte-clean.
- H7 "the addiu-$s1 (`arg0 + 0x58`) prologue slot follows C statement order" -
  moving the statement after ClearOTagR moves the insn from slot 8 to slot 11
  (target: 12). CONFIRMED as a mechanism but NET WORSE (12 -> 13) because the
  freed slot goes to `lui %hi(D_800A374C)` rather than target's `sw $ra`.
  Form banked in rejected/s2-p_old-computed-after-clearotagr.c.
- H8 "class D is reorg's fall-through delay-slot steal winning where the target's
  build lost it" - PROVEN, not inferred. DBRDBG trace for jump_insn 314 shows
  likely=0 / own=1 / tif=0 / setsopp=0 / WINNER trial=327 (the `ori`), and the
  what-if BB2_ALLLIVE_LABEL=355 (force mark_target_live_regs conservative at the
  join) reproduces the target's `beq` + copied `addu $2,$6,1` delay slot and
  closes the 174->175 insn-count gap. Full reading in evidence.md.

KILLED (measured, forms banked in rejected/):
- K4 second C handle for the func_8006E49C result, spelled the OTHER way from K1
  (reuse `r` as an int handle, `p_old = (s32 *)r`, stores via `r`) -> 26, build
  collapses 174 -> 169 (cse forwards the handle, deletes the D_800A35D0 preheader
  reload). With K1 this closes the "name the call result twice" axis in both
  directions. rejected/s2-call-result-second-handle.c
- K5 int-domain address form for the p_6a/p_7e bases (the s1-H4 trick applied to
  rows 62-64) -> 34. The +0x6A/+0x7E constants fold into the int expression and
  destroy the shared (t0*10) CSE. Confirms rows 62-64 are an allocator decision,
  not an address-shape decision. rejected/s2-p6a-int-domain-address.c
- K6 (byte-neutral, not a regression) writing the 0x30/0x34 stores as
  `*(s32 *)(D_800A36A0 + 0x30) = 0;` - cse resolves the global load to p_old's
  pseudo, so it cannot reach the raw call-result pseudo. No form banked (it is
  one-line and neutral); do not re-measure.

## Live frontier (for s3)

1. **Class D via own_fallthrough (the ONE untried half of a proven mechanism).**
   Mechanism: reorg.c own_thread_p returns 0 for the fall-through thread if ANY
   CODE_LABEL sits between the beqz and the if-body's first active insn; with
   own_fallthrough==0 fill_eager_delay_slots skips the fall-through steal entirely
   and fills from the branch-target thread by COPY+redirect - the target's shape,
   worth the last insn of the count gap. Probe: C shapes that materialise a label
   at the then-arm head - `if (A || B) { ... }` short-circuit forms of the existing
   single test (e.g. splitting the bit test), and `goto`-into-the-arm shapes.
   Sanity-check each candidate by re-running s2/run_idump.sh with BB2_DBR_DEBUG=1
   and reading the `own=` field of `DBRDBG thr insn=<uid>` - that field tells you
   whether the shape worked BEFORE you look at the score. Do NOT spend probes on
   making $v0 live at the join (option (b)); it needs find_basic_block()==-1, which
   no C respelling controls.
2. **Class A(ii): ClearOTagR a1-first argument evaluation** (~3 rows, the largest
   remaining cluster). Target emits `addiu $a1,$zero,0x1008` BEFORE the
   `lui/lw %hi/%lo(D_800A374C)` pair; ours emits the 2-insn a0 chain first because
   sched1 ranks it higher (longer path to the jal). Probe: read s2/idumps/text1b.sched
   for block 0's priorities and ready-list order, then try arg-expression shapes that
   shorten the a0 chain or lengthen the a1 one (a named local for the OT pointer read
   before the call, a differently-typed OT arg). H7 says statement order does move
   block-0 insns, so this is a live spelling axis, not a wall - but re-measure the
   prologue-save rows together with it (H7's regression came from the save order).
3. **Class B/C are allocator decisions, not spellings.** B: our call-result temp is
   `Register 80 used 2 times` (dies at the copy) - both second-handle spellings are
   dead, so the next honest step is tools/ra_solver / inverse_compose.py classify on
   the p_old-vs-temp seat, not more C. C: same, .lreg "dies in N places" for the
   row-62 base/offset pseudos ([[local-alloc-death-count-class-wall]] gate first).
   Neither should consume a whole session before frontier 1 and 2 are spent.

## [s2] Target's tail groups ONLY the {+0, +0x65} stores behind one D_800A36A0 read
- mechanism: a store through a pointer may alias the global, so GCC reloads it between statements; one local for that pair removes the lw + load-delay nop
- probe: { u8 *q = D_800A36A0; *(s32 *)q = arg1; *(s8 *)(q + 0x65) = 0; }
- result: 14 -> 12 (176 -> 174 insns); tail becomes byte-clean modulo the global count gap
- verdict: CONFIRMED

## [s2] a2's initialiser is a statement inside the outer loop body, not a preheader init + tail reset
- mechanism: sched1 block-0 placement; the preheader form ranks a2=0 at slot 30, the in-body form leaves it at slot 37 after the 0x30/0x34 store cluster
- probe: single `a2 = 0;` as the first statement of the do-body; delete the preheader init and the tail reset
- result: 12 -> 10; rows 30-34 and 37 byte-clean
- verdict: CONFIRMED

## [s2] The prologue addiu $s1,$s0,0x58 slot follows C statement order
- mechanism: sched1 LUID tie-break in block 0
- probe: move `p_old = (s32 *)(arg0 + 0x58);` to after ClearOTagR
- result: insn moves slot 8 -> 11 (target 12) but score 12 -> 13; the freed slot goes to lui %hi(D_800A374C), not target's sw $ra
- verdict: CONFIRMED (mechanism) / form REJECTED (net worse)

## [s2] Class D is reorg's fall-through delay-slot steal, which the target build lost
- mechanism: reorg.c fill_eager_delay_slots, likely=0 => fall-through tried first; own_fallthrough=1 and setsopp=0 let our `ori` win the slot. Blocking it requires own_fallthrough==0 (a CODE_LABEL at the then-arm head) or mark_target_live_regs answering all-live at the join.
- probe: instrumented cc1 BB2_DBR_DEBUG=1 trace of jump_insn 314 + what-if BB2_ALLLIVE_LABEL=355
- result: what-if reproduces the target's beq + copied `addu $2,$6,1` delay slot and closes the 174->175 count gap
- verdict: CONFIRMED

## [s2] A second C-level handle for the func_8006E49C result reaches the $v0-based 0x30/0x34 stores
- mechanism: two live pseudos for one value (spelled the opposite way from K1: `r` as the int handle, p_old as the derived pointer)
- probe: r = (s32)func_8006E49C(...); p_old = (s32 *)r; stores via r
- result: 26, build collapses 174 -> 169 (cse forwards the handle, deletes the D_800A35D0 preheader reload) - same failure as K1 on a different chassis
- verdict: KILLED

## [s2] The s1 int-domain address trick (H4) generalises to the p_6a/p_7e bases at rows 62-64
- mechanism: int-domain PLUS preserves written operand order, so the addu dest should follow the offset pseudo
- probe: (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A)
- result: 34 - the byte constants fold into the int expression and destroy the shared (t0*10) CSE
- verdict: KILLED

## [s2] The target's tail reads D_800A36A0 once for the {+0 (arg1), +0x65} store pair only; our per-statement global derefs reload it between them.
- mechanism: A store through a pointer may alias the global, so GCC re-emits the gp-relative load; the reload costs lw + a load-delay nop that the target does not have. Target's tail grouping is exactly {+0,+0x65} | {+0x67} | {lbu +0x67, sb +0x66}.
- probe: { u8 *q = D_800A36A0; *(s32 *)q = arg1; *(s8 *)(q + 0x65) = 0; } scoped to just that pair; sandbox --disable all.
- result: 14 -> 12 (176 -> 174 insns); the whole tail from row 148 to the epilogue becomes byte-clean modulo the one-insn global count gap. s1's K2 (grouping the WHOLE tail) stays dead - only this pair groups.
- verdict: CONFIRMED

## [s2] a2's initialiser is a statement inside the outer loop body, not a preheader init plus a duplicate reset at the outer-loop tail.
- mechanism: sched1 block-0 placement: the preheader form ranks `addu $a2,$zero,$zero` into slot 30 (ahead of the hoisted loop invariants and the 0x30/0x34 store cluster); the in-body form leaves it at slot 37, exactly where the target has it.
- probe: Single `a2 = 0;` as the first statement of the outer do-body; delete both the preheader init and the tail reset. sandbox --disable all.
- result: 12 -> 10; rows 30-34 and row 37 all byte-clean; the pre-loop cluster residual (s1 class 2a) is fully closed.
- verdict: CONFIRMED

## [s2] The prologue `addiu $s1,$s0,0x58` slot follows C statement order (LUID tie-break in sched1's block 0).
- mechanism: sched.c rank_for_schedule tie-break on equal priority; the insn has no in-block dependents so its slot tracks its RTL position.
- probe: Move `p_old = (s32 *)(arg0 + 0x58);` from before ClearOTagR to after it; sandbox + positional diff.
- result: The insn does move (slot 8 -> 11; target has it at 12), so the mechanism is real - but the score goes 12 -> 13 because the vacated slot 7/8 pair goes to `lui %hi(D_800A374C)` rather than target's `sw $ra`/`sw $s1`. Form banked in rejected/s2-p_old-computed-after-clearotagr.c.
- verdict: CONFIRMED

## [s2] The last residual insn-count gap (ours 174, target 175) is reorg's fall-through delay-slot steal winning in our build where the target build lost it.
- mechanism: reorg.c fill_eager_delay_slots. mostly_true_jump returns 0 for our beqz (EQ test, equal rare_destination on both sides), so the FALL-THROUGH thread is tried first; own_fallthrough=1 and the trial's dest $v0 is not in opposite_needed (the join's first insn sets $v0 without reading it), so our `ori` legally takes the slot. The target instead filled from the branch-target thread by copying `addiu $v0,$a2,1` and redirecting past it.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_DBR_DEBUG=1 -> trace for jump_insn 314; then the what-if knob BB2_ALLLIVE_LABEL=355 (force mark_target_live_regs to its conservative everything-live answer at the join thread, i.e. simulate find_basic_block()==-1) and diff the emitted .s.
- result: Trace: `DBRDBG thr insn=314 thread=327 opp=355 own=1 likely=0 tif=0 oppregs=200c0fe0_00000000 oppmem=1` / `trial=327 ... setsopp=0 trap=0` / `WINNER insn=314 trial=327`. The what-if reproduces the target EXACTLY in that region (beq + delay `addu $2,$6,1` + then `ori`) and takes the build 174 -> 175 insns. Only two things can block the steal: own_fallthrough==0 (a CODE_LABEL at the then-arm head - C-reachable) or an all-live mark_target_live_regs answer (not C-reachable).
- verdict: CONFIRMED

## [s2] A second C-level handle for the func_8006E49C result reproduces the target's $v0-based 0x30/0x34 stores.
- mechanism: Two live pseudos for one value; spelled the OPPOSITE way from s1's K1 (reuse `r` as the int handle, derive p_old = (s32 *)r, do the last two stores through r).
- probe: r = (s32)func_8006E49C(r, D_800A35D8); p_old = (s32 *)r; stores at +0x30/+0x34 through r; sandbox --disable all.
- result: 26, and the build collapses 174 -> 169 insns - cse forwards the handle and deletes the D_800A35D0 loop-preheader reload. Identical failure mode to K1 on a different chassis, so the 'name the call result twice' axis is now dead in BOTH directions. Separately measured byte-NEUTRAL: writing those stores as `*(s32 *)(D_800A36A0 + 0x30) = 0;` (cse resolves the global load to p_old's pseudo, never to the call-result temp).
- verdict: KILLED

## [s2] The s1 H4 int-domain address trick (which fixed the row-43 addu operand order) generalises to the p_6a/p_7e base addresses at rows 62-64.
- mechanism: int-domain PLUS preserves written operand order where pointer-domain PLUS is canonicalized, so the addu dest should follow the offset pseudo as it does in the target.
- probe: s16 *p_6a = (s16 *)((t0 * 10) + (s32)D_800A36A0 + 0x6A); same for p_7e; sandbox --disable all.
- result: 34 (from a 10 chassis). The +0x6A/+0x7E constants fold into the int expression, destroying the shared (t0*10) CSE and restructuring rows 55-70. With s1's K3 this closes the address-shape axis on rows 62-64 from both directions - it is a local-alloc dest-coalesce decision.
- verdict: KILLED

## Session 3 (structural, 2026-09-01) - floor 10 -> 9

CONFIRMED:
- H9 "class D closes if inner loop 2 is spelled as a `for` loop" - the C-reachable
  blocker of reorg's fall-through delay-slot steal. 10 -> 9, 174 -> 175 insns; our
  insn count now equals the target's exactly. Rows 103-105 and 113 byte-clean.
- H10 "BB2_NO_FT_STEAL=1 reproduces the target region" - re-verified the s2
  mechanism from the other knob (skip the fall-through fill outright rather than
  force mark_target_live_regs all-live); identical output, so any blocker suffices.
- H11 "class A is a sched2 (post-reload) decision, not sched1" - PASS-ATTRIBUTION
  CORRECTION. The register saves do not exist in sched1's RTL; text1b.sched2 shows
  the whole contested cluster at priority 1, decided by rank_for_schedule
  tie-breaks. s2's sched1 attribution was wrong and should not be inherited.
- H12 "pointer-domain PLUS is canonicalized, so C source order cannot set the RTL
  operand order of the row-62 addu" - C1 measured NEUTRAL on both chassis.

KILLED (measured, forms banked in rejected/):
- K7 short-circuit `||` respelling of the bit test to force a CODE_LABEL at the
  then-arm head (own_fallthrough==0). Measured 32 / 176 insns: the second test is
  really emitted, so the label costs more than the delay slot buys. It also has no
  semantic justification - the code has exactly one condition.
  rejected/s3-shortcircuit-or-duplicate-test.c
- K8 `if ((arg2 & mask) == mask)` to make the guard branch an NE test (which makes
  mostly_true_jump return 1 so fill_eager_delay_slots tries the branch-target
  thread FIRST). Measured 29: the register-to-register compare keeps `mask` live
  across the branch and restructures the loop. Route (b) through `prediction > 0`
  is closed at the C level. rejected/s3-mask-equality-test-ne-branch.c
- K9 goto-into-the-arm (`if (c) goto set; goto skip; set: ...; skip:`) to plant a
  CODE_LABEL at the then-arm head. Measured byte-NEUTRAL (10 on the s2 chassis):
  jump.c folds the conditional-goto/goto pair back into one conditional branch and
  the label does not survive to reorg, so own_fallthrough stays 1. Superseded by H9
  anyway.
- K10 `s32 ot = D_800A374C;` named local for the ClearOTagR OT argument - the s2
  frontier's own suggested class-A probe. 11 on the s2 chassis, 10 on the s3
  chassis: worse, not better. Form kept at tmp/grind/func_800770B8/s3/v/A6.c.
- K11 the `D_800A35D8 = arg0;` store moved anywhere BEFORE ClearOTagR (three
  placements) - 31/32/32 with the build collapsing 175 -> 172 because cse forwards
  $s0 across the call. rejected/s3-sp-inits-after-snd-stopall.c banks the nearest
  neighbour (A12, 13); rejected/s3-p_old-after-cleartotagr-sp-first.c banks A2.
- K12 named int/pointer row bases and constant-folded-onto-base address shapes for
  p_6a/p_7e - 33/34 and 13 (176 insns) respectively.
  rejected/s3-rowbase-int-named-intermediate.c and
  rejected/s3-p6a-constant-folded-onto-base.c
- K13 five restatements of the 0x30/0x34 stores (B1-B4 plus the s2 K6 global form):
  B2/B3/B4 exactly byte-NEUTRAL, B1 = 12. No C spelling reaches the raw
  call-result pseudo.

## Live frontier (for s4)

1. **Class C (3 diffs) - the row-62 dest coalesce - is now a SOLVER question, not a
   spelling question.** Eight address shapes are measured dead (s1 K3, s2 K5, s3
   C1/C2/C3/C4/C6/C7). The single decision is whether local-alloc ties the addu's
   output to the lw pseudo (ours, $v0) or the sll pseudo (target, $v1); both die at
   that insn. Probe: `tools/ra_solver/inverse_compose.py classify` on that seat,
   plus the `.lreg` "dies in N places" counts for the two input pseudos
   ([[local-alloc-death-count-class-wall]] gate first). Do this BEFORE any more C.
   The dumps for the floor-9 form are already in tmp/grind/func_800770B8/s3/idumps/.
2. **Class A (4 diffs) - sched2 ready-list order among eight priority-1 insns.**
   Twelve statement orderings are measured dead and the attribution is now correct
   (sched2, post-reload; text1b.sched2 block 0 "from 546 to 81"). The open question
   is whether the target's emission order is reachable AT ALL, since the save
   insns' LUIDs are fixed by save_restore_insns and only {15, 26, 28, 18, 21} are
   C-controllable. Probe: `tools/sched_solver` on sched2 block 0 with the target's
   emission order as the goal - it is order- and clock-exact, so it returns
   REACHABLE (with the required LUID permutation, which then becomes a C question)
   or FORECLOSED. That verdict is worth more than another dozen permutations.
3. **Class B (2 diffs) - closed at the C level; five spellings measured.** The
   target keeps the raw func_8006E49C result live past the copy into p_old and uses
   it for the 0x30/0x34 stores. Every C form that names the value twice triggers a
   cse forward that deletes the D_800A35D0 preheader reload (-5 insns). Next step is
   the ra_solver on that seat, not more C variants; if it classifies the seat as
   reachable only via a second live pseudo, that is a ruling-request, not a grind.

## [s3] Class D closes if inner loop 2 is written as a `for` loop
- mechanism: reorg.c fill_eager_delay_slots - the `for` spelling blocks the fall-through delay-slot steal of the `ori`, so reorg falls through to the branch-target-thread call and fills by copying `addiu $v0,$a2,1` and redirecting past it (the target's shape). Verified independently with the instrumented cc1's BB2_NO_FT_STEAL=1 knob (reorg.c:3817), which produces the identical region from the s2 body.
- probe: `for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) { ... }` replacing the preheader `a2 = 0;` plus do-while with trailing increment; sandbox --disable all.
- result: 10 -> 9, and 174 -> 175 insns - our insn count now EQUALS the target's for the first time. Rows 103-105 and 113 byte-clean. `for` respellings of the outer loop and of inner loop 1 (E1/E2/E3) are byte-neutral, so they are not adopted.
- verdict: CONFIRMED

## [s3] Class A is a sched2 (post-reload) decision, not a sched1 block-0 decision
- mechanism: the `sw $s0/$s1/$s2/$s3/$ra` prologue stores are emitted by save_restore_insns AFTER reload, so they are absent from sched1's RTL entirely. text1b.sched2 block 0 ("from 546 to 81") carries them as insns 546..556, and every insn in the contested cluster has priority 1 - the order is settled by rank_for_schedule's dependence-class and LUID tie-breaks plus the "greater potential hazard" reordering.
- probe: read tmp/grind/func_800770B8/s3/idumps/text1b.sched (pseudos, no saves present) and text1b.sched2 (real regs, saves present, all priority 1).
- result: attribution corrected. The target's emission order for the cluster is ours exactly REVERSED: target 548, 554, 28, 26, 15 vs ours 554, 15, 26, 28, 548.
- verdict: CONFIRMED

## [s3] Statement ordering around the ClearOTagR call can move class A
- mechanism: LUID order feeds sched2's final rank_for_schedule tie-break
- probe: twelve orderings (A2-A12) of {p_old assign, sp[0]/sp[1] inits, ClearOTagR call, D_800A35D8 store, snd_StopAll call}, measured on both the s2 and s3 chassis
- result: the best is NEUTRAL (A5, sp inits before p_old). A2/A6/A10/A11 = +1, A3/A4 = +2, A12 = +4, A7/A8/A9 = +21..+22 with a 3-insn collapse (cse forwards $s0 across the call once the global store moves ahead of it).
- verdict: KILLED

## [s3] A short-circuit `||` respelling plants a surviving CODE_LABEL at the then-arm head (own_fallthrough==0)
- mechanism: do_jump's TRUTH_ORIF_EXPR creates a drop_through_label at the then-arm head when if_true_label is null, which makes own_thread_p return 0 for the fall-through thread
- probe: `if (((arg2 & mask) != 0) || ((arg2 & mask) == mask))`; sandbox --disable all
- result: 32 with 176 insns - the second test is genuinely emitted and costs more than the delay slot buys. It also has no semantic justification: the code has exactly one condition. Superseded by H9, which achieves own_fallthrough==0 from ordinary loop spelling.
- verdict: KILLED

## [s3] `if ((arg2 & mask) == mask)` makes the guard an NE branch so mostly_true_jump returns 1 and reorg tries the branch-target thread first
- mechanism: reorg.c mostly_true_jump's final switch returns 1 for NE and 0 for EQ; prediction > 0 sends fill_eager_delay_slots to the target thread before the fall-through thread
- probe: the equality-against-mask spelling (semantically identical for a single-bit mask); sandbox --disable all
- result: 29 with 175 insns - the register-to-register compare keeps `mask` live across the branch and restructures the loop. Route (b) (prediction > 0) is closed at the C level; route (a) (own_fallthrough == 0) is the one that works.
- verdict: KILLED

## [s3] A goto-into-the-arm shape plants a CODE_LABEL that survives to reorg
- mechanism: an explicit label at the then-arm head, referenced by a conditional goto, would make own_thread_p return 0 for the fall-through thread
- probe: `if (c) goto set; goto skip; set: <arm>; skip: <increment>`; sandbox --disable all
- result: 10 on the s2 chassis - byte-NEUTRAL. jump.c folds the conditional-goto/goto pair back into a single conditional branch and the label does not survive, so own_fallthrough stays 1.
- verdict: KILLED

## [s3] The row-62 base addu's operand order is settable from C source order
- mechanism: s1's H4 showed int-domain PLUS preserves written operand order, so the pointer-domain form should at least be testable
- probe: `(t0 * 10) + D_800A36A0 + 0x6A` (C1), plus five further address shapes (C2/C3/C4/C6/C7)
- result: C1 byte-NEUTRAL on both chassis (pointer-domain PLUS is canonicalized, confirming H4's negative direction); C2/C3/C4 = 33/34; C6/C7 = 13 with 176 insns. Row 62 is a local-alloc dest-coalesce decision between two dying input pseudos, not an address shape.
- verdict: KILLED

## [s3] Class B's $v0-based 0x30/0x34 stores are reachable by restating the store expressions
- mechanism: a different C spelling of the same address might resolve to the raw call-result pseudo rather than to p_old's
- probe: B2 (`(u8 *)p_old` casts), B3 (all four stores through the global), B4 (`p_old[12] = 0;` array form), B1 (stores emitted first); sandbox --disable all
- result: B2/B3/B4 all byte-NEUTRAL (9); B1 = 12 (it reorders the output too). With s1's K1 and s2's K4 that is five measured spellings; class B has no C-level lever left.
- verdict: KILLED

## [s3] Class D (the reorg delay-slot fill divergence, and the entire 174-vs-175 insn-count gap) closes if inner loop 2 is spelled as a `for` loop instead of a preheader `a2 = 0;` plus a do-while with a trailing increment.
- mechanism: reorg.c fill_eager_delay_slots. s2 proved our build wins the fall-through steal of the `ori` (own_fallthrough=1, likely=0, setsopp=0) where the target build lost it and filled from the branch-target thread by copying `addiu $v0,$a2,1` and redirecting past it. The `for` spelling blocks the fall-through fill, so control reaches the branch-target-thread call. Independently re-verified with the instrumented cc1's BB2_NO_FT_STEAL=1 knob (reorg.c:3817), which skips the fall-through fill outright and produces the identical region from the s2 body.
- probe: for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) { ... } replacing the preheader init + do-while; sandbox func_800770B8 --disable all
- result: 10 -> 9; build_insns 174 -> 175, exactly the target's count for the first time in the grind. Rows 103-105 and 113 byte-clean. `for` respellings of the outer loop and of inner loop 1 measured byte-neutral (9), so they are not adopted.
- verdict: CONFIRMED

## [s3] Class A (prologue rows 7-12) is a sched1 block-0 ordering decision, as recorded by s2.
- mechanism: s2 attributed the cluster to sched1. In fact the `sw $s0/$s1/$s2/$s3/$ra` prologue stores are emitted by save_restore_insns AFTER reload and do not exist in sched1's RTL at all; text1b.sched shows sched1's block 0 operating on pseudos with no saves, while text1b.sched2 block 0 ("from 546 to 81") carries them as insns 546..556 with every insn in the contested cluster at priority 1.
- probe: Read tmp/grind/func_800770B8/s3/idumps/text1b.sched and text1b.sched2 for func_800770B8 block 0 (priorities, ready lists, insn UID->pattern mapping).
- result: Attribution corrected to SCHED2 (post-reload). All eight contested insns are priority 1, so the order is settled by rank_for_schedule tie-breaks (dependence class vs last_scheduled_insn, then INSN_LUID) and the 'greater potential hazard' reordering. The target's emission order is ours exactly reversed: target 548(sw ra), 554(sw s1), 28(li a1), 26(lui/lw a0), 15(addiu s1) vs ours 554, 15, 26, 28, 548.
- verdict: KILLED

## [s3] Statement ordering around the ClearOTagR call can move the class-A cluster toward the target's order.
- mechanism: C statement order sets RTL LUIDs, which feed sched2's final rank_for_schedule tie-break; s2's H7 proved the addiu-$s1 slot does move with statement order.
- probe: Twelve orderings (A2-A12) of {p_old assign, sp[0]/sp[1] inits, ClearOTagR call, D_800A35D8 store, snd_StopAll call}, measured on both the s2 chassis (base 10) and the s3 chassis (base 9).
- result: Best is byte-NEUTRAL (A5: sp inits before p_old). A2/A6/A10/A11 = +1, A3/A4 = +2, A12 = +4, A7/A8/A9 = +21..+22 with the build collapsing 175 -> 172 (moving the D_800A35D8 store ahead of the call lets cse forward $s0). Axis spent.
- verdict: KILLED

## [s3] A short-circuit `||` respelling of the bit test plants a surviving CODE_LABEL at the then-arm head, giving own_fallthrough==0 (the s2 frontier's suggested class-D route).
- mechanism: do_jump's TRUTH_ORIF_EXPR creates a drop_through_label at the then-arm head when if_true_label is null; own_thread_p returns 0 on any CODE_LABEL between the branch and the thread's first active insn when label==NULL_RTX.
- probe: if (((arg2 & mask) != 0) || ((arg2 & mask) == mask)) { ... }; sandbox --disable all
- result: 32 with 176 insns - the second test is genuinely emitted and costs more than the delay slot buys. It also has no semantic justification (the code has exactly one condition). Superseded by the `for` spelling, which achieves the same block from ordinary C.
- verdict: KILLED

## [s3] Making the guard branch an NE test (`if ((arg2 & mask) == mask)`) makes mostly_true_jump return 1, so fill_eager_delay_slots tries the branch-target thread FIRST - the second C-reachable route to the target's fill.
- mechanism: reorg.c mostly_true_jump's final switch returns 1 for NE and 0 for EQ; fill_eager_delay_slots only tries the fall-through thread first when prediction <= 0.
- probe: if ((arg2 & mask) == mask) { ... } (semantically identical for a single-bit mask); sandbox --disable all
- result: 29 with 175 insns - the register-to-register compare keeps `mask` live across the branch and restructures the loop. Route (b) through prediction > 0 is closed at the C level.
- verdict: KILLED

## [s3] A goto-into-the-arm shape plants a CODE_LABEL at the then-arm head that survives to reorg.
- mechanism: An explicit label referenced by a conditional goto would make own_thread_p return 0 for the fall-through thread.
- probe: if (c) goto set; goto skip; set: <arm>; skip: <increment>; sandbox --disable all
- result: Byte-NEUTRAL (10 on the s2 chassis). jump.c folds the conditional-goto/goto pair back into one conditional branch and the label does not survive, so own_fallthrough stays 1.
- verdict: KILLED

## [s3] The row-62 base addu's operand order (class C) is settable from C source order, as s1's H4 achieved for the row-43 addu.
- mechanism: s1's H4 established that an int-domain PLUS preserves written operand order while a pointer-domain PLUS is canonicalized; if the p_6a/p_7e base add could be written (offset, base) the dest would coalesce onto the sll pseudo as in the target.
- probe: C1 `(t0 * 10) + D_800A36A0 + 0x6A` (pointer domain), C2 named int base, C3 named u8* row, C4 mixed, C6 constant folded onto the base first, C7 scaled-array form; sandbox --disable all on both chassis.
- result: C1 byte-NEUTRAL on both chassis (positively confirming that pointer-domain PLUS is canonicalized); C2/C3/C4 = 33/34; C6/C7 = 13 with 176 insns. With s1's K3 and s2's K5 that is eight measured address shapes - row 62 is a local-alloc dest-coalesce decision between two dying input pseudos, not an address shape.
- verdict: KILLED

## [s3] Class B's $v0-based 0x30/0x34 stores are reachable by restating the store expressions (without a cse-visible second handle).
- mechanism: A different C spelling of the same address might resolve to the raw func_8006E49C result pseudo rather than to p_old's.
- probe: B1 (stores emitted before the global/+4 stores), B2 (`(u8 *)p_old` casts), B3 (all four stores through the global), B4 (`p_old[12] = 0;` array form); sandbox --disable all.
- result: B2/B3/B4 all exactly byte-NEUTRAL (9); B1 = 12 (it reorders the output too). With s1's K1 and s2's K4 that is five measured spellings; class B has no C-level lever left.
- verdict: KILLED

## [s4] The class-A prologue residual is reachable — sched2 interleaves the reload-emitted save stores with the first body insns, and a mid-block LOOP note stops it.
- mechanism: sched2's block 0 contains both the `save_restore_insns` stores and the first body insns; with nothing between them the ready-list interleaves `addiu $s1` / `a0` / `a1` ahead of `sw $ra`. A NOTE_INSN_LOOP_BEG/END pair emitted between the two groups (from a `do { } while (0);` at that exact point) survives to sched2 anchored mid-block via reemit_notes and prevents the interleave; NOTE_INSN_BLOCK notes do not, because they all migrate to the top of the function.
- probe: permuter campaign A (16166 iters) rediscovered the construct from three independent seeds; sandbox measured V1 = 7 (from 9); cc1 `-da` dumps for both forms compared at `.rtl/.jump/.loop/.combine/.lreg/.greg/.jump2/.sched/.sched2/.dbr` (tmp/grind/func_800770B8/s4/dumps/).
- result: CONFIRMED as a mechanism; the construct itself is a CHEAT (scheduling barrier / empty-bodied wrap outside the reorg.c-scoped do-while-zero carve-out) and is banked in rejected/, not candidate.c.

## [s4] Ordinary C block scoping can supply the same mid-block note as the do-while(0).
- mechanism: an inner `{ }` scope holding `p_old`/`r` emits NOTE_INSN_BLOCK_BEG/END, which if anchored between the saves and the body insns would fence sched2 identically while being completely ordinary C.
- probe: V7 (one inner scope) and V8 (two nested inner scopes), sandbox-measured; plus a read of the `.sched2` note positions in both dump sets.
- result: KILLED. Both measure exactly 9 (byte-neutral). The dumps show every NOTE_INSN_BLOCK_BEG/END has already migrated to the very top of the function, ahead of the first prologue insn, in BOTH forms — block notes are not anchored mid-block, so they cannot fence.

## [s4] s3's "twelve statement orderings around ClearOTagR are all dead" is chassis-conditional, not absolute.
- mechanism: the ordering only changes which insns are AVAILABLE to sched2's ready list at the prologue boundary; if sched2 is free to interleave, any ordering is re-sorted and the axis looks flat.
- probe: `p_old = (s32*)(arg0+0x58);` moved after the ClearOTagR call, measured on both chassis: unfenced (V9) and fenced (V10).
- result: CONFIRMED coupling. V9 = 10 (one WORSE than the 9 baseline — this is the shape of s3's negative result). V10 = 5 (four better). The ordering axis is alive; it was masked. Any future chassis carrying a legitimate fence must re-run the full A2-A12 ordering set.

## [s4] A non-empty do-while(0) wrap can substitute for the empty one.
- mechanism: if the lever is purely the note pair's position, a wrap with real statements inside placed at the same boundary should fence identically while having a defensible body.
- probe: V2 (wrap the first three statements, unmoved chassis) = 11; V3 (wrap only the p_old assign) = 9; V11 (wrap the entire remaining body, moved chassis) = 39 at 176 insns; V12 (wrap `sp[0]/sp[1]/ClearOTagR` on the p_old-moved chassis) = 8 at 174 insns.
- result: KILLED as written. Only V12 beats the honest floor, and it is one insn SHORT of the target's 175 — it is not on the path to 0. The note's position relative to the FIRST body insn is what matters, and every real-bodied wrap moves at least one insn inside the notes.

## [s4] The class-A prologue residual (sched2 interleaving the reload-emitted register-save stores with the first body insns) is reachable, and a mid-block NOTE_INSN_LOOP_BEG/END pair is what stops the interleave.
- mechanism: sched2 block 0 holds both the save_restore_insns stores (emitted after reload) and the first body insns. With nothing between them the ready list emits addiu $s1 / a0 / a1 ahead of sw $ra. A loop-note pair emitted at that boundary survives to sched2 anchored mid-block (sched.c reattaches loop notes via REG_NOTES in reemit_notes) and fences the two groups apart. Basic-block structure is unchanged (14 blocks in both dumps), so this is an ordering effect inside block 0, not a region split.
- probe: Permuter campaign A (base score 372, 16166 iters, 7 finds) rediscovered `do { } while (0);` in the prologue from three independent seeds, scoring best at 155 when placed immediately before the p_old assign. Sandbox-measured V1 = 7 (from 9, 175/175 insns). Full cc1 -da dump sets for both forms compared stage by stage at tmp/grind/func_800770B8/s4/dumps/{cand,v1}/.
- result: CONFIRMED as a mechanism. sched2 block 0 with V1 emits all five saves contiguously, then NOTE_INSN_PROLOGUE_END, then NOTE_INSN_LOOP_BEG 583 / NOTE_INSN_LOOP_END 582, then insns 27/38/40; without it insn 560 (sw $ra) lands after insns 15/26/28. The construct itself is a CHEAT (scheduling barrier; empty body; outside the reorg.c-scoped do-while-zero carve-out) and is banked in rejected/, not candidate.c.
- verdict: CONFIRMED

## [s4] Ordinary C inner block scopes can supply the same mid-block note and fence sched2 without any dead construct.
- mechanism: An inner `{ }` scope holding p_old and r (both dead after the post-call block) emits NOTE_INSN_BLOCK_BEG/END, which would be completely ordinary C if it anchored at the prologue boundary.
- probe: V7 (one inner scope) and V8 (two nested inner scopes) built from candidate.c and sandbox-measured; note positions read directly out of both .sched2 dumps.
- result: KILLED. Both measure exactly 9 (byte-neutral). Every NOTE_INSN_BLOCK_BEG/END in BOTH dumps has already migrated to the very top of the function, ahead of the first prologue insn (notes 12/52/64/83 precede insn 558) - block notes are never anchored mid-block, so they cannot fence sched2. Only loop notes are.
- verdict: KILLED

## [s4] s3's result that all twelve statement orderings around the ClearOTagR call are dead is an absolute property of the function.
- mechanism: Statement order sets RTL LUIDs and therefore what is available to sched2's ready list at the prologue boundary; if sched2 is free to interleave that boundary, every ordering is re-sorted and the axis measures flat.
- probe: Moved `p_old = (s32 *)(arg0 + 0x58);` to after the ClearOTagR call and measured it on BOTH chassis: unfenced (V9) and fenced (V10). Campaign B, seeded on the fenced chassis (base score 155, 17760 iters), independently found the same reorder as its best result (score 35).
- result: KILLED (the absolute reading). V9 = 10, one WORSE than the 9 baseline - exactly the shape of s3's negative result. V10 = 5, four BETTER. The ordering axis is alive and was masked by the unfenced chassis; s3's conclusion is now scoped to the unfenced chassis only.
- verdict: KILLED

## [s4] A non-empty do-while(0) wrap can substitute for the empty one, giving the fence a defensible body.
- mechanism: If the lever is purely the note pair's position, a wrap containing real statements placed at the same boundary should fence identically.
- probe: V2 (wrap the first three statements, unmoved chassis), V3 (wrap only the p_old assign), V11 (wrap the entire remaining body, moved chassis), V12 (wrap sp[0]/sp[1]/ClearOTagR on the p_old-moved chassis) - all sandbox-measured.
- result: KILLED as written. V2 = 11, V3 = 9, V11 = 39 at 176 insns. V12 = 8 is the only one that beats the honest floor and it is 174 insns, one SHORT of the target's 175, so it is not on the path to 0. Every real-bodied wrap necessarily moves at least one insn inside the notes, which is the thing that breaks the fence.
- verdict: KILLED

## [s5] There is an ORDINARY-C construct that anchors a NOTE_INSN_LOOP_BEG/END pair between the reload-emitted save stores and the first body insn (the s4 frontier's #1 hypothesis).
- mechanism: only LOOP notes survive to sched2 anchored mid-block (sched.c reemit_notes reattaches them via REG_NOTES); BLOCK notes all migrate to the top of the function and were measured byte-neutral in s4 (V7/V8). So the honest fence had to be a REAL loop construct placed as the function's first statement, not a scope.
- probe: four spellings built and sandbox-measured — P1 `for (t0=0;t0<2;t0=(s16)(t0+1)) sp[t0]=0;` as the first statement then p_old then ClearOTagR; P2 the same loop with p_old moved after ClearOTagR (the V10 pairing); P3 p_old first then the loop; P4 a `while` spelling. Plus a full cc1 `-da` dump set for P1 and for the baseline, with the sched2 note anchor read directly.
- result: KILLED, and the kill is two-sided. The MECHANISM half is CONFIRMED — P1's `.sched2` shows `(note 599 611 16 "" NOTE_INSN_PROLOGUE_END)` immediately followed by `(note 16 599 25 "" NOTE_INSN_LOOP_BEG)`, i.e. a real first-statement loop anchors its note pair at exactly the V1 fence position, so ordinary C CAN reach the anchor. The COST half is fatal: all four forms measure 41-42 at **186 insns** (+11 over the baseline 175). A real loop over `sp[]` forces the two `sh $zero` stores onto a computed stack address and adds loop control. Our insn count has equalled the target's 175 since s3, so any construct that adds instructions is off the path to 0 by construction; the only zero-insn loop is an empty one, which is the s4 cheat.
- verdict: KILLED. rejected/s5-real-loop-note-anchor-insn-cost.c

## [s5] The target's contiguous prologue save emission is produced by a note fence of some kind, so finding an honest fence is the route to class A.
- mechanism: s4 proved a mid-block loop-note pair reproduces the target's emission order (V1 = 7, V10 = 5), which made "find an honest fence" look like the whole game.
- probe: read the TARGET's own prologue out of asm/funcs/func_800770B8.s (rows 1-14) and compare the frame directive against ours.
- result: KILLED. The target's prologue is straight-line — five saves interleaved with three arg copies, then `addiu $a1,$zero,0x1008`, `lui/lw $a0`, `addiu $s1,$s0,0x58`, two `sh $zero`, then the jal. No branch, no label, no loop anywhere near it. Whatever produced the target's order, it was NOT a loop-note fence, because the original source had no loop there. The s4 `do { } while (0);` is a coincidental route to the same emission order. Frames are byte-identical on both sides (0x40; s0/s1/s2/s3/ra at 0x28/0x2C/0x30/0x34/0x38), so class A has no frame-shape component either — it is purely the emission order of five insns.
- verdict: KILLED — and this also RETIRES the s4 frontier item that proposed a ruling-request on extending the `do-while(0)` carve-out from reorg.c to sched2. That request is withdrawn before filing: there is nothing to extend the carve-out TO, since the construct does not reconstruct what the original source did. Do not re-open it.

## [s5] The residual at floor 9 is dominated by sched2 emission order (the working assumption of s2/s3/s4).
- mechanism: s3 corrected the pass attribution from sched1 to sched2 and every session since has attacked ordering. Nobody had ever run the object-level RA classifier on this function.
- probe: `tools/ra_solver/goal_from_tgt.py classify text1b func_800770B8` (object-level; `inverse_compose.py classify` refuses on a zero-rule function and names goal_from_tgt as the replacement). Corroborated with `tools/sched_solver/perturb.py --pass 2 --goal-from-target text1b --target-object build/src/text1b.o --ours-object tmp/sandbox/func_800770B8/text1b.o --atoms luid,luid_move --depth 2`, plus `--self-check`.
- result: KILLED as stated. FIRST DIVERGENCE = **RA**: `$v0->$v1 x4`, `$s1->$v0 x2`, `$v1->$v0 x1` — 5 renamed pairs, 2 pairs skipped as "skeleton differs — reloc/immediate, not a rename" (those two are the class-A order rows). Seven of the residual half-rows are register NAMING. The sched solver prints no differing pass-2 block for this function while its self-check reports 14250/14250 exact priority recomputation, so the model is live and the silence is a verdict. CAVEAT recorded honestly: goalmap could not align the two non-rename rows and holds them at OUR positions, so the sched silence is evidence that the ordering residual is not independently expressible as a sched2 goal, not proof the emission order matches.
- verdict: KILLED (re-attribution: the residual is majority-RA, and three sessions of ordering work were aimed at the smaller half)

## [s5] Class B (the 0x30/0x34 stores through $v0 rather than $s1) is reachable by some C spelling not yet found.
- mechanism: the target keeps the raw `func_8006E49C` result live for two stores after copying it into `$s1`; ours attaches the stores to `p_old`. Five C spellings had been measured dead across s1/s2/s3 with no model-level verdict on the seat.
- probe: `goal_from_tgt.py goal text1b func_800770B8 --model tmp/ra_solver_work/func_800770B8.model.json` to attribute the substitution onto a pseudo, then `tools/ra_solver/inverse.py global tmp/ra_solver_work/func_800770B8.model.json --goal '{"75": 2}' --depth 2 --top 8`.
- result: **FORECLOSED**, with a named mechanism. The `$s1->$v0 x2` substitution attributes UNIQUELY to pseudo 75 (`p_old`). The inverse solver returns a NEGATIVE result over 161 single perturbations in 6 classes at depth 2, and explains why the key atom cannot even be emitted: *"pseudo 75 crosses 4 call(s) and $v0 is call-used, so prune_preferences (global.c:897) strips it from this allocno's preferences before find_reg ever sees it."* No C spelling that only moves refs / live span / birth order / conflicts / preferences / calls-crossed can give `p_old` $v0. Semantically the target's store base is a pseudo whose life ENDS before the next call — i.e. a second handle on the raw call result — which is the family that is now six measured spellings dead (s1 K1 = 28, s2 K4, s3 B1 = 12, B2/B3/B4 byte-neutral), every one lost to cse forwarding the handle and deleting the D_800A35D0 loop-preheader reload.
- verdict: CONFIRMED as FORECLOSED for the rename reading. The solver explicitly names what remains outside its model for this seat — the local-alloc suggested-register pass (`qty_phys_copy_sugg` / `qty_phys_sugg`, reported but not scored today), `qty_size` for DImode, and reload's spill-retry — and says the next move there is INSTRUMENTATION, not another spelling search. That boundary is the honest scope of the foreclosure.

## Live frontier (for s6) — reset by the s5 synthesis

1. **Class C is the ONLY unspent typed-verdict axis, and its inputs are now all in place.**
   The remaining `$v0 <-> $v1` substitutions (rows 62-64, the p_6a/p_7e base addu's
   dest coalesce) are a local-alloc question, and local-alloc has never been run on
   this function. Eight address shapes are already measured dead (s1 K3, s2 K5,
   s3 C1/C2/C3/C4/C6/C7), so do NOT spell more addresses first.
   - mechanism: `tools/ra_solver/inverse.py local` models local-alloc's
     `block_alloc` and returns REACHABLE-with-lever-vectors or FORECLOSED for a
     dest-coalesce seat between two dying input pseudos, exactly the shape here.
   - next probe: `python3 tools/ra_solver/local_extract.py text1b` to build
     `tmp/ra_solver_work/text1b.local.json`, find the block holding the row-62
     `addu`, then `inverse.py local tmp/ra_solver_work/text1b.local.json --func
     func_800770B8 --block <N> --swap <A>,<B> --depth 2`. Attribute the seat first
     with `goal_from_tgt.py goal ... --model` (it reported the $v0/$v1 pairs as
     AMBIGUOUS across 38 and 13 pseudos, so the block-level view is what
     disambiguates). Spell any returned vector in C and re-measure — a vector is a
     hypothesis, not a result.

2. **The suggested-register pass is the named gap in the model for class B, and it is an INSTRUMENTATION job, not a spelling job.**
   - mechanism: `inverse.py`'s own negative report names `qty_phys_copy_sugg` /
     `qty_phys_sugg` (local-alloc's suggested registers, currently reported but not
     scored), `qty_size` for DImode, and reload's spill-retry as the mechanisms
     outside the model. The instrumented cc1 is `tools/gcc-2.7.2/cc1` and already
     carries BB2_* debug hooks (BB2_NO_FT_STEAL was used in s3), so adding a
     read-only observation of the suggested-register state for pseudo 75's quantity
     is in-family with what the toolchain already supports.
   - next probe: dump the local-alloc quantity state around the
     `addu $s1,$v0,$zero` copy and check whether target's assignment is explicable
     as a copy-suggestion that our RTL does not offer. If it is, the C question
     becomes "what makes the copy's SOURCE the suggested register", which is a
     different and much narrower search than the six dead second-handle spellings.
     If it is not, class B is fully foreclosed and the residual is a two-class
     problem.

3. **Do NOT re-open: the fence, the ordering set, and the do-while(0) ruling.**
   - mechanism: s5 killed the honest-fence hypothesis on insn cost AND showed the
     target has no loop in its prologue, so the fence is not the original mechanism.
     The s4 frontier's item 2 (re-run the A2-A12 ordering set on a fenced chassis)
     is DIAGNOSTIC-ONLY by its own terms — every fenced chassis carries the cheat —
     and with the fence now known not to be the target's mechanism the diagnostic
     no longer points anywhere. The s4 frontier's item 3 (a ruling-request on
     extending the do-while(0) carve-out from reorg.c to sched2) is withdrawn for
     the same reason.
   - next probe: none. Any session that finds itself respelling a prologue fence
     should stop and go to frontier item 1 or 2 instead.

## [s5] There is an ORDINARY-C construct that anchors a NOTE_INSN_LOOP_BEG/END pair between the reload-emitted save stores and the first body insn (the s4 frontier's #1 hypothesis, worth floor 5 if it existed).
- mechanism: Only LOOP notes survive to sched2 anchored mid-block (sched.c reemit_notes reattaches them via REG_NOTES); BLOCK notes migrate to the top of the function and measured byte-neutral in s4 (V7/V8). So the honest fence had to be a REAL loop placed as the function's first statement.
- probe: Four spellings sandbox-measured against candidate.c: P1 `for (t0=0;t0<2;t0=(s16)(t0+1)) sp[t0]=0;` first then p_old then ClearOTagR; P2 same loop with p_old moved after ClearOTagR (the V10 pairing); P3 p_old first then the loop; P4 a `while` spelling. Plus a full cc1 -da dump set for P1 and for the baseline, with the sched2 note anchor read directly out of in.i.sched2.
- result: Two-sided kill. MECHANISM CONFIRMED: P1's .sched2 shows (note 599 611 16 "" NOTE_INSN_PROLOGUE_END) immediately followed by (note 16 599 25 "" NOTE_INSN_LOOP_BEG) - a real first-statement loop anchors its note pair at exactly the V1 fence position, so ordinary C CAN reach the anchor. COST FATAL: all four forms measure 41-42 at 186 insns (+11 over the 175 baseline); a real loop over sp[] forces the two sh $zero stores onto a computed stack address and adds loop control. Our insn count has equalled the target's 175 since s3, so any construct that adds instructions is off the path to 0 by construction; the only zero-insn loop is an empty one, which is the s4 cheat.
- verdict: KILLED

## [s5] The target's contiguous prologue save emission is produced by a note fence of some kind, so finding an honest fence is the route to class A.
- mechanism: s4 proved a mid-block loop-note pair reproduces the target's emission order (V1 = 7, V10 = 5), which made 'find an honest fence' look like the whole game.
- probe: Read the TARGET's own prologue out of asm/funcs/func_800770B8.s rows 1-14 and compared its frame directive against ours.
- result: The target's prologue is straight-line: five saves interleaved with three arg copies, then addiu $a1,$zero,0x1008, lui/lw $a0, addiu $s1,$s0,0x58, two sh $zero, then the ClearOTagR jal. No branch, no label, no loop anywhere near it - so whatever produced the target's order, it was NOT a loop-note fence, because the original source had no loop there. The s4 do-while(0) is a coincidental route to the same emission order. Frames are byte-identical on both sides (0x40; s0/s1/s2/s3/ra at 0x28/0x2C/0x30/0x34/0x38), so class A has no frame-shape component either. This also RETIRES the s4 frontier item proposing a ruling-request on extending the do-while(0) carve-out from reorg.c to sched2 - withdrawn before filing, because there is nothing to extend the carve-out TO.
- verdict: KILLED

## [s5] The residual at floor 9 is dominated by sched2 emission order (the working assumption of s2, s3 and s4).
- mechanism: s3 corrected the pass attribution from sched1 to sched2 and every session since attacked ordering; nobody had ever run the object-level RA classifier on this function.
- probe: tools/ra_solver/goal_from_tgt.py classify text1b func_800770B8 (object-level path; inverse_compose.py classify refuses on a zero-rule function and names goal_from_tgt as the replacement). Corroborated with tools/sched_solver/perturb.py --pass 2 --goal-from-target text1b --target-object build/src/text1b.o --ours-object tmp/sandbox/func_800770B8/text1b.o --atoms luid,luid_move --depth 2, plus --self-check.
- result: FIRST DIVERGENCE = RA: $v0->$v1 x4, $s1->$v0 x2, $v1->$v0 x1 - 5 renamed pairs, 2 pairs skipped as 'skeleton differs - reloc/immediate, not a rename' (those two are the class-A order rows). Seven of the residual half-rows are register NAMING, not scheduling. The sched solver prints no differing pass-2 block for this function while its self-check reports 'priority recomputation: 14250/14250 exact (100.00%)', so the model is live and the silence is a real verdict. CAVEAT banked honestly: goalmap could not align the two non-rename rows and holds them at OUR positions, so the sched silence is evidence the ordering residual is not independently expressible as a sched2 goal, not proof the emission order matches.
- verdict: KILLED

## [s5] Class B (the 0x30/0x34 stores based on $v0 rather than $s1) is reachable by some C spelling not yet found.
- mechanism: The target keeps the raw func_8006E49C result live for two stores after copying it into $s1; ours attaches the stores to p_old. Five C spellings were already measured dead across s1/s2/s3 with no model-level verdict on the seat.
- probe: goal_from_tgt.py goal text1b func_800770B8 --model tmp/ra_solver_work/func_800770B8.model.json to attribute the substitution onto a pseudo, then tools/ra_solver/inverse.py global tmp/ra_solver_work/func_800770B8.model.json --goal '{"75": 2}' --depth 2 --top 8.
- result: FORECLOSED with a named mechanism. The $s1->$v0 x2 substitution attributes UNIQUELY to pseudo 75 (p_old), giving goal {"75": 2}. The inverse solver returns a NEGATIVE result over 161 single perturbations in 6 classes at depth 2 and explains why the key atom cannot even be emitted: 'pseudo 75 crosses 4 call(s) and $v0 is call-used, so prune_preferences (global.c:897) strips it from this allocno's preferences before find_reg ever sees it.' No C spelling that only moves refs / live span / birth order / conflicts / preferences / calls-crossed can give p_old $v0. Semantically the target's store base is a pseudo whose life ENDS before the next call - a second handle on the raw call result - which is the family now six measured spellings dead (s1 K1 = 28, s2 K4, s3 B1 = 12, B2/B3/B4 byte-neutral), every one lost to cse forwarding the handle and deleting the D_800A35D0 loop-preheader reload. The solver explicitly names what remains outside its model for this seat - the local-alloc suggested-register pass (qty_phys_copy_sugg / qty_phys_sugg, reported but not scored today), qty_size for DImode, and reload's spill-retry - and says the next move there is INSTRUMENTATION, not another spelling search; that is the honest scope of the foreclosure.
- verdict: CONFIRMED

## Session 6 (synthesis, 2026-09-01) — floor 9 (unchanged); class C RE-ATTRIBUTED

Chassis re-measured at session start with candidate.c applied: score 9,
build_insns 175, target_insns 175. Identical to the s3/s4/s5 chassis, so every
banked spelling conclusion below is still chassis-current.

### The headline: class C is NOT a register-allocation question
s5 handed s6 a frontier whose item 1 said "class C is a local-alloc
dest-coalesce decision; run inverse.py local". That framing is WRONG, and this
session disproves it from the RTL rather than from a model. The correct
attribution is **RTL plus operand order at insn 173** (an expand/fold-level
property of the C expression), and the register naming at rows 62-64 is a
downstream CONSEQUENCE, not the cause.

Proof, from tmp/grind/func_800770B8/dumps/text1b.lreg (regenerated this session
off the floor-9 body) and the positional diff:

    rows 60/61 are BYTE-IDENTICAL on both sides:
        60  lw   $v0, %gp_rel(D_800A36A0)($gp)     (insn 171, reg 109)
        61  sll  $v1, $v1, 1                       (insn 169, reg 108)
    row 62 differs only in the plus's operand order:
        ours    addu $2,$2,$3     = (plus 109 108)  -> dest ties to reg 109 ($v0)
        target  addu $v1,$v1,$v0  = (plus 108 109)  -> dest ties to reg 108 ($v1)

Both input pseudos already carry the TARGET's hard registers (109 -> $v0,
108 -> $v1). There is therefore nothing for local-alloc to fix: the allocator is
already doing what the target's allocator did. The mips addsi3_internal template
is `addu %0,%1,%2` and in BOTH builds the dest ties to operand 1 (the first
source), so the entire 3-row class collapses to a single question — which of
reg 108 / reg 109 is operand 0 of the plus in insn 173.

inverse.py local was therefore NOT run: running an allocator solver on a seat
where the allocator already agrees with the target would have produced a
meaningless verdict. local_extract.py text1b --func func_800770B8 --suggest WAS
run and its outputs are banked (tmp/ra_solver_work/text1b.local.json,
text1b.sugg.json) for the class-B instrumentation item, which is untouched.

### The operand order IS C-controllable — and every flip costs the double read
Thirteen further address spellings were built and sandbox-measured (bringing the
project total for this seat to nineteen). The finding is a clean dichotomy:

  * POINTER-domain `D_800A36A0 + (t0*10)` -> canonicalized to (base, offset).
    This is the floor-9 form; it also produces the target's SECOND lw of
    D_800A36A0 at row 60 (the loop-top `u8 *base = D_800A36A0;` local supplies
    the first read at row 40, and the global is genuinely re-loaded here).
  * INT-domain `(t0*10) + (s32)D_800A36A0` -> preserves written order, i.e. it
    DOES flip insn 173 to the target's (108, 109). Confirmed in D2's output:
    rows 55-57 compute t0*5, sll 1, then `addu $2,$2,$3` with the shift as
    operand 0 — the target's shape.
  * But every int-domain spelling ALSO collapses the double read: the value of
    D_800A36A0 already sits in `base`'s register, and once the expression is
    int-typed cse forwards it, deleting the row-60 lw and re-floating the whole
    p_6a/p_7e computation ahead of the 0x42/0x40/0x68 stores. Score 33-36.

So the residual C question is now sharply stated for the first time:
**flip insn 173's plus to (shift, load) while keeping the row-60 lw a distinct
in-place memory read.** Every spelling measured so far buys one and loses the
other.

### Measured this session (all on the floor-9 chassis, score / build_insns)
  D1 two-statement multi-set int rowb (H3 anti-hoist + H4 int order)    33 / 175
  D2 symmetric int-domain, unnamed                                      33 / 175
  D3 multi-set pointer, int add written first                           12 / 174
  D4 int rowb built off the loop-top base local                         45 / 174
  E1 shift as the pointer operand, (s32)D_800A36A0                      33 / 175
  E2 constant folded onto the shift side                                20 / 176
  E3 multi-set u8 *row, shift as pointer base                           33 / 175
  E4 (u8*)(t0*10) + (s32)(D_800A36A0 + 0x6A)                            13 / 176
  F2 shared row, global carries 0x6A, p_7e = row + 0x14                 35 / 175
  F4 same value spelled through p_6a                                    35 / 175
  F5 named pointer handle g = D_800A36A0 + 0x6A, int add                15 / 175
  G0 loop-top base local removed (control)                              34 / 176
  G1 G0 + int-domain p_6a                                               36 / 176

E4 is the most informative loser: it is the ONLY form that flips the order AND
keeps the row-60 re-read, and it costs exactly one insn (176) because the
+0x6A / +0x7E constants fold onto the shift side BEFORE the add, so the
single addu + two addiu shape becomes two addus. G0/G1 prove the loop-top
`base` local is load-bearing: removing it costs an insn outright.

### A residual row is a SCORING ARTIFACT, not a byte difference
Row 50 (`addiu $2,$2,0` vs target `addiu $v0,$v0,%lo(D_800A35D0)`) is NOT a real
divergence. `mipsel-linux-gnu-objdump -dr` on tmp/sandbox/func_800770B8/text1b.o
shows R_MIPS_HI16 / R_MIPS_LO16 against D_800A35D0 at 0x8bc8 / 0x8bcc with a zero
addend; the engine's scorer masks the HI16 immediate (row 49 is not flagged) but
not the LO16 addend, so an unlinked-relocation row is counted as a difference.
The linker resolves it to exactly the target's bytes. See the auto-memory note
[[sandbox-lo16-text-addend-false-distance]]. **The honest residual is therefore
8 real rows, not 9**, distributed entirely across classes A (4), B (2) and C (3).
Any future exhaustion record for this function must state the floor as "9 as
scored, 8 real, one unrelocated-LO16 artifact".

## [s6] Class C (rows 62-64) is a local-alloc dest-coalesce decision between two dying input pseudos (the s5 frontier's item 1).
- mechanism: s5 read the object-level classifier's $v0->$v1 x4 output as register renaming and concluded the seat belonged to local-alloc's block_alloc, which ties a dest quantity to a dying source quantity.
- probe: regenerated the full cc1 -da dump set off the floor-9 body (tmp/grind/func_800770B8/run_dump.sh) and read insns 168/171/169/173/175/190 out of text1b.lreg; cross-read the positional diff rows 60-64. Also ran local_extract.py text1b --func func_800770B8 --suggest to have the local-alloc ground truth in hand.
- result: KILLED as an attribution. insn 173 is (set (reg 110) (plus (reg 109) (reg 108))), reg 109 = the lw D_800A36A0 (insn 171), reg 108 = the sll ...,1 (insn 169). Rows 60 and 61 are byte-identical to the target, i.e. BOTH input pseudos already receive the target's hard registers ($v0 and $v1 respectively). The allocator is already in agreement with the target; in both builds the dest ties to operand 1 of addu %0,%1,%2. The only difference is the ORDER of the plus's two register operands in the RTL, which is settled at expand/fold time from the C expression's type domain, long before local-alloc runs. inverse.py local was deliberately not run on this seat: an allocator solver cannot return anything meaningful about a seat where the allocator already matches.
- verdict: KILLED (re-attributed: RTL plus operand order, not RA)

## [s6] The row-62 plus operand order can be flipped from C without collateral damage.
- mechanism: s1's H4 established that a pointer-domain PLUS is canonicalized to (base, offset) while an int-domain PLUS preserves written source order. Applying H4's own lever to the p_6a/p_7e base add should therefore produce the target's (shift, load) order.
- probe: thirteen spellings built and sandbox-measured on the floor-9 chassis — D1-D4 (int row bases, multi-set and single-set, off the global and off the base local), E1-E4 (the shift cast to u8 * so it becomes the canonical pointer operand, with and without the constants folded onto it), F2/F4/F5 (a named pointer handle carrying the 0x6A so the global read stays pointer-typed), G0/G1 (the loop-top base local removed, as a control).
- result: HALF-CONFIRMED, HALF-KILLED. The flip itself is REACHABLE: D2's output shows addu $2,$2,$3 with the shift as operand 0, exactly the target's shape. But every int-domain spelling simultaneously deletes the target's second lw of D_800A36A0 at row 60 — the value is already live in the loop-top base local's register, and once the expression is int-typed cse forwards it and re-floats the whole p_6a/p_7e computation ahead of the 0x42/0x40/0x68 stores. Scores 33-36 at 175 insns (no insn-count change: it is pure reordering + renaming cascade). E4 is the sole form that keeps the re-read AND flips the order, and it costs exactly one insn (176) because the +0x6A/+0x7E constants fold onto the shift side before the add. G0/G1 show the base local itself is load-bearing (removing it costs an insn: 176). Nineteen address spellings are now measured across s1/s2/s3/s6; the naive axis is exhausted, but the question is now sharply stated rather than merely dead.
- verdict: KILLED (as a spelling search) / CONFIRMED (as a mechanism: the order IS C-controllable)

## [s6] Row 50 of the residual is a real byte divergence.
- mechanism: the positional diff shows addiu $2,$2,0 against the target's addiu $v0,$v0,%lo(D_800A35D0), which looks like a missing low-half materialisation.
- probe: mipsel-linux-gnu-objdump -dr -Mreg-names=numeric tmp/sandbox/func_800770B8/text1b.o, grepping for D_800A35D0.
- result: KILLED. The object carries R_MIPS_HI16 and R_MIPS_LO16 relocations against D_800A35D0 at 0x8bc8 / 0x8bcc with a zero in-place addend. The engine's scorer masks the HI16 immediate (row 49 is correctly not flagged) but does not mask the LO16 addend, so the unrelocated row is counted as a difference that the linker resolves to the target's exact bytes. Matches the auto-memory note [[sandbox-lo16-text-addend-false-distance]]. The honest residual is 8 real rows, not 9.
- verdict: KILLED (the row is a scoring artifact)

## Live frontier (for s7) — reset by the s6 synthesis

1. **Class C, restated precisely: flip insn 173's plus to (shift, load) while keeping the row-60 lw a distinct in-place memory read.**
   - mechanism: pointer-domain PLUS canonicalizes to (base, offset) and keeps the
     re-read; int-domain PLUS preserves written order and lets cse forward the
     loop-top base local's register, deleting the re-read. The two properties
     have never been obtained together. E4 comes closest (re-read kept, order
     flipped, +1 insn from the constants folding onto the shift side).
   - next probe: attack the CONSTANT FOLD, not the operand order. E4's only defect
     is that 0x6A/0x7E land on the shift operand before the add. Spellings that
     keep the shift as the canonical pointer operand while forcing the constants to
     stay on the RESULT: index the row through a struct/array element type so the
     +0x6A is a member offset rather than a folded addend, or give the two
     inner-loop pointers a common (u8 *)(t0*10) + (s32)D_800A36A0 row whose two
     users each add their own constant. Read text1b.rtl (pre-optimization) for E4
     and for the baseline side by side to see exactly where the fold happens — the
     baseline dumps are already in tmp/grind/func_800770B8/dumps/.

2. **Class B's suggested-register instrumentation (unspent, and now the ONLY item with a solver-named mechanism).**
   - mechanism: s5's inverse.py global foreclosure on pseudo 75 explicitly named
     qty_phys_copy_sugg / qty_phys_sugg as reported-but-not-scored. The
     instrumented cc1 already emits SUGGDBG-QTY / SUGGDBG-FFR lines and this
     session banked a full extraction for text1b:
     tmp/ra_solver_work/text1b.sugg.json (2.7 MB) and text1b.local.json.
     Nobody has read them yet.
   - next probe: read the SUGGDBG rows for func_800770B8's block containing the
     addu $s1,$v0,$zero copy out of text1b.sugg.json and check whether the
     target's assignment is explicable as a copy-suggestion our RTL does not offer.
     The data is already on disk; this is a read, not a build.

3. **Do NOT re-open: the prologue fence, the A2-A12 ordering set, the do-while(0) ruling, OR inverse.py local on the row-62 seat.**
   - mechanism: the first three are closed by s5 (the target has no loop in its
     prologue and a real loop costs +11 insns). The fourth is closed by s6: both
     input pseudos at insn 173 already carry the target's hard registers, so the
     allocator has nothing to fix and a local-alloc solver run on that seat cannot
     return a meaningful verdict.
   - next probe: none. Any session that finds itself running an RA solver on rows
     62-64, or respelling a prologue fence, should stop and go to item 1 or 2.

## [s6] Class C (rows 62-64) is a local-alloc dest-coalesce decision between two dying input pseudos, as the s5 frontier's item 1 asserted, and inverse.py local should be run on that seat.
- mechanism: s5 read the object-level classifier's '$v0->$v1 x4' output as register renaming and concluded the seat belonged to local-alloc's block_alloc, which ties a dest quantity to a dying source quantity.
- probe: Regenerated the full cc1 -da dump set off the floor-9 body (tmp/grind/func_800770B8/run_dump.sh) and read insns 168/171/169/173/175/190 out of text1b.lreg; cross-read the positional diff rows 60-64. Also ran tools/ra_solver/local_extract.py text1b --func func_800770B8 --suggest to have local-alloc ground truth in hand.
- result: insn 173 is (set (reg 110) (plus (reg 109) (reg 108))), with reg 109 = the lw of D_800A36A0 (insn 171) and reg 108 = the sll ...,1 (insn 169). Rows 60 and 61 are BYTE-IDENTICAL to the target, i.e. both input pseudos already receive the target's hard registers ($v0 and $v1). In both builds the dest ties to operand 1 of the mips addsi3_internal template 'addu %0,%1,%2'. The allocator is therefore already in agreement with the target and has nothing to fix; the sole difference is which of reg 108 / reg 109 is operand 0 of the plus, which is settled at expand/fold time from the C expression's type domain, long before local-alloc runs. inverse.py local was deliberately NOT run: an allocator solver cannot return a meaningful verdict on a seat where the allocator already matches.
- verdict: KILLED

## [s6] The row-62 plus operand order can be flipped from C to the target's (shift, load) without collateral damage, by applying s1's H4 int-domain lever to the p_6a/p_7e base add.
- mechanism: s1's H4 established that a pointer-domain PLUS is canonicalized to (base, offset) while an int-domain PLUS preserves written source order.
- probe: Thirteen spellings built and sandbox-measured on the floor-9 chassis: D1-D4 (int row bases, multi-set and single-set, off the global and off the loop-top base local), E1-E4 (the shift cast to u8 * so it becomes the canonical pointer operand, with and without the constants folded onto it), F2/F4/F5 (a named pointer handle carrying the 0x6A so the global read stays pointer-typed), G0/G1 (the loop-top base local removed, as a control).
- result: HALF-CONFIRMED / HALF-KILLED. The flip is REACHABLE: D2's output shows 'addu $2,$2,$3' with the shift as operand 0, exactly the target's shape. But every int-domain spelling simultaneously deletes the target's SECOND lw of D_800A36A0 at row 60 - the value already sits in the loop-top base local's register, and once the expression is int-typed cse forwards it and re-floats the whole p_6a/p_7e computation ahead of the 0x42/0x40/0x68 stores. Scores: D1 33/175, D2 33/175, D3 12/174, D4 45/174, E1 33/175, E2 20/176, E3 33/175, E4 13/176, F2 35/175, F4 35/175, F5 15/175, G0 34/176, G1 36/176. E4 is the sole form that keeps the re-read AND flips the order, costing exactly one insn because 0x6A/0x7E fold onto the shift side before the add. G0/G1 prove the loop-top base local is load-bearing (removing it costs an insn). Nineteen address spellings are now measured for this seat across s1/s2/s3/s6.
- verdict: KILLED

## [s6] Residual row 50 (ours 'addiu $2,$2,0' vs target 'addiu $v0,$v0,%lo(D_800A35D0)') is a real byte divergence that C must close.
- mechanism: The positional diff shows a missing low-half materialisation of D_800A35D0, which reads as a codegen difference.
- probe: mipsel-linux-gnu-objdump -dr -Mreg-names=numeric tmp/sandbox/func_800770B8/text1b.o, grepping for D_800A35D0 (output banked at tmp/grind/func_800770B8/s6/dis.txt).
- result: The object carries R_MIPS_HI16 and R_MIPS_LO16 relocations against D_800A35D0 at 0x8bc8 / 0x8bcc with a zero in-place addend. The engine's scorer masks the HI16 immediate (row 49 is correctly NOT flagged) but does not mask the LO16 addend, so an unlinked-relocation row is counted as a difference that the linker resolves to exactly the target's bytes. This matches the auto-memory note sandbox-lo16-text-addend-false-distance. The honest residual is 8 real rows, not 9.
- verdict: KILLED
