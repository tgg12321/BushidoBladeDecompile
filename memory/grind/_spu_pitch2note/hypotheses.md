# Hypothesis ledger — _spu_pitch2note

## s1 (recon, 2026-09-08)
H1 natural `lower + step*j` / `lower + step*(j+1)` (v1)  - KILLED (instance, 59):
   the first sum is a whole giv reduced with `lower` folded into its init;
   target keeps `lower` as a separate add on both sides. Mechanism read in
   evidence.md (loop.c:5096 / loop.c:3823).
H2 explicit loop-carried `acc`/`next` accumulators (v2/v3) - CONFIRMED: reproduces
   the zero- and step-initialised bivs and both `lower + x` adds; with the
   shifted bounds computed before the `if` (v3) the delay-slot pattern of the
   first branch matches (55).
H3 named `quot = result/128; rem = result%128;` before `note`/`fine` (v7) -
   CONFIRMED: epilogue byte-identical, `result` moves to $v0, hit block stolen
   into the `bnez -> found` slot (36).
H4 `cents++` running counter eliminated by loop.c into `li 0x600` / `base+j`
   (v9/v10) - KILLED (class): unroll.c:3051 refuses final_biv_value for any
   loop with a non-fallthrough exit; measured 45 with `addiu cents,1` left in
   the inner loop and no hoisted base.
H5 hit label after the loops (`goto hit; ... result = 0x600; goto found; hit:
   result = base + inner; found:`) (v5/v11/v12/v17) - KILLED (instance, 41/60):
   the pre-reload jump.c pass relocates the SCAN loop's break block out of line
   (fresh label 262 after the `j found` barrier) and the `j found` survives.
H6 `result = 0x600` initialised before the loops (v14/v15/v16) - KILLED
   (instance, 57/58): loses the `li v0,0x600` placement and shuffles seats.
H7 increment order `next += step; acc += step;` (v13) - CONFIRMED neutral (36 =
   v7); the target's slot insn is acc's increment, so acc must be nearest the
   branch - keep this order.

## s2 (structural, 2026-09-08) — floor 36 -> 21

H8 SIBLING TRANSPLANT: the COMPLETED-C sibling `_spu_2pitch` spells the same
   0x103B curve step as `ratio *= 0x103B; ratio >>= 12;` (split compound
   assignments) - CONFIRMED, -12: transplanted onto the s1 chassis
   (`curve *= 0x103B; curve >>= 12;` replacing `curve = (curve*0x103B)>>12;`)
   the floor drops 36 -> 24 with no other change.
H9 HIT ARM FIRST closes s1's F1 structural residual - CONFIRMED, -1 and
   structural: `if (target >= lo && target < hi) { result = (outer<<5)+inner;
   goto found; } next += step; acc += step;` puts the CODE_LABEL BEFORE the
   increments, so reorg.c:2969's backward search from the back-edge bnez
   reaches `acc += step` and fills the slot with it, exactly as the target.
   With H8: 23, and build_insns 74 == target_insns 74.
H10 DECLARATION ORDER is inert for this function - KILLED (instance): four
   permutations of the 21 locals (first-use order, counters-first, target-early,
   accumulators-early) all measure 23 with a BYTE-IDENTICAL instruction stream.
   GCC 2.7.2 numbers pseudos at first RTL definition, not at expand_decl of the
   local, so decl order cannot move the allocno tie-break here.
H11 PRE-LOOP STATEMENT ORDER is a live lever - CONFIRMED: all 24 permutations of
   the four pre-loop assignments (`oct`, `scale`, `curve`, `target`) measured;
   range 21..25; unique minimum p21 = `target; scale; curve; oct;` at 21, which
   seats `bit` in $v1 as the target does. (p00 = the s1 order = 23.)
H12 Statement order INSIDE the outer-loop body is inert - KILLED (instance):
   `next = step; acc = 0;` swapped (q1) and `acc = 0;` hoisted to the top of the
   outer body (q2) both measure 21 on the p21 base; on the v22 base both measure
   23. sched1 re-orders that block regardless of the C order.
H13 `bit = 0;` before `search = ~pitch;` (q3/v41) - KILLED (instance): 23 on the
   p21 base, 24 on the v22 base.
H14 `target = pitch;` as the first statement of the function (v35) - KILLED
   (instance): 38; it lengthens the widened value's live range across the scan
   loop and reshuffles the seats.
H15 `scale = 1; scale <<= bit;` split init (v44) - KILLED (instance): 24 on the
   v22 base.

## s3 (structural, 2026-09-08) � floor 21 -> 20

