# Hypothesis ledger — func_80016E60

## [s1] H1 CONFIRMED — the `1` of `1 << (select-3)` is hoisted only because loop.c combine_movables merges the two arms' constants (threshold 61 vs insn_count 127); a multi-set mask variable (`mask = 1; mask <<= shift;` in both arms, function scope) keeps `li v1,1` inside the arms with the target's `addiu; li` order. Measured 46 -> 34 (v1 -> v3). Evidence E-s1-3. KILLED sub-forms (instance, chassis asm-until-matched, no FAKE present): single-set `mask = 1 << shift` (re-hoists, 21 on carrier); block-scoped mask (order flips to `li; addiu`, 17 on carrier).

## [s1] H2 CONFIRMED — prev/next arms are conditional expressions: `select = (select == 0) ? limit - 1 : select - 1;` / `select = (select == limit - 1) ? 0 : select + 1;` reproduce the a1/v1 temp seats exactly. Measured 34 -> 30 (v3 -> v4). Evidence E-s1-4.

## [s1] H3 KILLED (instance; chassis asm-until-matched; no FAKE) — a `goto loop;` spelling of the main loop. Mechanism guess: no loop notes -> no LICM/loop-depth weighting. Measured 54 (v1 46): the exit-arm layout changes, and the `1` hoist is not what governs the arm placement. rejected/goto-loop-breaks-exit-arm-layout-54.c.

## [s1] H4 CONFIRMED (mechanism) / OPEN (honest spelling) — env/select seat swap is global.c allocno priority: env 4615 < select 6878; env needs >= 8 weighted refs (one more real in-loop use) or livelen <= 17. do-while(0) carrier on {PutDispEnv, PutDrawEnv} measures 11 (E-s1-6). Honest levers NOT yet measured: (i) a genuine 4th in-loop use of env — none identified from the bytes (env feeds only +0x5C and PutDrawEnv; DrawOTag uses arg0+0x408C via $s5); (ii) lowering select's refs below 18 weighted (needs ~5 fewer real in-loop uses — every use is byte-visible, judged implausible but unmeasured); (iii) shortening env's livelen via fewer RTL insns between its birth and PutDrawEnv on BOTH if/else paths (e.g. a different spelling of the special-arm argument `select | (D_800A3788 << 16)` or of the two func_8005C8A8 calls; needs 9 fewer counted insns — unmeasured). KILLED (instance, no FAKE): copy alias `disp = env` (cse deletes it before flow; 30 unchanged).

## [s1] H5 OPEN — bit-arm seats (chain $a0 / shift $v0 / mask $v1). Mechanism E-s1-8: local-alloc seats the chain in $v0 before global.c seats shift/mask. Six spellings measured on the carrier (14-17, none < 11). Untried: (a) giving the chain a hard-reg suggestion for $a0 (qty_phys_copy_sugg) — would need the or-result to be an argument copy, no such call exists in the bytes; (b) making shift+mask local (single block) AND higher priority than the chain — s4 (all block-scoped) = 15, but the order flip (`li` first) suggests block-scoped temps change expand order; try block-scoped with `mask` declared BEFORE `shift`, and the `mask <<= select - 3` no-shift form inside a block; (c) check whether the v5 prologue pair reorder (E-s1-5c) is coupled to these seats via .sched2.

## [s1] The `li s4,1` hoist comes from loop.c combine_movables merging the two arms' single-use `1` constants (threshold 61 vs insn_count 127); a function-scope multi-set `mask = 1; mask <<= shift;` in both arms keeps the constant inside the arms with the target's addiu-then-li order.
- mechanism: loop.c:532 threshold=(1)*(1+60)=61 (hard-float build: 32 FP regs non-fixed); loop.c:1631 threshold*savings*lifetime >= insn_count; combine_movables loop.c:1244-1284 merges equal CONST_INT movables (life 1+1, savings 1+1 -> 244 >= 127); n_times_set != 1 makes mask non-movable
- probe: v1 (plain 1 << (select-3)) vs v3 (mask=1; mask<<=...) sandbox --disable all; .loop dump lines 'Insn 385 ... moved to 516' / 'Insn 419 ... matches 385'
- result: 46 -> 34; single-set `mask = 1 << shift` re-hoists (21 on the carrier vs 11)
- verdict: CONFIRMED

## [s1] prev/next arms are `select = (select == 0) ? limit - 1 : select - 1;` and `select = (select == limit - 1) ? 0 : select + 1;` - reproduces the target insns and the a1/v1 temp seats exactly.
- mechanism: COND_EXPR expands into a fresh target pseudo per arm; jump.c 'if (...) x = a; else x = b' -> 'x = b; if (...) x = a' produces the delay-slot unconditional form
- probe: v3 (explicit next temp, v1 seat in arm 1) vs v4 (?:) objdiff
- result: 34 -> 30, arm insns byte-identical
- verdict: CONFIRMED

