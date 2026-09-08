# Hypothesis ledger — func_8002E6B0

## Floor history
- pre-migration (2026-08-19): 64 (body G, cross_point-first, `return 0` exits)
- s2 (2026-09-08, structural): 40 -> 40 (flat; 14 structural forms measured, 10 byte-identical to v12)
- s1 (2026-09-08, recon): 64 -> **40** (v12 = candidate.c: cross_center-first, block-scoped single-set dz/dx, function-scope cross_center/cross_point, plain `return 0` exits)

## KILLED (s1; all instance kills on the -mel -msoft-float chassis, no FAKE constructs present)
- H-ret-var: result variable `ret = 0` + `goto end` exits (G order 72; HEAD order 60; with block-scoped dz/dx 45). Measured; and shown mechanically unable to put `ret` in $v0 (allocno priority + local-alloc invisibility). Do not re-spell.
- H-andif: `&&` chains (ret var 64, direct return 64). Front-end shape produces branch + `li 1`, never the store-flag tail. jump.c store-flag conversion disabled on MIPS.
- H-all-block-scoped / H-fully-inlined: cross_center/cross_point as per-block locals or inlined = 60. Target's consistent cc->$a0 / cp->$v1 across blocks is a global-pseudo signature.

## CONFIRMED (s1)
- dz/dx must be SINGLE-SET pseudos (block-scoped or otherwise once-written): sched.c:2526 `birthing_insn_p` needs `reg_n_sets == 1` for the LAUNCH_PRIORITY boost that places their defs next to their mults (60 -> 45 -> 40 with the rest).
- cross_center-before-cross_point statement order (HEAD order) reproduces the target's multiply order; G order (cross_point first) does not (72 vs 60 in the ret-var pair).

