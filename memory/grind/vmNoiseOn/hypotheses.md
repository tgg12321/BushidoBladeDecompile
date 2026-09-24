# vmNoiseOn — hypotheses (manual session 2026-09-24)

- [CONFIRMED] voice index bound before SpuSetNoiseClock -> $s0 (49 -> 15). Named intermediate `idx`.
- [CONFIRMED] record-array declaration is required for the voice-reset loop (per-word spellings
  all hoist `la sym`); merge is byte-neutral for every C sibling (129/129).
- [KILLED] per-word array-index spellings `(&sym)[i * 54]`, `*(&sym + i * 54)`, split
  load/store: 15 each, loop still hoists the symbol.
- [KILLED] pan stages without a shared multi-write local: direct reads, three once-written
  locals — both differ (birthing boost puts the loads late).
- [RESOLVED] `temp` reuse (SOTN verbatim) is the only matching pan spelling found; the owner
  admitted it for vmNoiseOn only (Ruling 8, 2026-09-24, commit 070ee0196).
- [CONFIRMED on the final body] idx still required: vc at every use and idx at every use both
  differ (evidence.md).

COMPLETED-C 2026-09-24: commit a64e6340c (layer-2: round 1 FAIL on symbol-row retirement / stale
idx receipts / message overclaim, fixed; round 2 PASS). Sandbox 0 for vmNoiseOn (305/305) and
all 7 converted siblings; oracle SHA1 62efab4f.
