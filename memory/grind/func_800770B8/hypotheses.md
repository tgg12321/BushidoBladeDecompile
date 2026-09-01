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

## [s7] Class B's residual is explicable by local-alloc's suggested-register pass (the s6 frontier's item 2).
- mechanism: s5's inverse.py global foreclosure of pseudo 75 (p_old) rested on prune_preferences (global.c:897) and explicitly listed qty_phys_copy_sugg / qty_phys_sugg as reported-but-not-scored. s6 banked the instrumented-cc1 extraction (tmp/ra_solver_work/text1b.sugg.json, 2.7 MB) but nobody had read it.
- probe: sliced text1b.sugg.json for func_800770B8 and printed every local-alloc quantity in the function (blk / qty / reg1 / birth / death / refs / calls / ncopysugg / copysugg / nsugg / sugg). 34 quantities across 14 blocks.
- result: KILLED. Pseudo 75 does NOT appear as a local-alloc quantity ANYWHERE in the function - local-alloc never sees it, exactly as s5's global-alloc attribution implies. Moreover the ENTIRE function carries exactly one suggestion of any kind: blk0 qty0 (reg1=72, the arg0 pseudo) with ncopysugg=1, copysugg=[4] ($a0). qty_phys_copy_sugg/qty_phys_sugg therefore cannot explain the $s1-vs-$v0 residual, and the last unspent RA-layer mechanism for class B is spent. Class B is fully foreclosed at the REGISTER-ALLOCATION layer.
- verdict: KILLED

## [s7] Class B is not an allocation question at all: it is cse pseudo IDENTITY, and the target's 2+2 register split is reachable but costs exactly one insn.
- mechanism: GCC 2.7.2 cse.c make_regs_eqv() ("Prefer fixed hard registers to anything. Prefer pseudo regs to other hard regs") makes a PSEUDO the qty_first_reg of the quantity a call result joins, and canon_reg() refuses to substitute a hard reg ("Never replace a hard reg") - so a non-fixed hard reg such as $v0 can NEVER be canonicalized into a use. The target's `sw $zero,0x30($v0)` / `sh $zero,0x34($v0)` therefore cannot be hard-reg forwarding of the $s1 pseudo; they must be a SECOND pseudo that the allocator seated in $v0. make_regs_eqv promotes a later-joining pseudo to canonical only when `uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start`, i.e. only when that pseudo is referenced somewhere outside the current cse extended basic block.
- probe: twelve spellings built and sandbox-measured on the floor-9 chassis (H1-H8, H10-H12, H15-H17), plus a cc1 -da re-dump of the winning shape and a read of tools/gcc-2.7.2/cse.c canon_reg/make_regs_eqv. H5/H6/H7 vary only WHICH base (p_old vs the D_800A36A0 re-read) each of the four stores is spelled through. H1/H3/H4/H8 give the raw call result its own C name. H10/H12 hoist a `u8 *pp` to function scope, assign it AFTER the global + 0x4 stores, use it for 0x30/0x34, and REUSE it in the tail block so its regno_last_uid lies past the cse block end. H11 is the same but block-local (control).
- result: HALF-CONFIRMED / HALF-KILLED, with the mechanism proven at RTL level. (a) Store-base SPELLING is completely byte-neutral: H5, H6, H7 all measure 9 / 175 - cse collapses all four bases onto one canonical pseudo regardless of how the C names them. (b) Naming the raw call result FIRST (H1/H3/H4/H8) makes that pseudo canonical, sends ALL FOUR stores through it, deletes the `move $s1,$v0` copy and drops a whole callee-saved register: 170 insns, score 23 - strictly worse and structurally wrong. (c) H11 (block-local second name) measures 9 / 175, confirming the canonicality test is exactly the uid-vs-block-boundary one. (d) H10/H12 DO produce the target's split: text1b.cse for H10 reads insn 68 `(set (mem (symbol_ref D_800A36A0)) (reg 75))`, insn 71 `(set (mem (plus (reg 75) 4)) (reg 81))`, insn 74 `(set (reg 76) (reg 75))`, insn 77 `(set (mem (plus (reg 76) 48)) 0)`, insn 80 `(set (mem (plus (reg 76) 52)) 0)` - two stores on pseudo 75 and two on pseudo 76, structurally identical to the target's $s1/$v0 division. The sole defect is insn 74: the copy that makes pseudo 76 join the quantity is a real instruction (176 insns, score 12). (e) The copy is not removable by construction: it vanishes only if 75 and 76 receive the same hard register, which destroys the split; and 76 cannot instead be born free from `(set 76 (reg:SI 2 v0))` because C provides exactly one assignment of a call result, and whichever pseudo takes it becomes canonical for the whole block (case b). (f) Composing the split with s6's D3 address form (H15) is 176 / 15, and with E4 (H17) 177 / 16 - the +1 does not cancel against the class-C forms.
- verdict: KILLED as a spelling search / CONFIRMED as a mechanism: the split is real, cse-controlled, and costs exactly one insn.

## [s7] s6's D3 address form is a 174-insn form.
- mechanism: s6 recorded D3 (multi-set int row base, written-order pointer chain) at score 12 / 174 insns, one insn UNDER the target, which made it a candidate donor of an insn to pay for any +1 construct elsewhere.
- probe: re-generated D3 against the committed candidate.c on today's chassis and measured (H16).
- result: KILLED. D3 measures 12 / **175** insns here, not 174. s6's 174 was measured against s6's own working base, not the committed floor-9 candidate; the insn-count saving does not exist on the ledger chassis. No form measured to date is under 175 insns while keeping the floor-9 structure, so there is no insn "credit" available to pay for the class-B split copy.
- verdict: KILLED

## Live frontier (for s8) - reset by the s7 solver session

1. **Class B: pay for the split copy, or find a second pseudo whose definition is free.**
   - mechanism: the split needs TWO pseudos over the call result; whichever pseudo
     is defined first becomes cse's canonical for the whole extended basic block
     (make_regs_eqv), so the second one must be introduced by a later copy, and
     that copy is a real instruction. The target is 175 insns and our floor-9 form
     is already 175, so the +1 must be repaid somewhere else in the body.
   - next probe: hunt for a byte-neutral 174-insn variant of the floor-9 body
     (any region), then compose it with H10. s6's D3 was the only 174 ever
     recorded and s7 disproved it. Alternatively look for a shape in which the
     second pseudo is born from a value that is NOT in the call result's cse
     quantity yet is provably equal - e.g. a member/element access whose address
     GCC recomputes rather than forwards. Both are ordinary-C questions.

2. **Class C (rows 62-64) - unchanged from s6, still the best-understood axis.**
   - mechanism: flip insn 173's plus to (shift, load) while keeping the row-60 lw
     of D_800A36A0 a distinct in-place memory read. E4 gets both but folds
     0x6A/0x7E onto the shift operand, costing one insn.
   - next probe: attack the CONSTANT FOLD. Index the row through a struct/array
     element type so +0x6A is a member offset rather than a folded addend. Read
     text1b.rtl for E4 and for the baseline side by side (dumps already banked).

3. **Do NOT re-open: any RA-layer attack on class B, the prologue fence, the
   A2-A12 ordering set, the do-while(0) ruling-request, or an RA solver run on
   the row-62 seat.**
   - mechanism: s5 foreclosed class B at global-alloc (prune_preferences), s7
     foreclosed the last RA-layer mechanism (local-alloc suggested registers: pseudo
     75 is not a local quantity at all and the function carries exactly one
     suggestion anywhere). Class B is now a PRE-RA question. The other three were
     closed by s5/s6.
   - next probe: none.

## [s7] Class B's residual is explicable by local-alloc's suggested-register pass (qty_phys_copy_sugg / qty_phys_sugg), the mechanism s5's inverse.py global foreclosure named as reported-but-not-scored.
- mechanism: s5 foreclosed pseudo 75 (p_old) at global.c prune_preferences (line 897) because p_old crosses 4 calls and $v0 is call-used; the solver's own negative report listed local-alloc's copy/plain suggestion arrays as outside its model. s6 banked the instrumented-cc1 SUGGDBG extraction (tmp/ra_solver_work/text1b.sugg.json) but never read it.
- probe: Sliced text1b.sugg.json for func_800770B8 and printed every local-alloc quantity in the function (blk / qty / reg1 / birth / death / refs / calls / ncopysugg / copysugg / nsugg / sugg): 34 quantities across 14 blocks.
- result: Pseudo 75 forms NO local-alloc quantity anywhere in the function - local-alloc never sees it, exactly as the global-alloc attribution implies. The entire function carries exactly one suggestion of any kind: blk0 qty0 (reg1=72, birth 2, death 20, refs 3, calls 1) with ncopysugg=1, copysugg=[4] ($a0). The suggested-register pass therefore cannot explain the $s1-vs-$v0 residual.
- verdict: KILLED

## [s7] Class B is not an allocation question at all but cse pseudo identity, and the target's 2+2 register split over the four post-call stores is reachable from ordinary C.
- mechanism: GCC 2.7.2 cse.c canon_reg() never substitutes a hard register into a use ('Never replace a hard reg'), so the target's sw $zero,0x30($v0) cannot be $v0 forwarding - it must be a SECOND pseudo the allocator seated in $v0. make_regs_eqv() prefers a pseudo over a non-fixed hard reg as qty_first_reg, so the first pseudo the call result is copied into becomes canonical for the whole extended basic block; a later-joining pseudo takes over as canonical only when uid_cuid[regno_last_uid[new]] > cse_basic_block_end or uid_cuid[regno_first_uid[new]] < cse_basic_block_start.
- probe: Twelve spellings built and sandbox-measured on the floor-9 chassis (H1-H8, H10-H12, H15-H17), plus a cc1 -da re-dump of the winning shape and a source read of tools/gcc-2.7.2/cse.c. H5/H6/H7 vary only which base each store is spelled through; H1/H3/H4/H8 give the raw call result its own C name; H10/H12 hoist a u8 *pp to function scope, assign it AFTER the global + 0x4 stores, use it for the 0x30/0x34 stores and reuse it in the tail so its regno_last_uid lies past the cse block end; H11 is the same but block-local (control).
- result: (a) Store-base spelling is byte-neutral - H5/H6/H7 all measure 9 / 175. (b) Naming the raw result first (H1/H3/H4/H8) makes THAT pseudo canonical, routes all four stores through it, deletes the move $s1,$v0 copy and frees a callee-saved register: 170 insns, score 23. (c) H11 measures 9 / 175, confirming the uid-vs-block-boundary canonicality test. (d) H10 DOES produce the split - text1b.cse shows insn 68 (set (mem (symbol_ref D_800A36A0)) (reg 75)), insn 71 (set (mem (plus (reg 75) 4)) (reg 81)), insn 74 (set (reg 76) (reg 75)), insn 77 (set (mem (plus (reg 76) 48)) 0), insn 80 (set (mem (plus (reg 76) 52)) 0) - two stores on pseudo 75 and two on pseudo 76, structurally the target's $s1/$s1/$v0/$v0. Its only defect is insn 74: 176 insns, score 12. (e) The copy is structural - it vanishes only if 75 and 76 share a hard register, which destroys the split, and pseudo 76 cannot instead be born free from (set 76 (reg 2)) because C offers exactly one assignment of a call result and whichever pseudo takes it becomes canonical for the whole block.
- verdict: CONFIRMED

