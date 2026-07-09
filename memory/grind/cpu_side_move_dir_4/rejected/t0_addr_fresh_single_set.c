/* s65 rejected: fresh s32 t0_addr = (s32)((u8*)tbl_125c + t0);
   Result: masked=9 (+7). Novel intermediate basin, distinct from the
   +13 basin of P1/P2 (multi-set-forcing) and from the g3=6 / g3=7 basins
   already ledger-known. Splitting the tbl+t0 PLUS to a fresh single-set
   dest (t0_addr) gives that insn LAUNCH via reg_n_sets(t0_addr)==1,
   but the tiebreak against insn 121 lands in a novel misalignment. */
    s32 t0_addr;
    t0 *= 4;
    t0_addr = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    /* call reads *(s32*)t0_addr instead of *(s32*)t0 */
