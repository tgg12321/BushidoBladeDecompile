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

## [s4] The `off`-in-$a1 residual is caused SOLELY by local-alloc's qty_phys_sugg path, so any C form that makes `off` non-block-local (reg_qty < 0) frees $a1 and collapses the residual
- mechanism: s3's frontier item 1 named combine_regs (local-alloc.c:1857-1896) recording $a1 in `qty_phys_sugg[qty(129)]` from `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`, and combine_regs' early bail at local-alloc.c:1826 (`reg_qty[ureg] < 0` when the pseudo is not local to the block or dies more than once) as the escape. The prediction was that a cross-block `off` would be handed to global.c, which allocates from REG_ALLOC_ORDER ($v0 first), reproducing target.
- probe: Two forms that make `off` cross-block, each verified DIRECTLY with the instrumented compiler (tools/gcc-2.7.2/cc1, `BB2_SUGG_DEBUG=1`, which prints the complete per-qty suggestion table at block-alloc time) rather than inferred from the score: (A) `off` as a loop-carried induction variable declared at function scope, initialised to 0 before the inner loop and re-computed as `off = idx << new_var;` at the BOTTOM of the loop body; (B) `off` computed in the loop-CONDITION block via a comma operator, `while (off = idx << new_var, (sent = tbl[0]) >= 0)`. Baseline block-10 table for reference: `blk=10 qty=0 reg1=129 birth=6 death=12 refs=6 nsugg=1 sugg=5,` — the only qty in the whole function carrying a plain suggestion.
- result: BOTH forms remove the suggestion exactly as predicted — in vA and vB the block-10 table has NO qty with `sugg=5` at all (only the two unrelated `copysugg=4,` / `copysugg=5,` argument qtys remain) — and BOTH are WORSE, not better: vA = 19, vB = 17, both at 135 insns. A normalized objdump of vB against target shows `sll a1,s1,5` and `addu a1,a1,t0` still present: global.c put `off` in $a1 anyway. Reading global.c confirms the second, independent route — record_one_conflict (global.c:1728 and :1747) does `SET_REGBIT (hard_reg_preferences, reg_allocno[src_regno], dest_regno)` for exactly this hard-dest/pseudo-source relation, and find_reg consumes hard_reg_preferences at global.c:1133-1140.
- verdict: KILLED. Denying the local-alloc SUGGESTION is not sufficient and is not the lever. `off` is pushed to $a1 by the set-to-hard-$a1 relation itself, in BOTH allocators, and any C form that only changes block-locality changes which allocator does it, not the outcome. The only remaining mechanical escape is a CONFLICT: $a1 must be live somewhere inside `off`'s [born_index, dead_index) so that find_free_reg's `for (ins = born_index; ins < dead_index; ins++) IOR_HARD_REG_SET (used, regs_live_at[ins]);` (local-alloc.c:2168-2171) excludes it, and the analogous conflict set excludes it in global.c.

## [s4] Frontier item 2 (the trailing `li v1,1 / bne v0,v1` in the `func_8003E2A0() == 1` test) is the mirror image of the `off`/$a1 residual and will only move when item 1 moves
- mechanism: s3 argued that ours ALLOCATES the constant 1 where target rematerializes it and vice versa inside the loop, so a single global.c allocno-ordering shift would flip both, and that the item must not be tuned independently.
- probe: The s4 permuter campaign (tools/permuter_campaign.py, workspace tmp/grind/func_80041BF4/s4/ws, 21428 iterations / 714 s, base weighted score 80) returned output-70-1, which is exactly an independent attack on this item: an `int new_var2 = 1;` constant holder consumed as `func_8003E2A0() == new_var2`. Re-measured in the real chassis with `sandbox --disable all`, plus three ORDINARY-C spellings of the same test as controls (result named in a block-local `rc`; result named in a function-scope `rc`; Yoda form `1 == func_8003E2A0()`).
- result: The constant holder alone takes the floor 13 -> 11 at 135 insns, and a normalized objdump against target shows the ONLY change is `li v1,1 / bne v0,v1` -> target's `li t0,1 / bne v0,t0`; the loop-body `off`/$a1 divergence is completely untouched. All three ordinary-C spellings are EXACTLY inert at 13 and are banked in rejected/.
- verdict: KILLED (the coupling claim). The two items are independent: item 2 is closable on its own and its closure buys nothing for item 1. The residual 11 is now PURELY the `off`-in-$a1 allocation and its four downstream renames.

