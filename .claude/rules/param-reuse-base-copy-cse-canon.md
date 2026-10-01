---
name: param-reuse-base-copy-cse-canon
paths: [".claude/rules/param-reuse-base-copy-cse-canon.md"]
description: "Target has a param-to-callee-save copy plus a base copy the honest build folds away: reuse the PARAMETER as the walking cursor, and name the call arg early so cse keeps the in-place advance."
metadata:
  type: rule
---

# Param-reuse walker + early-named call arg

## Symptom

A list/record walker whose target has TWO copies our build lacks (target 49 insns, ours 48):

```mips
move  s0, a0          # param copied to callee-save at entry
lw    v0, 0(a1)
move  s4, s0          # base copy, in the lw's load-delay slot
addu  s0, s0, v0      # walker advances IN PLACE
```

Ours keeps `arg0` in `$a0` until a single `addu s0,a0,v0`. (Historically faked with pins + an
`__asm__("move ...")` — forbidden.)

## Lever 1 — the walking pointer is the parameter itself

A separate single-set walker local (`s32 cached = arg0;`) is copy-propagated away. Reuse the parameter:

```c
base = arg0;        /* must materialize: arg0 is redefined while base is live */
arg0 += off;        /* multi-set param lives in a callee-save from entry */
count = *(s32 *)arg0;
arg0 += 4;
```

## Lever 2 — name the call's first arg early

If the advance still reads the copy (`addu s0,s2,v0`) with a 3-register rotation: cse.c `make_regs_eqv` makes
the longest-lived register canonical, so a call arg `base + off` expanded at the call site makes `base`
canonical. Name it at the TOP of the loop body, before the remaining reads/advances:

```c
do {
    s32 entry;
    entry = base + *(s32 *)arg0;   /* base's last use — early */
    arg0 += 8;
    dx_u = *(u16 *)arg0; arg0 += 2; /* walker's last use — later */
    ...
    func_callee(entry, dx + sx, dy + sy);
} while ((count--) != 0);
```

Combine folds `entry` back into the arg setup; only cse's canonical choice flips. `base` and `entry` carry real
meaning (list base, element pointer); `entry` is the frozen-list named-intermediate entry with its
prerequisites ([[no-new-park-categories]]). func_800483DC: 6 → 0.

## Related

[[hoist-call-arg-local-flips-jal-delay]] · [[drop-param-alias-local]] (the inverse lever) ·
[[register-alloc-pure-c]] · [[exit-path-return-set-cse-join]]
