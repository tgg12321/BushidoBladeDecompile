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
extern s32 D_800F33D8;
extern u32 D_800A378C;
extern u32 D_80101E3C;
extern u32 D_80101E44;

/* Extern function declarations */
extern void func_80023CB4(s32, s32);
extern s32 func_80037110(s32);
extern void game_FrameInit(void);
extern void game_FrameLoop(void);
extern void func_800194F4(void);
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
extern u32 D_800A3D30;
extern s16 D_800A38C4;
extern s16 D_80101F32;
extern void func_80035F30(s32, s32, s32, s32);
extern void obj_InitChars(void);
extern void obj_Reset(void);
extern void obj_InitTask(void);
extern void obj_InitPair(void);
extern void player_SetCharId(s32, s32);
extern void func_80021974(s32);
extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
extern s32 D_80101E70;
extern s32 D_800A3894;
extern u8 D_80102781;
extern s8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 *func_8004153C(s32);
extern void func_800432A0(s32, s32, s32, s32, s32);
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);
extern void md_menu_logo_exec(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern s32 func_80036D88(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void initLoadImage(u32 *, s16 *, s32, s32);
extern s32 game_GetPlayerCount(void);
extern s32 func_80052C28(s32, s32);
extern s32 func_800788B0(void);
extern void func_800372C0(void);
extern void func_800548DC(void);
extern s32 func_8005FC9C(s32, s32);
extern s32 func_80054F68(void);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void func_8003B5A4(void);
extern s32 func_8005E54C(s32, s32, s32);
extern void func_8005C650(s32, s32, s32);
extern s32 *func_80077D00(void);
extern void func_80060758(void);
extern void func_8001CD68(u8 *);
extern void func_80046BF4(s16 *, s32, s32);
extern void game_StageInit(s32);
extern s32 func_800600C8(s32, s32, s32);
extern void func_8001DA2C(void);
extern void func_8003B10C(s32);
extern void func_8005FBC8(s32, s32);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_8001DBE4(void);
extern void func_80041BF4(s32, s32, s32);
extern void func_8004659C(s32);
extern s32 func_8005C8A8(s32, s32, s32, s32);
extern s32 func_8005FA98(s32, s32, s32);
extern void func_800342A0(void);
extern s32 func_80022408(s32 *);
extern void gpu_StoreImage(s32 *, u16 *);
extern void func_80052BE4(u8 *);
extern void func_8003F388(s16 *);
extern void func_80037774(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
extern s8 D_8008EA70;
extern s32 D_8009060C;
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
extern s32 g_file_disc_size;
extern s32 D_80106A58;
extern u8 D_800A3768;
extern u8 D_800A36A8;
extern u8 D_800A376C;
extern u8 D_800A37B4;
extern u8 D_800A37B5;
extern u8 D_800A37B6;
extern s32 D_800A37B8;
extern u8 D_800A390F;
extern s16 D_800A3834;
extern s16 D_80101EDA;
extern s16 D_80102326;
extern void gpu_InitDisplay(void);
extern void gpu_DisableDisplay(void);
extern void gpu_EnableDisplay(void);
extern void md_menu_logo_exec(void);
extern void func_80020CDC(void);
extern void func_80020D38(void);
extern void func_80041688(s32, s32);
extern void func_8004659C(s32);
extern void func_80035FA8(void);
extern s32 func_80036EA8(s32, s32);
extern void mottest_rob_init(s32, s32);
extern void func_80037260(void);
extern void saTan4FireDisp(s32, s32, s32);
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

void func_8003B9D0(void) {
    s32 saved_first;
    s32 saved_44c;
    s32 a3_arg;
    s32 a0_arg;
    s32 magic;
    s32 v0;
    u8 *p;
    u8 flags;

    magic = 0x80190800;
    func_8001DA2C();
    game_Cleanup();
    if (D_800A3768 != 0x14) gpu_InitDisplay();
    if (D_800A3768 != 0xFF) gpu_DisableDisplay();
    gnd_disp_loop_ctrl();
    gpu_EnableDisplay();
    func_80020D38();
    disp_SetFramebufferMode(1, 0, 0, 0);
    if (((u8 *)D_800A3878)[3] & 0x80) {
        func_80020CDC();
        magic = 0x80118800;
    }
    {
        u8 *q = (u8 *)D_800A3878;
        u8 qf = q[3];
        if (qf & 0x30) {
            s16 *eda = &D_80101EDA;
            __asm__ __volatile__("" : "=r"(eda) : "0"(eda));
            saved_first = eda[0];
            saved_44c = eda[0x226];
            if (qf & 0x10) eda[0] = 0x32;
            if (q[3] & 0x20) eda[0x226] = 0x32;
            md_menu_logo_exec();
            eda[0] = saved_first;
            eda[0x226] = saved_44c;
        }
    }
    a3_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA;
    __asm__ __volatile__("" ::: "memory");
    a0_arg = -1;
    if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326;
    __asm__ __volatile__("" ::: "memory");
    p = (u8 *)D_800A3878;
    flags = p[3];
    if (flags & 0x10) a3_arg = 0x32;
    if (flags & 0x20) a0_arg = 0x32;
    D_800A390F = 0;
    func_80054884(D_800A376C, p[0], 0, a3_arg, a0_arg, -1, -1, magic);
    func_80041688(0, 0);
    func_80041688(1, 0);
    if (((u8 *)D_800A3878)[3] & 0x40) func_8004659C(-1);
    if (D_8010277D == 0xE || D_8010277D == 0x1D) {
        saTan4FireDisp(D_800A37B4, D_800A37B5, D_800A37B6);
    }
    func_8001DBE4();
    D_800A3768 = 0xFF;
    D_800A36A8 = 0;
    func_80035FA8();
    v0 = func_80036EA8(5, ((u8 *)D_800A3878)[1]);
    mottest_rob_init(v0, ((u8 *)D_800A3878)[2]);
    func_80037260();
    D_800A37B8 = 0;
    D_800A3834 = 7;
    gpu_DisableDisplay();
}
/* kengo:HIGH  |  md_game/md_game_check_change_sub_mode  |  87i */

void md_game_check_change_sub_mode(void) {
    D_800A37B8++;

    if (func_80054F68() != 0) {
        if ((D_80102794 & 0x400040) == 0) {
            return;
        }
    }

    func_800372C0();
    func_800548DC();

    if (D_800A38DC != 0) {
        return;
    }

    if (D_800A3894 != 0) {
        D_800A3834 = 0;
        switch (D_800A37B0) {
        case 1:
        case 2:
        case 4:
        case 5:
            D_800A3907++;
            return;
        case 3:
            func_8003AF40(0);
            md_menu_logo_exec();
            /* fall through */
        case 6:
            D_800A3894 = 0;
            goto call_bar;
        default:
            return;
        }
    }

    if (D_800A385C != 0) {
        s32 val = D_800A390C;
        if (val == 1) {
            D_800A3834 = 0;
            return;
        }
        if (val == 0) {
            return;
        }
        if (val >= 4) {
            return;
        }
        D_800A385C = 0;
    }

call_bar:
    func_8003B5A4();
}

extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80078824(s32);
extern void func_80035FA8(void);
extern void func_80036FD4(s32, s32);
extern void func_80037260(void);
void func_8003BE10(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    obj_InitAll();
    func_80078824((s32)0x80118800);
    func_80035FA8();
    {
        s32 v0 = func_80036EA8(5, 0x20);
        func_80036FD4(v0, 4);
    }
    func_80037260();
    D_800A3834 = 0xB;
    gpu_DisableDisplay();
}
void func_8003BEA8(void) {
    s32 s0 = 0;
    s32 v0;
    s32 v1;

    v0 = func_800788B0();
    v1 = D_80102794 & 0x40;
    v0 = (u32)v0 > 0u;
    if (v1 != 0) {
        v0 = 1;
    }
    if (v0 != 0) {
        func_800372C0();
        {
            s32 *ptr = &g_file_disc_size;
            s32 old_val = *ptr;
            s32 new_val = old_val | D_800A37A4;
            if (new_val != old_val) {
                *ptr = new_val;
                s0 = 1;
            }
        }
        {
            s32 stage_u = (s32)(u16)D_80101ED2;
            s16 stage;
            if ((u32)stage_u < 2u) {
                goto check_bit;
            }
            stage = (s16)stage_u;
            if (stage == 2) {
                goto check_bit;
            }
            if ((u32)(stage_u - 0xC) < 2u) {
                goto check_bit;
            }
            if (stage == 0xE) {
                goto check_bit;
            }
            goto skip_bit;
        }
    check_bit:
        {
            s32 bit = (s16)D_80101ED2;
            u8 val;
            if (bit >= 0xC) {
                bit = bit - 9;
            }
            {
                u8 *vptr = &g_file_disc_type;
                val = *vptr;
                if (!((val >> bit) & 1)) {
                    *vptr = val | (1 << bit);
                    s0 = 1;
                }
            }
        }
    skip_bit:
        if (s0 != 0) {
            D_800A3834 = 0x1A;
        } else {
            D_800A3834 = 8;
        }
    }
}
void func_8003BFC4(void) {
    s32 v;
    gpu_EnableDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    v = func_80045814();
    func_80037540(v, (s32)0x80118000, 1, 0xCF8, 0xB01);
    game_Init();
    D_800A3834 = 8;
}

extern void kgm_init_hitrect(s32);
extern void mottest_rob_init(s32, s32);

void cpu_side_move_dir_2(void) {
    s32 a0;
    s8 *p;
    gpu_InitDisplay();
    gpu_EnableDisplay();
    func_80020CDC();
    if (((u32)(D_800A38A4 - 4)) < 2u) {
        file_ResetDmaFlag();
    }
    {
        u8 v = D_800A38A4;
        if (D_800A38A4 == 6) {
            D_8010277C = 8;
            D_8010277E = 6;
            a0 = 0;
            goto after_dispatch;
        }
        if (D_800A38A4 == 7) {
            D_8010277C = 0x16;
            D_8010277E = 7;
            a0 = 0;
            goto after_dispatch;
        }
        if (v == 8) {
            a0 = 0;
            D_8010277C = 0x1E;
            goto write_e_zero;
        }
        if (D_800A38A4 == 9) {
            a0 = 0;
            D_8010277C = 0x20;
        write_e_zero:
            D_8010277E = 0;

        after_dispatch:
            func_8003AF40(a0);

            kgm_init_hitrect(0);
        }
    }
    if (D_800A38A4 == 9) {
        a0 = 8;
    } else {
        a0 = D_800A38A4;
    }
    func_8005FBC8(a0, (s32)0x80118800);
    {
        u8 val = D_800A38A4;
        if (val == 4) {
            if ((&D_8008D9EC)[D_80101ED2] != 0) {
                goto do_copy;
            }
        }
        if (val == 5) {
            if ((&D_8008D9EC)[D_80101ED2] != 0) {
                goto skip_copy;
            }
        do_copy:
            D_8010277C = D_8010277D;

            D_8010277E = D_8010277F;
            func_8003AF40(0);
            kgm_init_hitrect(0);
        }
    skip_copy:;
    }
    func_80054884(0x16, (&D_8009016C)[D_800A38A4], 0, D_80101EDA, -1, -1, -1, (s32)0x80118800);
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    p = (s8 *)(((s8 *)(&D_8008EA70)) + (D_800A38A4 << 1));
    if (p[0] >= 0) {
        func_80035FA8();
        mottest_rob_init(func_80036EA8(5, p[0]), (u8)p[1]);
        func_80037260();
    }
    D_800A37B8 = 0;
    D_800A3834 = 0x13;
    gpu_DisableDisplay();
}
/* kengo:HIGH  |  nm_cpu/cpu_side_move_dir_2  |  160i  |  x4 size collision */
void func_8003C2C0(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_8005FC9C(D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    if (func_80054F68() == 0 || (D_80102794 & 0x400040) != 0) {
        if ((u32)(D_800A38A4 - 6) < 2u && D_800A3781 != 0) {
            s32 stage = (s16)D_80101ED2;
            s32 newval = 8;
            if (*((u8 *)&D_8008D9EC + stage) != 0) {
                newval = 9;
            }
            D_800A38A4 = newval;
            D_800A3834 = 0x12;
        } else {
            u8 a4val = D_800A38A4;
            if ((u32)(a4val - 4) < 2u) {
                D_800A3834 = 0x18;
            } else if ((u32)(a4val - 6) < 2u) {
                D_800A3834 = 0x1A;
            } else if ((u32)(a4val - 8) < 2u) {
                D_800A3834 = 0x1A;
            } else if (a4val < 2u) {
                D_800A3834 = 0xA;
            } else {
                D_800A3834 = 8;
            }
        }
    }
    {
        s16 state = D_800A3834;
        if (state != 0x13) {
            func_800372C0();
            state = D_800A3834;
            if (state != 0x12) {
                func_800548DC();
            }
        }
    }
}
extern u8 D_800A377C[];
extern u8 D_800F65F8[];
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
void func_8003C42C(void) {
    s32 counts[8];
    s32 i;
    s32 v0;
    v0 = D_800A389B;
    i = 0;
    counts[2] = 0;
    counts[1] = 0;
    counts[0] = 0;
    if (v0 > 0) {
        s32 *base = counts;
        s32 n = v0;
        do {
            s32 idx = D_800A377C[i];
            base[idx]++;
            i++;
        } while (i < n);
    }
    v0 = counts[0];
    if (v0 != counts[1]) {
        v0 = counts[0] < counts[1];
    } else {
        v0 = D_800A389B;
        i = 0;
        counts[1] = 0;
        counts[0] = 0;
        if (v0 > 0) {
            s32 *base2 = counts;
            s32 n2 = v0;
            u8 *tbl = D_800F65F8;
            do {
                s32 j = 0;
                s32 *p = base2;
                u8 *q = tbl;
                do {
                    *p += *q++;
                    j++;
                    p++;
                } while (j < 2);
                i++;
                tbl += 2;
            } while (i < n2);
        }
        v0 = counts[0];
        if (v0 != counts[1]) {
            v0 = v0 < counts[1];
        } else {
            v0 = 2;
        }
    }
    D_800A382D = v0;
    do { } while (0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x15;
}
void func_8003C560(void) {
    s32 counter;
    s32 ret;
    s32 id;
    u8 a4val;

    counter = D_800A37B8 + 1;
    D_800A37B8 = counter;
    if (D_800A382D == 2) {
        if (counter == 0x1E) {
            func_8005C650(0xA4, 0x7F, 0x7F);
        }
    } else {
        if (counter == 0x1E) {
            id = 0xA7;
            if (*((u8 *)&D_8008D9EC + *(s16 *)((u8 *)&D_80101ED2 + D_800A382D * 0x44C)) != 0) {
                id = 0xA8;
            }
            func_8005C650(id, 0x7F, 0x7F);
        }
        if (D_800A37B8 == 0x43) {
            func_8005C650(0xA9, 0x7F, 0x7F);
        }
    }
    ret = func_8005E54C(D_800A3784, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((D_80102794 & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        if (D_800A382D == 2) {
            D_800A3834 = 0x18;
        } else {
            func_800372C0();
            a4val = 4;
            if (*((u8 *)&D_8008D9EC + *(s16 *)((u8 *)&D_80101ED2 + D_800A382D * 0x44C)) != 0) {
                a4val = 5;
            }
            D_800A38A4 = a4val;
            D_800A3834 = 0x12;
        }
    }
}
void SetCurrentCursor(void) {
    u8 buf[4];
    s32 *s0;
    register s32 i asm("t0");
    register u8 *src asm("a2");
    register u8 *dst asm("a1");

    s0 = func_80077D00();
    func_800372C0();
    gpu_InitDisplay();
    func_80060758();
    i = 0;
    src = (u8 *)&D_80106A58;
    dst = (u8 *)s0;
    do {
        *(u8 *)(dst + 0x21) = (u8)(*(s32 *)(src + 4) / 1800);
        *(u8 *)(dst + 0x22) = (u8)((*(s32 *)(src + 4) / 30) % 60);
        i += 1;
        *(u8 *)(dst + 0x23) = (u8)(((*(s32 *)(src + 4) % 30) * 100) / 30);
        *(u8 *)(dst + 0x24) = *src;
        src += 8;
        dst += 4;
    } while (i < 3);
    func_8001CD68(buf);
    *(u8 *)((u8 *)s0 + 0x2D) = (u8)*(u16 *)buf;
    *(u8 *)((u8 *)s0 + 0x2E) = buf[2];
    *(u8 *)((u8 *)s0 + 0x2F) = buf[3];
    *(u8 *)((u8 *)s0 + 0x30) = (u8)D_80101ED2;
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x1F;
    gpu_DisableDisplay();
}
/* kengo:LOW  |  su_menu_edit/_SetCurrentCursor  |  104i  |  PS2 UI — reverted */
void func_8003C8B4(void) {
    s32 ret;

    D_800A37B8 = D_800A37B8 + 1;
    ret = func_80060768(D_800A38B4, 1, D_800A38E9);
    D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    if ((D_80102794 & 0x400040) != 0 || D_800A37B8 >= 0xF1) {
        func_80033FE4();
    }
}
void func_8003C958(void) {
    gpu_InitDisplay();
    D_800A3817 = 0;
    D_800A3929 = 0;
    D_800A37B8 = 0;
    D_800A3834 = 0x19;
    gpu_DisableDisplay();
}
extern void replay_camera_rob_back_win_near(s16 *, s16 *, s32);
void func_8003C9A4(void) {
    s32 ret;
    s32 *a0 = (s32 *)&D_800F6608;
    s16 *a1 = (s16 *)((u8 *)a0 + 0x10);

    game_SetControllerPorts(0);
    a0[0] = 0;
    D_800F660C = -0xBB8;
    D_800F6610 = 0;
    *a1 = 0x20;
    D_800F661C = 0;
    D_800F6620 = 0x2710;
    D_800F661A = (s16)(D_800A36AC << 2);
    replay_camera_rob_back_win_near((s16 *)a0, a1, 0x2710);
    game_StageInit(1);

    if (D_800A3929 == 0) {
        D_800A38B4 = D_800A38B4 + (func_8005C8A8(1, D_800A3817, D_800A38B4, 0) / 4) * 4;

        if ((D_80102794 & 0x10001000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 != 0) {
                D_800A3817 = D_800A3817 - 1;
            } else {
                D_800A3817 = 2;
            }
        } else if ((D_80102794 & 0x40004000) != 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (D_800A3817 == 2) {
                D_800A3817 = 0;
            } else {
                D_800A3817 = D_800A3817 + 1;
            }
        }

        if ((D_80102794 & 0x400040) != 0) {
            func_8005C650(1, 0x7F, 0x7F);
            D_800A3929 = (D_800A3817 == 0) ? 1 : 0x3C;
        }
        return;
    }

    if (D_800A3817 == 0) {
        ret = func_8005FA98(0, D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    }
    D_800A3929 = D_800A3929 + 1;
    if ((u8)D_800A3929 < 0x3C) return;

    func_800372C0();
    if (D_800A3817 == 0) {
        if (D_800A38DC == 5) {
            gpu_EnableDisplay();
            func_80020CDC();
            D_800A3874 = 0;
            func_800342A0();
            return;
        }
        D_800A3670 = 1;
        D_800A380C = D_800A380C + 1;
        D_800A38DF = (u8)func_80022408((s32 *)((u8 *)&D_80101FBC + (s32)D_800A3748 * 1100));
        D_800A3834 = 0;
        return;
    }
    if (D_800A3817 == 1) {
        func_8001DA2C();
        D_800A31DA = 1;
        D_800A3834 = 8;
        return;
    }
    if (D_800A3817 == 2) {
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
void func_8003CCCC(void) {
    gpu_InitDisplay();
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x21;
    gpu_DisableDisplay();
}
void func_8003CD10(void) {
    s32 *a0 = (s32 *)&D_800F6608;
    s16 *a1 = (s16 *)((u8 *)a0 + 0x10);
    s32 ret;

    game_SetControllerPorts(0);
    a0[0] = 0;
    D_800F660C = -0xBB8;
    D_800F6610 = 0;
    *a1 = 0x20;
    D_800F661C = 0;
    D_800F6620 = 0x2710;
    D_800F661A = (s16)(D_800A36AC << 2);
    func_80046BF4((s16 *)a0, a1, 0x2710);
    game_StageInit(1);

    ret = func_800600C8(D_800A391F, D_800A38B4, 1);
    D_800A38B4 = D_800A38B4 + ret * 4;
    D_800A37B8 = D_800A37B8 + 1;
    if (D_800A37B8 >= 0x97 || (D_80102794 & 0x400040) != 0) {
        func_800372C0();
        func_8001DA2C();
        D_800A3834 = 8;
    }
}
extern u16 D_80101ED6;
extern s32 D_800A3818;
extern void func_8001DA2C(void);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8003E22C(void);
extern void game_SetPlayerCount(s32);
extern s32 func_80022408(s32 *);
extern void func_80054884(s32, s32, s32, s32, s32, s32, s32, s32);
void func_8003CE18(void) {
    s32 s0;
    s32 v0;
    s8 player;

    func_8001DA2C();
    func_800372C0();
    gpu_InitDisplay();
    gpu_EnableDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8003E22C();
    game_SetPlayerCount(0);
    v0 = disp_CalcFov(0x2D);
    func_8007EFFC(v0);
    player = D_800A3748;
    {
        u16 val = *((u16 *)((u8 *)&D_80101ED6 + player * 1100));
        if ((u16)(val - 6) < 2) {
            s0 = 8;
            if (player != 0) {
                s0 = 9;
            }
        } else {
            s0 = 6;
            if (player != 0) {
                s0 = 7;
            }
        }
    }
    {
        s32 *addr = (s32 *)&D_80101FBC;
        s32 result;
        if (D_800A3748 == 0) {
            addr = (s32 *)((u8 *)addr + 0x44C);
        }
        result = func_80022408(addr);
        D_800A3818 = result;
        func_80054884(0x16, s0, result, (s32)D_80101EDA, (s32)D_80102326, -1, -1, 0);
    }
    func_80041688(0, 0);
    func_80041688(1, 0);
    game_Cleanup();
    D_800A37B8 = 0;
    D_800A3834 = 0x1D;
    gpu_DisableDisplay();
}
extern void mk_leaf_newpos(void);
extern void func_80021D10(s32, s32 *, s32);
extern void func_800618B4(s32 *, s32 *);
extern s32 *func_8005507C(void);
extern s32 *func_8005508C(void);
extern void func_80061064(s32 *, s32 *);
extern void func_8001979C(s32, u32 *);
extern void func_8003B328(void);
extern void func_8003B534(s32);
extern s32 D_800A312C;
void func_8003CF84(void) {
    volatile s32 pad[4];
    s32 vec[3];
    volatile s32 pad2[2];
    s32 *vp;
    s32 *a;
    s32 *b;
    s32 s1 = 0;
    s8 p;
    s16 stage;

    mk_leaf_newpos();
    p = D_800A3748;
    stage = *(s16 *)((u8 *)&D_80101ED2 + p * 0x44C);
    if (D_800A37B8 == (&D_8008EAC0)[stage]) {
        func_8005C650(40 * p + 0x2D, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB04) {
        func_8005C650(40 * D_800A3748 + 0x31, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB06) {
        func_8005C650(40 * D_800A3748 + 0x36, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB08) {
        if (D_800A3748 == 0) {
            func_8005C650(0x53, 0x7F, 0x7F);
        } else {
            func_8005C650(0x2B, 0x7F, 0x7F);
        }
    }
    if (D_800A37B8 == D_8008EB0A) {
        func_8005C650(0x71, 0x7F, 0x7F);
    }
    if (D_800A37B8 == D_8008EB0C) {
        vp = vec;
        func_80021D10(0, vp, D_800A3818);
        vp[0] += D_8008EB10;
        vp[1] += D_8008EB14;
        vp[2] += D_8008EB18;
        func_800618B4(vp, &D_800A312C);
    }
    a = func_8005507C();
    b = func_8005508C();
    func_80061064(a, b);
    if (func_80054F68() == 0) {
        s1 = 1;
    }
    if (s1 != 0 || (D_80102794 & 0x400040) != 0) {
        func_800548DC();
        if (D_800A38DC == 4 || D_800A38DC == 6) {
            (&D_800A37D2)[D_800A3748] = (&D_800A37D2)[D_800A3748] + 1;
        }
        func_8001979C(0, (u32 *)D_80102770);
        func_8001979C(1, (u32 *)D_801027C0);
        func_8001979C(2, (u32 *)D_801027D4);
        if (D_800A38DC == 0 && (u8)D_800A3836 != 0xFF) {
            func_8001DA2C();
            func_8003B328();
            func_8003AF40(0);
            md_menu_logo_exec();
            func_8003B534(4);
        } else {
            D_800A3834 = 0x18;
        }
    }
    D_800A37B8 = D_800A37B8 + 1;
}
void func_8003D2C4(void) {
    gpu_LoadImage((s32)&D_800A3220, (s32)&D_80090178);
}
extern s32 D_800A3364;
extern s32 D_800A3218;
extern s32 D_800A321C;
extern s32 D_800A3358;
extern s32 D_800A335C;
extern s32 D_800A3360;
void func_8003D2F4(void) {
    s32 v0;
    s32 v1;
    D_800A3364 = 0xF0F0F0;
    v0 = D_800A3218;
    v1 = D_800A321C;
    D_800A3358 = 0;
    D_800A3360 = 0;
    D_800A335C = 0;
    D_800A3218 = (u32)v0 < 1u;
    if (v1 == 0) {
        D_800A3358 = 0x20;
    }
}
void func_8003D330(void) {
    register u32 code asm("a1") = 0xE100001F;
    register u32 mask_lo asm("a2") = 0x00FF0000;
    u8 *base;
    register u8 *p asm("v1");
    register u32 mask_hi asm("a3");
    u32 *ot;
    u32 tag;

    base = (u8 *)&D_800A3D30;
    __asm__ volatile("ori %0, %0, 0xFFFF" : "=r"(mask_lo) : "0"(mask_lo), "r"(base));
    p = base + (D_800A3218 << 3);
    mask_hi = (u32)0xFF000000;

    p[3] = 1;
    *(u32 *)(p + 4) = code;

    ot = (u32 *)D_800A374C;
    tag = *(u32 *)p;
    tag = (tag & mask_hi) | (*ot & mask_lo);
    *(u32 *)p = tag;
    *ot = (*ot & mask_hi) | ((u32)p & mask_lo);
}
extern u32 D_800A3930;
void func_8003D39C(s32 x, s32 y, s32 ch, s32 color) {
    register s32 sx asm("t0") = x;
    register s32 sy asm("t1") = y;
    s32 count;
    u8 *p;
    register u32 tag asm("a1");
    register s32 scolor asm("t2") = color;

    count = D_800A3358;
    if (count == 0x20) return;

    {
        register u32 mask_lo asm("a3") = 0x00FFFFFF;
        u32 *ot;
        u32 mask_hi;
        u32 cmd_tag;
        u32 base;

        D_800A3358 = count + 1;
        __asm__ volatile("" ::: "memory");
        cmd_tag = 0x74000000;
        base = (count << 4) + (u32)&D_800A3930;
        {
            register u8 *pp asm("a0") = (u8 *)((D_800A3218 << 9) + base);

            pp[3] = 3;
            pp[7] = 0x74;
            pp[0xC] = ((ch & 7) * 8) - 64;
            pp[0xD] = ((ch >> 5) * 8) - 32;
            *((u16 *)(pp + 0xE)) = ((ch << 3) & 0xC0) | 0x773F;

            {
                u32 hc;
                __asm__ volatile("sra %0, %1, 1" : "=r"(hc) : "r"(scolor));
                tag = *((u32 *)pp);
                *((u32 *)(pp + 4)) = hc | cmd_tag;
            }

            ot = (u32 *)D_800A374C;
            mask_hi = 0xFF000000;
            *((s16 *)(pp + 8)) = sx;
            *((s16 *)(pp + 0xA)) = sy;

            tag = (tag & mask_hi) | ((*ot) & mask_lo);
            *((u32 *)pp) = tag;
            *ot = ((*ot) & mask_hi) | (((u32)pp) & mask_lo);
        }
    }
}
void func_8003D478(s32 x, s32 y, u8 *str, s32 color) {
    s32 ch;
    s32 start_x = x;

    ch = *str++;
    if (ch == 0) return;

    do {
        if (ch == 0x20) {
            /* space - advance */
        } else if (ch == 0x0A) {
            x = start_x;
            y += 8;
            goto next_char;
        } else {
            func_8003D39C(x, y, ch, color);
        }
        x += 8;
    next_char:
        ch = *str++;
    } while (ch != 0);
}
typedef char *va_list;
#define va_start(ap, last) (ap = (char *)__builtin_next_arg(last))
#define va_arg(ap, type) ((type *)(void *)(ap += 4))[-1]

s32 func_800791D8(u8 *);
void func_80079A30(u8 *, u8 *, s32);

void DispSleepMenuTex(u8 *fmt, s32 first_arg, ...) {
    u8 buf[0x400];
    u8 seg[0x100];
    register va_list ap asm("s3");
    s32 cur_arg;
    s32 seen_pct;
    register u8 *seg_ptr asm("s1");
    s32 ch;

    cur_arg = first_arg;
    seen_pct = 0;
    seg_ptr = seg;
    va_start(ap, first_arg);
    buf[0] = 0;

    ch = *fmt++;
    if (ch != 0) {
        s32 pct = 0x25;
        u8 *buf_ptr = buf;
        u8 *seg_start = seg_ptr;

        do {
            if (ch == pct) {
                if (seen_pct == 0) {
                    seen_pct = 1;
                } else {
                    *seg_ptr = 0;
                    func_80079A30(buf_ptr + func_800791D8(buf), seg_start, cur_arg);
                    seg_ptr = seg_start;
                    cur_arg = va_arg(ap, s32);
                }
            }
            *seg_ptr++ = ch;
            ch = *fmt++;
        } while (ch != 0);
    }

    *seg_ptr = 0;
    func_80079A30(buf + func_800791D8(buf), seg, cur_arg);

    ch = buf[0];
    if (ch != 0) {
        u8 *p = &buf[1];
        do {
            s32 row = D_800A3360;
            if (row >= 0x1A) goto done;

            if (ch == 0x20) goto inc_col;
            if (ch == 0x0A) {
                D_800A335C = 0;
                D_800A3360 = row + 1;
                goto check_wrap;
            }
            if (ch == 0x7E) {
                ch = *p++;
                if (ch == 0) goto done;
                if (ch == 0x63 || ch == 0x43) {
                    s32 d1, d2, d3;
                    d1 = *p++;
                    if (d1 == 0) goto done;
                    d2 = *p++;
                    if (d2 == 0) goto done;
                    d3 = *p++;
                    if (d3 == 0) goto done;
                    D_800A3364 = ((d1 - 0x30) << 5) | ((d2 - 0x30) << 13) | ((d3 - 0x30) << 21);
                }
                goto check_wrap;
            }
            _McAccessSection(D_800A335C * 8 + 0x10, row * 8 + 0x10, ch, D_800A3364);
        inc_col:
            D_800A335C = D_800A335C + 1;
        check_wrap:
            if (D_800A335C >= 0x4C) {
                D_800A335C = 0;
                D_800A3360 = D_800A3360 + 1;
            }
            ch = *p++;
        } while (ch != 0);
    }
done:;
}

/* kengo:LOW  |  su_menu_home/_DispSleepMenuTex  |  146i  |  PS2 UI — reverted */
void func_8003D774(s32 arg0, s32 arg1) {
    s32 *ptr = (s32 *)((u8 *)&D_800A3D40 + arg1 * 24);
    ptr[0] = arg0;
    ptr[1] = 0;
    ptr[2] = 0;
    *(s16 *)((u8 *)ptr + 0x16) = 0;
    *(s16 *)((u8 *)ptr + 0x14) = 0;
    *(s16 *)((u8 *)ptr + 0x12) = 0;
    *(s16 *)((u8 *)ptr + 0x10) = 0;
    *(s16 *)((u8 *)ptr + 0xE) = 0;
    *(s16 *)((u8 *)ptr + 0xC) = 0;
}
extern s32 func_8003D888(u32 *, s32);
s16 *func_8003D7B4(s32 arg0) {
    s32 i = 0;
    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);
    s32 new_var2;
    u8 *new_var;
    u8 *p = base;
    do {
        s32 nbits;
        s16 val;
        s32 sign_bit;
        do {
            nbits = func_8003D888((s32 *)base, 4);
            if (nbits == 0) {
                nbits = 16;
            }
            new_var = p;
            val = (s16)func_8003D888((s32 *)base, nbits);
        } while (0);
        if (!val) {
        }
        sign_bit = nbits - 1;
        new_var2 = 1;
        if ((val >> sign_bit) & new_var2) {
            val = val | (0xFFFF << sign_bit);
        }
        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(new_var + 0xC) + val);
        i++;
        p += 2;
    } while (i < 6);
    return (s16 *)(base + 0xC);
}

s32 func_8003D888(u32 *a0_param, s32 a1_param)
{
  register u32 *a2 asm("$6");
  register s32 a3 asm("$7");
  register s32 v0 asm("$2");
  register u32 v1 asm("$3");
  register s32 a0 asm("$4");
  register s32 a1 asm("$5");

  __asm__ __volatile__("addu\t$6, $4, $0" : "=r"(a2));
  __asm__ __volatile__("addu\t$7, $5, $0" : "=r"(a3));
  a1 = *((s32 *)((u8 *)a2 + 8));
  if (a1 < a3) {
    a3 -= a1;
    a0 = 1;
    v0 = (a0 << a1) - 1;
    a0 = (a0 << a3) - 1;
    v1 = *((u32 *)((u8 *)a2 + 4));
    a1 = *(s32 *)a2;
    v1 &= v0;
    v0 = a1 + 4;
    *(u32 *)a2 = v0;
    v0 = 32 - a3;
    a1 = *(s32 *)a1;
    v1 <<= a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v0 = (u32)a1 >> v0;
    v0 &= a0;
    v1 |= v0;
    *((u32 *)((u8 *)a2 + 4)) = a1;
  } else {
    v1 = *((u32 *)((u8 *)a2 + 4));
    v0 = a1 - a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v1 >>= v0;
    v0 = 1;
    v0 = (v0 << a3) - 1;
    v1 &= v0;
  }
  return (s32)v1;
}
extern s32 light_effect_col;
extern s32 D_800A4340;
void func_8003D91C(void) {
    s16 buf[4];
    s16 s1 = 0x1E0;
    s16 s0 = 0x140;
    buf[0] = 0;
    buf[1] = s1;
    buf[2] = s0;
    buf[3] = 1;
    func_8003D9A0(buf, 0x1F, &light_effect_col);
    buf[2] = 0x40;
    buf[0] = s0;
    buf[1] = s1;
    buf[3] = 8;
    func_8003D9A0(buf, 0x13, &D_800A4340);
}
typedef struct { s32 w[6]; } Copy24;
void func_8003D9A0(s16 *a0, s32 a1, u32 *a2) {
    register s16 *s0 asm("s0") = a0;
    register u32 *s1 asm("s1") = a2;
    s32 s4, s3;
    s32 s2;

    asm("" : : "r"(s1));
    s4 = s0[0];
    s3 = s0[1];

    if (a1 != 0) {
        s2 = a1 - 1;
        do {
            s16 v0;
            v0 = (u16)s0[1] + (u16)s0[3];
            s0[1] = v0;
            if (v0 >= 0x200) {
                s0[1] = s3;
                s0[0] = (u16)s0[0] + (u16)s0[2];
            }
            initLoadImage(s1, s0, s4, s3);
            *(Copy24 *)((u8 *)s1 + 0x18) = *(Copy24 *)s1;
            s1 = (u32 *)((u8 *)s1 + 0x30);
        } while (--s2 != -1);
    }
}
extern s16 D_800F6656;
extern void func_8003DBE4(s32, s32, s32 *, s32, s32);
void tslLineG5Init(s32 arg0, s32 arg1) {
    s32 dist;
    s16 *new_var2;
    s32 offset;
    int new_var;
    s32 *ptr;

    D_800905F8 = 0xFFFF;
    new_var2 = &D_800906A4 + arg0 * 2;
    if (*new_var2 != 0) {
        if (game_GetPlayerCount() != 0) {
            dist = 0x55F0;
        } else {
            dist = 0x6590;
        }
        dist = dist - arg1;
        new_var = dist < 0x1770;
        if (new_var) {
            offset = 0x1770 - dist;
            dist = 0x1770;
        } else {
            offset = 0;
        }
        {
            s16 v1 = D_800F6656;
            s16 mask = D_80090608;
            if ((v1 & ~mask) & 1) {
                D_80090600 = dist;
                D_80090604 = offset;
            }
            if (v1 & 1) {
                arg1 -= D_80090604;
            } else {
                arg1 -= offset;
            }
        }
        {
            s32 idx = arg0 * 4;
            D_80090608 = (u16)D_800F6656;
            ptr = (s32 *)((u8 *)&D_8009060C + idx);
            func_8003DBE4(arg1, 0x1F, &light_effect_col, *ptr, *(s16 *)((u8 *)&StatusUpBuf + idx));
            func_8003DBE4(arg1, 0x13, &D_800A4340, *ptr, *(s16 *)((u8 *)&StatusUpBuf + idx));
        }
    }
}
void func_8003DBE4(s32 arg0, s32 arg1, s32 *arg2, s32 arg3, s32 arg4) {
    s32 limit;
    s32 step;
    s32 *colors;
    s32 i;

    colors = arg2;

    if (arg4 != 0) {
        limit = arg1;
    } else {
        limit = arg1 - 1;
    }

    if (D_800F6656 & 1) {
        step = D_80090600;
    } else {
        s32 v0 = 0x55F0;
        if (game_GetPlayerCount() == 0) {
            v0 = 0x6590;
        }
        step = v0 - arg0;
    }

    if (step < 0) {
        return;
    }

    if ((u32)arg3 < (u32)step) {
        step = (s32)((u32)arg3 / (u32)arg1);
    } else {
        step = step / arg1;
    }

    i = 0;
    colors = (s32 *)((u8 *)colors + (D_800A36AC & 1) * 24);

    if (limit > 0) {
        u32 rgb_mask = 0xFFFFFF;
        u32 alpha_mask = 0xFF000000;

        do {
            s32 idx = func_80052C28((u32)arg0 >> 2, 2);
            if (idx < 0x1000) {
                s32 *pal = (s32 *)(D_800A378C + (u32)idx * 4);
                *colors = (*colors & alpha_mask) | (*pal & rgb_mask);
                *pal = (*pal & alpha_mask) | ((u32)colors & rgb_mask);
                if (i + 1 >= limit) {
                    D_800905F8 = idx;
                }
                colors = (s32 *)((u8 *)colors + 0x30);
            }
            arg0 += step;
            i++;
        } while (i < limit);
    }

    if (arg4 != 0) {
        func_8003DDF8((u32)colors);
    } else {
        s32 *pal = (s32 *)D_800A378C;
        *colors = (*colors & (s32)0xFF000000) | (*(s32 *)((u8 *)pal + 0x3FEC) & 0xFFFFFF);
        *(s32 *)((u8 *)pal + 0x3FEC) = (*(s32 *)((u8 *)pal + 0x3FEC) & (s32)0xFF000000) | ((u32)colors & 0xFFFFFF);
    }
}
void func_8003DDF8(u32 arg0) {
    u32 *ptr = (u32 *)D_800A378C;
    arg0 &= 0xFFFFFF;
    ptr[0x3FFC / 4] = arg0;
}
void func_8003DE14(s16 *rect, s32 count) {
    u16 src_buf[0x200];
    u16 dst_buf[0x200];
    u8 color_info[0x20];
    register s32 i asm("s1");
    register s32 saved_y asm("s7");
    register s32 r asm("s5");
    register s32 g asm("s4");
    register s32 b asm("s3");
    s32 target_color;

    gpu_DrawSync(0);
    count--;
    gpu_StoreImage((s32 *)rect, src_buf);
    gpu_DrawSync(0);
    i = 0;
    ((u16 *)rect)[1] -= ((u16 *)rect)[3];
    gpu_LoadImage((s32)rect, (s32)src_buf);
    saved_y = rect[1];
    rect[1] = ((u16 *)rect)[3] + saved_y;
    func_80052BE4(color_info);

    r = color_info[0];
    g = color_info[1];
    b = color_info[2];
    target_color = (u32)r >> 3;
    target_color |= ((g & 0xF8) << 2) | (s32)-0x8000;
    target_color |= (b & 0xF8) << 7;

    if (count > 0) {
        s32 blend_base = 0x1000;
        do {
            s32 total = rect[2] * rect[3];
            u16 *src = src_buf;
            u16 *dst = dst_buf;
            s32 factor = ((i + 1) << 12) / count;
            s32 j = 0;

            if (total > 0) {
                s32 complement = blend_base - factor;
                do {
                    __asm__ volatile("" : "=r"(count) : "0"(count));
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst = pixel;
                            src++;
                            goto advance_dst;
                        }
                        *dst++ = target_color;
                        src++;
                        goto loop_check;
                    }
                    {
                        u16 pixel = *src;
                        s32 px = pixel & 0xFFFF;
                        if (px == 0) {
                            *dst = pixel;
                            src++;
                            goto advance_dst;
                        }
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 b_src = ((u32)px >> 7) & 0xF8;
                            s32 r_ch;
                            s32 g_ch;
                            s32 b_shift;
                            src++;
                            r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
                            g_ch = ((g_src * complement + g * factor) >> 10) & 0x3E0;
                            b_shift = (b_src * complement + b * factor) >> 5;
                            *dst = (pixel & 0x8000) | r_ch | g_ch | (b_shift & 0x7C00);
                        }
                    }
                advance_dst:
                    dst++;
                loop_check:
                    j++;
                } while (j < rect[2] * rect[3]);
            }

            {
                s32 new_y = ((u16 *)rect)[1] + ((u16 *)rect)[3];
                ((u16 *)rect)[1] = new_y;
                if ((s16)new_y >= 0x200) {
                    rect[1] = saved_y;
                    ((u16 *)rect)[0] += ((u16 *)rect)[2];
                }
            }
            gpu_LoadImage((s32)rect, (s32)dst_buf);
            gpu_DrawSync(0);
            i++;
        } while (i < count);
    }
}
void func_8003E0E0(void) {
    s16 buf[4];
    buf[1] = 0x1E1;
    buf[2] = 0x140;
    buf[0] = 0;
    buf[3] = 1;
    func_8003DE14(buf, 0x1F);
}
void func_8003E120(void) {
    s16 buf[4];
    buf[0] = 0x140;
    buf[1] = 0x1E8;
    buf[2] = 0x40;
    buf[3] = 8;
    func_8003DE14(buf, 0x13);
}
extern s32 D_800A3228;
extern void func_8007B6C8(s16 *, s32, s32);
void func_8003E164(s32 arg0) {
    s16 buf[4];
    s32 *s0;

    if (D_800A3228 == arg0) {
        goto end;
    }
    func_8003E22C();
    s0 = func_8004153C(arg0);
    if (s0 == 0) {
        goto end;
    }
    if (arg0 != 0) {
        buf[0] = 0x300;
    } else {
        buf[0] = 0x280;
    }
    buf[1] = 0xF8;
    buf[2] = 0x40;
    buf[3] = 6;
    func_8007B6C8(buf, 0x140, 0x1E8);
    if (arg0 == 0) {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x140, 0xE8);
    } else {
        func_800432A0(*(s16 *)((u8 *)s0 + 0x14), 0, 0, -0x1C0, 0xE8);
    }
    gpu_DrawSync(0);
    func_8003E120();
end:
    D_800A3228 = arg0;
}
extern s32 D_800A3228;
void func_8003E22C(void) {
    s32 *v1;

    if (D_800A3228 != -1) {
        v1 = func_8004153C(D_800A3228);
        if (v1 != 0) {
            if (D_800A3228 == 0) {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x140, -0xE8);
            } else {
                func_800432A0(*(s16 *)((u8 *)v1 + 0x14), 0, 0, 0x1C0, -0xE8);
            }
        }
        D_800A3228 = -1;
    }
}
extern s32 D_800A3228;
s32 func_8003E2A0(void) {
    return D_800A3228;
}
extern u16 g_game_p1_ctrl;
void func_8003E2AC(void) {
    u16 *p = &g_game_p1_ctrl;
    *p = *p & 0xFFFD;
}
u32 func_8003E2C8(void) {
    return D_800905F8;
}
INCLUDE_ASM("asm/funcs", replay_camera_get_attack_number);
/* kengo:HIGH  |  nm_replay_cam/replay_camera_get_attack_number  |  242i */
void func_8003E6A0(s32 arg0, s32 arg1) {
    replay_camera_get_attack_number(D_80101E3C, D_80101E44, arg0, arg1);
}
INCLUDE_ASM("asm/funcs", DispHira);
/* kengo:MED  |  am_rmd/DispHira  |  299i */
INCLUDE_ASM("asm/funcs", func_8003EB84);
INCLUDE_ASM("asm/funcs", md_game_check_mode);
/* kengo:HIGH  |  md_game/md_game_check_mode  |  234i */
