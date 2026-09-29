s32 func_80062FEC(void) {
    s32 i;
    s32 bit;
    s32 *src;

    D_800F10F0 = 1;
    for (i = 0; i < 12; i++) {
        bit = 1 << i;
        if (!(D_800A3448 & bit)) {
            D_800A3448 |= bit;
            break;
        }
    }
    src = (s32 *)D_800A347C;
    D_800F0E38[i].unk0 = src[0];
    D_800F0E38[i].unk4 = src[1];
    D_800F0E38[i].unk8 = src[2];
    D_800F0BEC[i] = 0;
    return 1;
}