## Live frontier (ranked)
1. **Reorg all-live label: measure, don't spell.** Run the instrumented cc1 on v12 with `BB2_DBR_DEBUG=1` (dump.ps1 -Instrumented does not pass env; write a tmp .sh that exports the env and runs the exact cc1 line from tools/grinder/dump.ps1:46-50) and read `DBRDBG mtlr target=<uid> block=<b>` for the first bltz's target label; then rerun with `BB2_ALLLIVE_LABEL=<that uid>` and diff the .s tail against the target (expect slot 1 = `move v0,zero`, both bltz -> epilogue, no `j`). If confirmed, the C question is: which ordinary exit shape leaves the shared `v0 = 0` block under a label flow never saw (created by jump2 cross-jump). Candidates, one sandbox measurement each: nested `if (t1 >= 0) { if (t2 >= 0) { return ...; } } return 0;`; exits ordered so the SECOND `return 0` is the surviving one; `goto`-less nesting with the `return 0` last.
2. **Seat cascade under the corrected exits.** After (1), re-extract the RA model (`tools/ra_solver/extract.py func_8002E6B0 code6cac_b`) and run `inverse.py global` with goal {cross_center:4, cross_point:3, arg2[2]:13, arg2[0]:14}; run `local_extract.py`/`local_alloc.py code6cac_b` on block 2 to learn what must exclude $v0 there (is hard $v0 live across block 2 in flow's view when the surviving return-0 block PRECEDES block 2 in the insn stream? that would be the missing exclusion).
3. **arg2 coordinate seats.** Target: arg2[0]->$t6, arg2[2]->$t5, no coordinate in an a-register; ours arg2[2]->$a0. Expected to fall out of (2) (cc in $a0 conflicts with every coordinate pseudo); measure only after (1)/(2).

## [s1] Declaration fix / DATA MODEL: none applicable; func_8002E6B0 references zero globals (pure leaf over four s32* params)
- mechanism: OBJECT MODEL: n/a - no flagged symbols, nothing to measure
- probe: read asm/funcs/func_8002E6B0.s + banked body; canonical verdict C, tier LOW
- result: no globals touched; no declaration hypothesis exists for this function
- verdict: CONFIRMED

## [s1] dz/dx as function-scope multi-set variables prevent sched1's LAUNCH_PRIORITY boost; block-scoped single-set dz/dx place their defs next to the mults as in the target
- mechanism: sched.c:2543 adjust_priority grants LAUNCH_PRIORITY only via sched.c:2505 birthing_insn_p, which requires reg_n_sets[dest]==1 (sched.c:2526); multi-set pseudos stay at priority 2 and are emitted first in the block by the backward list scheduler
- probe: sandbox on v3 (function-scope dz/dx, 60) vs v7 (block-scoped dz/dx, 45) vs v12 (same + return 0 exits, 40); sched_solver pass-1 pick trace tmp/grind/func_8002E6B0/s1/sched1_picks.txt shows insns 54/61 (dz/dx) at pri 2 vs 0x7f000001 around them
- result: 60 -> 45 -> 40; v12 block 1 matches target order line-for-line up to the first bltz (registers aside)
- verdict: CONFIRMED

## [s1] A result variable initialised to 0 on the main path with goto-end exits (ret-var family) reproduces the target's v0=0 delay slot and epilogue-direct exits, measured on the v3/v7 chassis
- mechanism: hypothesised: ret pseudo allocated $v0 so `move v0,zero` sits before the first bltz and fill_simple_delay_slots lifts it
- probe: sandbox v1/v2 (G order) = 72/72, v3/v4 (HEAD order) = 60/60, v7 (block-scoped dz/dx) = 45; .greg shows ret (pseudo 96/98) conflicting with hard $v0/$v1; RA model: ret priority ~4242 vs cross_point 20000 so ret is never allocated first; local-alloc gives block-2 temps $v0 because global pseudos are invisible to it
- result: ret lands in $a2/$a1, a `move v0,ret` survives, an extra $s6 save appears; mechanically cannot reach $v0 on this chassis
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v1-v4 and v7 bodies, no FAKE constructs

## [s1] && chains (`ret = t1>=0 && t2>=0 && t3>=0` and `return t1>=0 && ...`) reproduce the target's exit structure, measured on this chassis
- mechanism: expr.c:5678 TRUTH_ANDIF_EXPR emit_clr_insn + jumpifnot + emit_0_to_1_insn: third term becomes a branch + li 1, never the do_store_flag xor/nor/srl the target ends with; jump.c:1023-1190 store-flag rewrite is disabled on MIPS (BRANCH_COST 1, no conditional move)
- probe: sandbox v8 = 64/94, v9 = 64/94; v9 disassembly shows `move t6,zero` in the slot (pseudo target, not hard $v0) and a li-1 tail
- result: 64 both; tail shape incompatible with target
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, v8/v9 bodies, no FAKE constructs

## [s1] cross_center/cross_point as per-block locals (or fully inlined expressions) improve the floor on the block-scoped dz/dx chassis
- mechanism: hypothesised single-set boost for the cross temps too; but target's consistent cc->$a0 / cp->$v1 in all three blocks is the signature of one global pseudo each (a block-local cc in block 3 would take the free $v0)
- probe: sandbox v5 (all block-scoped) = 60/96, v6 (fully inlined) = 60/96 vs v7 = 45
- result: worse by 15; cross vars must stay function-scope
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, v5/v6 bodies, no FAKE constructs

## [s2] Exit-shape restructuring (nested ifs / shared `goto ret0` label / if-else arms) changes the emitted object on the v12 chassis
- mechanism: hypothesised that the surviving return-0 block's provenance (which of the two `return 0`s survives jump2 cross-jumping, and whether its label is one flow analysed) is steerable from the exit spelling; jump.c cross_jump + jump-threading run before RA
- probe: sandbox on v12 (plain double `return 0`), v20 (nested `if (t>=0) { ... }` with a single trailing `return 0`), v21 (`goto ret0` twice + trailing `ret0: return 0;`), v25 (`if (...) return 0; else { block }` arms); md5 of the resulting tmp/sandbox/func_8002E6B0/code6cac_b.o
- result: all four = score 40 / 93, and all four objects are md5-IDENTICAL (45e95a1329ccc6c9cceb08218344b024). jump.c collapses the four shapes to one RTL before RA
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12/v20/v21/v25 bodies, no FAKE constructs

## [s2] Statement-level structural levers (declaration order, coordinate hoisting, named difference temps, named sums, distinct single-set dz/dx names, xor operand order) move the floor below 40
- mechanism: hypothesised that changing pseudo birth order / count would re-order global.c's allocno priority list and break the cross_center->$v1 / cross_point->$v0 seat cascade; sched.c birthing_insn_p already showed single-set naming is a live lever on this function (s1: 60 -> 45)
- probe: sandbox on 11 forms — v30 xor operand swap, v31 cross decl order, v32 cross_point assigned first, v33 center_z first, v34 arg3 coords hoisted, v35 arg0+arg3 coords hoisted, v40/v41/v42 named delta temps (all/center-only/point-only), v50 cross decls before center decls, v51 named sums, v52 function-scope distinct dz1..dz3/dx1..dx3
- result: 40 for all except v32 = 57 and v42 = 44 (both worse). Only v30 produced different object bytes at the same score; the other eight 40s are byte-identical to v12. CSE + jump normalisation absorb the edits before RA sees them
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v30-v35/v40-v42/v50-v52 bodies, no FAKE constructs

## [s2] reorg.c's all-live fallback at the first bltz's target is the mechanism that puts `v0 = 0` in the delay slot and sends both bltz to the epilogue
- mechanism: reorg.c mark_target_live_regs -> find_basic_block(target) == -1 -> SET_HARD_REG_SET (everything live) -> fill_eager_delay_slots rejects the fallthrough insn and steals the target thread's `move v0,zero`, redirecting the branch past it
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) on a single-function TU (s2/solo.c) with BB2_DBR_DEBUG=1, then with BB2_ALLLIVE_LABEL=170; diff of the two .s files
- result: baseline prints exactly one `DBRDBG mtlr target=170 block=4` (flow KNOWS the label, precise liveness -> fallthrough steal). Forcing all-live on label 170 changes the first bltz's slot to `move $2,$0` and redirects the branch — exactly the target's lines 48-49. The second bltz has a separate target (`mtlr target=183 block=0`) and is unaffected, so the forced body keeps the `j .Lend` + orphan block and is 99 insns
- verdict: CONFIRMED