## [s1] A `goto loop;` spelling of the main loop, measured on the asm-until-matched chassis with no FAKE constructs present.
- mechanism: no NOTE_INSN_LOOP_BEG -> no LICM, loop_depth weight 1
- probe: v2 sandbox
- result: 54 (worse than v1's 46): exit-arm layout changes, hoist not the governing factor
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis (INCLUDE_ASM main, 0 rules), no FAKE constructs, body_v2_goto.c

## [s1] A pure copy alias `disp = env; PutDispEnv(disp + 0x5C);` measured as a ref lift for env's global.c priority, on the asm-until-matched chassis with no FAKE present.
- mechanism: cse.c replaces the copy's uses and flow deletes the dead copy before reg_n_refs is counted
- probe: v8 sandbox + objdiff
- result: 30 unchanged; inert
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, no FAKE constructs, body_v8_copyalias.c

## [s1] env/select seat swap is global.c allocno priority (env 4615 < select 6878; env needs >= 8 weighted refs or livelen <= 17); a do-while(0) wrap of {PutDispEnv, PutDrawEnv} raises env to 8 refs / 9230 and seats env=$s0, select=$s1.
- mechanism: flow.c reg_n_refs += loop_depth; global.c allocno_compare priority floor_log2(refs)*refs/livelen; ra_solver inverse ranks refs_up env 6->8 first
- probe: tools/ra_solver extract + inverse --swap 74,75; v5 carrier sandbox
- result: v5 = 11 (only bit-arm seats + a prologue pair reorder remain). Mechanism measurement only - NOT a submission: lever exhaustion incomplete (H4 (ii)/(iii) unmeasured)
- verdict: CONFIRMED

## [s2] H4(iii) — shortening env's live range to the <=17 that global.c priority needs, by re-associating the statements between env's birth and PutDrawEnv (measured on the asm-until-matched chassis, candidate.c/v4 and the two split forms, no FAKE present).
- mechanism: flow.c:1685/2087 count one live-length unit per insn on every path; global.c allocno_compare pri = floor_log2(refs)*refs/livelen
- probe: read the target's env birth (asm/funcs/func_80016E60.s, `addu $s0,$v0,$v1` in ClearOTagR's delay slot, both operands CALL_USED so it cannot have come from after the jal); measured livelen on v4 (26), w1 (48), w2 (41) via tools/ra_solver/extract.py
- result: env's birth is pinned before ClearOTagR, so its range always contains the 9 display calls and their argument insns; the smallest livelen any spelling produced this session was 26. No form measured at or below 17.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis (INCLUDE_ASM main, 0 rules), no FAKE constructs, body_v4.c / body_w1_splitinit.c / body_w2_offfirst.c

## [s2] H6 CONFIRMED — split-init accumulation on env (`env = (u8 *)(idx * 0x4090); env += (s32)&D_800F7438;`) is an HONEST ref-lift that wins the env/select seat swap with no FAKE construct: flow counts 10 weighted refs (pri 7317 > select's 6878) because the second set is arithmetic (cse cannot fold it the way it folded s1's copy alias) and combine merges it only AFTER flow has counted.
- mechanism: flow.c:2087 counts refs before combine runs; global.c allocno_compare then orders env ahead of select; combine.c re-merges the two sets into one addu
- probe: body_w2_offfirst.c sandbox + tools/ra_solver/extract.py (allocdbg: 74 env refs 10 livelen 41 pri 7317 -> $s0; 75 select 27/157/6878 -> $s1)
- result: 33 (vs the 30 baseline). Seat swap achieved honestly; the cost is (a) the merged set emits `sll s0,v0,4; lui v0; addiu v0; addu s0,s0,v0` instead of the target's `sll v0,v0,4; lui v1; addiu v1; addu s0,v0,v1` — one operand is forced into env's own hard reg by the accumulation — and (b) the earlier birth frees ClearOTagR's delay slot, deleting the three reorg.c back-edge peels of `li a1,1` (208 vs 211 insns). Ordering the split the other way (symbol first, w1) makes the first set loop-invariant: livelen 48, pri 6250, no swap, 50.
- verdict: CONFIRMED

## [s2] H7 CONFIRMED (partial byte win) — block-scoping `shift` inside each bit arm (with `mask` at function scope and `bits` block-scoped) makes shift a single-block quantity that local-alloc seats FIRST, producing the target's `addiu $v0,$s1,-3` exactly; the bit-arm residual collapses from a 3-way rotation to a 2-way mask/chain swap.
- mechanism: local-alloc.c block_alloc/qty_compare orders single-block quantities before global.c ever sees them; find_free_reg scans ascending so the first-ordered quantity takes $v0
- probe: body_c3.c (blk shift / fn mask / blk bits) vs the v5 carrier; c3_objdiff.txt no longer contains the addiu line
- result: 14 (v5 carrier is 11). Residual: ours mask=$a0, chain=$v1; target mask=$v1, chain=$a0. Full scope matrix in E-s2-7.
- verdict: CONFIRMED

## [s2] H8 KILLED (instance) — re-ordering the statements inside the bit arm to change the local-alloc birth order, measured on the v5 do-while(0) carrier.
- mechanism guess: qty_compare ties break on qty number (birth order in the block), so source order would decide
- probe: e1..e4, four permutations of {bits = D_800A3788; mask = 1; shift = select - 3; mask <<= shift;}, all three variables block-scoped
- result: all four measure 15 — sched1 canonicalises the arm before local-alloc, so source text order is inert. Local order must be moved via refs or span.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present), body_e1..e4.c

