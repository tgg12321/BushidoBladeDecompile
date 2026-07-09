/* s98 rederive KILLED - masked=15 (+13 from h5) - probe1
 * Angle: idx_1495 base spelled as raw SYMBOL_REF (s32)&D_800A125C instead of
 *        the local pseudo (u8*)tbl_125c. All three uses in the initializer
 *        become SYMBOL_REF-based; RTL folds via expmed.c into a constant, and
 *        p79 reference profile collapses to the +13 basin per s90's mechanism.
 */
  idx_1495 = (u8 *)((s32)&D_800A125C + ((s32)&D_800A1494 - (s32)D_800A125C) + 1);
