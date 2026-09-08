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

## [s4] Borrowing an existing function-scope local (`cross_point`) to stage block 2's `dx` lowers the floor below 40 on the v12 chassis
- mechanism: the borrow makes the `dx` value live in the cross_point pseudo across the cross_center product, which raises block-1/block-2 register pressure enough that global.c find_reg exhausts the caller-save set and falls back to a SIXTH callee-save — the target's `sw s5` / `mflo s5`. A fresh local cannot do this because CSE/copy-propagation folds a plain `s32 dxa = dx;` away before RA ever sees a second live range.
- probe: decomp-permuter campaign `tmp/grind/func_8002E6B0/s4/perm_b` (v12 chassis, 26,669 iterations) produced output-345-1; sandbox --disable all on that body, then a hand ablation sweep (fresh named intermediate in the same position; borrow in block 1 only; borrow in all three blocks; borrow cross_center for dz instead; xor-swap and mixed-exit respellings)
- result: **40 -> 34 / 95 insns.** Fresh intermediate = 40, block-1-only = 40, all-three-blocks = 61, cross_center-for-dz = 53; xor-swap and mixed-exit respellings TIE at 34. The 34 body's prologue and block-1 tail now carry the target's sixth callee-save (`sw s5,20(sp)`, `mflo s5`) and lines 1-17/20/22-23/26-34/37/39/45 of the -dz side-by-side are identical to the target.
- verdict: CONFIRMED
- NOTE: the borrow is a FAKE construct in the variable-reuse family (.claude/rules/defeat-licm-hoist-var-reuse.md, gated by .claude/rules/staged-value-reused-variable.md — an EXISTING local borrowed for a second value, not an invented one). It carries no annotation yet because this session is not submitting; any candidate-ready must add the `/* FAKE: ... */` line with mechanism + lever-exhaustion and quote that rule's scope sentence. The best FAKE-FREE form remains v12 at 40 (`rejected/s4_v12_plain_no_varreuse_40.c`).

## [s4] The target's sixth callee-save ($s5) is an independent target that must be chased with its own spellings
- mechanism: s3 frontier-3 asserted the opposite (that it is a consequence of block-1 tail pressure); this hypothesis tested the assertion by NOT aiming at it
- probe: the 34 body was produced by a pressure-raising borrow in block 2 with no statement anywhere aimed at block 1's fourth product; -dz side-by-side of the result vs asm/funcs/func_8002E6B0.s
- result: `sw s5,20(sp)` and `mflo s5` appeared with no spelling aimed at them, exactly as s3 predicted. s3's frontier-3 reading is confirmed; the callee-save is a consequence of pressure.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12 and q345 bodies, no FAKE constructs in v12, one variable-reuse borrow in q345

## [s4] The permuter can improve on the 34-scoring borrow body by further randomisation of the same chassis
- mechanism: decomp-permuter's randomizer over the same base source; a basin either yields early or not at all (permuter-directives §Campaign discipline)
- probe: campaign `tmp/grind/func_8002E6B0/s4/perm_c`, seeded on the 34 body (permuter base score 345), 24,869 iterations over ~23 min with fresh seeds
- result: ZERO finds below base. The single same-score find is a respelling of the identical borrow (`dx = center_z - arg0[2];` with `cross_point * dx`), not a new construct. The basin is saturated for this chassis, so re-seeding it is not a valid probe for a later session.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, q345 body with its variable-reuse borrow present, permuter with --stack-diffs

## [s4] The exit structure and the sixth callee-save can be held simultaneously by adding the ret-var/goto-end exit form to the borrow chassis
- mechanism: s3 established the ret-var + `goto end` form reproduces the target's exit shape exactly (zero-move in the first bltz's delay slot, both bltz to the epilogue, no `j`); s4 established the block-2 borrow buys the sixth callee-save. The hypothesis was that the two are independent and compose.
- probe: sandbox --disable all on the ret-var chassis WITH the block-2 borrow (`rejected/s4_retvar_plus_borrow_exit_exact_loses_s5_45.c`) and on the same body with the borrow ALSO in block 1 (`rejected/s4_retvar_borrow_blocks1and2_45.c`); -dz disassembly of the first (`tmp/grind/func_8002E6B0/s4/w6.ins.txt`)
- result: both = 45. The composed body keeps the target's exit shape but DROPS back to five callee-saves (no `sw s5`) and puts ret in `$a1` with a trailing `move v0,a1`; adding the block-1 borrow does not recover s5. The two halves are anti-correlated on the bodies measured: the ret pseudo consumes the pressure the borrow was creating.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, w6/x2 bodies, one (w6) or two (x2) variable-reuse borrows present

