# func_800871D4 — BLOCKED (cc1-vs-cc1psx andi-fold divergence CONFIRMED)

## TL;DR
Bit-flag setter in main.c: reads D_8010280A (u16), builds two shift masks, clears a
table slot, RMWs four flag globals. HEAD has 2 cheat-asm andi blocks + empty scheduling
barrier. cc1psx calibration (2026-06-16) CONFIRMS: cc1psx generates `andi $v1,$a0,0xffff`
from `var_v1 = temp_a0 & 0xFFFF` (no fold); our cc1 folds it. Best honest floor: score=10.

## Confirmed gap (10 Levenshtein diffs, oracle=52 insns, candidate=50 insns)
1. Oracle: `lui a0; lhu a0` — candidate: `lui v1; lhu v1` (2 load-register diffs)
2. Oracle has `andi v1,a0,0xffff` at pos 3 (missing from candidate, 1 delete)
3. Oracle has `andi v1,a0,0xffff` at pos 13 (restore after else, missing, 1 delete)
4. a1/a2 register swap throughout (6 diffs: var_a2 in a1, var_a1 in a2 in candidate vs oracle)

## cc1psx calibration results (2026-06-16)
- func_testA (temp_a0 + `& 0xFFFF`): cc1psx generates `andi $3,$4,0xffff` (NOT folded)
- func_testB (direct `var_v1 = D_8010280A`): cc1psx generates `lhu $3` (no andi, same as cc1)
- Divergence CONFIRMED: cc1psx does not fold andi from u16-bounded source; our cc1 does.

## Rejected forms
- `var_v1 = temp_a0 & 0xFFFF` (one-expr): our cc1 folds the andi completely
- `var_v1 = temp_a0; var_v1 &= 0xFFFF` (two-stmt): generates move+andi, not single andi
- declaration-order swap (var_a1 before var_a2): no effect on register assignment, score=10

## Best candidate (candidate.c, score=10, cheat-reviewer PASS)
Direct form: `var_v1 = D_8010280A` (no temp_a0, no mask). Semantics differ from oracle
in else-path (oracle restores var_v1 to original D_8010280A via second andi; direct form
leaves var_v1 = D_8010280A-16 for the post-if computation). Score is 10 (honest floor).

## Next step (Adjudicator)
Park with confirmed cc1psx divergence. The oracle requires cc1psx-specific behavior
(no andi fold from u16-bounded lhu) that our cc1 cannot reproduce in pure C.