H16 SPLIT-COMPOUND-ASSIGNMENT SWEEP over the remaining folded expressions:
   splitting the UPPER inner-loop bound alone (`hi = lower + next; hi >>= 12;`,
   variant r2b) is worth -1 - CONFIRMED, 21 -> 20.  It is the unique winner of
   the sweep: splitting the LOWER bound alone (r2a) 21, splitting BOTH (r2) 21,
   both-with-interleaved-shifts (h4) 20 = byte-identical to r2b, hi-first
   ordering (h2/h3/e2) 20 = same, lo-first-both-split (e3) 21, lo-split-hi-folded
   with the shift last (e1) 21.  Everything else in the family is NEGATIVE:
   `step = upper - lower; step >>= 5;` (r1) 33, `result = outer << 5;
   result += inner;` (r3) 35 at 75 insns, `note <<= 8; return note | fine;` (r4)
   44 at 76 insns, `lower = curve; lower *= scale;` (f1) 22, `upper = curve;
   upper *= scale;` (f2) 21, `scale = 1; scale <<= bit;` (r7) 23 (re-confirms
   s2's H15 on this chassis), `oct = bit; oct -= 12;` (r6) 20 = inert,
   `search = pitch; search = ~search;` (r8) 20 = inert, and writing the curve
   step as `curve = curve * 0x103B; curve >>= 12;` instead of the compound form
   (f3) is byte-neutral (20).

H17 The pre-loop statement-order sweep re-run on the NEW inner-loop shape:
   all 24 permutations of `target/scale/curve/oct` measured on the r2b base
   (s3 q00..q23), range 20..24, and q00 = `target; scale; curve; oct;` = s2's
   p21 is STILL the unique minimum - CONFIRMED, the s2 F3 caveat is discharged
   for this loop shape.

H18 Making `target = pitch;` an invariant of the outer or the inner loop so that
   loop.c hoists it into the preheader (which would place the `andi` AFTER the
   `move outer,zero` as the target does) - KILLED (instance): all 12 forms
   (m1_* = first statement of the outer body, m2_* = first statement of the inner
   body, crossed with the 6 orders of the remaining three pre-loop assignments)
   measure 21, 22 or 24; the best is 21, one point WORSE than leaving it in the
   pre-loop block.  LICM lands the hoisted insn in the same slot it already
   occupies, and the loss comes from the reshuffled pseudo numbers.

H19 Statement order INSIDE the outer-loop body is still inert on the r2b base -
   KILLED (instance): `next = step; acc = 0;` (n1) and `acc = 0;` hoisted to four
   different points of the outer body (n2..n5) all measure 20 - re-confirms s2's
   H12 on the new loop shape.

H20 Block-local (nested-scope) declarations move the allocation - KILLED
   (instance): declaring `lo`/`hi` inside the inner loop (b1/cb1) and
   `lower/upper/step/acc/next` inside the outer loop (b2) measure exactly the
   base score (21 on the p21 base, 20 on r2b).  Same mechanism as s2's H10:
   GCC 2.7.2 mints the pseudo at the first RTL definition, so scope placement of
   the declaration cannot move the allocno tie-break.

H21 Re-spelling the scan loop - KILLED (instance): a named intermediate for the
   shifted bit (`u32 probe = search >> shift;`, s3) and the explicit `== 0`
   comparison (s6) are byte-neutral at 20; carrying `bit` as the loop's own
   assignment (`bit = shift;` at the top of the body, s5) lets GCC delete `bit`
   as a copy of `shift` - 71 insns, 24.

H22 Dropping the `target` local and comparing `pitch` directly (d2) 21;
   deriving `search` from `target` (`target = pitch; search = ~target;`, d1)
   22 at 73 insns (the copy is eliminated - the target HAS the copy);
   `result = (outer << 5) | inner` (d4) 21; swapping the two && arms
   (`target < hi && target >= lo`, d6) 23; `s32 lo, hi` instead of `u32`
   (g1) 21 - all KILLED (instance).

## Frontier for s4
F1 (RA, unchanged and still the whole residual): the 5-seat permutation
   pitch-copy $a3 vs $a0, shift $a0 vs $t2, lower $t3 vs $t4, acc $t2 vs $t3,
   outer $t4 vs $t2.  s2 banked: ra_solver forward model EXACT (21/21) on the
   solo TU (tmp/grind/_spu_pitch2note/s2/solo/ + extract_solo.py - the
   instrumented cc1 SEGFAULTS on the full main.i), inverse.py depth-2 NEGATIVE
   for the 6-seat goal and for the single flip {80:10}.  The s3 floor drop did
   not change the seat set, so the s2 model is still current modulo one insn.
   next probe (unchanged, and now the only untried modelled input):
   local_extract.py --suggest / BB2_SUGG_DEBUG on the solo TU to price the
   pitch-copy quantity - the target seats the copy in $a0, an incoming-arg
   register freed by `move t8,a0`, which is exactly what local-alloc's
   qty_phys_copy_sugg pass would do.
F2 (sched1): two order inversions - (andi target / move outer,zero) and
   (addu lo / addu hi).  The addu pair is NEW information from s3: the target
   emits the LOWER bound's addu first, ours emits the UPPER bound's first, and
   no source ordering of the two bounds changes it (h2/h3/e2/e3 sweep) - it is
   set by sched1, not by the C order.  Classify with tools/sched_solver AFTER
   F1.
F3 (spelling, cheap): the 24-permutation pre-loop sweep is re-validated for the
   CURRENT loop shape (H17).  Re-run it (tmp/grind/_spu_pitch2note/s3/v/q00..q23
   + sweep.ps1) after ANY change to the loop body; the spread is 4 points.

## (superseded) Frontier for s3
F1 (RA, the whole remaining residual): a 5-seat permutation - shift $a0 vs $t2,
   pitch-copy $a3 vs $a0, lower $t3 vs $t4, acc $t2 vs $t3, outer $t4 vs $t2 -
   plus two sched1 order inversions (the `andi a2,<pitchcopy>,0xffff` / `move
   <outer>,zero` pair and the `srl v0,v0,0xc` / `addu v1,<lower>,a3` pair).
   ra_solver STATUS (measured this session, do not re-derive):
     * the instrumented cc1 SEGFAULTS on the full main.i (dies after 59 .ent,
       at _spu_FiDMA) - extract.py cannot run on stem `main`.  The workaround
       that WORKS and reproduces our exact allocation is a solo TU:
       tmp/grind/_spu_pitch2note/s2/solo/solo.c (`#include "common.h"` + the
       function body) + tmp/grind/_spu_pitch2note/s2/extract_solo.py (a copy of
       extract.py with ROOT and the src dir repointed; do NOT put it in tools/).
     * simulate.py on that model is EXACT: 21/21 dispositions, sort order MATCH.
     * pseudo map: 72 cen_note, 74 cen_fine, 76 pitch-copy, 79 bit, 80 shift,
       81 oct, 82 scale, 83 curve, 84 target, 85 lower, 87 step, 88 acc,
       89 next, 92 outer, 93 inner, 94 result, 117 base.
     * inverse.py global, goal {76:4,79:3,80:10,85:12,88:11,92:10}: NEGATIVE at
       depth 2 (refs +12/-6, livelen +/-2,4,8).  The single flip {80:10}
       (shift $v1 -> $t2) is ALSO negative at depth 2.
     * hand what-if (tmp/grind/_spu_pitch2note/s2/whatif.py): applying the
       livelen deltas implied by the two observed sched1 inversions
       (84 -1, 92 +1, 88 -2, 93 +1, 117 +1) changes NOTHING - 0/6 goal seats.
   next probe: the negative points at inputs the global model does not carry -
   local-alloc's SUGGESTED-REGISTER pass (qty_phys_copy_sugg / qty_phys_sugg)
   and the pre-RA hard-reg preference set.  Run local_extract.py --suggest on
   the solo TU (BB2_SUGG_DEBUG) and price the pitch-copy quantity (`move
   a3,a2`, a copy off the incoming $a2) - the copy-suggestion is the one input
   that plausibly seats it in $a0 in the target.
F2 (sched1): the two order inversions are inside blocks the RA already colours
   differently; classify with tools/sched_solver AFTER F1, not before - the
   object-level classifier calls the first divergence RA, and livelen what-ifs
   for those inversions are already measured inert.
F3 (spelling, cheap): p21 is the minimum over the 24 pre-loop permutations on
   the CURRENT inner-loop shape.  Any future change to the inner loop invalidates
   that sweep - re-run the 24-permutation sweep (measure.sh p00..p23) whenever
   the loop body changes; the spread is 4 points and it is nearly free.

## (superseded) Frontier for s2
F1 (structural, the only non-seat residual): find the ordinary-C shape whose
   RTL has the hit block OUT OF LINE while the increments fall straight into
   `inner++` with no label/jump between (reorg.c:2969 stops at a CODE_LABEL).
   Lead: whatever relocated v11's scan block (compare v11 vs v13 .jump->.cse2
   dumps with the instrumented cc1 and BB2_XJUMP_DEBUG=1; candidates jump.c:1531,
   1718, 2324 get_label_after sites and the range swap at jump.c:1840ff) is
   plausibly the mechanism that put the target's hit block out of line. Try:
   (a) v13 with the `else` arm first (`if (t>=lo && t<hi) {hit} else {incs}`),
   (b) v3 shape with the `goto found` replaced by `break` + a post-loop test
   that GCC can fold, (c) a `do {} while` inner loop, (d) `continue`-based
   increments in the for-clause. Measure each with pairdiff + the .dbr dump.