## [s4] The permuter can reach the residual by respelling the LoadImage source-address expression
- mechanism: The campaign's other novel find, output-75-1, hoists the symbol base into a function-scope pointer (`u8 *new_var2 = (u8 *)&D_800A9A24;` assigned immediately after the func_8004153C call) and consumes it as `new_var2 + off` — a plausible route to changing whether loop.c hoists the `(set rN (symbol_ref D_800A9A24))` and therefore whether the symbol pseudo is a block-10 qty competing for $a1.
- probe: Re-measured in the real chassis with `sandbox --disable all` (the permuter's weighted score 75-vs-base-80 is not the project metric).
- result: EXACTLY INERT — 13 at 135 insns, byte-identical to the s3 base. Banked in rejected/symbol-base-named-in-function-scope-local.c. The weighted-score improvement was entirely an artifact of the permuter's own scoring.
- verdict: KILLED

## Frontier after s4 (floor 11)
1. **Make $a1 CONFLICT with `off` rather than merely un-preferred.** This is now the
   whole residual: all eleven differing insns are `off` sitting in $a1 plus the
   $t0/$v0/$v1 renames downstream of it, and the instruction stream is otherwise
   byte-for-byte target's in target's order. Mechanism (MEASURED this session, not
   inferred): `off` is a direct register source of `(set (reg:SI 5 a1) (plus
   (reg/v:SI 129) (reg:SI 139)))`, and BOTH allocators independently route it to
   $a1 from that one relation — local-alloc via combine_regs -> qty_phys_sugg
   (local-alloc.c:1857-1896) and global.c via record_one_conflict ->
   hard_reg_preferences (global.c:1728/1747, consumed at global.c:1133-1140).
   Preference-removal is measured dead (see the [s4] KILL above). A conflict is not:
   find_free_reg unions `regs_live_at[ins]` over [born_index, dead_index) into its
   `used` set (local-alloc.c:2168-2171), so a form in which $a1 is LIVE strictly
   between `off`'s birth (the `idx << 5` shift) and its death (the `addu a1,...`)
   excludes $a1 for `off`. Next probe: note target's own shape is `lui a1,%hi /
   addiu a1,a1,%lo / addu a1,v0,a1` — in TARGET the SYMBOL value is the thing living
   in $a1 while `off` lives in $v0. So the sharpest question is: what makes the
   symbol pseudo (139) the $a1 winner? It is currently hoisted out of both loops by
   loop.c and left unallocated by global.c, so it never competes; if it were
   block-local to the loop body it would carry the same $a1 suggestion and, by
   qty_sugg_compare ordering, could take $a1 first and force `off` down
   REG_ALLOC_ORDER to $v0. Read the `.loop` movable log for regno 139 FIRST and find
   what would make move_movables decline it — that is a loop.c question, not an
   allocation question, and it is the same shape as the axis s2 closed one level out
   for D_80094DF0.
2. **The constant holder for the trailing test is banked but NOT vetted.** The floor-11
   candidate carries a `/* FAKE */`-annotated `int one = 1;` (constant-holder family,
   .claude/rules/named-local-fake-exception.md). Three ordinary-C spellings are
   measured inert, so it is the only known form — but it has never been through a
   layer-1 cheat-reviewer or the Judge, and the FAKE prerequisites demand the full
   modality ladder be spent BEFORE it is spent. Do not treat it as accepted. If a
   later session closes item 1 and reaches distance 0, the self-vet must claim the
   constant-holder family explicitly and quote that rule's scope sentence verbatim.
3. **Do NOT reopen**: everything in the s1/s2/s3 do-not-reopen lists, plus (s4) any
   form whose whole theory is "make `off` non-block-local so local-alloc stops
   suggesting $a1" — measured dead twice with the compiler's own suggestion table as
   the witness — plus the function-scope symbol-base pointer, plus the three
   ordinary-C spellings of the trailing `== 1` test.

## [s4] The `off`-in-$a1 residual is caused solely by local-alloc's qty_phys_sugg path, so any C form making `off` non-block-local (reg_qty < 0, combine_regs bails at local-alloc.c:1826) frees $a1 and collapses the residual. (s3 frontier item 1.)
- mechanism: combine_regs (local-alloc.c:1857-1896) records $a1 in qty_phys_sugg[qty(129)] from `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`; find_free_reg's just_try_suggested pass honours it. Making `off` cross-block was predicted to hand it to global.c, which allocates from REG_ALLOC_ORDER ($v0 first) as target does.
- probe: Two cross-block forms built and verified DIRECTLY against the instrumented cc1's per-qty suggestion table (tools/gcc-2.7.2/cc1 with BB2_SUGG_DEBUG=1) rather than inferred from the score: (A) `off` as a function-scope loop-carried induction variable, initialised before the inner loop and recomputed at the BOTTOM of the body; (B) `off` computed in the loop-condition block via `while (off = idx << new_var, (sent = tbl[0]) >= 0)`. Baseline block-10 row: `blk=10 qty=0 reg1=129 birth=6 death=12 refs=6 nsugg=1 sugg=5,` — the only suggested qty in the function. Then sandbox --disable all and a normalized objdump against target.
- result: Both forms remove the suggestion exactly as predicted (no `sugg=5,` qty remains in block 10 in either dump), and both are WORSE: 19 and 17 at 135 insns. The vB objdump still shows `sll a1,s1,5` / `addu a1,a1,t0` — global.c put `off` in $a1 anyway. Reading global.c names the second, independent route: record_one_conflict does SET_REGBIT(hard_reg_preferences, reg_allocno[src], dest) at global.c:1728 and :1747 for exactly this hard-dest/pseudo-source relation, consumed by find_reg at global.c:1133-1140.
- verdict: KILLED

## [s4] The trailing `li v1,1 / bne v0,v1` in the `func_8003E2A0() == 1` test is the mirror image of the `off`/$a1 residual, so one allocno-ordering shift would flip both and it must not be tuned independently. (s3 frontier item 2.)
- mechanism: s3 read it as ours ALLOCATING the constant 1 where target rematerializes it and vice versa inside the loop — a single global.c allocno-ordering difference expressed twice.
- probe: The s4 permuter campaign returned output-70-1, an independent attack on this item alone: an `int new_var2 = 1;` constant holder consumed as `func_8003E2A0() == new_var2`. Re-measured in the real chassis with sandbox --disable all plus a normalized objdump, alongside three ordinary-C controls (result named in a block-local `rc`; named in a function-scope `rc`; Yoda `1 == func_8003E2A0()`).
- result: The constant holder alone takes 13 -> 11 at 135 insns and the objdump shows the ONLY change is `li v1,1 / bne v0,v1` -> target's `li t0,1 / bne v0,t0`; the loop-body $a1 divergence is untouched. All three ordinary-C controls are exactly inert at 13.
- verdict: KILLED

## [s4] The permuter can reach the residual by respelling the LoadImage source-address expression (the campaign's other novel find, output-75-1: a function-scope `u8 *` symbol base consumed as `base + off`).
- mechanism: Hoisting the symbol base into a named function-scope pointer could change whether loop.c hoists the `(set rN (symbol_ref D_800A9A24))` and therefore whether the symbol pseudo is a block-10 qty competing for $a1.
- probe: Re-measured in the real chassis with sandbox --disable all (the permuter's weighted 75-vs-base-80 is not the project metric).
- result: Exactly inert — 13 at 135 insns, byte-identical to the s3 base. The permuter-metric gain was an artifact of its own weighted scorer.
- verdict: KILLED

## [s5] The symbol pseudo (139) can be made a short-lived block-local allocno inside the loop body by naming it in a block-scope pointer, so that it carries the $a1 preference and — by allocno ordering — takes $a1 first, forcing `off` down REG_ALLOC_ORDER to $v0 exactly as target has it. (s4 frontier item 1, its named next probe.)
- mechanism: s4 established that BOTH allocators route `off` to $a1 from the single relation `(set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139)))`, and that the symbol pseudo 139 never competes because loop.c hoists it out of both loops and global.c leaves it unallocated (reload rematerializes it into $t0). Target's own shape is the mirror image — `lui a1,%hi(D_800A9A24) / addiu a1,a1,%lo / addu a1,v0,a1` INSIDE the loop body at .L80041D3C — i.e. in target the SYMBOL holds $a1 and `off` holds $v0. Naming the symbol base in a block-local pointer declared inside the loop body was the predicted route to making 139 a competing block-local qty.
- probe: Two spellings measured with `sandbox --disable all` in the real chassis, plus a normalized objdump of the first against target: (v1) `{ u8 *sp = (u8 *)&D_800A9A24; LoadImage((s32)rect, (s32)(sp + off)); }` with the pointer declared AFTER `off`; (v4) the same pointer declared BEFORE `off` in the loop-body block (declaration order is the s4-named lever for qty_sugg_compare ordering). Both banked in rejected/.
- result: BOTH are 47 at 140 insns — five instructions WORSE than the floor-11 form, not better. The objdump names the mechanism: the named pointer is loop-INVARIANT, so loop.c hoists it to a function-wide pseudo that needs a callee-saved register; the resulting pressure evicts `fp_ptr` from $s8 to the stack (frame 96 vs target's 88, `sw v0,32(sp)` / `lh v1,8(v0)` instead of target's `move s8,v0` / `lh v1,8(s8)`), and adds the spill/reload traffic. Naming the base does the OPPOSITE of shortening 139's live range — it lengthens it to whole-function scope.
- verdict: KILLED. Making the symbol block-local by NAMING it is self-defeating: any C name for a loop-invariant symbol address is hoisted by loop.c into a long-lived pseudo, which raises pressure rather than creating an in-loop competitor for $a1. Declaration order relative to `off` is irrelevant (v1 and v4 are identical at 47/140). A form that makes 139 compete must keep it loop-invariant-but-unhoisted, which is a loop.c/move_movables question and NOT reachable by naming.

## [s5] Interposing a named intermediate between `off` and the hard-$a1 argument breaks the source-of-a-set-into-$a1 relation that both allocators key on
- mechanism: s4 showed the preference comes from `off` being a direct register SOURCE of a set whose destination is hard reg $a1. If the address is computed into its own local first (`addr = sym + off;` then `LoadImage(..., addr)`), `off` is a source of a set into `addr` (a pseudo), and only `addr` is a source of the set into $a1 — the relation that produces the $a1 preference would then attach to `addr`, not to `off`.
- probe: Three spellings, each measured with `sandbox --disable all`: (v5) `{ s32 addr = (s32)&D_800A9A24 + off; LoadImage((s32)rect, addr); }`; (v6) `{ u8 *addr = (u8 *)&D_800A9A24 + off; LoadImage((s32)rect, (s32)addr); }`; (v2) no temp at all but integer-typed symbol arithmetic, `LoadImage((s32)rect, (s32)&D_800A9A24 + off)`.
- result: All three are EXACTLY INERT at 11 / 135 insns — byte-identical to the floor-11 form. The interposed local is coalesced away before allocation, so `off` remains a direct source of the set into $a1 in the RTL the allocators see.
- verdict: KILLED. Named-intermediate interposition on the address expression is not a lever for this residual; the relation survives copy coalescing.

## [s5] Random permuter sampling from a structurally different chassis than the floor-11 form can reach the `off`-in-$a1 residual (s4 frontier item 3, its named next probe)
- mechanism: s4 recorded the floor-11 basin as spent (both novel finds inside 32 s, nothing in the following ~11 min across 21428 iterations) and prescribed reseeding from a chassis "in which the LoadImage source address is built without a standalone loop-invariant symbol_ref" rather than re-sampling the same one.
- probe: TWO fresh chassis-faithful campaigns, each built by tmp/grind/func_80041BF4/s5/mkwsA.sh and mkwsB.sh (the s4 recipe: full-TU cpp, -G8, -mel, prologue_fix, MASPSX_FLAGS_GP, the .align 3 -> .align 2 sed, multu_pad, per-function extraction; both base builds reproduce the residual insn-for-insn at 135/135). Campaign A: base = v2, the integer-arithmetic address chassis, 6 jobs, 20303 iterations / 664 s. Campaign B: base = v6, the pointer-temp address chassis, 8 jobs, 18966 iterations / 563 s. Both harvested and STOPPED in-session under the fresh-seed rule (>= 9 min with no novel find).
- result: Each campaign produced exactly ONE find, both inside the first 10 seconds (A: output-65-1 at 7.4 s; B: output-65-1 at 9.9 s) and then NOTHING for the remaining ~9 minutes. Both finds re-measured in the real chassis with `sandbox --disable all` are EXACTLY INERT at 11 / 135: A's find is the function-scope `s32 sym_base = (s32)&D_800A9A24;` consumed as `sym_base + off` (the s32 twin of the s4-killed `u8 *` version), B's find is the permuter artifact `((u8 *)(&D_800A9A24)) - (-off)`. As in s4, the permuter's weighted-score improvement (70 -> 65) is an artifact of its own scorer and does not correspond to any byte change.
- verdict: KILLED. Random sampling is now spent across THREE structurally distinct chassis for this function (s4's floor-11 form, s5's integer-arithmetic form, s5's pointer-temp form): 60k+ total iterations, every find inside the first 32 seconds, every find measured inert or already banked. The permuter modality has nothing further to give on this residual — the remaining gap is a single register-allocation decision that random C perturbation demonstrably cannot express.

## Frontier after s5 (floor 11, unchanged)
1. **The residual is unchanged and is still exactly one decision: `off` (pseudo 129) takes $a1 where target gives $a1 to the symbol.** What s5 ADDS is that the two obvious C-level routes to that decision are now both measured dead: naming the symbol base (any scope, either declaration order) makes it WORSE by 5 insns because loop.c hoists the named pointer to a whole-function pseudo and the pressure evicts `fp_ptr` from $s8; and interposing a named address intermediate is inert because the copy is coalesced before allocation. The one route s4 named that s5 did NOT close is the loop.c one: make move_movables DECLINE to hoist the `(set rN (symbol_ref D_800A9A24))` while it stays unnamed. Next probe: read the .loop movable log for regno 139 (`pwsh tools/grinder/dump.ps1 func_80041BF4`) and identify which movable field (m->global / m->done / m->partial / the threading test) would have to differ, then look for a C form that changes THAT field without introducing a named long-lived pointer. This is forensics modality, not permuter, and it is the highest-value remaining axis.
2. **Permuter is spent for this function.** Three structurally distinct chassis, 60k+ iterations, zero byte-moving finds beyond the s4 constant holder. Do not spend another session on random sampling; a future permuter mandate should be redirected, or the campaign should be given explicit PERM_ directives over the loop.c movable question rather than free randomization.
3. **The banked floor-11 constant holder remains UNVETTED** (unchanged from s4): it is a `/* FAKE */`-annotated constant-holder-family construct (.claude/rules/named-local-fake-exception.md) whose FAKE prerequisites include the full modality ladder being demonstrably spent. Permuter is now spent, but forensics and rederive are not. Do not submit on it yet.

## [s5] The symbol pseudo (139) can be made a short-lived block-local allocno inside the loop body by naming it in a block-scope pointer, so that it carries the $a1 preference and — by allocno ordering — takes $a1 first, forcing `off` down REG_ALLOC_ORDER to $v0 exactly as target has it. (This was s4 frontier item 1's named next probe.)
- mechanism: s4 established that BOTH allocators route `off` to $a1 from the single relation (set (reg:SI 5 a1) (plus (reg/v:SI 129) (reg:SI 139))), and that pseudo 139 never competes because loop.c hoists it out of both loops and global.c leaves it unallocated (reload rematerializes it into $t0). Target's own shape is the mirror image — lui a1,%hi(D_800A9A24) / addiu a1,a1,%lo / addu a1,v0,a1 INSIDE the loop body at .L80041D3C — so in target the SYMBOL holds $a1 and `off` holds $v0. Naming the base in a block-local pointer was the predicted route to making 139 a competing block-local qty.
- probe: Two spellings measured with `sandbox --disable all` in the real chassis plus a normalized objdump against target.o: (v1) `{ u8 *sp = (u8 *)&D_800A9A24; LoadImage((s32)rect, (s32)(sp + off)); }` with the pointer declared AFTER `off`; (v4) the identical pointer declared BEFORE `off` in the loop-body block, since declaration order is the s4-named lever for qty_sugg_compare ordering.
- result: BOTH are 47 at 140 insns — five instructions WORSE than the floor-11 form, and identical to each other, so declaration order is irrelevant. The objdump names the mechanism: the named pointer is loop-INVARIANT, loop.c hoists it to a function-wide pseudo that needs a callee-saved register, and the resulting pressure evicts `fp_ptr` from $s8 to the stack — frame 96 vs target's 88, `sw v0,32(sp)` / `lh v1,8(v0)` where target has `move s8,v0` / `lh v1,8(s8)` — plus the extra spill/reload traffic.
- verdict: KILLED

## [s5] Interposing a named address intermediate between `off` and the hard-$a1 LoadImage argument breaks the source-of-a-set-into-$a1 relation that both allocators key on, so `off` loses the $a1 preference to the intermediate.
- mechanism: s4 showed the preference comes from `off` being a direct register SOURCE of a set whose destination is hard reg $a1 (local-alloc combine_regs -> qty_phys_sugg at local-alloc.c:1857-1896; global.c record_one_conflict -> hard_reg_preferences at global.c:1728/1747). Computing the address into its own local first makes `off` a source of a set into a PSEUDO, and only that pseudo a source of the set into $a1.
- probe: Three spellings each measured with `sandbox --disable all`: (v5) `{ s32 addr = (s32)&D_800A9A24 + off; LoadImage((s32)rect, addr); }`; (v6) `{ u8 *addr = (u8 *)&D_800A9A24 + off; LoadImage((s32)rect, (s32)addr); }`; (v2) the temp-free integer-arithmetic form `LoadImage((s32)rect, (s32)&D_800A9A24 + off)`.
- result: All three are EXACTLY INERT at 11 / 135 insns, byte-identical to the floor-11 form. The interposed local is coalesced away before allocation, so `off` remains a direct source of the set into $a1 in the RTL the allocators actually see.
- verdict: KILLED

## [s5] Random permuter sampling from a structurally different chassis than the floor-11 form can reach the `off`-in-$a1 residual (s4 frontier item 3's named next probe: reseed from a chassis in which the LoadImage source address is built without a standalone loop-invariant symbol_ref).
- mechanism: s4 recorded the floor-11 basin as spent (both novel finds inside 32 s, nothing across the following ~11 min / 21428 iterations) and prescribed reseeding rather than re-sampling. Two new chassis-faithful workspaces were built with the s4 recipe (full-TU cpp, -G8, -mel, prologue_fix, MASPSX_FLAGS_GP, the .align 3 -> .align 2 sed, multu_pad, per-function extraction); both base builds reproduce the residual insn-for-insn at 135/135.
- probe: Campaign A `s5-intaddr` on the integer-arithmetic address chassis (base = v2, 6 jobs, 20303 iterations / 664 s, base weighted score 70) and campaign B `s5-ptrtemp` on the pointer-temp chassis (base = v6, 8 jobs, 18966 iterations / 563 s, base weighted score 70), both launched and harvested through tools/permuter_campaign.py with telemetry, both waited on IN-turn via `wait`, both STOPPED in-session under the fresh-seed rule. Every find re-measured in the real chassis with `sandbox --disable all`.
- result: Each campaign produced exactly ONE find, both inside the first 10 seconds (A: output-65-1 at 7.4 s; B: output-65-1 at 9.9 s), then nothing for the remaining ~9 minutes — the fresh-seed exhaustion signature. Both finds are EXACTLY INERT at 11 / 135 in the real chassis: A's is the function-scope `s32 sym_base = (s32)&D_800A9A24;` consumed as `sym_base + off` (the s32 twin of the `u8 *` form s4 already killed), B's is the artifact spelling `((u8 *)(&D_800A9A24)) - (-off)`. As in s4 the weighted-score gain (70 -> 65) is an artifact of the permuter's own scorer, not a byte change.
- verdict: KILLED

## [s6] forensics — hypothesis log

### H-s6-1 — "the $a1 theft can be broken by making $a1 LIVE inside `off`'s range"
(inherited from s3 frontier item 1, restated by s4 as "the remaining escape is a
CONFLICT, not a preference")
- Probe: instrumented cc1 (BB2_SUGG_DEBUG/BB2_QTY_DEBUG) on the floor-11 chassis +
  read of find_free_reg (local-alloc.c:2168-2172).
- Result: `off` (pseudo 130) has born=6 dead=12, i.e. exactly three RTL insns —
  189 (`sll`), 192 (`idx++`), 227 (`a0 = fp+24`). The conflict scan is half-open, so
  the $a1-setting insn at index 12 is excluded by construction, and `used` comes back
  {0,1,4,26..67} with reg 5 absent. Inserting an $a1 use into that window costs an
  instruction target does not have; extending `off` past index 12 crosses the
  LoadImage call and flips `used` to call_used_reg_set, which excludes $v0 too.
- Verdict: **KILLED.** Both ends of the conflict route are closed.

### H-s6-2 — "in a cross-block regime, commuting the address sum moves global.c's
hard-reg preference off `off` (set_preference only looks at XEXP(src,0))"
- Probe: read global.c:1680-1745, then build `off + (u8 *)&D_800A9A24` and dump
  .lreg with the canonical cc1 (tmp/grind/func_80041BF4/s6/rtlord.sh).
- Result: the premise about set_preference is TRUE (it reduces a non-REG src to its
  first operand), but the lever is unreachable: the RTL is byte-identical for both C
  operand orders — `(plus:SI (reg/v:SI 130) (reg:SI 140))`, `;; Register 130 in 5.`
  The symbol is force_reg'd into its own pseudo by expand in a separate preceding
  insn (.cse insn 223), so `off` is always operand 0 of the sum regardless of how the
  C is written.
- Verdict: **KILLED.** This also supplies the missing mechanism for s4's observation
  that global.c re-derives the same preference in every cross-block form.

### H-s6-3 — "combine.c is the pass that produces the divergence-causing fact"
- Probe: side-by-side of the .flow and .combine dumps at insn 225/229.
- Result: CONFIRMED. .flow carries `(set (reg 139) (plus 130 140))` +
  `(set (reg 5 a1) (reg 139))`; .combine carries the single merged
  `(set (reg 5 a1) (plus (reg 130) (reg 140)))`. local-alloc and global.c are
  consumers of that fact, not its authors.
- Verdict: **CONFIRMED** (pass attribution corrected for the whole ledger).

### H-s6-4 — "even if pseudo 140 were block-local it would lose the race for $a1"
- Probe: read qty_sugg_compare (local-alloc.c) and apply it to the measured numbers.
- Result: FALSIFIED — 140 would WIN. Both qtys would carry nsugg=1 (first sort key a
  tie), and the priority key `floor_log2(n_refs)*n_refs*size/(death-birth)*10000`
  gives a short-lived 140 60000 against `off`'s measured 20000; higher priority is
  allocated first. 140 takes $a1, `off` falls to the plain REG_ALLOC_ORDER pass and
  its first free register is $v0 — target exactly, including the
  `addu $a1,$v0,$a1` operand shape.
- Verdict: **KILLED as an objection**; the frontier item survives with a numeric
  prediction a future session can check in one build.

### Frontier after s6 — ONE item, unchanged in target but now fully instrumented
Stop loop.c's move_movables from hoisting insn 327,
`(set (reg:SI 140) (symbol_ref/v:SI ("D_800A9A24")))`, out of the inner loop body.
Everything downstream of that is now measured or derived (H-s6-4). s5 closed the
NAMING route (any C name for the loop-invariant address lengthens its live range to
whole-function and costs 5 insns via $s8 eviction), so the change must come from the
movable's CLASSIFICATION, not from a declaration. Unprobed sub-questions, in order:
  (a) loop.c's own dump (`-dL` / the loop_dump_stream "savings N" lines) is NOT in the
      -da set that tools/grinder/dump.ps1 produces; getting move_movables' per-movable
      log for this loop is the single highest-value unread artifact left. Check
      whether cc1 can be invoked with the loop dump enabled without editing tools/.
  (b) the gate is `already_moved[regno] || (threshold * savings * m->lifetime) >=
      insn_count || (m->forces && m->forces->done && n_times_used[...] == 1)`
      (loop.c:1631) with threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs);
      the inner loop HAS calls, so threshold is ~31 and the product dwarfs
      insn_count=37. The reachable-from-C fields are `savings` and `lifetime`, not
      `threshold`.
  (c) whether any C form makes the address non-invariant WITHOUT an opaque-arithmetic
      construct. If the honest answer is no, this axis is dead and the function
      should move to the `rederive` / `synthesis` modality rather than more
      allocation work.

## [s6] The $a1 theft from `off` can be broken by making $a1 LIVE inside `off`'s live range (s3 frontier item 1, restated by s4 as 'the remaining escape is a CONFLICT, not a preference').
- mechanism: find_free_reg excludes any hard reg present in regs_live_at[ins] for ins in [born_index, dead_index) (local-alloc.c:2168-2172); if $a1 were live in that window the just_try_suggested pass would fail and `off` would fall through to the plain REG_ALLOC_ORDER pass.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1, BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1) on the floor-11 chassis, plus a read of find_free_reg. Raw output: tmp/grind/func_80041BF4/s6/sugg_base.txt.
- result: SUGGDBG-QTY blk=10 qty=0 reg1=130 birth=6 death=12 refs=6 nsugg=1 sugg=5, ncopysugg=0; SUGGDBG-FFR used=0,1,4,26..67 (hard reg 5 ABSENT); QTYDBG-SUGG ord=5 got=5. `off`'s range is exactly three RTL insns (189 sll, 192 idx++, 227 a0=fp+24); the $a1-setting insn is the death insn at index 12 and the scan is half-open, so $a1 can never appear in `used`. Adding an $a1 use inside that window costs an instruction target does not have; extending the death past index 12 necessarily crosses the LoadImage call, which flips `used` to call_used_reg_set and excludes $v0 as well.
- verdict: KILLED

## [s6] In a cross-block regime for `off`, commuting the address sum in C moves global.c's hard-reg preference off `off`, because set_preference inspects only the FIRST operand of a non-REG src.
- mechanism: global.c:1680 `if (GET_RTX_FORMAT (GET_CODE (src))[0] == 'e') src = XEXP (src, 0), copy = 0;` — for a (plus A B) only A can receive the preference toward the hard destination.
- probe: Read global.c:1660-1745, then built `off + (u8 *)&D_800A9A24` (banked as rejected/loadimage-address-commuted-c-order-does-not-reach-rtl.c) and dumped .lreg with the canonical cc1 via tmp/grind/func_80041BF4/s6/rtlord.sh.
- result: The premise about set_preference is TRUE but the lever is unreachable: both C operand orders give byte-identical RTL — `(plus:SI (reg/v:SI 130) (reg:SI 140))` with `;; Register 130 in 5.` The symbol is force_reg'd into its own pseudo by expand in a separate preceding insn (visible as insn 223 in .cse), so `off` is always operand 0 of the sum however the C is written. This also supplies the missing mechanism behind s4's observation that global.c re-derives the same preference in every cross-block form.
- verdict: KILLED

## [s6] combine.c — not local-alloc.c or global.c — is the pass that produces the fact both allocators consume (`off` is a direct register source of a set whose destination is hard reg $a1).
- mechanism: combine propagates the address-sum pseudo into the following hard-register argument move and deletes the intermediate copy.
- probe: Side-by-side read of tmp/grind/func_80041BF4/dumps/text1a_post.flow and .combine at insns 225/229.
- result: CONFIRMED. .flow carries `(insn 225 (set (reg:SI 139) (plus (reg/v:SI 130) (reg:SI 140))))` plus `(insn 229 (set (reg:SI 5 a1) (reg:SI 139)))`; .combine carries the single merged `(insn 229 (set (reg:SI 5 a1) (plus (reg/v:SI 130) (reg:SI 140))))` with insn 225 deleted. Every prior session attributed this to the allocators, which are only consumers of the fact.
- verdict: CONFIRMED

## [s6] Even if pseudo 140 (the D_800A9A24 symbol_ref) were made block-local, it would lose the race for $a1 against `off`, so the surviving loop.c frontier item is not worth pursuing.
- mechanism: block_alloc orders quantities with qty_sugg_compare: fewer suggestions first (sugg = ncopysugg ? ncopysugg : nsugg * FIRST_PSEUDO_REGISTER), then priority = floor_log2(n_refs) * n_refs * size / (death - birth) * 10000, higher priority allocated first.
- probe: Read qty_sugg_compare in local-alloc.c and applied it to the measured block-10 numbers (`off`: refs=6, death-birth=6; a block-local symbol pseudo would carry the same loop-depth-weighted refs=6 over a 2-index range).
- result: FALSIFIED — 140 would WIN. Both tie at nsugg=1 on the first key; on the priority key 140 scores floor_log2(6)*6/2*10000 = 60000 against `off`'s measured floor_log2(6)*6/6*10000 = 20000. 140 takes $a1, `off` then fails the just_try_suggested pass and the plain REG_ALLOC_ORDER pass hands it the first free register, $v0 — target's assignment exactly, and it also reproduces target's `lui $a1 / addiu $a1 / addu $a1,$v0,$a1` operand shape.
- verdict: KILLED

## [s7] forensics — the loop.c hoist item, KILLED

**H-s7-1 (KILLED).** "Stopping loop.c's move_movables from hoisting the
`(set (reg) (symbol_ref D_800A9A24))` out of the inner loop is reachable from C."
Mechanism read from source and measured: the only gate field (loop.c:1631) with a usable
sign is insn_count — savings and lifetime are already 1 and only help the hoist if raised;
already_moved / m->forces only ever force moves. Threshold measured at 61-62 by synthetic
bisection, minus 3 per prior moved movable (loop.c:1719/1904), so the inner loop would
have to reach >= 56 real RTL insns against its actual 37. Every way of getting there is
closed: real insns cost bytes (+19 on a 135-insn function); in-loop invariant work is
byte-neutral but would leave hoisted insns in a preheader, and target's preheaders are
bare; dead invariant work is deleted by cse.c before loop.c sees the loop.
VERDICT: KILLED.

**H-s7-2 (CONFIRMED).** "If the hoist were suppressed, the symbol pseudo takes $a1 and
`off` falls to $v0 — target's allocation." s6 derived this from qty_sugg_compare; s7
MEASURED it twice: once by padding the inner loop past the threshold (symbol pseudo 156
-> $a1) and once with both loops goto-spelled (symbol 137 -> $a1 AND `off` 127 -> $v0,
exactly target). VERDICT: CONFIRMED — but only reachable in chassis that cost bytes or
the frame.

**H-s7-3 (KILLED).** "A goto-spelled loop, which carries no NOTE_INSN_LOOP_BEG, removes
the loop from loop.c's view for free." Measured: inner-only goto = 46 at 136 insns and the
symbol is still hoisted out of the remaining outer loop (54 insns vs an effective threshold
of 55/56 — short by two); both-loops goto = 43 at 136 insns with target's allocation but
frame 80 vs 88 and the callee-save rotation lost. VERDICT: KILLED as a floor-improving
form on this chassis.

**H-s7-4 (KILLED).** "The original wrote the r/g/b /255 conversions inside the loop, which
would be byte-neutral after hoisting and would supply the 7 extra movables the gate needs."
Refuted by target's bytes: the conversions are at 0x78-0xd8, BEFORE the `jal func_800486FC`
at 0xd8; a hoisted in-loop conversion must land in the loop preheader, i.e. after the
func_8004881C block (0xe0-0xfc). VERDICT: KILLED.

## [s7] Stopping loop.c's move_movables from hoisting the unnamed (set rN (symbol_ref D_800A9A24)) out of the inner loop is reachable from honest C.
- mechanism: The gate is loop.c:1631 `already_moved[regno] || (threshold * savings * m->lifetime) >= insn_count`, with m->savings = n_times_used (loop.c:793) and threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs) (loop.c:532), decremented by 3 after every moved movable (loop.c:1719/1904). For this movable savings = 1 and lifetime = 1, both already minimal and both only making the hoist MORE likely if raised; already_moved and m->forces can only force a move. insn_count is the only field whose sign can help.
- probe: Read the move_movables per-movable log in the -da .loop dump (it IS in the -da set, contrary to the s6 ledger note). Bisected the threshold on a synthetic call-containing loop carrying one symbol_ref movable (tmp/grind/func_80041BF4/s7/gen_thr*.py + thr*.sh): 61 real insns -> moved, 63 -> not desirable. Then padded the real inner loop (s7/pad.py): 46 insns -> moved, 54 -> not desirable, 62 -> not desirable.
- result: The symbol is the THIRD movable moved out of the 37-insn inner loop, so it is tested against an effective threshold of 55-56. Making the gate fail needs the inner loop at >= 56 real RTL insns, i.e. +19 instructions inside a 135-instruction function. Byte-neutral ways to raise insn_count are both closed: in-loop loop-invariant work is byte-neutral (measured on licm_in.c vs licm_out.c: same instructions, +7 insn_count, +7 movables = -21 threshold) but would leave hoisted insns in a preheader, and target's preheaders are bare (its three /255 conversions sit at 0x78-0xd8, BEFORE the jal func_800486FC at 0xd8, not after the func_8004881C block); dead invariant work never reaches loop.c at all (four dead divisions in the inner loop left insn_count at exactly 37 - cse.c deletes them first).
- verdict: KILLED

## [s7] If the hoist were suppressed, the symbol pseudo would take $a1 first and push `off` down REG_ALLOC_ORDER to $v0 - target's exact allocation (s6 derived this from qty_sugg_compare's priority key).
- mechanism: A block-local symbol pseudo ties `off` on qty_sugg_compare's suggestion-count key and wins the priority key (60000 vs the measured 20000), takes $a1 on the just_try_suggested pass, and leaves $v0 as the first free register for `off`.
- probe: Two independent in-situ measurements of the instrumented/plain cc1's .lreg: (a) inner loop padded to 54 insns so the movable is declined; (b) both loops goto-spelled so loop.c sees no loop in this function at all.
- result: (a) `;; Register 156 in 5.` - the symbol pseudo takes $a1 and displaces `off`. (b) `;; Register 137 in 5.` and `;; Register 127 in 2.` with the address insn reading (set (reg:SI 5 a1) (plus (reg/v:SI 127) (reg:SI 137))): symbol in $a1, `off` in $v0 - target's allocation exactly. s6's derivation is confirmed end-to-end.
- verdict: CONFIRMED

## [s7] A goto-spelled loop carries no NOTE_INSN_LOOP_BEG, so loop.c never scans it and the hoist disappears for free.
- mechanism: loop.c enumerates loops from the front end's loop notes; a label+goto backedge produces none, so scan_loop/move_movables never see the body.
- probe: Two forms built and measured with sandbox --disable all plus the .loop and .lreg dumps: inner loop goto-spelled with the outer left as the real do-while; and both loops goto-spelled.
- result: Inner-only goto = 46 at 136 insns: loop.c then sees ONE loop (the outer, 54 real insns) and STILL hoists the symbol, missing the 55-56 effective threshold by exactly two insns. Both-loops goto = 43 at 136 insns: allocation is target's, but the frame collapses to 80 vs target's 88, the callee-save rotation is lost (s4/s5/s6/s7 permuted) and a nop appears where target has lhu v1,0(s0). Both banked in rejected/.
- verdict: KILLED

## [s7] The original source wrote the r/g/b (aN<<12)/255 conversions inside the loop; hoisting would put them back byte-identically while supplying the extra movables and insn_count the gate needs.
- mechanism: loop.c hoists an invariant to the loop preheader, so an in-loop spelling and a pre-loop spelling emit the same instructions; the in-loop spelling additionally raises insn_count and decrements threshold by 3 per hoisted movable (measured: one such division adds 7 movables, -21 threshold).
- probe: Compared target's own instruction order around the conversions and the calls (objdump of tmp/grind/func_80041BF4/s5/wsA/target.o).
- result: Target computes all three conversions at 0x78-0xd8, BEFORE the jal func_800486FC at 0xd8. A hoisted in-loop conversion must land in the loop preheader, i.e. after the func_8004881C block at 0xe0-0xfc. Target's outer preheader (move s2,zero at 0x100) and inner preheader (j 1ac / move s1,zero at 0x140) contain no hoisted invariant at all, so the original had no extra loop-invariant work in either loop.
- verdict: KILLED
