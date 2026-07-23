# Evidence bank — func_800692C0

- s1 (recon 2026-07-23): floor RE-CONFIRMED THIS session — canonical verdict=C
  distance=9; sandbox --disable all score=9 (67/67 insns, rules_dropped=1,
  cheat_asm_stripped=355). Pin-free candidate.c form measured in src.
- s1 ISOLATION (new, refines "propagates through ~7 insns"): the 9 diffs reduce to
  EXACTLY TWO allocnos disagreeing. Only sum and bitpos swap hard regs
  (target sum=$t2/$10, bitpos=$t1/$9; ours reversed). EVERY other allocno is
  register-IDENTICAL to target: i=$t3, a3_off=$t5, p=$t4, const1=$t6, plus the
  whole diamond body byte-identical. Second root cause: li t6,1 (LICM-hoisted
  loop-invariant const 1) sits at preheader slot 4 in target, slot 8 (last) in
  ours. Artifact: tmp/grind/func_800692C0/s1/ours_disasm.txt.
- s1: NO duplicate/sibling analog — grep 800692C0 in tmp/duplicates_leads.txt = 0 hits.
- s1: target places bitpos in the LOWER temp ($t1/$9), sum in the higher ($t2/$10).
  Priority-consistent read: sum is the return value (live from prologue set to the
  final `addu v0,t2,0`) => LONGEST live range => LOWER local-alloc priority
  (refs/live_length) => assigned later => higher reg. bitpos dies at its `+=0x10`
  => shorter range => higher priority => lower reg. Target matches this;
  our build inverts the tie. Prior sessions permuted decl/init ORDER only (all
  score>=9) — live-RANGE restructuring of sum's return path is UNTRIED.

- WIP rejected_form: {'form': 'bitpos declared first (DECL_BITPOS_FIRST), init base', 'score': 9, 'why': 'no allocation change'}

- WIP rejected_form: {'form': 'sum-first decl, bitpos initialized before sum', 'score': 12, 'why': 'worse'}

- WIP rejected_form: {'form': 'sum-first decl, sum=0 moved to last init', 'score': 10, 'why': 'worse'}

- WIP rejected_form: {'form': 'sum-first decl, bitpos=0 moved to last init', 'score': 10, 'why': 'worse'}

- WIP rejected_form: {'form': 'bitpos-first decl, bitpos=0 moved to last init', 'score': 10, 'why': 'worse'}

- WIP rejected_form: {'form': '1<<bitpos extracted to s32 sh local (force bitpos read before sum)', 'score': 11, 'why': 'worse — adds an extra computation that broke other allocation'}

- WIP rejected_form: {'form': 'plain `register` (no asm pin) on sum and bitpos', 'score': 9, 'why': 'no effect — GCC ignores the K&R hint here'}

- WIP rejected_form: {'form': 'p=&D_800A32D0 materialized FIRST in init', 'score': 17, 'why': 'much worse — disrupts the whole prologue scheduler'}

- WIP rejected_form: {'form': 'rewrite do/while as for(bitpos=0,i=0; i<2; bitpos+=0x10,p++,i++)', 'score': 15, 'why': 'for-clause comma operator changes RTL gen'}

- WIP rejected_form: {'form': 'ANSI decl+init combining decl and assignment', 'score': 11, 'why': 'worse'}

- WIP rejected_form: {'form': 'arg1<<=4 hoisted to first init line', 'score': 13, 'why': 'worse'}

- WIP rejected_form: {'form': 'arg1<<=4 deferred past zero-inits', 'score': 11, 'why': 'worse'}

- == imported from memory/wip notes.md ==
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


- [s1] s1 canonical func_800692C0: verdict=C, asm_insns=0, distance=9.

- [s1] s1 sandbox --disable all: score=9, target_insns=67, build_insns=67, rules_dropped=1, cheat_asm_stripped=355.

- [s1] Isolation: ONLY sum($t2 target / $t1 ours) and bitpos($t1 target / $t2 ours) allocnos disagree. i=$t3, a3_off=$t5, p=$t4, const1=$t6 are register-identical to target; diamond body byte-identical.

- [s1] Second root cause: li t6,1 (LICM loop-invariant const 1) at preheader slot 4 in target vs slot 8 (last) in ours.

- [s1] Priority read: sum is the return value (live prologue->final `addu v0,t2,0`) => longest live range => lowest local-alloc priority => higher reg; bitpos dies at its `+=0x10` => shorter range => lower reg. Target obeys this; our build inverts the tie. Prior sessions permuted decl/init ORDER only (all >=9); live-RANGE restructuring of sum's return path is UNTRIED.

