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

## [s3] Declaration order of the function's locals is a live lever for this function
- mechanism: pseudo birth order feeds local-alloc's quantity numbering and global.c's allocno ordering, so permuting declarations should shuffle the v0/v1/t0/a1 assignments in the inner loop.
- probe: 12 forms measured with sandbox --disable all on the floor-17 base: sent first, rect first, tbl/idx before xoff/yoff, idx before tbl, sent before tbl, rect after outer, yoff before xoff, fp_ptr last, new_var last, new_var deleted (literal shift), `off` hoisted to function scope in two positions.
- result: ALL TWELVE are exactly inert - 17 at 135 insns, byte-identical output. Re-measured on the floor-13 base (three more permutations): still exactly inert at 13.
- verdict: KILLED - declaration order does not move anything in this function. Do not spend another session on it.

## [s3] The LoadImage argument-setup block is stranded mid-body by an anti-dependence from the `tbl += 2` pointer bump, and moving the bump to the tail of the loop body frees sched1 to issue the args first
- mechanism: In the floor-17 form the body order was `off/idx++ | x-store | y-load | tbl+=2 | a0/a1 setup | rect[2]/rect[3] | y-store | jal`. The .greg RTL showed `(insn 210 (set tbl (plus tbl 4)) ... (insn_list:REG_DEP_ANTI 201))` - an anti-dependence against the y-coordinate load at insn 201 - sitting directly between the coordinate work and the argument-setup chain (insns 227 `a0=sp+24`, 343 `la`, 229 `addu a1`). sched1 could not lift the arg-setup block above that anti-dependence, so the a0/a1 chain stayed mid-body while target issues it as the first thing in the loop body. Moving the pointer bump past DrawSync removes the anti-dependence from between them.
- probe: `tbl += 2;` relocated from between the rect[1] store and the rect[2] store to immediately after `DrawSync(0)` (i.e. just before the func_80048A7C call); sandbox --disable all, then a normalized objdump side-by-side against build/src/text1a_post.o.
- result: 17 -> 13 at 135/135 insns. The side-by-side now shows ZERO ordering divergence: target insns 82..118 and ours are in identical positions, and the only remaining differences are register names. Moving the bump further (before DrawSync, or after func_80048A7C) is equally good (13); leaving it mid-body is 17.
- verdict: CONFIRMED

## [s3] The residual 13 is an expression-shape problem inside the loop body
- mechanism: if the surviving t0-vs-v0/a1 renames rode on how the address, the coordinates or the loop condition are spelled, respelling would move them.
- probe: 20 further forms on the floor-13 base: source address named in a block-local; address add commuted; `idx * 32`; literal `idx << 5` with new_var deleted; `off` inlined into the call (two increment placements); `off` at function scope; rect[2]/rect[3] swapped; rect[2]/rect[3] moved after the LoadImage args; `&rect[0]` for `rect`; `sent` deleted; `*tbl` for `tbl[0]`; u16* tbl with tbl[0]/tbl[1]; coordinates named in block-locals; rect[1] stored before rect[0]; DrawSync before LoadImage; idx++ at the body end; three more declaration permutations; `tbl += 2` in two further positions.
- result: fourteen are exactly inert (13 at 135). Six are WORSE and are banked in rejected/: rect[1]-before-rect[0] 45/139, DrawSync-before-LoadImage 73/136, idx++-at-end 16, coordinates-in-block-locals 15, rect[2]/rect[3]-after-args 15, off-inlined 15. Nothing improves on 13.
- verdict: KILLED - the residual is not reachable by inner-body expression shape.

