/* s2: *(D_800F7298 + addr) pointer-arithmetic spelling on the HEAD volatile u16[] decl - 45/178,
 * byte-identical to the plain arr[addr] form: c-typeck decays the array to ADDR_EXPR and expr.c
 * expands the PLUS with EXPAND_SUM to (plus reg symbol_ref), the same address the ARRAY_REF
 * *(&array+index) path (expr.c:4624) builds. Not a lever. */
s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        var_t0 = ((*(D_800F7298 + addr2) & 0xFF) << 16) | *(D_800F7298 + addr1);
    } else {
        var_t0 = ((((volatile u16 *)D_800A2CDC)[addr2] & 0xFF) << 16) | ((volatile u16 *)D_800A2CDC)[addr1];
    }
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            *(D_800F7298 + addr1) |= bits;
            *(D_800F7298 + addr2) |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[addr1] |= bits;
            ((volatile u16 *)D_800A2CDC)[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            *(D_800F7298 + addr1) &= ~bits;
            *(D_800F7298 + addr2) &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[addr1] &= ~bits;
            ((volatile u16 *)D_800A2CDC)[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            *(D_800F7298 + addr1) = bits;
            *(D_800F7298 + addr2) = (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[addr1] = bits;
            ((volatile u16 *)D_800A2CDC)[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
