---
name: register-asm-pins
paths: ["src/*.c"]
description: "GCC 2.7.2 `register asm` pins are HINTS, not hard bindings. DIAGNOSTIC-ONLY: a pin/regfix is never a committed match -- strip it and find the C structure that makes GCC pick the register (see completion-standard)."
metadata:
  type: reference
---


# Register-asm pin reliability — GCC 2.7.2

> **Pins are DIAGNOSTIC-ONLY, never a committed match ([[completion-standard]]).**
> A `register T x asm("$N")` pin left in the committed source is cheat-asm, same
> kind as a regfix rule. Use a pin to *learn* which register the target wants; the
> finished match must make GCC choose that register through **C structure**, with
> the pin removed. The mechanics below (what holds vs. what's ignored) are for that
> diagnosis. **An ignored pin is NOT a cue to regfix the register or to inject an
> `__asm__` move** (both forbidden) — it's a cue that you haven't found the C
> structure yet.

`register T x asm("regname") = expr;` is a **hint**, not a hard binding. GCC 2.7.2's
register allocator tries to honor it but will silently override it when its own
preference is strong. Do not assume the pin "took" — check the build output.

**Why:** matching func_8002EA24 (commit 0993cf2), the pin
`register s32 neg_threshold asm("t1") = -threshold;` was silently ignored — GCC
emitted `subu $a0,$0,$a2` instead of `subu $t1,$0,$a2`. The same session's pins on
`x asm("a1")`, `dist_sq asm("a0")`, `shifted asm("v0")` all held. Identical syntax,
different outcome.

## What held vs. what was ignored (observed func_8002EA24 / func_8002D320)

| Pin | Init form | Held? |
|---|---|---|
| `t4 asm("t4")` then `t4 = obj + 0xF8` | uninit decl, later assign, **also used as `__asm__ "=r"` output** | ✓ always |
| `x asm("a1") = *(s32*)(obj+0x100)` | load destination | ✓ |
| `dist_sq asm("a0") = x*x + z*z` | arithmetic result (final `addu` to pinned reg) | ✓ |
| `shifted asm("v0") = (u32)a0_var >> shift` | shift result | ✓ |
| `min_y asm("a2")` / `max_y asm("a1")` | uninit decl, later assigned const / var-copy | ✓ |
| `tbl asm("a0")` | load dest, but **another live var also pinned to $a0** | ⚠ partial — lbu landed right, follow-on `sll` did not |
| `neg_threshold asm("t1") = -threshold` | negation result | ✗ ignored — landed in $a0 |

## The pattern

The pin **fails when it fights a strong RA preference.** `neg_threshold` was computed
right after the GTE block where `$a0` had been dead since instruction 1 (obj saved to
`$t0`). GCC's RA strongly wants to reuse the longest-dead register; the `$t1` hint lost
to that. `dist_sq`'s pin to `$a0` *agreed* with where the RA was already cycling values,
so it held.

The pin **holds reliably** only when:
- It is also the operand of an `__asm__` `"=r"` / `"r"` constraint — the asm constraint
  binding *is* enforced (this is why every `t4` pin works — t4 is always an asm operand).
- OR it agrees with / gently nudges what the RA would pick anyway.

Two pins on the same register with overlapping liveness (`tbl asm("a0")` while
`a0_var asm("a0")` is still in scope) → unpredictable; GCC honored one and not the other.

## How to apply

1. **Use the pin to DIAGNOSE, and verify whether it took.** After a build,
   `dc.sh dump-text <func>` — if the pinned var isn't in `regname`, the pin was ignored.
   Either way, the pin tells you which register target wants; that's its only job.
2. **An ignored pin means your C structure is wrong — not that you should force it.**
   Adding more pins, regfix, or an `__asm__` move to "convince" GCC is forbidden
   ([[completion-standard]]). Once a pin is ignored, it will keep being ignored.
3. **Find the C structure that makes GCC choose `regname` naturally** — declaration
   order, intermediate vars, the liveness pattern that agrees with target's allocator.
   A k-mer sibling that matched pin-free is the template. This is the actual work. See
   [[register-alloc-pure-c]] for the concrete levers (block-local var split, narrow int
   type, loop precompute, REG_ALLOC_ORDER diagnosis).
4. **Strip every pin before committing.** A `register asm` pin in the committed result
   is unmatched WIP. Do **not** "regfix the register name instead" and do **not** route
   the value through an `__asm__("move ...")` operand — both are cheat-asm
   ([[inline-move-aliasing]], now diagnostic-only). If the register genuinely cannot be
   reached in C, the function is either still-WIP (keep going) or, only for a physically
   un-compilable construct, canonical-asm ([[canonical-asm-retirement]]).

## Related

- [[register-alloc-pure-c]] — retire the pin in PURE C (block-local var split, narrow
  int type, REG_ALLOC_ORDER diagnosis) instead of regfixing the register name; cleared all
  5 pins on saTan0Main and the last on InitHiraRmd_80047FBC.
- [[inline-move-aliasing]] — asm-operand enforcement of register bindings
- [[quick-reference]] Part 4 (register-asm hints — where the line is)
