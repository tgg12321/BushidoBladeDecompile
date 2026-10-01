/* Character-select cursor / pick handler; called once per player per frame.
 *   arg0 = this frame's pad bits, both players packed (player N in bits N*16)
 *   arg1 = select page into D_8009BCF8 (2 rows x 5 columns per page)
 *   arg2 = this player's pick list (character ids, -1 = cleared slot)
 *   arg3 = player index (0/1)
 * SELWORK is the shared select work area (per-player pairs); D_8009BCE4[] is
 * the per-character flag byte (bit 0 = selectable, bit 4<<player = already
 * taken by that player). */
void func_80075F80(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    u8 *flag;
    s32 result;
    s32 bit;
    s32 i;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }

    if (arg0 & (0x10 << (arg3 * 16))) {
        s32 index;

        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C.half[arg3] != 0) {
            arg2[SELWORK->f3C.half[arg3]] = -1;
            SELWORK->f3C.half[arg3]--;
            index = arg2[SELWORK->f3C.half[arg3]];
            D_8009BCE4[index] &= ~(4 << arg3);
            return;
        }
        if (SELWORK->f3C.word != 0) {
            return;
        }
        if (SELWORK->f14.half[(arg3 != 0) ? 0 : 1] == 2) {
            SELWORK->f10.half[1] = 3;
            SELWORK->f10.half[0] = 3;
            SELWORK->f18[1] = 1;
            SELWORK->f18[0] = 1;
        }
        return;
    }

    result = func_800692C0(&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]);
    if (arg0 & (0xF000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    {
        s32 low;

        low = result & 0xFF;
        if (low < 3 && low != 0) {
            SELWORK->f1C[arg3] = (SELWORK->f1C[arg3] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 value;

        value = SELWORK->f20[arg3];
        if (value == 4) {
            SELWORK->f20[arg3] = 0;
        } else {
            SELWORK->f20[arg3] = value + 1;
        }
    } break;
    case 2: {
        s16 value;
        s16 next;

        value = SELWORK->f20[arg3];
        if (value == 0) {
            next = 4;
        } else {
            next = value - 1;
        }
        SELWORK->f20[arg3] = next;
    } break;
    }

    {
        u8 entry;

        entry = D_8009BCF8[arg1][SELWORK->f1C[arg3] * 5 + SELWORK->f20[arg3]].unk0;
        flag = &D_8009BCE4[entry];
        if ((*flag & 1) != 0) {
            bit = 4 << arg3;
            if ((*flag & bit) == 0) {
                u16 count;

                arg2[SELWORK->f3C.half[arg3]] = entry;
                if (arg0 & (0x40 << (arg3 * 16))) {
                    func_8005C650(1, 0x7F, 0x7F);
                    *flag |= bit;
                    count = SELWORK->f3C.half[arg3];
                    SELWORK->f3C.half[arg3] = count + 1;
                    if (SELWORK->f3C.half[arg3] == SELWORK->f65 + 3) {
                        SELWORK->f10.half[arg3] = 1;
                        SELWORK->f3C.half[arg3] = count;
                        SELWORK->f38[arg3] = 0;
                        SELWORK->f18[arg3] = 3;
                        for (i = 0; i < SELWORK->f60[arg3]; i++) {
                            SELWORK->f48[arg3][i] = i;
                        }
                        if (arg1 != 0) {
                            SELWORK->f48[arg3][4] = 5;
                        }
                    }
                }
                return;
            }
        }
        arg2[SELWORK->f3C.half[arg3]] = 0x14;
        if (arg0 & (0x40 << (arg3 * 16))) {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
