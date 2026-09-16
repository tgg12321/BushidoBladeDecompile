/* REJECTED (s7, 2026-09-16): replacing the two (&D_8009A821)[i*2] /
 * (&D_8009A820)[i*2] byte-table reads with two PER-TABLE pointer locals
 * measured WORSE than array-index syntax on the same chassis.
 *
 *   u8 *pf = &D_8009A821 + i * 2;
 *   u8 *ps = &D_8009A820 + i * 2;
 *   ...
 *   flags = *pf << 8;
 *   ...
 *   scale = *ps << 8;
 *
 * Measured: sandbox func_80056CB8 --disable all score 81 -> 83 (WORSE),
 * build_insns 197 -> 196. Fewer raw instructions but a worse weighted
 * register-allocation match. Measured on the s6/s7-baseline chassis
 * (before the flags/ang/code variable-reuse merge banked this session).
 * KILLED, instance, re-testable.
 *
 * A further variant (`ps = pf - 1`, treating D_8009A820 and D_8009A821
 * as one array via cross-declaration pointer arithmetic since their
 * addresses are adjacent) was drafted but NEVER MEASURED — it is
 * undefined-behavior C (pointer arithmetic across two independently
 * declared objects is not defined by the C standard even when the
 * linker happens to place them adjacently) and would deepen the exact
 * declaration-pun the grind brief's DATA MODEL section flags for these
 * two symbols. Reverted without running sandbox; not a measured kill,
 * just a note so a future session does not re-derive and try it.
 */
