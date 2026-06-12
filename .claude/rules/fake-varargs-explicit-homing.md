---
name: fake-varargs-explicit-homing
description: "A printf-style wrapper whose target homes arg registers as RTL-SCHEDULED body stores (interleaved/delay-slot) was NOT variadic in the original source — it's the pre-stdarg fake-varargs idiom: 4 named args + explicit stores through the va pointer. True `...` signatures emit bulk pre-subu pretend-args text stores."
paths: [".claude/rules/fake-varargs-explicit-homing.md"]
metadata:
  type: reference
---

# Fake-varargs wrappers: explicit arg homing, not `...`

## Symptom

A small printf/debug-print wrapper plateaus with regfix rules (or distance)
clustered on the **argument-register home stores** (`sw $a1..$a3` to the
incoming-arg area). Your build emits them as a **bulk block of pre-subu
stores** (`sw $5,4($sp); sw $6,8($sp); sw $7,12($sp)` BEFORE
`addiu $sp,-frame`), while the target has them **post-subu, scheduled into
the body** — interleaved with call setup, one possibly in the `jal` delay
slot, and the first named arg homed from a COPY register (e.g. `sw $v0`
after `$a0` was clobbered).

It is tempting to call this a varargs prologue scheduling divergence
(cc1psx vs fork). **It is not** — it's proof the original source was not
variadic.

## The decisive evidence (debug_printf, 2026-06-12)

1. **True-variadic functions in the TARGET binary** (DispSleepMenuTex
   0x538 frame, saTan0GaugeDraw, func_80044170) carry the same bulk
   pre-subu pretend-args homing our fork emits for `...` signatures. The
   original compiler handles `...` exactly like ours → a target WITHOUT
   that pattern was not `...` in the original source.
2. **Text-prologue stores can't schedule.** GCC 2.7.2's pretend-args homing
   is emitted as prologue text — it cannot land in a jal delay slot or home
   a param from a copy register. RTL-scheduled homes = named-param homing
   (addressability) or explicit C stores.
3. **GCC 2.7.2 homes ONLY the explicitly-addressable named param** (probe
   sweep: `(fmt,a,b,c)` + `&a` homes a alone — measured distance 4). Extra
   homed registers in target ⇒ the original STORED them explicitly.

## The matching shape (the 1998 idiom)

```c
void debug_printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;          /* &fmt → compiler homes fmt (from its copy reg) */
    ap[1] = a;               /* explicit homes — RTL stores, fully schedulable */
    ap[2] = b;
    ap[3] = c;
    func_80079244(1, fmt, ap + 1);   /* printf core walks the homed block */
}
```

Every store is live (the callee reads the block through the pointer), so
this is plain program logic — NOT the dead-param family. Contrast the
FORBIDDEN sibling spelling: `(fmt,a,b,c)` with b/c **unused**, declared
only to suppress the varargs prologue (in-session reviewer correctly
FAILed it, dead-param family, 2026-06-12). The distinction is whether
b/c are read/stored for the callee, not the signature arity.

## How to apply

1. Confirm the target's homes are body-scheduled (delay slot / interleave /
   copy-register home) — if instead the target shows the bulk pre-subu
   block, keep the `...` signature (it matches natively; see the
   DispSleepMenuTex family).
2. Write the named-arg + explicit-store form above; sweep spelling
   variants (`ap+1` vs `&a`, K&R) if the masked score isn't 0 first try —
   all 5 spellings hit masked-0 for debug_printf; prefer the most natural.
3. `retire` is the proof (masked-0 caveat applies).

## Related

- [[lost-codegen-insert-cheat]] — the rule family these regfix bridges
  fell under before retirement
- [[narrow-stack-param-subword-offset]] — sibling "read params through
  their home slots" SOTN-sanctioned pattern
- [[difficult-is-not-impossible]] — the park this disproved claimed
  "compiler divergence"; the variable was the C, as always
