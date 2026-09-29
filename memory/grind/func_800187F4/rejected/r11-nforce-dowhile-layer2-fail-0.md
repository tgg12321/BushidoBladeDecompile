# rejected: r11-nforce-dowhile-layer2-fail-0 (2026-09-28, second fresh layer-2 FAIL)

Body: the v2 landing (c6; ledger 05f9b8638), sandbox 0 (644/644), oracle-green when spliced. PASSED:
islands, both rows, the 85 region hashes, the sq1/depth FAKE dead stores (mechanism regclass.c:1763-1764 +
cse.c:840-857 confirmed), the Q28 copy clause, lz[6] annotation + census, naming/header.

FAILED (Ruling 11 (D)(3)) for `nforce`: the one-variable-per-value spelling (nforce_add / nforce_sub) with
six single-level FAKE `do { } while (0);` wraps (around the loads of vx, vy, vz, nforce_add, bits and bits2)
is byte-identical to the landing body (nforce-split-six-dowhile-wraps-closes-0.template.c). Mechanism:
flow.c adds loop depth to reg_n_refs (:2081 etc.), so the wraps re-order global.c priorities (nforce_add
above idx) without a reuse. The proof's premise "a single count's refs are 7 in every per-value spelling"
ignored loop-depth weighting, and §5 swept only dead stores / self-assigns / chain-extenders.

Also flagged: idx / temp / nbits / nbits2 (D)(3) unproven against multi-wrap do-while(0) combinations
(500 single-wrap variants by the reviewer: none closed; bests idx 62, nforce 21, temp 42, nbits 2);
"at every anchor" overstated; dead-store comments say "insns" for differing lines.
Reviewer sweep scripts/results: v2_review/.
