void func_80020E74(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    u16 loads[2];
    u8 file_name[256];
    s32 i;
    u16 *load;
    s32 cache_offset;
    s32 output_offset;
    s32 index;

    if (D_800A3880 == 0) {
        func_80020DDC();
    }

    if (D_800A38DC == 1 || D_800A38DC == 4 || D_800A38DC == 6) {
        i = 0;
        do {
            s32 player = arg0;

            if (i != 0) {
                player = arg2;
            }

            if (D_800A38C0[i] != player) {
                D_800A38C0[i] = player;
                func_80036E34(func_80036EA8(1, 0), D_800A3888[i], player * 7, 7);
                game_FrameLoop();
            }
            i++;
        } while (i < 2);
    }

    loads[1] = 0;
    loads[0] = 0;
    {
        u16 player0 = D_8008DB1C[arg0][arg1] | (arg1 << 12);
        u16 player1;

        if (D_800A38DC == 3) {
            player1 = player0;
        } else {
            player1 = D_8008DB1C[arg2][arg3] | (arg3 << 12);
        }

        D_80101F10 = player0;
        D_8010235C = player1;

        if (player0 == player1) {
            if (D_800A38C4[0] == player1 || D_800A38C4[1] == player1) {
                return;
            }
            loads[0] = player0;
        } else if (D_800A38C4[0] == player0) {
            if (D_800A38C4[1] == player1) {
                return;
            }
            loads[1] = player1;
        } else if (D_800A38C4[0] == player1) {
            if (D_800A38C4[1] == player0) {
                return;
            }
            loads[1] = player0;
        } else if (D_800A38C4[1] == player1) {
            loads[0] = player0;
        } else {
            loads[0] = player0;
            loads[1] = player1;
        }
    }

    i = 0;
    load = loads;
    cache_offset = 0;
    output_offset = 0;
    do {
        s32 selection = *load;
        if (selection != 0) {
            s32 *base_slot;
            s32 temp;
            s32 load_handle;
            s32 relative;

            index = 0;
            if (menuDat != 0) {
                s32 *menu_entry = &menuDat;
                s32 menu_value = *menu_entry;
            menu_loop_20E74:
                menu_entry += 2;
                if (menu_value != selection) {
                    menu_value = *menu_entry;
                    index++;
                    if (menu_value != 0) {
                        goto menu_loop_20E74;
                    }
                }
            }

            load_handle = func_80036EA8(1, index + 2);
            relative = (s32)D_800A3860;
            index = i * 4;
            base_slot = (s32 *)(relative + index);
            replay_camera_Init(load_handle, *base_slot);
            game_FrameLoop();

            temp = *base_slot;
            relative = ((*(u8 *)(temp + 3) - 1) * 6) + 0x6C;
            *(s32 *)((u8 *)&D_801027B0 + output_offset) = temp + relative;
            temp = *base_slot;
            relative = *(s32 *)(temp + 4);
            *(s32 *)((u8 *)&D_801027B4 + output_offset) = temp + relative;
            temp = *base_slot;
            relative = *(s32 *)(temp + 8);
            *(s32 *)((u8 *)&D_801027B8 + output_offset) = temp + relative;
            temp = *base_slot;
            relative = *(s32 *)(temp + 12);
            *(s32 *)((u8 *)&D_801027BC + output_offset) = temp + relative;
            temp = *base_slot;
            relative = *(s32 *)(temp + 16);
            *(s32 *)((u8 *)&D_801027C0 + output_offset) = temp + relative;
            *(u16 *)((u8 *)D_800A38C4 + cache_offset) = *load;
        }
        load++;
        cache_offset += 2;
        output_offset += 20;
        i++;
    } while (i < 2);
}