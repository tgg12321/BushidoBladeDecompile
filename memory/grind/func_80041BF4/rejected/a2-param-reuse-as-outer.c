/* REJECTED s1 (2026-08-19): reuse the a2 PARAM variable as the outer
 * counter (variable-reuse family). Score 34 (vs 29 base).
 * WHY DEAD: merging the a2 home (2 refs / short) with outer (5 refs)
 * yields one pseudo with refs 7 over live length ~86 (two disjoint
 * phases) -> global.c priority floor_log2(7)*7/86*10000 ~= 1628,
 * BELOW b (2333) and yoff (2325), so it allocates LAST and lands in s4,
 * also perturbing the prologue save order (s4 saved before s3/s2).
 * Target's s2-sharing between the a2 home and outer comes from two
 * SEPARATE pseudos non-conflictingly sharing s2 after the do-while
 * loop-notes priority lift (see candidate.c) — not from source-level
 * variable reuse.
 *   ...
 *   a2 = 0;
 *   loop_outer:
 *   if (a2 == 0) { xoff = -0x140; yoff = 0xF0; } else { xoff = 0x80; yoff = 0; }
 *   ...
 *   a2++;
 *   if (a2 < 2) goto loop_outer;
 */