## Live frontier after s2 (ranked)
1. **Make the shared return-0 label flow-UNKNOWN.** Both bltz targets must return -1 from find_basic_block for the target's exit shape to fall out. Ours is block=4 / block=0 (known) because jump2's cross-jump reuses labels that existed at the last find_basic_blocks. Next probe: read tools/gcc-2.7.2/jump.c `do_cross_jump` / `find_cross_jump` to learn under which condition it EMITS A FRESH label instead of reusing one (that is the only C-visible knob), and confirm with the instrumented cc1 loop (s2/dbr2.sh — cc1-only, ~2s per body, no sandbox round trip needed) by grepping `DBRDBG mtlr target=<uid> block=` for a body whose block is -1. Only then measure with sandbox. Note the four exit spellings already tried are one object; the fresh-label condition, if it exists, is NOT among them.
2. **Seat cascade (unchanged from s1, still the larger half of the 40).** Target needs a SIXTH callee-save (`s5` for block 1's 4th product) and cross_center->$a0 / cross_point->$v1 / arg2[2]->$t5, where we get $v1 / $v0 / $a0. All 14 structural spellings measured in s2 leave the cascade untouched, so the lever is not statement-level: use tools/ra_solver (extract.py + inverse.py global --goal '{"<cc>":4,"<cp>":3,"<arg2z>":13}') to type it REACHABLE/FORECLOSED before any further spelling.
3. **arg2 coordinate seats** — expected to fall out of (2); no independent probe.

## [s2] Exit-shape restructuring (nested ifs, a shared `goto ret0` label, if/else arms) changes the emitted object on the v12 chassis
- mechanism: hypothesised that which of the two `return 0` blocks survives jump2 cross-jumping, and whether its label is one flow analysed, is steerable from the C exit spelling; jump.c cross-jump/threading runs before RA
- probe: sandbox on v12 (double `return 0`), v20 (nested ifs + single trailing `return 0`), v21 (`goto ret0` twice + trailing labelled `return 0`), v25 (if/else arms), then md5 of each resulting tmp/sandbox/func_8002E6B0/code6cac_b.o
- result: all four = score 40 / build_insns 93 and all four objects are md5-IDENTICAL (45e95a1329ccc6c9cceb08218344b024); jump.c collapses the four shapes to one RTL before RA, so frontier-1's suggested nested-if / reordered-return-0 probes are spent
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12/v20/v21/v25 bodies, no FAKE constructs

## [s2] Statement-level structural levers (declaration order, coordinate hoisting, named delta temps, named sums, distinct single-set dz/dx names, xor operand order) move the floor below 40
- mechanism: hypothesised that changing pseudo birth order/count would re-order global.c's allocno priority list and break the cross_center->$v1 / cross_point->$v0 seat cascade; sched.c birthing_insn_p single-set naming was a live lever in s1 (60 -> 45)
- probe: sandbox on 11 forms: v30 xor operand swap, v31 cross decl order, v32 cross_point assigned first, v33 center_z first, v34 arg3 coords hoisted, v35 arg0+arg3 coords hoisted, v40/v41/v42 named delta temps (all / center-only / point-only), v50 cross decls before center decls, v51 named sums, v52 function-scope distinct dz1..dz3/dx1..dx3
- result: 40 for all except v32 = 57 and v42 = 44 (both worse); only v30 produced different object bytes at the same score, the other eight 40s are byte-identical to v12 — CSE and jump normalisation absorb the edits before RA sees them
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v30-v35 / v40-v42 / v50-v52 bodies, no FAKE constructs

## [s2] reorg.c's all-live fallback at the first bltz's target is the mechanism that puts `v0 = 0` in the delay slot and sends both bltz to the epilogue
- mechanism: reorg.c mark_target_live_regs -> find_basic_block(target) == -1 -> SET_HARD_REG_SET (everything live) -> fill_eager_delay_slots rejects the fallthrough insn (it sets a0) and steals the target thread's `move v0,zero`, redirecting the branch past it
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) on a single-function TU (tmp/grind/func_8002E6B0/s2/solo.c) with BB2_DBR_DEBUG=1 (dbr2.sh), then with BB2_ALLLIVE_LABEL=170 (dbr3.sh); diff the two .s files and disassemble both
- result: baseline prints exactly one `DBRDBG mtlr target=170 block=4` — flow KNOWS the label, so precise liveness is used and the fallthrough insn is stolen. Forcing all-live on label 170 makes the first bltz's delay slot `move $2,$0` with the branch redirected past it, matching the target's lines 48-49. The second bltz has a separate target (`mtlr target=183 block=0`) and is unaffected, so the forced body keeps the `j .Lend` + orphan block (99 insns).
- verdict: CONFIRMED