## [s2] H9 KILLED (instance) — widening the do-while(0) wrap to cover the whole display block, measured on the asm-until-matched chassis.
- mechanism: more of the body at loop_depth 3 raises env's and select's refs together
- probe: body_y1_wrapall.c (env set .. DrawOTag) and body_y2_wrapcalls.c (ClearOTagR .. DrawOTag)
- result: 30 and 29 respectively, vs 11 for the narrow wrap around {PutDispEnv, PutDrawEnv}. The narrow wrap remains the only useful extent.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, do-while(0) FAKE construct present (wrap extent is the variable), body_y1/y2

## [s2] H10 KILLED (instance) — single-set `mask = 1 << shift` block-scoped per arm, measured on the v5 carrier.
- mechanism: block scoping makes each arm's mask a distinct pseudo, but the `1` is still a fresh single-use loop invariant in each arm
- probe: body_g1_singleset_allblk.c
- result: 23 — combine_movables still merges the two arms' constants and loop.c hoists (same mechanism as E-s1-3 at function scope)
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present), body_g1_singleset_allblk.c

## [s2] Shortening env's live range to the <=17 that global.c priority needs, by re-associating the statements between env's birth and PutDrawEnv.
- mechanism: flow.c:1685 / flow.c:2087 count one live-length unit per insn on every path; global.c allocno_compare pri = floor_log2(refs)*refs/livelen. The target's env set `addu $s0,$v0,$v1` sits in ClearOTagR's delay slot and both operands are CALL_USED, so it cannot have been moved from after the jal - env's birth is pinned before the display calls.
- probe: body_v4.c / body_w1_splitinit.c / body_w2_offfirst.c sandbox + tools/ra_solver/extract.py livelen readout
- result: livelen measured 26 (v4), 41 (w2), 48 (w1); nothing at or below 17. The span always contains the 9 display calls plus their argument insns on both if/else paths.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis (INCLUDE_ASM main, 0 rules), no FAKE constructs, body_v4.c/body_w1_splitinit.c/body_w2_offfirst.c

## [s2] Split-init accumulation on env (env = (u8 *)(idx * 0x4090); env += (s32)&D_800F7438;) raises env's flow reference count from 6 to 10 and wins the env/select seat swap with no FAKE construct present.
- mechanism: flow.c:2087 counts reg_n_refs before combine runs, and the first set is arithmetic rather than a copy so cse cannot fold it (unlike the s1 copy-alias kill); global.c allocno_compare then orders env (pri 7317) ahead of select (6878); combine.c re-merges the two sets afterwards.
- probe: body_w2_offfirst.c sandbox + tools/ra_solver/extract.py (allocdbg: 74 env refs 10 livelen 41 pri 7317 -> $s0; 75 select 27/157/6878 -> $s1)
- result: Seat swap achieved honestly; score 33 vs the 30 baseline. Cost: combine parks one operand in env's own hard reg, so the set emits `sll s0,v0,4; lui v0; addiu v0; addu s0,s0,v0` instead of the target's `sll v0,v0,4; lui v1; addiu v1; addu s0,v0,v1`, and the earlier birth frees ClearOTagR's delay slot, deleting the three reorg.c back-edge peels of `li a1,1` (208 insns vs 211).
- verdict: CONFIRMED

## [s2] Block-scoping shift inside each bit arm (mask at function scope, bits block-scoped) makes it a single-block quantity that local-alloc seats first, producing the target's `addiu $v0,$s1,-3` exactly.
- mechanism: local-alloc.c block_alloc/qty_compare orders single-block quantities before global.c sees them; find_free_reg scans ascending so the first-ordered quantity takes $v0.
- probe: body_c3.c on the v5 carrier; c3_objdiff.txt no longer contains the addiu line
- result: 14 (the carrier alone is 11). The bit-arm residual collapses from a 3-way rotation to a 2-way swap: ours mask=$a0/chain=$v1, target mask=$v1/chain=$a0.
- verdict: CONFIRMED

## [s2] Re-ordering the statements inside the bit arm to change local-alloc's birth order, with shift, mask and bits all block-scoped.
- mechanism: qty_compare ties break on qty number (birth order within the block), so source statement order was expected to decide which quantity takes $v0.
- probe: body_e1.c..body_e4.c - four permutations of {bits = D_800A3788; mask = 1; shift = select - 3; mask <<= shift;}
- result: all four measure 15; sched1 canonicalises the arm before local-alloc, so source text order is inert. Local order has to be moved through refs or span.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present), body_e1..e4.c

## [s2] Widening the do-while(0) wrap to cover the whole display block instead of just {PutDispEnv, PutDrawEnv}.
- mechanism: putting more of the body at loop_depth 3 raises env's weighted references further, at the cost of raising select's too.
- probe: body_y1_wrapall.c (env set .. DrawOTag) and body_y2_wrapcalls.c (ClearOTagR .. DrawOTag)
- result: 30 and 29 respectively, against 11 for the narrow wrap. The wider wraps re-time the loop and lose more than the seat is worth.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, do-while(0) FAKE construct present (wrap extent is the variable), body_y1_wrapall.c/body_y2_wrapcalls.c

## [s2] Single-set mask = 1 << shift written block-scoped inside each arm, to lower mask's local-alloc reference count.
- mechanism: block scoping gives each arm a distinct mask pseudo, which was expected to stop loop.c combine_movables from merging the two arms' 1 constants.
- probe: body_g1_singleset_allblk.c
- result: 23 - the 1 is still a fresh single-use invariant in each arm, the pair still merges and still hoists (same mechanism as E-s1-3 at function scope).
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present), body_g1_singleset_allblk.c

