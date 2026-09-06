# Hypothesis ledger — func_80072E10

## s1 (recon, 2026-09-06)
- H1 CONFIRMED — Writing the eight per-block vertex stores as struct-field stores through a TU-local POLY_G4-shaped typedef (`p->x0 = 0x50;` ...) instead of `*(s16 *)((s32)p + 8) = 0x50;` lets sched.c hoist `lh D_800A3580` above the stores (true_dependence waives the conflict for a MEM_IN_STRUCT_P varying-address store vs. a fixed-address non-struct load, sched.c:834-839). Measured: sandbox --disable all 0 (control cast form 44).
- H0 KILLED (instance) — Cast-scalar stores `*(s16 *)((s32)p + off) = k` with no empty-if dead-reads: 44 on this chassis; the .sched dump shows the lh insn true-dependent on all eight stores per block. kill_scope instance, measured_on HEAD main 1a876b5f chassis, no FAKE constructs.
