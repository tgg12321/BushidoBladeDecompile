---
name: shared-end-label
paths: [".claude/rules/shared-end-label.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "When a switch has multiple cases each ending with `return s2;` where s2 is a different constant per case, GCC constant-folds case 1's `return s2;` to `return 0;` and drops the `s2 = 0;` instruction. Restructure with `goto end; ... end: return s2;` — GCC sees s2 as potentially varying at end and keeps every `s2 = N;` assignment alive."
metadata:
  type: recipe
---


## Symptom

A switch-with-return function where each case sets a return variable and explicitly returns:

```c
switch (state) {
case 0: ...; s2 = -1; return s2;
case 1: ...; s2 = 0;  return s2;   // ← s2 = 0 dropped by GCC
case 2:      s2 = 2;  return s2;
case 3:      s2 = 3;  return s2;
case 5: ...; s2 = 0;  return s2;   // ← s2 = 0 dropped by GCC
}
```

After build, the function is missing `addu $sN, $0, $zero` instructions in case 1 and case 5 specifically — the cases where the return value is always 0. Target has them; mine doesn't.

Mechanism: GCC's constant-prop sees that case 1 always returns 0 (no path inside changes s2 from 0). It folds `return s2;` to `return 0;` and dead-store-eliminates the `s2 = 0;` assignment. Cases 0, 2, 3, 5 with non-zero returns (or paths that re-assign s2) are unaffected. Case 5 returns 0 unconditionally too — same fold.

## Recipe

Route all case paths through a single shared `end:` label that does the only `return s2;`:

```c
s32 s2;

if (bounds_check_fails) goto end;
switch (state) {
case 0: ...; s2 = -1; goto end;
case 1: s2 = 0; ...; goto end;
case 2: s2 = 2; goto end;
case 3: s2 = 3; goto end;
case 5: s2 = 0; ...; goto end;
}
end:
    return s2;
```

Why this works: at `end:`, GCC sees s2 as potentially -1, 0, 2, or 3 depending on which case ran. The shared end-block cannot constant-fold — it must do `addu $v0, $s2, $zero` (read s2 register). Once GCC can't fold the return, every per-case `s2 = N;` becomes load-bearing and stays in the output.

Bonus: the bounds-check `goto end` (instead of `return 0;`) matches the common target pattern where the failure path returns the CALLER's $s2 value (uninitialized $s2 at function entry). This is C-spec UB but matches GCC's actual emitted code.

## Pre-conditions for the pattern to apply

- Switch with 2+ cases where each sets the same return variable
- At least ONE case always returns 0 (the constant-fold trigger)
- Original target asm has `addu $sN, $0, $zero` at case heads / before computation

If you see the lost-codegen audit (`audit_asm_cheats.py`) flagging a function with `insert_after "addu $sN, $0, $zero"` rules, that's the same pattern manifested as a regfix cheat — the prior agent restored what GCC ate. Restructure to single-end-label and the inserts become unnecessary.

## Reference match

- `func_80077B30` (commit `0f206e5`, 2026-05-16). Switch on `D_800A35E4` with cases 0-5. Cases 1 and 5 always return 0; GCC was dropping their `s2 = 0;` inits. Prior commit (`60d8f76`) used 2 `insert_after` rules to restore them. The restructure removed BOTH inserts AND all 17 register/label substs that were paper-thinly compensating for the multi-return scheduling — the shared end-label gave GCC the natural shape for the whole function. Final: 100% pure C, zero regfix rules.

## When this does not apply

- Single-case switches or 2-case switches — GCC may not even use jtbl dispatch; the optimizer landscape is different.
- Cases that all return DIFFERENT non-zero constants — no constant-fold trigger, vanilla per-case `return N;` already works.
- Functions where the cases return through different paths (some return early, some fall through) — the shape doesn't unify into a shared end-label.

See [[lost-codegen-insert-cheat]] for the cheat pattern this recipe replaces.
