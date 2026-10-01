---
name: restore-discarded-return-displaces-v0
paths: [".claude/rules/restore-discarded-return-displaces-v0.md"]
description: "Post-call global reload lands in $v0 where target uses $v1, and a caller captures the 'void' function's return: restore `return ret;` so $v0 stays live and the reload moves to $v1."
metadata:
  type: reference
---

# Caller captures a return the void impl discards — restore it to displace $v0

## Symptom

A wrapper ends with a call followed by a simple global copy; ours puts the copy in `$v0`, target in `$v1`:

```c
void func_XXX(s32 arg0) {
    ...
    SomeCall(...);              /* declared returning s32 */
    GLOBAL_DST = GLOBAL_SRC;    /* ours $v0, target $v1 */
}
```

## Diagnosis

`grep -rn func_XXX src/ include/`: another file declares `extern s32 func_XXX(...)` and writes
`var = func_XXX(...)`. The caller relies on the last call's `$v0` surviving — the function semantically returns
it; `void` is only m2c's reconstruction.

## Fix

```c
s32 func_XXX(s32 arg0) {
    s32 ret;
    ...
    ret = SomeCall(...);
    GLOBAL_DST = GLOBAL_SRC;    /* $v0 busy -> $v1 */
    return ret;                 /* already in $v0, no move */
}
```

Applies only when: (1) a caller captures the return; (2) it is the last non-trivial call; (3) one simple
post-call reload/store follows; (4) the diff is exactly that `$v0`→`$v1` seat. With no capturing caller the
function really is void and the diff has another cause. This is alignment with the call-site contract, not a
codegen hint. Example: func_8005B7C4 (caller `src/ings.c` `size = func_8005B7C4(0x801D8800);`).

## Related

[[store-const-reload-cse]] · [[register-alloc-pure-c]] · [[call-return-if-result-reuse-v0]]
