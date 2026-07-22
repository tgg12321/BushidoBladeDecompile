# Evidence bank — damage_DebugDisp

## s1 (recon, 2026-07-22) — floor 8 confirmed, diff map nailed
- Applied candidate.c (for(;;) fence on CopyBlock loop) → `sandbox --disable all` = **8**
  (HEAD do-while form = 9). Verdict C, target 79 / build 79 insns, 8 rules dropped.
- NOT in tmp/duplicates_leads.txt — no near-clone sibling to transplant.
- Objdump of the floor-8 build (tmp/grind/.../s1/build_disasm.txt) vs asm/funcs shows
  9 positional diffs in TWO independent regions (sandbox masks 1 → score 8):
  * **Region A** (inner accumulator loop, build 0x124/0x128/0x12c/0x140): pure a0↔a1
    swap. sum=$a0/j=$a1 in target vs sum=$a1/j=$a0 in build. Scheduling identical
    (j++ fills lbu load-delay in both). greg dispositions: sum=pseudo 77→a1,
    j=pseudo 79→a0 — sum has the LOWER pseudo but j wins the LOWER reg, so it is a
    local-alloc PRIORITY tiebreak (counter j outranks accumulator sum for $a0), not a
    pseudo-order tie. sum is live across loop exit (beq), j dead at exit, yet j wins.
    j is a genuine int counter (target `sltiu ...,0x24`) → cannot be replaced by a
    pointer-end compare.
  * **Region B** (2nd-loop preheader, build 0x1e8–0x1f8): LICM-hoisted constant block
    {lui t0=0x80000000; lui a3=0x1f; ori a3=0x1FFFFF} vs pre-loop move block
    {a0=base; a2=a0}. Build emits moves-first; target emits constants-first. Mechanism:
    LICM inserts hoisted invariants at preheader END (higher LUID); sched1 ties break
    to lower-LUID moves. Independent of Region A.
- Both regions are pure reg-alloc/scheduling ties (no cheat needed); the matching C
  exists per [[difficult-is-not-impossible]] / [[no-compiler-divergence]].

- WIP rejected_form: {'form': 'magic_base/magic_max = const named locals declared before ap=base assignment', 'score': 4, 'reason': 'no-semantic-purpose constant rename for LUID bias only; the locals have no observable behavior change (GCC const-folds them) and lack SOTN-master-branch evidence as a sanctioned spelling family per [[no-new-park-categories]] cheats-by-any-spelling. The `narrow-byte-args-packed-call` hi/lo example sanctions named-intermediates of COMPUTATION; mine renames PURE CONSTANTS, which extends the family.'}

- WIP rejected_form: {'form': "sum=0 before bp/j init (V11): swap inner-loop init order to bias sum's pseudo number lower", 'score': 9, 'reason': "not a cheat per se, but score WORSENED — swap rotated multiple registers (sum -> $a1, bp -> $a0, j -> $v1) instead of swapping just $a0/$a1. Local-alloc priority isn't trivially controlled by source order alone."}

- WIP rejected_form: {'form': 'for(j=0;j<0x24;j++) instead of do-while with explicit init (V7)', 'score': 9, 'reason': 'for-loop form changes the codegen of the inner test/inc placement; no improvement.'}

- WIP rejected_form: {'form': 'j as s32 instead of u32 (V8)', 'score': 9, 'reason': 'sltiu vs slti changes the compare opcode; lost 1 insn but added 2 diffs elsewhere — net negative.'}

- == imported from memory/wip notes.md ==
# damage_DebugDisp WIP

## TL;DR (2026-06-13, session 1)

Floor lowered 9 → 8 via the user-sanctioned **loop-exit-work-inside-loop-sched-fence**
respelling on the 4-block `CopyBlock` copy loop in the if-skip-able copy block. The
`for(;;)/continue/break` with the tail-word copy INSIDE the loop and `k = 0;` OUTSIDE
puts the `NOTE_INSN_LOOP_END` mid-block, pinning the `k=0` after the `sw` and
materialising the load-delay nop on the trailing-word `lw`. Retires the two trailing-
region rules (`insert "nop" @ 53`, `reorder 55,54 @ 54-55`). Cheat-reviewer PASS.

The remaining 8 diffs are TWO independent register-allocation / scheduling problems
in OTHER regions of the function — both untouched this session, both already-known
shape, neither closed by any clean lever found here.

## Resume (apply candidate, then iterate)

