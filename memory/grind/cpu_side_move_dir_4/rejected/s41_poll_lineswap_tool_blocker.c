/*
 * s41 rejected form: PERM_LINESWAP over the poll-loop callback if-blocks
 * (novel un-permuted axis targeting the s-reg web coupling frontier).
 *
 * Wrapping the two brace-delimited if-blocks (status & 4 callback, status & 2
 * callback) in PERM_LINESWAP failed to parse:
 *   Syntax error in base.c. before: } at approximately line 425, column 23
 *   (after PERM expansion)
 *
 * PERM_LINESWAP requires line-terminated statement-form items; brace-delimited
 * compound `if (...) { ... }` blocks trip the annotator's statement splitter.
 * The intended axis (callback statement reorder as an s-reg-web mutator) is
 * unreachable through the tooled permuter directive set on this AST shape
 * without rewriting the callbacks into single-line ternary or comma-op form,
 * which itself would materially change codegen and thus is not a permutation
 * of the current shape.
 *
 * Recorded to eliminate the axis from the frontier and to document the
 * tool-side constraint for future sessions.
 *
 * Original callback shape (unchanged in HEAD):
 *   if (status & 4) { if (D_800A11B8 != 0) ((...) D_800A11B8)(*idx_1495, ...); }
 *   if (status & 2) { if (D_800A11B4 != 0) ((...) D_800A11B4)(*idx_1494, ...); }
 */