## [s3] H11 CONFIRMED — the bit-arm chain (lbu/or/sb) must be a PER-ARM BLOCK-LOCAL quantity, because the target gives it $a0 in the `|=` arm and $v0 in the `&= ~` arm; any spelling that shares one `bits` variable across the two arms is a single global.c allocno and can only reproduce one arm.
- mechanism: global.c assigns one hard register per allocno; local-alloc.c allocates per basic block, so two per-arm quantities can land on different hard registers. A function-scope variable whose references all lie in one block is still local-allocated (reg_basic_block), so function scope is not a way to force the global path.
- probe: r1 (bits referenced only in the `|=` arm) and r2 (only in the `&= ~` arm) + local_extract QTYDBG rows — pseudo 81 (the function-scope `bits`) appears as a LOCAL quantity row in blk 22 / blk 24 respectively; target register readout from asm/funcs/func_80016E60.s (0x800170A0 `lbu $a0`, 0x800170E8 `lbu $v0`)
- result: r1 = 15, r2 = 17; every shared-chain spelling (q2/u1/u2/u3 = 11, c3 = 14, f1 = 17, v5 = 11) reproduces at most one arm
- verdict: CONFIRMED

## [s3] H12 CONFIRMED — the "q2" arm spelling (block-scoped `shift` AND block-scoped `mask` per arm, function-scope `bits`) is the only measured spelling whose `|=` arm reproduces the target's bit-arm REGISTERS exactly (shift $v0, mask $v1, chain $a0).
- mechanism: two block-local quantities (shift pri 40000, mask pri 30000) take $v0 and $v1 in ascending find_free_reg order, and the global `bits` allocno is pushed to $a0 by its conflicts with both.
- probe: body_q2_blkshift_blkmask_fnbits.c on the do-while(0) carrier (score 11, q2_objdiff.txt) + objdump readout of the arm; honest form body_h1_q2_honest.c (score 30, h1_objdiff.txt)
- result: carrier 11 with the `|=` arm's registers byte-exact; residual is the `li`-before-`addiu` emission order in both arms plus the `&= ~` arm's chain in $a0. Honest 30 — same score as the s2 candidate but strictly closer, so it is the new candidate.c.
- verdict: CONFIRMED

## [s3] Producing the arm-A local-alloc seat order shift, mask, chain from three block-local quantities whose measured sched1 birth order is mask, chain, shift (or mask, shift, chain when no `bits` variable exists), measured on the v5 do-while(0) carrier and on the honest chassis.
- mechanism: for next_qty == 3 local-alloc skips qsort and runs the hand-rolled sort at tools/gcc-2.7.2/local-alloc.c:1541-1553, whose three qty_compare calls use the fixed qty indices 0,1,2 while EXCHANGE permutes qty_order; enumerating its three branches shows the reachable permutations are exactly [0,1,2], [0,2,1] and [2,1,0]. With qty0 = mask, qty1 = chain, qty2 = shift the wanted seat order is the permutation [2,0,1]; with qty0 = mask, qty1 = shift, qty2 = chain it is [1,0,2]. Neither is in the reachable set for any priority values.
- probe: body_q1_allblk.c (15), body_p1_blkshift_blkmask_nobits.c (17), body_r1_bitsA_only.c (15), body_r2_bitsB_only.c (17), body_c4.c (26) plus the priority replay of every QTYDBG row from tools/ra_solver/local_extract.py
- result: every three-quantity spelling measured lands on [mask, shift, chain] (v0/v1/a0) or [mask, chain, shift]; none reaches the target seats. The 4-quantity probe body_x1_probe4qty.c does take the real qsort path (local-alloc.c:1502) and orders strictly by priority, but the extra insn stretched mask's span from 8 to 12 and dropped its priority below the chain's, so it landed shift/z/chain/mask (29).
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present) and the honest asm-until-matched chassis; body_q1_allblk.c / body_p1_blkshift_blkmask_nobits.c / body_r1_bitsA_only.c / body_r2_bitsB_only.c

## [s3] Changing the sched1 birth order inside a bit arm — making the `addiu` (shift) the first-born quantity instead of the `li` (mask) — through the source form of the arm.
- mechanism: qty numbers are assigned in birth order within the block, and birth order is the post-sched1 RTL order; changing which insn sched1 places first would change which permutations the 3-element sort can reach.
- probe: nine spellings, each read back through tools/ra_solver/local_extract.py QTYDBG rows — u1/u2/u3 (statement permutations on the q2 shape), w1 (`u8 bits`), w3 (`u32 mask`), w2 (no `shift` variable, `mask <<= select - 3`), t1/t3 (split-init accumulation `shift = select; shift -= 3;`), q1 (all block-scoped)
- result: all nine produce the identical geometry mask birth 10 / death 18 / refs 8, chain 12 / 20 / 8, shift 14 / 16 / 4 (or mask, shift, chain when no `bits` variable exists). Source statement order is inert (extends the s2 H8 kill to the q2 shape) and the split-init form is folded back to one insn by cse before local-alloc runs.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present) and the honest chassis; body_u1.c/body_u2.c/body_u3.c/body_w1_u8bits.c/body_w2_noshiftvar.c/body_w3_umask.c/body_t1_q1_splitshift.c/body_t3_q2_splitshift.c

