/* REJECTED s2: structural-axis direct sweep on csmd4 (h5 base)
 *
 * Confirms the marionation twin's s2/s3 finding (evidence.md line 124:
 * "the hand-STRUCTURAL axis for the pair is closed") DIRECTLY on csmd4.
 * Prior csmd4 measurements had only *=4-late respellings and value-staging
 * variants; the base structural axes (decl order, type narrowing, interior
 * statement re-association within arg5-after-t0 constraint) were assumed
 * closed by twin transfer. Now measured on csmd4 itself.
 *
 * Sweep (tmp/grind/cpu_side_move_dir_4/s2/struct_sweep.py, sweep.log):
 *   A_decl_reverse   (t0 decl before arg5)                     masked 2  INERT
 *   B_u32_narrow     (u32 arg5, u32 t0, u32 casts)             masked 2  INERT
 *   C_v0shl_early    (v0<<=2 hoisted before pp/t0 chain)       masked 2  INERT
 *   D_interleave     (v0<<=2 interleaved between t0 stmts)     masked 2  INERT
 *   E_arg5_first     (v0=idx_1494[1] BEFORE t0=idx_1494[0])    masked 7  REGRESS
 *
 * Verdict:
 *  - Decl-order and s32<->u32 type narrowing are qty-INERT on csmd4 too
 *    (pseudo birth follows RTL first-use, not source decl order; the u32
 *    change doesn't add zero-extend RTL that would grow the qty). KILLED.
 *  - Statement re-association WITHIN the constraint (arg5 chain follows
 *    t0 chain at head, v0<<=2 placement varies) is masked-invariant. The
 *    scheduler places the resulting insns identically. KILLED.
 *  - Head-load ordering (v0 = idx_1494[1] before t0 = idx_1494[0])
 *    regresses to 7 - the t0-first head load is load-bearing for the
 *    masked-2 alignment. KILLED as an improvement axis.
 *
 * Implication for the ledger: the hand-structural modality is fully
 * exhausted on csmd4's h5 base. Progress on the residual v1/a0 exchange
 * requires the non-structural levers already on the frontier (differential
 * do-while(0) wrap re-weighting the arg5 qty refs via NOTE_INSN_LOOP_BEG;
 * fmt-la-late already killed at 2 INERT per rejected/fmt_la_late_local.c;
 * multi-set v0 value-staging already killed at 12 per
 * rejected/arg5_split_multiset_v0.c; ip statement-form base killed at 7
 * per rejected/ip_base_statement_form.c).
 */

/* All 5 variant bodies live in tmp/grind/cpu_side_move_dir_4/s2/struct_sweep.py.
 * Reproduce: `python tmp/grind/cpu_side_move_dir_4/s2/struct_sweep.py`.
 */