## [s4] Untried ordinary-C exit and coordinate spellings move the floor below 40 on the v12/ret-var chassis
- mechanism: assorted — do_store_flag comparison-code choice (GE vs GT vs logical negation), extra named coordinate locals raising pressure, a ret-var whose default is 1 rather than 0, and swapping the cross_point/cross_center assignment order in ONE block rather than all three (s2 had only measured all-three, which was 57)
- probe: sandbox --disable all on `return !((cc ^ cp) < 0)`, `return (cc ^ cp) > -1`, all eight vertex coordinates hoisted into named locals, ret-var defaulted to 1 with `ret = 0;` on both failing paths, and block-2-only assignment-order swap
- result: 40, 40, 44, 41, 46 respectively — none below 40, and the two 40s are the v12 object. Ordinary statement-level spelling remains exhausted on this chassis; the 34 came from a variable-reuse borrow, not from ordinary C.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v92/v93/v96/v91/q445 bodies, no FAKE constructs

## Live frontier after s4 (ranked)
1. **Compose the exit shape with the sixth callee-save.** The 34 body has the target's s5 save + a line-for-line block-1 prefix but the wrong tail; the 45 body has the exact tail but only five callee-saves. Both halves are now individually MEASURED, not hypothesised. Next probe: on the 34 (borrow) chassis, find an exit form that materialises the returned zero WITHOUT introducing a long-lived ret pseudo that eats the pressure — e.g. a second borrow that carries the zero (borrow `cross_center` or `center_x` at the exits rather than declaring `ret`), or the ret-var declared inside a `do { } while (0);` wrap so its live range does not span block 1. Screen cheaply with `tmp/grind/func_8002E6B0/s3/da.sh` + the .lreg dump before spending sandbox measurements.
2. **Close the remaining block-1 seats on the 34 chassis.** The residual there is `lw t5,8(a2)` vs our `lw a0,8(a2)` and `mfhi t7`/`mfhi a1` vs our `mfhi t5`/`mfhi a2`, which then permutes blocks 2 and 3. Now that the multiset finally matches the target's (six callee-saves, 95 vs 96 insns in the permuter normalisation), `tools/ra_solver` inverse/classify on the 34 body is worth running for the first time — s1 reported the goal was not a topological order, but that was measured before the s5 save existed.
3. **Do NOT re-seed a permuter campaign on the 34 chassis** — s4's perm_c saturated it (24,869 iterations, zero finds below base). A future permuter session must seed a structurally different chassis (the ret-var + borrow 45 body is the obvious untried one) or a different lever hint.

## [s4] Borrowing an existing function-scope local (cross_point) to stage block 2's dx lowers the floor below 40 on the v12 chassis
- mechanism: the borrow keeps dx's value live in the cross_point pseudo across the cross_center product, raising block-1/block-2 pressure until global.c find_reg exhausts the caller-save set and falls back to a sixth callee-save ($s5) - the target's sw s5 / mflo s5. A fresh named local cannot do this: CSE/copy-propagation folds a plain `s32 dxa = dx;` away before RA ever sees a second live range (measured: fresh intermediate = 40).
- probe: decomp-permuter campaign tmp/grind/func_8002E6B0/s4/perm_b (v12 chassis, 26,669 iterations, --stack-diffs) produced output-345-1; sandbox --disable all on that body, then a hand ablation sweep of four neighbouring spellings and two respellings
- result: 40 -> 34 / 95 insns. Ablations: fresh named intermediate in the same position = 40, borrow in block 1 only = 40, borrow in all three blocks = 61, borrow cross_center for dz instead = 53. Ties at 34: xor operands swapped throughout, and mixed exit forms (first exit `goto end` + a trailing `end: return 0;`). The -dz side-by-side now matches the target on lines 1-17, 20, 22-23, 26-34, 37, 39 and 45, INCLUDING the prologue's sw s5,20(sp) and block 1's mflo s5.
- verdict: CONFIRMED

## [s4] The target's sixth callee-save ($s5) is an independent target that must be chased with its own statement spellings
- mechanism: s3's frontier-3 asserted the opposite - that it falls out of block-1 tail pressure once $v0 is unavailable - and this hypothesis tested that assertion by aiming nothing at it
- probe: the 34 body was produced by a pressure-raising borrow in block 2 with no statement anywhere aimed at block 1's fourth product; -dz disassembly compared against asm/funcs/func_8002E6B0.s (tmp/grind/func_8002E6B0/s4/q345.ins.txt)
- result: sw s5,20(sp) and mflo s5 appeared with no spelling aimed at them. s3's reading is confirmed and the 14+4 previously-measured neutral statement spellings were correctly not the route.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v12 and q345 bodies; no FAKE constructs in v12, one variable-reuse borrow present in q345

## [s4] The permuter improves on the 34-scoring borrow body by further randomisation of that same chassis
- mechanism: decomp-permuter randomizer over the same base source; per permuter-directives a basin yields early or not at all
- probe: campaign tmp/grind/func_8002E6B0/s4/perm_c seeded on the 34 body (permuter base score 345), 24,869 iterations over ~23 min with fresh seeds, then harvest --stop
- result: zero finds below base. Its single same-score find is only a respelling of the identical borrow (`dx = center_z - arg0[2];` with `cross_point * dx`). The basin is saturated for this chassis; re-seeding it later is not a valid probe.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, q345 body with its variable-reuse borrow present, decomp-permuter with --stack-diffs

