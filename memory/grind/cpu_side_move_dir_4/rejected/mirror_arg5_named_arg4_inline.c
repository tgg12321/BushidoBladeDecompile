/* s9 REJECTED: mirror of marionation-hybrid (arg5 named, arg4 inline).
 * form: `s32 arg5; arg5 = tbl_125c[idx_1494[1]];
 *        debug_printf(fmt, D_800F19C0, D_800A11DC[D_800A11D5],
 *                     tbl_125c[idx_1494[0]], arg5);`
 * score: masked 14 (baseline h5 = 2). +12 regression.
 * mechanism: matches the WIP-recorded 'inline-all args (v1/v8/v9): 14'
 *   basin (evidence.md L7). Right-to-left eval evaluates the inlined
 *   arg4 (last written) into an early-birthed pseudo AFTER arg5 is
 *   staged, further disrupting qty birth order.
 * verdict: KILLED. The mirror doesn't yield a novel basin either.
 */
