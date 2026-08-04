#ifndef GPU_H
#define GPU_H

/* GPU subsystem - display, drawing, math LUTs */

#include "common.h"

/* PS1 ordering-table / primitive tag word — the head word of PsyQ
 * LIBGPU.H's P_TAG (`unsigned addr:24; unsigned len:8;`): next-packet
 * address in the LOW 24 bits, packet word count in the HIGH 8.
 *
 * FIELD ORDER NOTE (updated 2026-08-04, -mel adoption): cc1psx (PsyQ
 * GCC 2.7.2.SN) allocates the FIRST-declared bitfield at the LOW bits,
 * so PsyQ's addr-first declaration puts addr low. Before 2026-08-04 our
 * fork ran with a big-endian target default and allocated HIGH-first,
 * which forced a reversed (len-first) declaration as compensation
 * (probe-verified 2026-06-11). With -mel in CC_FLAGS the fork allocates
 * LOW-first exactly like cc1psx, so the ORIGINAL PsyQ field order is
 * restored below; see .claude/rules/bitfield-direction-divergence.md. */
typedef struct {
    u32 addr : 24; /* LOW 24 bits: next-packet address */
    u32 len : 8;   /* HIGH 8 bits: packet word count */
} OTag;

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
