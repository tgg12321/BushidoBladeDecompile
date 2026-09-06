/* s2: extern volatile u16 D_800F7298[0xCE]; (complete array sized to the .bss extent
 * 0x800F7298..0x800F7434) with plain arr[addr] indexing - 45/178, identical to the incomplete
 * array. Array completeness does not change the ARRAY_REF expansion path. */
s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        var_t0 = ((D_800F7298[addr2] & 0xFF) << 16) | D_800F7298[addr1];
    } else {
        var_t0 = ((((volatile u16 *)D_800A2CDC)[addr2] & 0xFF) << 16) | ((volatile u16 *)D_800A2CDC)[addr1];
    }
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            D_800F7298[addr1] |= bits;
            D_800F7298[addr2] |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[addr1] |= bits;
            ((volatile u16 *)D_800A2CDC)[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            D_800F7298[addr1] &= ~bits;
            D_800F7298[addr2] &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[addr1] &= ~bits;
            ((volatile u16 *)D_800A2CDC)[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            D_800F7298[addr1] = bits;
            D_800F7298[addr2] = (bits >> 16) & 0xFF;
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
