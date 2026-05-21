---
name: register-asm-pins
paths: ["src/*.c"]
description: "GCC 2.7.2 `register T x asm(\\"reg\\")` local pins are HINTS, not hard bindings. They hold when they agree with the RA's natural choice and are silently ignored when they fight a strong RA preference (e.g. reusing the longest-dead register). Reliable enforcement = pin + use as an `__asm__` input/output operand. When a pin is ignored, stop fighting it in C — regfix the register name."
metadata:
  type: reference
---


# Register-asm pin reliability — GCC 2.7.2

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

1. **Use the pin, but verify it took.** After a build, `dc.sh dump-text <func>` — if the
   pinned var isn't in `regname`, the pin was ignored.
2. **Don't escalate by adding more pins or restructuring C to "convince" GCC.** That was
   the time-sink in func_8002EA24. Once a pin is ignored, it will keep being ignored.
3. **Regfix the register name instead.** A failed pin is just a uniform register rename
   in the asm stream — `subst` rules. func_8002EA24 used 3 substs (`subu $4→$9` for the
   negu, `slt … $4→$9` ×2) and that was the whole fix. Cheap and deterministic.
4. **For values that MUST land in a specific reg, route them through an `__asm__`
   operand** — that binding is enforced. See [[inline-move-aliasing]] for the
   `__asm__ volatile("addu %0,%1,$zero" : "=r"(pinned) : "r"(src))` idiom, which both
   forces the redundant move AND enforces the destination register.

## Related

- [[register-alloc-pure-c]] — **retire the pin in PURE C** (block-local var
  split, narrow int type, REG_ALLOC_ORDER diagnosis) instead of regfixing the
  register name. Try this BEFORE accepting a pin or a regfix rename — it cleared
  all 5 pins on saTan0Main and the last on InitHiraRmd_80047FBC.
- [[inline-move-aliasing]] — asm-operand enforcement of register bindings
- [[quick-reference]] Part 4 (register-asm hints — where the line is)