F2 (seats): once F1 lands, classify the remaining $a3/$a0 pitch copy and the
   bit/shift/curve/lower/outer/acc/step cascade with
   tools/ra_solver inverse_compose.py classify before spelling anything.
F3 (fallback for F1): the two-pass reorg route - if the loop-top peel can be
   made ineligible for the first fill_eager pass, the second reorg pass sees
   the `j Lend` deleted and may take `acc += step`; low priority.

## [s1] The natural spelling `lower + step*j` / `lower + step*(j+1)` (v1) reproduces the target's inner-loop induction shape
- mechanism: loop.c strength reduction: `lower + step*j` is recorded as a whole giv (mult step, add lower) and reduced to a biv initialised to lower; the target keeps `lower` as a separate add on both sides with 0/step-initialised accumulators
- probe: sandbox v1 + .loop dump (tmp/grind/_spu_pitch2note/s1/v1.loop.txt: insn 128 giv reduced to reg 140 with add lower; insn 138 not a giv per loop.c:5096)
- result: 59; `move a3,t3` init of the folded giv present, target has none
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, prototype s32(u16,u16,u16)

## [s1] Explicit loop-carried accumulators `acc`/`next` with both shifted bounds computed before the if (v3) reproduce the target's inner-loop first block
- mechanism: mult-1 givs `lower + acc` fall under loop.c:3823 'not worth while' and stay as in-loop adds; both shifts in BB1 let reorg fill the first bnez slot with `srl v1` as in the target
- probe: sandbox v2 (60) then v3 (55); pairdiff v3
- result: 55; addu/addu/srl/sltu/bnez/[srl v1] block matches
- verdict: CONFIRMED

## [s1] Named `quot = result/128; rem = result%128;` before note/fine (v7) matches the epilogue and frees the hit block to be stolen into the `bnez -> found` slot
- mechanism: shorter live range for `result` lets global.c allocate it to $v0 (dead on the fallthrough path), satisfying reorg fill_eager's steal condition; in v4 `result` sat in $a2 = live `target` and the steal was refused
- probe: sandbox v6 (51), v7 (36), v8 (54); v13 side-by-side tmp/grind/_spu_pitch2note/s1/v13_sbs.txt
- result: 36; epilogue byte-identical, `addu v0,t5,a0` in the hit slot
- verdict: CONFIRMED

## [s1] A running `cents++` counter read after the `goto found` exit is biv-eliminated by loop.c into the target's `li v0,0x600` and `base + inner` form
- mechanism: unroll.c final_biv_value refuses to compute a final value for any loop whose loop_number_exit_count is nonzero, so the biv cannot be eliminated when the loop has a goto exit
- probe: sandbox v9 (&&-goto) and v10 (if/else) with cents counter; pairdiff v9
- result: 45 both; `addiu cents,1` left in the inner loop, no hoisted `sll t5` base, no `li 0x600`
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs
- predicate_cite: unroll.c:3051

