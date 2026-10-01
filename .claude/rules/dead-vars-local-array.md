---
name: dead-vars-local-array
paths: [".claude/rules/dead-vars-local-array.md"]
description: "FORBIDDEN: unused local arrays and (void)&scalar frame coercion. Narrow carve-outs (written-never-read array; OVERSIZED-LOCALS frame-proven tail; Q35 trailing sibling array) each need exhaustion, FAKE annotation, dual review."
metadata:
  type: reference
  status: forbidden
---

# FORBIDDEN — frame coercion (array AND scalar variants), with three narrow carve-outs

## The ban

A function carrying either form is not eligible for COMPLETED-C:

- **Array form:** `T name[N];` (2 ≤ N ≤ 64) declared in a function body and never referenced.
  Detector: `engine/volatile_cheats.find_unused_local_arrays` (skips struct members and referenced
  arrays).
- **Scalar address-coercion form:** `(void)&<name>;` as a body statement — no legitimate C reason.
  Detector: `engine/volatile_cheats.find_addr_coerced_locals`.

Both feed `engine/inlineasm.func_cheat_asm_count`; `queue done` refuses affected functions.
Rationale: frame size alone does not tell us what the original locals were; an anonymous
`pad`/`buf`/`spill` declaration is a guess that coerces the frame, not reconstruction. Locals must
have clear semantic meaning recovered from the asm and callers.

## Carve-out 1 — WRITTEN-never-read local array (owner ruling 2026-07-01)

A local array that is **WRITTEN but never read** is sanctioned as a last-resort matching construct
(SOTN: `u8 sp70[4]` written 4×/read 0× in `src/dra/62DEC.c`; `s16 z[5]`). Prerequisites
(cheat-reviewer FAILs if missing):

1. The TARGET function's bytes contain the corresponding dead stores / frame (oracle-enforced — the
   construct cannot fabricate a match where the original had none).
2. Documented lever-exhaustion (ledger).
3. `/* FAKE: ... */` annotation on the declaration or writes.
4. Layer-1 + layer-2 review.

## Carve-out 2 — OVERSIZED-LOCALS unwritten tail (owner ruling 2026-07-13)

A stack-locals object with an **unwritten tail** (e.g. func_80037540's `s32 sp[8]` with only
`sp[0..5]` stored and the buffer passed live to a callee) — or, as the fallback when no live object
exists to extend, a dead pad local — is sanctioned as a last-resort matching construct **when the
target bytes PROVE the original declared it**. Prerequisites — the cheat-reviewer / Judge FAILs if
ANY is missing:

1. **Frame-math proof from the target bytes alone** that the original declared a locals object
   strictly larger than the bytes it writes: frame size − callee-save bytes − outgoing-args bytes
   exceeds the bytes actually stored in the locals region, so the fully-written form yields a
   strictly SMALLER frame. Where the equation does NOT force the slack, declaring it remains
   FORBIDDEN.
2. **Prefer extending a live object**; a separate dead pad local only when the function has no live
   locals object.
3. **Range annotation on the declaration** (SOTN `// n.b.!` convention): ALIGN8 makes the size
   recoverable only as a range; the annotation states the frame derivation and the range and names
   this carve-out.
4. **Documented lever-exhaustion** in the ledger.
5. **Layer-1 + layer-2 review** (manual path) or the Grinder's default-FAIL Judge.

Still FORBIDDEN under this carve-out: volatile-qualified dead pads (`volatile char pad;` —
volatile coercion), `(void)&local`, `__asm__`-based frame tricks (e.g.
`__asm__ volatile("":"=m"(dummy_pad))` must be replaced, not grandfathered), and any pad whose size
the frame equation does not force. Fully-dead pads still trip `find_unused_local_arrays`; admit one
only through a prerequisite-aware engine allowlist row, never by weakening the detector.

## Carve-out 3 — unwritten pads under the phantom-frame-slot family

An UNWRITTEN `volatile` pad array is admitted only under the phantom-frame-slot volatile pad local
family in [[no-new-park-categories]] (leading `volatile u32 pad[N]; // !FAKE`, per-function
`_SANCTIONED_UNWRITTEN_PADS` row; owner rulings 2026-08-17/18) and its **Q35 extension** (owner
ruling 2026-09-29 — a trailing unused local array with sibling evidence): frame forensics prove the
bytes untouched; ≥2 COMPLETED-C same-file siblings declare a real array of the same type/count at the
same offset (one as a separate local immediately after the object, which is the array copied); the
declaration is that sibling's exact type, count and identifier with `volatile` added (byte-neutrality
banked), never a `pad`/`dummy` name; FAKE annotation; honest producers measured inert first; a
per-function `_SANCTIONED_UNWRITTEN_PADS` row; layer-2. That `volatile` is the allowlist marker,
proven byte-neutral — not the volatile coercion carve-out 2 forbids. Every other unwritten array
stays FORBIDDEN. Record: docs/grind/decisions.md 2026-09-29 OWNER RULING — a trailing unused local
array with sibling evidence.

## What to do instead

Identify what the locals semantically hold (matrices, GPU packets, OT buffers) and declare them with
meaningful names and real uses; try [[phantom-slot-frame-lever]]'s honest producers. If no carve-out
applies, keep grinding; canonical-asm only via the gate ([[canonical-asm-authorization-recipe]]).

Related: [[register-alloc-pure-c]] · [[phantom-slot-frame-lever]] · [[named-local-fake-exception]] ·
[[inline-asm-policy]]
