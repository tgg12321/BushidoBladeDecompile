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

## s3 (structural, 2026-07-22) — Region A range-mechanism CONFIRMED by measurement; zero-cost flip unavailable
- Confirmed floor 8 (candidate applied). Modality: structural.
- **`offset += j` (tail) => score 7 and FLIPS Region A completely.** qty_order among
  the inner pseudos: baseline `78 79 77` (bp=v1, j=a0, sum=a1) -> with offset+=j
  `78 77 79` (bp=v1, sum=a0, j=a1) == TARGET ORDER. dispositions confirm 77 in 4(a0=sum),
  78 in 3(v1=bp), 79 in 5(a1=j). This is the FIRST measured flip of the sum/j swap.
  Mechanism CONFIRMED: at loop exit j==0x24 so offset+=j is value-identical to +=0x24,
  but j now lives past the loop into the tail addu, raising j's live_length and dropping
  its global-allocno priority below sum's (was j 20000 > sum 13333). sum then wins a0.
- **offset+=j is NOT the match** (two independent reasons): (1) emits `addu $a3,$a3,$a1`
  vs target's literal `addiu $a3,$a3,0x24` -> can never byte-match that instruction, so
  cannot reach 0; (2) cheat-by-spelling (no-new-park-categories test #4) — value is
  identical to the literal, the only reason to spell it `+= j` is to steer RA via j's
  range. Rejected. Kept as EVIDENCE: Region A's swap is a pure j-vs-sum live-range/priority
  effect. rejected/offset-plus-j-flips-regionA-but-cheat.c
- **Zero-cost context perturbations both KILLED** (goal: flip A without the offset instr cost):
  * tail reorder `offset+=0x24; chkptr++; i++;` => score 10 (sched1 tie cascade).
  * index-based chk (drop chkptr walking-pointer IV, `((s32*)base)[i+0x1B]`) => score 14
    (forces base+i*4 recompute, diverges from target's matched `addiu $t0,$t0,4` walker).
    Confirms outer regs t1/a2/t0/a3 are load-bearing — any outer IV restructure cascades worse.
    rejected/tail-reorder-and-index-chk.c
- **Region B has no non-cheat structural lever** (re-confirmed): the two range-check constants
  (0x80000000, 0x1FFFFF) are LICM-hoisted from inside the k-loop; source order cannot place
  them before the ap/a2p moves because they are only referenced inside the loop. The only
  form that places them earlier is the already-rejected magic_base constant-holder (cheat).
- **KEY IMPLICATION for the permuter axis:** target's sum has the SAME isolated inner-loop
  range as build's (in both, `lw chk; nop; beq` sits between loop-exit and sum's death). So
  target lifts sum's priority WITHOUT lengthening j and WITHOUT shortening sum's isolated
  range — it must do so via the whole-function conflict graph / absolute LUID numbering. A
  match therefore needs the directed permuter's whole-function LUID sweep (frontier item 1),
  NOT a manual inner/outer source lever (all now measured dead). offset+=j proves the target
  register order is reachable; the permuter's job is to reach it with the literal offset intact.

- [s3] offset+=j => score 7, qty_order 78 77 79 (sum=a0,j=a1 == target): FIRST measured Region A flip; confirms it is a pure j-live-range/priority effect. Not match-viable (addu vs target addiu literal) + RA-motivated cheat -> rejected, kept as evidence.
- [s3] Zero-cost flips KILLED: tail-reorder offset-first=10, index-chk drop-chkptr-IV=14. Outer regs t1/a2/t0/a3 load-bearing; no zero-cost manual flip of Region A exists.
- [s3] Target's sum has identical isolated inner-loop range to build's (lw chk;nop;beq between loop-exit and sum death in both) => the sum>j priority in target comes from whole-function conflict-graph/LUID context, not the isolated range. Match requires the directed permuter LUID sweep, not a manual source lever.

- [s3] offset += j => sandbox score 7 (from floor 8): FIRST measured flip of Region A, qty_order 78 79 77 -> 78 77 79, dispositions sum=$a0/j=$a1/bp=$v1 == target. Confirms Region A is a pure j-vs-sum live-range/priority effect.

- [s3] offset += j is NOT match-viable: emits `addu $a3,$a3,$a1` where target has literal `addiu $a3,$a3,0x24`, and its only motivation is RA range-steering (cheat-by-spelling). Correctly rejected, retained as mechanism evidence.

- [s3] Zero-cost manual flips of Region A do not exist: tail-reorder offset-first=10, index-based chk (drop chkptr IV)=14. Outer regs t1/a2/t0/a3 are load-bearing; any outer IV restructure cascades worse.

- [s3] Target's sum has the SAME isolated inner-loop range as build's (in both, `lw chk; nop; beq` sits between loop-exit and sum's death), so target's sum>j priority comes from the whole-function conflict graph / absolute LUID numbering, not the isolated range. This is why no manual source lever isolates the flip and only the directed permuter can reach it.

- [s3] Region B: constants are LICM-controlled and only referenceable inside the k-loop; source order cannot place them before the ap/a2p moves without the rejected constant-holder cheat.

## s4 (permuter, 2026-07-22) — FLOOR 8 -> 6: do-while(0) on sum=0 solves Region A
- **Directed permuter on the REAL full-TU basin.** Built workspace tmp/grind/damage_DebugDisp/s4/ws:
  base.c = cpp(src/code6cac_c_mid.c) (full TU, real types), compile.sh = honest pipeline
  (cc1|prologue_fix|maspsx|align-sed|multu_pad, NO regfix/asmfix) extracting the
  damage_DebugDisp region, target.o from asm/funcs (prelude minus `.set gp=64`).
  Validated: base reproduces the EXACT sandbox diff (79 insns, Region A a0/a1 swap +
  Region B move/const order). This basin is CORRECT, unlike the s2 hand-built standalone
  (which gave sum=$a2) — the full preprocessed TU carries the real extern types/context.
- **KEY FIND (output-130-1, permuter-score 130 vs base 150): `do { sum = 0; } while (0);`
  SOLVES Region A.** The do-while(0) wrapper on the inner accumulator init delays sum's
  def LUID so sum's global-allocno priority rises above j's; sum takes $a0 == target,
  WITHOUT lengthening j (unlike the rejected offset+=j). Applied to src -> **sandbox
  --disable all = 6** (from floor 8). do-while(0)-for-any-codegen-effect is SANCTIONED
  per the FINAL 2026-07-06 do-while-zero-exception ruling (not a cheat). New candidate.c.
- **Remaining gap = 6 objdump diffs:** (A') a NEW 2-insn Region-A preheader reorder the
  do-while(0) introduced — target orders `addu $v1,$t1,$a3` (bp) before `move $a1,$zero`
  (sum), build swaps them; (B) the 4-insn Region B moves-vs-consts order (unchanged).
- **Region B has NO clean permuter lever (KILLED in the floor-6 chassis).** Reseeded a
  fresh campaign from the floor-6 (do-while0) base (ws6). Every sub-130 find was invalid:
  * output-75-1 (score 75): `chkptr = a2p` alias, but chkptr is s32* -> chkptr+0xD0 =
    a2p+0x340 (byte). SEMANTICALLY BROKEN (`lhu 832(a2)` vs target `208`). Wrong-pointer-
    type-offset trap. rejected/regionB-chkptr-alias-wrong-offset.c
  * output-70-1 (score 70): `base = a2p` alias (u8*, offset CORRECT) but emits 89 insns
    (+10 load-delay nops) — objectively longer, not a byte-match path.
    rejected/regionB-base-alias-nop-bloat.c
  * output-90-1 (score 90, from the floor-8 ws campaign): `src = a2p` (u8*, correct)
    reduced Region B 4->1 diff but lacked do-while(0) so Region A unsolved; also a
    dead-local-reuse aliasing that needs cheat-vetting and still short of match.
  Region B's moves-vs-consts LICM tie only "flips" via wrong-type offset aliasing (broken)
  or nop-bloat — no clean spelling reaches it in this basin. Confirms the ledger's s1-s3
  read that Region B is a context-dependent scheduling tie with no non-cheat source lever.

- [s4] Full-TU permuter basin (base.c = cpp of the real file) reproduces the exact sandbox allocation (Region A a0/a1 swap + Region B order); the s2 hand-built standalone did NOT — real extern types/context are load-bearing for the workspace to be valid.
- [s4] `do { sum = 0; } while (0);` (do-while(0), sanctioned for any codegen effect) lowers sandbox --disable all from 8 to 6 by flipping Region A's sum/j $a0<->$a1 swap: it delays sum's def LUID, raising sum's allocno priority above j's WITHOUT lengthening j. Applied to src; new floor 6. First clean Region A solve (offset+=j was value-equal but a cheat).
- [s4] Region B (4-insn moves-vs-consts LICM order) has NO clean permuter lever in the floor-6 chassis: permuter sub-130 finds are all invalid — chkptr(s32*)=a2p alias gives wrong offset 0x340 (broken), base(u8*)=a2p alias bloats to 89 insns (+10 nops). Confirms Region B is a context-dependent scheduling tie with no non-cheat source lever.
- [s4] do-while(0) introduced a NEW 2-insn Region-A preheader scheduling tie: target emits `addu $v1,$t1,$a3` (bp=base+offset) before `move $a1,$zero` (sum=0); build swaps them. Permuter did not crack it cleanly in-window.

- [s4] do { sum = 0; } while (0); lowers sandbox --disable all from 8 to 6 (applied in src/code6cac_c_mid.c) by flipping Region A's sum/j $a0<->$a1 swap; sanctioned do-while(0) RA-weighting, not a cheat.

- [s4] The full-TU permuter basin (base.c = cpp of the real file) is the valid chassis; a hand-built standalone (s2) does NOT reproduce the allocation because real extern types/context are load-bearing.

- [s4] Region A is fully solved by do-while(0); the residual is a NEW 2-insn preheader scheduling tie the do-while(0) introduced: target emits `addu $v1,$t1,$a3` (bp=base+offset) before `move $a1,$zero` (sum=0), build swaps them.

- [s4] Region B (4-insn moves-vs-consts) has no clean permuter lever from the floor-6 chassis: chkptr(s32*)=a2p alias gives wrong offset 0x340 (broken); base(u8*)=a2p alias bloats to 89 insns (+10 nops). Both rejected/banked.

- [s4] output-90-1 (score 90, floor-8 ws campaign): `src=a2p` (u8*, correct offset) cut Region B 4->1 diff but lacked do-while(0) so Region A was unsolved and it is a dead-local-reuse alias needing cheat-vetting; still short of match.

- [s5] Floor 6 reconfirmed this session (candidate.c applied to src, sandbox --disable all = 6).

- [s5] PERM_LINESWAP space over the inner-loop preheader is EXHAUSTED: all 6 statement orderings measured; (j=0; bp=base+offset; do{sum=0}while(0)) is a strict local optimum at 6, every neighbor 10-13. The Region A' preheader tie is a sched1 whole-function LUID effect, NOT reachable by preheader statement reordering.

- [s5] Fresh-seed floor-6 permuter campaign (20,895 iters, structurally distinct seed from s4 ws6) yields ONLY: score-130 laterals (output-130-1/2/3) and the Region B dead-alias family (75/90/70/100), every one a cheat-by-spelling or semantically broken, none reaching floor<6.

- [s5] output-75-1 (chkptr=a2p) is BROKEN not just a cheat: chkptr is s32* so chkptr+0xD0 addresses a2p+0x340 bytes vs target's a2p+0xD0 (lhu 832 vs 208). Confirms the wrong-pointer-type-offset trap for any Region B pointer-alias through a s32* handle.

- [s5] Region B has no clean permuter lever from the floor-6 chassis (reconfirmed twice now, s4 + s5 fresh seed): the LICM range constants (0x80000000, 0x1FFFFF) are only referenceable inside the k-loop, so LICM controls their placement; the only source forms that flip the moves-vs-consts order are dead-alias cheats or nop-bloat. Region B needs a genuine control-flow change, which the permuter's local mutations cannot synthesize.

## s6 (forensics, 2026-07-22) — exact pass+decision named for BOTH residuals; do-while(0) coupling proven
- Floor 6 reconfirmed (candidate applied to src, sandbox --disable all = 6). cc1 -da full
  RTL dump series saved: tmp/grind/damage_DebugDisp/s6/dumps/dd.c.{rtl..dbr} (13 passes),
  region extracts dd_greg/dd_sched/dd_loop.txt, ops diff build_ops.txt vs tgt_ops.txt.
- **6 residual objdump insns = 2 code diffs** (branch/reloc address deltas are ws-offset noise):
  * Region A' preheader EMIT order: build `j=0; bp=addu; sum=0` vs target `sum=0; bp=addu; j=0`.
  * Region B 2nd-loop preheader: build `ap;a2p;C0;C1;C2` vs target `C0;C1;C2;ap;a2p`
    (C=LICM invariants lui 0x8000 / lui 0x1f / ori 0xffff).
- **Region A' PASS+decision NAMED:** sched.c sched1 `schedule_block`/`rank_for_schedule`.
  Preheader BB1 insns 27(j:a1) 30(bp:v1) 36(sum:a0) all INSN_PRIORITY=1; ready-list init={36}
  (sum=0 is the sole ref_count=0 leaf at block bottom -> scheduled to last slot). Remaining
  {27,30} tie on priority -> `rank_for_schedule` FINAL tiebreak = `INSN_LUID(tmp)-INSN_LUID(tmp2)`
  (sched.c:54-57) -> emits in LUID order 27,30 -> net j,bp,sum. Target = sum,bp,j.
- **do-while(0) coupling PROVEN by A/B greg (ab_greg.sh, identical TU except the wrapper):**
  plain `sum=0;`     -> alloc order `100 78 79 77` -> 77(sum)=a1, 79(j)=a0 (floor-8 swap).
  `do{sum=0}while(0)`-> alloc order `100 78 77 79` -> 77(sum)=a0, 79(j)=a1 (TARGET).
  The wrapper is the ONLY thing that swaps 77<->79 in global.c's allocation order. PASS:
  global.c `allocno_compare`, priority = floor_log2(n_refs)*n_refs*size/live_length*10000.
  For 77/79 n_refs=4,size=1 equal; only live_length differs. do-while(0) relocates sum's def
  to insn 36 (block bottom, after NOTE_INSN_LOOP_BEG 285) -> SHORTENS pseudo-77 live_length ->
  priority > j(79) -> sum allocated first -> a0. **IRREDUCIBLE COUPLING:** the same def-relocation
  makes insn 36 the HIGHEST-LUID preheader insn, so sched1's LUID tiebreak emits sum=0 LAST (A').
  Target raises sum priority WITHOUT a late def (sum=0 first, still a0) -> unreachable on the
  do-while(0) chassis by def position; s5's 6-ordering exhaustion is EXPLAINED (sum's a0 requires
  its def at block-bottom = highest LUID = the slot sched1 emits last).
- **Region B PASS+decision NAMED:** loop.c `move_movables` hoists the two invariants via
  `emit_insn_before(newpat, loop_start)` (loop.c:1652) = immediately before NOTE_INSN_LOOP_BEG,
  i.e. AFTER the pre-existing ap/a2p preheader moves. Hoisted consts thus ALWAYS get higher LUID
  than the moves; sched1's LUID tiebreak (all prio=1) emits moves-first. Target = constants-first
  => target's invariants had LOWER LUID (positioned before the ptr init). Consts are referenced
  ONLY inside the k-loop, so the only source form placing them earlier is a pre-loop reference =
  the rejected constant-holder cheat family (magic_base / regionB-*-alias). Confirmed dead s1-s6.

- [s6] Region A' NAMED: sched.c `rank_for_schedule` INSN_LUID tiebreak (sched.c:54-57) over preheader BB1 (all prio=1); sum=0 is block-bottom sole leaf -> emits last; target emits it first.
- [s6] do-while(0) A/B greg proof: it is the SOLE cause of the 77(sum)<->79(j) swap in global.c's allocation order (`100 78 79 77` plain -> `100 78 77 79` do-while0). Mechanism = global.c `allocno_compare` live_length priority (n_refs=4/size=1 equal for both); the wrapper shortens sum's live_length by relocating its def to the preheader block bottom.
- [s6] IRREDUCIBLE COUPLING: the do-while(0)'s RA win (short live_length -> a0) and its scheduling loss (block-bottom def -> highest LUID -> sched1 emits sum=0 last) share ONE cause: sum's def LUID. So A' cannot be flipped on the do-while(0) chassis by def position (s5's 6-ordering exhaustion explained). Target achieves sum=a0 with sum=0 first -> needs a whole-function priority lift with sum's def at LOW LUID.
- [s6] Region B NAMED: loop.c `move_movables` `emit_insn_before(loop_start)` (loop.c:1652) inserts hoisted invariants at preheader END (after ap/a2p moves) -> higher LUID -> sched1 LUID tiebreak emits moves-first. Target needs consts at lower LUID = a pre-loop reference = constant-holder cheat. Dead.

