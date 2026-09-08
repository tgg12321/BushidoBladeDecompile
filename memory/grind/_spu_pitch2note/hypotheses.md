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
