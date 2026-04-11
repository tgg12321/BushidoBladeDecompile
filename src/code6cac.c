#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "code6cac.h"
#include "bb2_const.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* Extern data declarations */
extern s32 D_800F33D8;
extern u32 D_800A378C;
extern u32 D_80101E3C;
extern u32 D_80101E44;

/* Extern function declarations */
extern s32 func_80037110(s32);
extern void game_FrameInit(void);
extern void game_FrameLoop(void);
extern void seq_Reset(void);
extern void func_8003A39C(void);
extern void sys_VSync(s32);
extern void gpu_LoadImage(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern void game_Cleanup(void);
extern s32 func_800371E8(s16);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern s32 D_800A3894;
extern s16 D_800A38C4;
extern s16 D_80101F32;
extern void func_80035F30(s32, s32, s32, s32);
extern void obj_InitChars(void);
extern void obj_Reset(void);
extern void obj_InitTask(void);
extern void obj_InitPair(void);
extern void player_SetCharId(s32, s32);
extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
extern s32 D_80101E70;
extern s32 D_800A3894;
extern s8 D_80102781;


extern s8 D_8010277D;
extern s8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);

extern void sys_Panic(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *, s32);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern u8 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern s32 stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void func_80037774(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern u8 D_800A377C;
extern u8 D_800A37A8;
extern u16 D_800A3904;
extern u8 D_800F65F8;
extern s32 g_anim_func_table;
extern s32 g_pad_data;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s16 D_80101ED6;
extern u8 D_800F1B18[];
extern s32 g_file_disc_size;
extern s32 replay_camera_Init(s32, s32);
extern s32 func_80079154(s32);
extern void func_800325E0(s32, s32);
extern void func_80046BF4(s32 *, s32 *, s32);
extern s32 game_GetPlayerData(s32);
extern void func_8002EECC(s32, s32 *);
extern void func_80061064(s32 *, s32 *);
extern s32 func_8007E11C(s32);
extern void func_8007F87C(s32, s32);
extern void func_8007FA1C(s32, s32);
extern void func_8007FBBC(s32, s32);
extern s32 func_80053614(s32 *, s32 *, s32 *, s32 *, s32);
extern u16 D_8008D59C;
extern u8 D_800F5F68;
extern s16 Judge;
extern void func_80033BC0(void);
extern void func_8001DA2C(void);
extern void func_80055138(s32, s32, s32);
extern void func_8003FFE0(s32);
extern s32 D_80101F90;
extern s32 D_801020C0;
extern s32 D_80102114;
extern s32 camera_GetBoneData(void);
extern void func_80039320(void);
extern void func_8002C61C(void);
extern void func_80030D7C(void);
extern void func_800321E8(void);
extern void func_800397A0(void);
extern void func_8003E6A0(s32, s32);
extern void game_StageInit(s32);
extern void func_800335D8(void);
extern s32 D_80102030;

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_80017FA0(s32 *a0) {
    s32 _frame;
    s32 temp;
    s32 *ptr;

    temp = a0[3];
    if (temp == 0) {
        goto end;
    }
    ptr = (s32 *)temp;

    {
        s32 val = ptr[0] << 7;
        asm volatile("" : "=r"(val) : "0"(val));
        *(volatile s32 *)0x1F8000B8 = val;
    }

    {
        s32 i = 0;
        if (ptr[1] > 0) {
            s32 *p68 = ptr;
            volatile s32 *ac_base = (volatile s32 *)0x1F800000;
            s32 sp_off = 0;
            do {
                s32 j;
                s32 data_off;
                s32 sp_inner;

                j = 0;
                data_off = i << 5;
                sp_inner = sp_off;
                do {
                    s32 *dp = (s32 *)((u8 *)ptr + data_off);
                    *(volatile s32 *)(0x1F800064 + sp_inner) = dp[2] << 2;
                    data_off += 0x10;
                    *(volatile s32 *)(0x1F800068 + sp_inner) = dp[3] << 2;
                    j++;
                    *(volatile s32 *)(0x1F80006C + sp_inner) = dp[4] << 2;
                    sp_inner += 0xC;
                } while (j < 2);

                ac_base[0x2B] = *(s32 *)((u8 *)p68 + 0x68) << 2;
                p68 = (s32 *)((u8 *)p68 + 4);
                sp_off += 0x18;
                i++;
                ac_base++;
            } while (i < ptr[1]);
        }
    }

    *(volatile s32 *)0x1F800060 = ((s32 *)a0[3])[1];

end:
    asm volatile("" : : "m"(_frame));
}
INCLUDE_ASM("asm/funcs", marionation_camera_Exec);
/* kengo:MED  |  nm_mario_cam/marionation_camera_Exec  |  155i */
INCLUDE_ASM("asm/funcs", cpu_check_run_attack);
/* kengo:HIGH  |  nm_cpu/cpu_check_run_attack  |  322i  |  +5 near-exact */
INCLUDE_ASM("asm/funcs", single_game_setModeRequest);
/* kengo:HIGH  |  nm_single_game/single_game_setModeRequest  |  663i  |  +1 near-exact */
extern s32 g_file_data_buf;
void func_8001924C(s16 *arg0, s32 arg1) {
    s32 i;
    s16 *s0;
    s32 pad[2];
    s32 new_var;

    i = 0;
    if (arg1 <= 0) return;

    new_var = (s32)&g_file_data_buf;
    s0 = arg0;
    do {
        if (*(u8 *)((u8 *)s0 + 2) & 1) {
            s16 val = s0[0];
            func_80019310(s0, (s32 *)(val * 52 + new_var));
        } else {
            s16 val = s0[0];
            single_game_setModeRequest(s0, (s32 *)(val * 52 + new_var));
        }
        i++;
        s0 = (s16 *)((u8 *)s0 + 16);
    } while (i < arg1);
}
INCLUDE_ASM("asm/funcs", func_80019310);
void func_8001945C(void) {
    D_80106A70 = 0x11;
    D_80106A71 = 0x44;
    D_80106A72 = 0x88;
}
s32 func_80019488(void) {
    return (D_80106A70 & 0xF) | ((D_80106A71 & 0xF) << 4) | ((D_80106A72 & 0xF) << 8);
}
void func_800194C0(s32 arg0) {
    D_800A3912 = arg0 & 0xF;
    D_800A3913 = (arg0 >> 4) & 0xF;
    D_800A3914 = (arg0 >> 8) & 0xF;
}
void func_800194F4(void) {
    D_80102788 = 4;
    D_8010278A = 4;
    D_80102790 = 0;
    D_80102794 = 0;
    D_80102798 = 0;
    D_8010279C = -1;
}
void func_80019534(void) {
    func_800194F4();
    D_8010278C = 1;
    D_8010278E = 1;
}
INCLUDE_ASM("asm/funcs", single_game_VoiceContorol);
void func_8001979C(s32 arg0, u32 *arg1) {
    s32 bits_left;
    u32 base;
    s32 i;
    u32 cur;
    u32 hi;
    s32 needed;
    u32 dst;
    s32 val;

    bits_left = 0x20;
    base = (u32)&D_800F1B18[arg0 * 0x570];

    *(u32 **)base = arg1;
    cur = *arg1;
    arg1++;

    i = 0;
    dst = base;
    do {
        if (bits_left < 0xC) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 0xC - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0xA) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0xA) = (s16)(cur >> 20);
            cur <<= 0xC;
            bits_left -= 0xC;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    i = 0;
    dst = base;
    do {
        if (bits_left < 2) {
            hi = cur >> (0x20 - bits_left);
            cur = *arg1;
            arg1++;
            needed = 2 - bits_left;
            bits_left = 0x20 - needed;
            *(s16 *)(dst + 0x8E) = (s16)((hi << needed) | (cur >> bits_left));
            cur <<= needed;
        } else {
            *(s16 *)(dst + 0x8E) = (s16)(cur >> 30);
            cur <<= 2;
            bits_left -= 2;
        }
        i++;
        dst += 2;
    } while (i < 0x3F);

    val = -2;
    i = 3;
    dst = base + 0x348;
    do {
        *(s32 *)(dst + 0x110) = val;
        i--;
        dst -= 0x118;
    } while (i >= 0);
    *(s32 *)(base + 0x10C) = 0;
}
INCLUDE_ASM("asm/funcs", func_800198D0);
void func_8001A484(u16 *arg0) {
    s32 i;
    u16 *p;
    i = 0;
    p = arg0 + 2;
    do {
        i++;
        DispSleepMenuTex((s32)&D_800100A4, arg0[0], p[-1], p[0]);
        p += 3;
        arg0 += 3;
    } while (i < 0x16);
}
s32 func_8001A4F0(s32 arg0, s32 arg1) {
    s32 v = arg0 & 0xFFF;
    if (v >= 0x800) {
        v -= 0x1000;
    }
    return v / arg1;
}
void func_8001A538(s32 *arg0, s32 *arg1) {
    s16 mat[9];
    s32 pad[2];
    mat[0] = 0x1000;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = 0;
    mat[4] = 0x1000;
    mat[5] = 0;
    mat[6] = 0;
    mat[7] = 0;
    mat[8] = 0x1000;
    func_8007F87C(-*(s16 *)((u8 *)arg0 + 0x10), (s32)mat);
    func_8007FA1C(-*(s16 *)((u8 *)arg0 + 0x12), (s32)mat);
    func_8007FBBC(-*(s16 *)((u8 *)arg0 + 0x14), (s32)mat);
    arg1[0] = arg0[0] - ((s32)(mat[2] * arg0[6]) >> 12);
    arg1[1] = arg0[1] - ((s32)(mat[5] * arg0[6]) >> 12);
    arg1[2] = arg0[2] - ((s32)(mat[8] * arg0[6]) >> 12);
}
s32 func_8001A62C(s32 arg0) {
    if (arg0 < 0) {
        return -((0x7CF - arg0) / 2000);
    }
    return arg0 / 2000;
}
INCLUDE_ASM("asm/funcs", func_8001A67C);
INCLUDE_ASM("asm/funcs", func_8001A820);
void func_8001B138(s32 *arg0) {
    D_800FF5C8 = 0;
    D_800FF5CC = 0;
    D_800FF5D0 = 0;
    D_800FF5D8 = 0;
    D_800FF5DA = 0;
    D_800FF5DC = 0;
    D_800FF5E0 = 0;
    if (D_800A38BA != 0 && D_800A3834 == 1) {
        if (*arg0 & 1) {
            D_800A37E0 = 1;
            if (*arg0 & 8) {
                g_file_vram_timer = (u16)(g_file_vram_timer + 0x4CC);
            }
            if (*arg0 & 2) {
                g_file_vram_timer = (u16)(g_file_vram_timer - 0x4CC);
            }
            if ((s16)g_file_vram_timer < -0x1C00) {
                { s16 tmp = -0x1C00; g_file_vram_timer = tmp; }
            }
            if ((s16)g_file_vram_timer >= 0x7401) {
                g_file_vram_timer = 0x7400;
            }
            *arg0 = *arg0 & ~0xB;
        }
        {
            s32 v;
            v = (s16)g_file_vram_timer;
            if (v < 0) {
                v = v + 0xF;
            }
            D_800FF5E0 = v >> 4;
        }
    }
    *arg0 = *arg0 & (s32)0xFFFEFFFE;
}
void func_8001B294(s32 *a0, s32 *a1) {    s32 v0;    D_800A36FA = 0;    D_800F6638 = 0x64;    D_800F663A = 0;    D_800F663C = 0x64;    D_800F6640 = 0x64;    D_800F6642 = 0;    D_800F6644 = 0x64;    game_SetControllerPorts(0);    D_800F6608 = (*(s32 *)((u8 *)a0 + 0xF4) + *(s32 *)((u8 *)a1 + 0xF4)) / 2;    D_800F660C = (*(s32 *)((u8 *)a0 + 0xF8) + *(s32 *)((u8 *)a1 + 0xF8)) / 2;    {        s32 t1 = *(s32 *)((u8 *)a0 + 0xFC);        s32 t2 = *(s32 *)((u8 *)a1 + 0xFC);        D_800F6618 = 0;        D_800F6610 = (t1 + t2) / 2;    }    {        s32 dx = *(s32 *)((u8 *)a1 + 0xF4) - *(s32 *)((u8 *)a0 + 0xF4);        s32 dy = *(s32 *)((u8 *)a1 + 0xFC) - *(s32 *)((u8 *)a0 + 0xFC);        v0 = func_8007FD5C(dx, dy);    }    D_800F661A = 0x400 - v0;    D_800F661C = 0;    D_800F6620 = 0x1388;    D_800F6626 = 0;}
void func_8001B3C0(s32 *a0, s32 *a1) {    D_800A36FA = 0;    D_800F5358 = 0x64;    D_800F535A = 0;    D_800F535C = 0x64;    D_800F5360 = 0x64;    D_800F5362 = 0;    D_800F5364 = 0x64;    game_SetControllerPorts(0);    if (D_800A36F6 != 0) {        a0 = a1;    }    D_800F5328 = *(s32 *)((u8 *)a0 + 0x180);    D_800F5330 = *(s32 *)((u8 *)a0 + 0x188);    {        s32 v = *(s32 *)((u8 *)a0 + 0x184);        D_800F5368 = 0;        D_800F532C = v;    }}
INCLUDE_ASM("asm/funcs", myRobGeneiMove);
/* kengo:MED  |  my_eff/myRobGeneiMove  |  134i */
void func_8001B690(s32 arg0, s32 arg1) {
    if (D_800A38BA == 0) {
        return;
    }
    if (D_800A36F6 != arg0) {
        return;
    }
    arg1 &= 0xFFF;
    if (arg1 >= 0x800) {
        arg1 = 0x1000 - arg1;
    }
    if (arg1 >= 0x401) {
        D_800A36FC = 0x19;
        D_800F5344 = 0x800;
    }
}
void func_8001B6F4(void) {
    func_80041688(0, 0);
    func_80041688(1, 0);
    D_800A36FA = 1;
    D_800F6627 = 0;
    D_800F5347 = 0;
    game_SetControllerPorts(0);
}
INCLUDE_ASM("asm/funcs", DispPracticeMenuTex_A);
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
INCLUDE_ASM("asm/funcs", func_8001BAE4);
void func_8001BBD8(s32 *arg0, s32 *arg1, s32 *arg2) {
    s32 temp_s0;
    temp_s0 = (D_800A387C < 0x2711) << 0xB;
    DispPracticeMenuTex_A((s32 *)&D_800F5328, arg0, arg1, arg2, temp_s0, -0x200 - func_8007FD5C(*(s16 *)((u8 *)arg1 + 4) - *(s16 *)((u8 *)arg0 + 4), *(s16 *)((u8 *)arg1 + 8) - *(s16 *)((u8 *)arg0 + 8)));
}
void func_8001BC70(u8 *arg0, s32 arg1) {
    typedef struct { s32 x, y, z; } Vec3;
    Vec3 *dst;
    Vec3 *src;
    game_SetControllerPorts(0);
    dst = (Vec3 *)&D_800F6608;
    src = (Vec3 *)(arg0 + 0x174);
    *dst = *src;
    D_800F6618 = 0x120;
    D_800F661A = arg1;
    D_800F661C = 0;
    D_800F6620 = 0x1162;
}
void func_8001BCF0(u8 *arg0, s32 arg1) {
    typedef struct { s32 x, y, z; } Vec3;
    s32 diff = 0x1000 - arg1;

    game_SetControllerPorts(0);

    *(Vec3 *)&D_800F6608 = *(Vec3 *)(arg0 + 0xB8);

    D_800F660C -= 0x44C;

    D_800F6618 = 0x100 - (arg1 * 288) / 4096;

    {
        s32 div4 = arg1 / 4;
        s32 sum = arg1 * 3000 + diff * 8000;
        u16 lhu_val = *(u16 *)(arg0 + 0x1CA);
        s32 val;
        D_800F6620 = sum >> 12;
        val = 0xB00 - div4;
        D_800F661C = 0;
        D_800F661A = val - lhu_val;
    }
}
void func_8001BE08(s32 *arg0) {
    arg0[2] = 0;
    arg0[3] = 0;
    arg0[4] = 0;
    arg0[5] = -1;
}
INCLUDE_ASM("asm/funcs", func_8001BE20);
void func_8001C444(void) {
    D_8010277A = 0x800;
    D_80102778 = 0x800;
    D_8010277C = 1;
    D_8010277D = 0x10;
    D_80102784 = 0xC;
    D_8010277E = 0;
    D_8010277F = 0;
    D_80102781 = 0;
    D_80102780 = 0;
    D_80102785 = 4;
    D_80102786 = 0;
    D_80102787 = 0;
}
extern u16 D_80101F32;
void func_8001C4C0(void) {
    u16 v = D_80101F32;
    if (v == 0x32 || v == 0x11) {
        func_800218C8(0);
        {
            s32 v0 = func_80021974(0);
            D_80101F26 = 0;
            func_80021A98(0, v0, 0);
        }
    }
}
void func_8001C51C(void) {
    s16 *s0;
    s32 v0;
    func_8001C4C0();
    if (D_800A38DC == 3 && D_800A3728 != 0) {
        func_80022580(1, D_80102781, D_8010277D, D_8010277F, 0);
    } else {
        func_80022580(1, D_80102781, D_8010277D, D_8010277F, 1);
    }
    func_80022F34();
    func_800218C8(1);
    v0 = func_80021974(1);
    s0 = &D_80102372;
    *s0 = 0;
    func_80021A98(1, v0, 0);
    D_800A382E = 0;
    D_800A3748 = -1;
    func_80030524();
    func_80030D04();
    func_8001B294((s32)((u8 *)s0 - 0x4AA), (s32)((u8 *)s0 - 0x5E));
    func_800392C8();
    func_80021280(1);
}
INCLUDE_ASM("asm/funcs", func_8001C624);
void func_8001C820(void) {
    s16 *s0 = &D_80101ED2;
    s32 a0;
    if ((&D_8008D9EC)[*s0] != 0) {
        if (D_800A37A0 == 1) return;
    }
    if (D_800A38DC != 0) return;
    if (D_800A3712 != 0) return;
    a0 = 0x56;
    if (D_800A3680 != D_800A3671) {
        if (func_80079154(0x56) & 1) {
            a0 = 0x57;
        } else {
            a0 = 0x58;
        }
    }
    func_800325E0(a0, (s32)((u8 *)s0 + 0x536));
}
INCLUDE_ASM("asm/funcs", func_8001C8DC);
void func_8001CD68(s16 *arg0) {
    s32 val = D_800A3858;

    if (val > 0x2BF1F) {
        *(s16 *)arg0 = 99;
        *((u8 *)arg0 + 2) = 59;
        *((u8 *)arg0 + 3) = 99;
        return;
    }
    {
        s32 minutes = val / 1800;
        s32 seconds = val / 30 - minutes * 60;
        *((u8 *)arg0 + 2) = seconds;
        {
            s32 frames;
            s32 centiseconds = (D_800A3858 % 30) * 100 / 30;
            *(s16 *)arg0 = minutes;
            *((u8 *)arg0 + 3) = centiseconds;
        }
    }
}
INCLUDE_ASM("asm/funcs", camera_set_target_zoom);
/* kengo:MED  |  nm_camera/camera_set_target_zoom  |  593i  |  +5 */
extern s8 D_800A30FC;
extern s8 D_800A30FD;
extern s32 D_800FF6A8;
void se_data_set(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;

    gpu_EnableDisplay();

    if (D_800A36A4 != D_800A390E
        || *(&D_8008E5A8 + (s8)D_8010277C) != D_800A30FC
        || *(&D_8008E5A8 + D_8010277D) != D_800A30FD) {
        s8 *p = (s8 *)&D_8010277C;

        func_80020D38();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + *p), *(&D_8008E5A8 + D_8010277D));

        D_800A390E = (s8)(u16)D_800A36A4;
        D_800A30FC = *(&D_8008E5A8 + *p);
        D_800A30FD = *(&D_8008E5A8 + D_8010277D);

        if (s1 >= 0x2519) {
            sys_Panic();
        }

        s0 = &D_800FF6A8;
        bb2_memcpy(s0, s2, s1);
        func_8005BD30((s32)s0 - s2);
    }
}
/* kengo:HIGH  |  md_game/se_data_set  |  93i */
void func_8001D904(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_EnableDisplay();
    func_80020D38();
    obj_InitTask();
    s1 = obj_InitTaskCamera((s32)0x80190800);
    if (s1 >= 0xE81) {
        sys_Panic();
    }
    s0 = &D_80104F38;
    bb2_memcpy(s0, (s32)0x80190800, s1);
    obj_ExecTask((s32)s0 - s2);
}
void func_8001D998(void) {
    s32 s2 = (s32)0x80190800;
    s32 s1;
    s32 *s0;
    gpu_EnableDisplay();
    func_80020D38();
    obj_InitPair();
    s1 = func_8005B8B8((s32)0x80190800);
    if (s1 >= 0x1B19) {
        sys_Panic();
    }
    s0 = &D_80104F38;
    bb2_memcpy(s0, (s32)0x80190800, s1);
    func_8005B98C((s32)s0 - s2);
}
void func_8001DA2C(void) {
    obj_InitChars();
    obj_Reset();
    if (D_800A38DC == 5) {
        obj_InitTask();
    }
    if (D_800A38DC == 3) {
        obj_InitPair();
    }
}
INCLUDE_ASM("asm/funcs", func_8001DA8C);
s32 func_8001DB58(void) {
    s32 v = D_800A38DC;
    if (v >= 5) {
        return 1;
    }
    if (v >= 2) {
        return file_GetFlag2();
    }
    return 1;
}
void func_8001DB9C(void) {
    seq_Start((&D_8008D9EC)[D_80101ED2] < 1, (s32)0x80190800);
    D_800A38C6 = (u16)0xFFFF;
}
void func_8001DBE4(void) {
    s32 i;

    if (g_disp_enable != DISP_ACTIVE) {
        return;
    }
    func_8003AA78();
    if (D_800A38F8 > D_800A37A0) {
    } else {
        do {
            func_8003AA48();
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (!(D_800A38F8 > D_800A37A0));
        i = 0;
        do {
            func_8003AA48();
            i += 1;
            gnd_disp_loop_ctrl();
            sys_VSync(2);
        } while (i < 15);
    }
    func_8003AAB0();
    gpu_InitDisplay();
    gpu_DisableDisplay();
}
INCLUDE_ASM("asm/funcs", mario_test_Exec);
/* kengo:MED  |  nm_mario_test/mario_test_Exec  |  450i  |  -19 */
INCLUDE_ASM("asm/funcs", func_8001E404);
typedef struct {
    s32 vx, vy, vz;
    s32 pad0;
    s16 rx, ry, rz;
    s16 pad1;
    s32 dist;
    s32 pad2[11];
} CamWork;

void func_8001E6E4(s32 arg0) {
    s32 pre_pad[2];
    CamWork local;
    s32 *s2;

    s2 = (s32 *)&D_800F5328;
    if ((u32)(arg0 - 0x555) >= 0x556U) {
        s2 = (s32 *)&D_800F6608;
    }

    local.vx = s2[0] + D_800FF5C8;
    local.vy = s2[1] + D_800FF5CC;
    local.vz = s2[2] + D_800FF5D0;
    local.rx = *(u16 *)((u8 *)s2 + 0x10) + (u16)D_800FF5D8;
    local.ry = *(u16 *)((u8 *)s2 + 0x12) + (u16)D_800FF5DA;
    local.rz = *(u16 *)((u8 *)s2 + 0x14) + (u16)D_800FF5DC;

    local.dist = *(s32 *)((u8 *)s2 + 0x18) + D_800FF5E0;
    func_80046BF4((s32 *)&local, &local.rx, local.dist);

    {
        s32 *p20 = (s32 *)((u8 *)s2 + 0x20);
        func_8001A538((s32 *)&local, p20);
        func_80061064((s32 *)&local.rx, p20);
    }

    D_800A36B4 = (s32)s2;
}
void func_8001E800(void) {
    s32 v = D_800A36F6;
    volatile u8 *ptr = (volatile u8 *)(&D_80101EC8 + v * 1100);
    s32 a1 = -1;
    if (ptr[0x62] & 1) {
        a1 = *(s16 *)(ptr + 0xE);
    }
    {
        u32 flags = ptr[0x62] & 4;
        func_80048BA4(D_800F5344, a1, flags > 0);
    }
}
void func_8001E878(void) {
    s32 buf[6];
    s32 v0;
    s32 *a0 = &D_80102030;
    u8 *s0;
    v0 = camera_GetBoneData();
    s0 = (u8 *)a0 - 0x168;
    D_800A3778 = v0;
    func_8001A820((s32)a0, (s32)((u8 *)a0 + 0x44C), (s32)s0, (s32)((u8 *)a0 + 0x2E4));
    if (D_800A38BA != 0) {
        myRobGeneiMove((s32)(s0 + D_800A36F6 * 1100));
    }
    func_8001E404();
    func_80039320();
    func_8002006C();
    func_8001BE20(0, (s32)buf);
    func_80023F08(0, (s32)buf);
    func_8001BE20(1, (s32)buf);
    func_80023F08(1, (s32)buf);
    func_8002C61C();
    func_80030D7C();
    func_800321E8();
    func_800397A0();
    if (D_800A38BA != 0 && D_800A36FA == 0) {
        func_8001E800();
    } else {
        func_8003E6A0(D_80101FBC, D_80101FC4);
        func_8003E6A0(D_80102408, D_80102410);
    }
    game_StageInit((D_800A3690 ^ 1) != 0);
    camera_set_target_zoom();
    func_800335D8();
    func_8001C8DC();
}
void func_8001EA04(void) {
    u8 v;
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    v = D_800A38D4;
    D_8010262E = 0;
    D_801021E2 = 0;
    D_800A37B8 = 0;
    D_800A3929 = 0;
    D_800A3834 = 0xD;
    D_800A3804 = v < 1;
    D_800A3817 = v < 1;
}
INCLUDE_ASM("asm/funcs", cpu_get_move_pattern_table_number);
/* kengo:HIGH  |  nm_cpu/cpu_get_move_pattern_table_number  |  265i  |  -3 near-exact */
void func_8001EEB4(void) {
    s8 idx = D_800A3748;
    u8 *entry = (u8 *)&D_80101EC8 + idx * 0x44C;
    u16 a1 = *(u16 *)(entry + 0x6A);

    if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
        (u32)((s32)a1 - 0x17) >= 2 && *(s16 *)(entry + 0x96) == 0) {
        func_800218C8(D_800A3748);
        {
            s32 ret = func_80021A3C(D_800A3748, *(s16 *)(entry + 0xA));
            *(s16 *)(entry + 0x5E) = 1;
            func_80021A98(D_800A3748, ret, 1);
        }
        *(s16 *)(entry + 0x26C) = 1;
    }

    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x11;
}
INCLUDE_ASM("asm/funcs", func_8001EFA0);
void func_8001F1C4(u8 *arg0, u8 *arg1, u8 *arg2, u8 *arg3) {
    s16 temp_v1;
    if (!(*(u8 *)(arg1 + 0x18) & 0x80)) {
        func_80027334((s32 *)arg2);
        func_80027334((s32 *)arg3);
    }
    func_8002F770((s32 *)(arg2 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    func_8002F770((s32 *)(arg3 + 0x36), *(s8 *)(arg1 + 0x14) * 4, *(s8 *)(arg1 + 0x15) * 4, 0);
    temp_v1 = *(s16 *)(arg0 + 0xC);
    if ((temp_v1 == 0x1D) || (temp_v1 == 0xE)) {
        *(u16 *)(arg2 + 0x7E) = (u16)(*(u16 *)(arg2 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x7E) = (u16)(*(u16 *)(arg3 + 0x7E) + (*(s8 *)(arg1 + 0x16) * 4));
    }
    if ((u32)(*(u16 *)(arg0 + 0xE) - 6) < 2U) {
        *(u16 *)(arg2 + 0x72) = (u16)(*(u16 *)(arg2 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
        *(u16 *)(arg3 + 0x72) = (u16)(*(u16 *)(arg3 + 0x72) + (*(s8 *)(arg1 + 0x16) * 4));
    }
}
INCLUDE_ASM("asm/funcs", md_game_rob_data_init);
/* kengo:HIGH  |  md_game/md_game_rob_data_init  |  351i */
void func_8001F860(s16 *arg0, s32 arg1) {
    arg1 = (arg1 - *(s16 *)((u8 *)arg0 + 0x1CA)) & 0xFFF;
    if (arg1 >= 0x800) {
        arg1 -= 0x1000;
    }
    *(s16 *)((u8 *)arg0 + 0x14C) = arg1;
}
s32 func_8001F888(void) {
    s32 dx = D_80102408 - D_80101FBC;
    s32 dy = D_80102410 - D_80101FC4;
    s32 s0 = 0;
    while ((u32)(dx + 0x4000) > 0x8000 || (u32)(dy + 0x4000) > 0x8000) {
        s32 t;
        t = dx + ((u32)dx >> 31);
        dx = (s32)t >> 1;
        t = dy + ((u32)dy >> 31);
        dy = (s32)t >> 1;
        s0 += 1;
    }
    {
        s32 v0 = dx * dx;
        s32 v1 = dy * dy;
        s32 r = func_8007E11C(v0 + v1);
        return r << s0;
    }
}
INCLUDE_ASM("asm/funcs", func_8001F938);
s32 func_8001FAE4(s32 *arg0) {
    s32 v0;
    u16 v1;
    s32 *a0;
    char new_var;

    v1 = *(u16 *)((s32)arg0 + 0xA);
    a0 = (s32 *)((s32)arg0 + 0xA);
    if (v1 == 0) goto ret_zero;
    v0 = v1 & 0x4000;
loop:
    if (v0 != 0) {
        v0 = (s32)a0;
        goto end;
    }
    new_var = 0;
    if ((v1 & 0xC000) != new_var) {
        a0 = (s32 *)((s32)a0 + 8);
    } else {
        a0 = (s32 *)((s32)a0 + 4);
    }
    v1 = *(u16 *)a0;
    v0 = v1 & 0x4000;
    if (v1 != new_var) goto loop;
ret_zero:
    v0 = 0;
end:
    return v0;
}
s32 func_8001FB34(s32 *arg0, s32 arg1) {
    s16 v1;
    s32 v0;
    v1 = D_800A38DC;
    if (v1 == 2) return 0;
    if (v1 == 5) return 0;
    if (v1 == 3) return 0;
    if (v1 != 0) goto check2;
    if (D_800A385C != 0) return 0;
check2:
    v0 = *(s32 *)arg0;
    v1 = *(s16 *)(v0 + 0xC);
    if (v1 == 0xD) return 0;
    if (v1 == 0x1C) return 0;
    v1 = *(s16 *)((u8 *)arg0 + 0xA);
    if (v1 != 0xE) goto check3;
    if (*(s16 *)((u8 *)arg0 + 0x330) == 0) return 0;
    v1 = *(s16 *)((u8 *)arg0 + 0x332);
    if (v1 == 0xA) goto check3;
    return 0;
check3:
    v0 = 1;
    if (arg1 != 0) {
        v0 = *(s16 *)((u8 *)arg0 + 0x26C);
        v0 = (u32)0 < (u32)v0;
    }
    return v0;
}
INCLUDE_ASM("asm/funcs", single_game_CheckStatusUpDataTotalOver);
/* kengo:HIGH  |  nm_single_game/single_game_CheckStatusUpDataTotalOver  |  289i */
s32 func_8002006C(void) {
    s32 s0 = D_800A387C;
    s32 v0 = func_8001F888();
    s32 v = D_800A38DC;
    D_800A387C = v0;
    D_800A38F0 = v0 - s0;
    if (v != 5 && v != 2) {
        single_game_CheckStatusUpDataTotalOver();
    }
    D_800A38A8 = 0;
}
INCLUDE_ASM("asm/funcs", func_800200DC);
INCLUDE_ASM("asm/funcs", func_800203B4);
INCLUDE_ASM("asm/funcs", single_game_SetAbilityData);
/* kengo:HIGH  |  nm_single_game/single_game_SetAbilityData  |  124i */
void func_800206B0(s32 arg0, s32 arg1) {
    u8 *a3 = (u8 *)&D_8008D59C;
    u8 *a2 = (u8 *)&D_800F5F68 + arg0 * 0x1B8;
    s32 t0 = 0;
    u8 *a0 = a2 + 0x12;
    u8 *v1 = a3 + 0x12;

loop:
    *(u16 *)a2 = *(u16 *)a3;
    *(u16 *)(a0 - 0x10) = *(u16 *)(v1 - 0x10);
    *(s16 *)(a0 - 0xE) = (s32)(*(s16 *)(v1 - 0xE) * arg1) >> 0xC;
    *(s16 *)(a0 - 0xC) = (s32)(*(s16 *)(v1 - 0xC) * arg1) >> 0xC;
    *(s16 *)(a0 - 0xA) = (s32)(*(s16 *)(v1 - 0xA) * arg1) >> 0xC;
    *(s16 *)(a0 - 6) = (s32)(*(u16 *)(v1 - 6) * arg1) >> 0xC;
    *(s16 *)(a0 - 4) = (s32)(*(u16 *)(v1 - 4) * arg1) >> 0xC;
    *(s16 *)(a0 - 2) = (s32)(*(u16 *)(v1 - 2) * arg1) >> 0xC;
    {
        s32 temp_lo = *(u16 *)v1 * arg1;
        t0 += 1;
        a3 += 0x14;
        a2 += 0x14;
        v1 += 0x14;
        *(s16 *)a0 = temp_lo >> 0xC;
        a0 += 0x14;
    }
    if (t0 < 0x16) goto loop;
}
INCLUDE_ASM("asm/funcs", func_800207C8);
void func_80020CDC(void) {
    if (D_800A38C6 == 0xFFFF) {
        seq_Reset();
    }
    D_800A3880 = 0;
    D_800A38C6 = 0;
    D_800A38C4 = 0;
    D_800A38C1 = 0xFF;
    D_800A38C0 = 0xFF;
}
void func_80020D38(void) {
    if (D_800A38C6 == 0xFFFF) {
        seq_Reset();
    }
    D_800A38C6 = 0;
}

void func_80020D70(void) {
    D_800A3888 = (s32)0x80118800;
    D_800A388C = (s32)0x8011C400;
    D_800A3830 = (s32)0x80120000;
    D_800A3860 = (s32)0x80148800;
    D_800A3864 = (s32)0x80190800;
    func_80020CDC();
}
void func_80020DDC(void) {    s32 v0;    s32 v1;    s32 v2;    v0 = func_80036EA8(1, 1);    replay_camera_Init(v0, D_800A3830);    game_FrameLoop();    v1 = D_800A3830;    D_80102760 = v1 + 0x14;    D_80102764 = v1 + *(s32 *)(v1 + 4);    D_80102768 = v1 + *(s32 *)(v1 + 8);    v2 = *(s32 *)(v1 + 0x10);    D_800A3880 = 1;    D_80102770 = v1 + v2;}
INCLUDE_ASM("asm/funcs", DispPracticeMenuTex_B);
/* kengo:LOW  |  su_menu_tuto/_DispPracticeMenuTex  |  231i  |  PS2 UI — size coincidence, different stack frames */
extern u16 D_800A38C4;
void func_80021210(void) {
    func_8001979C(0, D_80102770);
    if (D_800A38C4) {
        func_8001979C(1, D_801027C0);
    }
    if (D_800A38C6) {
        func_8001979C(2, D_801027D4);
    }
}
void func_80021280(s32 a0) {
    s32 a1 = 0;
    u8 *a2 = (u8 *)&D_80101EC8 + a0 * 1100;
    u16 a3 = *(u16 *)(a2 + 0x48);
    u16 *v1 = (u16 *)&D_800A38C4;

loop1_21280:
    if (a3 == *v1) goto done1_21280;
    a1++;
    v1++;
    if (a1 < 2) goto loop1_21280;
done1_21280:

    {
        u16 val = *(u16 *)(a2 + 0x48);
        *(s16 *)(a2 + 0x4A) = a1;
        *(s16 *)(a2 + 0x4C) = 0;

        if ((u32)(val >> 12) < 2) {
            s32 i = 0;
            u16 t1 = val;
            s32 t4 = 4;
            s32 t3 = 3;
            s32 t2 = 1;
            s32 mode = D_800A38DC;
            u8 t0 = D_800A384C;

        loop2_21280:
            {
                u16 nibble = (t1 >> (i << 2)) & 0xF;
                if (nibble != t4) goto not4_21280;
                *(s16 *)(a2 + 0x88) = i;
                if (mode != t3) goto store4_21280;
                if (a0 != t2) goto store4_21280;
                if (t0 != nibble) goto next_21280;
            store4_21280:
                *(u16 *)(a2 + 0x8A) = *(u16 *)(a2 + 0x26C);
                goto next_21280;
            not4_21280:
                if (nibble != 5) goto next_21280;
                *(s16 *)(a2 + 0x8E) = i;
                if (mode != 0) goto store5_21280;
                if (D_800A385C == 0) goto store5_21280;
                if (a0 == 0) goto next_21280;
            store5_21280:
                *(u16 *)(a2 + 0x90) = *(u16 *)(a2 + 0x26C);
            }
        next_21280:
            i++;
            if (i < 3) goto loop2_21280;
        }
    }
}
void func_800213A0(s16 *arg0) {
    s16 a1 = arg0[0x86 / 2];
    if (a1 != arg0[0x88 / 2]) {
        if (a1 != arg0[0x8E / 2]) {
            return;
        }
    }
    {
        s16 *v = (s16 *)(&D_800A3860)[arg0[0x4A / 2]];
        arg0[0x86 / 2] = (s16)((a1 + 1) % v[0x14 / 2]);
    }
}
INCLUDE_ASM("asm/funcs", func_80021424);
void func_800218C8(s32 a0) {
    s32 offset = a0 * 1100;
    *(u16 *)((u8 *)&D_80101F4E + offset) = *(u16 *)((u8 *)&D_80101F4C + offset);
}
s32 func_80021904(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4E + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
s32 func_80021974(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4C + offset);
    s32 a0_2 = v1 * 4;
    s32 v1_2 = (a0_2 + v1) * 4;
    s32 base = *(s32 *)((u8 *)&D_800A3860 + a0_2);
    u16 idx = *(u16 *)(base + v0 * 2 + 0x4E);
    s32 tbl = *(s32 *)((u8 *)&D_801027B0 + v1_2);
    return tbl + idx * 2;
}
s32 func_800219E4(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + 0x16);
    return D_80102760 + idx * 2;
}
s32 func_80021A3C(s32 a0, s32 a1) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + a1 * 2 + 0x18);
    return D_80102760 + idx * 2;
}
INCLUDE_ASM("asm/funcs", func_80021A98);
void func_80021D10(s32 arg0, s32 *arg1, s32 arg2) {
    s16 *temp_v0;
    temp_v0 = (s16 *)(stage_GetDataPtr() + (((D_800A36A4 * 0x18) + (arg2 * 6) + (arg0 * 3)) * 2));
    arg1[0] = (s32)temp_v0[0];
    arg1[1] = (s32)temp_v0[1];
    arg1[2] = (s32)temp_v0[2];
}
INCLUDE_ASM("asm/funcs", func_80021DB0);
INCLUDE_ASM("asm/funcs", func_80022224);
s32 func_80022408(s32 *arg0) {
    s16 *p;
    s32 i;
    s32 best_dist;
    s32 best;
    s32 t1;
    s32 t2;
    int new_var;
    s32 dx;
    s32 dz;
    s32 dist;
    p = (s16 *)stage_GetDataPtr();
    best_dist = 0x7FFFFFFF;
    i = 0;
    t1 = arg0[0];
    t2 = arg0[2];
    p = p + ((D_800A36A4 * 3) * 8);
loop:
    dx = ((p[0] + p[3]) / 2) - t1;
    new_var = dx * dx;
    dist = new_var;
    dz = ((p[2] + p[5]) / 2) - t2;
    dist = dist + (dz * dz);
    if (dist < best_dist) {
        best_dist = dist;
        best = i;
    }
    i++;
    if (i < 4) {
        p += 6;
        goto loop;
    }
    return best;
}
s32 func_800224E0(s32 *arg0) {
    u8 *new_var2;
    u8 *p;
    u8 *end;
    u8 *base;
    u32 pad;
    s32 val;
    s32 i;
    s32 *new_var;
    s32 *ptr;

    p = (&D_8008EB1C) + (D_800A384C * 2);
    new_var2 = &D_8008DB1C;
    new_var = (s32 *)(*arg0);
    if ((!i) && (!i) && (!i)) {
    }
    end = p + 2;
    ptr = new_var;
    base = new_var2 + (*(s16 *)((u8 *)ptr + 0xA) * 16);
    val = *(u16 *)(base + *(s16 *)((u8 *)ptr + 0xE) * 2);
loop:
    i = 0;
    do {
        if (*p == ((val >> (i * 4)) & 0xF)) {
            return i;
        }
        i++;
        if ((val && val) && val) {
        }
    } while (i < 3);
    p++;
    if ((s32)p < (s32)end) {
        goto loop;
    }
    return 0;
}
void func_80022568(s16 *arg0) {
    arg0[0x136] = 1;
    arg0[0x137] = 0;
    arg0[0x138] = 0;
    arg0[0x139] = 0;
}
INCLUDE_ASM("asm/funcs", func_80022580);
INCLUDE_ASM("asm/funcs", func_80022F34);
INCLUDE_ASM("asm/funcs", func_8002304C);
INCLUDE_ASM("asm/funcs", func_800233AC);
INCLUDE_ASM("asm/funcs", func_80023648);
INCLUDE_ASM("asm/funcs", camera_set_zoom);
/* kengo:HIGH  |  nm_camera/camera_set_zoom  |  219i */
void func_80023C30(s32 arg0, s32 arg1, s32 arg2, s16 *arg3) {
    arg3[0] = 0x1000;
    arg3[1] = 0;
    arg3[2] = 0;
    arg3[3] = 0;
    arg3[4] = 0x1000;
    arg3[5] = 0;
    arg3[6] = 0;
    arg3[7] = 0;
    arg3[8] = 0x1000;
    func_8007F87C(arg0, (s32)arg3);
    func_8007FA1C(arg1, (s32)arg3);
    func_8007FBBC(arg2, (s32)arg3);
}
void func_80023CB4(s16 *arg0, s16 arg1) {
    s16 v;
    *(u16 *)((u8 *)arg0 + 0x31A) += 1;
    v = *(s16 *)((u8 *)arg0 + 0x31A);
    if (v == 1) {
        *(s16 *)((u8 *)arg0 + 0x318) = arg1;
        *(s32 *)((u8 *)arg0 + 0x320) = 0;
        *(s32 *)((u8 *)arg0 + 0x328) = 0;
    }
    *(s16 *)((u8 *)arg0 + 0x31C) = arg1;
    if (*(s16 *)((u8 *)arg0 + 0x152) == 0) {
        *(s16 *)((u8 *)arg0 + 0x152) = 1;
        *(s16 *)((u8 *)arg0 + 0x154) = *(u16 *)((u8 *)arg0 + 0x1D8);
    }
}
void func_80023D08(s32 arg0) {
    func_80023CB4(arg0, 0x200);
}
void func_80023D28(u8 *arg0) {
    if (*(s32 *)(arg0 + 0x108) < 0) {
        *(s16 *)(arg0 + 0x1DC) = 0;
        return;
    }
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 0x1F4;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    *(s16 *)(arg0 + 0x1DC) = func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8);
}
s32 func_80023DB8(u8 *arg0) {
    *(s32 *)0x1F8001E0 = *(s32 *)(arg0 + 0xB8);
    *(s32 *)0x1F8001E4 = *(s32 *)(arg0 + 0xBC) + 5;
    *(s32 *)0x1F8001E8 = *(s32 *)(arg0 + 0xC0);
    if (func_8005344C((s32 *)(arg0 + 0xB8), (s32 *)0x1F8001E0, (s32 *)0x1F8001B0, (s32 *)0x1F8001C0, 0x1F8002B8) != 0) {
        if ((arg0 && arg0) && arg0) {
        }
        return *(s16 *)0x1F8001C2 < -0x800;
    }
    return 0;
}
void func_80023E40(u8 *arg0) {
    s32 *s1 = (s32 *)0x1F8001B0;
    s32 a0;
    s32 v1;
    a0 = *(u16 *)(arg0 + 0x6A);
    v1 = a0 & 0xFFFF;
    if (v1 == 8) goto done;
    if (v1 == 0x22) goto done;
    if ((u32)(a0 - 0x17) < 2 || v1 == 0x28 || v1 == 0xA) {
        s32 v0;
        s32 *v3 = (s32 *)0x1F8002B8;
        s1[0xC] = *(s32 *)(arg0 + 0xB8);
        s1[0xD] = *(s32 *)(arg0 + 0xBC) + 0x1F40;
        s1[0xE] = *(s32 *)(arg0 + 0xC0);
        v0 = func_80053614((s32 *)(arg0 + 0xB8), &s1[0xC], s1, &s1[4], (s32)v3);
        if (v0 == 0) goto done;
        *(s32 *)(arg0 + 0x148) = s1[1];
        goto done;
    }
    *(s32 *)(arg0 + 0x148) = *(s32 *)(arg0 + 0xBC);
done:;
}
INCLUDE_ASM("asm/funcs", func_80023F08);