## [s1] A `hit:` label placed after the loops (`result = 0x600; goto found; hit: result = base + inner; found:`) with a named per-octave base (v11/v12/v17) yields the target's out-of-line hit block
- mechanism: the hit block is out of line by construction; but the pre-reload jump_optimize (toplev.c:2923/2929) then relocates the SCAN loop's break block to a fresh label after the `j found` barrier and the `j found` survives
- probe: sandbox v5 (60), v11 (41), v12 (41), v17 (41); .jump vs .cse2 dumps of v11 (label 262 minted)
- result: 41; scan branch inverted with its block moved before hit:, extra `j found` kept
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs

## [s1] Initialising `result = 0x600` before the loops instead of assigning it after them (v14/v15/v16) keeps the target's `li v0,0x600` placement
- mechanism: the constant store would have to be sunk past both loops; GCC 2.7.2 has no code sinking, so it lands in the preheader and the seats reshuffle
- probe: sandbox v14 (57), v15 (57), v16 (58)
- result: 57-58, far worse than 36
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs

## [s2] The COMPLETED-C sibling _spu_2pitch's spelling of the shared 0x103B curve step (`ratio *= 0x103B; ratio >>= 12;`, two compound assignments) transplanted onto this chassis reproduces the target's curve block
- mechanism: the sibling walks the same 0x103B curve; the split compound-assignment form keeps the multiply and the >>12 as separate RTL statements instead of one folded expression, which changes what cse2/combine hands the allocator
- probe: v21 = s1 candidate with only that substitution; v22 = v21 + hit-arm-first; sandbox --disable all
- result: v21 24 (from 36); v22 23 with build_insns 74 == target_insns 74
- verdict: CONFIRMED

## [s2] Putting the HIT arm first (`if (target >= lo && target < hi) { result = (outer<<5)+inner; goto found; }` followed by the two increments) removes the CODE_LABEL between `acc += step` and `inner++` and fills the back-edge delay slot with `acc += step` as the target does
- mechanism: reorg.c:2969 fill_simple_delay_slots stops its backward search at a CODE_LABEL; with the hit arm first the label lands BEFORE the increments, so the back-edge bnez reaches acc's increment
- probe: v20 (hit arm first alone) and v22 (with the sibling curve spelling); pairdiff + sbs against build/src/main.o
- result: v20 35 (from 36), v22 23; the loop-top peel is gone and the slot insn is the accumulator increment, exactly as at 0x8008BD3C. s1's F1 frontier item is closed
- verdict: CONFIRMED

## [s2] Permuting the declaration order of this function's 21 locals changes its allocation
- mechanism: global.c breaks allocno-priority ties by pseudo number = birth order, and expand_decl was assumed to mint pseudos in declaration order
- probe: v24 (first-use order), v26 (target early), v27 (accumulators early), v28 (loop counters first) on the v22 base; sandbox + sbs
- result: all four measure 23 and v28's instruction stream is byte-identical to v22's - GCC 2.7.2 mints the pseudo at the first RTL definition, not at the declaration
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, v22 base at floor 23

## [s2] The order of the four pre-loop assignments (oct, scale, curve, target) is a live lever on this chassis
- mechanism: first-definition order sets pseudo numbers, which are global.c's allocno tie-break and also feed sched1's LUID tie-break
- probe: all 24 permutations p00..p23 generated and measured with sandbox --disable all
- result: spread 21..25; unique minimum p21 = `target; scale; curve; oct;` at 21 (s1's order p00 = 23). p21 seats `bit` in $v1 as the target does
- verdict: CONFIRMED

## [s2] The remaining 5-seat permutation (shift $a0 vs $t2, pitch-copy $a3 vs $a0, lower $t3 vs $t4, acc $t2 vs $t3, outer $t4 vs $t2) is reachable by perturbing a modelled global.c input - refs, live length, birth order, conflicts, preferences or calls-crossed
- mechanism: global.c allocno priority = floor_log2(nrefs)*nrefs/livelen; C spellings move refs and live spans, which is what inverse.py searches
- probe: ra_solver extract on a solo TU (the instrumented cc1 segfaults on full main.i), simulate.py (exact 21/21), then inverse.py global --depth 2 with the 6-seat goal and separately with the single flip {80:10}; plus a hand what-if applying the livelen deltas implied by the two observed sched1 inversions
- result: both inverse runs return the validated depth-2 NEGATIVE (search bounds refs +12/-6, livelen +/-2,4,8) and the livelen what-if scores 0/6 goal seats; the tool names the unmodelled inputs (local-alloc suggested-register pass, qty_size, reload retry)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, p21 body at floor 21, solo-TU model with simulate.py exact 21/21

## [s3] Splitting the UPPER inner-loop bound into two statements (`hi = lower + next; hi >>= 12;`) while leaving the lower bound folded drops the floor by one
- mechanism: the split keeps the add and the shift as separate RTL statements instead of one folded expression, which changes what cse2/combine hands the allocator - the same family as the s2 sibling transplant of `curve *= 0x103B; curve >>= 12;`
- probe: r2a (lower only), r2b (upper only), r2 (both), h1..h4/e1..e3 (interleavings and orderings) on the s2 p21 body; sandbox --disable all + pairdiff (tmp/grind/_spu_pitch2note/s3/r2b_pairdiff.txt)
- result: r2b 20 (from 21) and it is the unique winner - r2a 21, r2 21, e1 21, e3 21; h2/h3/h4/e2 also 20 with the same instruction stream
- verdict: CONFIRMED

## [s3] The pre-loop statement order p21 (`target; scale; curve; oct;`) is still the unique minimum after the inner-loop shape changed
- mechanism: first-definition order sets pseudo numbers, which are global.c's allocno tie-break and sched1's LUID tie-break; s2's F3 warned the 24-permutation sweep is only valid for the loop shape it was measured on
- probe: all 24 permutations regenerated on the r2b base (q00..q23) and measured with sandbox --disable all
- result: range 20..24; q00 = p21 = 20 is the unique minimum (next best 21 at q01/q06/q08/q09/q11/q14/q15)
- verdict: CONFIRMED

## [s3] Making `target = pitch;` a loop invariant so loop.c hoists it into the preheader reproduces the target's late `andi` placement
- mechanism: the target emits `andi a2,a0,0xffff` AFTER `move outer,zero`; a LICM-hoisted invariant lands in the preheader, which is emitted after the loop counter's initialisation
- probe: 12 variants - `target = pitch;` as the first statement of the outer body (m1_*) and of the inner body (m2_*), crossed with all 6 orders of the remaining three pre-loop assignments; sandbox --disable all
- result: 21, 21, 21, 24, 22, 24 (m1_*) and the identical six for m2_*; the best is 21, one point worse than the pre-loop placement, and the andi does not move
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] Block-local (nested-scope) declarations of the loop temporaries move the allocation
- mechanism: expand_decl for a nested block runs at block entry, which was assumed to mint the pseudos in a different order than function-scope declarations
- probe: b1/cb1 (`lo`,`hi` declared inside the inner loop) and b2 (`lower`,`upper`,`step`,`acc`,`next` inside the outer loop) on both the p21 base and the r2b base; sandbox --disable all
- result: exactly the base score in every case (21 on p21, 20 on r2b) - same conclusion as s2's H10 for function-scope declaration order
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, p21 body at 21 and r2b body at 20

