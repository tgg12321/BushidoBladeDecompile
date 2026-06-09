/* First half of src/code6cac_b2.c (split for Phase B sec.15.1 rodata-cleanup -
 * code6cac_b2 was split into _pre and _post around replay_camera_rob_back_loose2
 * (extracted to its own .c file), preserving sibling function text addresses). */
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
extern s32 EndADRSound(void);
extern s32 obj_InitTaskCamera(s32);
extern void *D_800A38B4;
extern s32 bb2_memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void motion_LoadPreCalcData_8005B98C(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void gnd_disp_loop_ctrl(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 single_game_getEnemyCharId(s32, s32);
extern s32 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void get_point_value(void);
extern void func_80037774(void);
extern void pad_Init(void);
extern void irq_Reset(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 gnd_close_8004939C(void);
extern s32 func_800392B8(void);
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern s32 D_80106A50;
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
