---
name: sandbox-zero-retire-fails
paths: ["src/*.c", "regfix.txt"]
description: "Engine gotcha: sandbox --disable all == 0 but retire FAILS the SHA1. Cause = a tier-3 barrier still in the C source that the regfix rules compensated for. Strip the barrier AND the rules."
metadata:
  type: reference
---

# `sandbox --disable all` says 0, but `retire` rolls back

## The symptom (engine workflow)

You take a queue item, route it `C`, and `sandbox <func> --disable all` reports
**score 0**. By the loop that means "already matchable in pure C — just retire."
But `retire <func>` comes back `ok:false` with a non-oracle SHA1 (auto-rolls
back). The masked-0 caveat in CLAUDE.md says "a 0 can hide a real register diff;
keep editing" — but here the gap is **not** ordinary reg-alloc work you have to
grind. It is mechanical, and this is why.

## Why the two disagree

`sandbox --disable all` does **two** things: disables regfix/asmfix rules **and
strips tier-3 inline asm** (`strip_tier3: true` in its output — note the
`tier3_stripped` count). `retire` does only **one**: it deletes the function's
regfix/asmfix rules. It does **not** touch source-level `__asm__`.

So the three builds are:

| build | barrier in src | rules | result |
|---|---|---|---|
| `build/` (oracle) | present | present | = target ✓ |
| `sandbox --disable all` | **stripped** | disabled | = target ✓ → score 0 |
| `retire` (rules only) | **present** | removed | **≠ target** ✗ |

When a tier-3 barrier is present *without* the rules that were compensating for
it, codegen diverges. The sandbox hid this because it stripped the barrier too.
The score-0 is real — it just describes the *fully* cheat-free build, which
includes removing the source asm that `retire` leaves behind.

## The fix

The function carries **two coupled cheats**: a source-level tier-3 `__asm__`
barrier AND regfix rules that paper over the register shuffle the barrier
causes. Remove **both**:

1. Delete the tier-3 `__asm__` line from `src/<file>.c` (it is debt, not a
   match — [[inline-asm-tiers]]).
2. `retire <func>` to drop the now-unneeded regfix rules and SHA1-gate.

`sandbox --disable all == 0` is the proof the pure-C (no-barrier, no-rules) form
matches; you just have to make the source actually pure-C first.

## Confirm before editing (decisive, ~5s)

Disassemble `build/src/<file>.o` (canonical) and the sandbox's
`tmp/sandbox/<func>/<file>.o` (stripped), extract the function, and index-align
the instruction streams (full operands, **not** masked). If they are
**byte-identical**, the barrier is pure debt and removing it cannot change the
result — retire will pass once it's gone. (`tmp/norm_diff.py` from the
func_800233AC session does this: objdump → strip addr column + absolute branch
targets → compare index-aligned.)

## The barrier shapes that trigger this

The classic is an **identity reload** that forces a value into a register
without changing it:

```c
s16 *tbl = &D_8008EB40;
__asm__ volatile("" : "=r"(tbl) : "0"(tbl));   /* tbl = tbl, in a register */
```

GCC, fed this, materializes `tbl` differently and shuffles v0/v1 (or other
adjacent regs) across the next few instructions — and someone then added
`subst "$3,..." "$2,..."`-style regfix rules to swap them back. Other shapes:
`__asm__ volatile("" ::: "memory")` scheduling barriers, and the
[[inline-move-aliasing]] `move %0,%1` idiom (now diagnostic-only). All are
tier-3; none belong in a committed match.

## Confirmed case — func_800233AC (code6cac.c, 2026-05-26)

Queue top, verdict C, distance 0, 4 regfix rules (all `subst` swapping
`$2`↔`$3` at maspsx idx 27–31). `sandbox --disable all` = 0; first `retire`
rolled back (504e1798… ≠ oracle). The function had one
`__asm__ volatile("" : "=r"(tbl) : "0"(tbl));` barrier on the `&D_8008EB40`
table pointer. objdump of `build/` vs the stripped sandbox object was
byte-identical (0/167 diffs). Removing the barrier + `retire` (drops the 4
rules) → SHA1 == oracle, 100% pure C. The 4 rules existed *only* to undo the
barrier's register shuffle.

## Related
- [[inline-asm-tiers]] — tier-3 barriers are debt, never a committed match
- [[inline-move-aliasing]] — the move-barrier variant (diagnostic-only)
- [[register-alloc-pure-c]] — when the gap is *genuine* reg-alloc (no barrier to
  strip), these are the pure-C levers
- [[register-asm-pins]] — pins are the same kind of debt as these barriers