## [s3] Statement order inside the outer-loop body is a live lever on the new (r2b) loop shape
- mechanism: sched1 re-orders that block regardless of the C order (s2's H12), but the loop body changed, so the conclusion needed re-measuring
- probe: n1 (`next = step; acc = 0;`) and n2..n5 (`acc = 0;` hoisted to four different points of the outer body) on the r2b base
- result: all five measure 20, unchanged from the base
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] Re-spelling the scan loop or the search value changes the seats
- mechanism: refs and live lengths of `search`/`shift`/`bit` are global.c allocno-priority inputs, and the pitch copy's seat is the largest single residual
- probe: s3 (named intermediate `probe = search >> shift`), s6 (`== 0` comparison), s5 (`bit = shift;` carried at the top of the body), r8 (`search = pitch; search = ~search;`), d1 (`target = pitch; search = ~target;`), d2 (drop `target`, compare `pitch` directly)
- result: s3/s6/r8 byte-neutral at 20; s5 24 with 71 insns (GCC deletes `bit` as a copy of `shift`); d1 22 with 73 insns (the pitch copy itself is eliminated - the target HAS the copy); d2 21
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] Splitting the UPPER inner-loop bound into two statements (hi = lower + next; hi >>= 12;) while leaving the lower bound folded drops the floor by one
- mechanism: the split keeps the add and the shift as separate RTL statements instead of one folded expression, which changes what cse2/combine hands the allocator - the same family as the s2 sibling transplant of curve *= 0x103B; curve >>= 12; from the COMPLETED-C sibling _spu_2pitch
- probe: variants r2a (lower only), r2b (upper only), r2 (both), h1..h4 and e1..e3 (interleavings and orderings) applied to the s2 p21 body; sandbox _spu_pitch2note --disable all on each, plus pairdiff on the winner
- result: r2b = 20 (from 21) and it is the unique winner: r2a 21, r2 21, e1 21, e3 21; h2/h3/h4/e2 also 20 with the same instruction stream. 74 build insns == 74 target insns.
- verdict: CONFIRMED

## [s3] The pre-loop statement order p21 (target; scale; curve; oct;) is still the unique minimum after the inner-loop shape changed
- mechanism: first-definition order sets pseudo numbers, which are global.c's allocno tie-break and sched1's LUID tie-break; s2's F3 warned the sweep is only valid for the loop shape it was measured on
- probe: all 24 permutations of the four pre-loop assignments regenerated on the r2b base (q00..q23) and measured with sandbox --disable all
- result: range 20..24; q00 = p21 = 20 is the unique minimum, next best 21
- verdict: CONFIRMED

## [s3] Making target = pitch; a loop invariant so loop.c hoists it into the preheader reproduces the target's late andi placement
- mechanism: the target emits andi a2,a0,0xffff after move outer,zero; a LICM-hoisted invariant lands in the preheader, which is emitted after the loop counter's initialisation
- probe: 12 variants: target = pitch; as the first statement of the outer body (m1_*) and of the inner body (m2_*), crossed with all 6 orders of the remaining three pre-loop assignments
- result: 21/21/21/24/22/24 for m1_*, identical six for m2_*; best 21, one point worse than the pre-loop placement, and the andi does not move
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] Block-local (nested-scope) declarations of the loop temporaries move the allocation
- mechanism: expand_decl for a nested block runs at block entry, which was assumed to mint the pseudos in a different order than function-scope declarations
- probe: b1/cb1 (lo, hi declared inside the inner loop) and b2 (lower, upper, step, acc, next inside the outer loop) measured on both the p21 base and the r2b base
- result: exactly the base score in every case (21 on p21, 20 on r2b) - same conclusion as s2's H10 for function-scope declaration order
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, p21 body at 21 and r2b body at 20

