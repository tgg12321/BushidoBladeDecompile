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
