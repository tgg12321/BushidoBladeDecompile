# Hypothesis ledger — func_80041BF4

## Frontier after s1 (floor 22, rotation solved — see evidence.md [s1])
1. **Symbol-address form of the in-loop D_80094DF0 load** (residual item 1).
   Mechanism: under the new outer-loop notes, loop.c/cse treats the
   loop-invariant `symbol_ref D_80094DF0` differently and keeps the full
   address in a pseudo ($t0) instead of the %hi/%lo indexed form. Probe:
   dump.ps1 on the candidate form, read .loop (was the address a movable?)
   and .cse; compare against sibling func_80041AC8's RTL (same expression,
   emits target form outside a loop). Candidate levers: hoist the
   D_80094E08[sid] index load into a named local so the address arithmetic
   is no longer the loop-invariant candidate; or spell the read through the
   already-named sid expression per [[defeat-licm-hoist-var-reuse]]
   (reuse-a-scratch-variable recipe, but note we need the INVERSE — keep
   the recompute IN the loop).
2. **LoadImage arg-setup scheduling** (residual item 2) — re-measure only
   AFTER item 1: the a1 arg chain is the same symbol machinery, so fixing
   the address form likely reflows the sched1 order; do not tune this
   independently first.
3. **Caller-save renames (a1/v0, v1/t0)** — expected to collapse with 1/2;
   if any survives, read .lreg for the specific pseudo and check
   [[local-alloc-death-count-class-wall]] / rank-for-schedule ties before
   inventing levers.

Standing kills to respect (do NOT reopen): every outer-ref spelling
(fold/emit dichotomy), a2-param-reuse (rejected/), the do-while(0) yoff wrap
under the real outer loop (it inflates yoff to 5853 and flips the rotation
backwards), permuter random campaigns on the old basin.

## [s1] The 3-cycle s2/s3/s4 rotation is reachable in pure C via loop-depth-weighted allocno priorities, without any new outer reference
- mechanism: flow.c counts REG_N_REFS weighted by loop_depth (NOTE_INSN_LOOP from real loop constructs only); global.c pri = floor_log2(refs)*refs/live_length*10000. A real `do {...} while (outer < 2);` lifts outer's in-loop refs to depth 2 (pri 2000 -> ~5192 with the floor_log2 jump at 9 weighted refs), above b (~3870), giving allocation order outer > b > yoff > xoff = target's
- probe: rewrote the goto outer loop as do-while AND removed the leftover `do { yoff = 0; } while (0);` wrap (which put yoff's else-def at depth 3, pri ~5853, and had flipped the prior do-while attempt's rotation the wrong way); sandbox --disable all
- result: score 22 (from 29); objdump shows a2 home->s2, outer->s2, b->s3, yoff->s4, xoff->s5, g/r->s6/s7, fp_ptr->fp — target's full callee-save allocation; 135/135 insns
- verdict: CONFIRMED

## [s1] Reusing the a2 param variable as the outer counter reproduces target's s2 sharing
- mechanism: merged pseudo would inherit the param's low pseudo number and win priority ties
- probe: a2 = 0; ... a2++; while (a2 < 2) spelling; sandbox
- result: score 34: merged pseudo has refs 7 over live length ~86 (two disjoint phases) -> pri ~1628, allocates LAST, lands in s4 and perturbs prologue save order; banked in rejected/a2-param-reuse-as-outer.c
- verdict: KILLED

## [s2] The in-loop D_80094DF0 symbol-address divergence is a loop.c LICM hoist that breaks combine's symbol-into-MEM fold, and it is repairable by removing the MULT from the address tree
- mechanism: scale-4 array indexing makes expand emit a standalone `(set rN (symbol_ref))` plus an explicit add insn; loop.c scan_loop records rN as a movable and move_movables (loop.c:1631, `threshold*savings*lifetime >= insn_count`) hoists it to the outer-loop preheader; the def and its use are then in different basic blocks so combine cannot fold the symbol into the MEM, and reload rematerializes a full `la` inside the loop. loop.c's own substitute-and-delete escape (loop.c:735) cannot fire because `(set r (plus r SYM))` is not a valid MIPS addsi3.
- probe: `pwsh tools/grinder/dump.ps1 func_80041BF4`, read `.loop` (the "Insn 163: regno 124 ... moved to 325" decision log) and `.rtl` (expand shape for the scale-1 D_80094E08 access vs the scale-4 D_80094DF0 access); then a 5-form spelling sweep of the table load with sandbox
- result: `tbl = *(s16 **)((u8 *) D_80094DF0 + (IDX << 2));` removes regno 124 from the .loop movables entirely and prints target's `lui at,%hi; addu at,at,v0; lw s0,%lo(at)`. Score 22 -> 17 at 135/135 insns. `*(D_80094DF0 + IDX)` and a `* 4` scale both stay at 22 (MULT survives).
- verdict: CONFIRMED

