/* s9 REJECTED: both arg4 and arg5 named via array-index syntax,
 * evaluated in reverse-order (arg5 = ...; arg4 = ...).
 * form: `s32 arg4, arg5;
 *        arg5 = tbl_125c[idx_1494[1]];
 *        arg4 = tbl_125c[idx_1494[0]];
 *        debug_printf(fmt, ..., arg4, arg5);`
 * score: masked 8 (baseline h5 = 2). +6 regression.
 * mechanism: both derefs via tbl_125c[idx[k]] lower to direct-LSHIFT
 *   in-place (g3-family) on their respective pseudos; the reverse
 *   evaluation order changes LUID assignment but not the birthing/qty
 *   arithmetic (measured differential vs P1's arg4-only-named=7 is +1,
 *   the extra arg5 pseudo).
 * verdict: KILLED. Rederive via array-index syntax + reverse-order
 *   naming lands in a novel intermediate basin (masked 8) that is
 *   between the ip-statement basin (7) and the fully-inline basin (14),
 *   but is still worse than h5 (2).
 */
