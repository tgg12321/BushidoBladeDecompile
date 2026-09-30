/* Character-select cursor / pick handler; called once per player per frame.
 *   arg0 = this frame's pad bits, both players packed (player N in bits N*16)
 *   arg1 = select page into D_8009BCF8 (10 cells per page, 2 rows x 5 columns)
 *   arg2 = this player's pick list (character ids, -1 = cleared slot)
 *   arg3 = player index (0/1)
 * D_800A36A0 is the shared select work area, two bytes of state per player at
 * each offset; D_8009BCE4[] is the per-character flag byte (bit 0 = selectable,
 * bit 4<<player = already taken by that player). */
void func_80075F80(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    typedef struct {
        u8 pad00[0x48];
        s16 slots[2][5];
    } MenuWork;
    u8 *flag;
    s16 *state;
    s32 result;
    s32 bit;
    s32 i;
    u8 *base;

    base = D_800A36A0;
    if (*(s16 *)((base + (arg3 * 2)) + 0x10) != 0) {
        return;
    }

    if (arg0 & (0x10 << (arg3 * 16))) {
        u8 *cancel_base;
        s16 *cancel_state;
        s32 index;

        func_8005C650(2, 0x7F, 0x7F);
        cancel_base = D_800A36A0;
        cancel_state = (s16 *)((arg3 * 2) + (s32)cancel_base);
        if (cancel_state[0x3C / 2] != 0) {
            arg2[cancel_state[0x3C / 2]] = -1;
            cancel_state[0x3C / 2]--;
            index = arg2[cancel_state[0x3C / 2]];
            (&D_8009BCE4)[index] &= ~(4 << arg3);
            return;
        }
        if (*(s32 *)(cancel_base + 0x3C) != 0) {
            return;
        }
        if (*(s16 *)(cancel_base + ((arg3 != 0) ? 0x14 : 0x16)) == 2) {
            *(s16 *)(cancel_base + 0x12) = 3;
            *(s16 *)(cancel_base + 0x10) = 3;
            *(s16 *)(cancel_base + 0x1A) = 1;
            *(s16 *)(cancel_base + 0x18) = 1;
        }
        return;
    }

    result = func_800692C0((u32 *)&arg0, arg3, (s16 *)(base + ((arg3 * 4) + 0x40)),
                           (&D_800A35D0) + (arg3 * 2));
    if (arg0 & (0xF000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    {
        s32 low;

        low = result & 0xFF;
        if (low < 3 && low != 0) {
            s16 *toggle_state;

            toggle_state = (s16 *)((arg3 * 2) + (s32)D_800A36A0);
            toggle_state[0x1C / 2] = (toggle_state[0x1C / 2] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 *pstate;
        s16 value;

        pstate = (s16 *)((arg3 * 2) + (s32)D_800A36A0);
        value = pstate[0x20 / 2];
        if (value == 4) {
            pstate[0x20 / 2] = 0;
        } else {
            pstate[0x20 / 2] = value + 1;
        }
    } break;
    case 2: {
        s16 *pstate;
        s16 value;
        s16 next;

        pstate = (s16 *)((arg3 * 2) + (s32)D_800A36A0);
        value = pstate[0x20 / 2];
        if (value == 0) {
            next = 4;
        } else {
            next = value - 1;
        }
        pstate[0x20 / 2] = next;
    } break;
    }

    {
        u8 *select_base;
        s32 index;
        u8 entry;

        select_base = D_800A36A0;
        state = (s16 *)((arg3 * 2) + (s32)select_base);
        index = (state[0x1C / 2] * 5) + state[0x20 / 2];
        entry = (&D_8009BCF8[index] + arg1 * 10)->unk0;
        flag = (&D_8009BCE4) + entry;
        if ((*flag & 1) != 0) {
            bit = 4 << arg3;
            if ((*flag & bit) == 0) {
                u8 *done_base;
                u16 count;

                arg2[state[0x3C / 2]] = entry;
                if (arg0 & (0x40 << (arg3 * 16))) {
                    func_8005C650(1, 0x7F, 0x7F);
                    *flag |= bit;
                    done_base = D_800A36A0;
                    state = (s16 *)((arg3 * 2) + (s32)done_base);
                    count = state[0x3C / 2];
                    state[0x3C / 2] = count + 1;
                    if (state[0x3C / 2] == (done_base[0x65] + 3)) {
                        state[0x10 / 2] = 1;
                        state[0x3C / 2] = count;
                        state[0x38 / 2] = 0;
                        state[0x18 / 2] = 3;
                        for (i = 0; i < state[0x60 / 2]; i++) {
                            ((MenuWork *)done_base)->slots[arg3][i] = i;
                        }
                        if (arg1 != 0) {
                            ((MenuWork *)D_800A36A0)->slots[arg3][4] = 5;
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
}
