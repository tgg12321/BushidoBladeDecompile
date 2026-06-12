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

## Session 3 (2026-06-11) — mechanism PROOF: mask-and-OR family cannot close

Instrumented diagnosis (tmp/oti_dump.sh, tmp/oti_sweep.py, tmp/oti_chain.py,
tmp/oti_blocks.py; dumps in tmp/oti/):

1. **The lui+ori split is sched1's own** — `sched.c:4891 try_split`
   ("split insns here to get max fine-grain parallelism", reload_completed==0).
   Every compile, original included, sees ADDR as lui+ori at scheduling time.
2. **sched1 schedules BACKWARD**; ADDR's lui+ori pair lands at the block
   head in every spelling (the ori gives ADDR's lui +1 chain depth over
   TAG's single li — priority, not LUID, decides; tie-breaks never engage).
   Our sched1 output order ALREADY equals target's final byte order.
3. **Post-sched1 live lengths** (printed by the sched trace): ADDR=13,
   TAG=9. local-alloc `qty_compare` pri = floor_log2(3)*3/span → TAG 3333 >
   ADDR 2307 → TAG allocates first, takes $a3; ADDR gets $t0. Target wants
   the reverse. Deterministic — same RTL in, same allocation out.
4. **9 spellings → byte-identical cc1 output** (all distance 7): pin-stripped
   baseline, decl swap, no-locals, addr=a1&mask early intermediate, hi/lo
   per-stmt intermediates, operand commute (changes load order = worse),
   a1&=mask param reuse, tag_mask=~mask, mask=~tag_mask. The complement
   derivations are const-propped by cse BEFORE flow counts refs.
5. **What flips it**: ADDR-mask pseudo needs ≥4 flow refs (flow runs
   BEFORE combine — a combine-folded extra use still counts) or span <9.
   No natural mask-and-OR spelling provides either; redundant-use
   respellings ((x&m)&m etc.) are forbidden coercion.
6. **Why the bitfield form hit 0**: expmed's bitfield insert+extract
   expansion BOTH materialize 0xFFFFFF uses (≥4 pre-combine refs on the
   ADDR-mask pseudo); combine folds the redundancy so the emitted bytes
   are identical. The PsyQ P_TAG type produces the target allocation as a
   natural consequence — supporting evidence that it IS the original
   spelling (P_TAG is documented in PsyQ LIBGPU.H).

**Conclusion**: resume direction 3 (header-level OTag re-proposal with user
sign-off) is the ONLY remaining path. Parked NEEDS_USER.

Sibling-cluster note: ot_SetAddr (single-statement, matched, no pins) has
TAG allocated first (lui $v1) — consistent with our toolchain's natural
behavior, confirming the toolchain is not the variable. ot_Link shares
ot_Insert's two-statement shape and the same proof applies (ADDR→$a2,
TAG→$a3 there).

## Sibling

ot_Link (distance 7, 0 rules, same pinned shape) is the next queue
neighbor — whatever closes ot_Insert almost certainly closes it too.
A retry worker session on ot_Link was stopped mid-WIP during the revert;
its partial edit (score 7, no improvement) was discarded.