## [s3] Statement order inside the outer-loop body is a live lever on the new (r2b) loop shape
- mechanism: sched1 re-orders that block regardless of the C order (s2's H12), but the loop body changed so the conclusion needed re-measuring
- probe: n1 (next = step; acc = 0;) and n2..n5 (acc = 0; hoisted to four different points of the outer body) on the r2b base
- result: all five measure 20, unchanged from the base
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] Re-spelling the scan loop or the search value moves the residual seats
- mechanism: refs and live lengths of search/shift/bit are global.c allocno-priority inputs, and the pitch copy's seat is the largest single residual
- probe: s3 (named intermediate probe = search >> shift), s6 (== 0 comparison), s5 (bit = shift; carried at the top of the body), r8 (search = pitch; search = ~search;), d1 (target = pitch; search = ~target;), d2 (drop target, compare pitch directly), g1 (s32 lo/hi), d4 (| instead of +), d6 (swapped && arms)
- result: s3/s6/r8 byte-neutral at 20; d2/d4/g1 21; d6 23; d1 22 with 73 insns (the pitch copy is eliminated, but the target HAS the copy); s5 24 with 71 insns (GCC deletes bit as a copy of shift)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20

## [s3] The split-compound-assignment family helps on the other folded expressions of this function (step, result, return, lower, upper, scale, oct)
- mechanism: same mechanism as the winning hi split - separate RTL statements survive combine differently and change what the allocator is handed
- probe: r1 (step), r3 (result), r4 (return), f1 (lower), f2 (upper), r7 (scale), r6 (oct), f3 (curve written as curve = curve * 0x103B) on the p21 and r2b bases
- result: r1 33, r3 35 at 75 insns, r4 44 at 76 insns, f1 22, f2 21, r7 23 (re-confirms s2 H15); only r6 and f3 are byte-neutral. The hi split is the sole member of the family that helps here
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, p21 body at 21 and r2b body at 20

## [s4] Moving the inner loop's accumulator initialisations into the for-init comma expression, with acc = 0 after inner = 0, drops the floor by one
- mechanism: the for-init clause is expanded as part of the loop's preheader rather than as ordinary outer-body statements, which moves the emission position of `move acc,zero` past the inner-loop counter's initialisation and into the neighbourhood the target puts it in (target insns 32/33/34 = `move inner,zero` / `sll base,outer,5` / `move acc,zero`)
- probe: 9 hand variants a1..a9 crossing which of {acc = 0, next = step} sits in the for-init and in what order relative to inner = 0; each spliced over the INCLUDE_ASM line in a pristine src/main.c and measured with sandbox --disable all
- result: a1 (inner, acc, next) 19, a3 (inner, next, acc) 19, a4 (next outside; inner, acc) 19, a8 (next, inner, acc) 19; a2/a6 (acc first) 20, a5 (acc outside the init clause) 20, a7/a9 (acc before inner) 20. All 74 insns. New floor 19; a1 is the candidate.
- verdict: CONFIRMED

## [s4] The decomp-permuter's weighted score tracks the engine's honest sandbox distance on this function's residual
- mechanism: both metrics diff the same two instruction streams, so a lower permuter score (regs x5 + reorderings x60) was expected to imply a lower masked differing-instruction count
- probe: campaign 1 on the s3 chassis (tmp/perm_p2n, 37,051 iterations, 103 finds, permuter scores 230..580); 38 finds spliced back into src/main.c and measured individually with sandbox --disable all
- result: no monotone relation. permuter 230 -> sandbox 20; permuter 480 -> sandbox 60; permuter 485 -> sandbox 20; permuter 565 -> sandbox 19 (the two best sandbox results in the whole campaign); permuter 580 (the base) -> sandbox 20. The campaign's own best find is no better than the base on the metric that counts.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20, tmp/perm_p2n campaign 1 (37,051 iters)

## [s4] Naming the LICM-hoisted inner-loop base (base = outer << 5) lets C control whether the sll is emitted before or after move acc,zero
- mechanism: the target emits `sll base,outer,5` at insn 33 and `move acc,zero` at 34 while we emit them in the opposite order; naming the value makes it an ordinary statement whose position the source can set, instead of a loop.c hoist placed by the preheader builder
- probe: c1 (for-init: inner, base, acc, next), c2 (next outside; for-init: inner, base, acc), c3 (base as a statement before the for), c4 (for-init: inner, base, next, acc), c5 (for-init: base, inner, acc, next), each with `result = base + inner;`
- result: c1/c2/c4 = 19 with the identical stream to a1 (byte-neutral); c3/c5 = 20. The sll/move pair never flips - the hoisted sll is placed by the preheader builder regardless of where the named statement sits.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s4] The pre-loop statement order changed again now that the inner loop's init clause moved
- mechanism: first-definition order sets pseudo numbers, which are global.c's allocno tie-break and sched1's LUID tie-break, so every loop-shape change invalidates the previous sweep (s2 F3 / s3 frontier item 3)
- probe: all 24 permutations of `target = pitch; scale = 1 << bit; curve = 0x1000; oct = bit - 12;` regenerated on the a1 base (s4 p00..p23) and measured with sandbox --disable all
- result: range 19..23; p00 = `target; scale; curve; oct;` (the s2 p21 / s3 q00 order) is still the unique minimum at 19, next best 20. Third consecutive session the same order wins.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s4] Moving the inner loop's accumulator initialisations into the for-init comma expression, with acc = 0 placed after inner = 0, lowers the honest floor
- mechanism: the for-init clause is expanded as part of the loop's preheader rather than as ordinary outer-loop-body statements, which moves the emission position of `move acc,zero` past the inner-loop counter's initialisation and into the neighbourhood the target puts it in (target insns 32/33/34 = `move inner,zero` / `sll base,outer,5` / `move acc,zero`)
- probe: nine hand variants a1..a9 crossing which of {acc = 0, next = step} sits inside the inner for-init and in what order relative to inner = 0; each spliced over the INCLUDE_ASM line of a pristine src/main.c by tmp/grind/_spu_pitch2note/s3/apply.py and measured with `sandbox _spu_pitch2note --disable all`
- result: a1 (inner, acc, next) 19, a3 (inner, next, acc) 19, a4 (next = step outside; for (inner = 0, acc = 0)) 19, a8 (next, inner, acc) 19; a2 and a6 (acc = 0 first in the clause) 20, a5 (acc = 0 left outside the clause) 20, a7 and a9 (acc = 0 before inner = 0) 20. All nine build 74 insns == 74 target insns. New floor 19; a1 is the candidate. The rule is that acc = 0 must be inside the for-init AND after inner = 0.
- verdict: CONFIRMED

