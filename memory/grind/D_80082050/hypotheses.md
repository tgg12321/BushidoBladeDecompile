# Hypothesis ledger — D_80082050

## s1 (2026-07-18, recon)
- H1 **CONFIRMED**: the residual distance-1 is not a codegen diff but a scorer artifact — the
  `%lo(D_80082320)` addend is section-local (static symbol) and shifts when the sandbox's file-wide
  cheat strip shrinks earlier functions. Probe: engine-normalized diff isolated the single diff to
  insn 34's addend (0x2294 ref vs 0x18F4 sandbox), both equal to D_80082050+0x2D0 in their own .o;
  positional diff vs the original EXE shows 0/180 real differences.
- H2 **CONFIRMED**: external linkage for D_80082320 is byte-neutral and removes the artifact.
  Probe: dropped `static`, full build SHA1 == oracle (MATCH), sandbox --disable all = 0.