## [s3] Keeping the bit-arm chain in a function-scope `bits` variable referenced in only ONE arm, so that the other arm gets a fresh compiler temp and the two arms can take different hard registers.
- mechanism guess: a function-scope declaration would keep the pseudo on the global.c path even when all its references fall inside one basic block.
- probe: body_r1_bitsA_only.c and body_r2_bitsB_only.c + local_extract QTYDBG rows
- result: 15 and 17. The QTYDBG rows show the function-scope pseudo (reg1 = 81) as a LOCAL quantity in the arm that references it, so the arm falls back to the three-block-local geometry and its unreachable seat order. Declaration scope does not control the local/global split; the span of the references does.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present), body_r1_bitsA_only.c / body_r2_bitsB_only.c

## [s3] The bit-arm lbu/or/sb chain must be a per-arm block-local quantity, because the target seats it in $a0 in the `|=` arm and $v0 in the `&= ~` arm, so a single `bits` variable shared by both arms (one global.c allocno, one hard register) can reproduce at most one arm.
- mechanism: global.c assigns one hard register per allocno; local-alloc.c allocates per basic block. A function-scope variable whose references all fall in one block is still local-allocated via reg_basic_block, so declaration scope does not control the local/global split.
- probe: body_r1_bitsA_only.c and body_r2_bitsB_only.c measured on the do-while(0) carrier, with tools/ra_solver/local_extract.py QTYDBG rows read back; target registers read from asm/funcs/func_80016E60.s (0x800170A0 lbu $a0, 0x800170E8 lbu $v0).
- result: r1 = 15, r2 = 17; the function-scope pseudo 81 shows up as a LOCAL quantity row in the arm that references it. Every shared-chain spelling measured (q2/u1/u2/u3 = 11, c3 = 14, f1 = 17, v5 = 11) reproduces at most one arm.
- verdict: CONFIRMED

## [s3] The q2 arm spelling - block-scoped `shift` and block-scoped `mask` per arm with a function-scope `bits` - reproduces the target's bit-arm registers exactly in the `|=` arm (shift $v0, mask $v1, chain $a0).
- mechanism: Two block-local quantities (shift pri 40000 from refs 4 / span 2, mask pri 30000 from refs 8 / span 8) take $v0 and $v1 in ascending find_free_reg order, and the global `bits` allocno is pushed to $a0 by its conflicts with both.
- probe: body_q2_blkshift_blkmask_fnbits.c on the v5 do-while(0) carrier (sandbox + objdump of the arm) and the honest wrap-free form body_h1_q2_honest.c.
- result: Carrier 11 with the `|=` arm byte-exact; honest 30, the same score as the s2 candidate but strictly closer, so it becomes the new candidate.c. Residual: `li` emitted before `addiu` in both arms, and the `&= ~` arm's chain in $a0 instead of $v0.
- verdict: CONFIRMED

## [s3] Producing the arm-A local-alloc seat order shift, mask, chain from three block-local quantities whose measured sched1 birth order is mask, chain, shift (or mask, shift, chain when no `bits` variable exists), measured on the v5 do-while(0) carrier and on the honest chassis.
- mechanism: For next_qty == 3 local-alloc skips qsort and runs the hand-rolled sort whose three qty_compare calls use the FIXED qty indices 0,1,2 while EXCHANGE permutes qty_order; enumerating its branches gives the reachable permutations [0,1,2], [0,2,1], [2,1,0] only. The wanted seat order is the permutation [2,0,1] (or [1,0,2] in the no-`bits` geometry).
- probe: body_q1_allblk.c (15), body_p1_blkshift_blkmask_nobits.c (17), body_r1_bitsA_only.c (15), body_r2_bitsB_only.c (17), plus a hand replay of the sort against every QTYDBG row emitted by tools/ra_solver/local_extract.py for those states.
- result: Every three-quantity spelling measured lands on [mask, shift, chain] or [mask, chain, shift]; none reaches the target seats. The 4-quantity probe body_x1_probe4qty.c does take the real qsort path (local-alloc.c:1502) and orders strictly by priority, but its extra insn stretched mask's span from 8 to 12 and dropped mask below the chain, landing shift/z/chain/mask at 29.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present) and the honest asm-until-matched chassis; body_q1_allblk.c / body_p1_blkshift_blkmask_nobits.c / body_r1_bitsA_only.c / body_r2_bitsB_only.c

## [s3] Changing the sched1 birth order inside a bit arm - making the addiu (shift) the first-born quantity instead of the li (mask) - through the source form of the arm.
- mechanism: Quantity numbers are assigned in birth order within the block and birth order is the post-sched1 RTL order, so a different sched1 placement would change which permutations the 3-element sort can reach.
- probe: Nine spellings, each read back through tools/ra_solver/local_extract.py QTYDBG rows: u1/u2/u3 (statement permutations on the q2 shape), w1 (`u8 bits`), w3 (`u32 mask`), w2 (no `shift` variable, `mask <<= select - 3`), t1/t3 (split-init accumulation `shift = select; shift -= 3;`), q1 (all block-scoped).
- result: All nine produce the identical geometry mask 10/18/refs 8, chain 12/20/refs 8, shift 14/16/refs 4 (or mask, shift, chain with no `bits` variable). Source statement order is inert on the q2 shape too, and cse folds the split-init shift back to one insn before local-alloc runs.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present) and the honest chassis; body_u1.c/body_u2.c/body_u3.c/body_w1_u8bits.c/body_w2_noshiftvar.c/body_w3_umask.c/body_t1_q1_splitshift.c/body_t3_q2_splitshift.c