## [s7] s6's D3 address form is 174 insns, one under the target, and can therefore donate the insn that the class-B split copy costs.
- mechanism: s6 recorded D3 (multi-set int row base, written-order pointer chain) at score 12 / 174 insns against its own working base.
- probe: Re-generated D3 against the committed candidate.c on today's chassis and measured (H16); also measured the composites H15 (split + D3) and H17 (split + E4).
- result: D3 measures 12 / 175 here, not 174 - the insn saving does not exist on the ledger chassis. H15 is 15 / 176 and H17 is 16 / 177, confirming there is no cancellation. No form on record is under 175 insns while keeping the floor-9 structure, so the class-B split currently has nothing to pay with.
- verdict: KILLED

## [s8] The target's class-B 2+2 store split costs ZERO extra instructions.
- mechanism: the target's own asm (asm/funcs/func_800770B8.s:29-38) shows one copy
  `addu $s1,$v0,$zero` with the raw call result still live in $v0. When the
  raw-result pseudo is seated in $v0 its birth insn `(set Q (reg:SI 2 v0))` is a
  self-move and is deleted, so the pseudo-to-pseudo copy `(set P (reg Q))` is the
  ONLY move — the same single move our one-pseudo form already emits.
- probe: read the target asm rows 27-38 and every `$s1` mention in the function.
- result: CONFIRMED. $s1 has exactly two lives and is dead after row 36. s7's
  "the split costs one insn, and the cost is structural" is corrected: that was a
  property of H10's spelling (two long-lived pseudos, two copies), not of the split.
