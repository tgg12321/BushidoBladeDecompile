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

## KILLED (session 3)

- **H3-A: D2 is a `local-alloc.c` `combine_regs` tie — giving the
  `0x20 - bits_left` shift amount a short dedicated life (a third named local,
  distinct from session 2's anonymous temp P8) will let local-alloc tie `hi`
  to the dying shift-amount quantity and put both in `$v1`.**
  *Mechanism claimed by session 2:* `combine_regs`/`block_alloc` tie a SET's
  destination quantity to a source quantity that dies in the same insn.
  *Probes:* vA (third named local `amt` declared last), vA2 (declared first),
  vC (reuse `hi` itself as the carrier), vB (reuse the dead `out` walker),
  plus `BB2_QTY_DEBUG` tables for P7 and vC.
  *Result:* **KILLED, three ways.** (1) The third-local spelling scores **31**
  for both declaration orders — identical to P8's anonymous temp, so the
  "short dedicated life" idea costs 11 points and is not the shape.
  (2) The tie itself IS reachable (vC/vB produce `subu v0,t2,a3 ; srlv v0,a2,v0`
  — one register for both, target's shape) but lands in `$v0`, scoring 22.
  (3) Decisively: the QTYDBG (local-alloc) table for the two arm blocks is
  **byte-identical between P7 and vC**, so local-alloc is not where the
  `$v0`/`$v1` choice is made at all. It is made in **global.c**: `val`
  (pseudo 83, nrefs 19, pri 50666) is allocated before `hi` (pseudo 78,
  nrefs 8, pri 13333) and takes `$v1`. Session 2's F1 mechanism statement is
  superseded.

- **H3-B: lowering `val`'s global ref count — by giving the third loop its own
  `-2` holder instead of reusing `val` — will drop `val` below `hi` in
  `allocno_compare` and flip the `$v0`/`$v1` pair into target's orientation.**
  *Mechanism:* global.c `allocno_compare` priority
  `floor_log2(nrefs)*nrefs/livelen`; `val`'s 19 refs come from three distinct
  reuses (shift amount, D1 intermediate, `-2` holder).
  *Probe:* vL — `s32 neg2;` for the third loop, `val` untouched elsewhere.
  *Result:* **KILLED as spelled.** Score 22 and **build_insns drops to 75**:
  removing the `-2` use also removes the ref pressure that materialises D1's
  `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair, so the two instructions
  session 1 fought for disappear. D1 and D2 are coupled through the same
  `val` ref count. Any future attack on the ordering must ADD refs to `hi`
  rather than REMOVE refs from `val`.

- **H3-C (re-kill at the settled baseline): hoisting `needed = w - bits_left;`
  to the top of the arm fixes D4's preheader `li` order.**
  *Probe:* vI — session 1's probe E, re-run on the P7 form as the session-2
  ledger explicitly asked.
  *Result:* **45** (from 20). Re-killed, and far worse than at the old
  baseline. D4 is not reachable by moving the subtraction that first uses the
  width constant.

- **H3-D: statement-order changes around the refill (`cur = *arg1; arg1++;`)
  shift the arm's RTL order enough to change the `$v0`/`$v1` outcome.**
  *Probes:* vG (refill below `needed`), vG2 (refill just above the store),
  vH (vG + the `hi` tie), vF (tie + `val` carrying `cur >> bits_left`),
  vK (`val` carrying `hi << needed`).
  *Result:* 24 / 26 / 26 / 28 / 26 (vK also loses 2 insns, 75). **All KILLED.**
  Every re-association inside the arm is neutral-to-worse; P7's statement
  order is a local optimum for the arm. (vE — `val` additionally carrying
  `cur >> bits_left` — is exactly neutral at 20, so it is an available spare
  spelling but buys nothing.)

- **H3-E (policy, not measurement): swapping the operands of the
  `(hi << needed) | (cur >> bits_left)` OR to flip the emission order.**
  *Result:* **FORBIDDEN, never to be measured.**
  `.claude/rules/or-tree-shape-shift.md` classifies operand reordering /
  reparenthesisation of an associative+commutative `|` as a cheat-by-spelling
  (status FORBIDDEN 2026-06-06, two confirmed rejected instances). Session 1's
  frontier probe F3(b) is closed by policy.

## CONFIRMED (session 3)

- **H3-F: the `$v0`/`$v1` mirror (D2) is decided by global.c allocno priority
  between exactly two pseudos — `val` and `hi` — and the required flip is
  "`hi` must outrank `val`".**
  *Mechanism:* `global.c` `allocno_compare` orders by
  `floor_log2(nrefs)*nrefs/livelen`; the earlier-allocated of the two takes
  `$v1` (P7: `val` pri 50666 -> `$v1`, `hi` pri 13333 -> `$v0`; target needs
  the reverse). Local-alloc is provably not involved (identical QTYDBG tables
  across forms with different outcomes).
  *Probe:* `BB2_QTY_DEBUG=1 BB2_ALLOC_DEBUG=1` instrumented-cc1 dumps of P7 and
  vC (`tmp/grind/func_8001979C/s3/qty_p7.log`, `qty_vC.log`) plus the vL
  ref-count experiment.
  *Result:* **CONFIRMED.** This replaces session 2's F1 mechanism and gives the
  next session an exact numeric target.

- **H3-G: the duplicated `dst += 2` is byte-neutral in the emitted code.**
  *Mechanism:* cross-jump (`jump2`) re-merges the two identical arm tails;
  target itself carries a single `addiu $t1,$t1,0x2` in the `bnez` delay slot
  (`asm/funcs/func_8001979C.s` lines 45 and 67) and our build emits exactly one
  too (build_insns 77 == target_insns 77 with the duplication in source).
  *Result:* **CONFIRMED** — the byte-neutrality prerequisite of the
  `duplicated-statement-into-arms` family is satisfied and verified, not
  assumed.

## FRONTIER (ranked, as of end of session 3)

1. **F1 — D2 (14 of the 20 points): make `hi` outrank `val` in global.c
   `allocno_compare` WITHOUT removing the `val` refs that materialise D1.**
   Numeric target from the measured dumps: `val` = nrefs 19 / livelen 15 /
   pri 50666; `hi` = nrefs 8 / livelen 18 / pri 13333; `hi` must be allocated
   first.
   *Next probes:* (a) ADD refs to `hi` — e.g. spell the store as
   `hi = (hi << needed) | (cur >> bits_left); *(s16 *)(dst + 0xA) = (s16)hi;`
   so `hi` gains references while `val` is untouched; (b) SHORTEN `hi`'s
   livelen (it currently spans the refill); (c) re-home the third loop's `-2`
   into a DIFFERENT already-dead local (`needed` or `hi`) rather than a new
   one, since vL proved a brand-new holder breaks D1 while the goal is only to
   move refs off `val`; (d) run `qty.sh` on every variant and read the ALLOCDBG
   `pri` column — it is a far better gradient than the sandbox score because it
   shows how close a form came to flipping the order.

2. **F2 — D4 (6 of the 20 points): preheader order, still coupled to the
   walker-init placement.** Target: `addu t0,zero,zero ; addiu t4,zero,<w> ;
   addiu t2,zero,0x20 ; addu t1,t3,zero`. Both source-level levers are now
   dead (session 1 H-E and session 3 vI for the `li` order; session 2 P6 for
   the init placement).
   *Next probes:* (a) attack the window from the counter's side — the counter
   is nrefs 21 / livelen 60 / pri 14000 and each walker is nrefs 13; anything
   that lengthens the counter's live range without dropping it a `floor_log2`
   bucket widens the window and lets the walker init move late; (b) read
   `tools/gcc-2.7.2/loop.c` `move_movables` / `scan_loop` and determine whether
   the two hoisted `li` constants are emitted in movable-list order or its
   reverse, then derive the source order that produces target's order.

3. **F3 — policy pre-clearance for the closing form (one prerequisite now
   discharged).** `dst += 2` duplicated into both arms is **verified
   byte-neutral** (H3-G). What remains before any `candidate-ready`: read
   `.claude/rules/duplicated-statement-into-arms.md`, confirm the family's
   scope sentence covers a duplication whose observable effect is a
   biv-count / `reg_n_refs` priority change, and write the self-vet with a
   VERBATIM scope quote plus a `file:line` or commit-hash precedent. If the
   scope sentence does not cleanly cover it, emit `ruling-request` instead of
   a submission.

## [s3] D2 is a local-alloc.c combine_regs tie: giving the 0x20 - bits_left shift amount a short dedicated life (a THIRD named local, distinct from session 2's anonymous temp P8) lets local-alloc tie hi to the dying shift-amount quantity so both land in $v1.
- mechanism: local-alloc.c combine_regs / block_alloc tie a SET's destination quantity to a source quantity that dies in the same insn; qty_compare orders quantities by floor_log2(n_refs)*n_refs*size/(death-birth).
- probe: vA (third named local 'amt' declared last), vA2 (same declared first), vC (reuse 'hi' itself as the carrier), vB (reuse the otherwise-dead third-loop walker 'out'), each measured with sandbox --disable all; plus BB2_QTY_DEBUG=1 instrumented-cc1 dumps of the P7 form and of vC (tmp/grind/func_8001979C/s3/qty_p7.log, qty_vC.log).
- result: vA 31, vA2 31 (identical to P8's anonymous temp, so declaration order is irrelevant and a dedicated short-lived local costs 11 points). vC 22 and vB 22: the tie DOES occur - the disassembly becomes 'subu v0,t2,a3 ; srlv v0,a2,v0', one register for both, exactly target's shape - but the merged quantity lands in $v0 instead of $v1 and we lose the 'subu $v1,$t2,$a3' line P7 already matched. Decisive: the QTYDBG local-alloc table for both if-arm blocks is BYTE-IDENTICAL between P7 and vC (blk=2: ord0 pseudo101 refs8 got=2, ord1 pseudo98 refs4 got=3; blk=7 the mirror), so local-alloc is not where the $v0/$v1 choice is made.
- verdict: KILLED

## [s3] The $v0/$v1 mirror (D2) is decided in global.c allocno_compare between exactly two pseudos, 'val' and 'hi', and closing D2 requires making hi outrank val.
- mechanism: global.c allocno_compare priority floor_log2(nrefs)*nrefs/livelen; the earlier-allocated allocno takes $v1 here.
- probe: BB2_ALLOC_DEBUG=1 dumps of the P7 form and of vC, read alongside the disassembly of each.
- result: P7: pseudo 83 = val, ord=1, nrefs 19, livelen 15, pri 50666 -> hardreg 3 ($v1); pseudo 78 = hi, ord=7, nrefs 8, livelen 18, pri 13333 -> hardreg 2 ($v0). vC: pseudo 78 = hi (tie merged), ord=1, nrefs 16, livelen 20, pri 32000 -> $v0; pseudo 83 = val, ord=2, nrefs 11, livelen 11, pri 30000 -> $v1. Target needs hi in $v1 and val in $v0. val's nrefs is 19 because the one local is reused three ways (shift amount, the 0x20 - needed D1 intermediate, the third loop's -2 holder).
- verdict: CONFIRMED

## [s3] Lowering val's global ref count by giving the third loop its own -2 holder will drop val below hi in allocno_compare and flip the pair into target's orientation.
- mechanism: global.c allocno_compare priority floor_log2(nrefs)*nrefs/livelen; removing one of val's three reuses removes 3-4 references.
- probe: vL - declare 's32 neg2;' and use it for the third loop's -2 fill, leaving val's other two roles untouched; sandbox --disable all.
- result: Score 22 and build_insns falls from 77 to 75. Removing the -2 use also removes the ref pressure that materialises D1's 'subu $v0,$t2,$a0' + 'addu $a3,$v0,$zero' pair, so the two instructions session 1 fought for disappear. D1 and D2 are coupled through the same val ref count; a future attack must ADD references to hi rather than REMOVE them from val (or re-home the -2 into an already-dead existing local instead of a brand-new one).
- verdict: KILLED

## [s3] Hoisting 'needed = width - bits_left;' to the top of the if-arm fixes D4's preheader li order, now that D1 and D3 are settled (session 1 killed it at the old 33-point baseline; the session-2 ledger asked for a re-test at the settled baseline).
- mechanism: loop.c hoists loop-invariant constant loads into the preheader; the hypothesis was that first-use order of the width constant drives the emission order of 'li $t4,<w>' vs 'li $t2,0x20'.
- probe: vI - 'needed = 0xC - bits_left;' / 'needed = 2 - bits_left;' moved to the first statement of each arm, on top of the P7 form; sandbox --disable all.
- result: 20 -> 45. Re-killed at the settled baseline, and far worse than at the session-1 baseline. D4 is not reachable by moving the subtraction that first uses the width constant.
- verdict: KILLED

## [s3] Statement-order changes around the refill (cur = *arg1; arg1++;) or around which temp 'val' carries will shift the arm's RTL order enough to flip the $v0/$v1 outcome.
- mechanism: RTL emission order feeds both local-alloc quantity spans and global-alloc live lengths.
- probe: vG (refill moved below 'needed = w - bits_left'), vG2 (refill moved down to just above the store), vH (vG plus the hi tie), vF (hi tie plus val carrying 'cur >> bits_left'), vK (val carrying 'hi << needed'), vE (val additionally carrying 'cur >> bits_left'); all sandbox --disable all.
- result: vG 24, vG2 26, vH 26, vF 28, vK 26 with build_insns 75, vE exactly 20 (neutral). Every re-association inside the arm is neutral-to-worse; the P7 statement order is a local optimum for the arm.
- verdict: KILLED

## [s3] The duplicated 'dst += 2' in both if-arms is byte-neutral in the emitted code (the prerequisite the duplicated-statement-into-arms family asks for).
- mechanism: GCC's jump2 cross-jump pass re-merges the two identical arm tails into a single increment in the shared tail, which the delay-slot filler then puts in the bnez delay slot.
- probe: Read the target's loop bottoms in asm/funcs/func_8001979C.s and compare with our build's instruction count and disassembly.
- result: Target carries exactly ONE 'addiu $t1,$t1,0x2' per loop, in the bnez delay slot (asm/funcs/func_8001979C.s lines 45 and 67), and our build with the duplication in source emits exactly one too (build_insns 77 == target_insns 77). Byte-neutrality is verified, not assumed.
- verdict: CONFIRMED

## [s3] Swapping the operands of '(hi << needed) | (cur >> bits_left)' to flip the emission order (session 1 frontier probe F3(b)).
- mechanism: cc1's OR-tree shape drives INSN_PRIORITY ordering in sched.c.
- probe: NOT MEASURED - closed by policy after reading .claude/rules/or-tree-shape-shift.md.
- result: FORBIDDEN. That rule classifies reordering operands or reparenthesising an associative+commutative OR/AND expression as a cheat-by-spelling (status FORBIDDEN 2026-06-06, two confirmed rejected instances on func_8007CBB0 and func_8007C97C). The axis is permanently closed and must not be re-opened by a later session.
- verdict: KILLED

## KILLED (session 4)

- **H4-A: an opaque local holding the 0x20 constant changes the shift/subtract
  allocation in our favour** (permuter outputs 345-1 / 375-1 / 400-1 / 325-2).
  *Probe:* vA (holder feeding all four `0x20 - x` subtractions), vB (shift
  amount only), vI (the session-4 carrier line only), vJ (vI + a condition
  holder).
  *Result:* 21 / 31 / 31 / 35, and vB/vI/vJ also push build_insns to 78.
  **KILLED** - and note vA at 21 is worse than the 18 reached without it, so
  the constant-holder axis is dead at the current baseline in every spelling
  measured. (It is also the spelling closest to the forbidden `s32 one = 1;`
  opaque-variable family, so it should not be revived without a ruling.)

- **H4-B: hoisting the arm's `if` condition into an existing dead local
  (`needed` / `nd`) changes the branch/compare allocation** (permuter output
  300-1, minus its nonsense float cast).
  *Probe:* vG (`needed = bits_left < w; if (needed)`) and vH (same into `nd`).
  *Result:* vG 22, vH 18 (exactly neutral). **KILLED** - no gradient in either
  direction; the condition holder buys nothing.

- **H4-C: session 3's frontier F1(c) - re-homing one of `val`'s three roles
  into an already-dead EXISTING local will lower `val`'s ref count and flip
  the D2 ordering without the D1 loss that vL suffered.**
  *Mechanism:* global.c allocno_compare priority floor_log2(nrefs)*nrefs/livelen.
  *Probe:* the session-4 carrier form itself, read against ALLOCDBG.
  *Result:* **KILLED as a mechanism** (even though the form it produced lowered
  the floor for other reasons). Spelling the re-home as `dst2 = 0x20 - bits_left;
  val = dst2;` leaves `val` with all 19 references - the read-back IS a
  reference - so val's priority is unchanged at 50666 and `hi`'s at 13333. A
  re-home that actually removes refs from `val` is the vL case, which loses D1.
  F1(c) is therefore closed: the ordering must be attacked from `hi`'s side.

- **H4-D: the walker used as the carrier matters.**
  *Probe:* vE5 - carrier = the third-loop walker `out` instead of the other bit
  loop's walker.
  *Result:* 20 vs 18. **KILLED** - only the OTHER bit loop's walker produces
  the gain, consistent with the effect being live-range pressure inside the bit
  loops rather than anything about the carrier's identity.

## CONFIRMED (session 4)

- **H4-E: a permuter campaign on the cheat-invisible full-TU chassis finds
  allocation-pressure levers that hand structural search missed.**
  *Probe:* two campaigns (~13k iterations each), proposals filtered for
  semantics and policy, then re-measured on the engine gradient.
  *Result:* **CONFIRMED.** Floor 20 -> 18. The winning edit (per-loop carrier
  re-home + `nd` holder) is not in any shape sessions 1-3 tried, and its halves
  are individually neutral - a gradient-following hand search could not have
  reached it.

## FRONTIER (ranked, as of end of session 4)

1. **F1 - D2 (14 of the remaining 18 points): make `hi` outrank `val` in
   global.c allocno_compare.** Unchanged numeric target, now re-confirmed at the
   18-point baseline: `val` nrefs 19 / livelen 15 / pri 50666; `hi` nrefs 8 /
   livelen 18 / pri 13333. Session 4 closed the "remove refs from val" and
   "re-home val's roles" sub-branches for good (H4-C, and session 3's vL), so
   only the `hi` side is left.
   *Next probes:* (a) ADD refs to `hi` without touching `val` - e.g.
   `hi = (hi << needed) | (cur >> bits_left); *(s16 *)(dst + 0xA) = (s16)hi;`
   (this is the one F1(a) probe session 3 listed and session 4 did not reach);
   (b) SHORTEN `hi`'s livelen of 18 - it currently spans the `cur = *arg1;`
   refill, so anything that moves the refill out of `hi`'s span raises its
   priority; (c) note the arithmetic: `hi` needs pri > 50666, i.e. with
   floor_log2(8)=3 and nrefs 8 it would need livelen < 1 - so a pure livelen
   change cannot do it and `hi` MUST gain refs to cross a floor_log2 bucket
   (nrefs 16 -> floor_log2 4 -> pri 64000/livelen). That arithmetic is the
   concrete design constraint for every future D2 probe.

2. **F2 - D4 (4 of the remaining 18 points): our `move $t1,$t3` is emitted
   before the `sw`/`lw` preamble instead of last.** The `li` order half of D4
   closed as a side effect this session, so the residual is purely the walker
   init's position, which is still coupled to the priority window (moving the
   init late costs livelen and flips the walker above the counter - session 2's
   P6). Counter is nrefs 21 / livelen 60 / pri 14000; each walker is nrefs 13.
   *Next probes:* (a) widen the window from the counter's side (lengthen the
   counter's live range without dropping it a floor_log2 bucket); (b) read
   tools/gcc-2.7.2/loop.c move_movables / scan_loop for where the preheader
   insertion point is chosen relative to the loop's first insn.

3. **F3 - policy pre-clearance, now TWO constructs.** Before any
   candidate-ready: (i) the duplicated `dst += 2` (byte-neutrality proven in
   session 3, family scope quote + precedent still owed); (ii) NEW - the
   session-4 carrier reuse (`dst2 = 0x20 - bits_left; val = dst2;`), i.e.
   writing a walking pointer that is live LATER with an unrelated value purely
   for allocation pressure. It plausibly sits in the frozen "variable reuse for
   codegen control" family, but that is a claim; it must be checked against the
   rule's scope sentence with a file:line or commit precedent, and a
   ruling-request emitted if the scope does not cleanly cover it. Do not
   submit either construct on assumption.

## [s4] A permuter campaign on a cheat-invisible full-TU chassis will find allocation-pressure levers that sessions 1-3's hand structural search could not reach.
- mechanism: decomp-permuter's randomizer mutates statement placement, temp introduction and variable reuse at a granularity that produces individually-neutral edits whose COMBINATION shifts global.c allocno priorities; a hand search that follows the sandbox gradient one edit at a time cannot see such a combination because each half scores exactly neutral.
- probe: Built tmp/perm_979C (baseline chassis, perm base_score 410) and tmp/perm_979C_b (vE3 chassis, base_score 370) with tmp/grind/func_8001979C/s4/mk_workspace.sh; ~13k iterations each with -j 8; proposals filtered for semantics and policy with odiff.py, then re-measured on the engine gradient (sandbox --disable all).
- result: Baseline chassis best perm-score 305 (output-305-1) contained the winning edit; hand-minimised to vE3. Engine gradient: carrier chain in loop 1 alone 20, 'nd' holder in loop 1 alone 20, both in loop 1 19, both in both loops 18 (build_insns 77 == target_insns 77 throughout). Floor 20 -> 18.
- verdict: CONFIRMED

## [s4] Session 3's frontier F1(c) - re-homing one of 'val's three roles into an already-dead EXISTING local - lowers val's ref count and flips the D2 $v0/$v1 ordering without the D1 loss that session 3's vL suffered.
- mechanism: global.c allocno_compare orders allocnos by floor_log2(nrefs)*nrefs/livelen; val's 19 refs come from three reuses, and removing one was expected to drop it below hi (nrefs 8, pri 13333).
- probe: The session-4 carrier form itself (dst2 = 0x20 - bits_left; val = dst2;) read against BB2_ALLOC_DEBUG output, tmp/grind/func_8001979C/s4/qty_vE3.log.
- result: KILLED as a mechanism. The read-back 'val = dst2;' IS a reference, so val keeps all 19 refs: pseudo 83 ord 1 nrefs 19 livelen 15 pri 50666 -> $v1 and pseudo 78 (hi) ord 7 nrefs 8 livelen 18 pri 13333 -> $v0, bit-for-bit identical to the session-3 numbers. The only re-home that actually removes val's refs is vL, which loses D1 (build_insns 75). The D2 ordering can now only be attacked from hi's side.
- verdict: KILLED

## [s4] An opaque local holding the 0x20 constant (permuter outputs 345-1 / 375-1 / 400-1 / 325-2) improves the shift/subtract allocation.
- mechanism: a separate pseudo for the constant changes which allocno owns 0x20 and when it is live, feeding global.c allocno priorities and loop.c's constant hoist.
- probe: vA (holder feeding all four 0x20-minus-x subtractions), vB (shift amount only), vI (the session-4 carrier line only), vJ (vI stacked with a condition holder); sandbox --disable all on each.
- result: 21, 31, 31, 35 - vB/vI/vJ also push build_insns to 78. Every spelling is worse than the 18 reached without it. The axis is dead at this baseline (and is the spelling nearest the forbidden 's32 one = 1;' opaque-variable family, so it must not be revived without a ruling).
- verdict: KILLED

## [s4] Hoisting the arm's if condition into an existing dead local (permuter output 300-1) changes the compare/branch allocation.
- mechanism: the condition becomes a pseudo with its own allocno instead of feeding the branch directly, changing local-alloc quantity spans in the loop head.
- probe: vG ('needed = bits_left < w; if (needed)') and vH (same into the session-4 local 'nd'); sandbox --disable all.
- result: vG 22, vH 18 (exactly neutral). No gradient in either direction; the condition holder buys nothing.
- verdict: KILLED

## [s4] The identity of the walker used as the shift-amount carrier matters.
- mechanism: any dead local should serve equally if the effect is purely an extra def-use pair.
- probe: vE5 - carrier = the third-loop walker 'out' instead of the other bit loop's walker; sandbox --disable all.
- result: 20 vs 18 for the other-bit-loop walker. Only a walker whose live range lies inside the other bit loop produces the gain, so the effect is live-range pressure in the bit loops, not the extra def-use pair.
- verdict: KILLED

## KILLED (session 5)

- **H5-B: the third loop's `-2` holder can be moved off `val` without losing
  D1, now that the D2 contest is settled.**
  *Mechanism:* global.c allocno_compare - target wants the -2 in $v1 and the
  `out` address in $v0, i.e. the -2 allocno must rank BELOW `out`.
  *Probes (all on the score-12 chassis):* onto `hi`, onto `needed`, onto `nd`,
  onto a fresh local, and copied through a fresh local with `val` still
  holding it.
  *Result:* 12/build 75, 14/build 75, 31, 12/build 75, and 12/build 77
  (neutral). **KILLED.** Every spelling that removes a real `val` reference
  costs D1's `subu $v0,$t2,$a0` + `addu $a3,$v0,$zero` pair (build_insns 75).
  This re-confirms session 3's vL coupling at a completely different
  allocation, so it should be treated as structural, not baseline-specific.

- **H5-C: with D2 closed the walker/counter priority window has moved, so the
  late walker init (target's preheader order, D4) is now affordable.**
  *Probe:* session 2's P6 shape re-applied to the score-12 chassis, both loops
  and loop-1-only.
  *Result:* 27 and 27. **KILLED at this baseline too.** D4's coupling to the
  allocno window survived the D2 fix untouched.

- **H5-D: the opaque constant-holder axis (killed in s4) reopens at the new
  baseline** - the vTie campaign's best find (perm 290 of base 420) is a holder
  for the field-width constant feeding `nd = wc - bits_left;`.
  *Probe:* holder in both loops, and the literal loop-2-only find.
  *Result:* 18 and 12 (neutral). **RE-KILLED.** No engine-gradient value, so the
  policy question about opaque constant holders never has to be answered here.

- **H5-E: D5 (target's `sllv $a2,$a2,$a0` before the `or`) is reachable by
  putting `cur <<= needed;` before the store in source.**
  *Probe:* vTieShiftEarly.
  *Result:* 12 with a bit-identical residual. **KILLED** - the emission order is
  chosen by the scheduler, not by statement order.

## CONFIRMED (session 5)

- **H5-A: `hi` can be made to outrank the arm's temps in global.c
  allocno_compare by GAINING references (session 3's F1(a), never executed),
  and once it holds $v1 the session-3 vC tie closes D2 instead of mirroring
  it.**
  *Mechanism:* allocno_compare priority `floor_log2(nrefs)*nrefs/livelen`.
  Routing the merged value through `hi` before the store adds two references
  per arm (weighted x2 in-loop), taking `hi` from nrefs 8 / pri 13333 to
  nrefs 16 / livelen 20 / pri 32000 - across the floor_log2 3 -> 4 bucket that
  session 4 derived as the necessary condition. `hi` then takes hardreg 3
  ($v1) and `val` hardreg 2 ($v0), target's orientation, after which
  `hi = 0x20 - bits_left; hi = cur >> hi;` produces target's shared-register
  `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1`.
  *Probes:* vF1a_loop1 (18), vF1a_both (16), vTie (12), each with
  `sandbox --disable all`, plus the ALLOCDBG dump
  `tmp/grind/func_8001979C/s5/qty_vF1a.log` and cmp.py residuals.
  *Result:* **CONFIRMED. Floor 18 -> 12 and D2 (14 points across sessions 1-4)
  is closed.** Note the lesson for the pipeline: the winning probe had been
  written down as the top frontier item since session 3 and skipped twice.

## FRONTIER (ranked, as of end of session 5)

1. **F1 - D5 (4 pts, NEW): the arm's `or` / `sllv $a2,$a2,$a0` emission
   order.** Target schedules `cur <<= needed` before the `or`; we schedule it
   after. Source order is proven neutral (H5-E), so this is a list-scheduler
   question: `tools/sched_solver` models GCC 2.7.2's sched.c exactly
   (memory/project/sched-solver-campaign-2026-08-05.md) and should be pointed
   at this arm to find which INSN_PRIORITY tie-break decides it and what C
   change moves it.

2. **F2 - D6 (4 pts, NEW): the third loop's $v0/$v1 mirror.** Target: `li
   $v1,-2` + `addiu $v0,$t3,0x348`; ours is the reverse because `val` (which
   holds the -2) is now the $v0 allocno. Every re-home of the -2 is measured
   dead (H5-B) because it costs D1. The untried direction is the mirror of the
   session-5 win: ADD references to `out` (the third loop's address walker) so
   it outranks `val` and is allocated first, instead of trying to demote
   `val`. `out` currently has ~6 refs against `val`'s 19, so the same
   floor_log2-bucket arithmetic applies (out would need pri > 50666).

3. **F3 - D4 (4 pts): the walker init's position in the preheader.**
   Unchanged and now re-tested at two different allocations (session 2's P6 at
   floor 20, session 5's vTieLateInit at floor 12; 31 and 27). The remaining
   untried lever is the one session 3 wrote down and no session has executed:
   read `tools/gcc-2.7.2/loop.c` `move_movables` / `scan_loop` to find where
   the preheader insertion point is chosen relative to the loop's first insn,
   and whether the walker init is a movable at all in this form.

4. **Policy pre-clearance (smaller than it was).** The session-4 carrier
   construct is gone from the candidate. What remains for a self-vet: the
   duplicated `dst += 2` (byte-neutrality proven in session 3) and ordinary
   variable reuse (`hi` carrying its own shift amount; `val` as the D1
   intermediate and the -2 holder). Both still need a VERBATIM scope quote
   plus a file:line/commit precedent before any candidate-ready, or a
   ruling-request.

## [s5] Session 3's F1(a) probe - routing the merged field value through `hi` before the store so `hi` gains references - lifts `hi` across a floor_log2 bucket in global.c allocno_compare, gives it $v1, and (with the session-3 vC tie stacked on top) closes D2 entirely.
- mechanism: allocno_compare priority = floor_log2(nrefs)*nrefs/livelen, in-loop refs weighted x2. `hi` goes from nrefs 8 / livelen 18 / pri 13333 (hardreg 2 = $v0) to nrefs 16 / livelen 20 / pri 32000 (hardreg 3 = $v1); `val` keeps nrefs 19 / pri 50666 and takes $v0. With `hi` already in $v1, computing the 0x20 - bits_left shift amount into `hi` itself (session 3's vC) reproduces target's shared-register `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1` instead of session 3's $v0 mirror.
- probe: vF1a_loop1, vF1a_both, vTie - each measured with sandbox --disable all; BB2_ALLOC_DEBUG dump tmp/grind/func_8001979C/s5/qty_vF1a.log; residual diffs with tmp/grind/func_8001979C/s3/cmp.py.
- result: 18 (loop 1 alone, neutral), 16 (both loops), 12 (both loops + the tie), build_insns 77 == target_insns 77 throughout. Both bit loops are now byte-identical to target through the entire shift/or/store chain. Floor 18 -> 12; the 14-point D2 family is closed and the session-4 carrier construct is deleted along the way.
- verdict: CONFIRMED

## [s5] The third loop's -2 holder can be moved off `val` without losing D1's copy pair, now that the D2 contest is settled.
- mechanism: global.c allocno_compare - target wants the -2 in $v1 and the `out` address in $v0, so the -2 allocno must rank below `out`.
- probe: On the score-12 chassis: -2 re-homed onto `hi`, onto `needed`, onto `nd`, onto a fresh local, and copied through a fresh local while `val` still holds it; all sandbox --disable all.
- result: 12 (build 75), 14 (build 75), 31, 12 (build 75), 12 (build 77, exactly neutral). Every spelling that removes a real `val` reference drops build_insns to 75 - D1's subu/addu pair stops materialising, exactly as session 3's vL did at a completely different allocation. Instruction parity is a hard invariant, so all are rejected.
- verdict: KILLED

## [s5] With D2 closed the walker/counter allocno window has shifted, so target's late walker init (D4) is now affordable.
- mechanism: global.c allocno_compare; moving an init late shortens the walker's live range and lifts it above the counter (session 2's P6).
- probe: Session 2's P6 shape re-applied to the score-12 chassis, both loops and loop-1-only; sandbox --disable all.
- result: 27 and 27, against the 12 baseline. The coupling survived the D2 fix untouched.
- verdict: KILLED

## [s5] The opaque constant-holder axis (killed in session 4) reopens at the score-12 baseline - the vTie campaign's best find (perm 290 vs base 420) is a holder for the field-width constant feeding `nd = wc - bits_left;`.
- mechanism: a separate pseudo for the constant changes which allocno owns the width and when it is live.
- probe: Holder in both loops, and the literal loop-2-only permuter find; sandbox --disable all.
- result: 18 and 12 (neutral). No gradient. Re-killed at the new baseline, so the policy question about opaque constant holders does not have to be answered for this function.
- verdict: KILLED

## [s5] D5 (target emits `sllv $a2,$a2,$a0` before the `or`, we emit the `or` first) is reachable by moving `cur <<= needed;` above the store in source.
- mechanism: RTL emission order feeding the delay-slot/scheduling decision.
- probe: vTieShiftEarly - `cur <<= needed;` moved above the store in both arms (semantically neutral because `hi` already holds the merged value); sandbox --disable all + cmp.py.
- result: 12, with a bit-identical residual line for line. The order is chosen by sched.c, not by statement order.
- verdict: KILLED

## [s5] Splitting the OR into an accumulation (`hi = hi << needed; hi = hi | (cur >> bits_left);`), a permuter proposal, changes the arm's codegen. Operand ORDER is preserved, so this is not the or-tree reshape forbidden by .claude/rules/or-tree-shape-shift.md.
- mechanism: split-init accumulation gives the OR's two operands separate RTL statements.
- probe: vSplitOr; sandbox --disable all.
- result: 12, exactly neutral.
- verdict: KILLED

## [s5] Session 3's never-executed F1(a) probe - routing the merged field value through `hi` before the store so `hi` gains references - lifts `hi` across a floor_log2 bucket in global.c allocno_compare, gives it $v1, and (with session 3's vC tie stacked on top) closes D2 entirely.
- mechanism: allocno_compare priority = floor_log2(nrefs)*nrefs/livelen with in-loop refs weighted x2. `hi` goes from nrefs 8 / livelen 18 / pri 13333 (hardreg 2 = $v0) to nrefs 16 / livelen 20 / pri 32000 (hardreg 3 = $v1) while `val` keeps nrefs 19 / pri 50666 and takes $v0 - target's orientation. With `hi` already in $v1, computing the 0x20 - bits_left shift amount into `hi` itself reproduces target's shared-register `subu $v1,$t2,$a3 ; srlv $v1,$a2,$v1` instead of session 3's $v0-landing mirror.
- probe: vF1a_loop1 / vF1a_both / vTie measured with `sandbox func_8001979C --disable all`; BB2_ALLOC_DEBUG + BB2_QTY_DEBUG dump (tmp/grind/func_8001979C/s5/qty_vF1a.log); residuals with tmp/grind/func_8001979C/s3/cmp.py.
- result: 18 (loop 1 alone, neutral), 16 (both loops), 12 (both loops + the tie); build_insns 77 == target_insns 77 throughout. Both bit loops are byte-identical to target through the whole shift/or/store chain.
- verdict: CONFIRMED

## [s5] The third loop's -2 holder can be moved off `val` without losing D1's copy pair, now that the D2 contest is settled (target wants the -2 in $v1 and the `out` address in $v0).
- mechanism: global.c allocno_compare - the -2 allocno must rank below `out` to be allocated second and take $v1.
- probe: On the score-12 chassis: -2 re-homed onto `hi`, onto `needed`, onto `nd`, onto a fresh local, and copied through a fresh local while `val` still holds it; sandbox --disable all on each.
- result: 12 (build 75), 14 (build 75), 31, 12 (build 75), 12 (build 77, exactly neutral). Every spelling that removes a real `val` reference drops build_insns to 75 - D1's subu/addu pair stops materialising, exactly as session 3's vL did at a completely different allocation.
- verdict: KILLED

## [s5] With D2 closed the walker/counter allocno window has shifted, so target's late walker init (D4) is now affordable.
- mechanism: global.c allocno_compare; moving an init late shortens the walker's live range and lifts it above the counter (session 2's P6 regression).
- probe: Session 2's P6 shape re-applied to the score-12 chassis, both loops and loop-1-only; sandbox --disable all.
- result: 27 and 27 against the 12 baseline. The coupling survived the D2 fix untouched.
- verdict: KILLED

## [s5] The opaque constant-holder axis (killed in session 4) reopens at the score-12 baseline - the vTie campaign's best find (perm 290 vs base 420) is a holder for the field-width constant feeding `nd = wc - bits_left;`.
- mechanism: a separate pseudo for the constant changes which allocno owns the width and when it is live.
- probe: Holder in both loops, and the literal loop-2-only permuter find; sandbox --disable all.
- result: 18 and 12 (neutral). No gradient in either direction, so the policy question about opaque constant holders never has to be answered for this function.
- verdict: KILLED

## [s5] D5 (target emits `sllv $a2,$a2,$a0` before the `or`; we emit the `or` first) is reachable by moving `cur <<= needed;` above the store in source.
- mechanism: RTL emission order feeding the scheduling decision.
- probe: vTieShiftEarly - `cur <<= needed;` moved above the store in both arms (semantically neutral because `hi` already holds the merged value); sandbox --disable all + cmp.py.
- result: 12, with a residual identical line for line. The order is chosen by sched.c, not by statement order.
- verdict: KILLED

## [s5] Splitting the OR into an accumulation (`hi = hi << needed; hi = hi | (cur >> bits_left);`), a permuter proposal that preserves operand ORDER and so is not the reshape forbidden by .claude/rules/or-tree-shape-shift.md, changes the arm's codegen.
- mechanism: split-init accumulation gives the OR's two operands separate RTL statements.
- probe: vSplitOr; sandbox --disable all.
- result: 12, exactly neutral.
- verdict: KILLED