## [s3] Keeping the bit-arm chain in a function-scope `bits` variable referenced in only ONE arm, so that the other arm gets a fresh compiler temp and the two arms can take different hard registers.
- mechanism: The guess was that a function-scope declaration keeps the pseudo on the global.c path even when all its references lie inside one basic block.
- probe: body_r1_bitsA_only.c and body_r2_bitsB_only.c plus their QTYDBG rows.
- result: 15 and 17. The function-scope pseudo (reg1 = 81) appears as a LOCAL quantity row, so the arm falls back to the three-block-local geometry and its unreachable seat order. The span of the references, not the declaration, decides local vs global.
- verdict: KILLED
- kill_scope: instance
- measured_on: v5 do-while(0) carrier (FAKE present), body_r1_bitsA_only.c / body_r2_bitsB_only.c

## [s4] H13 CONFIRMED - the bit-arm `li`-before-`addiu` divergence is sched1's adjust_priority/birthing_insn_p register-pressure boost, and it fires because a per-arm block-local `shift` has reg_n_sets == 1.
- mechanism: tools/gcc-2.7.2/sched.c:2504 birthing_insn_p returns 1 for a SET whose REG dest is live and whose reg_n_sets[regno] == 1; sched.c:2586 adjust_priority then raises that insn's INSN_PRIORITY to max_priority (0x7F000001 here, inherited from the block-ending jump). sched1 scans bottom-up, so the boosted insn is picked FIRST and therefore EMITTED LAST. `mask` (two sets) and `bits` (two sets) print birth=0 and stay at priority 1, where rank_for_schedule (sched.c:2408) falls through to the INSN_LUID tie-break.
- probe: `pwsh tools/grinder/dump.ps1 func_80016E60` for pass attribution, then the instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SCHED_DEBUG=1 BB2_RANK_DEBUG=1 BB2_PRIO_DEBUG=1 - tmp/grind/func_80016E60/s4/sched.log lines 7788-7812, tmp/grind/func_80016E60/s4/prio.log
- result: `SCHEDDBG ADJPRI insn=390 deaths=0 birth=1 maxpri=2130706433 pri=1` then `PICK clock=5 picked=390 (pri=2130706433 luid=4)` out of the ready list `[390(p=1,l=4) 399(p=1,l=8) 393(p=1,l=5)]`. Removing the boost (function-scope `shift`, reg_n_sets == 2) produces the target's emission order in BOTH arms, measured - see H14.
- verdict: CONFIRMED

## [s4] H14 CONFIRMED - a function-scope `shift` and `mask` (written in both bit arms) removes the birthing boost and makes the bit-arm residual pure register naming with zero insn movement.
- mechanism: writing one variable in both arms gives its pseudo reg_n_sets == 2, so birthing_insn_p is false, no insn in the arm is boosted, and rank_for_schedule's INSN_LUID tie-break emits `addiu, li, lbu` (arm A) and `addiu, li, sllv, lbu` (arm B) - the target's orders.
- probe: tools/sweep_variants.py over the scope grid on the do-while(0) carrier (tmp/grind/func_80016E60/s4/variants/): n1 fn/fn/fn = 17, n2 fn-shift/blk-mask/fn-bits = 17, n3 fn-shift/blk-mask/blk-bits = 17, n4 fn-shift/fn-mask/blk-bits = 11, n5 blk-shift/fn-mask/fn-bits = 17; then tools/objdiff.py on n4
- result: n4 = 11 on the carrier and 30 honest (the same numbers as the s3 candidate) but the diff no longer contains ANY insn position change: it is `addiu a0 -> v0` and `sllv v1,v1,a0 -> v1,v1,v0` in both arms plus arm A's `lbu/or/sb` on $v0 where the target uses $a0. Arm B's chain is byte-correct. The honest n4 form is the new candidate.c.
- verdict: CONFIRMED

## [s4] Buying reg_n_sets != 1 for a BLOCK-LOCAL `shift` through same-variable split-init accumulation, so that the target's local-alloc birth order (shift, mask, chain) and its seats (shift $v0, mask $v1, chain $a0/$v0) are reachable without making `shift` a global.c allocno.
- mechanism: flow.c counts reg_n_sets before combine, so the s2 H6 env split-init (`env = idx*0x4090; env += (s32)&D_800F7438;`) raised its count from 6 to 10 and survived to flow. The same trick was expected to give the shift two sets and defeat sched.c:2504 birthing_insn_p.
- probe: three arithmetic split spellings, all block-local on the s3 q2 shape, swept with tools/sweep_variants.py on the q2 do-while(0) carrier - `shift = select - 1; shift -= 2;` (p1), `shift = select + 1; shift -= 4;` (p2), `shift = select * 1; shift -= 3;` (p5); plus the all-block-local variant (p3) and a blk-shift/fn-mask/blk-bits variant (p4)
- result: p1 = 11, p2 = 11, p5 = 11 - all three reproduce the s3 q2 objdiff EXACTLY (the addiu still moves in both arms), so the boost still fires; p3 = 15 (identical to the s3 q1 all-block form), p4 = 14. cse2 reassociates two integer-constant offsets into a single addiu before flow counts the sets; the env split-init survived only because its second operand was a symbol address that cse cannot fold into the first set.
- verdict: KILLED
- kill_scope: instance
- measured_on: q2 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present); tmp/grind/func_80016E60/s4/variants2/p1_q2_splitarith_m1m2.c, p2, p5, p3_allblk_splitarith.c, p4_blkshift_fnmask_blkbits_splitarith.c

