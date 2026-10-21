/* func_800343F0 and func_800344B4. .text 0x800343F0 (ROM 0x24BF0). Start
 * boundary: PHASE (rodata-align site 3), placed by the per-file gp model (Q65).
 */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* func_800343F0 lives in this file because the file boundary follows the
 * per-file gp evidence (owner ruling Q65). */
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
    func_80041604(0, 0);
    func_80041604(1, 0);
    D_800A376A = 0;
    D_800A376B = 0;
    D_800A380C = 0;
    D_800A38D4 = 2;
    D_800A37D3 = 0;
    D_800A37D2 = 0;
}

void func_800343F0(void);

const char g_str_chanbara[] = "CHANBARA";
const char g_str_practice[] = "PRACTICE";
const char D_80010834[] = "%sCR%3d\n";
const char D_80010840[] = "%sCO%3d\n";

void func_800344B4(void) {
    func_800343F0();

    switch (D_800A38DC) {
    case 6:
        D_80102778.unk_E = 1;
        D_800A3768 = 1;
        D_800A3834 = 0;
        D_800A36F6 = (D_800A38A0 != 0);
        break;
    case 0:
        fight_InitCommandStream(D_8008D538[(s8)D_80102778.unk_4[0]]);
        D_80102778.unk_4[1] = 0;
        fight_RunCommandStream();
        fighter_ClearCpuTally(1);
        break;
    case 1:
        D_80102778.unk_4[5] = 1;
        fight_BuildOpponentOrder();
        D_800A3768 = 1;
        fight_AdvanceOpponentOrder();
        break;
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
        D_800A3768 = 1;
        D_800A3834 = 0;
        break;
    case 5:
        D_800A36A4 = (D_800A38E1 & 1) ? 0x21 : 0x20;
        g_fight_match_cursor = 0;
        gpu_ResetGraphMode1();
        eff_Init();
        fight_AdvanceMatch();
        break;
    case 2:
        D_800A3713 = (D_800A389A == 0) << 1;
        D_800A36A4 = (D_800A389A != 0) ? 0x24 : 0x23;
        D_80102778.unk_4[5] = 1;
        if (D_800A389A == 0) {
            D_80102778.unk_4[1] =
                D_8008D9EC[D_8008D538[(s8)D_80102778.unk_4[0]]] ? 0xE : 0x1D;
        }
        D_800A3834 = 0;
        break;
    case 4:
        D_800A3768 = 1;
        D_800A3834 = 0;
        break;
    default:
        D_800A3834 = 0;
        break;
    }
    if ((s8)D_80102778.unk_4[5] != 0) {
        fighter_ClearCpuTally(1);
    }
}
