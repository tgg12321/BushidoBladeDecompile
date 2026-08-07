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
extern void VSync(s32);
extern void LoadImage(s32, s32);
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
extern s32 memcpy(s32 *, s32, s32);
extern void obj_ExecTask(s32);
extern s32 func_8005344C(s32 *, s32 *, s32 *, s32 *);

extern void func_8005B98C(s32);
extern void func_8003AA78(void);
extern void func_8003AA48(void);
extern void func_800174F4(void);
extern void func_8003AAB0(void);
extern s32 D_800A384C;
extern s32 ratan2(s32, s32);
extern s32 D_80101E74;

extern void file_LoadOverlay(void);
extern void func_80040510(s32, s32, s32);
extern void stage_GetDataPtr(void);

extern void func_8005B50C(void);
extern void func_80037774(void);
extern void StopPAD(void);
extern void StopCallback(void);
extern s32 EnterCriticalSection(void);
extern void sys_Init(void);
extern void file_LoadSoundData(void);
extern s32 func_8004939C(void);
extern s32 func_800392B8(void);
extern s16 Judge;
extern s16 D_800A3678;
extern s32 D_800A3708;
extern s32 D_800A374C;
extern s32 D_80106A50;
extern void SetPolyG4(u8 *p);
extern void AddPrim(u32 *a0, u32 *a1);
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
    }
    D_800A31D8 = 1;
    func_80035438();
    func_80077820((s32)0x80118800);
    D_800A37B8 = 0;
    D_800A3834 = 9;
    gpu_DisableDisplay();
}
typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} POLY_G4;

void func_8003553C(void) {
    POLY_G4 *g;
    POLY_G4 *q;
    u32 *ot;

    g = (POLY_G4 *)D_800A38B4;
    SetPolyG4((u8 *)g);
    g->x0 = 0; g->y0 = 0;
    g->x1 = 640; g->y1 = 0;
    g->x2 = 0; g->y2 = 240;
    g->x3 = 640; g->y3 = 240;
    g->r0 = 0; g->g0 = 0; g->b0 = 0x80;
    g->r1 = 0; g->g1 = 0; g->b1 = 0x80;
    g->r2 = 0; g->g2 = 0; g->b2 = 0;
    g->r3 = 0; g->g3 = 0; g->b3 = 0;
    ot = (u32 *)(D_800A374C + 0x401C);
    q = g;
    g += 1;
    AddPrim(ot, (u32 *)q);
    D_800A38B4 = g;
}
void func_800355E8(void) {
    func_80035FA8();
    func_80037110(1);
    func_800371E8(1);
}