## [s4] The decomp-permuter's weighted score tracks the engine's honest sandbox distance on this function's register-seat residual, so a campaign can be steered by its own gradient
- mechanism: both metrics diff the same two instruction streams, so a lower permuter score (regs x5 + reorderings x60) was expected to imply a lower masked differing-instruction count
- probe: campaign 1 on the s3 chassis (tmp/perm_p2n, 37,051 iterations, 1,105 s, 103 finds, permuter scores 230..580, base 580) plus campaign 2 on the new a1 chassis (tmp/perm_p2n2, 8,845 iterations, 35 finds, base 575); 64 finds in total spliced back into src/main.c and measured individually with `sandbox _spu_pitch2note --disable all`
- result: no monotone relation in either campaign. Campaign 1: permuter 230 -> sandbox 20; permuter 480 -> sandbox 60; permuter 485 -> sandbox 20; permuter 565 -> sandbox 19 (the two best sandbox results in the whole campaign); permuter 580 (the base) -> sandbox 20. Campaign 2: permuter 380 -> sandbox 20 while permuter 575 -> sandbox 19. Neither campaign's own best find beat the base on the metric that counts. Full table in tmp/grind/_spu_pitch2note/s4/sandbox_measurements.txt.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, r2b body at floor 20 and a1 body at floor 19; campaigns tmp/perm_p2n (37,051 iters) and tmp/perm_p2n2 (8,845 iters)

## [s4] Naming the LICM-hoisted inner-loop base (base = outer << 5) lets the C source decide whether the sll is emitted before or after move acc,zero
- mechanism: the target emits `sll base,outer,5` at insn 33 and `move acc,zero` at 34 while we emit them in the opposite order; naming the value makes it an ordinary statement whose position the source can set, instead of a loop.c invariant hoisted by the preheader builder
- probe: c1 (for-init: inner, base, acc, next), c2 (next = step outside; for-init: inner, base, acc), c3 (base as a plain statement before the for), c4 (for-init: inner, base, next, acc), c5 (for-init: base, inner, acc, next), each rewriting the hit block to `result = base + inner;`, measured with sandbox --disable all
- result: c1, c2 and c4 measure 19 with the stream identical to a1 (byte-neutral); c3 and c5 measure 20. The sll/move pair never flips: the hoisted sll is placed by the preheader builder regardless of where the named statement sits in the source.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s4] The pre-loop statement order that was optimal on the s3 loop shape stops being optimal now that the inner loop's init clause has moved
- mechanism: first-definition order sets pseudo numbers, which are global.c's allocno tie-break and sched1's LUID tie-break, so the ledger warns that every loop-shape change invalidates the previous sweep (s2 F3 / s3 frontier item 3)
- probe: all 24 permutations of `target = pitch; scale = 1 << bit; curve = 0x1000; oct = bit - 12;` regenerated on the a1 base (s4 p00..p23) and measured with sandbox --disable all
- result: range 19..23; p00 = `target; scale; curve; oct;` (the s2 p21 / s3 q00 order) is still the unique minimum at 19, next best 20. Third consecutive session in which the same order survives a loop-shape change.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s4] Putting the outer-loop or scan-loop initialisations into their own for-init comma expressions moves the andi/oct order inversion the way the accumulator move did
- mechanism: the same preheader-expansion mechanism that paid off for the inner loop's acc = 0 should apply to `target = pitch` and `oct = bit - 12` at the outer loop and to `bit = 0` at the scan loop, and the target emits oct at insn 16 and the andi at 21 while we emit the andi at 16 and oct at 20
- probe: e1 for (outer = 0, target = pitch; ...), e2 for (outer = 0, curve = 0x1000; ...), e3 for (outer = 0, oct = bit - 12; ...), e4 for (shift = 15, bit = 0; shift >= 0; shift--), e5 for (outer = 0, target = pitch, oct = bit - 12; ...), e6 for (target = pitch, outer = 0; ...), all on the a1 base
- result: e3 is byte-neutral at 19; e1, e4, e5, e6 are 20; e2 is 30. All build 74 insns. The andi/oct inversion does not move - the outer-loop preheader is not the lever the inner-loop preheader was.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s5] One counter variable driving BOTH the scan loop and the outer loop reproduces the target's shared $t2 seat and collapses the whole 5-seat residual
- mechanism: the target's disassembly holds the scan counter and the outer counter in the same register ($t2 receives `li 15`, is decremented through the scan loop, and is then zeroed for the outer loop). One C variable for both loops gives local-alloc a single quantity to seat instead of two competing ones, and the rest of the allocation follows into the target's assignment.
- probe: m2 (delete the scan counter's declaration, drive both loops from the outer counter's variable) and m1 (the mirror: delete the outer counter's declaration, drive both from the scan counter's slot), applied to the a1 body and measured with sandbox --disable all
- result: m2 = 7 (from 19); m1 = 19. 74 insns in both. At 7 the pairdiff shows EVERY register matching; only three order inversions remain (andi/oct/outer-init 3-cycle, sll/move acc pair, addu upper/lower pair). The declaration slot that survives is load-bearing.
- verdict: CONFIRMED