## [s4] The target's exit structure and the sixth callee-save are held simultaneously by adding the ret-var/goto-end exit form to the borrow chassis
- mechanism: s3 established the ret-var + goto-end form reproduces the target's exit shape exactly (zero-move in the first bltz's delay slot, both bltz to the epilogue, no j); s4 established the block-2 borrow buys the sixth callee-save; the hypothesis was that the two compose
- probe: sandbox --disable all on the ret-var chassis WITH the block-2 borrow, and on the same body with the borrow also added to block 1; -dz disassembly of the first (tmp/grind/func_8002E6B0/s4/w6.ins.txt)
- result: both = 45. The composed body keeps the target's exit shape but drops back to FIVE callee-saves (no sw s5) and puts ret in $a1 with a trailing move v0,a1; adding a second borrow in block 1 does not recover s5. On the bodies measured the two halves are anti-correlated - the ret pseudo consumes the pressure the borrow creates.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, w6/x2 bodies, one (w6) or two (x2) variable-reuse borrows present

## [s4] Untried ordinary-C exit and coordinate spellings move the floor below 40 on the v12 / ret-var chassis
- mechanism: assorted: do_store_flag comparison-code choice (GE vs GT vs logical negation), extra named coordinate locals raising pressure, a ret-var defaulting to 1 rather than 0, and swapping the cross_point/cross_center assignment order in ONE block (s2 had only measured all three blocks, which was 57)
- probe: sandbox --disable all on `return !((cc ^ cp) < 0)`, `return (cc ^ cp) > -1`, all eight vertex coordinates hoisted into named locals, a ret-var defaulted to 1 with `ret = 0;` on both failing paths, and the block-2-only assignment-order swap
- result: 40, 40, 44, 41 and 46 respectively; the two 40s are the v12 object. Ordinary statement-level spelling stays exhausted on this chassis - the 34 came from a variable-reuse borrow, not from ordinary C.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, v92/v93/v96/v91/q445 bodies, no FAKE constructs

## [s4] The s3 kill of the ret-var seat (no ordinary-C spelling moves ret off $a1) still holds on the current chassis and is not an artefact of a FAKE carrier occupying $v0
- mechanism: mandated kill re-audit: an instance kill is only as good as the chassis and FAKE state it was measured under
- probe: re-measured both banked bodies on HEAD this session (v12 = 40/93, the s3 exit-exact ret-var body = 45/93) and inspected them for FAKE constructs before running tools/fake_ablate.py
- result: chassis unmoved, both scores reproduce exactly. fake_ablate is NOT APPLICABLE here: neither banked body contains a single FAKE construct (plain locals, plain arithmetic, plain returns), so no carrier occupied the contested pseudo and there is nothing to ablate. The s3 kill stands as measured - and s4's 34 body confirms its diagnosis from the other side, since the thing that finally moved the allocation was added register PRESSURE, not any ret-var spelling.
- verdict: CONFIRMED

## [s5] A permuter campaign on the ret-var + borrow (45) chassis - structurally distinct from the permuter-saturated 34 basin - still has room
- mechanism: s4's perm_c saturated the 34 basin (24,869 iterations, zero finds below base), but the ret-var chassis carrying the target's exact exit shape had never been permuted WITH the borrow present; a basin yields early or not at all
- probe: campaign `tmp/grind/func_8002E6B0/s5/perm_d`, seeded on `rejected/s4_retvar_plus_borrow_exit_exact_loses_s5_45.c` (base 630), harvested and stopped in-session
- result: it yielded within minutes. `output-265-1` stages block 2's `dz` through the return carrier `ret` - semantically BROKEN as found (no restore, so the second early exit returns `dz`), measuring 25. Repaired with a `ret = 0;` restore at the end of block 2 it measures **26 / 96 insns**, dropping the floor from 34.
- verdict: CONFIRMED

## [s5] The target's exit structure and its sixth callee-save can be held simultaneously if the pressure carrier is the ret pseudo ITSELF rather than another local
- mechanism: s4 measured the two halves as anti-correlated because the ret pseudo absorbed exactly the block-1/2 pressure that a cross_point borrow created. Staging a real block-2 value THROUGH the ret pseudo puts the pressure back on the pseudo that was consuming it, so global.c find_reg still exhausts the caller-save set and falls back to a sixth callee-save.
- probe: sandbox --disable all on the repaired 26 body, then a -dz side-by-side against the assembled target (`tmp/grind/func_8002E6B0/s5/cmp2.sh e7_ret_stage_first_product_min`)
- result: the 26 body carries BOTH halves - `move v0,zero` in the first bltz's delay slot, both bltz to the epilogue, no `j`, no orphan zero block, AND `sw s5,20(sp)` + `mflo s5`. s4's "anti-correlated" reading was an instance property of the carriers it tried, not of the chassis.
- verdict: CONFIRMED

