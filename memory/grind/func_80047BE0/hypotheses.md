# Hypothesis ledger — func_80047BE0

## s1 (recon, 2026-09-08) — chassis -mel -msoft-float, no FAKE constructs
- H1 CONFIRMED: declaration fix — D_800EF0D8 / D_800EF168 are SVECTOR[17] arrays, not s16 scalars. With the SVECTOR-typed vertex fill and RotTransPers3(&v[0], &v[1], &v[2], ...) the body measures 6 (v1) and, after H2, 0.
- H2 CONFIRMED: the score-6 residual was pure source statement order (independent register inits are emitted in RTL = source order): `i = 0` must precede `src = D_800EF59C`, and `j = 0` must precede `z = -0x2EE0` in the first inner loop's setup. Measured 0 (v2 = candidate.c).
- Floor: 63 (pre-migration pin, cheat body) -> 6 (v1 natural) -> 0 (v2). candidate.c is the score-0 body; it is installed in src/sound.c.
