---
name: restore-discarded-return-displaces-v0
description: Caller captures a return the void impl discards; restoring `return ret;` forces $v0 live across an intermediate global reload, displacing it to $v1 — retires a 2-rule `$v0`→`$v1` subst cluster
paths: [".claude/rules/restore-discarded-return-displaces-v0.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
---

# Caller captures a return the void impl discards — restore it to displace $v0

## Symptom

A function carries a 2-rule cluster `subst "$2" "$3"` swapping `$v0` → `$v1`
on a **post-call** global lw/sw pair, e.g.:

```
func_XXX: subst "\$2" "$3" @ N      # the lw $v0, GLOBAL($gp)
func_XXX: subst "\$2" "$3" @ N+2    # the sw $v0, OTHER_GLOBAL($gp)
```

The body is a simple wrapper ending in:

```c
void func_XXX(s32 arg0) {
    ...
    SomeCall(...);              /* declared returning s32 */
    GLOBAL_DST = GLOBAL_SRC;    /* GCC picks $v0; target uses $v1 */
}
```

The single regfix flips the reload's register because GCC's allocator picks
`$v0` (lowest free post-call) but target uses `$v1`.

## Diagnosis — check callers for a captured return

`grep -rn "func_XXX" src/ include/`. If a sibling .c declares the function as
`extern s32 func_XXX(...)` and writes `var = func_XXX(...)`, the caller is
relying on **the call's $v0 surviving the function epilogue**. The
implementation is `void` only by header convention — semantically the function
DOES return the last call's result. That's the "$v0 must stay live across the
reload" signal target was compiled from.

## Fix — make the implementation return the call's result

Restore the return path:

```c
s32 func_XXX(s32 arg0) {                    /* was void */
    s32 ret;
    ...
    ret = SomeCall(...);                    /* $v0 = ret, must survive */
    GLOBAL_DST = GLOBAL_SRC;                /* forced to $v1 — $v0 is busy */
    return ret;                             /* $v0 already holds ret; no extra move */
}
```

GCC's RA sees `ret` is live from the call to the `return`. The intermediate
`GLOBAL_DST = GLOBAL_SRC;` cannot reuse `$v0`, so the lw and sw both pick
`$v1`. The epilogue emits no `move $v0, ...` (the value is already there).
Target's asm matches byte-for-byte: no return-value materialization before
`jr $ra`, just the reload in `$v1`.

## When this applies

All four must hold:
1. **Caller-side evidence**: `extern <T> func_XXX(...)` in another .c, with
   the return captured. (Local prototype in the function's own file is
   `void` only because m2c reconstructed it from the impl's lack of return
   value.)
2. The call whose return target uses is the **last non-trivial call** in the
   function body.
3. After that call there is ONE statement that reads a global into a register
   and writes it to another global (or similar simple post-call reload).
4. The regfix cluster is exactly the `$v0` → `$v1` rename for those 1-2
   post-call insns.

If the caller does NOT capture the return (or no caller exists outside the
file), the implementation truly is void and this lever doesn't apply — the
register diff has a different cause.

## Why this is not a "cheats by any spelling" violation

The function semantically returns the call's result (the caller proves it).
We're not synthesizing a useless `return` to bend codegen — we're restoring
the return that matches the call-site contract. A human writing this function
from the caller's `size = func_XXX(...)` line would write `return ret;`
naturally. The lever is **alignment with the call-site contract**, not a
codegen hint.

## Confirmed case — func_8005B7C4 (text1b.c, 2026-06-06)

Queue top, verdict C, distance 2, 2 regfix `subst $2 $3 @ 31` and `@ 33`
covering `lw $v0, %gp_rel(D_800A3404)` and `sw $v0, %gp_rel(D_800A340C)` after
a `tslGlobalMemFree_8005C2A8` call. Caller `src/ings.c:392` writes
`size = func_8005B7C4(0x801D8800);` with `extern s32 func_8005B7C4(u32);`.
Changing the impl from `void` → `s32 ... return ret;` (capturing the
tslGlobalMemFree call's return into `ret` and returning it) → both regfix
rules retired, SHA1 == oracle, 100% pure C.

## Related
- [[store-const-reload-cse]] — sibling "post-store global reload" lever; there
  the fix is removing a saved local at the read site. Here the fix is forcing
  $v0 live by restoring the discarded return.
- [[register-asm-pins]] — pinning $v1 directly is cheat-asm and likely won't
  hold against $v0's default-ascending preference; this lever solves the same
  RA tie via call-site contract instead.
- [[register-alloc-pure-c]] — the general RA-via-C-structure playbook; this is
  a specific instance of "make a register live to displace its natural user".