- [s1] No duplicate analog: grep 800692C0 in tmp/duplicates_leads.txt = 0 hits.

- [s1] src/text1b.c restored to HEAD after measurement (oracle-green); pin-free floor-9 form preserved in candidate.c.

== s2 (structural, 2026-07-23) — greg ALLOCDBG isolation of the RA tie ==
- EXACT allocno numbers (tmp/gccdbg/cc1 BB2_ALLOC_DEBUG on standalone sa.c):
  sum=pseudo76 nrefs=10 livelen=62 pri=4838 -> $t1($9);
  bitpos=pseudo79 nrefs=9 livelen=57 pri=4736 -> $t2($10);
  i=pseudo77 nrefs=9 livelen=60 pri=4500. TARGET wants bitpos->$t1, sum->$t2.
  global.c allocno_compare: pri=flog2(nrefs)*nrefs*size/live_length, ties by
  allocno# (sum# < bitpos# -> sum wins ties). ALLOC ORDER == REGISTER ORDER
  (no prefs on these temps; caller-saved handed out $t0,$t1,... in pri order).
- To flip: need pri(bitpos) > pri(sum) while keeping BOTH above i(4500).
  Deterministic: with nrefs(10,9) fixed by the byte-identical (post-jump2)
  loop body, flip requires sum livelen>=63 OR bitpos livelen<=55.
- MEASURED DEAD (clean structural):
  * 10 preheader init orderings (ABCDEF..): sum ll PINNED at 62, bitpos ll
    floor 56 (candidate order=57). const1(li t6) always schedules AFTER
    bitpos's def -> can't shave bitpos below 56. Gap always >=17. Init-order
    CANNOT flip. (confirms + supersedes prior "source-not-orderable".)
  * recompute bitpos=a3_off<<3 (and i<<4): flips RA correctly (bitpos ll->24,
    pri 5000, return move $2,$10) BUT drops carried `addiu $t1,0x10` ->
    build_insns 66 vs 67, score 14. Target's bitpos IS carried; recompute
    family DEAD regardless of source var.
  * shared-goto accumulate (matches target's merged .L8006938C addu): jump2
    merges the tail AFTER alloc, so target's alloc-time form is TWO
    accumulates (=candidate, sum nrefs 10). Writing the merge in source drops
    sum nrefs->6, pri 4838->2033 -> sum lands $t5. score 16. DEAD.
- PROOF-OF-CONCEPT (cheat, NOT banked): `s32 one=1; sum += one<<bitpos;` +
  bitpos=0 last. Injects a redundant preheader li in sum's live range ->
  sum ll 62->63 pri 4761, bitpos ll 56 pri 4821 -> FLIPS to target RA,
  return move $2,$10, sandbox 9->5. REJECTED: build_insns 68 vs 67 (extra
  li has no semantic purpose beyond RA-steering; target is 67/one-li). Score-5
  is cheat-assisted; HONEST FLOOR STAYS 9. Proves RA-flip is worth ~4 pts and
  the sole obstacle to a clean 67-insn flip is +1 sum-livelen at constant
  insn count (unreachable by ordering). Artifacts: tmp/grind/func_800692C0/s2/
  allocdbg_base.txt, allocdbg_combo_flip.txt, func692C0.greg.txt.

- [s2] greg ALLOCDBG (BB2_ALLOC_DEBUG): sum=pseudo76 nrefs=10 livelen=62 pri=4838 ->$t1($9); bitpos=pseudo79 nrefs=9 livelen=57 pri=4736 ->$t2($10); i=pseudo77 nrefs=9 livelen=60 pri=4500. Target wants bitpos->$t1, sum->$t2.

- [s2] allocno_compare is deterministic (pri=flog2(nrefs)*nrefs*size/live_length, ties by allocno#); alloc order == register order (no register prefs on these caller-saved temps).

- [s2] 10 preheader init orderings measured: sum livelen invariant at 62, bitpos livelen floor 56 (candidate order 57). const1 (li t6,1) always schedules AFTER bitpos's def -> bitpos livelen cannot drop below 56. Gap >=17 always.

- [s2] Target's merged single `addu $t2` (.L8006938C) is produced by jump2 cross-jump AFTER register allocation -> target's allocation-time form is our candidate (two separate accumulates, sum nrefs=10), not a source-level shared accumulate.

- [s2] `one` opaque-var flip: sandbox 9->5 with build_insns 68 (extra redundant li). Cheat (RA-steering, no semantic purpose, not the 67-insn original) -> not banked; proves RA-flip worth ~4pts.

- [s2] src/text1b.c restored to HEAD (pinned form) after measurement; tree oracle-green. Pin-free floor-9 form preserved in candidate.c.
