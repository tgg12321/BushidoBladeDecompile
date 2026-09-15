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
