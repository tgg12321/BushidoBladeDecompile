/* menu_PrintEditAndApplyConfigWithPerPadCursors alone. .text 0x80034708 (ROM
 * 0x24F08). Start boundary: G8: compiled -G8, as the gp-relative reads of the
 * cursor array D_800A3174 show; so the <=8-byte strings sit in .sdata, the
 * 9-byte ones in .rodata. */
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include "bb2_const.h"

/* This menu's two cursors ([0] = P1 row 0..11, [1] = P2 row 0..3) and its
 * colour / format strings. */
extern s16 D_800A3174[2];
extern u8 D_800A3178[];
extern u8 D_800A3180[];
extern u8 D_800A3188[];
extern u8 D_800A3190[];
extern u8 D_800A3198[];
extern u8 D_800A31A0[];
extern u8 D_800A31A8[];
extern u8 D_800A31B0[];
extern u8 D_800A31B8[];
extern u8 D_800A31C0[];
extern u8 D_800A31C8[];
extern u8 D_800A31D0[];
extern const char D_80010834[];
extern const char D_80010840[];

void menu_PrintEditAndApplyConfigWithPerPadCursors(void) {
    s32 i;
    u8 *var_1;
    u8 *var_2;

    g_handler_frame_counter++;
    rand();
    var_1 = D_800A3178;
    var_2 = D_800A3180;
    font_Printf(D_800A3188, (s32)(D_800A3174[0] == 0 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[0]);
    font_Printf(D_800A3190, (s32)(D_800A3174[1] == 0 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[1]);
    font_Printf(D_800A3188, (s32)(D_800A3174[0] == 1 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[2]);
    font_Printf(D_800A3190, (s32)(D_800A3174[1] == 1 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[3]);
    font_Printf(D_800A3198, (s32)(D_800A3174[0] == 2 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[4]);
    font_Printf(D_800A3190, (s32)(D_800A3174[1] == 2 ? var_2 : var_1),
                (s8)g_fight_config.unk_4[5]);
    font_Printf(D_800A31A0, (s32)(D_800A3174[0] == 3 ? var_2 : var_1),
                g_fight_config.unk_0[0] >> 8);
    font_Printf(D_800A31A8, (s32)(D_800A3174[1] == 3 ? var_2 : var_1),
                g_fight_config.unk_0[1] >> 8);
    font_Printf(D_800A31B0, (s32)(D_800A3174[0] == 4 ? var_2 : var_1),
                (s8)g_fight_config.unk_C);
    font_Printf(D_800A31B0, (s32)(D_800A3174[0] == 5 ? var_2 : var_1),
                (s8)g_fight_config.unk_D);
    font_Printf(D_800A31B8, (s32)(D_800A3174[0] == 6 ? var_2 : var_1),
                (s8)g_fight_config.unk_E);
    font_Printf((u8 *)D_80010834, (s32)(D_800A3174[0] == 7 ? var_2 : var_1),
                (s8)g_fight_config.unk_F);
    font_Printf((u8 *)D_80010840, (s32)(D_800A3174[0] == 8 ? var_2 : var_1),
                g_saved_record.flags & 1);
    font_Printf(D_800A31C0, (s32)(D_800A3174[0] == 9 ? var_2 : var_1),
                (g_saved_record.flags >> 1) & 1);
    font_Printf(
        D_800A31C8, (s32)(D_800A3174[0] == 10 ? var_2 : var_1), D_800A36F9);
    font_Printf(
        D_800A31D0, (s32)(D_800A3174[0] == 11 ? var_2 : var_1), D_800A3690);

    for (i = 0; i < 2; i++) {
        if (g_pad_state.pressed & (0x1000 << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3174[i] > 0) {
                D_800A3174[i]--;
            } else {
                D_800A3174[i] = (i != 0) ? 3 : 11;
            }
        } else if (g_pad_state.pressed & (0x4000 << (i * 16))) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A3174[i] < ((i != 0) ? 3 : 11)) {
                D_800A3174[i]++;
            } else {
                D_800A3174[i] = 0;
            }
        } else if (g_pad_state.pressed & (0x8000 << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                g_fight_config.unk_4[i]--;
                break;
            case 1:
                g_fight_config.unk_4[2 + i]--;
                break;
            case 2:
                g_fight_config.unk_4[4 + i]--;
                break;
            case 3:
                g_fight_config.unk_0[i] -= 0x80;
                break;
            case 4:
                g_fight_config.unk_C--;
                break;
            case 5:
                g_fight_config.unk_D--;
                break;
            case 6:
                g_fight_config.unk_E--;
                break;
            case 7:
                g_fight_config.unk_F--;
                break;
            case 8:
                g_saved_record.flags ^= 1;
                break;
            case 9:
                g_saved_record.flags ^= 2;
                break;
            case 10:
                D_800A36F9--;
                break;
            case 11:
                D_800A3690--;
                break;
            }
        } else if (g_pad_state.pressed & (0x2000 << (i * 16))) {
            snd_QueueSound(4, 0x3F, 0x3F);
            switch (D_800A3174[i]) {
            case 0:
                g_fight_config.unk_4[i]++;
                break;
            case 1:
                g_fight_config.unk_4[2 + i]++;
                break;
            case 2:
                g_fight_config.unk_4[4 + i]++;
                break;
            case 3:
                g_fight_config.unk_0[i] += 0x80;
                break;
            case 4:
                g_fight_config.unk_C++;
                break;
            case 5:
                g_fight_config.unk_D++;
                break;
            case 6:
                g_fight_config.unk_E++;
                break;
            case 7:
                g_fight_config.unk_F++;
                break;
            case 8:
                g_saved_record.flags ^= 1;
                break;
            case 9:
                g_saved_record.flags ^= 2;
                break;
            case 10:
                D_800A36F9++;
                break;
            case 11:
                D_800A3690++;
                break;
            }
        }
        g_fight_config.unk_4[i] = ((s8)g_fight_config.unk_4[i] + 33) % 33;
        g_fight_config.unk_4[2 + i] = ((s8)g_fight_config.unk_4[2 + i] + 8) % 8;
        g_fight_config.unk_4[4 + i] = ((s8)g_fight_config.unk_4[4 + i] + 2) % 2;
    }
    g_fight_config.unk_C = ((s8)g_fight_config.unk_C + 38) % 38;
    g_fight_config.unk_D = ((s8)g_fight_config.unk_D + 7) % 7;
    g_fight_config.unk_E &= 1;
    g_fight_config.unk_F &= 1;
    D_800A36F9 = (D_800A36F9 + 4) % 4;
    D_800A3690 &= 1;
    if (g_pad_state.pressed & 0x08000800) {
        snd_QueueSound(1, 0x7F, 0x7F);
        ApplyPendingConfigAndInitializeMode();
    }
}

/* Q65: this file's initialized small data (.sdata, 0x800A3174..0x800A31D8),
 * in address order; values from the original EXE. */
s16 D_800A3174[2] = {0, 0};
u8 D_800A3178[] = "~c777";
u8 D_800A3180[] = "~c737";
u8 D_800A3188[] = "%s%5d";
u8 D_800A3190[] = "%s%3d\n";
u8 D_800A3198[] = "%sC%4d";
u8 D_800A31A0[] = "%sCL%3x";
u8 D_800A31A8[] = "%s%3x\n";
u8 D_800A31B0[] = "%s%5d\n";
u8 D_800A31B8[] = "%sO%4d\n";
u8 D_800A31C0[] = "%sL%4d\n";
u8 D_800A31C8[] = "%sM%4d\n";
u8 D_800A31D0[] = "%sD%4d\n";