```pwsh
cp memory/wip/damage_DebugDisp/candidate.c /tmp/cand.c
# splice into src/code6cac_c_mid.c lines 249-322 (or just diff in the for-loop fence)
& tools/eng.ps1 sandbox damage_DebugDisp --disable all   # expect score 8
```

## Remaining gap (8 diffs)

### Inner sum-loop $a0/$a1 swap (3 diffs)

Build: `$a0 = j` (loop counter), `$a1 = sum` (accumulator), `$v1 = bp`.
Target: `$a0 = sum`, `$a1 = j`, `$v1 = bp` — same regs used, $a0/$a1 swapped.

The 3 diffs are at `addiu`/`addu`/`sltiu` in the inner loop body plus the `beq` post-
loop comparison.

GCC's local-alloc tiebreaker assigns the lower-pseudo-number first. `j = 0;` is in
source order BEFORE `sum = 0;`, so j gets the lower pseudo, allocated to $a0 first.
Reordering source to put `sum = 0;` first (V11) rotated multiple regs (sum→$a1,
bp→$a0, j→$v1) instead of just flipping $a0/$a1 — score went 8→9.

### Constants-vs-moves order in second-loop preheader (~5 diffs)

Build:
```
move a0,t1      (ap = base)
move a2,a0      (a2p = ap)
lui  t0,0x8000  (const 0x80000000)
lui  a3,0x1f
ori  a3,a3,0xffff (const 0x1FFFFF)
```

Target:
```
lui  $t0,0x8000
lui  $a3,0x1f
ori  $a3,$a3,0xffff
move $a0,$t1
move $a2,$a0
```

The constants are LICM-hoisted from inside the do-while body; the moves are the
pre-loop init. The basic-block scheduler picks moves first because their INSN_PRIORITY
(longest dependency chain to function exit through `lw v1,120(a0)`) exceeds the
constants' priority. To flip, the constants would need higher priority OR lower LUID.

## Ruled-out this session

- swap inner-loop init order `sum=0` first (V11): score 9 (regression — rotation)
- `for (j=0; j<0x24; j++)` form (V7): score 9
- `j` as `s32` (V8): score 9 — slti vs sltiu changes the cmp opcode
- swap inner-loop body order `j++; sum += *bp; bp++` (V10, V12): score 5-10
- sum at function-level scope (V9): stays 8 — sum's outer scope doesn't change alloc priority
- inline init declarations with combined decl+init (V2, V3): score 8-13

## Next hypotheses (not yet tried)

1. **BB2_ALLOC_DEBUG dump on cc1** — see which pseudos receive which regs and why,
   for the inner sum loop. Lever evidence may emerge from the conflict graph.
2. **Block-local sum split**: `sum_inner` used only in the inner loop + `sum_outer =
   sum_inner;` for the compare. Shortens inner pseudo's live range, may flip
   priority. Not tried.
3. **Sibling cross-reference**: find a function in `code6cac_c_mid.c` (or its
   sister files) with the same `(byte sum-loop, post-loop compare)` shape that
   MATCHED in pure C, and diff its declaration / RTL shape against this one.
4. **Constants-vs-moves: structural lever for the second loop** — instead of
   biasing LUID by renaming constants (rejected as cheat-by-spelling), look for
   a control-flow / type change that makes GCC emit constants first naturally.
   None found yet.

## Pointers

- Rule used (sanctioned): `.claude/rules/loop-exit-work-inside-loop-sched-fence.md`
- Rejected technique: `magic_base/magic_max` constant-rename — score 4 but
  no-semantic-purpose. See `meta.json.rejected_forms[0]`.


- [s1] canonical verdict C, pure-C distance 9 at HEAD; candidate floor 8 reproduced this session (sandbox --disable all).

- [s1] damage_DebugDisp is NOT in tmp/duplicates_leads.txt — no near-clone COMPLETED-C sibling to transplant.

- [s1] Total 9 raw positional diffs (4 Region A + 5 Region B); sandbox masks 1 register-rename pair -> score 8.

- [s1] Region A: build j=$a0/sum=$a1, target sum=$a0/j=$a1; scheduling identical; greg: sum=pseudo77->a1, j=pseudo79->a0 => local-alloc PRIORITY tiebreak, not pseudo-order.

- [s1] j is a genuine integer counter (target sltiu ...,0x24) — cannot be eliminated via a pointer-end compare without diverging from target's loop shape.

- [s1] Region B: LICM inserts hoisted invariants at preheader END (higher LUID) => sched1 schedules the lower-LUID ap/a2p moves first; target had constants at lower LUID.

