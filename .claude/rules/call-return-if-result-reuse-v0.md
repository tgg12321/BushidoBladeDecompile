---
name: call-return-if-result-reuse-v0
description: Initialize an if/else-result variable from the call return to keep it in `$v0` through GCC's bnez+delay-slot+overwrite merge form — retires the "branch-sense + 2 constant-load + downstream-subu" rename cluster
paths: ["src/*.c"]
---

# Initialize the if-else result var from the call return to land it in `$v0`

## Symptom

A function carries a small cluster of regfix `subst` rules swapping a branch
sense (`beq`↔`bne`) plus two if/else constant-load opcodes plus the
downstream subu, all on the SAME register — a "mirror-image" rename of
target's `bnez ...; addiu $v0,$zero,A; addiu $v0,$zero,B; .L: subu rD,$v0,rS`
pattern into our build's `beqz ...; addiu $vN,$zero,B; addiu $vN,$zero,A;
.L: subu rD,$vN,rS` where `$vN` is `$v1` (or any non-`$v0`) and the
delay/fall-through values are swapped.

Example regfix shape:

```
func: subst "beq" "bne" @ K
func: subst "\$X,\$zero,B" "$2,\$zero,A" @ K+1
func: subst "\$X,\$zero,A" "$2,\$zero,B" @ K+2
func: subst "\$Y,\$X,\$Z" "$Y,$2,$Z" @ K+3
```

The function shape:

```c
if (call() != 0) base = A;     /* taken case = then */
else             base = B;     /* fall-through = else */
dist = base - other;
```

Sandbox `--disable all` distance is small (3-4), all diffs in the if-else
emission.

## Why GCC picks `$v1` over `$v0`

GCC's RA ends `$v0`'s live range at the `bnez $v0, .L` and picks the next
free natural register (`$v1`) for the if-result `base`. It does NOT
automatically reuse `$v0` in the bnez's delay slot — even though the branch
condition is "consumed" by the time the delay slot executes — because the
allocator's live-range model treats the result var as a fresh pseudo.

Target was compiled from a form that gives GCC's RA a reason to keep the
value in `$v0`: the if-result variable was **initialized from the call**, so
its initial def IS the `$v0` return. The subsequent conditional overwrite in
the bnez+delay-slot+fall-through-overwrite merge form stays in the same
register.

## The fix

Initialize the if-result with the call's return, then conditionally overwrite:

```c
{
    s32 base = call();
    if (base == 0) base = B;    /* fall-through case */
    else           base = A;    /* taken-branch case */
    dist = base - other;
}
```

Two things happen together:
- `base` starts in `$v0` (the call return), so its register class is set to
  `$v0` from the first def.
- The `== 0` test makes the `bnez` branch on the same register that already
  holds the call return AND the if-result, so GCC fills the delay slot with
  the "else" assignment (`base = A`) and the fall-through assigns `base = B`,
  giving the merge form `bnez $v0, .L; addiu $v0,$zero,A; addiu $v0,$zero,B`.

If the source semantics is `if (call() != 0) X = A; else X = B;`, write it
as `if (call() == 0) X = B; else X = A;` (swap then/else bodies) so the
branch-sense flip lines up with target.

## Why this is not a "cheats by any spelling" violation

The construct has semantic purpose: `base = call()` is a meaningful read of
the call's return value, and the conditional overwrite is a real semantic
operation (the call's return is examined and the variable's value depends on
it). A human programmer writing this function from its specification
("compute the constant based on the player-count check, then subtract") might
naturally write this form — caching the call return in `base` before testing
it is idiomatic when the test compares against 0 and the result is reused.
The lever is **idiomatic C-source alignment with target's register flow**,
not a no-op coercion.

## Confirmed case — `tslLineG5Init` (code6cac_c2.c, 2026-06-07)

Queue top, verdict C, distance 4, 4 regfix `subst` rules covering branch
sense + two `addiu` opcodes + the downstream `subu` (all renaming `$a0`/`$v1`
to `$v0` plus the `beq`↔`bne` flip). Three steps:

1. Split the if-result into a fresh block-local `s32 base;` (distance 4 → 4,
   register flipped from `$a0` to `$v1` — block-local split per
   [[register-alloc-pure-c]] Lever A).
2. Swap the if's branch sense — `if (count() == 0) base = 0x6590; else base
   = 0x55F0;` (distance 4 → 3, branch sense and delay-slot value match
   target).
3. Initialize `base` from the call: `s32 base = game_GetPlayerCount(); if
   (base == 0) base = 0x6590; else base = 0x55F0;` (distance 3 → 0).

`retire` dropped all 4 regfix rules; SHA1 == oracle; 100% pure C.

## When this does NOT apply

- The if-result must outlive the merge into a register that conflicts with
  `$v0` later (e.g., the value flows directly into `$a0`/`$a1` for an
  imminent call). In that case GCC picks the destination register early and
  no init-from-call hack will displace it.
- The call's return is genuinely consumed elsewhere (a second use besides
  the branch test), so reusing the slot for the if-result would shorten
  the call return's live range incorrectly.
- The if-else result is not a 2-constant select — for more complex bodies
  the merge form doesn't apply and the lever is moot.

## Related
- [[register-alloc-pure-c]] — the parent playbook; this is a specific
  application of "initialize a variable from an existing-register value to
  steer RA".
- [[restore-discarded-return-displaces-v0]] — sibling "keep `$v0` busy"
  lever, where the caller's captured return forces a register-rename
  cluster to retire.
- [[store-before-jal]] — another delay-slot scheduling lever; this rule
  is about the OPPOSITE side (using a delay slot to reassign the branch
  test register).