## [s3] reorg.c's all-live fallback at the shared return-0 label can produce the TARGET'S FULL exit shape (both bltz to the epilogue, no `j`, no orphan block) if it is forced on the branch targets
- mechanism: reorg.c mark_target_live_regs -> find_basic_block == -1 -> SET_HARD_REG_SET; the stolen `move v0,zero` plus relax_delay_slots' redundant-target redirect (reorg.c ~3950) was expected to retarget the second bltz past the move and let the block be deleted
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) on the v12 body, BB2_DBR_DEBUG=1, three runs: unforced, BB2_ALLLIVE_LABEL=170, and BB2_ALLLIVE_LABEL=170,183 (tmp/grind/func_8002E6B0/s3/dbr.sh; outputs dbr/base.s, dbr/f170.s, dbr/f170_183.s)
- result: unforced = 88 insns, both bltz UNFILLED and both targeting .L3; forcing 170 = 89 insns, bltz#1 gets `move $2,$0` + retarget to .L4 but bltz#2 stays unfilled at .L3 and `j .L4` + `.L3: move $2,$0` SURVIVE; forcing BOTH 170 and 183 is byte-for-byte the same 89-insn result. The redundant-target redirect never fires because reorg.c:3950 gates it on the branch already being a filled SEQUENCE, and cc1 leaves both bltz slots unfilled in this function (maspsx/as fills them later). So the all-live route cannot reach the target's exit shape even when maximally forced, and s2's frontier-1 (find the C that makes the shared label flow-unknown) is not a route to it.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12 body, no FAKE constructs, instrumented cc1 with BB2_ALLLIVE_LABEL forced on 0/1/2 labels

## [s3] Some ordinary-C ret-var spelling (assignment position, declaration order, exit form, xor operand order) moves the ret pseudo out of $a1 into $v0 on the exit-correct chassis
- mechanism: global.c allocno_compare priority = floor_log2(n_refs)*n_refs/live_length*10000 (global.c:642-649) and prune_preferences' regs_someone_prefers (global.c:912-930); the hope was that changing ret's refs / live length / copy preference would let it pre-empt $v0
- probe: sandbox on v70 (= s1's ret-var body, re-measure), v71 (`ret = 0;` moved down to immediately before the first `if`), v74 (`ret` declared before cross_center/cross_point), v80 (xor operands swapped everywhere), v72 (`return ret;` at both early exits instead of `goto end`)
- result: v70 = 45, v71 = 45, v74 = 45, v80 = 45, v72 = 40 (v72 normalises back to the v12 object — ret is dead at the exits so jump.c rewrites it to `return 0` and the orphan block comes back). No spelling moved ret off $a1. The .greg dump shows the reason is a hard-reg CONFLICT, not priority: ret conflicts with hard reg 2 because local_alloc (which runs first and cannot see function-scope pseudos) has already put the sum accumulator, its `sra`, and every block-scoped `dz`/`dx` in $v0, and MIPS defines no REG_ALLOC_ORDER so ascending order hands out $v0 first every time.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v70/v71/v72/v74/v80 bodies, no FAKE constructs

## Live frontier after s3 (ranked)
1. **Get a HARD `(set (reg:SI 2 v0) (const_int 0))` in front of block 1's branch at LOCAL-ALLOC time.** The target's block 2 never touches $v0 while ours does, and local_alloc only respects HARD regs — so the original's `v0 = 0` was the function's return register written directly by expand (DECL_RESULT of a scalar return is the hard reg), not a pseudo. Next probe (cheap, cc1-only): for each candidate body write it through tmp/grind/func_8002E6B0/s3/da.sh and grep the `.lreg` dump for the position of `(set (reg:SI 2 v0) (const_int 0))` relative to the first `bltz` — only sandbox-measure a body where it lands BEFORE the branch. Candidates not yet screened this way: `if (c1 >= 0) { if (c2 >= 0) { return c3 >= 0; } } return 0;` with the `return 0` LAST; `return c1 >= 0 ? (c2 >= 0 ? c3 >= 0 : 0) : 0;` (COND_EXPR, never tried — s1 only tried TRUTH_ANDIF); `if (c1 < 0 || c2 < 0) return 0;` (TRUTH_ORIF, never tried).
2. **Type the local-alloc question.** tools/ra_solver local_extract.py/local_alloc.py on code6cac_b block 2: is there ANY exclusion of hard $v0 from block 2's local qtys reachable without a hard $v0 set? If FORECLOSED, frontier 1 is the only route and the answer is entirely in how the return value is expanded.
3. **The 6th callee-save is a consequence, not a target.** The target's `mflo $s5` (block 1's 4th product in a callee-save) follows from $v0 being unavailable in block 1's tail; do not chase it with spellings.

