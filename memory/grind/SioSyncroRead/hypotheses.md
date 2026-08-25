# Hypothesis ledger — SioSyncroRead

## [s1] 2026-08-25 (recon)

- **[s1-H1] Twin-transplant**: the matched SioSyncroWrite's construct set
  (volatile st[] ++/--, FAKE volatile pointer handles, subword table read,
  structured loops) reproduces SioSyncroRead's byte shape. — probe: fresh body
  modeled on twin, measured ladder 41→0 (evidence.md [s1] table). **CONFIRMED.**
- **[s1-H2] AE0 addiu requires volatility**: non-volatile `u16 *p_ae0 =
  &D_800F1AE0;` lets combine fold the address into the load (159i, addiu
  missing); `volatile u16 *p_ae0` preserves it (160i, byte-exact). Both measured
  this session. **CONFIRMED** — and it is the sole reason s1 is a
  ruling-request: AE0 has no volatile_extern_allowlist.txt row.
- **[s1-H3] goto-spelled loops caused the s0/s1 swap + fill_delay debt**:
  restructuring as real do-while (loop notes) fixed seats AND the loop bytes
  (22→11), and `count=0` before the flag[2] test fills the beqz delay slot
  (11→10 with nop removed). **CONFIRMED.**
- **[s1-H4] s3/s4 seat = allocno_compare priority (pkt_len 3refs/56len beats
  r_arg1 3refs/102len)**: ra_solver model confirms; duplicated return compute
  into both cleanup arms (sanctioned ref-lift family, FAKE-annotated) flips it;
  cross-jump re-merges byte-neutrally (9→3 with the only remaining diffs being
  scorer reloc aliasing). **CONFIRMED.**
- **[s1-H5] scorer 3-residual with direct B00/B04 spellings is reloc-addend
  aliasing vs the rule-era reference object, not bytes**: word-level diff shows
  identical instruction words; flag[1]/flag[2] respelling scores 0.
  **CONFIRMED** (keep the flag-relative spelling until integration).

## Frontier (for the next session)

1. **Get the D_800F1AE0 Ruling-4 volatile grant** (the s1 ruling_question).
   If granted (operator adds the allowlist row): re-run sandbox (expect 0),
   write self_vet.md, return candidate-ready. If REFUSED: the addiu needs a
   different producer — next probes: (a) second honest use of p_ae0 in
   cleanup_A (none exists in target semantics — likely dead end), (b) re-read
   the twin's s4 dumps for how AE2's pre-grant sessions failed non-volatile.
2. **Self-vet prep**: constructs = flag pointer (twin-precedent, no FAKE — see
   SioAnsyncRead/SioSyncroWrite committed bodies), volatile st[] (twin
   main.c:3055, no FAKE), FAKE handles p_ae2/p_ae0/p_b04/p_b04a/p_b04b
   (pointer-alias-fake-exception), duplicated return compute
   (duplicated-statement-into-arms). Quote each rule's scope sentence verbatim.
3. **Exhaustion note for the dup-return FAKE**: the honest-lever argument is
   structural (equal refs, livelen ratio fixed by target placement — no
   spelling changes allocno_compare's ordering); if the Judge wants measured
   honest kills, sweep decl-order/init-order variants (predicted flat: they do
   not alter refs or livelen) before re-submitting.