## Frontier after s3 (floor 13)
1. **Deny hard reg $a1 to the `off` pseudo (129).** This is the WHOLE residual - all
   thirteen differing insns are downstream of it, and the instruction stream is
   otherwise byte-for-byte target's in target's order. Mechanism (read from .lreg +
   the gcc-2.7.2 sources, not inferred): `off` is block-local to the loop body
   (block 10) and dies in `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`.
   local-alloc.c:1240-1300 walks that insn's source operands calling
   combine_regs(operand, recog_operand[0]); combine_regs at local-alloc.c:1884-1896
   sees a HARD setreg and unconditionally records a1 in qty_phys_sugg[qty(129)]
   (returning 0, so the operand loop does not break - putting the symbol first does
   NOT help, and that was measured). find_free_reg's just_try_suggested pass then
   honours it: ";; Register 129 in 5". With a1 taken, reload has nothing better than
   $t0 for the three loop-hoisted invariants 136/137/139 that global.c left
   unallocated. Target has off in $v0 - i.e. NO suggestion, so find_free_reg took the
   first reg in REG_ALLOC_ORDER. Next probe: find_free_reg excludes any hard reg that
   appears in `regs_live_at[ins]` for ins in [born_index, dead_index)
   (local-alloc.c:2170). So the lever is to make a1 LIVE somewhere inside off's live
   range, or to make off NOT block-local (reg_qty < 0 kills the suggestion path at
   local-alloc.c:1826 and hands the pseudo to global.c, which allocates from
   REG_ALLOC_ORDER = $v0 first). Both are structural questions about where `off` is
   born relative to the basic-block boundary at the top of the loop body - the one
   axis s3 could not reach from inside the body. The instrumented cc1 has a
   `BB2_SUGG_DEBUG` env hook (local-alloc.c, prints the `used`/`first_used` hard-reg
   sets per find_free_reg call) - use it to confirm any candidate form in one build
   instead of guessing from the score.
2. **The trailing `li v1,1 / bne v0,v1` vs target `li t0,1 / bne v0,t0`** (the
   `func_8003E2A0() == 1` test, outside both loops). Ours ALLOCATES the constant 1 to
   $v1; target rematerializes it into $t0. This is the mirror image of item 1 (ours
   allocates where target rematerializes and vice versa), which suggests a single
   global.c allocno-ordering difference rather than two independent problems.
   Re-measure only after item 1; read .greg's allocno order if it survives.
3. **Do NOT reopen**: declaration order (12+3 forms, exactly inert), inner-body
   expression shape (13 forms in s2 + 20 in s3, nothing improves), the plain
   array-index / `*(ptr+i)` spellings of the D_80094DF0 load, the a2-param-reuse
   outer counter, the do-while(0) yoff wrap.

## [s3] Declaration order of the function's twelve locals is a live lever for this function (pseudo birth order -> local-alloc quantity numbering -> the v0/v1/t0/a1 renames in the inner loop).
- mechanism: Pseudo numbers are assigned in declaration order; local-alloc numbers quantities by birth order within a block and global.c orders allocnos by priority, so permuting declarations should shuffle which short-lived inner-loop temp wins $v0.
- probe: Twelve permutations measured with `sandbox --disable all` on the floor-17 base (sent first, rect first, tbl/idx before xoff/yoff, idx before tbl, sent before tbl, rect after outer, yoff before xoff, fp_ptr last, new_var last, new_var deleted with a literal shift, `off` hoisted to function scope in two positions); three more permutations re-measured on the floor-13 base.
- result: ALL FIFTEEN are exactly inert - 17/135 on the floor-17 base and 13/135 on the floor-13 base, with byte-identical output in every case.
- verdict: KILLED

## [s3] The LoadImage argument-setup block is stranded in mid-body by an anti-dependence from the `tbl += 2` pointer bump, and relocating the bump to the tail of the loop body lets sched1 issue the arg setup first, as target does.
- mechanism: .greg on the floor-17 form shows `(insn 210 (set tbl (plus tbl 4)) (insn_list:REG_DEP_ANTI 201))` - an anti-dependence against insn 201, the y-coordinate load - sitting directly between the coordinate work and the arg-setup chain (insns 227 `a0=sp+24`, 343 the `la`, 229 `addu a1`). sched1 cannot lift the arg-setup above that anti-dependence, so the a0/a1 block lands mid-body while target issues it as the first thing in the loop body.
- probe: `tbl += 2;` moved from between the rect[1] store and the rect[2] store to immediately after `DrawSync(0)`; `sandbox --disable all`, then a normalized objdump side-by-side (tmp/grind/func_80041BF4/s3/sbs.py, built on engine.score.normalized_insns) against build/src/text1a_post.o.
- result: 17 -> 13 at 135/135 insns. The side-by-side reports ZERO ordering divergence afterwards: target insns 82..118 and ours occupy identical positions and only register names differ. Moving the bump further along the tail (before DrawSync, or after func_80048A7C) is equally good at 13; leaving it mid-body is 17.
- verdict: CONFIRMED

