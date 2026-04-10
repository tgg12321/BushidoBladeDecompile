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
extern void func_800194F4(void);
extern void seq_Reset(void);
extern void func_8003A39C(void);
extern void sys_VSync(s32);
extern void gpu_LoadImage(s32, s32);
extern void game_Cleanup(void);
extern void seq_Start(s32, s32);
extern u16 g_game_p1_ctrl;
extern s32 D_80102794;
extern s32 D_800A3894;
extern s16 D_800A38C4;
extern s16 D_80101F32;
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
extern volatile s32 D_80101E70;
extern s32 D_800A3894;
extern u8 D_80102781;


extern u8 D_8010277D;
extern u8 D_8010277F;
extern s16 D_800A391D;
extern s32 file_GetFlag2(void);
extern s16 *snd_GetSeId(void);
extern void func_8003AF40(s32);
extern void func_8003AFFC(void);

extern void sys_Panic(void);
extern s32 func_80020D38(void);
extern s32 obj_InitTaskCamera(s32);
extern void *D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 func_8007FD5C(s32, s32);
extern s32 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void func_80037774(void);
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
extern void initPolyG4(u8 *p);
extern void ot_Link(u32 *a0, u32 *a1);
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
/* --- Functions from 6CAC segment (0x80017FA0 - 0x8003EDC0) --- */

