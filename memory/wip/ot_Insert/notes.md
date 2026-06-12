# ot_Insert (gpu.c) — REVERTED match (layer-2 FAIL upheld by user); resume notes

## TL;DR

Commit `0768f0b8` closed ot_Insert via a local `OTag` bitfield struct
(`u32 len:8; u32 addr:24;`) — sandbox 0 (15/15), SHA1 == oracle. The layer-2
retroactive audit FAILed it (GCC-internals-led justification; file-local
typedef; alternative-lever space not exhaustively closed). The orchestrator
CONTESTED two of the four evidence items (see below) but the USER upheld the
strict reviewer line 2026-06-11 and ordered the revert (`6f97a879`).
Function is back to INCOMPLETE at the pinned mask-and-OR HEAD form
(0 rules, register pins as the bridge).

## What is settled fact (do not re-derive)

- **Bitfield direction (probe-verified by the orchestrator, tmp/bitfield_probe.sh):**
  this cc1 allocates the FIRST-declared bitfield in the HIGH bits.
  `{u32 len:8; u32 addr:24}` puts addr in the LOW 24 (`and 0x00FFFFFF`);
  `{u32 addr:24; u32 len:8}` puts addr in the HIGH 24 (`srl 8`). Any
  bitfield spelling of the OT layout under this toolchain MUST declare
  len first.
- The bitfield form produces target bytes (sandbox 0, oracle-verified, then
  reverted). Preserved at `rejected/otag-bitfield.c`.
- The pinned mask-and-OR form's natural codegen (pins stripped) mismatches:
  sched1 hoists OT_ADDR_MASK's lui+ori chain, TAG wins $a3, ADDR lands $t0
  (target wants the reverse). Distance 7.

## Why it was reverted (the audit's surviving objections)

1. The commit's justification was GCC-internals-led (sched1/RTL order as
   the stated reason for the bitfield choice) rather than semantics-led.
2. The typedef was file-local in src/gpu.c, not a project-level type in
   include/ — the form of a one-off lever, not a recovered type.
3. The alternative pure-C lever space was NOT exhaustively measured.
   (The audit's other two evidence items — selective non-application to
   ot_Link and the field-order NB comment — were contested by the
   orchestrator: scope discipline forbids sibling edits, and the field
   order is probe-verified compiler fact. Recorded for fairness; the user
   upheld the FAIL regardless.)

## Resume directions (the reviewer's next_action, in order)

1. Named-intermediate decomposition of the mask-and-OR expressions
   (SOTN-sanctioned LUID family) to change sched1's fill priority without
   a new type.
2. Instrumented cc1 -da dump FIRST to pin the exact sched1 fill decision
   (don't lever blind).
3. If the struct is re-proposed, it must be: a project-header type
   (include/gpu.h or psx.h) with the probe-verified layout note, applied
   consistently to ALL OT-manipulation functions (ot_Link, ot_SetAddr
   family) in the same change, justified semantically (OT tag = addr:24 +
   len:8 packed word), with the GCC narration as mechanism-documentation
   only — and it goes through user sign-off (proven-spelling-class path).
4. If all levers measure negative, re-park with the exhaustion ledger.

## Sibling

ot_Link (distance 7, 0 rules, same pinned shape) is the next queue
neighbor — whatever closes ot_Insert almost certainly closes it too.
A retry worker session on ot_Link was stopped mid-WIP during the revert;
its partial edit (score 7, no improvement) was discarded.