## [s3] reorg.c's all-live fallback at the shared return-0 label produces the target's full exit shape (both bltz branching to the epilogue, no j, no orphan move-block) when forced on the branch target labels
- mechanism: reorg.c mark_target_live_regs -> find_basic_block == -1 -> SET_HARD_REG_SET, after which relax_delay_slots' redundant-target redirect (reorg.c ~3950) was expected to retarget the second bltz past the stolen `move v0,zero` and let the block be deleted
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) on the v12 body with BB2_DBR_DEBUG=1 in three runs - unforced, BB2_ALLLIVE_LABEL=170, BB2_ALLLIVE_LABEL=170,183 - via tmp/grind/func_8002E6B0/s3/dbr.sh; outputs dbr/base.s, dbr/f170.s, dbr/f170_183.s
- result: unforced = 88 insns with both bltz UNFILLED and both targeting .L3. Forcing 170 = 89 insns: bltz#1 gets `move $2,$0` in its slot and is retargeted to .L4, but bltz#2 stays unfilled at .L3 and both `j .L4` and `.L3: move $2,$0` survive. Forcing 170 AND 183 gives byte-for-byte the same 89-insn output. The redundant-target redirect never fires because reorg.c gates that whole section on the branch already being a filled SEQUENCE ('Now look only at cases where we have filled a delay slot'), and cc1 leaves both bltz slots unfilled in this function - maspsx/as fills them afterwards. s2's frontier-1 (find the C that makes the shared return-0 label flow-unknown) is therefore not a route to the target's exit shape.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12 body, no FAKE constructs, instrumented cc1 with BB2_ALLLIVE_LABEL forced on 0, 1 and 2 labels

## [s3] Some ordinary-C ret-var spelling (assignment position, declaration order, exit form, xor operand order) moves the ret pseudo out of $a1 into $v0 on the exit-correct chassis
- mechanism: global.c:642-649 allocno_compare priority = floor_log2(n_refs)*n_refs/live_length*10000, plus prune_preferences' regs_someone_prefers (global.c:912-930): the hope was that changing ret's reference count, live length or copy preference would let it pre-empt $v0 from the cross pseudos
- probe: sandbox --disable all on v70 (s1's ret-var body re-measured), v71 (`ret = 0;` moved down to immediately before the first if), v74 (ret declared before cross_center/cross_point), v80 (xor operands swapped in all three tests), v72 (`return ret;` at both early exits instead of `goto end`); plus a -dg/-dl dump of the real TU (tmp/grind/func_8002E6B0/s3/real.i.greg)
- result: v70 = 45, v71 = 45, v74 = 45, v80 = 45, v72 = 40. No spelling moved ret off $a1. v72 normalises back to the v12 object (ret is dead at the exits, so jump.c rewrites it to `return 0` and the orphan block returns). The .greg dump shows the blocker is a hard-reg CONFLICT and not priority: the ret pseudo's conflict set contains hard reg 2, because local_alloc - which runs before global_alloc and cannot see function-scope pseudos - has already placed the sum accumulator, its sra, and every block-scoped dz/dx in $v0, and MIPS defines no REG_ALLOC_ORDER so plain ascending order hands out $v0 first.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v70/v71/v72/v74/v80 bodies, no FAKE constructs