- verdict: CONFIRMED (and it retires the "no insn credit available to pay for the
  split" blocker that closed s7)

## [s8] A NOTE_INSN_LOOP_END basic-block fence produces the target's split in cse1 and cse2 destroys it.
- mechanism: cse.c cse_end_of_basic_block() breaks the extended basic block on a
  LOOP_END note only when `! after_loop`. cse pass 1 runs with after_loop=0 (block
  breaks, reg_qty resets, the two pseudos are never equivalent, the split stands);
  cse pass 2 runs with after_loop=1, ignores the note, re-merges the block, and
  make_regs_eqv/canon_reg rewrite every use onto qty_first_reg. flow.c then deletes
  the now-dead copy.
- probe: W1 (H1 shape + `do { } while (0);` between the copy and the stores) built
  with the instrumented cc1; read text1b.rtl, .cse, .loop, .cse2, .flow, .greg in
  tmp/grind/func_800770B8/s8/dW1/.
- result: CONFIRMED as a mechanism, KILLED as a route. .cse shows insns 83/86 on
  reg 75 and 89/92 on reg 81 (the target's exact shape); .cse2 shows 83/86 rewritten
  to reg 81; .flow shows insn 68 as NOTE_INSN_DELETED; .greg shows all four stores on
  $v0. 170 insns / score 23. Any note-based fence — the s4 do-while(0) included —
  is undone two passes before RA and cannot buy class B under any spelling.
- verdict: KILLED (route) / CONFIRMED (mechanism + pass attribution)

## [s8] Within one cse extended basic block, a pseudo-to-pseudo copy ALWAYS collapses, so the target's split is unreachable by ordering or by naming.
- mechanism: make_regs_eqv puts both pseudos in one quantity; canon_reg rewrites
  every REG use onto qty_first_reg. Uses that precede the copy take the pre-copy
  canonical; uses that follow it take the post-copy canonical. The target requires
  the long-lived callee-saved pseudo to be canonical for two stores that FOLLOW the
  copy and the raw call result to be canonical for two other stores that also follow
  it — mutually exclusive.
- probe: seven spellings measured on the floor-9 chassis (all in
  tmp/grind/func_800770B8/s8/v/): H1 23/170, V1 25/170, V2 25/170, W1 23/170,
  W2 23/170, V3 20/175, V4 18/175, plus positional diffs for V3 and V4.
- result: KILLED. Every form collapses. The 170s collapse onto the call-result
  pseudo (copy deleted, callee-saved register freed, frame 0x40 -> 0x38); the 175s
  collapse onto p_old (copy kept) and rows 35/36 stay `sw $0,48($17)`.
- verdict: KILLED

## [s8] The make_regs_eqv canonicality promotion is C-controllable at zero insn cost.
- mechanism: promotion needs
  `(uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]`.
  Reusing `p_old` for the tail block's D_800A36A0 re-read (ordinary variable reuse,
  the same role the throwaway `u8 *p` local plays) pushes its last_uid past the
  outer loop's top label, i.e. past cse_basic_block_end.
- probe: V3 (V1 + tail reuse) and V4 (H1 + tail reuse), sandbox-measured.
- result: CONFIRMED as a lever, KILLED as a class-B fix. Both measure 175 insns
  (against 170 without the reuse), so the promotion itself is free; but the
  canonical flips to p_old and everything collapses onto $s1, and the tail regresses
  (V4 positional diff rows 126-137: `lw $17,0($28)` / `sw $0,32($17)` against the
  target's per-statement `lw ..., %gp_rel(D_800A36A0)` into $v0/$a0). 18/175.
- verdict: CONFIRMED (lever) / KILLED (as a class-B fix)

## [s8] A hard-register address for the 0x30/0x34 stores is not reachable from C.
- mechanism: canon_reg never replaces a hard reg, so `(mem (plus (reg:SI 2 v0) 48))`
  would survive cse, cse2 and RA and would reproduce the target at 175 insns.
- probe: read calls.c expand_call's return path.
- result: KILLED. When the call feeds an assignment, expand passes a TARGET and
  emits `emit_move_insn (target, valreg)` returning the pseudo (calls.c:2039);
  otherwise the tail is `else target = copy_to_reg (valreg);` (calls.c:2114). The
  hard return register never escapes into a later address rtx, and C offers exactly
  one assignment of a call result.
- verdict: KILLED

## [s8] The target's class-B 2+2 store split costs zero extra instructions, not one.
- mechanism: asm/funcs/func_800770B8.s:29-38 carries ONE copy (addu $s1,$v0,$zero) with the raw call result still live in $v0 for the 0x30/0x34 stores. When the raw-result pseudo is seated in $v0, its birth insn (set Q (reg:SI 2 v0)) is a self-move and is deleted, leaving the pseudo-to-pseudo copy (set P (reg Q)) as the only move - the same single move our one-pseudo form emits.
- probe: Read the target asm rows 27-38 and every $s1 mention in the function ($s1 has exactly two lives and is dead after row 36).
- result: The split is free; s7's 'the split costs one insn and the cost is structural' was a property of H10's two-long-lived-pseudo spelling, not of the split.
- verdict: CONFIRMED

## [s8] A NOTE_INSN_LOOP_END basic-block fence makes cse pass 1 emit the target's exact 2+2 pseudo split.
- mechanism: cse.c cse_end_of_basic_block() breaks the extended basic block on a LOOP_END note when !after_loop, so reg_qty resets and the two pseudos are never made equivalent.
- probe: W1 (H1 shape + do{}while(0) between the copy and the stores) built with the instrumented cc1; read tmp/grind/func_800770B8/s8/dW1/text1b.rtl and .cse.
- result: .cse shows insns 83/86 on reg 75 and insns 89/92 on reg 81 - structurally identical to the target's $s1/$s1/$v0/$v0.
- verdict: CONFIRMED

## [s8] That split cannot reach the assembler: cse pass 2 undoes it.
- mechanism: cse pass 2 runs cse_main with after_loop=1, so the '! after_loop' guard no longer fires, the LOOP_END note is ignored, the block re-merges, make_regs_eqv puts both pseudos in one quantity and canon_reg rewrites every use onto qty_first_reg; flow.c then deletes the now-dead copy.
- probe: Read .loop, .cse2, .flow and .greg of the same W1 build; sandbox-measured W1 and its control W2.
- result: .cse2 rewrites 83/86 to reg 81; .flow turns insn 68 into NOTE_INSN_DELETED; .greg shows insn 65 as (set (reg 2 v0) (reg 2 v0)) with all four stores on $v0. W1 23/170, W2 23/170. Any note-based fence, the s4 do-while(0) included, is undone two passes before RA and cannot buy class B under any spelling.
- verdict: KILLED

## [s8] Within one cse extended basic block, some naming or ordering of the two pointers produces the target's split.
- mechanism: make_regs_eqv puts both pseudos in one quantity and canon_reg rewrites uses onto qty_first_reg; uses preceding the copy take the pre-copy canonical, uses following it take the post-copy canonical. The target needs the long-lived callee-saved pseudo canonical for two stores that FOLLOW the copy and the raw call result canonical for two other stores that also follow it - mutually exclusive.
- probe: Seven spellings measured on the floor-9 chassis (tmp/grind/func_800770B8/s8/v/): H1 23/170, V1 25/170, V2 25/170, W1 23/170, W2 23/170, V3 20/175, V4 18/175, plus positional diffs for V3 and V4.
- result: Every form collapses. The 170s collapse onto the call-result pseudo (copy deleted, a callee-saved register freed, frame 0x40 -> 0x38); the 175s collapse onto p_old (copy kept) and rows 35/36 stay sw $0,48($17).
- verdict: KILLED

## [s8] The make_regs_eqv canonicality promotion is C-controllable at zero instruction cost.
- mechanism: Promotion requires (uid_cuid[regno_last_uid[new]] > cse_basic_block_end || uid_cuid[regno_first_uid[new]] < cse_basic_block_start) && uid_cuid[regno_last_uid[new]] > uid_cuid[regno_last_uid[firstr]]. Reusing p_old for the tail block's D_800A36A0 re-read (ordinary variable reuse, the same role the throwaway u8 *p local plays) pushes its last_uid past the outer loop's top label.
- probe: V3 (V1 + tail reuse) and V4 (H1 + tail reuse), sandbox-measured, plus V4's positional diff.
- result: Both measure 175 insns against 170 without the reuse, so the promotion is free - but the canonical flips to p_old, everything collapses onto $s1, and the tail regresses (V4 rows 126-137 become lw $17,0($28) / sw $0,32($17) where the target re-loads D_800A36A0 per statement). 18/175 and 20/175.
- verdict: KILLED

## [s8] A hard-register address for the 0x30/0x34 stores - the only rtx canon_reg refuses to rewrite - is reachable from C.
- mechanism: canon_reg never replaces a hard reg, so (mem (plus (reg:SI 2 v0) 48)) would survive cse, cse2 and RA and reproduce the target at 175 insns.
- probe: Read calls.c expand_call's return path.
- result: When the call feeds an assignment, expand passes a TARGET and emits emit_move_insn (target, valreg), returning the pseudo (calls.c:2039); otherwise the tail is 'else target = copy_to_reg (valreg);' (calls.c:2114). The hard return register never escapes into a later address rtx, and C offers exactly one assignment of a call result.
- verdict: KILLED

## [s9] Class A (prologue rows 8-12) is a 5-insn intra-block permutation that some source-statement order can reach.
- mechanism: sched2's backward list scheduler decides the interleave of the reload-emitted frame saves with the parameter moves and the ClearOTagR setup; the ledger had never read the .sched2 ready lists, so statement order was still an open lever.
- probe: instrumented cc1 -da dumps (tmp/grind/func_800770B8/s9/d/), then tools/sched_solver: extract.py text1b (parity=True), simulate block 0 (base match True), then an EXHAUSTIVE depth-1 sweep of all 3234 input atoms against the exact target emission order (tmp/grind/func_800770B8/s9/probe2.py, probe3.py, probe4.py, probe5.py).
- result: 0/3234 atoms reach the target order. The only sub-goal that is individually reachable ("sw $ra emitted before sw $s1") is reached ONLY by add_dep atoms onto a reload-emitted save store, by one del_dep that removes p_old from $s1, and by cost atoms that turn a register parameter move into a load. No luid / luid_move atom - the spellable statement-move classes - flips it. Root cause: 560 (`sw $ra`) is released by the first `jal` and 566 (`sw $s1`) by `addu $s1,$s0,88`; the jal is emitted after that addu, so in the BACKWARD pass 560 is always released first, and being the only function-unit insn in its priority-1 group it is promoted by schedule_select's potential-hazard rule (sched.c:2708-2721) and taken at once.
- verdict: KILLED

## [s9] Hoisting `p_old = arg0 + 0x58` past the ClearOTagR argument setup fixes the class-A window (the s4 "10 unfenced / 5 fenced" lead).
- mechanism: raising insn 15's LUID above 26/28 would change the rank_for_schedule tie order in the priority-1 group.
- probe: sched_solver simulation of `luid_move 15 -> before 18 / 21 / 30` and `luid swap 15 <-> 28` on the exact block-0 model.
- result: all four give emission 558,568,4,562,6,564,8,26,28,566,15,560,18,21,30 - a different wrong permutation with `sw $s1` still before `sw $ra`. Matches s4's measured 10.
- verdict: KILLED

## [s9] Class B closes if a REAL control-flow boundary (a label/jump that survives to cse pass 2), not a note fence, separates the D_800A36A0 copy from the 0x30/0x34 stores.
- mechanism: cse pass 2 respects CODE_LABEL / JUMP_INSN block boundaries but ignores LOOP_END notes, so a genuine boundary would keep the two pseudos in different quantities through RA and give the target's 2+2 $s1/$v0 store split.
- probe: `if (p_old != 0) { *(s32 *)(D_800A36A0 + 0x30) = 0; *(s16 *)(D_800A36A0 + 0x34) = 0; }` - measured with sandbox --disable all.
- result: 176 insns, score 16 (baseline 175 / 9). The boundary is not free in instructions, exactly as the target's straight-line asm predicts, and the branch also perturbs the surrounding allocation.
- verdict: KILLED

## [s9] Class C's plus-operand order can be flipped without spending an instruction by making the SHIFT the pointer operand and the global the integer operand.
- mechanism: c-typeck.c:1988 rewrites `int + pointer` to `pointer_int_sum(PLUS_EXPR, ptr, int)` and c-typeck.c:2695 builds the PLUS with ptrop as operand 0 unconditionally, so the operand order is decided by WHICH SIDE IS POINTER-TYPED, not by written order; and the c-typeck.c:2654-2678 distributive law only moves a constant off intop when intop is a non-constant PLUS/MINUS, so keeping `+ 0x6A` as a separate outer pointer_int_sum with a CONSTANT intop leaves the shift clean.
- probe: `(s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A)` (and a named-`row`-local variant), measured with sandbox --disable all and read back from a fresh cc1 dump.
- result: CONFIRMED for the flip itself - 175 insns (s6/s7 had this at 176) and the emitted `lw $3,D_800A36A0; sll $2,$2,1; addu $2,$2,$3; addu $7,$2,106; addu $5,$2,126` has the target's operand order with the dest tied to the shift pseudo. But the whole-function score is 33, not 9: the inner loop's addressing chain re-schedules and re-allocates around the new tree shape. Rows 62-64 become a REGISTER SEAT question (target: lw in $v0, sll in $v1; ours: reversed).
- verdict: CONFIRMED (mechanism) / KILLED (as a spelling that lowers the floor)

## [s9] Class A (prologue rows 8-12) is a 5-insn intra-block permutation that some source-statement order can reach.
- mechanism: sched2's backward list scheduler decides the interleave of the reload-emitted frame saves with the parameter moves and the ClearOTagR setup. Each save is released only by the insn that clobbers its register: 568<-4, 562<-6, 564<-8, 566<-insn 15 (addu $s1,$s0,88), 560<-insn 30 (the first jal, which sets $ra). The jal is emitted AFTER that addu so it is PICKED BEFORE it in the backward pass, hence sw $ra (UID 560) is always released strictly earlier than sw $s1 (566); being the only function-unit insn in its priority-1 group it is then promoted by schedule_select's potential-hazard rule (sched.c:2708-2721, printed as ';; insn 560 has a greater potential hazard') and taken at once.
- probe: Instrumented cc1 -da dumps (tmp/grind/func_800770B8/s9/d/, text1b.sched2 block 0 at line 75962) mapped 1:1 onto both asm streams; then tools/sched_solver extract.py text1b (parity=True, 486 funcs / 1752 blocks / 14250 picks), simulate block 0 (base match True), then an EXHAUSTIVE depth-1 sweep of all 3234 input atoms (add_dep / del_dep / luid / luid_move / cost) against the exact target emission order.
- result: 0/3234 atoms reach the target order. Only the sub-goal 'sw $ra emitted before sw $s1' is individually reachable, by 32 atoms: 26 x add_dep onto a reload-emitted save store (not expressible in C), 1 x del_dep 15<-566 (p_old not living in $s1, an RA outcome, and it still does not give the target order), 5 x cost atoms turning the register-parameter move into a multi-cycle load (instruction selection). NO luid or luid_move atom - the toolkit's spellable statement-move classes - flips the pair.
- verdict: KILLED

## [s9] Hoisting `p_old = arg0 + 0x58` past the ClearOTagR argument setup fixes the class-A window (the s4 '10 unfenced / 5 fenced' lead).
- mechanism: Raising insn 15's LUID above insns 26/28 would change the rank_for_schedule LUID-descending tie order inside the priority-1 group.
- probe: sched_solver simulation of luid_move 15 -> before 18 / 21 / 30 and luid swap 15 <-> 28 on the exact block-0 model (tmp/grind/func_800770B8/s9/probe5.py).
- result: All four give emission 558,568,4,562,6,564,8,26,28,566,15,560,18,21,30 - a different wrong permutation with sw $s1 still before sw $ra. This is the model's explanation of s4's measured 10 for that move.
- verdict: KILLED

## [s9] Class B closes if a REAL control-flow boundary (a label/jump surviving to cse pass 2), not a note fence, separates the D_800A36A0 copy from the 0x30/0x34 stores.
- mechanism: cse pass 2 (after_loop=1) respects CODE_LABEL / JUMP_INSN block boundaries but ignores LOOP_END notes, so a genuine boundary would keep the two pseudos in different quantities through RA and give the target's 2+2 $s1/$v0 store split.
- probe: if (p_old != 0) { *(s32 *)(D_800A36A0 + 0x30) = 0; *(s16 *)(D_800A36A0 + 0x34) = 0; } measured with `sandbox func_800770B8 --disable all`.
- result: 176 insns, score 16 (baseline 175 / 9). The boundary is not free in instructions - exactly as the target's straight-line asm predicts - and the branch also perturbs the surrounding allocation. Banked as rejected/s9-classB-real-branch-boundary-176insn-score16.c. Class B now has no untested mechanism on record.
- verdict: KILLED

## [s9] Class C's plus-operand order at insn 173 can be flipped without spending an instruction by making the SHIFT the pointer operand and the global the integer operand.
- mechanism: build_binary_op's PLUS_EXPR case rewrites `int + pointer` to pointer_int_sum(PLUS_EXPR, op1, op0) (c-typeck.c:1988) and pointer_int_sum ends `result = build (resultcode, result_type, ptrop, intop);` (c-typeck.c:2695) - the POINTER-typed side is unconditionally operand 0, so the operand order is decided by which side is pointer-typed, not by written order. Separately c-typeck.c:2654-2678 applies a distributive law that moves a constant term off intop onto ptrop only when intop is a NON-CONSTANT PLUS/MINUS, so keeping `+ 0x6A` as its own outer pointer_int_sum with a constant intop leaves the shift clean (this is the s6 '0x6A folds onto the shift' hazard, now named).
- probe: (s16 *)((u8 *)(t0 * 10) + (s32)D_800A36A0 + 0x6A) and a named-`row`-local variant, measured with sandbox --disable all and read back from a fresh instrumented-cc1 dump (tmp/grind/func_800770B8/s9/dC2/).
- result: CONFIRMED for the flip: 175 insns (s6/s7 had this shape at 176) emitting `lw $3,D_800A36A0; sll $2,$2,1; addu $2,$2,$3; addu $7,$2,106; addu $5,$2,126` - the target's operand order, dest tied to the shift pseudo, constants off the shift. Rows 62-64 therefore reduce from an operand-order question to a two-register SEAT question (target: lw in $v0, sll in $v1; ours reversed). NOT a win as a spelling: whole-function score 33 (vs 9) because the inner loop's addressing chain re-schedules and re-allocates around the new tree shape. Banked as rejected/s9-classC-shift-as-ptrop-flips-order-free-but-score33.c and s9-classC-shift-as-ptrop-named-row-local-score33.c.
- verdict: CONFIRMED

## [s10] The original C used a struct type for the D_800A36A0 block, and writing the accesses as struct/array references reproduces the target's addressing.
- mechanism: the block is addressed only through one base register at strides 1/2/4/10, which is genuine aggregate evidence; an ARRAY_REF/COMPONENT_REF tree would build the row addresses differently from our explicit pointer arithmetic and could change the plus operand order at insn 173.
- probe: full struct-typed rewrite (unk08/0C/10/14/3C[2], unk40[2][2], unk68[2], unk6A[2][5], unk7E[2][5]) applied to src/text1b.c and measured with `sandbox func_800770B8 --disable all`.
- result: 178 insns / score 22 (baseline 175 / 9). Two regressions: LICM hoists `&D_800A35D0 + 2` into an outer-preheader register (+3 insns, absent from the target), and the 0x6A/0x7E constant folds onto the INDEX side (`addiu $2,$3,106` before the base addu) instead of remaining a trailing addiu. The plus operand order at insn 173 did NOT flip. Banked rejected/s10-struct-typed-rederive-licm-hoists-D3-178insn.c.
- verdict: KILLED

## [s10] The class-C flip's +24 score collateral is a spelling artifact of s9's cast form, and re-spelling only the 0x6A/0x7E row will confine the damage.
- mechanism: s9 hypothesised the flipped tree re-seats the loop body's other addressing chains and that keeping the `base` local for the 0x10/0x40 chains would leave them intact.
- probe: four distinct flip spellings measured with `sandbox func_800770B8 --disable all`: s9's `(u8 *)(t0*10) + (s32)D_800A36A0 + 0x6A`; the same with operands textually swapped; the pure int-domain `(t0*10) + (s32)D_800A36A0 + 0x6A`; and a named `s32 row10 = t0 * 10;` intermediate (plus a named `s32 i4 = t0 * 4;` index variant).
- result: ALL measure exactly 175 insns / score 33 with a byte-identical positional diff. The collateral is intrinsic to the flipped tree, not to a spelling. Banked rejected/s10-classC-flip-int-domain-same-score33-collateral.c, s10-classC-flip-named-i4-index-score33.c, s10-classC-flip-named-shift-local-qty4-span-unchanged-score33.c.
- verdict: KILLED

## [s10] Class C's register seat is reachable by a C-spellable perturbation of local-alloc's inputs.
- mechanism: the addu dest ties to operand 0, so the flip moves the dest's quantity merge from the short lw pseudo (span 8, refs 10, qty_compare pri 37500) to the long sll chain (span 28, refs 22, pri 31428). Both rank ord1 in block 1 and both take $v0; the target needs the merged quantity in $v1 and the bare lw in $v0.
- probe: tools/ra_solver/local_extract.py QTYDBG ground truth on BOTH builds (tmp/grind/func_800770B8/s10/text1b.local.BASE.json and text1b.local.FLIPPED.json), then tools/ra_solver/inverse.py local --func func_800770B8 --block 1 --swap 3,4 --depth 2 over a 392-atom space, then a direct measurement of the one C-plausible family.
- result: the solver returns REACHABLE with 30 minimal single-atom vectors in exactly three families - live_shrink on qty0 (the sign-extension of t0, born >=33 instead of 6), live_shrink on qty1 (the FIRST D_800A36A0 read, born >=33 instead of 10), and live_shrink/refs_up on qty4 (the SECOND D_800A36A0 read, span 4->2 or refs 4->7). Families 1 and 2 contradict the target's own emission (its sext and first lw ARE the first insns of the loop body, rows 38-41), so they describe a different function, not a spelling of this one. Family 3 was measured: naming the shift in its own statement leaves the QTYDBG rows byte-identical (qty4 still [48,52) refs 4), i.e. GCC emits the global read before the final `sll ...,1` regardless of spelling, and refs 7 would need the second read used seven times where the target uses it twice. Class C is therefore REACHABLE-in-model but FORECLOSED to every C-spellable input perturbation found at depth 2.
- verdict: KILLED

## [s10] Statement-level restructuring outside the three residual classes can still move the floor.
- mechanism: the rederive ladder's remaining shapes - the outer loop as a `for`, and the call-result local `r` removed by nesting the two calls (m2c's literal reading) - change pseudo birth order around the prologue and the call sequence, which is where classes A and B live.
- probe: both shapes applied and measured with `sandbox func_800770B8 --disable all`.
- result: both measure 9 / 175 insns - byte-neutral. Banked rejected/s10-outer-for-loop-byte-neutral.c and rejected/s10-nested-call-no-r-local-byte-neutral.c.
- verdict: KILLED

## [s10] The original C used a struct type for the D_800A36A0 block, and writing the accesses as struct/array references reproduces the target's addressing.
- mechanism: The block is addressed only through one base register at strides 1/2/4/10 - genuine aggregate evidence - so an ARRAY_REF/COMPONENT_REF tree would build the row addresses differently from our explicit pointer arithmetic and could change the plus operand order at insn 173.
- probe: Full struct-typed rewrite (s16 unk08/0C/10/14/3C[2], s16 unk40[2][2], u8 unk68[2], s16 unk6A[2][5], s16 unk7E[2][5]) applied to src/text1b.c and measured with sandbox func_800770B8 --disable all.
- result: 178 insns / score 22 (baseline 175 / 9). LICM hoists `&D_800A35D0 + 2` into an outer-preheader register (+3 insns; the target has no such hoist) and the 0x6A/0x7E constant folds onto the INDEX side (`addiu $2,$3,106` before the base addu). The plus operand order at insn 173 did not flip. Banked rejected/s10-struct-typed-rederive-licm-hoists-D3-178insn.c.
- verdict: KILLED

## [s10] The class-C flip's +24 score collateral is a spelling artifact of s9's cast form, and re-spelling only the 0x6A/0x7E row confines the damage.
- mechanism: s9 hypothesised the flipped tree re-seats the loop body's other addressing chains and that keeping the `base` local for the 0x10/0x40 chains would leave them intact.
- probe: Four distinct flip spellings measured with sandbox --disable all: s9's `(u8 *)(t0*10) + (s32)D_800A36A0 + 0x6A`; the same with operands textually swapped; the pure int-domain `(t0*10) + (s32)D_800A36A0 + 0x6A`; a named `s32 row10 = t0 * 10;` intermediate; plus a named `s32 i4 = t0 * 4;` index variant.
- result: Every one measures exactly 175 insns / score 33 with a byte-identical positional diff. The collateral is intrinsic to the flipped tree, not to a spelling. Also corrects s6: the pure int-domain form KEEPS the second lw of D_800A36A0 on this chassis. Banked three rejected forms.
- verdict: KILLED

## [s10] Class C's register seat is reachable by a C-spellable perturbation of local-alloc's inputs.
- mechanism: The addu dest ties to operand 0, so the flip moves the dest's quantity merge from the short lw pseudo (span 8, refs 10, qty_compare pri 37500) to the long sll chain (span 28, refs 22, pri 31428). Both rank ord1 in block 1 and both take $v0; the target needs the merged quantity in $v1 and the bare lw in $v0.
- probe: tools/ra_solver/local_extract.py QTYDBG ground truth on BOTH builds, then tools/ra_solver/inverse.py local --func func_800770B8 --block 1 --swap 3,4 --depth 2 over a 392-atom space, then a direct measurement + re-extraction of the one C-plausible family.
- result: REACHABLE with 30 minimal single-atom vectors in three families: live_shrink qty0 (the sign-extension of t0, born >=33 instead of 6), live_shrink qty1 (the FIRST D_800A36A0 read, born >=33 instead of 10), and live_shrink/refs_up qty4 (the SECOND D_800A36A0 read, span 4->2 or refs 4->7). Families 1 and 2 contradict the target's own emission (its sext and first lw ARE the loop body's first insns, rows 38-41). Family 3 was measured: naming the shift in its own statement leaves the QTYDBG rows byte-identical (qty4 still [48,52) refs 4), so GCC emits the global read before the final `sll ...,1` regardless of spelling. Class C is REACHABLE-in-model but FORECLOSED to every C-spellable depth-2 input perturbation.
- verdict: KILLED

