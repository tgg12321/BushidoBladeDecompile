# func_800692C0 (text1b.c) — BLOCKED on coupled li-hoist + sum/bitpos RA swap

## TL;DR
Honest pure-C floor = **9**, from exactly TWO coupled effects:
1. **`li t6,1` scheduling position** — the hoisted loop-invariant constant `1`
   sits right after `sll a1,a1,0x4` in target, but after the `lui/addiu t4`
   (p=&D_800A32D0) pair in our build. (the regfix `reorder 6,3,4,5 @ 3-6` fixes this.)
2. **sum/bitpos register swap** — target sum=`$t2`($10), bitpos=`$t1`($9); GCC
   gives us the reverse. Propagates through ~7 sllv/addu/move insns. (the two
   `register asm("$10")`/`asm("$9")` pins fix this.)

HEAD "matches" only via those pins + the reorder rule — both forbidden cheats.
Confirmed clean side-by-side diff (tmp/cmp611.sh) shows the rest of the 67-insn
body is byte-identical. This needed BOTH a register pin AND a reorder regfix
historically (commit ada74e46 "inline-replace + register-asm pin + reorder
regfix") — a strong signal it's a genuine coupled scheduling+RA wall.

## What was tried (don't re-derive)
- 6 decl-order / init-order permutations (bitpos-first decl; bitpos-before-sum;
  sum-last; bitpos-last; combos). ALL score >=9 (several made it worse). Source
  ordering does not move either diff.
- (2026-06-22) 8 more pure-C variants: shift-temp extraction `s32 sh=1<<bitpos`;
  plain `register` storage-class hint (no asm pin) — scored 9, GCC ignores the
  K&R hint here; `register`+shift-temp combined; p=&D_800A32D0 materialized
  FIRST; for-loop with bitpos in the for-clause; ANSI decl+init in bitpos-first
  order; arg1<<=4 hoisted to first init; arg1<<=4 deferred past zero-inits.
  Best non-baseline scored 11; baseline (and plain-register hint) stay at 9.
  Confirms the wall is not source-orderable.
- (2026-06-22) Attempted decomp-permuter import from candidate.c — import.py
  pulls in a sibling inline-asm body (func_8004A348) and the maspsx pipeline
  then fails with `too many values to unpack (expected 2)`. Permuter cannot
  drive this function until the import pipeline is patched to handle sibling
  inline-asm in text1b.c, OR the import is hand-trimmed to JUST this function's
  context.

## Resume guidance
The two diffs look coupled (sched li-placement + allocator t1/t2 tie). Best next
steps (meta.json next_hypotheses, in order):
1. decomp-permuter from candidate.c (floor 9) — the documented next step for a
   scheduling+RA plateau; setup must use the project maspsx/fix_lwl pipeline.
2. li-hoist: try re-materializing p=&D_800A32D0 differently so the scheduler
   floats `li t6,1` up into the slot the lui/addiu currently occupies.
3. sum/bitpos: sum is the return value (move v0,t2). Try restructuring the
   accumulation / explicit return var to flip the t1<->t2 allocator tie.
4. Instrumented-cc1 SCHED/PRIO dumps to pinpoint the minimal flip.

Source left at HEAD (oracle green). Card BLOCKED. Needs a permuter find, a
deeper RA/sched lever, or user canonical-asm authorization.
