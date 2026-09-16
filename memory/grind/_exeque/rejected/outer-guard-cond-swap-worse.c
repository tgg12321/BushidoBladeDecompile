/* _exeque — s6 (structural) rejected form.
 * Swapping the && operand order in the OUTER post-loop guard from
 * `if (D_8009BF78 == D_8009BF7C && !(*D_8009BF54 & 0x01000000))` to
 * `if (!(*D_8009BF54 & 0x01000000) && D_8009BF78 == D_8009BF7C)` on the
 * s4/s5 floor-2/187 do-while(0)-wrapped chassis (candidate.c unchanged
 * otherwise).
 *
 * Measured: sandbox _exeque --disable all score 2 -> 14 (WORSE, much
 * worse than the inner-guard swap). Testing D_8009BF54 first instead of
 * the queue-empty comparison first changes multiple downstream register
 * choices, not just the jalr delay slot. Reverted; not adopted.
 * kill_scope: instance — this exact operand-order swap, on the s4 chassis,
 * both do-while(0) FAKE wraps present, no other construct changed.
 */
