/* rejected: block-scope tbl_11dc pointer stage (mirrors saEft01Init pointer-decl
 * pattern but at block-scope inside the debug_printf inline block).
 * s27 REDERIVE probe P2: `s32 *tbl_11dc; tbl_11dc = D_800A11DC;` declared inside
 * the { } block, use `tbl_11dc[D_800A11D5]` in the debug_printf call.
 * Result: masked=2 INERT, target_insns=160, build_insns=160. Bytes identical to h5.
 * Mechanism: cse.c copy-propagation folds the single-use pointer alias — RTL for
 * `tbl_11dc[D_800A11D5]` reduces to identical form as `D_800A11DC[D_800A11D5]`,
 * assembler-macro pair emission preserved. Novel data point: block-scope pointer
 * stages of the dispatch symbol are cse-folded (contrast fn-scope tbl_11dc stage
 * which is +12 regression per s27 P1 — fn-scope changes symbol materialization to
 * lui/addiu prologue pair, adding 3 insns; block-scope stays as assembler macro).
 */