## [s5] Reusing an EXISTING local as the return carrier (instead of declaring a fresh `ret`) reaches the target's exit shape on the 34 borrow chassis
- mechanism: the s4 frontier proposed it explicitly - a carrier whose live range does not span block 1 would supply the returned zero without absorbing block-1 pressure
- probe: sandbox --disable all on the 34 body with `cross_center`, `cross_point`, `center_x` and `center_z` each used as the carrier (`X = 0; goto end;` at both exits, `X = (cc ^ cp) >= 0; end: return X;`), plus a mixed-exit variant
- result: 34 (identical object to the plain candidate - jump.c folds the carrier assignment back into a plain `return 0`), 37, 50, 50 and 35. None reaches the target's exit shape; the exit shape needs the top-initialised ret pseudo, which is what the s5 winner uses.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, a1/a2/a3/a4/a5 bodies on the s4 34 chassis, one variable-reuse borrow (cross_point = dx) present in each

## [s5] The ret-var chassis's five-callee-save allocation can be pushed to six by borrowing some OTHER local (block 3, blocks 2+3, or a second block-2 borrow staging dz through cross_center)
- mechanism: s4 had only tried the block-2 and blocks-1+2 placements of the cross_point borrow on the ret-var chassis; more borrows should mean more pressure
- probe: sandbox --disable all on the 45 body with the borrow added to block 3, with the borrow ONLY in block 3, and with a second block-2 borrow staging `dz` through `cross_center`
- result: 45 / 93 insns in all three cases - byte-for-byte the same allocation, still five callee-saves. On this chassis a borrow of a local OTHER than the ret pseudo is neutral; only staging through the ret pseudo itself moves the allocation.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, c1/c2/c3 bodies, one or two variable-reuse borrows present in each

## [s5] The `ret = 0;` restore that repairs the permuter's find can be spelled somewhere cheaper than an extra insn at the end of block 2
- mechanism: the restore costs a `move v1,zero` in the second bltz's delay slot, one of the two insns separating the 26 body from the target. Alternative spellings that keep the second exit returning 0 - put the zero inside the branch, spell the second exit as an inline `return 0;`, or send it to a separate `zero:` label - should be free if jump.c cross-jumps them.
- probe: sandbox --disable all on the restore moved inside the second exit's branch, on the second exit spelled `return 0;` inline with no restore, on a separate `zero: return 0;` label with no restore, and on a live READ of `ret` in block 2 (`... + ret`) instead of the staging
- result: 45, 45, 45 and 58. Every spelling that removes the restore statement also removes the pressure and the sixth callee-save with it; the restore is load-bearing, not incidental. The only 26-scoring alternative is the same restore hoisted one statement earlier (before the second `if`), which compiles to the identical object.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, g1/g2/h1/h2 bodies, one variable-reuse borrow (the ret staging) present in each

## [s5] Declaring `ret` uninitialised and assigning the zero at the exits (so its live range does not span block 1) keeps the s5 save while shortening the pseudo
- mechanism: the s4 frontier's leading idea - the target's `$v0` is set in the first bltz's delay slot, i.e. its live range begins AFTER block 1, so a carrier that does not span block 1 should match it more closely
- probe: sandbox --disable all on the 26 body with `s32 ret;` uninitialised and `ret = 0;` at the first exit, both with the dz staging on both block-2 products and on the first product only; plus the same shape without any staging
- result: 30 / 95 insns for both staged variants and 41 for the unstaged one. The 30 body has NO `sw s5` and the WRONG first exit (`bgez` + `j` + `move v1,zero`) - dropping the top initialisation costs both halves at once. The top-initialised `s32 ret = 0;` is required.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, f1/f2/b4 bodies, one variable-reuse borrow present in f1/f2 and none in b4

## [s5] The permuter improves on the repaired 26 body by further randomisation of that same chassis
- mechanism: campaign `perm_e` seeded on the repaired 26 body (base 365), fresh seeds, harvest+stop in-session
- probe: 11,782 iterations / 444 s, 9 novel finds
- result: best 265, but every sub-base find is the same semantically-broken family the permuter already produced on the other chassis - a dead `ret = 0;` moved ABOVE the staging (the second exit then returns dz), or `dx` clobbered by a staged `center_z - arg0[2]` before its later use. Repairing the dx shape by hand (recompute, or restore dx) measures 26 again, i.e. a tie, not an improvement. No legal improvement came out of this basin.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, the 26 body (e7/d2) with its single ret-staging borrow present, decomp-permuter with --stack-diffs

## Live frontier after s5 (ranked)
1. **Get the ret pseudo into `$v0`.** The 26 body's largest remaining cost is that our carrier lands in `$v1`, forcing a trailing `move v0,v1` and renaming {v0,v1,a0,a1,a2} against the target throughout. The register MULTISET now matches the target and the insn stream is 98 vs 96, so `tools/ra_solver` (extract.py + inverse.py global --goal, then `inverse_compose.py classify`) can finally type this seat REACHABLE or FORECLOSED instead of it being guessed. This supersedes the s4 frontier-2 item - run it on the 26 body, not the 34 one.
2. **Make the `ret = 0;` restore free.** It is the second of the two extra insns, and it exists only because our block 2 clobbers the carrier while the target's `$v0` is never touched across block 2. Every spelling that DROPS the restore also drops the sixth callee-save (measured: 45 four ways). The open question is whether a different block-2 value staged through `ret` - one whose last use lets the restore merge into an insn the target already has, or one staged only across the second product - keeps the pressure while letting the zero survive. Screen candidates cheaply with `tmp/grind/func_8002E6B0/s3/da.sh` + the .lreg/.greg dump for a sixth callee-save before spending sandbox measurements.
3. **Permute a structurally different chassis again - the tie family, not the 26 basin.** perm_e saturated the 26 basin with nothing legal; do NOT re-seed it. The untried seeds are the 26 TIES (d2 with two borrows, e1 with ret declared first, i2 with `dx` additionally staged) - each is a different statement geometry that lands on the same object, so their permuter neighbourhoods differ. And note the standing lesson: on this function the permuter's best find has twice been semantically invalid, so read every find's semantics and try the REPAIR before discarding it - the s5 floor drop came entirely from repairing a broken find.

