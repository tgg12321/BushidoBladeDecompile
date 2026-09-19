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





/* Extern function declarations */



















extern void player_Destroy(s32);
extern void file_ResetDmaFlag(void);
extern void obj_InitAll(void);
extern void func_80077820(s32);
















extern void *D_800A38B4;





























extern s32 D_800A374C;

extern void SetPolyG4(u8 *p);
extern void AddPrim(u32 *a0, u32 *a1);










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
    snd_SerialMixOn();
    func_80037110(1);
    func_800371E8(1);
}
