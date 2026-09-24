extern u8 D_8009BCE4;

/* Character-select cursor / pick handler, one player per call.
 *   arg0 = this frame's pad bits (both players packed, 16 bits each)
 *   arg1 = select page (row block) index into D_8009BCF8
 *   arg2 = the player's pick list (character ids)
 *   arg3 = player index (0/1)
 * D_800A36A0 is the shared select work area; D_8009BCE4[] is the per-character
 * flag byte (bit 0 = selectable, bit 4<<player = already taken by that player).
 */
void func_80075F80(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    u8 *work;
    s16 *state;
    u8 *flag;
    s32 result;
    s32 bit;
    s32 i;
    s32 low;
    s16 value;
    u8 entry;

    work = D_800A36A0;
    if (*(s16 *)(arg3 * 2 + (s32)work + 0x10) != 0) {
        return;
    }

    if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        work = D_800A36A0;
        state = (s16 *)(arg3 * 2 + (s32)work);
        if (state[0x3C / 2] != 0) {
            arg2[state[0x3C / 2]] = -1;
            state[0x3C / 2]--;
            bit = 4;
            i = arg2[state[0x3C / 2]];
            bit <<= arg3;
            (&D_8009BCE4)[i] &= ~bit;
            return;
        }
        if (*(s32 *)(work + 0x3C) != 0) {
            return;
        }
        if (*(s16 *)(work + (arg3 != 0 ? 0x14 : 0x16)) == 2) {
            *(s16 *)(work + 0x12) = 3;
            *(s16 *)(work + 0x10) = 3;
            *(s16 *)(work + 0x1A) = 1;
            *(s16 *)(work + 0x18) = 1;
        }
        return;
    }

    result = func_800692C0((u32 *)&arg0, arg3, (s16 *)(arg3 * 4 + (s32)work + 0x40),
                           &D_800A35D0 + arg3 * 2);
    if (arg0 & (0xF000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }

    low = result & 0xFF;
    if (low < 3 && low != 0) {
        state = (s16 *)(arg3 * 2 + (s32)D_800A36A0);
        state[0x1C / 2] = (state[0x1C / 2] + 1) & 1;
    }

    switch (result >> 16) {
    case 1:
        state = (s16 *)(arg3 * 2 + (s32)D_800A36A0);
        value = state[0x20 / 2];
        if (value == 4) {
            state[0x20 / 2] = 0;
        } else {
            state[0x20 / 2] = value + 1;
        }
        break;
    case 2:
        state = (s16 *)(arg3 * 2 + (s32)D_800A36A0);
        value = state[0x20 / 2];
        if (value == 0) {
            state[0x20 / 2] = 4;
        } else {
            state[0x20 / 2] = value - 1;
        }
        break;
    }

    work = D_800A36A0;
    state = (s16 *)(arg3 * 2 + (s32)work);
    entry = ((u8 *)D_8009BCF8)[(state[0x1C / 2] * 5 + state[0x20 / 2]) * 2 + arg1 * 20];
    flag = &D_8009BCE4 + entry;
    if (*flag & 1) {
        bit = 4 << arg3;
        if ((*flag & bit) == 0) {
            u16 count;

            arg2[state[0x3C / 2]] = entry;
            if (arg0 & (0x40 << (arg3 * 16))) {
                func_8005C650(1, 0x7F, 0x7F);
                *flag |= bit;
                work = D_800A36A0;
                state = (s16 *)(arg3 * 2 + (s32)work);
                count = state[0x3C / 2];
                state[0x3C / 2] = count + 1;
                if (state[0x3C / 2] == work[0x65] + 3) {
                    state[0x10 / 2] = 1;
                    state[0x3C / 2] = count;
                    state[0x38 / 2] = 0;
                    state[0x18 / 2] = 3;
                    for (i = 0; i < state[0x60 / 2]; i++) {
                        *(s16 *)(arg3 * 10 + (s32)work + i * 2 + 0x48) = i;
                    }
                    if (arg1 != 0) {
                        *(s16 *)(arg3 * 10 + (s32)D_800A36A0 + 0x50) = 5;
                    }
                }
            }
            return;
        }
    }
    arg2[state[0x3C / 2]] = 0x14;
    if (arg0 & (0x40 << (arg3 * 16))) {
        func_8005C650(4, 0x7F, 0x7F);
    }
}