## [s10] Statement-level restructuring outside the three residual classes can still move the floor.
- mechanism: The rederive ladder's remaining shapes - the outer loop as a `for`, and the call-result local `r` removed by nesting the two calls (m2c's literal reading) - change pseudo birth order around the prologue and the call sequence, where classes A and B live.
- probe: Both shapes applied and measured with sandbox func_800770B8 --disable all.
- result: Both measure 9 / 175 insns - byte-neutral. Banked rejected/s10-outer-for-loop-byte-neutral.c and rejected/s10-nested-call-no-r-local-byte-neutral.c.
- verdict: KILLED

## [s11] The s4 empty do-while(0) fence is a SANCTIONED construct under the current rules, not the cheat s4 recorded, so the honest floor is 5 rather than 9.
- mechanism: `.claude/rules/do-while-zero-exception.md` carries owner ruling 2026-07-06, whose scope sentence reads "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation". Its body states "The former scoping to the reorg.c label-note mechanism is abolished", and its Confirmed-applications section records marionation_Exec as a wrap candidate reviewer-FAILed under the old scoping and reinstated by this ruling. s4 rejected the form under the abolished scoping.
- probe: Re-applied rejected/s4-dw0-fence-plus-pold-move-FLOOR5-CHEAT.c verbatim and measured `sandbox func_800770B8 --disable all` on today's chassis; then re-read the positional diff with tmp/grind/func_800770B8/s3/posdiff.py.
- result: **score 5, 175/175 insns** (vs 9 for the s3 body measured in the same session). The positional diff loses residual class A entirely - the prologue matches row for row - leaving only class B (rows 35-36) and class C (rows 62-64) plus the row-50 LO16 scorer artifact. candidate.c replaced with the annotated floor-5 form.
- verdict: CONFIRMED

