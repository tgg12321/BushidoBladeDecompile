# hypotheses — func_8001979C

## KILLED (session 1)

- **H-B: the walking store pointer's C type matters.** Spelling the loop-1
  walker as `s16 *dp` with `dp[5]` / `dp++` instead of `u32 dst` with
  `*(s16 *)(dst + 0xA)` / `dst += 2` will change whether GCC folds the `+0xA`
  displacement into the induction variable's initial value.
  *Mechanism:* different `mult_val` scaling in `loop.c`'s giv records for
  pointer vs integer arithmetic.
  *Probe:* probe B, `sandbox --disable all`.
  *Result:* score 24, build_insns 75 — output identical to the integer
  spelling, instruction for instruction. **KILLED.** Do not re-spell the walker
  hoping for a codegen change; pick the spelling that reads best.

- **H-E: emitting the `0xC - bits_left` subtraction before the
  `0x20 - bits_left` subtraction in source order will make LICM hoist the
  constants in target's order (`li $t4,0xC` before `li $t2,0x20`) and fix D4.**
  *Mechanism:* `loop.c` hoists loop-invariant constant loads into the preheader
  in first-use order.
  *Probe:* probe E (on top of probe D), `sandbox --disable all`.
  *Result:* 33 → 43. The `li` order did not flip and the allocation degraded
  further. **KILLED as a standalone lever.** D4 is not a source-statement-order
  effect; it follows the allocation, so it should be re-tested only after D1/D3
  are settled, never chased on its own.

## CONFIRMED (session 1)

- **H-C: D1's two missing `addu $a3,$v0,$zero` copies are produced by routing
  the new `bits_left` through a named intermediate**
  (`val = 0x20 - needed; bits_left = val;`).
  *Mechanism:* the intermediate is a second pseudo that GCC 2.7.2's local
  allocator does not tie to `bits_left`'s allocno, so the `subu` writes the
  intermediate's register and a reg-reg copy materialises the assignment.
  *Probes:* C (loop 1 alone) and D (both loops).
  *Result:* **CONFIRMED.** The copy appears in exactly target's position, and
  with both loops converted `build_insns == target_insns == 77` for the first
  time. Cost: +1 allocno of loop pressure, which cascades the whole register
  assignment (score 24 → 33). Form banked as
  `variant_named_intermediate_77insn.c`.

- **H-A: D3 (the folded `+0xA` / `+0x8E` displacement) is `loop.c` biv
  elimination driven by having *two* identical `DEST_ADDR` givs, one per
  if-arm.**
  *Mechanism:* `combine_givs` merges the two address givs, the merged giv gets
  reduced, `bl->all_reduced` goes true, `maybe_eliminate_biv` deletes the biv
  and folds the constant into its initial value.
  *Probe:* A — collapse the two per-arm stores into a single store at the
  if/else join.
  *Result:* **CONFIRMED.** With one address giv the preheader becomes
  `move $t1,$t3` and the store becomes `sh $v0,10($t1)` — target's exact form.
  Caveat that makes this a frontier item rather than a solution: target still
  contains *two* `sh` instructions per loop, so the single-store form cannot be
  the final shape.

## FRONTIER (ranked)

1. **F1 — two stores, one address giv.** Find the C shape that keeps a store in
   each if-arm (target has two `sh`) while presenting `loop.c` with a single
   reducible address giv, or that makes the address giv not worth reducing.
   *Mechanism to attack:* `combine_givs` merging + `maybe_eliminate_biv` in
   `tools/gcc-2.7.2/loop.c`.
   *Concrete next probes:* (a) hoist the address into a loop-local pointer
   computed once per iteration (`s16 *q = &dp[5];`) and store through `*q` in
   both arms — one giv, two stores; (b) read `tools/gcc-2.7.2/loop.c`
   `strength_reduce` / `combine_givs` / `maybe_eliminate_biv` and identify the
   exact predicate that flipped between probe A and the floor, then target it;
   (c) check whether making the counter `i` and the walker the *same* induction
   variable (drop `i`, test the walker against an end pointer) is compatible
   with target's `slti $v0,$t0,0x3F` — it is not, so (c) is a diagnostic only.

