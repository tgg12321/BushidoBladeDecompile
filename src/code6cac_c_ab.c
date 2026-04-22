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
extern u8 D_800F33D8;
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
extern void sys_VSync(s32);
extern void gpu_LoadImage(s32, s32);
extern s32 func_80036FD4(void);
extern void func_80035FA8(void);
extern s32 D_800109BC;
extern void game_Cleanup(void);
extern s32 func_800371E8(s16);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern u8 *D_800A3894;
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

extern u8 D_80102781;


extern u8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003553C(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern void func_800602AC(s32, s32);
extern s32 obj_InitTaskCamera(s32);
extern s32 D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern s32 func_80036D88(void);
extern void gnd_disp_loop_ctrl(void);
extern s32 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s16 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void special_camera_get_rot_dir(s32 *);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 D_800A3210;
extern void func_8008BE04(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern u8 D_8008E6A4;
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A36C0;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern u8 D_800A377C;
extern u8 D_800A37A8;
extern s16 D_800A3904;
extern u8 D_800F65F8;
extern s32 g_anim_func_table;
extern s32 g_pad_data;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s16 D_80101ED6;
extern s32 g_file_disc_size;

/* GP-relative extern data (for decompiled functions) */
extern s32 D_800A3730;
extern s16 D_800A379E;
extern s16 D_800A3814;
extern s16 D_800A37C8;
extern s32 D_800A31F4;
extern u8 D_800A38CC;
extern u8 D_800A382C;
extern s32 D_800A3734;
extern s32 D_800A373C;
extern s32 D_800A3810;
extern s32 D_800A3738;
extern u8 D_800A320C;
extern s32 D_800A37DC;
extern s32 D_800A37F0;
extern s32 D_800A37FC;
extern s32 D_800A3800;
extern s32 D_800A3838;
extern s32 D_800A383C;
extern s32 D_800A3848;
extern s32 D_800A3850;
extern s32 D_800A31F8;
extern u8 D_800A36F8;
extern s32 D_800A36EC;
extern u8 D_800A3782;
extern u8 D_800A3203;
extern u8 D_800A31FC;
extern s32 D_80106A50;
extern s32 D_800A38C8;
extern u8 D_80102810;
extern s32 D_800A3870;
extern s32 D_800A3688;
extern s32 D_800A3698;
extern u8 D_800A37D0;
extern s32 D_800A3915_ext;
extern s32 D_800A36F4_ext;

/* Extern function declarations for decompiled functions */
extern s32 func_80078998(s32);
extern void func_80078988(s32);
extern void func_800789A8(s32);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void func_800789F8(s32, s32 *, s32);
extern void func_80078A18(s32);
extern s32 func_80078A38(s32 *, s32 *);
extern s32 func_80078A48(s32 *);
extern void func_80078BA8(s32);
extern s32 func_80078B04(s32);
extern void func_8007A400(void);
extern void func_8008BE4C(void);
extern s32 func_8006BEC4(s32, s32);
extern void func_8003E22C(void);
extern void game_SetPlayerCount(s32);
extern s32 disp_CalcFov(s32);
extern void func_8007EFFC(s32);
extern void func_8001B6F4(void);
extern void func_80022568(u8 *);
extern s32 g_str_memcard_fmt;
extern s32 D_80102810;
extern s32 D_800F34D8;

/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */
extern void func_80077AE0(void);
extern void func_80077B00(void);
extern u16 func_80035E88(s32);
extern s32 func_80035EDC(s16);
extern u16 func_80019488(void);
extern void func_800194C0(s16);
extern u16 func_80079154(void);
extern void func_80019568(s32);
extern u8 D_800A38AC;
extern s32 D_800A37D8;
extern u8 D_800A3916;
extern s32 D_800A38D0;
extern s32 D_800A3908;
extern s32 D_800A38FC;
extern u16 D_800A37C4;

s32 func_8003AB44(void) {
    D_800A37B8++;
    switch (D_800A38AC) {
        case 0:
            D_800A38AC = 1;
            __asm__ __volatile__ ("" ::: "memory");
            return 0;
        case 1:
            func_8003A308();
            D_800A37D8 = 0;
            if (D_800A38A0 == 0) {
                gpu_SetDispMask(1);
                D_800A38AC = 2;
                __asm__ __volatile__ ("" ::: "memory");
                return 0;
            }
            D_800A38AC = 3;
            __asm__ __volatile__ ("" ::: "memory");
            return 0;
        case 2:
            if (D_80102794 & 0x10) {
                goto fail;
            }
            if (func_8008C464(3, 1, 0) == 0) {
                return 0;
            }
            goto done;
        case 3:
            if (func_8008C464(3, 1, 0) != 0) {
                goto retry;
            }
            /* fall through */
        done:
            func_8008C464(3, 0, 0);
            D_800A38AC = 4;
            return 0;
        retry:
            D_800A37D8++;
            if (D_800A37D8 < 4) {
                return 0;
            }
            /* fall through */
        fail:
            func_8003A39C();
            return -1;
        case 4:
            gpu_SetDispMask(0);
        case 5:
        case 6:
            D_800A38AC++;
            return 0;
        case 7:
            D_800A3916 = 1;
            func_8003A360();
            return 1;
    }
    return 0;
}
s32 func_8003ACB8(void) {
    s32 temp_s0;

    func_80077AE0();
    gpu_SetDispMask(0);
    D_800A37B8 = 0;
    D_800A38AC = 0;
    D_800A38D0 = 0;
    D_800A3908 = 0;
    D_800A38FC = 0;
    func_8003A174();
    do {
        func_80019568(1);
        func_8005C6D0();
        temp_s0 = func_8003AB44();
        sys_VSync(2);
    } while (temp_s0 == 0);
    func_80077B00();
    func_800194F4();
    D_800A37C4 = func_80035E88(g_file_disc_size);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    D_800A38E4 = func_80035EDC(D_800A36C6);
    D_800A37C4 = func_80019488();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    func_800194C0(D_800A36C6);
    D_800A37C4 = func_80079154();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    sys_VSync(1);
    func_8003AA48();
    {
        s32 var_v0;
        if (D_800A38A0 == 0) {
            var_v0 = (u16)D_800A37C4;
        } else {
            var_v0 = D_800A36C6;
        }
        D_800A3904 = var_v0;
    }
    gpu_InitDisplay();
    gpu_DisableDisplay();
    func_80078BA8(0xF2000001);
    return temp_s0;
}
void func_8003AE5C(u8 *arg0) {
    s32 addr = (s32)0x80190800;
    s32 result = -1;
    s32 done = 0;

    do {
        s32 cmd = *arg0++;
        switch (cmd) {
            case 1:
                result = *arg0;
            case 0:
            case 20:
                done = 1;
                break;
            case 16:
                arg0 += 5;
                break;
            case 17:
                arg0 += 3;
                break;
            case 3:
                arg0 += 2;
                break;
            case 19:
                arg0 += 4;
                break;
            case 2:
                arg0 += 0x1D;
                break;
        }
    } while (!done);

    if (result >= 0) {
        (&D_800A37A8)[D_800A37A0] = *(u16 *)&D_800A36A4;
        gpu_EnableDisplay();
        func_80020D38();
        func_800602AC(result, addr);
    }
}
void func_8003AF40(s32 arg0) {
    if ((&D_8010277E)[arg0] == 0xFF) {
        (&D_8010277E)[arg0] = (&D_8010277E)[(u32)arg0 < 1u];
    }
    func_80022580(arg0, ((s8 *)&D_80102780)[arg0], ((s8 *)&D_8010277C)[arg0], ((s8 *)&D_8010277E)[arg0], 0);
    gpu_EnableDisplay();
    func_80020D38();
    func_80040510(arg0, (&D_8008D578)[(s8)(&D_8010277C)[arg0]], (s32)0x80190800);
}
void func_8003AFFC(void) {
    s32 addr = (s32)0x80190800;
    s32 s0;
    s16 *edcp;
    s32 s2;
    u8 *tbl;
    s32 v1;

    gpu_EnableDisplay();
    func_80020D38();
    func_8004939C();

    tbl = &D_8008E5CC;
    edcp = &D_80101EDC;
    s2 = 0;
    s0 = 0;
loop:
    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + s0));
    func_800494D4(s2, *(tbl + *(s16 *)((u8 *)&D_80101ED2 + s0) * 8 + *(s16 *)((u8 *)&D_80101ED6 + s0)));

    v1 = *edcp;
    if (v1 != -1) {
        func_800493E4((&D_8008EB80)[v1]);
        v1 = *edcp;
        if (v1 == 14) {
            func_800493E4(D_8008EB8E + 3);
        }
    }
    edcp = (s16 *)((u8 *)edcp + 0x44C);
    s0 += 0x44C;
    s2++;
    if (s2 < 2) goto loop;

    func_80049584(addr);
}
void func_8003B10C(s32 arg0) {
    s32 addr = (s32)0x80190800;
    s32 s0;
    u8 *tbl;
    s32 v0;

    gpu_EnableDisplay();
    func_80020D38();
    func_8004939C();

    s0 = arg0 * 1100;
    func_800493E4(*(s16 *)((u8 *)&D_80101EDA + s0));

    if (D_800A38DC == 5) {
        tbl = &D_8008E6A4;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + s0) * 6;
    } else {
        tbl = &D_8008E5CC;
        v0 = *(s16 *)((u8 *)&D_80101ED2 + s0) * 8;
    }
    func_800494D4(arg0, *(tbl + v0 + *(s16 *)((u8 *)&D_80101ED6 + s0)));
    func_80049584(addr);
}
void func_8003B20C(s32 arg0) {
    u8 *new_var;
    register s32 one asm("v1") = 1;
    D_80102780 = 0;
    D_80102781 = one;
    arg0 = ((s32 *)(new_var = &D_800900EC))[arg0];
    D_800A3894 = 0;
    D_800A385C = 0;
    D_800A3836 = 0xFF;
    D_800A3915 = 0xFF;
    D_800A37C6 = one;
    D_800A37A0 = 0;
    D_800A37A4 = 0;
    D_800A3844 = arg0;
    file_LoadOverlay();
    func_8003AE5C(D_800A3844);
    func_8003AF40(0);
    D_800A376C = (&D_8008D538)[(s8)D_8010277C];
}
extern u8 *D_800A3894;
extern void player_SetCharId(s32, s32);
void func_8003B2C8(void) {
    u8 *p = &D_8010277C;
    D_800A3836 = *p;
    {
        u8 *base = D_800A3894;
        u8 v1 = D_800A376A;
        u8 v0 = *base;
        D_800A36C8 = v1;
        D_800A376A = 0;
        *p = v0;
    }
    player_SetCharId(0, 0);
}
void func_8003B328(void) {
    u8 *p = &D_8010277C;
    u8 v_277C = *p;
    u8 v_376A = D_800A376A;
    u8 v_3836 = D_800A3836;
    u8 v_36C8 = D_800A36C8;
    D_800A3836 = 0xFF;
    D_800A3915 = v_277C;
    D_800A36F4 = v_376A;
    *p = v_3836;
    D_800A376A = v_36C8;
    player_SetCharId(0, v_36C8);
    func_80022568(&D_80101EC8);
}
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
s32 func_8003B484(u8 *arg0) {
    D_800A3712 = 1;
    D_8010277D = arg0[0];
    D_8010277F = arg0[1];
    D_80102783 = arg0[2];
    func_8003AF40(1);
    func_8003AFFC();
    return 3;
}
void func_8003B4DC(void) {
    D_800A3712 = 1;
    D_8010277E = 0;
    D_8010277D = 0x1F;
    D_8010277F = 0;
    func_8003AF40(0);
    func_8003AF40(1);
    func_8003AFFC();
}
void func_8003B534(s32 a0) {
    D_800A37B0 = a0;
    D_800A3834 = 6;
    D_800A3878 = D_800A3894 + (a0 * 4 + 1);
}
void func_8003B56C(s32 arg0) {
    D_800A390C = arg0;
    D_800A3834 = 6;
    D_800A3878 = D_800A385C + (arg0 * 4 - 4);
}
INCLUDE_ASM("asm/funcs", suDispMentalBar);
void func_8003B870(void) {
    player_SetCharId(0, D_800A376A);
    player_SetCharId(1, 0);
    obj_InitChars();
    gpu_InitDisplay();
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A37B8 = 0;
    D_800A3834 = 0x17;
    gpu_DisableDisplay();
}
void func_8003B8E4(void) {
    s32 tmp;
    s32 ret;

    tmp = D_800A37B8 + 1;
    D_800A37B8 = tmp;
    if (tmp < 3) {
        ret = func_80060544(D_800A38B4, 1);
        D_800A38B4 = D_800A38B4 + (ret / 4) * 4;
    }
    if (D_800A37B8 == 3) {
        gpu_DrawSync(0);
        func_8003AE5C(D_800A3844);
        D_800A37C0 = 500;
        D_800A38F8 = 0;
        g_disp_enable = DISP_ACTIVE;
        se_data_set();
        func_8003B5A4();
        disp_SetFramebufferMode(1, 0, 0, 0);
        D_800A390D = 1;
    }
}
