---
name: sched-rank-class-tie-wall
paths: [".claude/rules/sched-rank-class-tie-wall.md"]
description: "Two ready insns feeding the SAME nearest successor have equal sched priority; rank_for_schedule then decides by dep CLASS (independent wins) — operand/decl/association reorders can't flip it. Diagnose first."
metadata:
  type: reference
---

# sched1's class tie-break wall

## Symptom

A load lands one slot early/late vs target and downstream registers shift by one seat. Every operand-order,
addend-order, reassociation, declaration-order or pointer-alias rewrite scores the same (cpu_get_dist: 24
variants; CD_ready: 155+ forms).

## Mechanism (`tools/gcc-2.7.2/sched.c`)

`rank_for_schedule` (sched.c:2399-2456) compares two ready insns by, in order:

1. `INSN_PRIORITY` (dependence height);
2. dependence CLASS relative to `last_scheduled_insn` (~2435-2448) — independent beats data-dependent;
3. `INSN_LUID` (source order).

`priority()` is dominated by the nearest successor, so two insns that are the inputs of the SAME downstream insn
(e.g. both operands of one `mult`) have identical priority, and step 2 decides — a property of dataflow, not of
source text. To move the low-class insn ahead it would need a deeper nearest successor, impossible while the two
are symmetric inputs of one operation. This closes the [[compare-operand-order-register]] lever class for that
pair: the remaining options are a genuinely different factoring of the computation (different successor graph)
or a different modality. `schedule_block` is a BACKWARD list scheduler — read traces in that direction.

## Diagnosis

`cc1 ... -da` verbose `.sched` dump: read `priority = N` of the two contended insns. **Equal priority is the
tell.** If they differ, source restructuring can plausibly move it. The instrumented cc1
(`tools/gcc-2.7.2/cc1`, `BB2_SCHED_DEBUG` / `BB2_PRIO_DEBUG`; not `build/cc1`) prints the
`RANKDBG` ready-set traces. If the class compare also ties, LUID decides — fixed by pre-sched RTL order.

## Related

[[compare-operand-order-register]] · [[loop-exit-work-inside-loop-sched-fence]] (the other way source shape
reaches sched1: splitting the block) · [[register-alloc-pure-c]]
