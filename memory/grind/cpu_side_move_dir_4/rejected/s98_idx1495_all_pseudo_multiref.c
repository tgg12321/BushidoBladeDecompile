/* s98 rederive KILLED - masked=15 (+13 from h5) - probe2
 * Angle: replace both SYMBOL_REF sides of the cross-symbol subtraction with
 *        pseudo (u8*)tbl_125c so `(s32)&D_800A1494 - (s32)tbl_125c` uses the
 *        LOCAL for the RHS. Predicted lift on p79 (extra local reads) - actual:
 *        collapses to same +13 basin. Confirms s97/s96 finding: the RTL insns
 *        34+38 emitted by expr.c::expand_expr for the cross-symbol SYMBOL_REF-
 *        SYMBOL_REF form are what h5 requires; pseudo-based tbl_125c refs do
 *        NOT substitute at expand_expr time and are unfoldable in cse.c.
 * Combined with probe1: cross-symbol arithmetic is the h5 driver; only the
 * exact spelling (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1494 - (s32)D_800A125C) + 1)
 * (local base + symbol_ref-symbol_ref delta) preserves masked=2.
 */
  idx_1495 = (u8 *)((u8 *)tbl_125c + (s32)&D_800A1494 - (s32)tbl_125c + 1);