## [s11] A SECOND wrap, somewhere else in the body, closes class B or class C.
- mechanism: If NOTE_INSN_LOOP_BEG/END pairs can bound a scheduling region (proven for class A), they can also break a cse extended basic block (class B) or change local-alloc's loop-depth ref weighting (class C, the s10 frontier's closed-form target of span > 44 or refs <= 15).
- probe: EXHAUSTIVE single-wrap sweep - an empty `do { } while (0);` inserted at each of the 63 legal statement positions of the floor-5 body (all lines ending `;` or `{` that are not declarations), each measured with sandbox --disable all. Generator/manifest tmp/grind/func_800770B8/s11/sw/manifest.tsv, results tmp/grind/func_800770B8/s11/sweep.log.
- result: minimum 5 over all 63 positions; nothing beats the wrap already in the form. Class-B-adjacent positions regress (10 before the stores, 11 after), consistent with s8's dump-proven cse-pass-2 undo. The whole outer-loop-body region is byte-neutral.
- verdict: KILLED

## [s11] Nested do-while(0) wraps multiply local-alloc's loop-depth ref weighting enough to move the class-C seat.
- mechanism: GCC 2.7.2's flow.c accumulates REG_N_REFS with a loop-depth weight, and local-alloc's qty_compare priority is floor_log2(refs)*refs*size/(death-birth)*10000; extra nesting levels around the second D_800A36A0 read were the only remaining route to the s10 closed-form target (raise qty4 above the merged chain, or drop the chain below 20000).
- probe: depth-2 and depth-3 wraps at 9 positions in and around the outer-loop body and the p_6a/p_7e inner loop, 18 variants, all measured. Log tmp/grind/func_800770B8/s11/nsweep.log.
- result: best 5; and **depth 3 is byte-identical to depth 2 at every one of the 9 positions** (8/8, 42/42, 7/7, 60/60, 5/5). Nesting does not reach the contested quantities at all. Banked rejected/s11-nested-dw0-depth2-in-6a7e-block-score8.c and s11-nested-dw0-depth2-inner-loop-byte-neutral-score5.c.
- verdict: KILLED

## [s11] The class-C operand flip's +24 collateral is specific to the floor-9 chassis and shrinks once class A is closed.
- mechanism: If the collateral were partly the prologue's re-scheduling, closing class A with the wrap would reduce it.
- probe: The s9/s10 flip spelling applied on top of the floor-5 body, alone and combined with wraps, measured with sandbox --disable all.
- result: 29 (vs 5 baseline) - the collateral is 24 rows on both chassis, so it is intrinsic to the flipped tree exactly as s10 concluded. Flip + class-B wrap 34; a wrap immediately before the flipped decls fuses the inner loops (60 / 163 insns). Banked rejected/s11-classC-flip-on-floor5-chassis-score29.c and s11-classB-wrap-plus-classC-flip-score34.c.
- verdict: KILLED

## [s11] The s4 empty do-while(0) fence is a SANCTIONED construct under the current rules, not the cheat s4 recorded, so the honest floor is 5 rather than 9.
- mechanism: .claude/rules/do-while-zero-exception.md carries owner ruling 2026-07-06 whose scope sentence sanctions 'do { ... } while (0);' (any body, incl. empty) as a pure-C match device for ANY codegen effect incl. register allocation, with a mandatory inline FAKE annotation; its body states 'The former scoping to the reorg.c label-note mechanism is abolished' and its Confirmed-applications section records marionation_Exec as a wrap candidate reviewer-FAILed under the old scoping and reinstated by this ruling. s4 rejected the form under the abolished scoping.
- probe: Re-applied rejected/s4-dw0-fence-plus-pold-move-FLOOR5-CHEAT.c verbatim, measured 'sandbox func_800770B8 --disable all' on today's chassis, then re-read the positional diff with tmp/grind/func_800770B8/s3/posdiff.py.
- result: score 5, 175/175 insns (the s3 body measured 9 in the same session). Residual class A (prologue rows 7-12) is GONE - the prologue matches row for row. Remaining: class B rows 35-36, class C rows 62-64, plus the known row-50 LO16 scorer artifact.
- verdict: CONFIRMED

## [s11] A SECOND do-while(0) wrap somewhere else in the body closes class B or class C.
- mechanism: If NOTE_INSN_LOOP_BEG/END pairs can bound a sched2 region (proven for class A), they might also break a cse extended basic block (class B) or change local-alloc's loop-depth ref weighting (class C, s10's closed-form target: span > 44 or refs <= 15).
- probe: EXHAUSTIVE single-wrap sweep - an empty 'do { } while (0);' inserted at each of the 63 legal statement positions of the floor-5 body (every line ending ';' or '{' that is not a declaration), each measured with sandbox --disable all. Manifest tmp/grind/func_800770B8/s11/sw/manifest.tsv, results tmp/grind/func_800770B8/s11/sweep.log.
- result: Minimum 5 over all 63 positions; nothing beats the wrap already in the form. Class-B-adjacent positions regress (10 before the stores, 11 after), consistent with s8's dump-proven cse-pass-2 undo. The whole outer-loop-body region P025-P051 is byte-neutral at 5.
- verdict: KILLED

## [s11] Nested do-while(0) wraps multiply local-alloc's loop-depth ref weighting enough to move the class-C seat.
- mechanism: flow.c accumulates REG_N_REFS with a loop-depth weight and local-alloc's qty_compare priority is floor_log2(refs)*refs*size/(death-birth)*10000; extra nesting was the only remaining route to s10's closed-form target.
- probe: depth-2 and depth-3 wraps at 9 positions in and around the outer-loop body and the p_6a/p_7e inner loop - 18 variants, all measured. Log tmp/grind/func_800770B8/s11/nsweep.log.
- result: Best 5, and depth 3 measures BYTE-IDENTICALLY to depth 2 at every one of the 9 positions (8/8, 42/42, 7/7, 60/60, 5/5). Nesting does not reach the contested quantities at all.
- verdict: KILLED

## [s11] The class-C operand flip's +24 collateral is specific to the floor-9 chassis and shrinks once class A is closed.
- mechanism: If part of the collateral were the prologue re-scheduling, closing class A with the wrap would reduce it.
- probe: The s9/s10 flip spelling applied on the floor-5 body, alone and combined with wraps, measured with sandbox --disable all.
- result: 29 against a 5 baseline - the collateral is 24 rows on BOTH chassis, so it is intrinsic to the flipped tree exactly as s10 concluded. Flip + class-B wrap 34; a wrap immediately before the flipped decls fuses the inner loops (60 / 163 insns).
- verdict: KILLED

## [s11] Endgame-lock gate (a): func_800770B8 qualifies for the canonical-asm grant path.
- mechanism: The disposition brief requires the scan_hand_coded tier before any foreclosure or grant record.
- probe: python3 tools/scan_hand_coded.py --single func_800770B8
- result: tier=LOW score=0/8, 'no strong hand-coded indicators'; S1-S8 all unset (175 insns, 5 spills, 14 distinct regs). The canonical-asm grant path is NOT available to this function.
- verdict: KILLED

## [s12] The class-C plus-operand flip is intrinsically worth +24 rows of collateral, so class C is FORECLOSED-in-C (the s10/s11 typing).
- mechanism: s10 measured four flip spellings at 175/33 with byte-identical positional diffs and concluded "the collateral is intrinsic to the flipped tree, not to a spelling"; s11 re-measured it at 29 on the floor-5 chassis and concluded the same. Both sessions varied the SPELLING OF THE FLIP while holding the rest of the loop body fixed.
- probe: hold the flip FIXED (int-domain spelling `(t0 * 10) + (s32)D_800A36A0 + 0x6A/0x7E`) and vary the REST of the outer-loop body instead. Exhaustive 24-way permutation of the four store groups of the outer loop body - A = `ptr = (t0*2)+base` + the five sh at 0x10/0x8/0xC/0x14/0x3C; B = `ptr = &D_800A35D0 + t0*4` + sh at +2/+0; C = `ptr = base + t0*4` + sh at 0x42/0x40; D = the `sb` at base+t0+0x68 - each measured with `sandbox func_800770B8 --disable all`. Generator tmp/grind/func_800770B8/s12/gen_perm.py, results tmp/grind/func_800770B8/s12/perm.log.
- result: KILLED. The collateral is NOT intrinsic. Source order ABCD (the order all prior sessions used) is one of the WORST of the 24: score 29. Group order CABD scores **12**, CADB 13, BACD 14, CBAD 16, BADC 16, CBDA 17. Spread 12..38 over the 24 orders. More importantly the CABD build closes residual class C outright: its objdump reads `lw $2,0($28) / sll $3,$3,0x1 / addu $3,$3,$2 / addiu $7,$3,106 / addiu $5,$3,126`, i.e. exactly the target's `lw $v0 / sll $v1,$v1,1 / addu $v1,$v1,$v0 / addiu $a3,$v1,0x6A / addiu $a1,$v1,0x7E` including the register seats ($v1 for the merged chain, $v0 for the bare lw). Rows 62-64 do not appear in the CABD positional diff at all.
- verdict: KILLED (and it overturns the s10 "class C is FORECLOSED to every C-spellable input perturbation" typing: class C is REACHABLE and has now been REACHED in a real build)

## [s12] A do-while(0) note fence can recover the flip's scheduling collateral.
- mechanism: the flipped ABCD build delays `sll <t0*4>` from target row 42 to row 50 and emits the D_800A36A0 lw before the t0*2 shift, which looks like a sched1 priority change; NOTE_INSN_LOOP_BEG/END pairs bound scheduling regions (proven for class A in s11).
- probe: exhaustive second-wrap sweep - an empty `do { } while (0);` at each of the 79 legal statement positions of the flipped ABCD body (which already carries the class-A wrap), all measured. Generator tmp/grind/func_800770B8/s12/gen_wrap.py, results tmp/grind/func_800770B8/s12/wrapflip.log.
- result: minimum 29 over all 79 positions (42 of the 79 are byte-identical at 29; the rest regress to 30-40, three of them by changing the instruction count). No wrap position touches the flip collateral at all. This is the second exhaustive wrap sweep on this function (s11 did 63 positions on the unflipped body) and it confirms wraps reach the prologue scheduling region and nothing else.
- verdict: KILLED

