/* func_800343F0 and func_800344B4. .text 0x800343F0 (ROM 0x24BF0). Start
 * boundary: PHASE (rodata-align site 3), placed by the per-file gp model (Q65).
 */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include "gte.h"
#include "bb2_const.h"

/* func_800343F0 lives here, not in code6cac_b_tu2.c: the file boundary follows
 * the per-file gp evidence (owner ruling Q65;
 * docs/grind/rodata-align-2026-09-30.md section 7). */

void func_800343F0(void) {
    s8 val_85 = (s8)D_80102778.unk_D;
    s8 val_86 = (s8)D_80102778.unk_E;
    s8 val_84 = (s8)D_80102778.unk_C;
    s32 val_87 = (s8)D_80102778.unk_F;

    D_800A36F6 = 0;
    D_800A38DC = val_85;
    D_800A38BA = val_86;
    D_800A3140 = val_87;
    D_800A36A4 = val_84;
    player_SetCharId(0, 0);
    player_SetCharId(1, 0);
    D_800A376A = 0;
    D_800A376B = 0;
    D_800A380C = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}

/* Declarations from the file this TU was split from (code6cac_b.c). */
void func_800343F0(void);

INCLUDE_RODATA("asm/rodata", jtbl_8001084C);

void func_800344B4(void) {
    func_800343F0();

    switch (D_800A38DC) {
    case 6:
        D_80102778.unk_E = 1;
        g_disp_enable = 1;
        D_800A3834 = 0;
        D_800A36F6 = (D_800A38A0 != 0);
        goto skip_clear;

    case 0:
        func_8003B20C(D_8008D538[(s8)D_80102778.unk_4[0]]);
        D_80102778.unk_4[1] = 0;
        func_8003B5A4();
        func_8005509C(1);
        goto skip_clear;

    case 1:
        D_80102778.unk_4[5] = 1;
        func_800338CC();
        g_disp_enable = 1;
        func_80033BC0();
        goto skip_clear;

    case 3:
        D_80102778.unk_4[5] = 1;
        D_80102778.unk_4[3] = 0;
        D_80102778.unk_4[2] = 0;
        D_800A38E2 = 0;
        D_800A38E0 = 0;
        D_800A3858 = 0;
        D_800A3728 = 0;
        D_800A36A4 = 0x22;
        func_80033DF4();
        g_disp_enable = 1;
        break;

    case 5: {
        s32 v1 = (D_800A38E1 & 1) ? 0x21 : 0x20;
        D_800A36A4 = (s16)v1;
    }
        D_800A3874 = 0;
        gpu_ResetGraphMode1();
        eff_Init();
        func_800342A0();
        goto skip_clear;

    case 2: {
        s32 v1 = D_800A389A;
        s32 cmp = (u32)v1 < 1u;
        D_800A3713 = (u8)(cmp << 1);
        {
            s32 da = (v1 != 0) ? 0x24 : 0x23;
            D_800A36A4 = da;
        }
        D_80102778.unk_4[5] = 1;
        if (v1 != 0) {
            break;
        }
    }
        {
            u8 idx = D_8008D538[(s8)D_80102778.unk_4[0]];
            u8 val = D_8008D9EC[idx];
            s32 tmp = (val != 0) ? 0x0E : 0x1D;
            D_80102778.unk_4[1] = tmp;
        }
        break;

    case 4:
        g_disp_enable = 1;
        break;
    }

    D_800A3834 = 0;

skip_clear:
    if ((s8)D_80102778.unk_4[5] != 0) {
        func_8005509C(1);
    }
}
