extern s32 cdrom_StartReadAt(s32, s32, s32, s32);

/* Loads the motion sets and models for the two characters about to fight:
 * slot i gets character chr0 / chr1 in costume costume0 / costume1. */
void func_80020E74(s32 chr0, s32 costume0, s32 chr1, s32 costume1) {
    u16 loads[130]; /* FAKE: frame layout -- only loads[0..1] are used; the target frame (0x140) reserves 0x100 untouched bytes after them (sp+0x14..0x113), N = 129..132; loads[2]: score 20, memory/grind/func_80020E74/rejected/loads2-frame.c */
    s32 i;
    s32 j; /* FAKE: one local for loop 1's character and loop 2's menuDat index; separate locals seat the index in $a1, the target keeps both in $s0 (separate locals: score 3, memory/grind/func_80020E74/rejected/separate-locals-score3.c) */

    if (D_800A3880 == 0) {
        func_80020DDC();
    }

    if (D_800A38DC == 1 || D_800A38DC == 4 || D_800A38DC == 6) {
        for (i = 0; i < 2; i++) {
            j = chr0;
            if (i != 0) {
                j = chr1;
            }
            if (D_800A38C0[i] != j) {
                D_800A38C0[i] = j;
                cdrom_StartReadAt(func_80036EA8(1, 0), (s32)D_800A3888[i], j * 7, 7);
                game_FrameLoop();
            }
        }
    }

    loads[1] = 0;
    loads[0] = 0;
    {
        u16 id0 = D_8008DB1C[chr0][costume0] | (costume0 << 12);
        u16 id1;

        if (D_800A38DC == 3) {
            id1 = id0;
        } else {
            id1 = D_8008DB1C[chr1][costume1] | (costume1 << 12);
        }
        g_practice_menu_table[0].unk_48 = id0;
        g_practice_menu_table[1].unk_48 = id1;

        if (id0 == id1) {
            if (D_800A38C4[0] == id1 || D_800A38C4[1] == id1) {
                return;
            }
            loads[0] = id0;
        } else if (D_800A38C4[0] == id0) {
            if (D_800A38C4[1] == id1) {
                return;
            }
            loads[1] = id1;
        } else if (D_800A38C4[0] == id1) {
            if (D_800A38C4[1] == id0) {
                return;
            }
            loads[1] = id0;
        } else if (D_800A38C4[1] == id1) {
            loads[0] = id0;
        } else {
            loads[0] = id0;
            loads[1] = id1;
        }
    }

    for (i = 0; i < 2; i++) {
        if (loads[i] != 0) {
            for (j = 0; menuDat[j].id != 0; j++) {
                if (menuDat[j].id == loads[i]) {
                    break;
                }
            }
            cdrom_StartRead(func_80036EA8(1, j + 2), (s32)D_800A3860[i]);
            game_FrameLoop();
            D_801027B0[i][0] = (s32)D_800A3860[i] + 0x6C + (D_800A3860[i]->unk_03 - 1) * 6;
            D_801027B0[i][1] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[0];
            D_801027B0[i][2] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[1];
            D_801027B0[i][3] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[2];
            D_801027B0[i][4] = (s32)D_800A3860[i] + D_800A3860[i]->unk_04[3];
            D_800A38C4[i] = loads[i];
        }
    }
}
