# func_8005763C — retro-audit fix-forward (owner Q49), 2026-09-30

Archived landing ledger: `memory/grind/_completed/func_8005763C/ledger.md` (landing 23068ab77, 2026-09-22).

## The concern (retro-audit 2026-09-29, batch_01, owner Q49 "fix forward")

The landing ledger says layer-2 round 1 FAILed the first score-0 body because "two gotos entered the
interiors of branch bodies", and that "a whole-cluster structured ablation replaced those shared-label
ladders with four ordinary guarded cases". That is false for the body that landed (23068ab77, identical to
that commit's `candidate.c`, banked here as `probes/base.c`): its `check_bounds:` label sits inside two
nested `if` bodies of the general case, six `goto check_bounds;` statements enter it from outside, and a
`goto no_intersection;` jumps to the very next statement (a no-op). The round-2 PASS therefore rested on a
false statement about the body.

## Fix: a structured body with no gotos (cheat-cleanup)

`candidate.c` (2026-09-30): the six axis-aligned cases become one `if / else if` chain whose final `else`
holds the general case (its three degenerate rejections as early `return 0;`), and control joins at the
endpoint-tolerance test, which is written as one condition with an early `return 0;` before the success
scaling. No `goto`, no label. Also, every tolerance comparison is spelled `(a - b) >= -50` (the landed
body mixed in the equivalent `((a - b) < -50) == 0`).

Measurements (`sandbox --disable all`, 2026-09-30, via `tools/sandbox_sweep.ps1`):

| probe | shape | score |
|---|---|---|
| probes/base.c | landed body (interior-entry gotos, no-op goto) | 0/276 |
| probes/v1.c | if/else-if chain + general case in final else; tail kept as two `goto intersection_found` | 0/276 |
| probes/v3.c | v1 with the two tail tests joined by `||`, one `goto intersection_found` | 0/276 |
| probes/v4.c | v3 with the tail inverted: `if (!(all tests)) return 0;` then success, no goto | 0/276 |
| candidate.c (v5) | v4 with every `((a - b) < -50) == 0` spelled `(a - b) >= -50` | 0/276 |
| probes/v2.c | v1 with the success scaling inside the tail `if` (`{ ...; return 1; } return 0;`) | 29 (274/276) |
| probes/v6.c | v5 with `y = *arg9;` hoisted out of the comma expression to a statement before the test | 20 (274/276) |

Readings of the remaining constructs (unchanged from the landing, each ordinary C):
- `x1 = arg0; x1 >>= 3;` — a same-variable compound-assignment split (Ruling 4, ordinary-c-judge-decidable).
- `if (arg2 == x1) return 0;` in the general case — reachable (e.g. x1 == arg2, arg4 == arg6,
  arg1 != arg3, arg5 != arg7 falls through every axis case) and guards the `/ dx1` divisions.
- `y = *arg9` inside the third test's comma expression — `*arg9` is read only once the first two tests have
  passed, which is the order the target loads it (hoisting it costs 20, probes/v6.c).