## [s5] A permuter campaign on the ret-var + borrow (45) chassis - structurally distinct from the permuter-saturated 34 basin - still has room
- mechanism: s4's perm_c saturated the 34 basin (24,869 iterations, zero finds below base), but the ret-var chassis that carries the target's exact exit shape had never been permuted with the borrow present; per permuter-directives a basin yields early or not at all
- probe: campaign tmp/grind/func_8002E6B0/s5/perm_d seeded on rejected/s4_retvar_plus_borrow_exit_exact_loses_s5_45.c (base 630), harvested and stopped in-session
- result: It yielded within minutes. output-265-1 stages block 2's dz through the return carrier ret, but is SEMANTICALLY BROKEN as found (no restore, so the second early exit would return dz instead of 0); that raw body measures 25/95. Adding a `ret = 0;` restore at the end of block 2 makes it correct and measures 26/96 - a floor drop from 34.
- verdict: CONFIRMED

## [s5] The target's exit structure and its sixth callee-save are held simultaneously when the pressure carrier is the ret pseudo itself rather than another local
- mechanism: s4 measured the two halves as anti-correlated because the ret pseudo absorbed exactly the block-1/2 pressure a cross_point borrow created; staging a real block-2 value THROUGH the ret pseudo puts the pressure back on the pseudo that was consuming it, so global.c find_reg still exhausts the caller-save set and falls back to a sixth callee-save
- probe: sandbox --disable all on the repaired 26 body, then a -dz side-by-side against the assembled target via tmp/grind/func_8002E6B0/s5/cmp2.sh e7_ret_stage_first_product_min
- result: The 26 body carries both halves: `move v0,zero` in the first bltz's delay slot, BOTH bltz branching straight to the epilogue, no `j` and no orphan zero block, AND `sw s5,20(sp)` + `mflo s5`. s4's anti-correlation was a property of the carriers it tried, not of the chassis.
- verdict: CONFIRMED

## [s5] Reusing an existing local (cross_center, cross_point, center_x, center_z) as the return carrier instead of declaring a fresh ret reaches the target's exit shape on the 34 borrow chassis
- mechanism: the s4 frontier proposed it explicitly - a carrier whose live range does not span block 1 would supply the returned zero without absorbing block-1 pressure
- probe: sandbox --disable all on the 34 body with each of the four locals as the carrier (X = 0; goto end; at both exits, X = (cc ^ cp) >= 0; end: return X;), plus a mixed-exit variant
- result: 34, 37, 50, 50 and 35. The cross_center version compiles to the IDENTICAL object as the plain 34 candidate - jump.c folds the carrier assignment back into a plain return 0 - and none of the four reaches the target's exit shape. The exit shape needs the top-initialised ret pseudo that the s5 winner uses.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, a1/a2/a3/a4/a5 bodies on the s4 34 chassis, one variable-reuse borrow (cross_point = dx) present in each

## [s5] The ret-var chassis's five-callee-save allocation is pushed to six by borrowing some other local (block 3, blocks 2+3, or a second block-2 borrow staging dz through cross_center)
- mechanism: s4 had only tried the block-2 and blocks-1+2 placements of the cross_point borrow on the ret-var chassis; more borrows should mean more register pressure
- probe: sandbox --disable all on the 45 body with the borrow added to block 3, with the borrow only in block 3, and with a second block-2 borrow staging dz through cross_center
- result: 45 / 93 insns in all three cases - the same allocation, still five callee-saves. On this chassis a borrow of a local OTHER than the ret pseudo is neutral; only staging through the ret pseudo itself moves the allocation.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, c1/c2/c3 bodies, one or two variable-reuse borrows present in each

## [s5] The ret = 0 restore that repairs the permuter's find can be spelled somewhere cheaper than an extra insn at the end of block 2
- mechanism: the restore costs a `move v1,zero` in the second bltz's delay slot, one of the two insns separating the 26 body from the target; alternative spellings that keep the second exit returning 0 (zero inside the branch, an inline `return 0;`, a separate `zero:` label) should be free if jump.c cross-jumps them
- probe: sandbox --disable all on the restore moved inside the second exit's branch, the second exit spelled `return 0;` inline with no restore, a separate `zero: return 0;` label with no restore, and a live READ of ret in block 2 (`... + ret`) instead of the staging
- result: 45, 45, 45 and 58. Every spelling that removes the restore statement also removes the pressure and the sixth callee-save with it, so the restore is load-bearing rather than incidental. The only 26-scoring alternative is the same restore hoisted one statement earlier (before the second if), which compiles to the identical object.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, g1/g2/h1/h2 bodies, one variable-reuse borrow (the ret staging) present in each

