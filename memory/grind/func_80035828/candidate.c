extern u8 D_800A3740;
extern u8 D_800A31D8;
extern u8 D_800A31D9;
extern s32 D_80102794;
extern s32 D_80106A50;
extern s32 rand(void);
extern void func_80035618(s32);
extern void func_8003553C(void);
extern s32 func_8003880C(void);
extern s32 func_800388A8(void);
extern s32 func_80038988(void);
extern void func_80035438(void);
extern void snd_SerialMixOn(void);
extern s32 func_80037110(s32);
extern void func_800371E8(s16);
extern s32 func_80077894(s32, s32);
extern void func_80035F78(s16, s32, s32, s32, s32);
extern void func_800372C0(void);
extern s32 func_80077904(void);
extern s32 func_80077B30(s32, s32);
extern void func_80034F88(void);
extern void func_80068ECC(s32);
extern s32 func_8006C1FC(s32, s32);
extern void func_80077B20(void);
extern void func_80035280(void);
extern s32 func_800779C8(s32, s32);
extern s32 func_8007855C(s32);
extern s32 func_80077A04(s32, s32);
extern s32 func_80077A60(s32, s32);
extern void func_80077940(s32);
extern s32 func_80077984(s32);
extern void func_800355E8(void);
extern s32 func_80077AC0(s32, s32);
extern void func_8003504C(void);
void func_80035828(void) {
    s32 ret;

    D_800A36F1 = 1;
    rand();
    if (D_800A3740 == 1 && D_800A31D9 != 0) {
        D_800A31D9 = 0;
        D_800A3740 = 7;
    }
    if (D_800A31DA != 0) {
        func_80035618(D_800A38DC);
        D_800A31DA = 0;
    }
    switch (D_800A3740) {
    case 7:
        func_8003553C();
        ret = func_8003880C();
        if (ret == 1) {
            D_800A3740 = 8;
            break;
        }
        if (ret == -1) {
            D_800A3740 = 1;
        }
        break;
    case 8:
        func_8003553C();
        ret = func_800388A8();
        if (ret == 1) {
            D_800A3740 = 9;
            break;
        }
        if (ret == -1) {
            D_800A3740 = 1;
        }
        break;
    case 9:
        func_8003553C();
        if (func_80038988() != 0) {
            func_80035438();
        }
        break;
    case 1:
        if (D_800A37B8 == 0) {
            snd_SerialMixOn();
            func_80037110(7);
            func_800371E8(1);
        }
        D_800A37B8++;
        if (D_80102790 != 0) {
            D_800A37B8 = 1;
        }
        if (D_800A37B8 >= 0xCA9) {
            D_800A3834 = 0xF;
        }
        ret = func_80077894(D_80102790, D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            func_80035F78(0x1E, 0, 0, 0, 0);
            D_800A3740 = 0xB;
        } else if (ret == -1) {
            func_80035F78(0x1E, 0, 0, 0, 0);
            D_800A3740 = 0xC;
        } else {
            break;
        }
        D_800A37B8 = 0x3C;
        break;
    case 11:
        if (--D_800A37B8 == 0) {
            func_800372C0();
            D_80102785 = func_80077904();
            func_80035618((s8)D_80102785);
        }
        break;
    case 12:
        if (--D_800A37B8 == 0) {
            func_800372C0();
            D_800A3834 = 0xF;
        }
        break;
    case 2:
        switch (func_80077B30(D_80102790, D_80102794)) {
        case -1:
        case 1:
            func_800372C0();
            D_800A3740 = 1;
            func_80034F88();
            D_800A37B8 = 0;
            if (g_file_disc_type == 0x3F) {
                func_80068ECC(0xFF);
            } else {
                func_80068ECC(0xF7);
            }
            break;
        case 0:
            break;
        case 2:
            func_80034F88();
            D_800A3740 = 0xD;
            break;
        case 3:
            func_80034F88();
            D_800A3740 = 0xE;
            break;
        }
        break;
    case 13:
        func_8006C1FC(0, 0);
        if (func_80038988() != 0) {
            func_80077B20();
            func_80035280();
            D_800A3740 = 2;
        }
        break;
    case 14:
        func_8006C1FC(0, 0);
        if (func_80038C70() != 0) {
            func_80077B20();
            D_800A3740 = 2;
        }
        break;
    case 3:
        ret = func_800779C8(D_80102790, D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto reset_mode;
        }
        goto check_quit;
    case 4:
        ret = func_8007855C(D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_quit;
    case 5:
        ret = func_80077A04(D_80102790, D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_cancel;
    case 6:
        ret = func_80077A60(D_80102790, D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_cancel;
        }
    load:
        func_800372C0();
        func_80077940(D_80106A50 & 0x3EF3DF);
        func_80077984(0x80118800);
        D_800A3740 = 3;
        func_800355E8();
        break;
    check_cancel:
        if (ret == -1) {
            func_800372C0();
            D_800A3740 = 1;
            D_800A37B8 = 0;
        }
        break;
    case 10:
        ret = func_80077AC0(D_80102790, D_80102794);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_quit;
        }
    reset_mode:
        func_800372C0();
        D_800A3834 = 0;
        func_8003504C();
        break;
    check_quit:
        if (ret == -1) {
            func_800372C0();
            D_800A3834 = 8;
            D_800A31D8 = 0;
        }
        break;
    }
    if (D_800A3834 != 9) {
        D_800A36F1 = 2;
    }
}
