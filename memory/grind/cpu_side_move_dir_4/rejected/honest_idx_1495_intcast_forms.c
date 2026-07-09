/* s61 P2/P3/P4: three "intermediate integer cast" variants of the honest
 * respelling idx_1495 = idx_1494 + 1. All three produce IDENTICAL sandbox
 * masked=15 build_insns=160 = the s8-probe1 baseline. All three carry the
 * REG_EQUIV note on p78.
 *
 *   P2:  idx_1495 = 1 + idx_1494;                       // marionation-form
 *   P3:  idx_1495 = (u8 *)((s32)&D_800A1494 + 1);       // int-cast &D
 *   P4:  idx_1495 = (u8 *)((s32)idx_1494 + 1);          // int-cast var
 *
 * Mechanism (extending s60 REG_EQUIV finding): the tree-level constant
 * recognition happens whenever the ADDR_EXPR reaches expand_expr; the
 * intermediate integer-CAST_EXPR does NOT block set_unique_reg_note from
 * attaching REG_EQUIV. And CSE folds any var-mediated form
 * (s32)idx_1494 -> (s32)&D_800A1494 before the note decision.
 *
 * Conclusion: every C-level respelling of idx_1495 that produces a
 * compile-time-constant address attaches REG_EQUIV, triggers the p78
 * alloc-web rotation, and regresses ~+13. No natural, non-cheat respelling
 * escapes REG_EQUIV; the reversible-lever hypothesis on this axis is CLOSED.
 */
  idx_1495 = 1 + idx_1494;                    /* P2 */
  idx_1495 = (u8 *)((s32)&D_800A1494 + 1);    /* P3 */
  idx_1495 = (u8 *)((s32)idx_1494 + 1);       /* P4 */
