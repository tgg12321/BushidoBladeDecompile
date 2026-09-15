s32 func_8005C074(s16 vabid, s32 base) {
    s16 order[16];
    s16 count;
    u16 mask;
    u32 min;
    s16 minidx;
    s16 i;
    s16 j;
    s16 k;
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
    addr = D_800EFB38[0] + D_800EFC38[0][3];
    for (j = 0; j < count; j++) {
    }
    for (j = 0; j < count; j++) {
        if (D_800EFB38[order[j]] == addr) {
            addr += D_800EFC38[order[j]][3];
        } else {
            for (k = j; k < count; k++) {
                addr = func_8005BF78(base, order[k], addr, D_800EFB38[order[k]]);
            }
            return 0;
        }
    }
    return 0;
}
