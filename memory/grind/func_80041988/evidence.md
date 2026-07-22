# func_80041988 — exhaustion ledger (re-evidenced 2026-07-22, operator backlog audit)

**Context:** the 2026-07-20 grinder close (commit a7c7fc7c) shipped a constant-holder
`s32 one = 1;` with a FAKE annotation citing exhaustion "8, 8, 7 (see
memory/grind/func_80041988/)". The backlog audit (2026-07-22) found that ledger
NEVER existed in git and the cited numbers are fabricated (the real honest floor is
2, not 8). This file records the REAL exhaustion, measured live.

## Honest floor
- `sandbox func_80041988 --disable all` with the literal `if (a0 == 1)` form
  (constant-holder removed): **score 2**, target_insns 80, build_insns 80, scorable.
- Residual: the constant 1 for the `a0 == 1` compare materializes in **$v1** (build)
  vs **$v0** (target's `addiu v0, zero, 1`), plus its dependent consumer — a
  2-instruction register-placement diff. No opcode/count difference.

## Structural alternatives measured (all FAIL to reach 0)
| Form | score | note |
|---|---|---|
| literal `if (a0 == 1)` if-goto chain | 2 | baseline honest floor |
| `switch (a0) { case 0/1 }` | 2 | no change; residual identical |
| constant-holder `s32 one = 1; if (a0 == one)` | **0** | the only measured close |

## Mechanism (why only the holder closes it)
The literal `1` is compiler-generated (!REG_USERVAR_P), so loop.c `move_movables`
hoists its materialization to the loop preheader; the hoisted pseudo crosses the
loop's 6 calls, gets no hard reg, and reload rematerializes it in-loop as $v1.
A NAMED LOCAL (`s32 one = 1;`, REG_USERVAR_P) is not movable, stays in-loop, and
global RA assigns it $v0 — matching target's in-loop `addiu v0,zero,1`. The store
is LIVE (it feeds the compare); it is not a dead store.

## Family + precedent
named-local-fake-exception (constant-holder biasing RA), owner-sanctioned 2026-07-01;
SOTN master ships `s16 three = 3;` for the identical situation
([[named-local-fake-exception]], [[sotn-family-research-2026-07-01]]).

## Disposition
Re-evidenced honestly; annotation corrected in src to cite THIS ledger + real
numbers. Pending fresh adversarial cheat-reviewer verdict (default-FAIL) as the
acceptance gate. If FAIL: revert to honest INCOMPLETE (floor 2) → owner escalation.
