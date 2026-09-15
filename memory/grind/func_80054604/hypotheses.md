# Hypothesis ledger — func_80054604

## s1 (2026-09-15) frontier — residual is ONLY the callee-saved permutation (floor 26)
H1 (open): a6's pseudo must rank above a1 (860) in global.c allocno_compare. Doubled (REG_EQUIV,
   local-alloc.c:1064) it scores 588 with 6 refs; needs >=8 refs or reg_n_sets != 1
   (local-alloc.c:1024). Next: enumerate NATURAL source forms that add a surviving second set or
   two more refs of the buffer param (e.g. the buffer being advanced/re-derived, or the a6==0 arm
   assigning it), checking each with the .lreg "used N times across L insns" line for reg 78
   BEFORE looking at the score. Dead post-use stores are KILLED (flow removes them first).
H2 (open): a4 must tie-or-beat a5. Post-sched1 order a4,a6,a5 makes L(a4)=L(a5)+1. Next: find a
   block-0 shape where sched1 does not put a6's lw between them — e.g. a natural statement that
   occupies the T-2 slot before beqz, or a source order that separates the a4/a5 stores by one
   independent store. Verify with tmp/grind/func_80054604/s1/schedpos.py on the .sched dump.
H3 (dependent): once s2=a6,s7=a4,fp=a5, sched2 should emit lw a6,a4,a5 interleaved with the
   matching sw's as in the target (anti-dependence lw sN after sw sN); confirm, do not assume.

## [s1] A struct declaration for D_800EFAE8 accessed through a pointer local, with D_800EFB14 kept as a separate plain extern for the two stores, reproduces the base-register body of func_80054604.
- mechanism: Target addresses ctrl fields via s1 = lui/addiu D_800EFAE8 (offsets 0x00..0x4A) but stores D_800EFB14 via lui $at/sw %lo(sym); a pointer local puts the address in one pseudo before the branch (prologue placement), while the plain-extern store stays symbolic.
- probe: sandbox --disable all on body_v2 (candidate.c) vs the same typedef used as a direct global (probe_direct.c).
- result: v2 = 26 (body instruction-identical except callee-saved register names + prologue lw order); direct-global form = 82.
- verdict: CONFIRMED

## [s1] Sharing one local between the two func_8004153C result tests and the cleanup index (which crosses gpu_EnableDisplay) forces that pseudo into s0 and inserts move s0,v0 at both test sites on this chassis.
- mechanism: One user variable = one pseudo in GCC 2.7.2; allocno_calls_crossed > 0 over the third live range excludes call-clobbered regs for the whole pseudo (global.c find_reg), so v0 cannot be used at the first two sites.
- probe: body_v1 (shared tmp) vs body_v2 (separate v and n locals), sandbox --disable all.
- result: v1 = 32 with move s0,v0 / move a0,s0 at both sites; v2 = 26 with v0 used directly as in the target.
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, TU-local StageCtrl typedef + pointer local, no FAKE constructs present

## [s1] A dead store to the buffer param after its last use (a6 = 0 before return) undoubles a6's live length and lifts it above a1 in global.c allocation on this chassis.
- mechanism: local-alloc.c:1064 doubles reg_live_length only when reg_n_sets == 1 (gate local-alloc.c:1024); a second set would skip the doubling.
- probe: probe_a6dead.c: v2 + `a6 = 0;` before `return ret;`, sandbox + prologue disassembly.
- result: score 26, allocation unchanged (a6 still s4, a1 s2, ret s3): flow deletes the dead set before local-alloc counts sets. Also unchanged: a6 typed u8* (26), a0 += 0x131 in place of the id local (26).
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, body_v2 base, no FAKE constructs present

## [s1] Typing a4/a5 (or a1) as s16 changes the parameter copy path enough to reorder the callee-saved assignment toward the target on this chassis.
- mechanism: assign_parms did_conversion path skips the REG_EQUIV note (function.c:3824-3855), so the pseudo would not be doubled.
- probe: probe_a45s16.c and probe_a1s16.c, sandbox + prologue disassembly.
- result: a4/a5 s16 = 32: the prologue copies disappear entirely (loads move to the use site) and a2/a3 take s7/fp — wrong shape. a1 s16 = 36 (worse).
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, body_v2 base, no FAKE constructs present

