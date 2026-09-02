# Hypothesis ledger — _spu_2pitch

## s1 (2026-09-02, recon)

| # | hypothesis | probe | result | verdict |
|---|---|---|---|---|
| H1 | Plain transliteration (`if (n) do..while`, single-expression ratio update) reaches a small residual | v1/v4/v7 | score 7, `.frame 8`; only sp-offset cascade differs | CONFIRMED (baseline) |
| H2 | Target's extra 8 frame bytes come from the rotated `for`-loop guard orphan pseudo (phantom-slot producer 1), not a dead local | v2 `for (i=0;i<n;i++)` | `.frame 16`, spill at +8, score 2 | CONFIRMED |
| H3 | `u32 steps` gives the target's `slt` | v3 | `sltu`, score 3 | KILLED (instance) |
| H4 | `while (i < n)` reproduces the for-loop guard | v5 | frame 16 but score 3 | KILLED (instance) |
| H5 | The chain-tail seat (`subu v1,v0,v1`) is the product assigned to `ratio`'s pseudo, i.e. the two-statement `ratio *= K; ratio >>= 12;` | v8/v9 | score 0 both | CONFIRMED — MATCH |

Frontier: none — function matched at sandbox 0.
