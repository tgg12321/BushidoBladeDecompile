s32 func_80089A48(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
    s32 mode = arg0;
    u32 mask = arg1;
    volatile u16 *base;
    u32 t2;

    if (D_800A2CD4 & 1) {
        t2 = ((*(&D_800F7298[arg3]) & 0xFF) << 16) | *(&D_800F7298[arg2]);
    } else {
        base = (volatile u16 *)D_800A2CDC;
        t2 = ((base[arg3] & 0xFF) << 16) | base[arg2];
    }
    switch (mode) {
    case 1:
        if (D_800A2CD4 & 1) {
            *(&D_800F7298[arg2]) |= mask;
            *(&D_800F7298[arg3]) |= (mask >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] |= mask;
            ((volatile u16 *)D_800A2CDC)[arg3] |= (mask >> 16) & 0xFF;
        }
        t2 |= mask & 0xFFFFFF;
        break;
    case 0:
        if (D_800A2CD4 & 1) {
            *(&D_800F7298[arg2]) &= ~mask;
            *(&D_800F7298[arg3]) &= ~((mask >> 16) & 0xFF);
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] &= ~mask;
            ((volatile u16 *)D_800A2CDC)[arg3] &= ~((mask >> 16) & 0xFF);
        }
        t2 &= ~(mask & 0xFFFFFF);
        break;
    case 8:
        if (D_800A2CD4 & 1) {
            *(&D_800F7298[arg2]) = mask;
            *(&D_800F7298[arg3]) = (mask >> 16) & 0xFF;
            D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
        } else {
            ((volatile u16 *)D_800A2CDC)[arg2] = mask;
            ((volatile u16 *)D_800A2CDC)[arg3] = (mask >> 16) & 0xFF;
        }
        t2 = mask & 0xFFFFFF;
        break;
    }
    return t2 & 0xFFFFFF;
}
