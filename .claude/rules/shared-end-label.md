---
name: shared-end-label
paths: [".claude/rules/shared-end-label.md"]
description: "Switch cases each `return s2;` with a per-case constant, and GCC drops the `s2 = 0;` init in always-0 cases: route every case through `goto end; ... end: return s2;` so every assignment stays live."
metadata:
  type: recipe
---

# Shared end label keeps per-case return-value inits

## Symptom

```c
switch (state) {
case 0: ...; s2 = -1; return s2;
case 1: ...; s2 = 0;  return s2;   /* s2 = 0 dropped */
case 2:      s2 = 2;  return s2;
case 5: ...; s2 = 0;  return s2;   /* s2 = 0 dropped */
}
```

Ours lacks the `addu $sN,$0,$zero` the target has at the head of the always-0 cases: constant-prop folds
`return s2;` to `return 0;` and deletes the store.

## Recipe

```c
if (bounds_check_fails) goto end;
switch (state) {
case 0: ...; s2 = -1; goto end;
case 1: s2 = 0; ...; goto end;
case 2: s2 = 2; goto end;
case 5: s2 = 0; ...; goto end;
}
end:
    return s2;
```

At `end:` `s2` may hold several values, so the return cannot fold and every per-case store stays. A bounds-check
`goto end` matches the target pattern of returning the incoming `$s2`.

Applies when: 2+ cases set the same return variable, at least one always returns 0, and the target has the
zero-init at the case heads. Not when all cases return different non-zero constants or the cases exit through
genuinely different paths. Example: func_80077B30 (0f206e5).

Distinct from the FORBIDDEN `goto end` accumulator added to an otherwise single-return function solely to fill
prologue delay slots (no semantic purpose).

## Related

[[cross-jump-store-tail-merge]] · [[switch-break-shared-return-sched-hoist]]