## [s2] Giving the buffer param a LIVE second assignment by advancing it in place (`a6 += ret; game_StageCleanup(n, a6);`) removes the REG_EQUIV live-length doubling and reproduces the target callee-saved allocation and prologue load order.
- mechanism: local-alloc.c:1024/1064 doubles reg_live_length only when reg_n_sets == 1 (REG_EQUIV stack-parm copy, function.c:3824); a second set whose value is consumed survives cse/flow, so the pseudo is scored undoubled in global.c allocno_compare (global.c:642) and its prologue lw is no longer a birthing load in sched1, which also stops it separating a4's and a5's loads.
- probe: tmp/grind/func_80054604/s2/p_a6pluseq.c (compound form on the s1 body: 3, allocation fully target) then p_final.c (+ sign-test polarity + integer-typed data base): 0; integrated tree verify-oracle --rebuild: SHA1 match.
- result: sandbox --disable all = 0 (160/160 insns); oracle ok. Ordinary C under Ruling 4 (.claude/rules/ordinary-c-judge-decidable.md:167); no FAKE annotation needed for it.
- verdict: CONFIRMED

## [s2] H2 from s1 (a4 must tie or beat a5 via a block-0 shape) does not need a separate lever: it is resolved by the same second set, because a6's lw stops being scheduled between a4's and a5's loads once it is not a single-set REG_EQUIV pseudo.
- mechanism: sched.c reverse list scheduling / LUID tie-break (sched.c:2460) on block 0; the birthing-load treatment of the single-set pseudo was what placed it between them.
- probe: p_a6pluseq.dis prologue: lw s2,88(sp) | lw s7,80(sp) | lw s8,84(sp) (target order a6,a4,a5).
- result: confirmed on the closing form; H3 (sched2 sw/lw interleave) also confirmed there (final_integrated.dis prologue byte-identical to the target).
- verdict: CONFIRMED

## [s2] A no-op re-store of the buffer param in the else arm (`a6 = 0;` where a6 is already 0) survives cse/flow as a second set and yields the full target allocation on this chassis.
- mechanism: same local-alloc.c:1064 gate; the store is not provably dead to flow because the pseudo is live afterwards (read in the cleanup test).
- probe: tmp/grind/func_80054604/s2/p_a6zero.c / .dis.
- result: 4 (target allocation + lw order; residual = the extra `move s2,zero`). Semantically a no-op store: T2 fail, dead-store family at best -- NOT submitted; the ordinary-C compound form supersedes it.
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, s1 v2 body (TU-local StageCtrl + pointer local), no other FAKE constructs present

## [s2] A single forward-order param alias `s32 buf = a6;` (declared first, mid or last) reproduces the target allocation on this chassis.
- mechanism: REG_EQUIV replacement of a used-twice param copy moves the lw to the copy site; but the alias itself has one set, so its lw stays a birthing load in sched1 and still separates a4's and a5's loads.
- probe: p_a6local.c / p_a6local_first.c / p_a6local_mid.c + .dis.
- result: 10 in all three positions (a6 -> s2 correct; a4/a5 still swapped). FAKE-gated family anyway.
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, s1 v2 body, alias was the only FAKE-class construct present besides the pointer local

## [s2] A do { } while (0) wrap of the a6 != 0 arm (loop-depth ref weighting) lifts the buffer param above a1 on this chassis.
- mechanism: loop_depth weighting of reg_n_refs (regclass.c / flow.c REG_N_REFS increment by loop depth) inside the wrapped arm.
- probe: p_dowhile.c / .dis.
- result: 21, allocation still wrong (a6 not in s2). Not pursued further; the ordinary-C form closes the function.
- verdict: KILLED
- kill_scope: instance
- measured_on: post -mel -msoft-float chassis, s1 v2 body, do-while(0) wrap present un-annotated (probe only)

## [s2 re-run 2] The banked closing form (candidate_merge.patch) still scores 0 and passes the full oracle on HEAD c700d9136 with NO undefined_syms_auto.txt edit; the only blocker is the default single-stem grind scope (include/game.h, src/text1b_b.c, undefined_syms_auto.txt need a scope_allow.txt line).
- mechanism: n/a (integration surface, not codegen) -- integration-handoff-self-serve (owner ruling 2026-08-19): Judge ESCALATE(integration-handoff) -> driver writes the scope grant -> next session lands it through the normal gates.
- probe: git apply candidate_merge.patch on clean HEAD; sandbox --disable all; verify-oracle --rebuild --allow-dirty.
- result: 0/160; ok=true SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. INTEGRATION HANDOFF entry filed in docs/grind/decisions.md; outcome owner-gated.
- verdict: CONFIRMED

