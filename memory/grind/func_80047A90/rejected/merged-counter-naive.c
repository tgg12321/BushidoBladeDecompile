/* REJECTED session 1: single counter `i` for both loops (naive) — sandbox 17.
 * Also tried with pt3 = pa1 + 0x11: identical 17.
 * The merged counter's ref count (~11-12) gives it top allocno priority so it is
 * allocated FIRST and takes a2; target needs the counter allocated LAST (t0).
 * The merge idea itself is still live (see hypotheses.md F2) — the conflict argument
 * (counter must conflict with both loops' a2/a3 holders to be forced into t0) needs
 * a merged counter — but only with refs shaved to <=7 so floor_log2 drops to 2.
 * Do not re-run THIS spelling. Diff vs candidate.c: s/k/i/ throughout, no `s32 k;`. */
