#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */





/* Extern function declarations */




extern void VSync(s32);














extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void func_8005B72C(void);
extern void func_80077820(s32);





















extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void func_800174F4(void);
extern void func_8003AAB0(void);







extern void snd_Quit(void);
extern void memcard_Quit(void);
extern void StopPAD(void);
extern void StopCallback(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);



















/* Continuation of src/code6cac_b2.c (split for Phase B sec.15.1 rodata-cleanup -
 * replay_camera_rob_back_loose2 extracted to its own .c file, requiring this
 * file to be split around it to preserve text addresses). */
extern u8 D_800A3740;
extern u8 D_800A31D8;
extern u8 D_800A31D9;
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
        if (D_80102788.held != 0) {
            D_800A37B8 = 1;
        }
        if (D_800A37B8 >= 0xCA9) {
            D_800A3834 = 0xF;
        }
        ret = func_80077894(D_80102788.held, D_80102788.pressed);
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
            D_80102778.unk_D = func_80077904();
            func_80035618((s8)D_80102778.unk_D);
        }
        break;
    case 12:
        if (--D_800A37B8 == 0) {
            func_800372C0();
            D_800A3834 = 0xF;
        }
        break;
    case 2:
        switch (func_80077B30(D_80102788.held, D_80102788.pressed)) {
        case -1:
        case 1:
            func_800372C0();
            D_800A3740 = 1;
            func_80034F88();
            D_800A37B8 = 0;
            if (D_80106A50.unk_04 == 0x3F) {
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
        ret = func_800779C8(D_80102788.held, D_80102788.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto reset_mode;
        }
        goto check_quit;
    case 4:
        ret = func_8007855C(D_80102788.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_quit;
    case 5:
        ret = func_80077A04(D_80102788.held, D_80102788.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_cancel;
    case 6:
        ret = func_80077A60(D_80102788.held, D_80102788.pressed);
        if (ret == 0) {
            break;
        }
        if (ret != 1) {
            goto check_cancel;
        }
    load:
        func_800372C0();
        func_80077940(D_80106A50.unk_00 & 0x3EF3DF);
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
        ret = func_80077AC0(D_80102788.held, D_80102788.pressed);
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
void func_80035DC8(void) {
    gpu_ResetGraphMode1();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    func_8005B72C();
    func_80077820((s32)0x80118800);
    D_800A3834 = 0x1B;
    gpu_SetDispMaskOn();
}
void func_80035E38(void) {
    D_800A36F1 = 1;
    func_8003553C();
    if (func_80038C70() != 0) {
        D_800A3834 = 8;
        D_800A36F1 = 2;
    }
}
s32 bits_ExtractMask3F83F8(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << i)) {
                result |= (one << j);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
s32 bits_DepositMask3F83F8(s32 a0) {
    s32 result = 0;
    s32 i = 0;
    s32 j = 0;
    s32 mask = 0x3F83F8;
    s32 one = 1;
    do {
        if ((mask >> i) & 1) {
            if (a0 & (one << j)) {
                result |= (one << i);
            }
            j++;
        }
        i++;
    } while (i < 0x1B);
    return result;
}