## [s2] The banked closing form (memory/grind/func_80054604/candidate_merge.patch: Unk800EFAE8Ctrl aggregate in include/game.h, FAKE-annotated pointer alias, ordinary-C compound split `a6 += ret; game_StageCleanup(n, a6);`, sibling consumers func_80054FDC/func_8005507C converted, src/text1b_b.c unused per-word externs removed) scores sandbox 0 and passes the full oracle on HEAD c700d9136 WITHOUT any undefined_syms_auto.txt edit.
- mechanism: Codegen unchanged from the first s2 attempt; the per-word rows in undefined_syms_auto.txt are link-time symbol addresses that the merged struct does not need (D_800EFAE8's own row supplies the base), so the config half of aggregate-merge prong (c) is byte-neutral and can land separately under a driver scope grant (integration-handoff-self-serve, owner ruling 2026-08-19).
- probe: python tmp/grind/func_80054604/s2/integrate2.py on clean HEAD; sandbox func_80054604 --disable all; verify-oracle --rebuild --allow-dirty; then git checkout of the three build files and verify-oracle --rebuild on the clean tree.
- result: sandbox score 0, target_insns 160, build_insns 160, rules_dropped 0 (metrics/events.jsonl 2026-09-15T16:36:27Z/16:36:32Z); verify-oracle ok=true, build_matches=true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa (16:36:44Z, git_commit c700d913). Clean-tree rebuild afterwards: ok=true. git apply --check of candidate_merge.patch clean against HEAD.
- verdict: CONFIRMED

## [s2] The undefined_syms_auto.txt rows D_800EFB14/18/1C/20 have a LIVE built referrer (asm/funcs/func_8005490C.s via src/text1b.c:1658 INCLUDE_ASM) and must STAY with the 2026-09-03 alias suffix, while D_800EFB0C/24/28 are named only by the unbuilt .s files of the already-C func_8005507C / func_80054FDC and must be DELETED -- avoiding the prong (c) defect that FAILed _clr at 10:19.
- mechanism: aggregate-merge prong (c) + the 2026-09-03 amendment: retained rows are justified only by a still-INCLUDE_ASM sibling that is actually built.
- probe: grep -l for each symbol across asm/funcs/*.s; grep INCLUDE_ASM func_8005490C in src/*.c; grep the seven names in src/ include/ after integration (clean).
- result: D_800EFB14: func_80054604.s, func_8005490C.s, func_80054FDC.s; D_800EFB18/1C/20: func_8005490C.s, func_80054FDC.s; D_800EFB0C: func_8005507C.s only; D_800EFB24/28: func_80054FDC.s only. func_8005490C is INCLUDE_ASM (src/text1b.c:1658); func_80054FDC and func_8005507C are C. No C names any of the seven after the patch.
- verdict: CONFIRMED

## [s3] Under the executed scope grant (tools/grinder/scope_allow.txt:76), applying candidate_merge.patch plus the seven undefined_syms_auto.txt row edits (suffix D_800EFB14/18/1C/20 with the alias comment; delete D_800EFB0C/24/28) lands func_80054604 at sandbox 0 with the full-tree oracle passing.
- mechanism: integration-handoff-self-serve (owner ruling 2026-08-19): the Judge ESCALATE(integration-handoff) verdict of s2 was executed by the driver as a scope grant; codegen is unchanged from s2, and the config half of aggregate-merge prong (c) is link-neutral because D_800EFAE8's own row supplies the base for the merged struct.
- probe: git apply memory/grind/func_80054604/candidate_merge.patch (clean); python tmp/grind/func_80054604/s3/edit_syms.py; sandbox func_80054604 --disable all; verify-oracle --rebuild --allow-dirty.
- result: sandbox score 0, target_insns 160, build_insns 160, rules_dropped 0; verify-oracle ok true, build_matches true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa. Dirty set = include/game.h, src/text1b.c, src/text1b_b.c, undefined_syms_auto.txt (all in scope) + pre-existing metrics/events.jsonl. No C names D_800EFB0C/14/18/1C/20/24/28 after the edit (only the include/game.h explanatory comment). Outcome: candidate-ready.
- verdict: CONFIRMED
