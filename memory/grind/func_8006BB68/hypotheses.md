# Hypothesis ledger — func_8006BB68

## s1 (2026-09-10, recon)
- H1 CONFIRMED: function-scope temp for the table entry forces global.c coloring (a0) — per-block temp + function-scope
  `p1` restores v0/v1 + a1 seating (21 -> 16).
- H2 CONFIRMED: `for (i = 0; i < 3; i++)` (i=0 emitted after the peeled first block) fixes block-0 hole filling
  (&s early), p1 -> a1, and the i/q s1/s2 order (16 -> 2).
- H3 CONFIRMED: `q++` placed after the call statement gives target loop-bottom order (2 -> 0).
- Result: sandbox distance 0, candidate.c = final form. Frontier empty pending Judge.
