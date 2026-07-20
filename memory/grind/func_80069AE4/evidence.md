# Evidence bank — func_80069AE4

## s1 (recon, 2026-07-20) — full scheduling-mechanism map

- Baseline re-confirmed: canonical C (205 insns), sandbox --disable all = 2, 3 rules dropped.
- Exact diff: `addiu $a0,$sp,0x18` (= &s.sp18, arg of func_8007352C) sits at loop-body
  slot 3 in our build (`lw; addiu s0; addiu a0; ...`), slot 1 (branch target) in target.
  Everything else in the function is byte-identical.
- MECHANISM (from cc1 -dS/-dR dumps, tmp/grind/func_80069AE4/s1/text1b.i.sched{,2}):
  GCC 2.7.2 schedules each block in REVERSE (picks from block end; last picks land on top).
  Loop body = one block (insns 281..316). The three top insns — 281 lw, 284 addiu s0,
  300 addiu a0 — are ALL priority 1; ranking falls through class-test to the LUID tie-break
  (rank_for_schedule, sched.c:2399ff: priority, then dep-class vs last-scheduled insn,
  then INSN_LUID — higher LUID picked first = placed lower, preserving RTL order).
  Both sched1 (T-11 pick) and sched2 make the identical decision.
- Therefore target order REQUIRES LUID(addiu a0) < LUID(lw) < ... i.e. the a0-set must
  precede the lw in post-combine RTL. But expand emits the arg load at the call, and
  COMBINE merges `P = sp+24` into the arg load whenever P dies there (intra-block
  LOG_LINKS), placing the merged addiu at the CALL-SITE position. Confirmed empirically:
  named-pointer form is byte-identical to HEAD; even a diagnostic register-asm pin
  ($4, set first in source; scratch-only) produced the identical slot-3 output.
- cse folds pointer-addressed store uses `0(dst)` -> `24(sp)` (P2 probe emitted
  `sw v1,24(sp)`), so a second use via store-routing does NOT keep the pointer alive:
  it still dies at the arg load; combine still merges; the longer-lived temp perturbed
  RA (score 5). The whole store-routing family is dead.
- Before-loop pointer forms (rejected bank b–e) die differently: cross-block def escapes
  combine but the pseudo lives across the call/backedge -> callee-save -> `move a0,sN`.
- PASS-ORDER FACT (toplev.c): between reload and sched2 there is NO jump pass;
  jump2 = `jump_optimize(insns,1,1,0)` (cross-jump, no-op-move deletion, label cleanup)
  runs AFTER sched2 and BEFORE reorg (dbr_schedule). => Control flow that survives
  through sched2 and is deleted by jump2 yields final instruction orders that NO
  single-block schedule can produce. This is the only mechanism-consistent C-space
  left for addiu-at-top: the loop body was MULTI-BLOCK at scheduling time (a0-set in
  its own early block, blocking the combine merge cross-block), and the extra control
  flow vanished at jump2.
- Sibling signature: func_8005C8A8 / func_8005D814 / func_8005E098 (all INCOMPLETE,
  same func_8007352C EnvA idiom) show the SAME `addiu a0,sp,0x18` at block top far from
  its jal even in STRAIGHT-LINE post-call blocks — the original source idiom is
  family-wide, not loop-specific. Matched text1b.c callers of func_8007352C exist at
  lines ~12986/13086/13172-95 (`new_var2 = &s` named-local form) /17248/17411 — their
  emitted-vs-target addiu positions are un-audited (next-session lead).

- WIP rejected_form: {'form': 'HEAD loop, rules removed (control)', 'score': 2, 'reason': 'DIFF (sha cebf6fc4) — proves the 3 rules load-bearing.'}

- WIP rejected_form: {'form': 'pre-materialize `s32 *p=&s.sp18` INSIDE the loop, route stores+arg through p', 'score': 2, 'reason': 'DIFF — retire failed in prior session (sha e207b1e7, from card).'}

- WIP rejected_form: {'form': 'before-loop `s32 *sp=&s.sp18`, sp[0..2] stores + (s32)sp call arg', 'score': 2, 'reason': 'DIFF (sha 2cf4b042) — does not place addiu at loop top.'}

- WIP rejected_form: {'form': 'before-loop `s32 sp18_addr=(s32)&s.sp18`, struct-member stores + sp18_addr call arg', 'score': 2, 'reason': 'DIFF (sha 8a540cd5).'}

- WIP rejected_form: {'form': 'before-loop `s32 *sp`, q++ moved to bottom (after the call)', 'score': 2, 'reason': 'DIFF (sha 9a671107).'}

- == imported from memory/wip notes.md ==
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


- [s1] Baseline: canonical verdict C, 205 target insns, sandbox --disable all = 2 (3 rules stripped), re-verified at session end after revert

- [s1] Exact diff: addiu $a0,$sp,0x18 at body slot 3 (ours) vs slot 1 / branch target (target); rest of function byte-identical

- [s1] Loop-body top three insns are ALL scheduler priority 1 -> placement is purely the LUID tie-break; both schedulers agree (traces banked)

- [s1] combine pins the arg-address addiu to the call site whenever the pointer dies at the arg load; cse folds pointer-store addresses to sp-form, so no in-loop second use survives; before-loop defs (rejected b-e) go callee-save because they cross the call/backedge -> move a0,sN

- [s1] Pass order (toplev.c): reload -> sched2 -> jump2 -> reorg. Final orders unreachable by single-block scheduling ARE reachable if the body was multi-block through sched2 and jump2 deleted the extra control flow

- [s1] Sibling signature: func_8005C8A8/D814/E098 (INCOMPLETE, same EnvA/func_8007352C idiom) place addiu a0,sp,0x18 at block top even in straight-line post-call blocks -> family-wide original source idiom, not loop-specific

- [s1] Matched text1b.c callers of func_8007352C exist (~lines 12986/13086/13172-95 with `new_var2 = &s`/17248/17411) — emitted-vs-target addiu positions un-audited
