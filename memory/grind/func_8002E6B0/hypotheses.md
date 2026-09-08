# Hypothesis ledger — func_8002E6B0

## Floor history
- pre-migration (2026-08-19): 64 (body G, cross_point-first, `return 0` exits)
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
