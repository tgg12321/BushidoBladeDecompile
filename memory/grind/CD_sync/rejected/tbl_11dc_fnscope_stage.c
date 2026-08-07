/* rejected: fn-scope tbl_11dc pointer stage (mirrors saEft01Init decl pattern).
 * s27 REDERIVE: pre-stage tbl_11dc = D_800A11DC at fn-scope, use tbl_11dc[D_800A11D5]
 * in the debug_printf call replacing inline D_800A11DC[D_800A11D5].
 * Result: masked=14, build_insns=163 (+3 target_insns). REGRESSION vs h5 masked=2.
 * Mechanism: fn-scope tbl_11dc adds competing pointer allocno (2 refs: init + use);
 * GCC materializes D_800A11DC as lui/addiu at prologue instead of the assembler-macro
 * pair the target uses inline, adding 3 insns and disturbing s-reg web.
 */