## [s4] Making `bits` a function-scope variable alongside a function-scope `shift` and `mask`, so that all three bit-arm quantities share the fixed emission order and the global.c seats.
- mechanism: a function-scope `shift` removes the sched1 boost (H14); the guess was that putting the chain on the same footing would let global.c seat all three at once.
- probe: body n1 (fn shift / fn mask / fn bits) and n2 (fn shift / blk mask / fn bits) in tmp/grind/func_80016E60/s4/variants/, swept on the do-while(0) carrier
- result: 17 and 17 (against 11 for n4, which keeps `bits` block-local). A function-scope `bits` is ONE global.c allocno shared by both arms and therefore cannot be $a0 in arm A and $v0 in arm B - the s3 H11 finding is unchanged by the emission-order fix.
- verdict: KILLED
- kill_scope: instance
- measured_on: q2 do-while(0) carrier (FAKE do-while wrap present); tmp/grind/func_80016E60/s4/variants/n1_fn_shift_fn_mask_fn_bits.c / n2_fn_shift_blk_mask_fn_bits.c

## [s4] A permuter campaign seeded from the s3 honest candidate finds a form below the honest floor of 30.
- mechanism: the permuter's random passes (perm_temp_for_expr, perm_reorder_stmts, perm_split_assignment, perm_refer_to_var, perm_duplicate_assignment) cover exactly the space of small C respellings that move sched1 birth order and flow reference counts.
- probe: tools/permuter_campaign.py launch --func func_80016E60 --dir tmp/grind/func_80016E60/s4/perm_a --label honest-q2-candidate-30 -j 8 --stop-on-zero (base permuter score 585, --stack-diffs default), harvested at 26,426 iterations / 805 s
- result: 64 finds, BEST 212 against a base of 585, no sub-floor form and no novel structural class - every find was an attractor around the seed. Campaign harvested and stopped. The productive lever this session came from pass attribution (dump.ps1 + the instrumented cc1), not from sampling; a re-seed of this same chassis is banked as spent.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, s3 honest candidate as base.c, no FAKE constructs; tmp/grind/func_80016E60/s4/perm_a

## [s4] The bit arms' `li`-before-`addiu` emission divergence is produced by sched1's register-pressure heuristic: adjust_priority raises the shift's addiu to max_priority because birthing_insn_p is true for it, and birthing_insn_p is true only because a per-arm block-local `shift` pseudo has reg_n_sets == 1.
- mechanism: tools/gcc-2.7.2/sched.c:2504 birthing_insn_p returns 1 for a SET whose REG dest is live in bb_live_regs and whose reg_n_sets[regno] == 1; sched.c:2586 adjust_priority then sets INSN_PRIORITY(prev) = max_priority. Here max_priority is 0x7F000001, inherited from the block-ending jump (insn 407, priority 2147483477). sched1 scans the basic block BOTTOM-UP, so a boosted insn is picked first and therefore EMITTED LAST. `mask` (mask = 1; mask <<= shift) and `bits` (bits = D_800A3788; bits |= mask) each carry two sets, print birth=0, stay at priority 1, and fall through rank_for_schedule (sched.c:2408) to the INSN_LUID tie-break.
- probe: pwsh tools/grinder/dump.ps1 func_80016E60 for pass attribution, then the instrumented cc1 (tools/gcc-2.7.2/cc1) run over src/ings.c with BB2_SCHED_DEBUG=1 BB2_RANK_DEBUG=1 and separately BB2_PRIO_DEBUG=1; read tmp/grind/func_80016E60/s4/sched.log lines 7788-7812 and tmp/grind/func_80016E60/s4/prio.log.
- result: The trace is explicit: ready list [390(p=1,l=4) 399(p=1,l=8) 393(p=1,l=5)], then `SCHEDDBG ADJPRI insn=390 deaths=0 birth=1 maxpri=2130706433 pri=1`, then `SCHEDDBG PICK clock=5 picked=390 (pri=2130706433 luid=4)`, then 399, then 393 - emission li, lbu, addiu. Every other insn in the arm prints birth=0. Enumerating the eight boost subsets over {shift, mask, chain} against the three possible source orders shows the target emission `addiu, li, lbu` is reachable from exactly two configurations ({none boosted} and {chain only}), and both require the shift unboosted. That is why the s2 H8 and s3 nine-spelling statement-permutation probes all came back inert: statement order only moves LUIDs and the boost overrides LUID entirely.
- verdict: CONFIRMED

