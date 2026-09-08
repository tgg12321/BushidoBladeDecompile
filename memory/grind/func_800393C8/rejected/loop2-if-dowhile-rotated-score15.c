void func_800393C8(s32 arg0, s32 arg1, s32 *arg2, u16 *arg3) {
    extern s16 D_800A3714;
    extern u8 D_800A3209;
    u8 *slot;
    s32 i;
    s16 idx;
    s32 next;
    s32 state;
    s32 age;
    s32 cnt;
    s32 raw;
    s32 rot;
    s32 mode;

    slot = (u8 *)D_800F68E0;
    i = 0;
    do {
        state = *(s16 *)(slot + 0);
        if (state != -1) {
            age = *(u8 *)(slot + 2);
            cnt = age & 0xFF;
            if (cnt == state - 1 && (u32)cnt < 0xFF) {
                raw = *(u16 *)(slot + 0xA) << 16;
                rot = raw >> 16;
                if ((s32)((u32)raw >> 28) == arg0 &&
                    *(s16 *)(slot + 4) == arg2[0] &&
                    *(s16 *)(slot + 6) == arg2[1] &&
                    *(s16 *)(slot + 8) == arg2[2] &&
                    ((rot - *(s16 *)(arg3 + 0)) & 0xFFF) == 0 &&
                    ((*(s16 *)(slot + 0xC) - *(s16 *)(arg3 + 1)) & 0xFFF) == 0 &&
                    ((*(s16 *)(slot + 0xE) - *(s16 *)(arg3 + 2)) & 0xFFF) == 0) {
                    *(u8 *)(slot + 2) = age + 1;
                    return;
                }
            }
        }
        i++;
        slot += 0x10;
    } while (i < 0xB4);

    idx = D_800A3714;
    slot = (u8 *)D_800F68E0 + idx * 0x10;
    if (idx < 0xB4) {
        do {
            if (*(s16 *)slot == -1) {
                break;
            }
            idx = idx + 1;
            D_800A3714 = idx;
            slot += 0x10;
        } while (D_800A3714 < 0xB4);
    }

    mode = arg0 << 12;
    if (D_800A3714 == 0xB4) {
        D_800A3209++;
        return;
    }

    *(s16 *)(slot + 0) = 0;
    *(u8 *)(slot + 3) = arg1;
    *(u8 *)(slot + 2) = 0;
    *(s16 *)(slot + 4) = arg2[0];
    *(s16 *)(slot + 6) = arg2[1];
    *(s16 *)(slot + 8) = arg2[2];
    *(u16 *)(slot + 0xA) = (arg3[0] & 0xFFF) | mode;
    *(u16 *)(slot + 0xC) = arg3[1];
    *(u16 *)(slot + 0xE) = arg3[2];
}
