---
name: sched-rank-class-tie-wall
description: "When two ready insns are the two inputs of the SAME nearest successor, GCC 2.7.2's priority() gives them EQUAL height, so rank_for_schedule (sched.c:2399-2456) falls through to the dep-CLASS compare and the independent insn always wins — operand-order, decl-order and association-order rewrites cannot flip it. Diagnose before spending a session on ordering levers."
paths: [".claude/rules/sched-rank-class-tie-wall.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
metadata:
  type: reference
---

# sched1's class tie-break wall — why reordering the source doesn't move the load

## Symptom

A load lands one slot too early (or too late) relative to the target, and the
whole downstream register allocation shifts by one position as a result. You
rewrite the expression: swap the multiply operands, swap the addends, reassociate,
change declaration order, alias through a pointer — and every variant scores the
same. `cpu_get_dist` measured **24 variants** across two sessions converging on
one number; `CD_ready` measured **155+ hand forms**.

## Mechanism (`tools/gcc-2.7.2/sched.c`)

`rank_for_schedule` (**sched.c:2399-2456**) compares two ready insns in this
fixed order:

1. **`INSN_PRIORITY`** difference (the computed dependence *height*);
2. **dependence CLASS relative to `last_scheduled_insn`** (sched.c ~2435-2448) —
   a higher class (independent of the just-scheduled insn) **beats** a lower
   class (data-dependent on it);
3. **`INSN_LUID`** — source order, the last-resort tie-break.

The trap is step 1. `priority()` computes a **height dominated by the nearest
successor**. If your two contended insns are the two **inputs of the same**
downstream insn (the classic case: the two operands of one `mult`), they have the
*same* nearest successor and therefore **identical priority** — deeper successors
further down the chain do **not** raise either one. So the decision is made at
step 2, by dependence class, which is a property of the *dataflow*, not of the
source text.

`cpu_get_dist` in full: after sched1 places `sll $3,$3,1` (the cos index), the
ready set holds `lh cos_val, K($3)` (class 1, data-dependent on the `sll`),
`lw vx, 68($s0)` (class 3, independent) and `andi sin_idx` (class 3). Both loads
feed the same `mult`, both have `priority = 2` (read directly from the verbose
`.sched` dump). The class compare hands it to `lw vx`. `lw vx` then commits to a
register while `$v1` still holds the live cos-index scratch, so `vx` goes to
`$a0` instead of `$v1`, and every downstream pseudo shifts one seat.

## The consequence to internalize

**To move the low-class insn ahead, it would need a *deeper nearest successor*
than its rival** — impossible while the two are the symmetric inputs of one
operation. This structurally closes the whole
[[compare-operand-order-register]] lever class for that insn pair. Stop
sweeping orderings; the answer is either a different algebraic factoring of the
computation (a genuinely different expression tree with a different successor
graph) or a disposition.

`schedule_block` is also a **BACKWARD** list scheduler (higher clock T = earlier
program position; each insn is spliced before the previous `last`,
sched.c:3735/3742) — reason about the trace in that direction, not forwards
(func_8007DC9C [s24]).

## Diagnosis recipe

```
cc1 ... -da            # verbose .sched dump
```

Find the two contended insns and read their `priority = N` values.
**Equal priority is the tell.** If they differ, the ordering IS priority-driven
and source restructuring can plausibly move it; if they are equal, you are at
this wall. The instrumented cc1 (`BB2_SCHED_DEBUG` / `BB2_PRIO_DEBUG`, at
`tools/gcc-2.7.2/cc1` — **not** `build/cc1`, see [[instrumented-cc1-location]])
prints the `RANKDBG` / `T-<clock>` traces that show the ready set at each choice
point.

## Confirmed occurrences

| function | contended pair | ledger |
|---|---|---|
| `cpu_get_dist` | `lw vx` (cls 3) vs `lh cos_val` (cls 1), both `priority=2` | `memory/wip/cpu_get_dist/notes.md` |
| `CD_ready` | T-14 pair, both `pri=2`; decided at the **class** compare (line 2448), not the LUID fallback — the s25 entry corrects hypothesis 1, which had blamed line 2455 | `memory/grind/CD_ready/evidence.md` [s25] |
| `CD_sync` | clock 13, insns 111/121, both `pri=0x7f000001`, class 3 both, cls diff 0 across all 51 RANKDBG samples | `memory/grind/CD_sync/hypotheses.md:910` |
| `func_8007DC9C` | counterfactual: equalizing priority does **not** flip the order — the load hazard alone reproduces it, with the LUID tie-break as a third backstop | `memory/grind/func_8007DC9C/evidence.md` [s24] |

`CD_sync`/`CD_ready` are the useful contrast case: there the class compare is
also 0, so the decision does reach the LUID fallback — and LUID is fixed by the
pre-sched RTL chain order, which `sched_analyze` assigns sequentially. Both
sub-cases end in "not source-C reachable"; they differ in which compare decided.

## Related
- [[compare-operand-order-register]] — the lever class this wall closes; read
  this rule first to find out whether it can work at all.
- [[loop-exit-work-inside-loop-sched-fence]] — the `NOTE_INSN_LOOP_BEG` fence at
  sched.c:2067-2095, the *other* way source shape reaches sched1 (by splitting
  the basic block rather than by re-ranking). `cpu_get_dist`'s HEAD cheat was a
  `do{}while(0)` abusing exactly that fence — forbidden, and the reason its
  honest floor is 15 not 8.
- [[register-alloc-pure-c]] — the cascade this wall produces is a register
  diff; do not misdiagnose it as an allocation tie.
- [[instrumented-cc1-location]] — where the debug cc1 actually lives.
