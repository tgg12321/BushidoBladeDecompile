s32 func_80089A48(s32 on_off, u32 bits, s32 addr1, s32 addr2)
{
    volatile u16 *rxx;
    u32 var_t0;

    if (D_800A2CD4 & 1) {
        rxx = D_800F7298;
    } else {
        rxx = (volatile u16 *)D_800A2CDC;
    }
    var_t0 = ((rxx[addr2] & 0xFF) << 16) | rxx[addr1];
    switch (on_off) {
    case 1:
        if (D_800A2CD4 & 1) {
            rxx = D_800F7298;
            rxx[addr1] |= bits;
            rxx[addr2] |= (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            rxx = (volatile u16 *)D_800A2CDC;
            rxx[addr1] |= bits;
            rxx[addr2] |= (bits >> 16) & 0xFF;
        }
        var_t0 |= bits & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            rxx = D_800F7298;
            rxx[addr1] &= ~bits;
            rxx[addr2] &= ~((bits >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            rxx = (volatile u16 *)D_800A2CDC;
            rxx[addr1] &= ~bits;
            rxx[addr2] &= ~((bits >> 16) & 0xFF);
        }
        var_t0 &= ~(bits & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            rxx = D_800F7298;
            rxx[addr1] = bits;
            rxx[addr2] = (bits >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((addr1 - 0xC6) >> 1);
        } else {
            rxx = (volatile u16 *)D_800A2CDC;
            rxx[addr1] = bits;
            rxx[addr2] = (bits >> 16) & 0xFF;
        }
        var_t0 = bits & 0xFFFFFF;
        break;
    }
    return var_t0 & 0xFFFFFF;
}
