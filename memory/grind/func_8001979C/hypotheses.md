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
