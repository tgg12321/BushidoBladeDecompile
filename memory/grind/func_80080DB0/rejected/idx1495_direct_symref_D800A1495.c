/* s90 REJECTED probe P1: idx_1495 = &D_800A1495 (honest direct-symref, symbolically distinct from idx_1494)
   Result: masked=16, build_insns=161 (+1 insn vs h5 baseline masked=2).
   Mechanism: separate symbol reference for D_800A1495 forces its own lui/addiu pair;
   the h5 candidate's cross-symbol tbl-routed form derives idx_1495 as tbl_125c + K
   which folds cleanly (byte-neutral) AND references &D_800A1494 in a way that ties
   into a file-level cse fabric preserving the block=3 window RA.
   Frontier item #3 axis (symbolically-distinct initializers not naming idx_1494) closed:
   both &D_800A1495 and tbl+0x239 forms regress; only the current cross-symbol semantic-lie
   form achieves masked=2. See rejected/idx1495_tbl_numeric_offset.c for the twin probe. */
  idx_1495 = &D_800A1495;