## [s2] The residual 17 is an inner-loop expression-shape problem
- mechanism: if the arg-setup schedule and the v0/v1/t0 renames rode on how the coordinate arithmetic and the LoadImage source expression are spelled, re-associating or naming them would move the allocation
- probe: 13 inner-body forms measured with sandbox (source named in a local; `(s32)&D_800A9A24 + off`; `tbl += 2` moved; `idx++` folded into the shift; address assigned back into `off`; both coordinate adds commuted, separately and together; each coordinate read named in a block-local; rect[2]/rect[3] order swapped; rect[2]/rect[3] stores hoisted above the coordinate stores)
- result: twelve are EXACTLY inert (17, 135 insns); the thirteenth (constant stores hoisted above the coordinate stores) is 46 at 139 insns. The residual does not move with inner-body spelling.
- verdict: KILLED

## Frontier after s2 (floor 17)
1. **Kill the inner-loop LICM hoist of regnos 136/137/139** (`(const_int 16)`,
   `(const_int 1)`, `(symbol_ref D_800A9A24)`), the same mechanism that item 1
   above solved one level out. Mechanism: loop.c move_movables hoists all three
   out of the 37-insn inner loop and then out of the outer loop; reload
   rematerializes them (count-neutral) but into `$t0` instead of target's
   `$v0`/`$a1`, and the LoadImage arg-setup block loses its top-of-body
   schedule slot. Next probe: the D_800A9A24 argument is a VALUE (a call
   argument), not a MEM address, so the scale-trick that fixed D_80094DF0 does
   not transfer directly — look for a spelling in which the source address is
   consumed as a MEM (so loop.c's `reg_single_usage` substitute-and-delete path
   at loop.c:735 CAN fire and delete the movable), or one in which the symbol
   is not a separable single-set. Read `.loop` after each form and check
   whether regno 139 still appears in the movable log — that is a faster,
   sharper signal than the score.
2. **The y-coordinate temp takes `$v0` where target takes `$v1`**, which is
   what denies `$v0` to the two rematerialized constants. Mechanism: local-alloc
   assignment order for the short-lived coordinate temps. Only worth attacking
   if item 1 does not collapse it — inner-body respellings are already measured
   dead for this (see the KILLED entry above), so the lever must come from
   outside the body (e.g. the loop-bottom `sll` / the sentinel read pair).
3. **The LoadImage arg-setup schedule position** (target: top of the loop body,
   right after `addiu s1,s1,1`; ours: mid-body). Expected to be a consequence of
   1, since the rematerialized `la` insns are inserted by reload AFTER sched1 and
   can only be moved by sched2. Re-measure only after item 1; if it survives,
   read `.sched`/`.sched2` for the arg pseudos before inventing any lever.

Standing kills to respect (do NOT reopen): everything in the s1 list, plus the
thirteen inner-body respellings measured dead in s2, plus the plain array-index
and `*(ptr + i)` spellings of the D_80094DF0 load (they keep the hoist).

## [s2] The in-loop D_80094DF0 symbol-address divergence is a loop.c LICM hoist that separates the (set reg symbol_ref) def from its MEM use across a basic-block boundary, so combine can no longer fold the symbol into the load; it is repairable in pure C by removing the MULT node from the address tree.
- mechanism: Scale-4 array indexing forces the address sum out of EXPAND_SUM into a real add insn, leaving a standalone (set rN (symbol_ref D_80094DF0)). scan_loop records rN as a movable and move_movables hoists it into the outer-loop preheader (loop.c:1631, threshold*savings*lifetime >= insn_count, threshold from loop.c:532). loop.c's own substitute-and-delete escape (loop.c:735) cannot fire because (set r (plus r SYM)) is not a valid MIPS addsi3. reload then rematerializes a full la inside the loop. Scale-1 indexing (the adjacent D_80094E08 access) keeps the sum un-emitted and already matched.
- probe: pwsh tools/grinder/dump.ps1 func_80041BF4; read the .loop movable log (Insn 163: regno 124 (life 2), move-insn savings 1  moved to 325), the .rtl expand shapes for both accesses, and .combine fold survival; then a 5-form spelling sweep of the table load measured with sandbox --disable all.
- result: tbl = *(s16 **)((u8 *) D_80094DF0 + (IDX << 2)) removes regno 124 from the .loop movables entirely and prints target's lui at,%hi / addu at,at,v0 / lw s0,%lo(at). Score 22 -> 17, 135/135 insns. Plain array index = 22, *(D_80094DF0 + IDX) = 22, (s32 *) deref with * 4 = 22, (s32) base cast with << 2 = 17.
- verdict: CONFIRMED

## [s2] The residual 17 (LoadImage arg-setup schedule position, t0-vs-v0/a1 scratch renames, sll a1 vs sll v0) is an expression-shape problem in the inner loop body.
- mechanism: If the arg-setup schedule and the caller-save renames rode on how the coordinate arithmetic and the LoadImage source expression are spelled, re-association or naming would move sched1 priorities and local-alloc order.
- probe: 13 inner-body forms measured with sandbox --disable all: source address named in a block-local; (s32)&D_800A9A24 + off without the u8* cast; tbl += 2 moved; idx++ folded into the shift; computed address assigned back into off; rect[0] add commuted; rect[1] add commuted; both commuted; x read named in a block-local; y read named in a block-local; rect[2]/rect[3] order swapped; rect[2]/rect[3] stores hoisted above the coordinate stores.
- result: Twelve are EXACTLY inert (17 at 135 insns). The thirteenth (constant stores above the coordinate stores) is 46 at 139 insns and is banked in rejected/rect-const-stores-before-coords.c. The residual does not move with inner-body spelling.
- verdict: KILLED
