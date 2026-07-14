/* REJECTED s2d 2026-07-14 — sandbox distance 2 (55/56, --disable all).
   Hypothesis: make the pointer legitimately MULTI-USE (p[0] diamond store +
   p[2] = 0 for D_8010277F) so the construct escapes the store-only-alias
   family entirely (pointer-rmw-like natural cluster access).
   KILLED by target bytes: p[2]=0 emits ONE register-indirect
   `sb $zero,2($v1)` (variantE objdump 0x8c8) where target spends TWO insns
   on the $at macro (`lui $at; sb $zero,%lo(D_8010277F)($at)`,
   asm/funcs/func_8003B3A4.s:34-35). All other 55 insns identical.
   Conclusion: the original source wrote D_8010277F as a DIRECT global store;
   the pointer touched ONLY D_8010277D. There is no multi-use spelling that
   moots the store-only-alias policy question. */
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            p[0] = 0xE;
        } else {
            p[0] = 0x1D;
        }
        p[2] = 0;
    }
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
