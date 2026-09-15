s32 func_8005C074(s16 vabid, s32 base) {
    s16 order[16];
    s16 count;
    u16 mask;
    u32 min;
    s16 minidx;
    s16 i;
    s16 j;
    s32 addr;

    count = 0;
    mask = 0;
    for (;;) {
        min = 0x7FFFF;
        minidx = -1;
        for (i = 1; i < 16; i++) {
            if (!((mask >> i) & 1) && D_800EFB38[i] != 0 && D_800EFB38[i] < min) {
                min = D_800EFB38[i];
                minidx = i;
            }
        }
        if (minidx == -1) {
            break;
        }
        order[count++] = minidx;
        mask += 1 << minidx;
    }
    for (i = 0; i < count; i++) {
        addr = D_800EFB38[0] + D_800EFC38[0][3];
    }
    for (i = 0; i < count; i++) {
        if (D_800EFB38[order[i]] == addr) {
            addr += D_800EFC38[order[i]][3];
        } else {
            for (j = i; j < count; j++) {
                addr = func_8005BF78(base, order[j], addr, D_800EFB38[order[j]]);
            }
            return 0;
        }
    }
    return 0;
}
