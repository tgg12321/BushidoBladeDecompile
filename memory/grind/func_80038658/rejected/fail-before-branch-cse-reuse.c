/* REJECTED — natural switch but with the fail-code assigned BEFORE the
 * ret==0 branch (`ret = func_800378A8(); fail = 1; if (ret == 0) goto
 * fail_store;`). Measured: sandbox --disable all = 18, 56/55 insns.
 * Mechanism (greg + final asm): cse's extended basic block carries
 * `fail == 1` into the close() path and REUSES fail as the `ret == 1`
 * compare constant (`bne s1,s0` instead of `li v0,1; bne s0,v0`), which
 * extends fail's live range ACROSS close() -> fail forced to callee-save
 * $s0, ret displaced to $s1, extra s1 save/restore + frame 0x20 (target
 * 0x18). The `fail = call(); ret = fail; fail = 1;` anti-dependence staging
 * variant produces the IDENTICAL 18 (cse collapses the staging copy).
 * Fix: assign fail INSIDE the if-arm so the close()-path cse block never
 * sees the constant (candidate.c).
 * Same-score variant also rejected: identical form with
 * D_800A379E = 1;/= 4; inline + return (no fail var / no shared label) = 11,
 * 58/55 — the two `sh; j epilogue` tails never cross-jump-merge because the
 * merged block must sit at the function END (source-level shared label),
 * not at either arm's site.
 */