- [s1] Both regions are pure-C reachable reg-alloc/scheduling ties (no compiler divergence; matching C exists per no-compiler-divergence).

## s2 (structural, 2026-07-22) — Region A quantified; naive structural levers KILLED
- Confirmed floor 8 with candidate.c applied. Modality: structural.
- **Full-file greg disposition proves the diff is a PURE sum/j swap with ALL surroundings matching target.**
  My build: base=t1(73), i=a2(74), chkptr=t0(75), offset=a3(76), sum=$a1(77),
  bp=$v1(78), j=$a0(79). Target: identical EXCEPT sum=$a0, j=$a1. So 4 Region-A
  diffs = sum($a1<->$a0) / j($a0<->$a1) swap only.
- **Register hand-out order for this class/range = v1 -> a0 -> a1** (1st/2nd/3rd
  qty allocated). qty_order among the three (baseline) = `78(bp) 79(j) 77(sum)`
  => bp=v1, j=a0, sum=a1. Target requires order `78 77 79` (bp,sum,j) i.e.
  priority **bp > sum > j**; baseline is bp > j > sum. Must lift sum above j.
- **Priority numbers (BB2_QTY_DEBUG via tmp/gccdbg/cc1):** local-alloc priority
  = floor_log2(refs)*refs*size/(death-birth)*10000. Inner loop: j range 4 refs 4
  -> 20000; sum range 6 refs 4 -> 13333 (sum lives to the post-loop `beq`, j dies
  at the in-loop `sltiu`). j strictly outranks sum => j takes the earlier reg.
  Both refs=4 are FIXED by the target loop shape (init+accumulate(r/w)+compare);
  the ONLY free variable is the birth/death LUID span, which is CONTEXT-DEPENDENT
  (full-file), not isolable — a standalone single-fn TU reproduces neither the
  pseudo numbering nor the allocation (standalone gives sum=$a2, real gives $a1).
- **KILLED structural levers (all measured this session):**
  * chk-hoist before inner loop (frontier probe 1): score 17 — loop-carried load
    poisons the whole allocation. rejected/chk-hoist-loop-carried.c
  * init reorder `bp;sum;j` and `sum;bp;j` (== inherited V11): score 13 — bp drops
    below j and cascades into $a0. Source order cannot swap sum/j without
    disturbing bp. rejected/inner-init-reorder-bp-cascade.c
  * block-local sum split (acc inner + sum=acc outer, hypothesis #2): score 8,
    79 insns — GCC coalesces acc->sum, byte-identical to baseline. Hypothesis #2
    DEAD. rejected/sum-split-acc-coalesced.c
- **Region B quick check:** swapping the ap/a2p pre-loop init order = score 8
  (GCC normalises the two base-copies). The LICM range constants (0x80000000,
  0x1FFFFF) are hard literals with no semantic tie to base, so no non-cheat lever
  makes them materialise before the pointer moves (a named constant-holder is the
  already-rejected magic_base cheat). Region B remains a context-dependent sched tie.
- **Net:** structural manual levers on both regions are exhausted for direct
  approaches — every one either cascades bp/a0 (worse) or coalesces to baseline.
  The remaining path is the directed permuter (real full-file build) sweeping the
  LUID cross-product that manual source order cannot reach surgically.

- [s2] Full-file greg proves the 8 diffs = Region A sum/j $a0<->$a1 swap (all other regs match target: base=t1, i=a2, chkptr=t0, offset=a3, bp=v1) + Region B const/move scheduling order.

- [s2] Register hand-out order for the inner-loop caller-saved class/range is v1 -> a0 -> a1 (1st/2nd/3rd qty allocated). Target needs qty_order 78(bp) 77(sum) 79(j) i.e. priority bp>sum>j; baseline is bp>j>sum.

- [s2] BB2_QTY_DEBUG (tmp/gccdbg/cc1) priority: j range4 refs4 => 20000; sum range6 refs4 => 13333. sum outlives the loop (post-loop beq) so it is genuinely lower priority; refs=4 for both are fixed by the target loop shape.

- [s2] A standalone single-function TU does NOT reproduce the real allocation (standalone sum=$a2 vs real $a1) - the sum/j tie is a whole-function LUID/context interaction, not isolable, so manual source levers cascade and only the real full-file build (permuter) can sweep it.

- [s2] chk-hoist=17, init-reorder bp;sum;j / sum;bp;j =13, sum-split(acc coalesced)=8-nochange, Region B init-swap=8-nochange. All structural manual levers on both regions exhausted for direct approaches.
