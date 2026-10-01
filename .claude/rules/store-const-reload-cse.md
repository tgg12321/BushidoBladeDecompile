---
name: store-const-reload-cse
paths: [".claude/rules/store-const-reload-cse.md"]
description: "Ours emits `li N` where target reloads a global after storing N to it: drop the saved-local reload and re-read the GLOBAL at the later test — the store kills the CSE entry. Never volatile."
metadata:
  type: reference
---

# Store-then-reload folded to `li` — re-read the global, don't save a local

## Symptom

Ours: `addiu $v1,$zero,2`; target: `lbu $v1, GLOBAL`. The C stores a constant and immediately reads the global
back into a local tested later:

```c
s32 v3 = GLOBAL;
if (v3 == 1) {
    if (flagsA || flagsB) {
        GLOBAL = 2;
        v3 = GLOBAL;        /* CSE forward-props the stored 2 -> li */
    }
}
if (v3 == 2) goto ...;
```

## Fix

```c
s32 v3 = GLOBAL;
if (v3 == 1) {
    if (flagsA || flagsB) {
        GLOBAL = 2;         /* kills the CSE entry for GLOBAL */
    }
}
if (GLOBAL == 2) goto ...;  /* re-read */
```

On the no-store path the entry value is still available and GCC reuses `$v1`; on the store path the store kills
it and GCC reloads with `lui;lbu %lo` exactly where the target does. func_8001E404: 2 → 0.

**Do NOT use `volatile`** (`*(volatile u8 *)&GLOBAL`): it is a forbidden coercion and measurably worse (full
address materialization, a stray `nop`, an extra `andi`).

## Related

[[u16-global-lhu-lbu-low-byte]] · [[register-alloc-pure-c]] · [[restore-discarded-return-displaces-v0]]