## [s4] Declaring `shift` and `mask` at function scope, so each is written in both bit arms, gives their pseudos reg_n_sets == 2, removes the sched1 boost, and reproduces the target's emission order in both arms.
- mechanism: Two sets of one pseudo make birthing_insn_p false (sched.c:2504), no insn in the arm is boosted, and rank_for_schedule's INSN_LUID tie-break emits addiu, li, lbu in arm A and addiu, li, sllv, lbu in arm B - the target's orders. flow.c counts reg_n_sets across the whole function, so scope, not statement order, is the control.
- probe: tools/sweep_variants.py over a five-cell scope grid on the do-while(0) carrier (tmp/grind/func_80016E60/s4/variants/), then tools/objdiff.py on the winner, then the wrap-free honest form.
- result: n4 (fn shift / fn mask / blk bits) = 11 on the carrier and 30 honest - the same scores as the s3 q2 candidate, but the objdiff no longer contains a single insn-position change. It is now `-addiu a0,s1,-3 / +addiu v0,s1,-3` and `-sllv v1,v1,a0 / +sllv v1,v1,v0` in both arms plus arm A's lbu/or/sb on $v0 where the target uses $a0; arm B's chain is byte-correct on $v0. Grid: n1 fn/fn/fn = 17, n2 = 17, n3 = 17, n4 = 11, n5 blk-shift = 17. The honest n4 is the new candidate.c.
- verdict: CONFIRMED

## [s4] Same-variable split-init accumulation on a BLOCK-LOCAL `shift` (shift = select - 1; shift -= 2, and two sibling spellings) raises reg_n_sets above 1 and defeats the sched1 birthing boost while keeping the shift out of global.c.
- mechanism: flow.c counts reg_n_sets before combine runs, which is why the s2 H6 env split-init raised env's count from 6 to 10 and survived; the same trick was expected to give the shift two sets.
- probe: tools/sweep_variants.py on the q2 do-while(0) carrier over tmp/grind/func_80016E60/s4/variants2/: p1 `shift = select - 1; shift -= 2;`, p2 `shift = select + 1; shift -= 4;`, p5 `shift = select * 1; shift -= 3;` (all block-local on the q2 shape), p3 the all-block-local variant, p4 blk-shift/fn-mask/blk-bits.
- result: p1 = 11, p2 = 11, p5 = 11 - and p1's objdiff is byte-for-byte the s3 q2 diff, i.e. the addiu still moves in both arms, so the boost still fired. p3 = 15 (identical to the s3 q1 all-block form), p4 = 14. cse2 reassociates two integer-constant offsets into a single addiu before flow counts the sets. The H6 env split-init survived cse only because its second operand was a symbol address that cse cannot fold into the first set; a pair of integer constants is foldable.
- verdict: KILLED
- kill_scope: instance
- measured_on: q2 do-while(0) carrier (FAKE do-while wrap around {PutDispEnv, PutDrawEnv} present); tmp/grind/func_80016E60/s4/variants2/p1_q2_splitarith_m1m2.c, p2_q2_splitarith_p1m4.c, p5_q2_splitmul.c, p3_allblk_splitarith.c, p4_blkshift_fnmask_blkbits_splitarith.c

## [s4] Putting `bits` at function scope alongside a function-scope `shift` and `mask` lets global.c seat all three bit-arm quantities at once, now that the emission order is fixed.
- mechanism: A function-scope shift removes the sched1 boost; the guess was that putting the chain on the same footing would let global.c allocate the whole arm coherently.
- probe: bodies n1 (fn shift / fn mask / fn bits) and n2 (fn shift / blk mask / fn bits) in tmp/grind/func_80016E60/s4/variants/, swept on the do-while(0) carrier.
- result: 17 and 17, against 11 for n4 which keeps `bits` block-local. A function-scope `bits` is one global.c allocno shared by both arms and therefore cannot be $a0 in arm A and $v0 in arm B - the s3 H11 finding survives the emission-order fix unchanged.
- verdict: KILLED
- kill_scope: instance
- measured_on: q2 do-while(0) carrier (FAKE do-while wrap present); tmp/grind/func_80016E60/s4/variants/n1_fn_shift_fn_mask_fn_bits.c and n2_fn_shift_blk_mask_fn_bits.c

## [s4] A permuter campaign seeded from the s3 honest candidate finds a form below the honest floor of 30.
- mechanism: The permuter's random passes (perm_temp_for_expr, perm_reorder_stmts, perm_split_assignment, perm_refer_to_var, perm_duplicate_assignment) cover exactly the space of small C respellings that move sched1 birth order and flow reference counts, so a hill-climb from the seed should reach any nearby honest basin.
- probe: tools/permuter_campaign.py launch --func func_80016E60 --dir tmp/grind/func_80016E60/s4/perm_a --label honest-q2-candidate-30 -j 8 --stop-on-zero (base permuter score 585, --stack-diffs default), waited in-turn and harvested at 26,426 iterations / 805 s, then harvest --stop.
- result: 64 finds, best 212 against a base of 585 - no sub-floor form and no novel structural class; every find was an attractor around the seed. The productive lever this session came from pass attribution (dump.ps1 plus the instrumented cc1), not from sampling. Campaign harvested and stopped; a re-seed of this same chassis is banked as spent.
- verdict: KILLED
- kill_scope: instance
- measured_on: asm-until-matched chassis, s3 honest candidate as base.c, no FAKE constructs; tmp/grind/func_80016E60/s4/perm_a
