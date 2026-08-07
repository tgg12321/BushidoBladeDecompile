/* s9 REJECTED: sibling transplant of marionation_Exec inline-block form.
 * form: `s32 arg4; arg4 = tbl_125c[idx_1494[0]];
 *        debug_printf(fmt, D_800F19C0, D_800A11DC[D_800A11D5], arg4,
 *                     tbl_125c[idx_1494[1]]);`
 *   (arg4 named via array-index syntax; arg5 fully inline at call).
 * score: masked 7 (baseline h5 = 2). +5 regression.
 * mechanism: matches ip_base_statement_form.c basin (masked 7). The
 *   array-index `tbl_125c[idx_1494[0]]` on arg4 lowers to the in-place
 *   direct-LSHIFT g3-family shape (p101 multi-set, non-LAUNCH); the
 *   inlined arg5 side re-competes at expand for the a-register seat.
 *   Not the h5 basin.
 * verdict: KILLED. Marionation-hybrid arg4-named+arg5-inline does not
 *   compose onto csmd4's h5 basin.
 */