## [s12] Naming the loop body's shared index expressions as locals recovers the flip's collateral (the s10 "named i4" lever, re-run on the floor-5 chassis).
- mechanism: the flip delays the birth of the t0*4 chain; hoisting the shared subexpressions into their own statements at the top of the loop body should force them to be computed early, as the target does at rows 40/42.
- probe: six variants on the flipped ABCD body, measured: R1 `s32 i2 = t0*2; s32 i4 = t0*4;` both hoisted; R2 i4 only; R3 i2 only; R4 `u8 *pC = base + (t0*4);` hoisted; R5 pC + `u8 *pB = (u8 *)&D_800A35D0 + (t0*4);` hoisted; R6 = R1 applied to the UNFLIPPED floor-5 body as a control. Also R7-R12 adding `u8 *pA` and `u8 *pD` hoists in every combination. Logs tmp/grind/func_800770B8/s12/r.log and r2.log.
- result: INTEGER index hoists are completely inert on the flipped body - R1/R2/R3 all 29, identical to no hoist at all. POINTER hoists do move it: R4 (pC) 25, R5/R7/R8/R9 (pB+pC, with or without pA/pD) 20, R10 (pC+pD) 25. R11 (pB alone) REGRESSES to 40 because hoisting the &D_800A35D0 base makes LICM lift its `lui/addiu` pair out of the outer loop entirely (they appear at build rows 30-31, in the prologue; the target computes them inside the loop at rows 49-50). R6 control measures 5 - the index hoists are byte-neutral on the unflipped body, so they are a free structural degree of freedom there.
- verdict: KILLED as a route to closing the flip collateral (best 20, vs 12 for plain CABD reordering); CONFIRMED as the mechanism note that pointer-valued hoists, not integer-valued ones, are what move this function's loop-head schedule.

## [s12] A second do-while(0) wrap recovers the CABD basin's store-group displacement.
- mechanism: in the CABD flipped build class C is closed and the entire residual is the displacement of the C store group; a NOTE fence between the store groups might re-order the emission back toward ABCD.
- probe: exhaustive second-wrap sweep at all 79 legal statement positions of the CABD flipped body (which already carries the class-A wrap), all measured. Log tmp/grind/func_800770B8/s12/wrapq12.log, manifest wrapq12/manifest.tsv.
- result: minimum 10, at exactly two positions - W039 (before `*(s16 *)(ptr + 0) = 0;`, the last store of the B group) and W040 (before the D-group `sb`), with W038 at 11. 38 of the 79 positions are byte-identical at 12. So a second wrap is worth 2 rows in this basin and no more; the CABD basin bottoms out at 10, which is still 5 worse than the plain unflipped floor-5 form and would additionally cost a second FAKE-annotated wrap. Banked rejected/s12-classC-closed-CABD-plus-second-wrap-score10.c.
- verdict: KILLED

## [s12] The flip's ABCD-order collateral is caused by the SPELLING of the other three store groups (base local, pointer vs int-domain addressing, one-step vs two-step &D_800A35D0).
- mechanism: if the target's original C is A-first (its emission is: sext, sll t0*2, lw, sll t0*4, then the A stores), and A-first + the flip measures 29 in our body, then our body must differ from the original in the spelling of some group other than class C.
- probe: eight spelling variants of groups A/B/D on the flipped ABCD body, measured: U1 `ptr = base + (t0*2)`; U2 `(t0*2) + (s32)D_800A36A0` (re-read instead of the `base` local); U3 `ptr = D_800A36A0 + (t0*2)`; U4 the `base` local deleted entirely (every group re-reads the global); U5 `base` declared `s32` instead of `u8 *`; U6 the two-step `&D_800A35D0` assignment collapsed into one statement; U7 the D-group store written as `base[t0 + 0x68]`; U8 the C-group pointer written int-domain. Log tmp/grind/func_800770B8/s12/u.log.
- result: U1/U2/U3/U5/U7/U8 all measure exactly 29 / 175 - byte-identical to the un-respelled flipped ABCD body. U4 costs an instruction (176 / 32): deleting the `base` local makes GCC emit a third D_800A36A0 read. U6 measures 31. So the spelling of the non-class-C groups is completely inert on this chassis; only their ORDER moves the codegen. This narrows the remaining unknown: the difference between our body and the original is a statement-level structure that is neither the class-C expression, nor the four groups' order (24/24 measured), nor the spelling of groups A/B/D (8 measured).
- verdict: KILLED

## [s12] The class-C plus-operand flip is intrinsically worth ~24 rows of collateral, so class C is FORECLOSED-in-C (the s10/s11 typing).
- mechanism: s10/s11 varied the SPELLING of the flip while holding the loop body at its ABCD store-group order and got 175/33 and 175/29 with byte-identical positional diffs, concluding the collateral was intrinsic to the flipped tree.
- probe: Inverted the experiment: held the flip fixed (int-domain spelling) and measured all 24 permutations of the outer loop body's four store groups (A = t0*2 base + five sh; B = &D_800A35D0 + t0*4 + two sh; C = base + t0*4 + two sh; D = the 0x68 sb). tmp/grind/func_800770B8/s12/gen_perm.py, perm.log.
- result: Spread 12..38, all 175 insns. ABCD (the order every prior session used, and the order the target emits stores in) is among the worst at 29; CABD scores 12, CADB 13, BACD 14. The CABD build closes class C outright - objdump reads lw $2,0($28) / sll $3,$3,0x1 / addu $3,$3,$2 / addiu $7,$3,106 / addiu $5,$3,126, matching the target's rows 60-64 including the seats ($v1 merged chain, $v0 bare lw). Rows 62-64 do not appear in its positional diff.
- verdict: KILLED

## [s12] A do-while(0) note fence can recover the flip's scheduling collateral on the ABCD body.
- mechanism: The flipped ABCD build delays the t0*4 shift from target row 42 to row 50 and emits the D_800A36A0 lw before the t0*2 shift, which looks like a sched1 priority change; NOTE_INSN_LOOP_BEG/END pairs bound scheduling regions (proven for class A in s11).
- probe: Exhaustive second-wrap sweep at all 79 legal statement positions of the flipped ABCD body. tmp/grind/func_800770B8/s12/wrapflip.log.
- result: Minimum 29 over all 79 positions; 42 of the 79 are byte-identical to no wrap at all, the rest regress to 30-40 (three by changing insn count). No position touches the flip collateral.
- verdict: KILLED

