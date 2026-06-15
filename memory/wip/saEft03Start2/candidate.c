s32 saEft03Start2(s32 a0) {
    volatile u16 *p;
    u16 v1;

    if (a0 == 0) goto case_0;
    if (a0 == 1) goto case_1;
    goto exit_load;

case_0:
    D_800A287C = 0;
    p = (volatile u16 *)(D_800A2CDC + 0x1AA);
    v1 = *p;
    v1 &= 0xFF7F;
    goto store_v1;

case_1:
    if (D_800A2880 == a0) goto set_flag;
    if (func_80089EB0(D_800A2884) == 0) goto set_flag;
    p = (volatile u16 *)(D_800A2CDC + 0x1AA);
    v1 = *p;
    D_800A287C = 0;
    v1 &= 0xFF7F;
    goto store_v1;

set_flag:
    p = (volatile u16 *)(D_800A2CDC + 0x1AA);
    v1 = *p;
    D_800A287C = a0;
    v1 |= 0x80;

store_v1:
    *p = v1;

exit_load:
    return D_800A287C;
}
