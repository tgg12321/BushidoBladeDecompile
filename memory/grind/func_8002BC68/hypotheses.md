# Hypothesis ledger — func_8002BC68

## s1 (2026-07-28, recon)

1. **BEA0-candidate transplant lowers the floor** — CONFIRMED (14 → 9).
   Consolidated GTE block + explicit `v1_m >>= 1` ordering + block-scoped
   division temps.
2. **BEA0's /100 magic-const ordering wall transfers here** — KILLED.
   With the else arm written as native `/ 16`, the beqz delay slot takes
   `lui magic` exactly like target (9 → 2). The wall was an artifact of
   the m2c hand-expanded else arm, not of `(a-b)*c/100` RTL order (here
   the subtrahend is a register, not 0x44C).
3. **Target else arm is GCC's native signed /16 expansion** — CONFIRMED.
   `var_a0 = (arg0 - (s32)var_t0) / 16;` reproduces subu/bgez/sra/addiu/sra
   with target registers; fixed the $2<->$3 rename cluster.
4. **Split numerator changes mfhi reload reg** — KILLED (codegen-neutral,
   identical bytes at floor 2).
5. **mfhi $13-vs-$24 is reload1.c bad_spill_regs exclusion; clobbering
   $13-$15 in the GTE island closes to 0** — CONFIRMED mechanically
   (sandbox 0, 130/130). Policy-blocked: layer-1 cheat-reviewer FAIL
   (pin-by-clobber-spelling); owner ruling requested. No pure-C lever can
   reach bad_spill_regs (C cannot mention hard regs) — the residual is
   decided at canonical-asm-authorization time, not by more C search.

## Frontier (for next session / owner)
- OWNER RULING (filed via s1 outcome): may the canonical GTE-LZCS island's
  clobber list include $13-$15 (bytes-forced: the original TU mentioned
  them at asm level; uniform across both byte-constrained siblings), or is
  extension beyond the template's physical register usage forbidden?
  If GRANTED: apply clobber-form-ruling-pending.c → distance 0 → proceed
  to canonical-asm authorization of the island (3rd+4th LZCS siblings,
  BC68+BEA0 together). If DENIED: function rests at honest floor 2 with
  the island's authorization form unchanged; no grind modality can move
  the residual (search space for these 2 insns is provably empty in C).
