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

## Frontier for s2
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
