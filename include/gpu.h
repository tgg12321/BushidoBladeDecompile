#ifndef GPU_H
#define GPU_H

/* GPU subsystem - display, drawing, math LUTs */

#include "common.h"

/* Named globals */
extern void (*g_gpu_debug_func)();
extern u8 g_gpu_debug_level;
extern u32 *g_gpu_dev_table;
extern s16 g_gpu_disp_x;
extern s16 g_gpu_disp_y;
extern u8 g_gpu_dither;
extern u8 g_gpu_type;

/* Functions */
extern void gpu_DisableDisplay(void);
extern void gpu_EnableDisplay(void);
extern void gpu_InitDisplay(void);

#endif /* GPU_H */
