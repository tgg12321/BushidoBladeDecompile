#ifndef GPU_H
#define GPU_H

/* GPU subsystem - display, drawing, math LUTs */

#include "common.h"
#include <psxsdk/libgpu.h>

/* The game's display double buffer at 0x800F7438: two 0x4090-byte records, one per
 * frame parity (D_800A36AC & 1). disp_Init / func_8006E10C pass +0x00 to
 * SetDefDrawEnv and +0x5C to SetDefDispEnv for each record; main clears +0x70 with
 * ClearOTagR(ot, 0x1008) (0x1008 words = 0x4020 bytes, which ends the record) and
 * draws it from its last entry, DrawOTag(+0x408C). */
typedef struct {
    DRAWENV draw;   /* +0x00 */
    DISPENV disp;   /* +0x5C */
    u32 ot[0x1008]; /* +0x70 ordering table */
} GpuDb; /* 0x4090 */

extern GpuDb g_gpu_db[2];

/* Functions */
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
extern void gpu_InitDisplay(void);

/* One VRAM-scroll channel at D_800EF848 (0x134 bytes each). */
typedef struct {
    s32 phase;          /* +0x000 */
    DR_MOVE move[2][6]; /* +0x004: one bank per frame parity, two packets per level */
    s16 ctl[7];         /* +0x124: filled from D_80099C34 by func_80048F58 */
} MoveChannel;

extern MoveChannel D_800EF848[];
extern u16 D_80099C34[][7];

/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *D_800A378C;
extern DR_MOVE light_effect_col[31][2];
extern DR_MOVE D_800A4340[19][2];
extern DR_MOVE D_800A9830[2][10];
/* 0x800A3220: the VRAM rectangle func_8003D2C4 passes to LoadImage with the
 * image at D_80090178 (x 0x3F0, y 0x1DC, 16 x 36); defined in main/2B344.c. */
extern RECT D_800A3220;

#endif /* GPU_H */
