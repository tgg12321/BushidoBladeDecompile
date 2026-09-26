/* REJECTED (layer-2 FAIL 2026-09-25): per-use ((s8 *)&D_X)[i] puns on the lesson bytes, justified by a
 * disproven separate-scalar claim. A single-base struct is byte-neutral; see evidence.md. */
extern void func_8003E164(s32);
extern s32 func_80048AD0(s32);
extern void func_80020D38(void);
extern void func_80020E74(s32, s32, s32, s32);
extern void func_80021210(void);
extern void func_80021280(s32);
extern void func_80022F34(void);
extern void func_800218C8(s32);
extern s32 func_80021974(s32);
extern s32 func_80021904(s32);
extern s32 func_800219E4(s32);
extern void func_8001B294(s32 *, s32 *);
extern void func_8001B3C0(s32 *, s32 *);
extern void func_80033510(void);
extern s32 func_8005BE84(s32);
extern void rng_SetSeed(s32);
extern void player_SetCharId(s32, s32);
void func_8001DCB0(void) {
    s32 i;
    s32 addr;
    s32 v;

    func_8005B5AC();
    if (g_disp_enable != DISP_ACTIVE) {
        gpu_InitDisplay();
        gpu_SetDispMaskOn();
    }
    func_800174F4();
    gpu_ResetGraphMode1();
    func_8003E22C();
    func_8003043C();
    func_80032040();
    func_8003F218(D_800A38BA);
    SetGeomScreen(math_FovToScreenDist(D_800A38BA != 0 ? 0x50 : 0x2D));
    /* The P1/P2 lesson bytes (0x8010277C/D, 7E/F, 80/81) are separate
     * scalars: func_80033BC0 stores 0x8010277F twice through its own
     * zero-addend symbol, which an s8[2] element cannot reproduce.  Per-player
     * code indexes from the P1 byte, as func_8003AF40 does. */
    for (i = 0; i < 2; i++) {
        if (D_800A38DC != 0) {
            player_SetCharId(0, 0);
        }
        func_80022580(i, ((s8 *)&D_80102780)[i], ((s8 *)&D_8010277C)[i], ((s8 *)&D_8010277E)[i], 0);
        if (D_800A38BA != 0 && D_800A36F6 == i) {
            func_8003E164(i == 0);
        }
    }
    func_8003FFE0(0);
    func_8003FFE0(1);
    if (D_800A3670 == 0) {
        func_8004939C();
        addr = (s32)0x80190800;
        func_80020D38();
        for (i = 0; i < 2; i++) {
            if (D_800A38BA != 0 && D_800A36F6 == i) {
                func_80040510(i, D_8008D578[((s8 *)&D_8010277C)[i]], addr);
                func_80048AD0(i);
            } else if (D_800A38DC == 3 && i == 1) {
                func_80040510(1, D_800A38DE, 0);
            } else {
                func_80040510(i, D_8008D578[((s8 *)&D_8010277C)[i]], addr);
            }
            func_800493E4(g_practice_menu_table[i].unk_12);
            if (D_800A38DC != 3 || i != 1) {
                if ((D_800A38DC == 2 && D_800A389A == 0) || D_800A38DC == 5) {
                    func_800494D4(i, D_8008E6A4[g_practice_menu_table[i].unk_0A][g_practice_menu_table[i].unk_0E]);
                } else {
                    func_800494D4(i, D_8008E5CC[g_practice_menu_table[i].unk_0A][g_practice_menu_table[i].unk_0E]);
                }
            }
            if (g_practice_menu_table[i].unk_14 != -1) {
                func_800493E4(D_8008EB80[g_practice_menu_table[i].unk_14]);
                if (g_practice_menu_table[i].unk_14 == 14) {
                    func_800493E4(D_8008EB80[14] + 3);
                }
            }
        }
        func_80049584(addr);
        func_80041688(0, 0);
        func_80041688(1, 0);
        if (D_800A38DC == 0 && D_800A3712 == 0) {
            func_80041BF4(D_800A37B4, D_800A37B5, D_800A37B6);
        } else if (D_800A38DC == 3) {
            func_80041BF4(D_800A38EC, D_800A38ED, D_800A38EE);
        } else if (D_800A38DC == 2) {
            u8 *p = D_800A3100[D_8008D9EC[g_practice_menu_table[0].unk_0A]];
            if (D_800A389A == 0) {
                func_80041BF4(p[0], p[1], p[2]);
            }
        }
        func_8001D790();
        if (D_800A38DC == 5) {
            func_8001D904();
        }
        if (D_800A38DC == 3) {
            func_8001D998();
            func_8001DB9C();
        }
        func_80020E74(D_8008D538[(s8)D_8010277C], (s8)D_8010277E,
                      D_8008D538[D_8010277D], D_8010277F);
    } else if (D_800A38DC == 5) {
        D_800A391E = 1;
    }
    func_80021210();
    func_80021280(0);
    func_80021280(1);
    func_80022F34();
    if (D_800A38DC == 2 || D_800A38DC == 5) {
        if (D_800A3670 != 0) {
            func_800218C8(0);
            v = func_80021974(0);
            g_practice_menu_table[0].unk_5E = 0;
            func_80021A98(0, (u8 *)v, 0);
            if (D_800A38DC == 2 && D_800A389A == 0) {
                v = func_80021904(1);
                g_practice_menu_table[1].unk_5E = 0;
                func_80021A98(1, (u8 *)v, 0);
            } else {
                func_800218C8(1);
                v = func_80021974(1);
                g_practice_menu_table[1].unk_5E = 0;
                func_80021A98(1, (u8 *)v, 0);
            }
        } else {
            func_800218C8(0);
            func_800218C8(1);
            v = func_800219E4(0);
            g_practice_menu_table[0].unk_5E = 1;
            func_80021A98(0, (u8 *)v, 1);
            v = func_800219E4(1);
            g_practice_menu_table[1].unk_5E = 1;
            func_80021A98(1, (u8 *)v, 1);
        }
    } else {
        func_800218C8(0);
        func_800218C8(1);
        v = func_80021974(0);
        g_practice_menu_table[0].unk_5E = 0;
        func_80021A98(0, (u8 *)v, 0);
        v = func_80021974(1);
        g_practice_menu_table[1].unk_5E = 0;
        func_80021A98(1, (u8 *)v, 0);
    }
    D_800A382E = 0;
    D_800A3748 = -1;
    func_8001B294((s32 *)&g_practice_menu_table[0], (s32 *)&g_practice_menu_table[1]);
    if (D_800A38BA != 0) {
        func_8001B3C0((s32 *)&g_practice_menu_table[0], (s32 *)&g_practice_menu_table[1]);
    }
    func_800392C8();
    game_Cleanup();
    func_8001DBE4();
    g_disp_enable = DISP_DISABLED;
    g_disp_fade = 0;
    eff_Init();
    D_800A3670 = 0;
    D_800A3834 = 1;
    func_8001C820();
    func_8001DA8C();
    func_80033510();
    func_8005BE84(D_800A36A4);
    if (D_800A38DC == 6) {
        rng_SetSeed(D_800A3904);
    }
}
