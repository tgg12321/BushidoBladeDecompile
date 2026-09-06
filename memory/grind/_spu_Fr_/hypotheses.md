# Hypothesis ledger — _spu_Fr_

## s1 (2026-09-06, recon)
- H1 CONFIRMED — declaration fix: D_800A2CE0/E4/E8 are pointers to DMA4
  MADR/BCR/CHCR MMIO registers; declaring the pointee volatile
  (`extern volatile s32 *`) closes the function. Measured sandbox 0 (from 2).
  Mechanism: reorg.c:760 resource_conflicts_p volatil short-circuit blocks the
  forward delay-slot fill past the volatile madr store (see evidence.md).
- H2 KILLED (instance) — hoisting the shift as a statement before
  _spu_FsetDelayR() (`size <<= 16;`): sandbox 5.
- H3 KILLED (instance) — local `bcr = (size<<16)|0x10` before
  _spu_FsetDelayR(): sandbox 5.
- H0 KILLED (instance) — plain `extern s32 *` pointer stores (the banked
  closer splice body): sandbox 2; cc1psx reproduces the same 2 (fork ruled out).