## [s5] Declaring ret uninitialised and assigning the zero at the exits, so its live range does not span block 1, keeps the sixth callee-save while shortening the pseudo
- mechanism: the s4 frontier's leading idea - the target's $v0 is set in the first bltz's delay slot, so its live range begins after block 1 and a carrier that does not span block 1 should match it more closely
- probe: sandbox --disable all on the 26 body with `s32 ret;` uninitialised and `ret = 0;` at the first exit, with the dz staging on both block-2 products and on the first product only, plus the same shape with no staging at all
- result: 30 / 95 insns for both staged variants and 41 for the unstaged one. The 30 body has NO sw s5 and the wrong first exit (bgez + j + move v1,zero) - dropping the top initialisation costs both halves at once, so the top-initialised `s32 ret = 0;` is required on this chassis.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, f1/f2/b4 bodies, one variable-reuse borrow present in f1/f2 and none in b4

## [s5] The permuter improves on the repaired 26 body by further randomisation of that same chassis
- mechanism: decomp-permuter randomizer over the 26 base (permuter base score 365) with fresh seeds; a basin yields early or not at all
- probe: campaign tmp/grind/func_8002E6B0/s5/perm_e, 11,782 iterations / 444 s, 9 novel finds, then harvest --stop
- result: Best find 265, but every sub-base find is the same semantically-broken family the permuter already produced on the other chassis - a dead `ret = 0;` moved ABOVE the staging (so the second exit returns dz), or `dx` clobbered by a staged `center_z - arg0[2]` before its later use. Repairing the dx shape by hand (recomputing the expression, or restoring dx) measures 26 again, i.e. a tie. No legal improvement came out of this basin.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, the 26 body (e7/d2) with its single ret-staging borrow present, decomp-permuter with --stack-diffs

## [s6] SYNTHESIS PASS - merged attack, frontier reset

The ledger's contradiction rule does not fire here: no chassis is marked
foreclosed, the floor has dropped in each of the last two working sessions
(40 -> 34 -> 26), and the residual is now TWO instructions plus one consistent
register permutation. This function is not near exhaustion; it is near a match.

### [s6] The seat is a hard-register CONFLICT, not an allocno priority
- mechanism: global.c:895-909 `prune_preferences` masks every preference set by `hard_reg_conflicts[allocno]`; global.c:580 dumps the graph after pruning and before allocation. Pseudo 96 (ret) is allocno ord 0 with pri 17368 - allocated FIRST - but its conflict line carries hard reg 2 ($v0) and it has no preference line, so find_reg cannot seat it in $v0 no matter how the C is spelled around refs/live-length/birth-order.
- probe: `pwsh tools/grinder/dump.ps1 func_8002E6B0` with the 26 body installed; read `tmp/grind/func_8002E6B0/s6/greg_seg.txt` (conflicts + preferences) and `lreg_seg.txt` (the RTL: hard $v0 appears only in insn 236 `(set (reg/i:SI 2 v0) (reg/v:SI 96))` with REG_DEAD on 96, and insn 237 `(use (reg 2))`; no $v0 clobbers anywhere)
- result: reg 96 conflicts with 2/29/64/66 and has zero preferences; reg 94 and reg 95 conflict with 29 only and both prefer 2/3/4; reg 95 (ord 1) takes $v0. s1's "allocno priority + local-alloc invisibility" explanation of the ret seat is superseded - ret has the HIGHEST priority in this body and still cannot reach $v0.
- verdict: CONFIRMED

### [s6] The ra_solver forward model reproduces this function well enough for inverse.py's verdict to be trusted
- mechanism: `simulate.py` replicates allocno_compare + prune_preferences + find_reg; `inverse.py global` searches input perturbations from the SIMULATED baseline
- probe: `extract.py func_8002E6B0 code6cac_b`; `simulate.py`; `inverse.py global --goal 96->2, 95->3 --depth 2 --top 8`
- result: sort order MATCHES but dispositions are 11/31, with misses at ord 0/1/2 (96 sim=$a1 dump=$v1, 95 sim=$v1 dump=$v0, 94 sim=$a0 dump=$v1). inverse.py prints the simulated baseline (96='$a1') and then a NEGATIVE RESULT; that negative is about a baseline this function does not have, so it is not a foreclosure of the seat. The s5 frontier's "type the ret seat REACHABLE or FORECLOSED with the solver before any further spelling" is not available on this function until the forward model is fixed for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, the 26 body (base26.c) with its single ret-staging FAKE borrow present; tools/ra_solver at its current commit

