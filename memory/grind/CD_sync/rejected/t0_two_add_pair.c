/* s65 rejected: t0 = t0+t0; t0 = t0+t0; instead of t0 *= 4.
   Result: masked=15 (+13 vs h5 baseline 2). Two chained PLUS creates a
   fresh single-set intermediate on each add; the mult-expander alg_shift
   path (which produced the h5 alignment via p106 fresh single-set SLL)
   is replaced by two PLUS insns with different LAUNCH signature; the
   whole allocation web shifts out of h5 basin. */
    t0 = t0 + t0;
    t0 = t0 + t0;
    t0 = (s32)((u8 *)tbl_125c + t0);
