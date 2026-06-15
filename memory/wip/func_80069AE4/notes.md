# func_80069AE4 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
GPU-gauge init (text1b.c:0x80069ae4). Matches ONLY with 3 regfix
scheduling-paperwork rules. 2-insn **loop scheduling plateau**: target hoists
the loop-invariant `addiu a0,sp,24` (= &s.sp18, the func_8007352C call arg) to
the LOOP TOP as the branch target; GCC's list scheduler emits it late. NOT
closed this pass; instrumented .sched dump is the un-tried next modality.

## Sandbox infra note (CORRECTION to the park reason)
The park said "sandbox unmeasurable (sibling .L8006BDF4 label collision)".
In THIS private worktree the sandbox is SCORABLE: `score 2, scorable: true,
target_insns 205`. The infra block did not reproduce — measurement is fine now.

## The exact gap (loop scheduling)
Target loop (0x80069c5c is the bnez target):
```
80069c5c: addiu a0,sp,24    <- LOOP TOP / branch target (=&s.sp18, invariant)
80069c60: lw    v0,0(s0)    (v = *q)
80069c64: addiu s0,s0,4     (q++)
80069c68: sw    v0,24(sp)   (s.sp18 = v)
...
80069c7c: jal   func_8007352C   (a0 already set up at top)
```
GCC instead schedules `addiu a0,sp,24` near the jal. The 3 rules reorder the
window, add a stable `.L_LOOP_TOP` label, and retarget the bnez.

## Resume steps (next session)
1. INSTRUMENTED .sched DUMP — build tmp/gccdbg/cc1 with BB2_SCHED_DEBUG
   (register-alloc-deep-dive.md), isolated standalone of func_80069AE4, dump
   the loop scheduling DAG. Learn why the invariant addiu lands last in its
   ready window.
2. Try loop-invariant-hoist forms that present `&s.sp18` as a pure invariant
   GCC 2.7.2 loop.c pulls to the header (must stay re-materialized at the
   header, NOT spilled to callee-save — target recomputes the addiu each iter).
3. Directed permuter (PERM_*) on the loop body from this structural seed.

## Ruled out (do NOT re-derive)
- HEAD loop, rules removed -> DIFF (rules load-bearing).
- pre-materialize `s32 *p=&s.sp18` INSIDE loop, route through p -> DIFF (prior
  session, sha e207b1e7).
- before-loop `s32 *sp=&s.sp18`, pointer stores + (s32)sp arg -> DIFF (2cf4b042).
- before-loop `s32 sp18_addr=(s32)&s.sp18`, member stores + int arg -> DIFF (8a540cd5).
- before-loop pointer + q++ moved to bottom -> DIFF (9a671107).

## Pointers
- `.claude/rules/loop-exit-work-inside-loop-sched-fence.md`
- `memory/project/register-alloc-deep-dive.md` (instrumented-dump recipe)
