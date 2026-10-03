#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* ---- merged from code6cac_b2_pre.c (owner ruling Q65: one original file) ---- */
/* First half of src/code6cac_b2.c (split for Phase B sec.15.1 rodata-cleanup -
 * code6cac_b2 was split into _pre and _post around replay_camera_rob_back_loose2
 * (extracted to its own .c file), preserving sibling function text addresses). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

/* Extern data declarations */





/* Extern function declarations */



















extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void func_8005B72C(void);
extern void func_80077820(s32);
















extern void *D_800A38B4;





























extern s32 g_gpu_ot_ptr;

extern void SetPolyG4(u8 *p);
extern void AddPrim(u32 *a0, u32 *a1);










/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

extern void func_80035280(void);
extern void func_80068ECC(s32);
extern u8 D_800A3740;
void func_80035438(void) {
    s32 a0;
    D_800A3740 = 1;
    func_80035280();
    if (D_80106A50.unk_04 == 0x3F) {
        a0 = 0xFF;
    } else {
        a0 = 0xF7;
    }
    func_80068ECC(a0);
}
extern u8 D_800A31D8;
extern void func_8003A41C(void);
extern void func_80020CDC(void);
void func_80035480(void) {
    gpu_ResetGraphMode1();
    gpu_InitDisplay();
    if (D_800A31DA == 0) {
        func_8003A41C();
    }
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    if (D_800A31D8 != 0) {
        func_8005B72C();
        D_800A390E = -1;
    }
    D_800A31D8 = 1;
    func_80035438();
    func_80077820((s32)0x80118800);
    D_800A37B8 = 0;
    D_800A3834 = 9;
    gpu_SetDispMaskOn();
}
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

void func_8003553C(void) {
    POLY_G4 *g;
    POLY_G4 *q;
    u32 *ot;

    g = (POLY_G4 *)D_800A38B4;
    SetPolyG4((u8 *)g);
    g->x0 = 0; g->y0 = 0;
    g->x1 = 640; g->y1 = 0;
    g->x2 = 0; g->y2 = 240;
    g->x3 = 640; g->y3 = 240;
    g->r0 = 0; g->g0 = 0; g->b0 = 0x80;
    g->r1 = 0; g->g1 = 0; g->b1 = 0x80;
    g->r2 = 0; g->g2 = 0; g->b2 = 0;
    g->r3 = 0; g->g3 = 0; g->b3 = 0;
    ot = (u32 *)(g_gpu_ot_ptr + 0x401C);
    q = g;
    g += 1;
    AddPrim(ot, (u32 *)q);
    D_800A38B4 = g;
}
void func_800355E8(void) {
    snd_SerialMixOn();
    func_80037110(1);
    func_800371E8(1);
}

/* ---- merged from replay_camera_rob_back_loose2.c (owner ruling Q65: one original file) ---- */
/* Sub-TU split out from src/code6cac_b2.c (Phase B §15.1). Same includes
 * + inline externs as code6cac_b2.c so cc1 sees identical declarations. */
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"


void func_80035618(s32 arg0) {
    s32 temp;

    if ((u32)arg0 >= 8) {
        return;
    }

    switch (arg0) {
    case 0:
        if (D_800A31DA == 0) {
            func_800784E4(0x80118800);
            snd_SerialMixOn();
            func_80037110(0);
            D_800A3740 = 4;
            return;
        }
        func_80077984(0x80118800);
        D_800A3740 = 3;
        func_800355E8();
        return;

    case 2:
        D_800A3740 = 5;
        func_800355E8();
        return;

    case 3:
        if (D_800A31DA == 0) {
            func_80077A28();
            D_800A3740 = 6;
        } else {
            func_80077984(0x80118800);
            D_800A3740 = 3;
            func_800355E8();
        }
        func_800355E8();
        return;

    case 6:
        if (D_800A31DA != 0) {
            func_80077984(0x80118800);
            D_800A3740 = 3;
            func_800355E8();
            return;
        }

        temp = func_8003ACB8();
        if (temp == 1) {
            func_80077940((D_80106A50.unk_00 | D_800A38E4 | 0x7007) & 0x003FF3FF);
            func_80077984(0x80118800);
            D_800A3740 = 3;
            func_800355E8();
            return;
        }

        if (temp == -1) {
            func_8005C650(2, 0x7F, 0x7F);
            D_800A3834 = 9;
            D_800A37B8 = 0;
            D_800A3740 = 1;
        }
        return;

    case 1:
        func_80077940(D_80106A50.unk_00 & 0x003EF3DF);
        func_80077984(0x80118800);
        D_800A3740 = 3;
        func_800355E8();
        return;

    case 5:
        func_80077940(D_80106A50.unk_00 & 0x003FF3FF);
        func_80077A80(0x80118800);
        D_800A3740 = 0xA;
        func_800355E8();
        return;

    case 4:
        func_80077940(D_80106A50.unk_00 & 0x053FF3FF);
        func_80077984(0x80118800);
        D_800A3740 = 3;
        func_800355E8();
        return;

    case 7:
        D_800A3740 = 2;
        func_800355E8();
        return;
    }
}

/* ---- merged from code6cac_b2_post.c (owner ruling Q65: one original file) ---- */
/* Padding NOP macro */

/* Extern data declarations */





/* Extern function declarations */




extern void VSync(s32);



































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
extern s32 func_8006C1FC(s32, s32);
extern void func_80077B20(void);
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
        if (g_pad_state.held != 0) {
            D_800A37B8 = 1;
        }
        if (D_800A37B8 >= 0xCA9) {
            D_800A3834 = 0xF;
        }
        ret = func_80077894(g_pad_state.held, g_pad_state.pressed);
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
        switch (func_80077B30(g_pad_state.held, g_pad_state.pressed)) {
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
        ret = func_800779C8(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto reset_mode;
        }
        goto check_quit;
    case 4:
        ret = func_8007855C(g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_quit;
    case 5:
        ret = func_80077A04(g_pad_state.held, g_pad_state.pressed);
        if (ret == 0) {
            break;
        }
        if (ret == 1) {
            goto load;
        }
        goto check_cancel;
    case 6:
        ret = func_80077A60(g_pad_state.held, g_pad_state.pressed);
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
        ret = func_80077AC0(g_pad_state.held, g_pad_state.pressed);
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

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
u8 D_800A31D8 = 1;
u8 D_800A31D9 = 1;
u8 D_800A31DA = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
u8 D_800A3740;
