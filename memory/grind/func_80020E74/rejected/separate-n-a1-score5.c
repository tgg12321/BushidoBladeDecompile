/* Test symbols MENU_TEST / TBL_TEST stand in for the retyped menuDat / D_8008DB1C (sandbox against the old header); separate menu index n: $a1 vs target $s0, residual 5 after the unknown-symbol relocs. */
typedef struct { s32 id; s32 unk4; } MenuTest;
extern MenuTest MENU_TEST[];
extern s32 cdrom_StartReadAt(s32, s32, s32, s32);
extern u16 TBL_TEST[][8];

void func_80020E74(s32 chr0, s32 costume0, s32 chr1, s32 costume1) {
    u16 loads[130]; /* FAKE: frame layout */
    s32 i;

    if (D_800A3880 == 0) {
        func_80020DDC();
    }

    if (D_800A38DC == 1 || D_800A38DC == 4 || D_800A38DC == 6) {
        for (i = 0; i < 2; i++) {
            s32 chr = chr0;

            if (i != 0) {
                chr = chr1;
            }
            if ((&D_800A38C0)[i] != chr) {
                (&D_800A38C0)[i] = chr;
                cdrom_StartReadAt(func_80036EA8(1, 0), (s32)D_800A3888[i], chr * 7, 7);
                game_FrameLoop();
            }
        }
    }

    loads[1] = 0;
    loads[0] = 0;
    {
        u16 id0 = TBL_TEST[chr0][costume0] | (costume0 << 12);
        u16 id1;

        if (D_800A38DC == 3) {
            id1 = id0;
        } else {
            id1 = TBL_TEST[chr1][costume1] | (costume1 << 12);
        }
        D_80101F10 = id0;
        D_8010235C = id1;

        if (id0 == id1) {
            if ((&D_800A38C4)[0] == id1 || (&D_800A38C4)[1] == id1) {
                return;
            }
            loads[0] = id0;
        } else if ((&D_800A38C4)[0] == id0) {
            if ((&D_800A38C4)[1] == id1) {
                return;
            }
            loads[1] = id1;
        } else if ((&D_800A38C4)[0] == id1) {
            if ((&D_800A38C4)[1] == id0) {
                return;
            }
            loads[1] = id0;
        } else if ((&D_800A38C4)[1] == id1) {
            loads[0] = id0;
        } else {
            loads[0] = id0;
            loads[1] = id1;
        }
    }

    for (i = 0; i < 2; i++) {
        if (loads[i] != 0) {
            s32 n;

            for (n = 0; MENU_TEST[n].id != 0; n++) {
                if (MENU_TEST[n].id == loads[i]) {
                    break;
                }
            }
            cdrom_StartRead(func_80036EA8(1, n + 2), (s32)D_800A3860[i]);
            game_FrameLoop();
            D_801027B0[i][0] = (s32)D_800A3860[i] + 0x6C + (((u8 *)D_800A3860[i])[3] - 1) * 6;
            D_801027B0[i][1] = (s32)D_800A3860[i] + ((s32 *)D_800A3860[i])[1];
            D_801027B0[i][2] = (s32)D_800A3860[i] + ((s32 *)D_800A3860[i])[2];
            D_801027B0[i][3] = (s32)D_800A3860[i] + ((s32 *)D_800A3860[i])[3];
            D_801027B0[i][4] = (s32)D_800A3860[i] + ((s32 *)D_800A3860[i])[4];
            (&D_800A38C4)[i] = loads[i];
        }
    }
}
