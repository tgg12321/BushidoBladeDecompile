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

void func_800375EC(void) {
    func_8007A370(1);
    func_8007A3C8();
    func_80078958();
    func_80078A58(0);
    EnterCriticalSection();
    D_800A37DC = func_80078978(0xF4000001, 4, 0x2000, 0);
    D_800A37F0 = func_80078978(0xF4000001, 0x8000, 0x2000, 0);
    D_800A37FC = func_80078978(0xF4000001, 0x100, 0x2000, 0);
    D_800A3800 = func_80078978(0xF4000001, 0x2000, 0x2000, 0);
    D_800A3838 = func_80078978(0xF0000011, 4, 0x2000, 0);
    D_800A383C = func_80078978(0xF0000011, 0x8000, 0x2000, 0);
    D_800A3848 = func_80078978(0xF0000011, 0x100, 0x2000, 0);
    D_800A3850 = func_80078978(0xF0000011, 0x2000, 0x2000, 0);
    ExitCriticalSection();
    func_800789A8(D_800A37DC);
    func_800789A8(D_800A37F0);
    func_800789A8(D_800A37FC);
    func_800789A8(D_800A3800);
    func_800789A8(D_800A3838);
    func_800789A8(D_800A383C);
    func_800789A8(D_800A3848);
    func_800789A8(D_800A3850);
}
void func_80037774(void) {
    EnterCriticalSection();
    func_80078988(D_800A37DC);
    func_80078988(D_800A37F0);
    func_80078988(D_800A37FC);
    func_80078988(D_800A3800);
    func_80078988(D_800A3838);
    func_80078988(D_800A383C);
    func_80078988(D_800A3848);
    func_80078988(D_800A3850);
    ExitCriticalSection();
    func_8007A400();
}
s32 func_80037804(void) {
    extern s32 D_800A3924;
    s32 result;
    s32 one;
    s32 temp;
    result = (func_80078998(D_800A37DC) == 1);
    one = 1;
    if (func_80078998(D_800A37F0) == one) {
        result = 2;
    }
    if (func_80078998(D_800A37FC) == one) {
        result = 3;
    }
    if (func_80078998(D_800A3800) == one) {
        result = 4;
    }
    temp = D_800A3924;
    D_800A3924 = temp + 1;
    if (temp >= 0x78) {
        result = 2;
    }
    return result;
}
s32 func_800378A8(void) {
    if (func_80078998(D_800A37DC) == 1) {
        return 1;
    }
    if (func_80078998(D_800A37F0) == 1) {
        return 2;
    }
    if (func_80078998(D_800A37FC) == 1) {
        return 3;
    }
    return (func_80078998(D_800A3800) == 1) * 4;
}
void func_8003791C(void) {
    func_80078998(D_800A37DC);
    func_80078998(D_800A37F0);
    func_80078998(D_800A37FC);
    func_80078998(D_800A3800);
}
s32 func_80037964(void) {
    s32 one = 1;
loop:
    if (func_80078998(D_800A3838) == one) { return 1; }
    if (func_80078998(D_800A383C) == one) { return 2; }
    if (func_80078998(D_800A3848) == one) { return 3; }
    if (func_80078998(D_800A3850) != one) { goto loop; }
    return 4;
}
void func_800379D8(void) {
    func_80078998(D_800A3838);
    func_80078998(D_800A383C);
    func_80078998(D_800A3848);
    func_80078998(D_800A3850);
}
s32 func_80037A20(s32 arg0, s32 arg1)
{
  register s32 *var_s0 asm("s0");
  register s32 var_s1 asm("s1");
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (func_80078A38(sp10, var_s0) != 0)
  {
    __asm__("" : "=r"(var_s1) : "0"(var_s1));
    var_s1++;
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
    v0_val = func_80078A48(var_s0);
    var_s1 += 1;
    if (v0_val) goto loop;
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
s32 func_80037AA4(void) {
    s32 var_a1;
    register s32 var_a2 asm("a2");
    register s32 var_a0 asm("a0");
    s8 *var_v1;
    register s32 var_v0 asm("v0");
    s32 sp_dummy[2];

    var_a1 = 0;
    var_a0 = 0;
    var_a2 = D_800A38C8;
    if (var_a2 > 0) {
        var_v1 = (s8 *)&D_80102810;
        do {
            var_v0 = *(s32 *)(var_v1 + 0x18);
            var_a1 += 1;
            var_a0 += var_v0;
            var_v1 += 0x28;
        } while (var_a1 < var_a2);
    }
    var_v0 = var_a0;
    if (var_a0 < 0) {
        var_v0 = var_a0 + 0x1FFF;
    }
    var_a0 = var_v0 >> 0xD;
    return 0xF - var_a0;
}
s32 func_80037B00(u8 *arg0) {
    s32 sp_dummy[2];
    register s32 var_t1 asm("t1");
    register s32 var_t3 asm("t3");
    register s32 var_t2 asm("t2");
    register s8 *var_a3 asm("a3");
    register s8 *var_a1 asm("a1");
    register s8 *var_a2 asm("a2");
    register s8 *var_t0 asm("t0");
    register s32 var_v1 asm("v1");
    register s32 var_v0 asm("v0");

    var_t1 = 0;
    var_v0 = D_800A38C8;
    if (var_v0 <= 0) {
        goto block_end;
    }
    var_t3 = var_v0;
    var_a3 = (s8 *)&D_80102810;
loop_outer:
    var_t2 = 0;
    var_a1 = var_a3;
    var_a2 = (s8 *)arg0;
    var_t0 = var_a3 + 0x15;
loop_inner:
    var_v1 = (u8)*var_a2;
    if (var_v1 == 0) {
        goto block_5c;
    }
    var_v0 = (u8)*var_a1;
    if (var_v1 != var_v0) {
        goto block_6c;
    }
    var_a1 += 1;
    var_a2 += 1;
    if ((s32)var_a1 < (s32)var_t0) {
        goto loop_inner;
    }
block_5c:
    var_t1 += 1;
    if (var_t2 != 0) {
        goto block_74;
    }
    return 1;
block_6c:
    var_t2 = 1;
    goto block_5c;
block_74:
    if (var_t1 < var_t3) {
        var_a3 += 0x28;
        goto loop_outer;
    }
block_end:
    return 0;
}
extern s32 func_800789E8(s32 *, s32);
typedef void (*Func79A30_5)(s32 *, s32 *, s32, s32, s32);
s32 func_80037B90(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 sp18[8];
    register s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    temp_v0 = func_800789E8(sp18, 0x8001);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_800789F8(temp_v0, arg3, arg4);
    return -(func_80037964() != 1);
}
extern void func_80078A18(s32);
extern void func_80078A08(s32, s32, s32);
s32 func_80037C34(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    s32 sp18[8];
    s32 temp_v0;

    ((Func79A30_5)func_80079A30)(sp18, &D_800109BC, arg0, arg1, arg2);
    if (arg6 != 0) {
        temp_v0 = func_800789E8(sp18, (arg4 << 16) | 0x200);
        if (temp_v0 == -1) {
            return -1;
        }
        func_80078A18(temp_v0);
    }
    temp_v0 = func_800789E8(sp18, 0x8002);
    if (temp_v0 == -1) {
        return -1;
    }
    D_800A3794 = temp_v0;
    func_8003791C();
    func_800379D8();
    func_80078A08(temp_v0, arg3, arg5);
    return -(func_80037964() != 1);
}