- [s6] Floor 6 reconfirmed (candidate applied, sandbox --disable all = 6); src reverted to clean HEAD after measurement.

- [s6] 6 residual objdump insns = exactly 2 code diffs (Region A' preheader order + Region B LICM/move order); all other objdump deltas are ws-offset branch/reloc address noise.

- [s6] Region A' PASS named: sched.c sched1 rank_for_schedule, final tiebreak = INSN_LUID (sched.c:54-57); sum=0 is the block-bottom sole leaf so it emits last.

- [s6] do-while(0) A/B greg proof: it is the ONLY cause of the 77(sum)<->79(j) allocation-order swap (plain 100 78 79 77 -> do-while0 100 78 77 79). Cause = global.c allocno_compare live_length priority; n_refs=4/size=1 equal for both, so def-relocation shortening sum's live_length is the lever.

- [s6] Irreducible coupling: sum's a0 (short live_length) and sum=0-emits-last (highest LUID) both derive from sum's def LUID, so Region A' is unreachable on the do-while(0) chassis by def position (explains s5's exhaustive 6-ordering result).

- [s6] Region B PASS named: loop.c move_movables emit_insn_before(loop_start) (loop.c:1652) places hoisted invariants at preheader END -> higher LUID than ap/a2p moves -> sched1 LUID order emits moves-first; target needs consts at lower LUID = a pre-loop reference = constant-holder cheat.