## [s5] Naming the scan-loop test's shifted value into an existing dead local is worth one point, and specifically into the outer-loop counter
- mechanism: the extra def/ref lands in the outer counter's quantity, which changes local-alloc's suggestion scan and frees $t4 for the lower bound; a fresh local mints a new quantity instead and does not move the seat
- probe: k1 (outer counter as the carrier), k2 (fresh local `probe`), k3 (the `& 1` folded in), k5 (inner), k6 (result), k7 (lower), k8 (acc), each on four chassis (a1, x1, x7, x8); the pointer was permuter find tmp/perm_p2n_x7/output-570-3
- result: k1 = 18 on all four chassis; k2 = k3 = k6 = 19; k7 = 20; k5/k8 = 33/32. The construct is superseded in the final body by the shared-counter form, which removes it.
- verdict: CONFIRMED

## [s5] Reusing a LIVE variable as the upper-bound carrier (the permuter's own 18) has a legal equivalent among the dead locals
- mechanism: the campaign's 18-scoring finds on the x8 chassis wrote `outer = lower + next; hi = outer;` inside the inner loop, clobbering the live outer counter; the hypothesis was that any dead u32 local would give the same refcount effect
- probe: g1 (upper), g2 (shift), g3 (bit), g4 (quot), g5 (rem), g6 (note), g7 (fine), g8 (scale), h1 (upper, shift folded), j1/j2 (lo reused as the high-bound carrier), j3 (search retyped u32), j4, j5, on the x8 and a1 chassis
- result: no legal carrier reaches 18. g1/h1 = 19 (byte-neutral), g8 = 23, j5 = 21, j1/j2 = 34, j4 = 35, j3 = 43/44 at 78 insns, and every s32-typed carrier (g2/g3/g4) costs 5 insns because the unsigned shift becomes arithmetic. The seat flip belonged to the outer counter's quantity, which the shared-counter form then obtained legally.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19 and x8 body at floor 19

## [s5] Moving the outer-loop body's remaining assignments (lower, upper, step, curve) into the inner loop's for-init comma expression is a live lever
- mechanism: s4's frontier item 3 — the for-init clause is expanded in the loop preheader rather than as ordinary body statements, and the s4 sweep showed the winning positions are narrow
- probe: f1 (step into the init before acc), f2 (step after acc), f3 (upper + step), f4 (all four incl. the curve update), f6 (curve update onward), on the a1 body
- result: f1/f2/f3 byte-neutral at 19; f4 and f6 = 31 because folding `curve *= 0x103B; curve >>= 12;` back into one expression undoes the s2 sibling transplant. The lever is inert for these statements — the point s4 found belonged to the accumulator init alone.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, a1 body at floor 19

## [s5] Every order-sweep conclusion banked in s2/s3/s4 survives the loop-shape change to the shared-counter (m2) chassis
- mechanism: the ledger's standing warning is that first-definition order sets pseudo numbers (global.c allocno tie-break, sched1 LUID tie-break), so each sweep is chassis-relative; three sessions of stable results had made the pre-loop order look settled
- probe: the 24-permutation pre-loop order sweep re-run on m2 (q00..q23); `target = pitch` into the outer for-init (t1/t2/t3) on five bases; the lo/hi bound spellings (L1..L6) and the accumulator-init placements (A2..A8, B1..B6) on the resulting bases
- result: all three flipped. Pre-loop order range 6..8 with `oct` FIRST as the new minimum (the three-session winner `target; scale; curve; oct;` is 7). `target = pitch` in the outer for-init is 4 vs 6 as a statement (s4 measured the same move as a LOSS, 20 vs 19). Splitting BOTH bounds with the low bound's add first (L6) is 2, fixing the addu inversion that s2/s3/s4 recorded as immune to source order. Accumulator-init placement is inert on this shape (all 6 variants tie the base).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, m2 body at floor 7 and t1q04 body at floor 4

## [s5] The (move acc,zero / sll base,outer,5) inversion is not steerable from C
- mechanism: s4's c1..c5 probes concluded that the sll is placed by loop.c's preheader builder regardless of where a named `base = outer << 5` sits in the source, so the pair could only be broken by a scheduler-input change
- probe: C1..C4 on the L6 body (named base in four positions), D1/D2 (arithmetic spellings of the hit value), D3 (acc seeded from next), B1..B6 (accumulator-init placements)
- result: FALSE on this chassis. C1 (`for (inner = 0, base = outer << 5, acc = 0, next = step; ...)` with `result = base + inner`) = 0 — the match. C2/C3/C4 = 2, D1 = 2, D2 = 3. The identical construct measured byte-neutral on the s4 chassis; "not steerable" was a chassis-relative instance result, not a property of loop.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD chassis 2026-09-08 (-mel -msoft-float), no FAKE constructs, L6 body at floor 2 (the kill is of the s4 NON-steerability claim; the probe that killed it is the matching form)
