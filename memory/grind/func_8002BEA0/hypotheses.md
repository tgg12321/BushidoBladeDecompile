# Hypothesis ledger — func_8002BEA0

## s1 (2026-08-20, recon)

- H1: "Widening the LZC island clobber list to $12-$15 (the unspent
  2026-07-28 grant) flips the /100 reload scratch from $13 to $24
  (mfhi $t8), closing 2 of the 4 residual points."
  Mechanism: reload1.c order_regs_for_reload bad_spill_regs.
  Probe: applied the sibling func_8002BC68 clobber spelling; sandbox.
  Result: 4 → 2. VERDICT: CONFIRMED.

- H2: "The remaining 2 points (delay-slot li $v1,0x44C vs target's
  lui $a0,magic) are an RTL-generation/LUID ordering problem"
  (inherited WIP diagnosis).
  Probe: read .sched/.sched2 dumps — both streams ALREADY carry target's
  order (lui 186, ori 187, li 99).
  Result: ordering is correct pre-reorg. VERDICT: KILLED (mis-attribution;
  the divergence is in reorg's delay-slot fill, not RTL gen or sched).

- H3: "reorg rejects the lui fill because $a0 is in the opposite-thread
  needs, and the hand-expanded /16 else arm is what puts it there;
  respelling as plain /16 (sibling BC68's accepted form) fixes the
  liveness shape."
  Mechanism: reorg.c fill_slots_from_thread + mark_target_live_regs
  (BB2_DBR_DEBUG proof: trial=186 setsopp=1 LOSE, oppregs=0x20000f90).
  Probe: else arm -> `var_a0 = (0x44C - (s32)var_t0) / 16;`; sandbox.
  Result: 2 → 0. VERDICT: CONFIRMED.

Frontier: EMPTY — sandbox --disable all == 0 with edits in src.
Remaining work is integration: layer-1/Judge acceptance + the driver's
inline_asm_canonical.txt listing under the pre-approved LZC wording
(docs/grind/decisions.md:3906) and the 2026-07-28 clobber grant
(docs/grind/decisions.md:1845).