extern void func_80035280(void);
extern void func_80068ECC(s32);
extern u8 D_800A3740;
void func_80035438(void) {
    s32 a0;
    D_800A3740 = 1;
    func_80035280();
    if (g_file_disc_type == 0x3F) {
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
    s32 v0;
    gpu_EnableDisplay();
    gpu_InitDisplay();
    if (D_800A31DA == 0) {
        func_8003A41C();
    }
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    if (D_800A31D8 != 0) {
        obj_InitAll();
        D_800A390E = -1;
        v0 = 1;
    } else {
        v0 = 1;
    }
    D_800A31D8 = v0;
    func_80035438();
    func_80077820((s32)0x80118800);
    D_800A37B8 = 0;
    D_800A3834 = 9;
    gpu_DisableDisplay();
}
void func_8003553C(void) {
    u8 *temp_s0;
    register s32 v280 asm("v1");
    register s32 vtmp asm("v0");

    temp_s0 = (u8 *)D_800A38B4;
    initPolyG4(temp_s0);
    __asm__ ("addiu $3,$zero,640\naddiu $2,$zero,240" : "=r"(v280), "=r"(vtmp));
    *(s16 *)(temp_s0 + 0x1A) = vtmp;
    *(s16 *)(temp_s0 + 0x22) = vtmp;
    vtmp = 0x80;
    *(u8 *)(temp_s0 + 4) = 0;
    *(u8 *)(temp_s0 + 5) = 0;
    *(u8 *)(temp_s0 + 6) = vtmp;
    *(u8 *)(temp_s0 + 0xC) = 0;
    *(u8 *)(temp_s0 + 0xD) = 0;
    *(u8 *)(temp_s0 + 0xE) = vtmp;
    *(u8 *)(temp_s0 + 0x14) = 0;
    *(u8 *)(temp_s0 + 0x15) = 0;
    *(u8 *)(temp_s0 + 0x16) = 0;
    *(u8 *)(temp_s0 + 0x1C) = 0;
    *(u8 *)(temp_s0 + 0x1D) = 0;
    *(u8 *)(temp_s0 + 0x1E) = 0;
    {
        register u32 *a0_arg asm("a0") = (u32 *)(D_800A374C + 0x401C);
        register u32 *a1_arg asm("a1") = (u32 *)temp_s0;
        *(s16 *)(temp_s0 + 8) = 0;
        *(s16 *)(temp_s0 + 0xA) = 0;
        *(s16 *)(temp_s0 + 0x10) = v280;
        *(s16 *)(temp_s0 + 0x12) = 0;
        *(s16 *)(temp_s0 + 0x18) = 0;
        *(s16 *)(temp_s0 + 0x20) = v280;
        temp_s0 += 0x24;
        ot_Link(a0_arg, a1_arg);
    }
    D_800A38B4 = temp_s0;
}
void func_800355E8(void) {
    func_80035FA8();
    func_80037110(1);
    func_800371E8(1);
}
INCLUDE_ASM("asm/funcs", replay_camera_rob_back_loose2);
/* kengo:MED  |  nm_replay_cam/replay_camera_rob_back_loose2  |  126i  |  -6 4.8% */
INCLUDE_ASM("asm/funcs", func_80035828);
void func_80035DC8(void) {
    gpu_EnableDisplay();
    gpu_InitDisplay();
    func_80020CDC();
    player_Destroy(0);
    player_Destroy(1);
    file_ResetDmaFlag();
    obj_InitAll();
    func_80077820((s32)0x80118800);
    D_800A3834 = 0x1B;
    gpu_DisableDisplay();
}
void func_80035E38(void) {
    D_800A36F1 = 1;
    func_8003553C();
    if (motion_SetMotion() != 0) {
        D_800A3834 = 8;
        D_800A36F1 = 2;
    }
}
s32 func_80035E88(s32 a0) {
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
s32 func_80035EDC(s32 a0) {
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
extern void func_80080600(u8 *);
extern u8 D_800A3718;
extern s16 D_800A3854;
void func_80035F30(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    D_800A3718 = (u8)arg0;
    D_800A3719 = (u8)arg1;
    D_800A371A = (u8)arg2;
    D_800A371B = (u8)arg3;
    func_80080600(&D_800A3718);
    D_800A3854 = 0;
}
extern u8 D_800A36B8;
extern s16 D_800A3840;
void func_80035F78(s16 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A36B8 = (u8)arg1;
    D_800A36B9 = (u8)arg2;
    D_800A36BA = (u8)arg3;
    D_800A3854 = arg0;
    D_800A3840 = 0;
    D_800A36BB = (u8)arg4;
}
void func_80035FA8(void) {
    spu_SetVolume(0, 0, 1);
    func_80085448(0, 0x7F, 0x7F);
}
extern u8 D_800A31E4;
void func_80035FE0(void) {
    func_8007FF7C();
    cdrom_SetDebugLevel(0);
    func_80035F30(0, 0, 0, 0);
    D_80101E62 = 0;
    if (D_800A31E4 == 0) {
        D_800A31E4 = 1;
    }
}
void func_80036034(void) {
    func_80080148();
    func_8007FF7C();
    sys_VSync(4);
}
extern void func_80080620(s32, s32);
extern s32 cdrom_BcdToFrames(s32);
void func_80036064(u8 arg0) {
    s32 sp[4];
    if (arg0 == 1) {
        D_80101E98 = 0;
        if (D_80101E80 <= 0) {
            return;
        }
        func_80080620((s32)sp, 3);
        {
            s32 v0 = cdrom_BcdToFrames((s32)sp);
            if (v0 != D_80101EA0) {
                D_80101E80 = -2;
                goto do_stop;
            }
        }
        func_80080620(D_80101E84, 0x200);
        D_80101E84 = D_80101E84 + 0x800;
        D_80101E80 = D_80101E80 - 1;
        D_80101EA0 = D_80101EA0 + 1;
        if (D_80101E80 == 0) {
            goto do_stop;
        }
        return;
    } else {
        D_80101E80 = -1;
    }
do_stop:
    cdrom_SetCallbackB(0);
    func_80080390(9, 0);
}
INCLUDE_ASM("asm/funcs", special_camera_set_win_cam);
/* kengo:MED  |  nm_special_cam/special_camera_set_win_cam  |  502i  |  -10 */
INCLUDE_ASM("asm/funcs", special_camera_Exec);
/* kengo:HIGH  |  nm_special_cam/special_camera_Exec  |  274i */
s32 func_80036D88(void) {
    return D_80101E62 == 0;
}
s32 replay_camera_Init(s32 a0, s32 a1) {
  register s32 v0 asm("$2");
  register s32 v1 asm("$3");
  register s32 a3 asm("$7");
  register volatile s16 *t0 asm("$8") = (volatile s16 *)&D_80101E62;

  __asm__ volatile("lh %0, 0(%1)" : "=r"(v0) : "r"(t0));
  a3 = a1;
  if (v0 != 0) {
    return 0;
  }
  v0 = ((s32)(a0 << 16)) >> 13;
  D_80101E60 = a0;
  v1 = *(s32 *)((u8 *)&SpecialCam + v0);
  __asm__ volatile("" ::: "memory");
  a0 = *(s32 *)((u8 *)&D_8008EC38 + v0);
  D_80101E6C = v1;
  __asm__ volatile("" ::: "memory");
  D_80101E70 = a0;
  v1 = D_80101E70;
  __asm__ volatile("" ::: "memory");
  a0 = 2;
  __asm__ volatile("" ::: "memory");
  D_80101E7C = a3;
  D_80101E68 = 0;
  *t0 = a0;
  D_80101E9E = 0;
  v1 += 0x7FF;
  v1 = (u32)v1 >> 11;
  D_80101E78 = v1;
  return 1;
}
/* kengo:HIGH  |  nm_replay_cam/replay_camera_Init  |  39i */
s32 func_80036E34(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    if (replay_camera_Init(arg0, arg1) == 0) {
        return 0;
    }
    cdrom_FramesToBcd(cdrom_BcdToFrames((s32)&D_80101E6C) + arg2, (s32)&D_80101E6C);
    D_80101E78 = arg3;
    return 1;
}
s32 func_80036EA8(s32 arg0, s32 arg1) {
    return (&D_8008F12C)[arg0] + arg1;
}
void game_FrameInit(void) {
    cdrom_SetCallbackB(0);
    func_80035F30(0, 0, 0, 0);
    func_80080148();
    func_80080390(9, 0);
    D_80101E68 = 1;
    D_80101E62 = 0xB;
    D_80101E5C = 0;
}
u32 func_80036F28(s32 arg0) {
    return (&D_8008EC38)[arg0 * 2];
}
void game_FrameLoop(void) {
    u16 *s0;
    func_8003AA78();
    s0 = (u16 *)&D_80101E9E;
    while (1) {
        if (func_80036D88() != 0) {
            break;
        }
        func_8003AA48();
        special_camera_Exec();
        if (D_800A3906 != 0) {
            func_8005C6D0();
        }
        gnd_disp_loop_ctrl();
        *s0 = *s0 + 2;
        sys_VSync(2);
    }
    func_8003AAB0();
}
extern void tslPolyF4Init(s32, u8 *, s32);
s32 func_80036FD4(s32 arg0, s32 arg1) {
    s16 *s0 = &D_80101E62;

    if (*s0 != 0) {
        return 0;
    }

    {
        extern u8 SpecialCam;
        s32 *entry = (s32 *)(&SpecialCam + ((arg0 << 16) >> 13));

        D_80101E60 = arg0;
        D_80101E6C = entry[0];
        D_80101E70 = entry[1];
    }

    asm volatile("" ::: "memory");

    {
        extern u8 SpecialCam;
        D_80101E74 = cdrom_BcdToFrames((s32)(&SpecialCam + D_80101E60 * 8)) + (*(u32 *)((u8 *)&D_8008EC38 + (D_80101E60 << 3)) >> 11) - 0x96;
    }

    if (arg1 < 0) {
        D_80101E94 = 0;
        D_80101E90 = 5;
    } else {
        u8 *base = (u8 *)s0 - 0xA;
        D_80101E94 = 1;
        *base = 1;
        D_80101E59 = arg1;
        tslPolyF4Init(0xD, base, 0);
        D_80101E90 = 0xC8;
    }

    D_80101E64 = 0;
    D_80101E68 = 0;
    D_80101E6A = 0;
    D_80101E62 = 0x10;

    return 1;
}
s32 func_80037110(s32 arg0) {
    u8 *s0 = (u8 *)&D_8008F13C + (arg0 << 3);
    s32 v0;
    v0 = func_80036EA8(5, s0[0]);
    v0 = func_80036FD4(v0, s0[1]);
    if (v0 != 0) {
        if (*(s32 *)(s0 + 4) != -1) {
            v0 = cdrom_BcdToFrames((s32)&SpecialCam + (s32)D_80101E60 * 8);
            D_80101E74 = v0 + *(s32 *)(s0 + 4);
        }
        return 1;
    }
    return 0;
}

s32 func_800371AC(void) {
    s32 ret = func_80037110();
    if (ret) {
        D_80101E64 = 1;
        return 1;
    }
    return 0;
}
void func_800371E8(s16 arg0) {
    D_80101E6A = arg0;
}
s32 func_800371F8(void) {
    if (func_80036FD4() != 0) {
        D_80101E64 = 1;
        return 1;
    }
    return 0;
}
void func_80037234(void) {
    D_80101E64 = 0;
    D_80101E68 = 1;
}
void func_80037250(void) {
    D_80101E64 = 0;
}
void func_80037260(void) {
    while (D_80101E62 != 0x16) {
        func_8003AA48();
        special_camera_Exec();
        sys_VSync(2);
    }
}
void func_800372C0(void) {
    if (D_80101E62 != 0) {
        game_FrameInit();
    }
    game_FrameLoop();
}
void func_800372F4(s32 arg0) {
    s32 v = arg0 + 0x7FF;
    if (v < 0) {
        v = arg0 + 0xFFE;
    }
    func_800826CC(v >> 11);
    do {
        v = func_800827D0(1, 0);
        if (v > 0) {
            sys_VSync(0);
        }
    } while (v > 0);
}
typedef struct { s32 w_q[4]; } Quad;
typedef struct { s32 w_t[3]; } Triple;
extern void func_80080258(s32, s32, s32);
extern void cdrom_FramesToBcd(s32, s32);
void special_camera_get_rot_dir(s32 *dest) {
    extern s32 func_800372F4(s32, s32, s32);
    u8 sp_buf[0x800];
    u8 sp_buf2[8];
    register s32 index asm("s2");
    register s32 cam_base asm("s3");
    s32 v0;
    register s32 constant_80 asm("s4");
    register Quad *copy_end asm("s5");
    s32 *buf2_ptr;

    constant_80 = 0x80;
    index = func_80036EA8(6, 0) << 3;
    cam_base = (s32)&SpecialCam;
    copy_end = (Quad *)&sp_buf[0x40];
    buf2_ptr = (s32 *)sp_buf2;

retry:
    func_80080258(2, index + cam_base, 0);
    v0 = func_800372F4(0x800, (s32)sp_buf, constant_80);
    if (v0 != 0) goto retry;

    {
        Quad *dst_q = (Quad *)dest;
        Quad *src = (Quad *)&sp_buf[0x10];
        do {
            *dst_q = *src;
            src++;
            dst_q++;
        } while (src != copy_end);
        *(Triple *)dst_q = *(Triple *)src;
    }

    v0 = cdrom_BcdToFrames(index + cam_base);
    cdrom_FramesToBcd(v0 + 1, (s32)buf2_ptr);
    func_80080258(2, (s32)buf2_ptr, 0);
    v0 = func_800372F4(dest[3], dest[2], constant_80);
    if (v0 != 0) goto retry;
}
/* kengo:MED  |  nm_special_cam/special_camera_get_rot_dir  |  66i  |  +6 9.1% */
void func_80037468(s32 a0, s32 *a1, s32 a2) {
    s32 sp[16];
    sys_VSync(0);
    gpu_SetDispMask(0);
    gpu_EnableDisplay();
    func_8005B50C();
    func_80037774();
    irq_DisableInterrupts();
    func_8007FF7C();
    special_camera_get_rot_dir(sp);
    gpu_DrawSync(0);
    gpu_SetMode(0);
    pad_Init();
    irq_Reset();
    sp[8] = a2;
    sp[9] = 0;
    EnterCriticalSection();
    func_80078948(sp, a0, a1);
    sys_Init();
    file_LoadSoundData();
    sys_VSync(0);
    gpu_SetDispMask(1);
}
extern s32 func_800392B8(void);
extern void func_80037468(s32, s32 *, s32);
void func_80037540(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4) {
    s32 sp[8];
    s32 v0;

    v0 = func_80036EA8(6, a2);
    sp[0] = (s32)&SpecialCam + v0 * 8;
    sp[1] = a3;
    sp[2] = a0;
    sp[3] = a1;
    v0 = func_80036EA8(6, 2);
    sp[4] = (s32)&SpecialCam + v0 * 8;
    sp[5] = a4;
    v0 = func_800392B8();
    func_80037468(6, sp, v0 + 0x7FC);
}

