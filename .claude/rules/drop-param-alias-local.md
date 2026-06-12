---
name: drop-param-alias-local
description: Removing an explicit T *local = paramN; alias can free the param register for reuse by another local — retires register-asm pin clusters
paths: [".claude/rules/drop-param-alias-local.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
---

# Removing an explicit param→local alias frees the param register for reuse

## Symptom

A short function carries 2 register-asm pins:

```c
void func(u32 *a0, ...) {
    register u32 *t0 asm("t0") = a0;          /* alias param to a callee-save */
    register s32 size asm("a0") = 5;          /* reuse $a0 for an unrelated local */
    ...
    t0[N] = ...;                              /* body uses t0 */
}
```

Sandbox `--disable all` distance is small (≤ 4). objdump (stripped vs target)
shows the body is identical EXCEPT one local lands in a higher-numbered
register ($t1 in mine, $a0 in target) AND the prologue scheduling is swapped
(`li size,5` first in mine vs `move t0,a0` first in target). Pin #1 (the
param alias) "holds" but pin #2 (size→$a0) appears to be load-bearing — it's
the cheat the function depends on.

## Cause

The explicit `T *t0 = a0;` local creates a separate pseudo for the alias.
At RTL expand, GCC sees:
- pseudo for `a0` param (live from entry until the copy)
- pseudo for `t0` local (live from the copy through all `t0[...]` uses)
- pseudo for `size` (live from init through the byte store)

Param `a0` and `size` both want a low-numbered register. GCC's scheduler
puts `size = 5` FIRST (it's a cheap constant) and `move t0,a0` SECOND in
the prologue's delay slot. At the moment `size = 5` is emitted, param `$a0`
is still live (the alias copy hasn't happened yet) — so `size` cannot land
in `$a0`, it goes to `$t1`.

Pin #2 forces `size → $a0` despite this. The pin is acting as a cheat: it
papers over a scheduling decision that prevents the natural register reuse.

## The fix — drop the explicit alias, use the param directly

```c
void func(u32 *a0, ...) {
    s32 size = 5;                             /* no pins */
    ...
    a0[N] = ...;                              /* body uses a0 directly */
}
```

With no explicit `t0` local, GCC sees `a0` as having a long live range
(through all the `a0[...]` uses). The RA promotes `a0` to a callee-save
register ($t0) on its own, AND it schedules that promotion-move FIRST in
the prologue (because the long downstream chain gives it higher
INSN_PRIORITY than the cheap `li size,5`). With the move first, `$a0`
dies at insn 0, and `size = 5` naturally lands in `$a0` (lowest free
register per default ascending preference).

Both pins retire; zero source asm; pure C.

## When this applies

Look for the cluster:
1. A function with a `register T *<name> asm("$tN") = paramN;` pin aliasing
   a parameter to a callee-save.
2. PLUS a second pin `register T <other> asm("$paramN") = ...;` reusing the
   paramN register for an unrelated local (often a constant or short-lived
   value).
3. The body uses the aliased pointer in many places (the long chain that
   would naturally promote the param to a callee-save).
4. Sandbox `--disable all` distance is small (the only diff is the
   reused-register slot).

Drop BOTH pins and the alias local; the body uses the parameter name
directly. Verify with sandbox 0 + SHA1 == oracle.

## When this does NOT apply

- The body actually needs a separate alias because the param is reassigned
  elsewhere (mutating writes to the original `a0`). Then dropping the alias
  changes semantics.
- The other "reused-register" local has a *long* live range — its priority
  is comparable to the param's, so the natural ordering doesn't favor
  promote-first. May need additional levers.
- The function has only 1 pin (just the param alias). Then this isn't the
  pattern — the alias may be benign / load-bearing for some other reason.

## Confirmed case — initLoadImage (gpu.c, 2026-06-07)

Queue top, verdict C, distance 4, 0 rules, 2 register-asm pins (`u32 *t0
asm("t0") = a0;` + `s32 size asm("a0") = 5;`). Target asm: `move t0,a0` at
prologue head, `li a0,5` in beqz delay slot, size → $a0. My build (pins
stripped): `li t1,5` at prologue head, `move t0,a0` in delay slot,
size → $t1. Dropping the `t0` alias and using `a0` directly → sandbox 4 → 0;
SHA1 == oracle; 100% pure C, 0 pins.

## Related

- [[register-alloc-pure-c]] — the parent playbook. This is the INVERSE of
  Lever A (block-local var split): there you ADD a local to shrink a live
  range; here you REMOVE a local to LENGTHEN a chain so GCC promotes the
  param naturally.
- [[register-asm-pins]] — pins are diagnostic-only; this rule is one route
  to retiring them.
- [[inline-asm-policy]] — pins as committed source are cheat-asm; the
  function is INCOMPLETE until they're removed.
