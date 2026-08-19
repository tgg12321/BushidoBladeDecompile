/* REJECTED s13 (driver session 8, 2026-08-10): do-while(0) forms of the X
 * clamp on the 5-chassis. Two variants measured:
 *   (a) plain wrap around the verbatim nested-if clamp: 5 @ 50 — INERT
 *       (identical to baseline; the loop-note/LABEL_OUTSIDE_LOOP_P mechanism
 *       has no reorg-side diff to act on here).
 *   (b) break-form below: 9 @ 50 — KILLED (break-target label restructures
 *       the arms like the flat else-if P3 form).
 * Neither materializes the join. Do not re-propose do-while(0) on this
 * function — its sanctioned mechanism (reorg.c interaction) is orthogonal to
 * the join-temp residual. Only the X region shown. */
    do {
        if (arg0 < 0) {
            arg0 = 0;
            break;
        }
        if ((D_8009BE78 - 1) < arg0) {
            arg0 = D_8009BE78 - 1;
        }
    } while (0);
