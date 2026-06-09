/* Sub-TU split out from src/code6cac_b2.c (Phase B §15.1). Same includes
 * + inline externs as code6cac_b2.c so cc1 sees identical declarations. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern s32 D_80106A50;
extern u8 D_800A3740;

void replay_camera_rob_back_loose2(s32 arg0) {
    s32 temp;

    if ((u32)arg0 >= 8) {
        return;
    }

    switch (arg0) {
    case 0:
        if (D_800A31DA == 0) {
            camera_get_rot_normal_rad(0x80118800);
            func_80035FA8();
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
            func_80077940((D_80106A50 | D_800A38E4 | 0x7007) & 0x003FF3FF);
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
        func_80077940(D_80106A50 & 0x003EF3DF);
        func_80077984(0x80118800);
        D_800A3740 = 3;
        func_800355E8();
        return;

    case 5:
        func_80077940(D_80106A50 & 0x003FF3FF);
        func_80077A80(0x80118800);
        D_800A3740 = 0xA;
        func_800355E8();
        return;

    case 4:
        func_80077940(D_80106A50 & 0x053FF3FF);
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
