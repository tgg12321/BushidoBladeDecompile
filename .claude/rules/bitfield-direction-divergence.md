---
name: bitfield-direction-divergence
description: Measured toolchain fact (2026-06-11) — our decompals cc1 allocates the FIRST-declared bitfield at the HIGH bits; cc1psx (PsyQ original) at the LOW bits. SDK bitfield structs must be declared in FLIPPED field order to reproduce the original layout.
paths: ["src/*.c", "include/*.h"]
---

# Bitfield allocation direction DIVERGES between our fork and cc1psx

## The measured fact (probes preserved: tmp/bitfield_probe.sh, tmp/bf_probe_psx.sh)

| Compiler | `struct {u32 addr:24; u32 len:8;}` → `p->addr` | First-declared field |
|---|---|---|
| **decompals cc1** (our frozen build compiler) | `srl $2,$2,8` → addr in HIGH 24 | **HIGH bits** |
| **cc1psx** (PsyQ GCC 2.7.2.SN, the original) | `and $2,$3,0x00FFFFFF` → addr in LOW 24 | **LOW bits** |

This is the FIRST known semantic divergence between the two compilers
(the 0/282 parity sweep never covered bitfields — no src/ file used them
before 2026-06-11).

## What this means for matching

The original BB2 source could use PsyQ SDK bitfield structs VERBATIM
(e.g. LIBGPU.H's `P_TAG { unsigned addr:24; unsigned len:8; ... }` — addr
lands in the LOW 24 under cc1psx, the hardware OT format). To reproduce
the SAME memory layout (and therefore the same bytes) under OUR toolchain,
the field declaration order must be FLIPPED (len first). This flip is a
measured divergence compensation, NOT a codegen lever — annotate it with a
comment pointing at this rule wherever it appears.

Implication for any function whose target codegen looks like bitfield
insert/extract (mask-AND + OR patterns with ≥4 mask references that
combine folds): the original likely accessed an SDK bitfield struct, and
the faithful reconstruction is the flipped-order struct, not mask
arithmetic. Confirmed mechanism: expmed's bitfield insert+extract
expansion materializes mask uses pre-combine, which feeds flow's ref
counts and flips local-alloc allocation priority — mask-and-OR spellings
cannot reproduce this (9 spellings measured byte-identical, ot_Insert
session 3).

## Confirmed case — ot_Insert + ot_Link (gpu.c, 2026-06-11)

Both closed by the `OTag` project type (include/gpu.h, len-first with the
divergence note): `a2->addr = a0->addr; a0->addr = a1;` (ot_Insert),
`a1->addr = a0->addr; a0->addr = (u32)a1;` (ot_Link). sandbox 0 (15/15)
each; 4 register pins retired; SHA1 == oracle. User-sanctioned 2026-06-11
after: SDK-header provenance verified externally, BOTH compiler probes,
and the 9-spelling mask-family closure. (First proposal was REVERTED for
GCC-internals-led justification + file-local typedef — the re-proposal
through the proper evidence path is the model to follow.)

## Per the no-compiler-divergence policy

The toolchain stays frozen — this rule documents a property, it does NOT
license compiler patches. The compensation lives in the C source (flipped
declaration order), which is exactly the "the variable is the C" posture.

## Related

- [[no-compiler-divergence]] — the frozen-toolchain policy this fact lives under.
- [[proven-spelling-class-reconstruction]] — the sanction pathway the OTag
  re-proposal followed.
- [[compiler-flags-canonical]] / [[cc1psx-calibration-only]] — the parity
  record this divergence amends (parity holds for everything measured
  EXCEPT bitfield direction).
