/* REJECTED: duplicated (per-arm ED6 variable) form with the table address
   INLINED into the call expression `*(&D_8008E6A4 + v0 + v1)` scores 17 (vs 13
   with a `tbl` variable). Inlining tbl removes the tbl variable's own qty and
   does NOT move ED6 into v1; keep `u8 *tbl` as a named variable on the
   duplicated chassis. */
