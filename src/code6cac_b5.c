/* The CD module's two state-machine steppers, func_80036140 and func_80036940,
 * moved out of code6cac_b4_post.c together into their own translation unit
 * compiled -G8 (Makefile GP_FILES; owner ruling 2026-09-26, Q10): both read
 * g_cd_result straight off $gp, which the original compiler emits only at -G8.
 * func_80036140 is INCLUDE_ASM again (2026-09-30): its C matched only through the
 * per-function maspsx COMMON gate, which the owner ruled a cheat. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"

extern void VSync(s32);
extern s32 CdPosToInt(s32);
extern void cdrom_ReadyCallback(u8 arg0);

extern void CdMix(CdlATV *);
extern s16 D_800A3854;
extern s16 D_800A3840;
extern void cdrom_SetMix(s32, s32, s32, s32);
extern s32 CdSync(s32, u8 *);
extern s32 CdReady(s32, u8 *);
/* func_80036140's jump table (func_80036140 is INCLUDE_ASM again, so the table is
 * transcribed; its last word is the zero word at 0x80010974 before
 * func_80036940's compiler-emitted table). */
const u32 jtbl_80010938[16] = {
    0x80036360, 0x800363A4, 0x800363DC, 0x80036434,
    0x80036490, 0x800364C0, 0x80036634, 0x8003683C,
    0x8003686C, 0x80036880, 0x800368CC, 0x800368E0,
    0x800367B0, 0x800367D8, 0x80036808, 0x00000000,
};
INCLUDE_ASM("asm/funcs", func_80036140);
/* kengo:MED  |  nm_special_cam/special_camera_set_win_cam  |  502i  |  -10 */
void func_80036940(void);
extern s32 CdSync(s32, u8 *);
extern void CdControl(s32, s32, s32);
extern void func_80036140(void);
void func_80036940(void) {
    u8 param[4];

    if (D_80101E58.rec.unk02 >= 0x10) {
        func_80036140();
        return;
    }
    switch (D_80101E58.rec.unk02) {
    case 0:
        break;
    case 2:
        if (D_80101E58.rec.unk08 != 0) {
            D_80101E58.rec.unk02 = 0;
            break;
        }
        param[0] = 0xA0;
        CdControlF(0xE, (s32)param);
        D_80101E58.rec.unk06 = 0;
        D_80101E58.rec.unk38 = 0;
        D_80101E58.rec.unk02 = 3;
        break;
    case 3: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = 4;
            D_80101E58.rec.unk2C = 0;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 4:
        if (++D_80101E58.rec.unk2C >= 3) {
            D_80101E58.rec.dest_buffer = D_80101E58.rec.unk1C;
            D_80101E58.rec.sectors_remaining = D_80101E58.rec.unk18;
            D_80101E58.rec.expected_pos = CdPosToInt((s32)&D_80101E58.rec.pair);
            CdControl(2, (s32)&D_80101E58.rec.pair, 0);
            D_80101E58.rec.unk38 = 0;
            D_80101E58.rec.unk02 = 5;
        }
        break;
    case 5: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk38 = 0;
            CdReadyCallback((s32)cdrom_ReadyCallback);
            CdControlF(6, (s32)&D_80101E58.rec.pair);
            D_80101E58.rec.unk02 = 6;
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 6: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (D_80101E58.rec.sectors_remaining == 0) {
                D_80101E58.rec.unk02 = 8;
            } else if (D_80101E58.rec.sectors_remaining < 0) {
                D_80101E58.rec.unk02 = 9;
            } else if (++D_80101E58.rec.unk38 > 0x3C) {
                CdReadyCallback(0);
                D_80101E58.rec.unk02 = 0xA;
            }
        } else if (ret == 5) {
            CdReadyCallback(0);
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.rec.unk38 > 0x3C) {
            CdReadyCallback(0);
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 8:
        D_80101E58.rec.unk02 = 0;
        break;
    case 9:
        if (g_cd_result[0] & 0x10) {
            D_80101E58.rec.unk02 = 0xA;
        } else {
            D_80101E58.rec.unk02 = 0xC;
        }
        break;
    case 0xA:
        CdControlF(1, 0);
        D_80101E58.rec.unk02 = 0xB;
        D_80101E58.unk04 = 0;
        break;
    case 0xB: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            if (g_cd_result[0] & 0x10) {
                D_80101E58.rec.unk02 = 0xA;
            } else {
                D_80101E58.rec.unk02 = 0xC;
            }
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 0xA;
        } else if (++D_80101E58.unk04 > 0xA) {
            CdFlush();
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    case 0xC:
        CdControlF(0x13, 0);
        D_80101E58.rec.unk02 = 0xD;
        D_80101E58.unk04 = 0;
        break;
    case 0xD: {
        s32 ret = CdSync(1, g_cd_result);
        if (ret == 2) {
            D_80101E58.rec.unk02 = 2;
            VSync(4);
            VSync(4);
            VSync(4);
            VSync(4);
        } else if (ret == 5) {
            D_80101E58.rec.unk02 = 9;
        } else if (++D_80101E58.unk04 > 0x1E) {
            CdFlush();
            D_80101E58.rec.unk02 = 0xA;
        }
        break;
    }
    }
}
/* kengo:HIGH  |  nm_special_cam/special_camera_Exec  |  274i */