### [s6] Some other block-2 or block-3 staging position keeps the sixth callee-save while letting the returned zero survive without a restore insn
- mechanism: s5 frontier item 2 - the restore exists only because our block 2 clobbers the carrier, so a carrier clobbered late enough (or in block 3, where the final `ret = (cc ^ cp) >= 0;` overwrites it anyway) should need no restore
- probe: five hand variants measured with sandbox --disable all - p1 (staging in block 2's second product), p2 (staging `center_x - arg0[0]` instead of dz), q3 (both), p3 (staging dx instead of dz), p4 (staging in block 3 only, no restore), p5 (staging in blocks 2 and 3)
- result: p1 = 26 and p2 = 26 and q3 = 26 but all three are BYTE-IDENTICAL to the base object (md5 921b8948), so they are respellings, not new geometry; p3 = 42; p4 (the only genuinely restore-free staging) = 48; p5 = 34. Every staging that needs no restore also loses the pressure that buys $s5 - the same coupling s5 measured for the restore's position, now measured for the staged VALUE and the staging SITE as well.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, p1/p2/p3/p4/p5/q3 bodies on the 26 chassis, one (p1/p2/p3/p4) or two (q3/p5) variable-reuse staging borrows present

### [s6] The xor operand order at either the first if or the final assignment moves the ret seat off $v1
- mechanism: the xor operands are pseudos 94/95, the two allocnos that hold the `preferences: 2 3 4` competing for $v0; swapping the operand order changes which of them dies where and could change the pruned preference sets
- probe: sandbox on q1 (final statement's xor swapped only) and q2 (first if's xor swapped only), then the normalised side-by-side against the assembled target (`tmp/grind/func_8002E6B0/s6/cmp2.sh`)
- result: both = 26 / 96 with DISTINCT objects (md5 078e7ad4 and 1b1fa1c0 vs the base's 921b8948), and both show the identical residual - ret still in $v1, `move v1,zero` still in the second bltz's delay slot, trailing `move v0,v1` still present. The seat is invariant to xor operand order. Kept as fresh permuter basins at the floor.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, q1/q2 bodies on the 26 chassis, one variable-reuse staging borrow present

## [s6] Live frontier (reset - ranked)
1. **Find and remove pseudo 96's hard conflict with $v0. This is the whole residual.** Hard $v0 occurs in exactly two insns of the pre-global RTL (the epilogue copy and its `use`), and global.c processes REG_DEAD before mark_reg_store, so the copy alone should not create the conflict - yet nineteen other long-lived pseudos also conflict with reg 2 (and with FIXED reg 29, which `mark_reg_store` skips), which can only come from `hard_regs_live` seeded at block entry (global.c:695-698 + `record_conflicts` at global.c:725). NEXT PROBE: run the instrumented cc1 (`tools/gcc-2.7.2/cc1`) on this body and print `basic_block_live_at_start`'s hard-reg part for each block, or add an env-guarded print in `record_conflicts` / `record_one_conflict`, to learn WHICH block entry marks $v0 live. Then ask the C question: which exit geometry stops flow from carrying $v0 live into those blocks. Do NOT spend more sandbox measurements on spelling until that print exists - s6 measured eight spellings and the seat did not move once.
2. **A permuter campaign on q1 or q2 - the two DISTINCT objects that tie at 26.** s5 saturated the 26 basin (perm_e, 11,782 iterations, every find semantically broken) but seeded on the base object; q1 and q2 compile to different objects from different source geometries, so they are different neighbourhoods. Use `tmp/grind/func_8002E6B0/s5/mkws.sh` (it carries the current -mel -msoft-float flags). READ EVERY FIND'S SEMANTICS before scoring it - three of this function's permuter bests have been semantically invalid, and both floor drops on record came from hand-repairing one.
3. **Fix `tools/ra_solver`'s forward model for this function, then re-run the inverse.** simulate.py is 11/31 here and misses at ord 0, so the whole solver suite is currently unusable on func_8002E6B0. The misses are all in the first three allocnos, which are exactly the $v0/$v1/$a0 group - a small, well-bounded discrepancy, and fixing it would type the seat REACHABLE or FORECLOSED instead of leaving it a guess. Lower priority than (1) only because (1) may answer the same question directly.

## [s6] The ret seat in $v1 rather than $v0 is caused by allocno priority (the s1 explanation), so raising ret's refs or live span can move it
- mechanism: global.c allocno_compare orders allocnos by priority; s1 recorded ret as low-priority and never allocated first. Re-checked against the ALLOCDBG/.greg ground truth for the CURRENT 26 body rather than the s1 chassis.
- probe: pwsh tools/grinder/dump.ps1 func_8002E6B0 with the 26 body installed in src/code6cac_b.c; read tmp/grind/func_8002E6B0/s6/greg_seg.txt (the ';; 31 regs to allocate' line, the per-allocno conflict lines and preference lines) and lreg_seg.txt (the pre-global RTL); cross-check with tools/ra_solver/extract.py's allocdbg stream.
- result: SUPERSEDED and replaced by a confirmed mechanism. Pseudo 96 (ret) is allocno ORDER 0, pri 17368, nrefs 11, livelen 19 - it is allocated FIRST, so priority is not the obstacle. Its .greg line reads ';; 96 conflicts: ... 2 29 64 66', i.e. a hard-register conflict with $v0 (reg 2), and it carries NO ';; 96 preferences:' line at all. The two xor operands reg 94 and reg 95 carry no conflict with reg 2 and both carry 'preferences: 2 3 4'; reg 95 (ord 1) takes $v0 and reg 94 takes $a0. The missing preference is a consequence of the conflict: prune_preferences masks every preference set by hard_reg_conflicts (global.c:907-909), deleting the copy preference that set_preference records for the epilogue copy (set (reg/i:SI 2 v0) (reg/v:SI 96)). dump_conflicts runs at global.c:580 - after prune_preferences, before the allocation loop - so these are genuine pre-allocation inputs, not post-assignment fallout.
- verdict: CONFIRMED

## [s6] The ra_solver forward model reproduces func_8002E6B0 well enough that inverse.py's verdict on the ret seat can be trusted
- mechanism: simulate.py replicates allocno_compare + prune_preferences + find_reg; inverse.py global searches perturbations of refs / live length / birth order / conflicts / preferences starting from the SIMULATED baseline assignment.
- probe: tools/ra_solver/extract.py func_8002E6B0 code6cac_b (model built: 31 allocnos, 53 dispositions); tools/ra_solver/simulate.py on that model; tools/ra_solver/inverse.py global with the goal pseudo 96 to reg 2 and pseudo 95 to reg 3, depth 2, top 8. Log: tmp/grind/func_8002E6B0/s6/inverse_global.log
- result: Sort order MATCHES but dispositions are only 11/31, and the misses start at ord 0: pseudo 96 sim=$a1 vs dump=$v1, pseudo 95 sim=$v1 vs dump=$v0, pseudo 94 sim=$a0 vs dump=$v1. inverse.py therefore prints a baseline the function does not have (96='$a1', 95='$v0') and its NEGATIVE RESULT is a statement about that wrong baseline. The solver route named by the s5 frontier is not usable on this function until the forward model is fixed for it, and the printed FORECLOSED/negative must not be cited as a foreclosure of the ret seat.
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, the 26 body (tmp/grind/func_8002E6B0/s6/base26.c) with its single ret-staging variable-reuse FAKE borrow present; tools/ra_solver at its current commit

## [s6] Some other block-2 or block-3 staging position keeps the sixth callee-save while letting the returned zero survive without the ret = 0 restore insn
- mechanism: The s5 frontier's item 2: the restore exists only because our block 2 clobbers the carrier, whereas the target's $v0 holds 0 untouched across block 2. A carrier clobbered late enough - or clobbered in block 3, where the final ret = (cc ^ cp) >= 0 overwrites it anyway - should need no restore at all.
- probe: Six hand variants, each one sandbox --disable all measurement on the 26 chassis: p1 (staging moved to block 2's second product), p2 (staging center_x - arg0[0] instead of dz), q3 (both staged values, one restore), p3 (staging dx instead of dz), p4 (staging in block 3 only, genuinely restore-free), p5 (staging in blocks 2 and 3). Objects md5-compared against the base.
- result: p1 = 26/96, p2 = 26/96 and q3 = 26/96 but all three are BYTE-IDENTICAL to the base object (md5 921b8948a0460d02b617959aacbd2403), so they are respellings and not new geometry. p3 = 42/94. p4 - the only genuinely restore-free staging - = 48/95, i.e. 22 worse. p5 = 34/94. Every staging position that needs no restore also loses the block-1/2 pressure that buys the target's sixth callee-save, which is the same coupling s5 measured for the restore's POSITION, now measured for the staged VALUE and the staging SITE as well. Banked: rejected/s6_stage_dx_not_dz_42.c, rejected/s6_stage_block3_only_no_restore_48.c, rejected/s6_stage_b2_and_b3_34.c
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, p1/p2/p3/p4/p5/q3 bodies on the 26 chassis, one (p1/p2/p3/p4) or two (q3/p5) variable-reuse staging borrows present

## [s6] The xor operand order at the first if or at the final assignment moves the ret seat off $v1
- mechanism: The xor operands are pseudos 94 and 95 - the two allocnos that hold the 'preferences: 2 3 4' competing with ret for $v0. Swapping the operand order changes which of them dies where, so it could change the pruned preference sets and hence which allocno wins $v0.
- probe: sandbox --disable all on q1 (final statement's xor operands swapped only) and q2 (first if's xor operands swapped only), then the normalised objdump side-by-side against the assembled target via tmp/grind/func_8002E6B0/s6/cmp2.sh
- result: Both measure 26 / 96 and both are DISTINCT objects (md5 078e7ad4 and 1b1fa1c0 vs the base's 921b8948), but the side-by-side residual is identical in both: ret still in $v1, the restore still emits move v1,zero in the second bltz's delay slot, and the trailing move v0,v1 is still present. The seat is invariant to xor operand order. Kept as two fresh permuter basins sitting exactly at the floor: rejected/s6_final_xor_swap_distinct_object_ties_26.c and rejected/s6_first_if_xor_swap_distinct_object_ties_26.c
- verdict: KILLED
- kill_scope: instance
- measured_on: -mel -msoft-float chassis, INCLUDE_ASM on main, q1/q2 bodies on the 26 chassis, one variable-reuse staging borrow present
