# Hypothesis ledger — func_8003F6D8

## s1 (2026-09-08, recon)
- H1 CONFIRMED: two nested counted loops; inner loop = strength-reduced givs (pointer giv for objs[], offset givs for pairs/quads); plain struct-array C reproduces the inner loop byte-exact (Form A, score 24 total, inner 0 residual).
- H2 KILLED (instance): pointer giv `&rec[i].inner` — loop.c folds arg0 into the giv add_val, producing a pointer in $s8; target needs an integer offset giv without arg0 folded. score 24. rejected/pointer-giv-rec-array-index-score24.c.
- H3 KILLED (instance): user biv `off = 8` hoisted before the loop with `off += 0xD0` in the for-increment — init before the entry test and increment after i++; target has init after the test and increment before i++. score 12. rejected/user-biv-off-hoisted-init-score12.c.
- H4 CONFIRMED: `s32 off = i * 0xD0 + 8;` computed inside the loop body → loop.c DEST_REG giv (config.loop:1876,1881) with init at loop start and increment at the biv update → score 0. candidate.c; in place in src/config.c.
