# func_80085270 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
SPU motion init (main.c:0x80085270). Clean pure-C body; matches ONLY with 2
EXTERNAL regfix reorder rules covering TWO INDEPENDENT cc1 list-scheduler
permutations. NOT closed this pass — dual-diff resists single levers; same hard
family as cpu_side_move_dir_4 / marionation_Exec.

## The two scheduling diffs
- Diff A (idx 1-4): cc1 = sll,sra,lui,addiu (sra right after sll, 1-cycle stall);
  target = sll,lui,addiu,sra (the `la D_80106F28` fills the sll->sra latency gap).
- Diff B (idx 51-59): cc1 schedules `move t0,s0` early + `li t1,127` late;
  target schedules both `li t2,64` + `li t1,127` before the lws and `move t0,s0`
  after them (post-load latency fill).

A single C lever is unlikely to fix BOTH independent windows at once.

## Resume steps (next session)
1. Instrumented .sched dump (BB2_SCHED_DEBUG, register-alloc-deep-dive.md) on
   BOTH diff windows — read the ready-list priority that makes cc1 NOT fill the
   sll->sra gap with the la (A) and NOT hoist the two li's before the lws (B).
2. Diff A: find a C ordering giving la's lui+addiu higher list-priority than sra
   at the gap cycle WITHOUT pulling la before sll (the named-local lever pulls it
   too early — collapse re-confirmed this pass).
3. cc1psx calibration cross-check (informational): does original PsyQ cc1psx
   schedule these gaps like target? Localizes the divergence to the scheduler
   port if so. (cc1psx is NOT the build compiler — calibration only.)

## Ruled out (do NOT re-derive)
- HEAD body, rules removed -> DIFF (both reorders load-bearing).
- named `s32 idx=(a0<<16)>>14` local -> DIFF, == HEAD (cc1 collapses it). [this pass]
- (prior card) named table local -> moved la before sll (wrong dir);
  split sign-extend a0_shl -> collapsed; named _tmp locals + hp=p -> 23 + cheat.

## Pointers
- `.claude/rules/register-alloc-pure-c.md`, `memory/project/register-alloc-deep-dive.md`
- `.claude/rules/cc1psx-calibration-only.md`