- [s6] No OWNER-ESCALATION entry for damage_DebugDisp exists in docs/grind/decisions.md (owner-gated not available).

## s7 (forensics, 2026-07-22) — CORRECTION: Region A is a reg_n_refs loop-depth-weight tie, NOT live_length
- Floor 6 reconfirmed (candidate applied, sandbox --disable all = 6; src reverted to HEAD).
  Dumps: tmp/grind/damage_DebugDisp/s7/dumps/{dd.c,dd_plain.c}.{lreg,greg}; FORENSIC_SUMMARY_s7.md.
- **The .lreg `dump_flow_info` (flow.c:2890) prints reg_n_refs + reg_live_length verbatim.** Measured:
  * do-while0: 77(sum)=11 refs / LL 9 ; 79(j)=11 refs / LL 9 ; 78(bp)=11/8.
  * plain:     77(sum)=**10** refs / LL 9 ; 79(j)=11 refs / LL 9 ; 78(bp)=11/8.
  **live_length is 9 for BOTH 77 and 79 on BOTH chassis — it never moves.** s6's "do-while(0)
  shortens sum's live_length" is FALSE. The ONLY change is reg_n_refs(sum): 10 -> 11.
- **Mechanism = loop-depth-weighted refs:** `reg_n_refs[regno] += loop_depth;` (flow.c:2081/2329/
  2515/2725). plain `sum=0` is in the outer-do preheader at loop_depth 1 (contributes 1) -> sum=10.
  The `do{sum=0}while(0)` emits NOTE_INSN_LOOP_BEG/END around sum=0, raising that block's loop_depth
  to 2 (contributes 2, +1) -> sum=11 = ties j(11).
