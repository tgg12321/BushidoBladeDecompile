/* s90 REJECTED probe P2: idx_1495 = (u8*)tbl_125c + 0x239 (tbl-routed with numeric constant, no &D_800A1494 reference)
   Result: masked=15, build_insns=160 (byte-neutral vs h5 baseline masked=2, +13 masked).
   Matches s8-probe1 honest form (idx_1495=idx_1494+1, masked=15) exactly - identical
   file-level s-reg regression signature despite different symbolic base.
   Confirms: the s-reg coupling is caused by the ABSENCE of the &D_800A1494 symbol
   reference in idx_1495's initializer, not by whether the base is idx_1494 vs distinct.
   Only the SEMANTIC-LIE cross-symbol form (which references BOTH tbl_125c AND &D_800A1494 AND D_800A125C)
   preserves the file-level RA at masked=2. */
  idx_1495 = (u8 *)tbl_125c + 0x239;