## [s12] A second wrap recovers the CABD basin's store-group displacement.
- mechanism: In the CABD build class C is closed and the whole residual is the displacement of the C store group, so a NOTE fence between store groups might re-order emission back toward ABCD.
- probe: Exhaustive second-wrap sweep at all 79 legal statement positions of the CABD flipped body. tmp/grind/func_800770B8/s12/wrapq12.log + wrapq12/manifest.tsv.
- result: Minimum 10, at exactly two positions (W039 before the B group's last store, W040 before the D-group sb), W038 at 11, 38 of 79 byte-identical at 12. The CABD basin bottoms out at 10 - still 5 worse than the plain unflipped form, and it would cost a SECOND FAKE-annotated wrap.
- verdict: KILLED

## [s12] Naming the loop body's shared index expressions as locals recovers the flip's collateral.
- mechanism: The flip delays the birth of the t0*4 chain; hoisting shared subexpressions into their own statements at the top of the loop body should force them computed early, as the target does at rows 40/42.
- probe: Twelve variants on the flipped ABCD body: integer hoists s32 i2/i4 (both, either); pointer hoists u8 *pA/pB/pC/pD in every combination; plus the integer hoists applied to the UNFLIPPED floor-5 body as a control. tmp/grind/func_800770B8/s12/r.log, r2.log.
- result: Integer index hoists are completely inert (29, byte-identical to no hoist). Pointer hoists move it: pC 25, pB+pC 20 (adding pA and/or pD changes nothing), pC+pD 25. pB ALONE REGRESSES to 40 - hoisting the &D_800A35D0 base makes LICM lift its lui/addiu pair out of the outer loop into the prologue (build rows 30-31), the same LICM hazard s10's struct rederive hit, now a two-witness fact. Control: the integer hoists measure 5 on the unflipped body, i.e. byte-neutral - a free structural degree of freedom.
- verdict: KILLED

## [s12] The flip's ABCD-order collateral is caused by the spelling of the OTHER three store groups.
- mechanism: The target's emission is A-first (sext, sll t0*2, lw, sll t0*4, then the A stores); A-first plus the flip measures 29 in our body, so our body must differ from the original in some non-class-C group's spelling.
- probe: Eight spelling variants of groups A/B/D on the flipped ABCD body (pointer vs int-domain A address, base local vs re-read, base deleted, base typed s32, one-step &D_800A35D0, array-subscript D store, int-domain C address). tmp/grind/func_800770B8/s12/u.log.
- result: Six of the eight measure exactly 29 / 175, byte-identical to the un-respelled body. Deleting the base local costs an instruction (176 / 32, GCC emits a third D_800A36A0 read); collapsing the two-step &D_800A35D0 assignment gives 31. The spelling of the non-class-C groups is inert; only their ORDER moves codegen.
- verdict: KILLED

## [s13] structural — 2026-09-01 (floor 5, unchanged)

### H13.1 — A loop-SHAPE change (not statement order, not spelling) makes the class-C flip free
- statement: s12 exhausted every statement-level degree of freedom it could enumerate
  (group order 24/24, group spelling, hoists, wrap positions) and concluded the only
  unmeasured statement-level structure was the shape of the loops themselves, which is
  what sets the pseudo birth order the class-C flip perturbs.
- mechanism: the inner p_6a/p_7e loop, the outer t0 loop and the third a2<0xA loop each
  fix a NOTE_INSN_LOOP_BEG/END placement and an induction-variable expansion; changing
  the shape changes loop.c's biv/giv discovery and hence the RTL emission order that
  the flip perturbs.
- probe: 315 builds. (a) inner-loop shape x 4 bases x {a2=0 hoisted, not} = 48
  (tmp/grind/func_800770B8/s13/sh.log); (b) the inner-loop block promoted to a fifth
  permutable element alongside the four store groups, all 5! orders x {flipped,
  unflipped} = 240 (tmp/grind/func_800770B8/s13/p5.log); (c) outer-loop shape x
  third-loop shape x 3 bases = 27 (tmp/grind/func_800770B8/s13/lp.log).
- result: for / while / do-while are BYTE-IDENTICAL for the inner loop on all four
  bases and for the outer loop on all three bases — zero information in either axis.
  Pointer-walking the inner loop elides two instructions the target has (173 vs 175);
  splitting it into two loops costs eleven (186); swapping its two stores costs +4
  uniformly. The third loop's `for` is uniquely correct — both do/while and while drop
  to 174 insns and +1 score on every base. Every one of the 96 inner-block positions
  that is not last is worse than the corresponding last position (best non-baseline
  order ABDCI = 7 vs ABCDI = 5); no non-final position reaches the top 24. Moving the
  `a2 = 0;` initialiser to the inner-loop boundary is byte-neutral on F/A/C, +2 on B.
  Both controls reproduce s12 exactly (ABCDI unflipped = 5, CABDI flipped = 12).
- verdict: KILLED. There is no loop-shape degree of freedom left in this function:
  the two loops that admit alternative spellings are byte-neutral under all of them,
  and the third is already at its unique optimum.

### H13.2 — Class C's cost comes from the `(s32)` cast / integer-domain address arithmetic, not from the operand order itself
- statement: every flipped spelling requires casting `D_800A36A0` to `s32`, so the
  24-point collateral cost might be an artefact of leaving the pointer domain
  (changed CSE of the address, changed `memory_address` handling) rather than of the
  operand order that the flip was introduced to change.
- mechanism: GCC 2.7.2 lowers `ptr + int` through pointer arithmetic and `int + int`
  through plain PLUS_EXPR; the two take different paths in expand and can CSE
  differently against the outer loop's other integer-domain address expressions
  (group A already spells its address `(t0 * 2) + (s32)base`).
- probe: 17 spellings of the inner-block address on the floor-5 base
  (tmp/grind/func_800770B8/s13/addr.log). The decisive one:
  `(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` — fully integer-domain, pointer still
  named FIRST.
- result: that spelling measures 5 / 175, byte-identical to the current pointer-domain
  form. The cast is free; only the order costs. Corroborated from the other side:
  `(s16 *)((t0 * 10) + D_800A36A0 + 0x6A)` (addend first, NO cast) also measures 5,
  because the C front end canonicalises the pointer back to first position — so the
  cast is not merely harmless, it is the ONLY way to express the flip at all.
- verdict: KILLED. Class C is a pure source-operand-order residual. This also
  forecloses the whole "find a cast-free / domain-preserving spelling of the flip"
  family: there isn't one.

### H13.3 — Both inner-loop pointers must be flipped for the operand order to change
- statement: implicit in every prior session's spelling probes, which always flipped
  `p_6a` and `p_7e` together.
- mechanism: cse.c gives the two addresses a common subexpression (`D_800A36A0 + t0*10`);
  whichever expression is emitted first fixes the `(plus A B)` operand order for the
  shared `addu`, and the other is derived from it by a constant `addiu`.
- probe: flip `p_6a` only (score 29) vs flip `p_7e` only (score 5), on the floor-5 base.
- result: flipping `p_7e` alone is completely inert — byte-identical to the unflipped
  form. Flipping `p_6a` alone is byte-identical to flipping both. The target's rows
  63/64 confirm the direction: `addiu $a3, $v1, 0x6A` precedes `addiu $a1, $v1, 0x7E`.
- verdict: KILLED (the conjunction is false). Practical consequence for the next
  session: class-C probes only need to vary `p_6a`'s spelling — `p_7e` is a derived
  address and carries no degrees of freedom. This halves the class-C search space.

### H13.4 — The inner block can re-use the outer loop's `base` copy instead of re-reading D_800A36A0
- statement: the inner block currently re-reads the global (`D_800A36A0 + ...`) while a
  live `u8 *base = D_800A36A0;` copy is in scope; using `base` would shorten the
  dependence chain and might re-seat the contested `addu`.
- mechanism: eliminating the second `lw ...($gp)` removes a definition from the block,
  changing local-alloc's quantity spans for the contested chain.
- probe: `(s16 *)(base + (t0 * 10) + 0x6A)` and `(s16 *)((t0 * 10) + (s32)base + 0x6A)`.
- result: both measure 174 build_insns / score 41. The second `lw` disappears — but the
  target HAS it (row 60 `lw $v0, %gp_rel(D_800A36A0)($gp)`), so any form that lets CSE
  reuse the outer copy is disqualified on instruction count alone.
- verdict: KILLED, and load-bearing: the inner block MUST re-read the global. Do not
  propose `base`-routed inner-block addresses again.

### H13.5 (NEW, carried to the frontier) — class C is not a register-allocation residual at all on this form
- statement: s10's QTYDBG work and s12's frontier both typed the contested `addu` as a
  local-alloc seat question and proposed an `inverse.py local` run restricted to
  `qty3 -> $v1`. On the floor-5 form the seats are ALREADY correct.
- mechanism / evidence: reading the floor-5 build against the target instruction for
  instruction (tmp/grind/func_800770B8/s13/ours.py) shows rows 37-61 identical, including
  the target's non-obvious `t0*4 -> +t0 -> <<1` reuse chain for `t0*10` (rows 42, 56, 61),
  and rows 60-61 already place the reloaded global in `$v0` and the shift chain in `$v1`
  — the target's own seats. The residual is a single `addu` whose two operands are the
  right values in the right registers in the wrong order. Under the flip, the ORDER
  becomes right and the two hard registers swap, so the printed insn is unchanged and
  24 further rows (38-59) regress because GCC hoists the `lw` ahead of the `sll` and
  delays the `sll ...,2` from row 42 to row 50.
- verdict: OPEN — this re-types the s12 frontier's second item. The allocator diff it
  proposes (QTYDBG on flipped-ABCD vs flipped-CABD) is still worth running, but the
  question it should answer is now "what makes the loop head's EMISSION order survive
  the flip", not "what re-seats the contested quantity".

## [s13] A loop-SHAPE change (not statement order, not spelling) makes the class-C plus-operand flip free, closing the gap between the two basins.
- mechanism: The inner p_6a/p_7e loop, the outer t0 loop and the third a2<0xA loop each fix a NOTE_INSN_LOOP_BEG/END placement and an induction-variable expansion, so changing a loop's shape changes loop.c's biv/giv discovery and hence the RTL emission order that the flip perturbs. s12 had eliminated every other statement-level degree of freedom (group order 24/24, group spelling, hoists, wrap positions) and left loop shape as the last one.
- probe: 315 builds across three sweeps. (a) 48 builds: the inner loop written as do-while / for / while / pointer-walking / split-in-two / stores-swapped, crossed with the `a2 = 0;` initialiser at the top of the outer body vs at the inner-loop boundary, on four bases (floor-5 unflipped ABCD, flipped ABCD=29, flipped BACD=14, flipped CABD=12) - tmp/grind/func_800770B8/s13/sh.log. (b) 240 builds: the inner-loop block promoted to a fifth permutable element beside the four store groups, all 5! = 120 orders x {flipped, unflipped} - tmp/grind/func_800770B8/s13/p5.log. (c) 27 builds: outer loop as do-while / for / while crossed with the third loop as for / do-while / while, on three bases - tmp/grind/func_800770B8/s13/lp.log.
- result: for / while / do-while are BYTE-IDENTICAL for the inner loop on all four bases and for the outer loop on all three bases - both axes carry zero information. Pointer-walking the inner loop elides two instructions the target has (173 vs 175 build_insns); splitting it into two loops costs eleven (186); swapping its two stores costs a uniform +4. The third loop's `for` is uniquely correct: both do-while and while drop to 174 insns and cost +1 on every base (F 5->6, A 29->30, C 12->13). In the 240-build position sweep every one of the 96 orders that does not put the inner block last is worse than the corresponding last-position order - the best non-baseline order is ABDCI = 7 against the ABCDI = 5 baseline, and no non-final position appears in the top 24. Moving `a2 = 0;` to the inner-loop boundary is byte-neutral on three of the four bases (+2 on BACD). Both controls reproduce s12 to the point (ABCDI unflipped = 5, CABDI flipped = 12), validating the harness.
- verdict: KILLED

## [s13] The flip's 24-point collateral cost comes from the (s32) cast / leaving the pointer domain, not from the operand order itself - so a domain-preserving spelling of the flip would be free.
- mechanism: GCC 2.7.2 lowers `ptr + int` through pointer arithmetic and `int + int` through plain PLUS_EXPR; the two take different paths in expand and can CSE differently against the outer loop's other integer-domain address expressions (group A already spells its address `(t0 * 2) + (s32)base`).
- probe: 17 spellings of the inner-block address on the floor-5 base (tmp/grind/func_800770B8/s13/addr.log). The decisive pair: `(s16 *)((s32)D_800A36A0 + (t0 * 10) + 0x6A)` - fully integer-domain, pointer still named FIRST - and `(s16 *)((t0 * 10) + D_800A36A0 + 0x6A)` - addend first, no cast.
- result: The fully-integer-domain, pointer-first form measures 5 / 175, byte-identical to the current pointer-domain form: the cast is free. The cast-free addend-first form ALSO measures 5, because the C front end canonicalises the pointer back to the first position - so the cast is not merely harmless, it is the only way to express the flip at all. Every flipped spelling that does express it (7 of the 17) collapses onto exactly 29 / 175.
- verdict: KILLED

## [s13] Both inner-loop pointers must be flipped for the contested addu's operand order to change (assumed by every prior session's class-C spelling probe, which always flipped p_6a and p_7e together).
- mechanism: cse.c gives the two addresses a common subexpression (D_800A36A0 + t0*10); whichever expression is emitted first fixes the (plus A B) operand order for the shared addu, and the other is derived from it by a constant addiu.
- probe: Flip p_6a only, and flip p_7e only, on the floor-5 base (s13/addr V02 and V03).
- result: Flipping p_7e alone is completely inert - byte-identical to the unflipped form at 5. Flipping p_6a alone is byte-identical to flipping both, at 29. The target's rows 63/64 confirm the direction: `addiu $a3, $v1, 0x6A` precedes `addiu $a1, $v1, 0x7E`.
- verdict: KILLED

## [s13] The inner block can re-use the outer loop's live `u8 *base = D_800A36A0;` copy instead of re-reading the global, shortening the dependence chain and re-seating the contested addu.
- mechanism: Eliminating the second `lw ...($gp)` removes a definition from the block, changing local-alloc's quantity spans for the contested chain.
- probe: `(s16 *)(base + (t0 * 10) + 0x6A)` and `(s16 *)((t0 * 10) + (s32)base + 0x6A)` on the floor-5 base.
- result: Both measure 174 build_insns / score 41. The second lw disappears - but the target HAS it (row 60 `lw $v0, %gp_rel(D_800A36A0)($gp)`), so any form that lets CSE reuse the outer copy is disqualified on instruction count alone.
- verdict: KILLED

## [s13] Class C is a local-allocation seat question (s10's QTYDBG priority target, carried by s10/s11/s12).
- mechanism: s10 modelled the contested quantity's span/refs/birth and derived a closed-form priority target for getting qty3 into $v1; s12's frontier proposed diffing QTYDBG between flipped-ABCD and flipped-CABD to read off which allocator atom moves.
- probe: Instruction-for-instruction read-out of the floor-5 build against the target (tmp/grind/func_800770B8/s13/ours.py), plus a posdiff of flipped-ABCD.
- result: On the floor-5 form the seats are ALREADY correct. Rows 37-61 are identical to the target row for row, including the target's non-obvious construction of t0*10 - it computes t0*4 for the 0x40/0x42 store group (row 42 `sll $v1,$a1,2`), adds t0 (row 56 `addu $v1,$v1,$a1` = t0*5), then shifts (row 61 `sll $v1,$v1,1`) - which our build reproduces in the target's own registers. Rows 60-61 already place the reloaded D_800A36A0 in $v0 and the shift chain in $v1. The residual is one token: ours `addu $2,$2,$3` vs target `addu $v1,$v1,$v0`, both `addu rd,rs,rt` with rd==rs, so the printed order IS the RTL (plus A B) order, which is the source order (GCC 2.7.2's fold() only commutes to move a CONSTANT second - it has no complexity-based swap). Under the flip the order becomes right but the two hard registers swap, so the printed insn is unchanged and 24 further rows regress.
- verdict: KILLED

## [s14] synthesis — 2026-09-01 (floor 5 -> 4)

Modality: synthesis. Merged the whole ledger (s1-s13), spent the s13 frontier's
item 1, and reset the frontier below. Four hypotheses measured, three settled.

## [s14] Under the p_6a flip, the loop head's emission order is restored by changing what the loop head COMPUTES (an additional early consumer of t0*4), not by re-spelling the inner block's address. (s13 frontier item 1.)
- mechanism: s13 proved the flip is the only way to reach the target's plus-operand
  order and that its 24-point collateral is entirely rows 38-59, where the flipped
  build hoists the D_800A36A0 load ahead of `sll $a1,1` and postpones `sll $a1,2`
  from row 42 to row 50. If the t0*4 shift is demanded EARLY under the flip, the
  loop head's emission order should return to the target's.
- probe: 11 perturbations of the loop head's first-demand order x 3 bases (floor-5
  candidate, flipped-ABCD `s12/perm/Q00.c`, flipped-CABD `s12/perm/Q12.c`) = 33
  builds; `tmp/grind/func_800770B8/s14/gen.py`, `s14/sweep.log`.
- result: CONFIRMED, and stronger than stated. Group A's index written
  `(t0 * 4) >> 1` on the flipped-ABCD body measures **4 / 175 insns** (from 29),
  a new floor. The positional diff shows the contested addition now emits
  `addu $v1,$v1,$v0` with the target's own seats and rows 43-64 matching the target
  row for row: **residual class C is CLOSED**. All ten other perturbations
  (group C integer-domain, group A pointer-domain, group B one-step, `t0 << 2`,
  group D hoisted, the 0x5C/0x60 pair hoisted two ways, the inner index built from
  t0*4 explicitly, groups B+C both integer-domain) are neutral or worse on every
  base; the pair hoists are disqualified on insn count (173 / 174).
- verdict: CONFIRMED

## [s14] The class-C fix is available without its `sra` if the t0*4 pseudo is given extra REFERENCES rather than an extra dependence (a reg_n_refs effect, not a dependence-graph effect).
- mechanism: if what reversed the two shifts' birth order under the flip were
  local-alloc's reference counting, then spelling group C's (and group B's) index
  inline at each store — two references to t0*4 instead of one through the shared
  `ptr` local, with no new instruction — should reproduce the effect and leave the
  target's two independent `sll`s intact, closing class D as well as class C.
- probe: 8 builds on flipped-ABCD; `s14/gen4.py`, `s14/sweep4.log`.
- result: KILLED. Group C inline = 25, group B inline = 35/177, both = 12/177,
  group A inline = 29, the 0x5C/0x60 pair derived from t0*4 = 39/176, group D
  derived from t0*4 = 7/176. Nothing reaches 4, and every variant that adds a real
  second derivation costs an instruction (176/177 against the target's 175). The
  effect is the DEPENDENCE EDGE t0*2 <- t0*4, not the reference count.
- verdict: KILLED

## [s14] The dependence can be spelled in the other direction (t0*4 derived from t0*2), which would let the target's `sll $v0,$a1,1` stay first and close class D at the same time.
- mechanism: the target computes t0*2 first (row 40) and t0*4 second (row 42). If
  the sharing that closes class C is symmetric, writing group C as
  `base + ((t0 * 2) * 2)` should give the same seat fix with the shifts in the
  target's order.
- probe: 13 builds on flipped-ABCD; `s14/gen2.py`, `s14/sweep2.log`.
- result: KILLED, and the asymmetry is now a measured fact. `(t0*2)*2` = 29 and
  `(t0*2)<<1` = 29 — i.e. byte-identical to the unperturbed flipped body, no effect
  whatsoever. Only t0*2-derived-from-t0*4 works. Also settled in the same sweep:
  `(t0 * 4) / 2` measures 29 because GCC 2.7.2's `fold()` collapses the exact
  division back to `t0 * 2` before RTL (the mult/shift identity is NOT collapsed);
  a named `s32 i4 = t0*4;` consumed only by group C measures 29 while the same
  local consumed by group A as `i4 >> 1` measures 4, so naming is not the lever;
  `(u32)(t0*4) >> 1` (srl) also measures 4.
- verdict: KILLED

## [s14] Class B (rows 35-36) may be re-openable on the floor-4/floor-5 chassis, since s7/s8/s9 foreclosed it on the floor-9 chassis.
- mechanism: the target's 0x30/0x34 stores go through the raw `func_8006E49C`
  result pseudo ($v0); ours go through the `p_old` copy ($s1). s7/s8 attributed the
  collapse to cse pass 2 + flow.c deleting the copy, but all of that was measured
  two chassis ago.
- probe: 4 fresh spellings of the whole post-call block on the floor-4 body;
  `s14/gen3.py`, `s14/sweep3.log`.
- result: KILLED (re-confirmed). Every spelling that actually reaches the raw
  result pseudo — fresh `u8 *nb` local with the stores first (24), the result
  assigned straight into `D_800A36A0` (22), fresh local with the global assigned
  first (22) — collapses the function to **170 instructions, five fewer than the
  target's 175**, exactly the s7 failure mode. The only spelling that preserves 175
  routes all four stores through `(u8 *)p_old` and is byte-worse (6). Class B is
  foreclosed on the current chassis, not just the old one.
- verdict: KILLED

## [s14] A second `do { } while (0);` recovers a row on the floor-4 chassis.
- mechanism: s12 found a second wrap worth 2 points in the flipped-CABD basin
  (12 -> 10 at positions W039/W040). The floor-4 body is a different basin
  (flipped-ABCD + the t0*4 dependence), so the sweep had to be re-run against it.
- probe: 79-position second-wrap sweep on the floor-4 body; `s14/wrap.log`.
- result: KILLED. Minimum 4, reached at 34 of the 79 positions, all byte-identical
  to the no-second-wrap build; nothing below 4. The floor-4 form carries exactly
  one wrap (the s11 prologue fence), and s12's "the CABD basin needs a second wrap"
  note no longer applies to the live basin.
- verdict: KILLED

## Live frontier (for s15) — reset by the s14 synthesis

1. **Class D is a two-instruction gap with a known cause, and the next lever is a
   spelling of the t0*2 <- t0*4 dependence that emits `sll` rather than `sra`, or a
   C construct that supplies the same dependence edge for free.**
   - mechanism: at floor 4 the loop head occupies exactly the target's three slots
     (`shift / lw / shift`) with the two quantities in the target's own registers by
     role ($3 = t0*4, $2 = t0*2). The only defects are that our first shift is the
     *4 and our second is an `sra` deriving t0*2 from it, where the target emits
     *2 first and *4 second, both directly from $a1. s14 measured that the
     dependence edge (not the reference count, not a named local, not the division
     identity) is what re-seats the contested plus — so the question is now narrow
     and mechanical: what else creates an RTL dependence from the t0*4 pseudo to the
     t0*2 pseudo?
   - next probe: (a) run `pwsh tools/grinder/dump.ps1 func_800770B8` on the floor-4
     body and read `.combine` / `.lreg` for the two shift pseudos — s14 never
     attributed WHICH pass reverses their birth order, and the dump names it in one
     read; then check whether the `sra` is a combine artifact that a different
     source shape would emit as `sll`. (b) Measure address spellings in which group
     A's five stores index a *half-word* object derived from group C's pointer, e.g.
     `s16 *rowC = (s16 *)(base + (t0 * 4));` with group A written as offsets from a
     narrowed re-cast of `rowC` — a dependence edge that is arithmetically forced
     rather than an identity, and therefore not a first-reach classification problem.
     (c) Measure whether a *struct/array* type over `base` whose element size is 4
     produces the same edge naturally (s10 killed a whole-block struct rewrite at 178
     insns, but never a type applied only to groups A and C).

2. **The `(t0 * 4) >> 1` token needs a ruling before ANY floor-4 form can be
   submitted, and the answer determines whether the honest floor is 4 or 5.**
   - mechanism: the token is an arithmetic identity (it computes `t0 * 2`) with no
     semantic purpose (T1), which no human would write from the specification (T2),
     whose justification is a named GCC mechanism (T3). The frozen family list
     sanctions "opaque arithmetic variables" but the forbidden catalog separately
     lists "`s32 one = 1;` opaque variable to defeat single-bit transform"; the two
     entries are in tension for this exact shape, and the non-extension clause
     forbids generalising from either. This is a first reach.
   - next probe: emit a `ruling-request` (s14 does). If it is ruled a cheat, the
     honest floor stays 5 and frontier item 1 must find a *forced* dependence
     (option (b)/(c) above) rather than an identity. If it is ruled sanctioned, the
     form still needs class D and class B closed before it is byte-complete, so the
     ruling unblocks a submission path but does not by itself finish the function.

3. **The do-while(0) scoping conflict is still unresolved and still gates the
   prologue fence (carried unchanged from s11/s12/s13).**
   - mechanism: `.claude/rules/do-while-zero-exception.md` (owner ruling 2026-07-06)
     abolishes the reorg.c-only scoping that `.claude/rules/no-new-park-categories.md`
     :256-271 and the grind role-prompt's frozen-family table still assert. The
     dedicated rule is the later document and the stale entry designates it as the
     authority, but a submission would be the first BB2 use of a wrap for a sched2
     effect.
   - next probe: unchanged — resolve by ruling, or have the operator update
     no-new-park-categories.md:256-271. s14 adds one datum: on the floor-4 chassis a
     SECOND wrap is inert (79 positions, min 4), so the wrap count in any submitted
     form will be exactly one.