- **Tie broken by allocno number:** global.c allocno_compare pri = floor_log2(n_refs)*n_refs/LL*1e4*size;
  floor_log2(10)=floor_log2(11)=3, size1, LL9 -> plain sum 33333 < j 36666 (j=a0); do-while0 sum 36666
  == j 36666 -> TIE -> `return *v1 - *v2` (allocno diff); allocnos ascend by pseudo (global.c:384-397),
  sum pseudo 77 < j 79 -> sum sorts first -> sum=$a0. greg alloc order plain `100 78 79 77` /
  do-while0 `100 78 77 79`. => RA win is an EXACT tie decided by sum's lower pseudo number (fragile).
- **Coupling recharacterized (supersedes s6):** the SAME NOTE_INSN_LOOP_BEG (a) raises loop_depth at
  sum=0 (+1 ref -> tie -> a0) AND (b) forces sum=0 to the bracketed block bottom = highest preheader
  LUID -> sched1 rank_for_schedule LUID tiebreak emits sum=0 LAST (Region A'). s5's 6-ordering
  exhaustion explained: +1 ref present in all 6 (do-while always brackets) so sum=a0 in all 6; only the
  schedule differs; pulling sum=0 earlier cascades (dw-first=10).
- **New axes measured dead:** (1) add +1 to sum weight without the loop bracket = a synthetic extra
  depth>=1 sum ref = dead code = cheat; `if(1){}` emits no loop note (adds 0). (2) drop j 11->10:
  j=0 must reset each outer iteration (can't hoist to depth 0), j++/j<0x24 are minimal depth-2 refs.
  Both blocked.
- **Sibling corroboration:** func_80037F40 (same file, COMPLETED-C, src ~line 190) is a near-twin
  (checksum accumulate + for(;;) CopyBlock fence + k-loop) whose accumulate runs ONCE before the outer
  loop (depth 0/1) -> matched pure C with NO do-while(0). Confirms the depth-weighting mechanism across
  functions; damage cannot adopt the shape (its accumulate MUST be inside the outer loop, re-checksums
  each of 3 candidate blocks).

- [s7] CORRECTION of s6: Region A's sum a1->a0 flip is a reg_n_refs LOOP-DEPTH-WEIGHT effect, not live_length. .lreg dump_flow_info: 77(sum) and 79(j) both have live_length=9 on plain AND do-while0; only reg_n_refs(sum) changes 10->11. `reg_n_refs += loop_depth` (flow.c) — the do-while(0)'s NOTE_INSN_LOOP_BEG raises sum=0's block depth 1->2, +1 weighted ref.
- [s7] do-while0 makes sum n_refs==j n_refs==11 => global.c allocno_compare priority TIE (both 36666) => tiebreak `return *v1-*v2` (allocno number); allocnos ascend by pseudo (global.c:384-397) so sum(77)<j(79) wins $a0. The RA win is an exact priority tie decided by sum's lower pseudo number — fragile to renumbering.
- [s7] Coupling recharacterized: ONE NOTE_INSN_LOOP_BEG both (a) raises loop_depth for the +1 ref (RA win) and (b) forces sum=0 to block-bottom/highest-LUID (sched1 emits it last = A'). Explains s5's 6-ordering exhaustion mechanistically (sum=a0 in all 6; only schedule differs).
- [s7] New Region-A axes measured dead: add sum weight without a loop-note bracket = synthetic dead ref (cheat) / if(1){} adds 0 note; drop j 11->10 blocked (j=0 must reset per outer iter, j++/j<0x24 minimal). Sibling func_80037F40 (singly-nested accumulate) matched with NO do-while(0) — cross-function confirmation; damage can't adopt its shallower nesting (semantics).

- [s7] Floor 6 reconfirmed: candidate.c applied to src, `sandbox damage_DebugDisp --disable all` = 6; src then reverted to clean HEAD.

- [s7] .lreg dump_flow_info (flow.c:2890): do-while0 chassis 77(sum)=11 refs/LL9, 79(j)=11 refs/LL9, 78(bp)=11/8; plain chassis 77(sum)=10 refs/LL9, 79(j)=11 refs/LL9. live_length is 9 for sum and j on BOTH chassis.

- [s7] reg_n_refs is loop-depth-weighted: `reg_n_refs[regno] += loop_depth` (flow.c:2081,2329,2515,2725). The do-while(0)'s NOTE_INSN_LOOP_BEG raises sum=0's block loop_depth 1->2, adding exactly +1 weighted ref (sum 10->11).

- [s7] global.c allocno_compare pri = floor_log2(n_refs)*n_refs/live_length*10000*size; floor_log2(10)=floor_log2(11)=3. plain sum=33333 < j=36666 (j=a0); do-while0 sum=36666 == j=36666 -> exact TIE.

- [s7] At a tie allocno_compare returns `*v1 - *v2` (allocno-number diff); allocnos are assigned ascending by pseudo number (global.c:384-397), so sum pseudo 77 < j pseudo 79 -> sum sorts first -> $a0. greg alloc order confirms: plain `100 78 79 77`, do-while0 `100 78 77 79`.

- [s7] Coupling (supersedes s6): the SINGLE NOTE_INSN_LOOP_BEG both (a) raises loop_depth for the +1 ref (RA win) and (b) forces sum=0 to the bracketed block bottom = highest preheader LUID -> sched1 rank_for_schedule LUID tiebreak emits sum=0 LAST (Region A'). Explains s5's 6-ordering exhaustion (sum=a0 in all 6; only schedule differs).

- [s7] Sibling func_80037F40 (same file, COMPLETED-C, src ~line 190): near-twin (checksum accumulate + for(;;) CopyBlock fence + k-loop) whose accumulate runs ONCE before the outer loop (depth 0/1) -> matched pure C with NO do-while(0). Cross-function confirmation of the depth-weighting mechanism; damage cannot adopt the shape (its accumulate must be inside the outer loop, re-checksumming each of 3 candidate blocks).

- [s7] Region B unchanged from s6: loop.c move_movables emit_insn_before(loop_start) places hoisted invariants at preheader END (higher LUID than ap/a2p moves) -> sched1 emits moves-first; target needs consts at lower LUID = a pre-loop reference = constant-holder cheat. Dead.

## s8 (rederive, 2026-07-22) — m2c/goto shape score-equivalent; tail-bracket decoupler KILLED; Kengo dead
- Floor 6 reconfirmed via the goto shape + do-while(0) (sandbox --disable all = 6); src reverted to clean HEAD.
- **Fresh m2c decompile** of asm/funcs/damage_DebugDisp.s reconstructs the outer search loop as a
  NEGATED compare with increments INSIDE the if and a `goto` loop-back (no `break`):
  `loop_1:{sum=0;bp;j;accum} if(sum!=chk){chkptr++;i++;offset+=0x24;if(i<3)goto loop_1;}`.
  Genuinely different outer BB layout than candidate's `do{...if(sum==chk)break;incr}while(i<3)`.
  * goto shape, PLAIN sum=0 -> score **8** (Region A UNSOLVED, identical to plain break form).
  * goto shape + do{sum=0}while(0) -> score **6**, objdump residual IDENTICAL to candidate
    (Region A solved: inner loop sum=$a0/j=$a1; residual = A' preheader order j,bp,sum vs target
    sum,bp,j + Region B). The outer control-flow shape does NOT move the preheader LUID ordering.
  KILL: rederiving the outer loop via m2c is score/residual-equivalent; A' emit order is set by the
  preheader inits' own LUIDs (do-while0 def relocation), not the surrounding CFG.
  rejected/m2c-goto-negated-compare-equivalent.c
- **Region A' decoupler probe (s7 frontier item 1): raise sum's weighted refs to 11 via a NON-def
  sum reference (the post-loop `sum==chk` compare) inside a do-while(0), keeping sum=0 plain/low-LUID.**
  Two spellings, BOTH score **12** (from floor 6):
  * `do{ if(sum!=chk){incr; goto} }while(0)` — the bracket raises loop_depth for chkptr++/i++/offset+=
    too, cascading the matched outer IVs (i=a2, chkptr=t0, offset=a3).
  * `do{ matched=(sum==chk); }while(0); if(!matched){incr}` — bracket only the compare via an
    intermediate; still 12 (boolean materialization + tail lw/beq reschedule).
  KILL: the tail region (compare + outer IVs) cannot absorb a loop-note bracket without cascading the
  outer allocation. The +1 sum ref for Region A can ONLY come from sum's own def (sanctioned
  do-while0 on sum=0) or the inner accumulate — both re-introduce the A'/A coupling. Reconfirms
  s6/s7 irreducible coupling from a fresh structural angle.
  rejected/regionA-tail-bracket-cascades-outer-iv.c
- **Kengo transplant DEAD (name-only match).** kengo_matches.csv maps damage_DebugDisp -> is_damage_calc
  (name-unique, 0.00 confidence). kengo_ref.py dump: Kengo's `damage_DebugDisp` @0x11fd88 is an
  UNRELATED font/matrix debug-display function (fnt_locate/fnt_print/conv_matrix_rotation/atan2), NOT a
  checksum validator. No structural transplant source. The BB2 function is really an is_damage_calc
  data-block checksum validator.
- **decomp.me corpus scrape UNAVAILABLE** in this env: tools/decomp_me_scrape.py needs curl_cffi
  (not installed) + network. Avenue environment-blocked, not measured.

- [s8] m2c goto/negated-compare outer-loop shape: PLAIN=8, +do-while0=6 with residual IDENTICAL to candidate; outer CFG structure does not move the A' preheader LUID order. Rederive of the outer loop is score-equivalent.
- [s8] Region A' decoupler KILLED: raising sum's weighted refs via a do-while0 bracket on the tail compare (both `if`-wrapped and `matched`-intermediate spellings) scores 12 — the bracket cascades the matched outer IVs (i/chkptr/offset at depth 1). The +1 sum ref cannot be sourced outside sum's own def/accumulate; s6/s7 coupling reconfirmed structurally.
- [s8] Kengo transplant dead: name-only match (is_damage_calc, 0.00 conf); Kengo's damage_DebugDisp is an unrelated font/debug-display function. decomp.me scrape env-blocked (no curl_cffi/network).

- [s8] Floor 6 reconfirmed this session: goto-shape + do{sum=0}while(0) -> sandbox --disable all = 6; src reverted to clean HEAD after measurement.

- [s8] m2c fresh decompile reconstructs the outer search loop as a negated compare with increments INSIDE the if and a goto loop-back (no break) - a genuinely different BB layout from candidate.c's do-while+break.

- [s8] goto shape with PLAIN sum=0 scores 8 (Region A sum/j swap unsolved); the inner loop is byte-identical target vs build, so control-flow reshape cannot change the inner-loop sum/j allocation.

- [s8] goto shape + do-while0 scores 6 with objdump residual IDENTICAL to candidate (A' preheader order + Region B); the outer control-flow structure does not move sched1's preheader-init LUID ordering.

- [s8] Region A' tail-bracket decoupler (both if-wrapped and matched-intermediate spellings) scores 12: bracketing the tail raises loop_depth for the matched outer IVs (i/chkptr/offset at depth 1) and cascades the outer allocation. The +1 sum ref for Region A cannot be sourced outside sum's own def/accumulate - s6/s7 A'/A coupling reconfirmed structurally.

- [s8] Kengo transplant dead: name-only match; Kengo's damage_DebugDisp is an unrelated font/debug-display function. decomp.me corpus scrape env-blocked (curl_cffi not installed, no network).

- [s8] Residual unchanged from s6/s7: Region A' = sched1 rank_for_schedule INSN_LUID tiebreak fed by the do-while0 def relocation of sum=0 to preheader block-bottom; Region B = loop.c move_movables emit_insn_before(loop_start) hoisting the range constants AFTER the ap/a2p moves.

## s9 (rederive, 2026-07-22) — fresh m2c reproduces candidate; inner-body reorder + accumulate-peel KILLED
- Floor 6 reconfirmed (candidate applied: for(;;) fence + do{sum=0}while(0); sandbox --disable all = 6). src reverted to clean HEAD after measurement.
- **Fresh m2c decompile executed** (tools/m2c/m2c.py --target mips-gcc-c; artifact tmp/grind/damage_DebugDisp/s9/m2c_out.c). Reconstructs the SAME structure as candidate: outer negated-compare+goto (== s8 m2c, equivalent), inner byte-accumulate do-while, and Region B `(u32)(ptr-0x80000000)<=0x1FFFFF` range check with ap/a2p as PLAIN preloop moves (`var_a0_2=arg0; var_a2_3=var_a0_2;`) — NO alternative Region-B source structure exists. The consts-before-moves in target therefore imply the ORIGINAL referenced the range constants pre-loop (constant-holder), which is a cheat for us; m2c offers no clean Region-B lever. Reconfirms s1-s8.
- **NEW measurement — m2c inner-body order `j++; sum+=*bp; bp++` on the floor-6 do-while0 chassis = score 8** (from 6). m2c reconstructs the inner loop as `var_a1+=1; var_a0+=*var_v1; var_v1+=1` (j++ first). Applied on the do-while0 chassis it REGRESSES: the reorder shifts sum's def/use LUID and UNDOES the do-while0 sum=$a0 RA win (Region A reopens). Confirms candidate's `sum+=*bp; bp++; j++` order is LOAD-BEARING for the do-while0 win — inner-body order is not a free lever here (s5 only exhausted PREHEADER order; s1 tested body order only on the plain floor-8 chassis). rejected/m2c-inner-order-jpp-first.c
- **NEW measurement — accumulate-peel `sum=*bp; bp++; j=1; do{...}while(j<0x24)` = score 12** (from 6). Peeling the first iteration to give sum a real, non-bracketed depth-1 ref (value-identical byte sum) emits an extra pre-loop lbu+addu and changes j's init/trip-count, diverging from target's clean 0x24-iter do-while. The byte-neutral +1 sum ref is unavailable by peeling — reconfirms (fresh rederive angle) that sum's +1 weighted ref only comes from its own def bracket (do-while0, which couples A') per s6/s7. rejected/peel-first-accumulate-iteration.c
- **Sibling func_80037F40 re-read (src line 190):** its k-loop writes ZERO with NO range-check constants (no 0x80000000/0x1FFFFF), so it does not exhibit Region B's LICM-const tie; and its accumulate runs once pre-outer-loop (shallower nesting) so it does not inform Region A. Confirms s7 — no transplant value for either residual.

- [s9] Fresh m2c (tools/m2c) reproduces the candidate structure exactly (outer goto == s8, inner byte do-while, Region B plain preloop moves + identical range check). No new rederive lever for either residual; Region B consts-before-moves implies an original pre-loop constant reference = constant-holder cheat for us.
- [s9] m2c inner-body order `j++; sum+=*bp; bp++` on the floor-6 do-while0 chassis = 8: the reorder undoes the do-while0 sum=$a0 RA win. Candidate's `sum+=*bp; bp++; j++` order is load-bearing. KILLED.
- [s9] Accumulate-peel (sum=*bp; j=1; loop 0x23) = 12: extra pre-loop insns + altered trip count diverge from target; a byte-neutral +1 sum ref is unavailable by peeling. Reconfirms s6/s7 do-while0-coupling from the rederive angle. KILLED.

- [s9] Floor 6 reconfirmed: candidate (for(;;) fence + do{sum=0}while(0)) applied to src -> sandbox --disable all = 6; src reverted to clean HEAD after measurement.

- [s9] Fresh m2c (tools/m2c/m2c.py, artifact tmp/grind/damage_DebugDisp/s9/m2c_out.c) reproduces the candidate structure exactly: outer negated-compare+goto (== s8, equivalent), inner byte-accumulate do-while, Region B plain preloop moves + identical (u32)(ptr-0x80000000)<=0x1FFFFF range check. No new rederive lever for either residual.

- [s9] m2c inner-body order `j++; sum+=*bp; bp++` on the floor-6 do-while0 chassis = 8: the reorder undoes the do-while0 sum=$a0 RA win; candidate's `sum+=*bp; bp++; j++` order is load-bearing. (s5 exhausted only PREHEADER order; s1 tested body order only on the plain floor-8 chassis — this is the first floor-6-chassis body-order measurement.)

- [s9] Accumulate-peel `sum=*bp; bp++; j=1; do{...}while(j<0x24)` = 12: extra pre-loop lbu+addu + altered j init/trip-count diverge from target. A byte-neutral +1 sum ref is unavailable by peeling — reconfirms s6/s7 that sum's +1 weighted ref only comes from its own def bracket (do-while0, which couples Region A').

- [s9] Sibling func_80037F40 (src line 190) re-read: its k-loop writes ZERO with NO range-check constants (no 0x80000000/0x1FFFFF) so it does not exhibit Region B's LICM tie; its accumulate runs once pre-outer-loop (shallower nesting) so it does not inform Region A. No transplant value for either residual (confirms s7).

## s10 (synthesis, 2026-07-22) — FLOOR 6 -> 2: Region B CLOSED (refutes the 9-session constant-holder-cheat conclusion)
- Confirmed floor 6 (candidate applied). Modality: synthesis. New candidate.c = floor 2.
- **BREAKTHROUGH: Region B is NOT a constant-holder-cheat wall.** s1-s9 concluded the
  consts-before-moves order needed a pre-loop constant reference (magic_base cheat). FALSE.
  Rewriting the k-loop INDEX-BASED (`base + k*4` / `base + k*2`) instead of explicit
  `ap=base; a2p=base;` walking pointers removes the explicit preheader moves, so GCC
  strength-reduces the pointers into GIVS whose inits are created by strength_reduce
  (runs AFTER loop.c move_movables/LICM) -> the hoisted range constants (0x80000000,
  0x1FFFFF) emit FIRST (== target order). Pure C, no cheat.
- **The giv-init FOLD is coupled to k's biv-init structure:**
  * index form + two-path k=0 (decl + CopyBlock, OR if/else both branches): consts-first
    + CORRECT regs (ap=$a0, a2p=$a2, k=$a1) but giv-init BLOATED to `sll;addu` x2 (k not
    proven const-0 at the immediate preheader) -> score 6, build 81. No net gain.
  * index form + SINGLE dominating k=0 after merge: immediate const-0 biv init -> giv init
    FOLDS to plain `move a0,t1; move a2,a0` (target structure) BUT swaps k/a2p regs
    (k=$a2, a2p=$a1) — folding removes k's use in the giv-init sll, dropping k's ref
    priority so the a2p giv wins $a1. score 9. rejected/regionB-single-k0-after-merge-regswap.c
  * **index form + `do { k = 0; } while (0);` -> score 2, build 79, Region B BYTE-EXACT.**
    The do-while(0) (a) is the dominating const-0 init that folds the giv inits AND (b) its
    loop-depth-weighted reg_n_refs bump wins k the $a1 tiebreak over the a2p giv (== target
    k=$a1, a2p=$a2). This is the EXACT Region A do-while(0) mechanism applied to Region B's
    counter. Disasm: `lui t0; lui a3; ori a3; move a0,t1; move a2,a0; lw 0x78(a0); ...
    lhu 0xD0(a2)` == target .L800380F0. Both do-while(0)s FAKE-annotated.
- **Region A' (the sole score-2 residual) reconfirmed coupled on the NEW index-B chassis:**
  target inner preheader sum=0($a0), bp($v1), j=0($a1) = order sum,bp,j; build (do-while(0)
  relocates sum=0 to bracketed block-bottom = highest LUID) = j,bp,sum. sched1 rank_for_schedule
  INSN_LUID tiebreak emits block-bottom leaf last. Re-measured on the score-2 chassis:
  plain sum-first=9 (RA reopens, sum=$a1), do-while(0) sum-first=9, co-bracket {sum,bp}=21
  (bp cascade), index-accumulate (base[offset+j])=12 (offset outer-variant, no clean reduce).
  The A'/A coupling (s6/s7) HOLDS independent of Region B. The natural sum weighted-refs>=11
  WITHOUT a bracket (target's mechanism) remains the open question -> directed permuter on the
  score-2 chassis is the next probe (numbering changed vs the s4/s5 old-chassis campaigns).

- [s10] FLOOR 6 -> 2. Region B closed: index-based k-loop (base+k*4 / base+k*2) makes ap/a2p strength-reduced givs whose inits land AFTER the LICM range constants (move_movables runs before strength_reduce) -> consts-first == target. `do{k=0}while(0)` folds the giv inits to plain moves AND wins k the $a1 tiebreak over the a2p giv. REFUTES s1-s9 "Region B needs a constant-holder cheat".
- [s10] Region B fold/reg-alloc is coupled to k's biv init: two-path k=0 -> correct regs but bloated giv init (score 6); single merged k=0 -> folded but k/a2p reg swap (score 9); do-while(0) on k=0 -> folded AND correct regs (score 2, byte-exact). The do-while(0)'s loop-depth ref bump is what wins k=$a1 after the fold removes k's giv-init refs.
- [s10] Region A' remains the sole residual at score 2 and is coupled on the index-B chassis too (plain sum-first=9, do-while0 sum-first=9, co-bracket sum+bp=21, index-accumulate=12). The A'/A do-while(0) coupling (s6/s7) is independent of Region B. Next: directed permuter on the score-2 chassis for the natural sum-11th-ref that lets sum win $a0 with sum=0 emitted first.
- [s10] The index trick is Region-B-specific: it needs a loop-invariant base. It DIVERGES on the accumulate loop (base+offset is outer-variant) -> score 12. Target's accumulate uses an explicit bp=base+offset walker (byte-identical to candidate).

## s10 (synthesis pass / re-spawn, 2026-07-22) — floor 2 independently reconfirmed; A' localized to 2 preheader reg=0 moves; plain=4 (corrects the ledger's implied 9)
- **Floor 2 REVERIFIED on the committed index-B chassis** (candidate.c applied to src -> `sandbox --disable all` = 2, target 79 / build 79, 8 rules dropped, 34 cheat-asm stripped). Region B judge-PASS ruling (docs/grind/decisions.md 2026-07-22 06:02) confirmed in place. src reverted to clean HEAD after measurement.
- **NEW / CORRECTION: plain `sum=0` (in candidate position, no do-while0) on the index-B chassis = score 4, NOT 9.** The s10 breakthrough note's "plain sum-first=9" referred to MOVING sum=0 to first position; keeping sum=0 in-position but unbracketed gives 4. Per-function objdump (tmp/grind/damage_DebugDisp/s10/{plain,dw0}_ops.txt vs asm/funcs) proves why:
  * **plain**: preheader is BYTE-PERFECT — insn4 `addu a0,zero,zero`, insn5 `addu v1,t1,a3`, insn6 `addu a1,zero,zero` == target .L8003801C exactly. The only 4 diffs are the inner-loop a0<->a1 swap (j=$a0/sum=$a1 vs target sum=$a0/j=$a1): insn8 addiu, insn9 addu, insn10 sltiu, insn15 beq. Region A' is ABSENT in plain (sum=0 sits at low LUID -> emitted first).
  * **do-while0** (floor 2): inner loop now BYTE-IDENTICAL to target (sum=$a0/j=$a1 ✓, insns 8/9/10/15 match). Residual = exactly 2 preheader diffs — the emit-POSITION swap of the two `reg=0` moves: build insn4=`addu a1,zero,zero`(j=0)/insn6=`addu a0,zero,zero`(sum=0) vs target insn4=`addu a0,zero,zero`(sum=0)/insn6=`addu a1,zero,zero`(j=0). The bracket forces sum=0 to schedule at position 6 (block-bottom, highest LUID) via sched1 rank_for_schedule INSN_LUID tiebreak; target has it at position 4.
- **EXACT TRADE quantified:** do-while0 fixes the 4 inner swap diffs (−4) but introduces 2 preheader-order diffs (+2) => 4 - 4 + 2 = score 2. The A'/A coupling (s6/s7) is now visible at the byte level: the do-while0's +1 sum ref (fixes swap) and its block-bottom sum=0 def (breaks emit order) are one and the same relocation.
- **Target's own structure (from the .s):** target's inner accumulate is byte-identical to ours AND sum=$a0 with sum=0 emitted FIRST (unbracketed). Since the inner body, the sum=0 def, and the sum==chk compare are all byte-identical between our chassis and target, target's sum reaches weighted-refs>=11 (tie j=11) from a source that does NOT relocate sum=0's def. That non-bracket 11th ref is the sole open question; it is a whole-function loop_depth/numbering effect (frontier).
- **Sibling func_80037F40 re-read in full (src 190-239):** its checksum accumulates ONCE before the outer loop (depth 1) and is WRITTEN (`*(base2+0x6C)=checksum`), then reused across 3 outer iterations; damage RECOMPUTES sum INSIDE the outer loop (depth 2) and READS/compares it. That depth-2 recompute is the root of damage's extra-weight requirement — the sibling's shallower shape is semantically unavailable to damage. Confirms s7/s9 (no transplant value).
- artifacts: tmp/grind/damage_DebugDisp/s10/{extract_ops.py, plain_ops.txt, dw0_ops.txt, build_ops.txt}

- [s10-synth] Floor 2 independently reconfirmed on the committed index-B chassis (score 2, 79/79); Region B judge-PASS in place; src reverted to HEAD.
- [s10-synth] CORRECTS ledger: plain sum=0 in-position on index-B chassis = 4 (not 9). objdump proves plain's preheader is byte-perfect and the only 4 diffs are the inner a0<->a1 swap; Region A' is absent in plain.
- [s10-synth] A' localized to exactly 2 insns: the emit-position swap of `addu a1,zero,zero`(j=0) and `addu a0,zero,zero`(sum=0) preheader moves. do-while0's block-bottom sum=0 def forces sched1 to emit sum=0 at position 6; target has it at position 4. Trade: do-while0 = −4 inner swap +2 preheader order = score 2.
- [s10-synth] Target's inner body/sum-def/compare are byte-identical to ours yet target's sum wins $a0 with sum=0 emitted first => target's sum has weighted-refs>=11 from a NON-bracket source (whole-function loop_depth/numbering). This is the sole residual mechanism; directed permuter on the score-2 index-B chassis (fresh numbering vs stale s4/s5) is the primary next probe.

- [s10] Floor 2 independently reverified on the committed index-B chassis (sandbox --disable all = 2, target 79 / build 79, 8 rules dropped, 34 cheat-asm stripped); Region B judge-PASS (docs/grind/decisions.md 2026-07-22 06:02) confirmed in place.

- [s10] CORRECTS ledger: plain sum=0 in candidate position on the index-B chassis = score 4, not 9. Per-function objdump proves plain's inner-loop preheader is BYTE-PERFECT (a0=0 first, v1=bp, a1=0 last == target); the only 4 diffs are the inner a0<->a1 swap. Region A' is ABSENT in plain (sum=0 at low LUID emits first).

- [s10] Region A' localized to exactly 2 insns: the emit-position swap of `addu a1,zero,zero`(j=0) and `addu a0,zero,zero`(sum=0). do-while(0)'s block-bottom sum=0 def -> highest preheader LUID -> sched1 emits sum=0 at pos6; target has it at pos4.

- [s10] EXACT trade quantified at the byte level: do-while(0) fixes the 4 inner swap diffs (-4) but introduces 2 preheader-order diffs (+2) => score 2. The A'/A coupling (s6/s7) is one relocation: the +1 sum ref (RA win) and the block-bottom sum=0 def (emit-order loss) are the same NOTE_INSN_LOOP_BEG effect.

- [s10] Target's inner accumulate body, sum=0 def, and sum==chk compare are ALL byte-identical to ours, yet target reaches sum=$a0 with sum=0 emitted FIRST (unbracketed) => target's sum has weighted reg_n_refs>=11 (tie j=11) from a NON-bracket source; that natural 11th ref is the sole open residual mechanism.

- [s10] Sibling func_80037F40 (src 190-239) re-read in full: its checksum accumulates ONCE before the outer loop (depth 1) and is WRITTEN then reused across 3 outer iterations; damage RECOMPUTES sum INSIDE the outer loop (depth 2) and READS/compares it. The depth-2 recompute is the root of damage's extra-weight requirement; the sibling's shallower shape is semantically unavailable (no transplant value; confirms s7/s9).

- [s10] The Region-B index rewrite renumbered the whole-function pseudos/LUIDs vs the s4/s5 permuter campaigns (old explicit-ap/a2p chassis), so those campaigns are STALE for Region A'; a fresh directed permuter on the score-2 chassis is genuinely unexplored.