## [s3] The residual 13 is an expression-shape problem inside the loop body, reachable by respelling the address, the coordinates, or the loop condition.
- mechanism: If the surviving $t0-vs-$v0/$a1 renames rode on how the LoadImage source address, the coordinate arithmetic or the sentinel test are spelled, re-association or naming would move sched1 priorities and local-alloc order.
- probe: Twenty forms measured on the floor-13 base: source address named in a block-local; address add commuted; `idx * 32`; literal `idx << 5` with new_var deleted; the `off` expression inlined into the call (two increment placements); `off` at function scope; rect[2]/rect[3] swapped; rect[2]/rect[3] moved after the LoadImage args; `&rect[0]` for `rect`; `sent` deleted; `*tbl` for `tbl[0]`; u16* tbl with tbl[0]/tbl[1] reads; coordinates named in block-locals; rect[1] stored before rect[0]; DrawSync before LoadImage; idx++ at the body end; three declaration permutations; `tbl += 2` in two further tail positions.
- result: Fourteen are exactly inert (13 at 135 insns). Six are WORSE and are banked in rejected/: rect[1]-before-rect[0] 45/139, DrawSync-before-LoadImage 73/136, idx++-at-body-end 16, coordinates-in-block-locals 15, rect[2]/rect[3]-after-args 15, off-expression-inlined 15. Nothing improves on 13. With s2's thirteen forms that is 33 measured inner-body spellings whose only winner was a STATEMENT MOVE, not a respelling.
- verdict: KILLED

## [s3] The entire residual 13 is downstream of a single local-alloc decision: `off` (pseudo 129) is handed hard reg $a1 by the qty_phys_sugg path, which denies $a1 to reload's rematerialization of the three loop-hoisted invariants.
- mechanism: `off` is block-local to the loop body (block 10) and dies in `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`. local-alloc.c:1240-1300 walks that insn's source operands calling combine_regs(operand, recog_operand[0]); combine_regs at local-alloc.c:1884-1896 sees a HARD setreg and unconditionally records a1 in qty_phys_sugg[reg_qty[129]], returning 0 (so the operand loop does not break - operand POSITION is irrelevant). find_free_reg's just_try_suggested pass (local-alloc.c:2208-2215) then restricts the candidate set to that suggestion. With $a1 taken, reload has nothing better than $t0 for regnos 136 (const 16), 137 (const 1) and 139 (symbol_ref D_800A9A24), which loop.c hoisted out of both loops and global.c left unallocated.
- probe: Read .lreg and .greg for func_80041BF4 on the floor-13 form (`;; Register 129 in 5.`, the insn-229 RTL, the register-dispositions table showing 136/137/139 absent) plus the .loop movable log; cross-read tools/gcc-2.7.2/local-alloc.c and loop.c for the exact code paths; then commuted the address add to test the operand-position prediction.
- result: Confirmed on both the floor-17 and floor-13 dumps: `;; Register 129 in 5.`, dispositions list has no entry for 136/137/139, and the .loop log still shows all three hoisted out of the 37-insn inner loop and then out of the 59-insn outer loop. The commuted-address form is exactly inert (13), exactly as the `combine_regs` return-0 reading predicts. Target has `off` in $v0 (no suggestion -> first reg in REG_ALLOC_ORDER) and the rematerialized symbol in $a1.
- verdict: CONFIRMED
