/* REJECTED s2 (frontier-3 kill): init order d0; p0; d1 — measured sandbox 5
 * (vs 2 for p0;d0;d1 and 0 for the final p0;d1;d0 + swapped-overwrite form).
 *
 *     hi = arg0;
 *     s.d0 = hi;
 *     s.p0 = &D_8009B6FC;
 *     s.d1 = hi;
 *     s.d0 = ((s16)arg0) % 10;
 *     hi = ((s16)arg0) / 10;
 *     s.d1 = hi % 10;
 *
 * Mechanism (verified with BB2_SCHED_DEBUG trace): chain [84(d0i), 89(p0),
 * 81(d1i)] is flow-alive, but reverse-scheduler picks by descending LUID
 * among the pri-1 leaf-store ties -> picks 81, 89(+launch 87), 84(+launch
 * 78) -> emit [move, sh48, lui/addiu, sw18, sh4A]: group split, both pair
 * order and group order wrong. Predicted ~6-8, measured 5. KILLED.
 */