2. **F2 — recover the floor's register assignment on top of the 77-insn
   shape.** The named-intermediate form is one allocno too heavy; the specific
   casualty is `hi`, which must stay in the caller-save temp it shares with the
   `0x20 - bits_left` sub-expression (target: `srlv $v1,$a2,$v1`).
   *Mechanism:* GCC 2.7.2 `local-alloc`/`global-alloc` allocno priority
   (`reg_n_refs` / live length) — one extra live pseudo pushes `hi` out of the
   `$v0`/`$v1` pair into `$t1` and cascades every `$t` register up one.
   *Concrete next probes:* (a) shorten the intermediate's live range by writing
   `val` immediately before `bits_left = val;` and immediately after the
   `subu`, with the `cur = *arg1++;` refill moved above it, so the intermediate
   is live across fewer insns; (b) run the instrumented cc1 `-da` `.greg` dump
   on both forms and diff `;; Register dispositions:` + the conflict lists to
   see precisely which conflict evicts `hi` (per
   `.claude/rules/register-alloc-pure-c.md`); note
   `memory/project/instrumented-cc1-location.md` — the instrumented cc1 is
   `tools/gcc-2.7.2/cc1`, not `build/cc1`.

3. **F3 — D2's `$v0`/`$v1` mirror.** At the floor the whole shift/or chain is a
   clean mirror image of target (`hi` chain in `$v0` instead of `$v1`), and the
   final `-2` fill loop is mirrored too (`li $v0,-2` + `addiu $v1,$t3,0x348`
   vs target's `li $v1,-2` + `addiu $v0,$t3,0x348`). That the *third* loop is
   also mirrored, despite being structurally identical to target otherwise,
   says the flip is decided function-wide by allocno ordering rather than by
   anything local to the bit loops.
   *Concrete next probes:* (a) reorder the local *declarations* (a
   SOTN-sanctioned named-intermediate-declaration-order lever) and re-measure —
   cheapest possible test of the allocno-ordering theory; (b) change which
   operand of the `|` is written first. **(b) must not be attempted before
   reading `.claude/rules/or-tree-shape-shift.md`**, which forbids
   parenthesisation-axis mutations in associative/commutative expressions; if
   that rule covers bare operand-order swaps too, drop (b) and say so here.

## [s1] D1's two missing 'addu $a3,$v0,$zero' copies are produced by routing the new bits_left through a named intermediate (val = 0x20 - needed; bits_left = val;) instead of assigning bits_left directly.
- mechanism: GCC 2.7.2 local-alloc does not tie the intermediate's allocno to bits_left's, so the subu writes the intermediate's register and a reg-reg copy materialises the assignment.
- probe: Probe C (loop 1 only) then probe D (both loops); sandbox --disable all + objdump of tmp/sandbox/func_8001979C/code6cac.o.
- result: Probe C: build_insns 75 -> 76, the new insn is 'subu $v0,$t3,$a0' + 'move $a3,$v0' in exactly target's position. Probe D: build_insns == target_insns == 77 for the first time, score 33. Cost is +1 allocno of loop pressure which cascades the whole register assignment (base $t3->$t4, consts $t2/$t4->$t3/$t5, walker $t1->$t2) and evicts hi from the $v0/$v1 pair into $t1.
- verdict: CONFIRMED

## [s1] D3 (target's 'addu $t1,$t3,$zero' + 'sh $x,0xA($t1)' vs our 'addiu $t1,$t3,10' + 'sh $x,0($t1)') is loop.c biv elimination, triggered by having two identical DEST_ADDR givs, one store per if-arm.
- mechanism: combine_givs merges the two identical address givs; the merged giv is reduced; bl->all_reduced goes true; maybe_eliminate_biv deletes the biv and folds the +0xA constant into its initial value.
- probe: Probe A: collapse loop 1's two per-arm stores into a single store at the if/else join through a local, then sandbox + objdump.
- result: With one address giv the preheader becomes 'move $t1,$t3' and the store becomes 'sh $v0,10($t1)' -- target's exact form (score 29, build_insns 76, the +1 being a sign-extension artifact of the s32 holding local). Caveat: target still has two sh per loop, so single-store cannot be the final shape; the job is two stores with one giv.
- verdict: CONFIRMED

## [s1] The C type of the walking store pointer matters: spelling loop 1's walker as 's16 *dp' with dp[5]/dp++ instead of 'u32 dst' with *(s16*)(dst+0xA)/dst+=2 will change whether GCC folds the +0xA displacement into the induction variable's initial value.
- mechanism: different mult_val scaling in loop.c's giv records for pointer vs integer arithmetic.
- probe: Probe B; sandbox --disable all.
- result: Score 24, build_insns 75 -- output identical to the integer spelling instruction for instruction. The two spellings are codegen-neutral here.
- verdict: KILLED

## [s1] Emitting the '0xC - bits_left' subtraction before the '0x20 - bits_left' subtraction in source order will make LICM hoist the preheader constants in target's order (li $t4,0xC before li $t2,0x20) and close D4.
- mechanism: loop.c hoists loop-invariant constant loads into the preheader in first-use order.
- probe: Probe E, applied on top of probe D (semantically neutral -- both reads use the old bits_left); sandbox --disable all.
- result: Score 33 -> 43. The li order did not flip and the allocation degraded further. D4 is not a source-statement-order effect; it follows the allocation and must not be chased on its own.
- verdict: KILLED

## KILLED (session 2)

- **H2-A: a single walking-pointer variable shared by both bit loops can be
  made to sit below the loop counter in allocno priority.**
  *Mechanism:* global.c `allocno_compare` priority
  `floor_log2(nrefs)*nrefs/livelen`.
  *Probe:* P5 (one `dst`, duplicated increments, third loop given its own
  pointer) plus the ALLOCDBG numbers.
  *Result:* nrefs 26 / livelen 54 / pri 19259 against a counter at 13548.
  Falling below the counter needs livelen > 76 and the longest live range in
  the entire function is 69. **KILLED** — the walker MUST be split per loop
  once the increment is duplicated.

- **H2-B: `hi` should be given its own anonymous temp for the
  `0x20 - bits_left` shift amount rather than reusing `val`.**
  *Mechanism:* fewer live pseudos in the arm.
  *Probe:* P8 (P7 with `hi = cur >> (0x20 - bits_left);`).
  *Result:* 20 -> 31. The separate temp pushes `hi` out of the $v0/$v1 pair.
  **KILLED.** Keep the `val` reuse.

- **H2-C (re-kill of a session-1 premise, old F3): the third `-2` fill loop is
  $v0/$v1-mirrored, therefore the mirror is decided function-wide.**
  *Probe:* direct disassembly of the floor object.
  *Result:* the third loop is byte-identical to target at the floor.
  **KILLED as a premise** — D2 is local to the two bit loops, so a
  function-wide allocno-ordering explanation is not required.

## CONFIRMED (session 2)

- **H2-D: giving the walking-pointer biv two increments (one per if-arm)
  blocks strength reduction of the store-address giv and restores target's
  `move $t1,$t3` + `sh $x,0xA($t1)` shape while keeping both stores.**
  *Mechanism:* loop.c:3804 `benefit -= add_cost * bl->biv_count`; with
  biv_count 2 the combined benefit of the two pooled DEST_ADDR givs reaches 0,
  loop.c:3823 sets `v->ignore`, `all_reduced` goes 0 and `maybe_eliminate_biv`
  (guarded at loop.c:4035) never runs.
  *Probes:* P1 (loop 1), P2 (both loops), disassembly of each.
  *Result:* **CONFIRMED.** D3 closed in both loops. This supersedes session-1
  frontier item F1 ("two stores, one address giv") — the answer was not one
  giv, it was two biv increments.

- **H2-E: with D1 + D3 closed, the whole remaining register cascade is
  allocno-priority arithmetic that per-loop walkers plus early walker
  initialisation resolve exactly.**
  *Mechanism:* global.c `allocno_compare`; the required order is
  bits_left > counter > walker, and each walker's priority must land in the
  window between the counter and the loop constants.
  *Probes:* P5 / P6 / P7 with ALLOCDBG.
  *Result:* **CONFIRMED.** P7 reproduces target's register assignment
  ($a3/$t0/$t1/$t2/$t3/$t4) with build_insns 77 == target_insns 77, score 20.
  This supersedes session-1 frontier item F2.

## FRONTIER (ranked, as of end of session 2)

1. **F1 — D2: make `hi` share the register of the `0x20 - bits_left` temp.**
   Target emits `subu $v1,$t2,$a3` then `srlv $v1,$a2,$v1`: the temp dies at
   the srlv and local-alloc ties `hi` to its quantity, so both are $v1, and
   `val` + `cur >> bits_left` then share $v0. In the current form `val` holds
   the shift amount but is re-assigned later in the same arm, so its quantity
   is still live at the srlv and cannot be tied to `hi`; we get the mirror
   (temp $v1, hi $v0, val $v1, `cur >> bits_left` $v1).
   *Next probes:* (a) a THIRD named local holding only `0x20 - bits_left`,
   never touched again in the arm — distinct from P8, which removed the
   variable entirely rather than giving it a short dedicated life;
   (b) read `tools/gcc-2.7.2/local-alloc.c` `combine_regs` / `block_alloc`
   and identify the precondition for tying a SET's dest to a dying source,
   then satisfy it in C; (c) the instrumented cc1 also prints
   `QTYDBG blk=.. ord=.. qty=.. reg1=.. birth=.. death=.. refs=.. got=..`
   under `BB2_QTY_DEBUG=1` — diff that between the current form and each
   variant to see which quantity grabs $v1 first.

2. **F2 — D4: preheader order, coupled to the walker-init placement.**
   Target's preheaders are `move $t0,zero ; li $t4,<width> ; li $t2,0x20 ;
   move $t1,$t3` — walker init LAST, width constant before 0x20. Our early
   walker init is load-bearing (it buys the livelen that keeps the walker
   below the counter), so D4 cannot simply be re-ordered: moving
   `dst2 = base;` back after `i = 0;` costs 1 livelen and flips the walker
   above the counter again (that is exactly P6, score 31).
   *Next probes:* (a) widen the priority window some other way so the inits
   can move late — the counter is nrefs 21 / livelen 60 / pri 14000 and each
   walker is nrefs 13, so anything that shortens the counter's live range
   without dropping it a floor_log2 bucket helps; (b) check whether the `li`
   order follows `move_movables` insertion order rather than source order,
   using the `.loop` dump already produced under
   `tmp/grind/func_8001979C/s2/dump/`. Session 1 killed the naive
   source-order swap (old H-E), so this must be attacked through the hoist
   mechanism, not by re-ordering statements.

3. **F3 — policy pre-clearance for the closing form (do this BEFORE a
   candidate-ready submission, not after).** `dst += 2` duplicated into both
   if-arms is a REAL statement duplicated into two control-flow arms: it has
   genuine semantic purpose (the pointer must advance on every path) and it is
   live in the emitted output, so it is not a dead-store family. The closing
   session should read `.claude/rules/duplicated-statement-into-arms.md`,
   decide whether the FAKE-annotation prerequisite applies to a statement that
   materialises in the output, and write the self-vet against that rule with a
   verbatim scope quote and a precedent citation. The per-loop walker split
   and the statement-order moves are ordinary C with no annotation question.

## [s2] Giving the walking-pointer biv TWO increments (one per if-arm) instead of one at the loop bottom blocks strength reduction of the store-address giv, so the biv survives and the +0xA / +0x8E stays a store displacement (target's D3 form) while both sh instructions are kept.
- mechanism: tools/gcc-2.7.2/loop.c:3804 computes benefit -= add_cost * bl->biv_count for each giv; combine_givs (loop.c:5500, combine_givs_p loop.c:5464) pools the two identical per-arm DEST_ADDR givs so their benefit is 2b. With biv_count 1 that leaves 2b - add_cost > 0 and the giv is reduced, which makes all_reduced 1 and lets maybe_eliminate_biv (guarded loop.c:4035) delete the biv and fold the constant into its initial value. With biv_count 2 the benefit is 2b - 2*add_cost == 0, the loop.c:3823 predicate fires, v->ignore is set, all_reduced goes 0 and the biv is never eliminated.
- probe: P1: move 'dst += 2' into both if-arms of loop 1 and remove it from the loop bottom; sandbox --disable all + objdump of tmp/sandbox/func_8001979C/code6cac.o. P2: same for loop 2.
- result: P1 score 45 (build_insns 75) but loop 1's preheader became 'move a3,t3' and the store 'sh v0,10(a3)' -- target's exact shape. P2 score 44, both loops in target's shape. The score rise is a pure register rotation, not lost structure.
- verdict: CONFIRMED

## [s2] With D1 and D3 closed, the entire remaining register cascade is global-alloc allocno-priority arithmetic, and it is fixed by splitting the walking pointer into one variable per loop and initialising each as early as its loop allows.
- mechanism: global.c allocno_compare orders allocnos by floor_log2(nrefs)*nrefs/livelen (in-loop refs weighted x2), and each allocno takes the first non-conflicting hard register in reg_alloc_order. Target's order is bits_left ($a3) > counter ($t0) > walker ($t1) > 0x20 ($t2) > base ($t3) > width ($t4). Duplicating the increment adds 4 weighted refs to the walker (9 -> 13), lifting it above the counter; splitting the walker per loop and lengthening each live range pushes it back into the window between the counter and the loop constants.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_ALLOC_DEBUG=1, script tmp/grind/func_8001979C/s2/alloc.sh) on the floor and on each variant, plus sandbox on P5 / P6 / P7.
- result: Floor: bits_left 17846 / counter 14482 / reduced-giv walkers 11739 -> target regs. P5 (one shared walker): walker 19259 vs counter 13548, walker steals $a3, score 40. P6 (per-loop walkers): walkers 14444 vs counter 13548, walkers steal $t0, score 31. P7 (per-loop walkers + early inits): 17313 / 14000 / 13928 / 12580 -> $a3/$t0/$t1 exactly as target, score 20, build_insns 77 == target_insns 77.
- verdict: CONFIRMED

## [s2] A single walking-pointer variable shared by both bit loops can still be made to sit below the loop counter in allocno priority once its increment is duplicated.
- mechanism: global.c allocno_compare priority floor_log2(nrefs)*nrefs/livelen.
- probe: P5 (one 'dst' across both bit loops, duplicated increments, third loop given its own pointer 'out'); ALLOCDBG + sandbox.
- result: nrefs 26, livelen 54, pri 19259 against a counter at 13548. Dropping below the counter would need livelen > 76; the longest live range anywhere in the function is 69, so it is unreachable. Score 40. The walker must be split per loop.
- verdict: KILLED

## [s2] Giving 'hi' its own anonymous temp for the 0x20 - bits_left shift amount (rather than reusing the named 'val') will keep the arm's pseudo count down and improve the allocation.
- mechanism: fewer live pseudos in the if-arm lowers local-alloc pressure.
- probe: P8: P7 with 'hi = cur >> (0x20 - bits_left);' replacing 'val = 0x20 - bits_left; hi = cur >> val;' in both loops; sandbox --disable all.
- result: 20 -> 31 with build_insns still 77. The separate temp pushes 'hi' out of the $v0/$v1 pair. The 'val' reuse is load-bearing.
- verdict: KILLED

## [s2] Session 1's premise that the third (-2 fill) loop is $v0/$v1-mirrored at the floor, which was the evidence for D2 being decided function-wide.
- mechanism: n/a - this was a factual claim about the floor's output.
- probe: Direct objdump of the floor object (tmp/grind/func_8001979C/s2/floor.txt).
- result: The floor emits 'li v1,-2 ; li t0,3 ; addiu v0,t3,840', byte-identical to target's 'addiu $v1,$zero,-0x2 ; addiu $t0,$zero,0x3 ; addiu $v0,$t3,0x348'. The third loop was never mirrored; D2 is confined to the two bit loops, so no function-wide allocno-ordering explanation is needed.
- verdict: KILLED
