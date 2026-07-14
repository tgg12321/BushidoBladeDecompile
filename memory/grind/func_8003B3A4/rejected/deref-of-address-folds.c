/* REJECTED s2e (2026-07-14): deref-of-address spelling.
   sandbox --disable all = 6 (55/56) — identical to the direct conditional
   write. GCC folds *(&SYM) to the SYMBOL_REF MEM immediately in
   expand_expr, so the store still goes through the assembler sb macro
   ($at) after the diamond; no address pseudo ever exists. Closes the last
   trivially-conceivable non-pointer-variable spelling.
   Artifact: tmp/grind/func_8003B3A4/s2/variantF_deref_of_address.txt */
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    if (a1 != 0) {
        *(&D_8010277D) = 0xE;
    } else {
        *(&D_8010277D) = 0x1D;
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
