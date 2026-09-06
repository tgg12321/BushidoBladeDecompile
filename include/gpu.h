#ifndef GPU_H
#define GPU_H

/* GPU subsystem - display, drawing, math LUTs */

#include "common.h"

/* PS1 ordering-table / primitive tag word -- the head word of PsyQ
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
/* PsyQ libgpu device table ("gpu" in the SDK's sys.c): a 0x40-byte struct of
 * function pointers, the object at D_8009BE2C, reached through the pointer
 * g_gpu_dev_table (0x8009BE6C).  Member names/offsets are the PsyQ ones; every
 * index used across src/ maps onto them exactly (p[2]=addque2, p[3]=clr,
 * p[5]=cwb, p[6]=cwc, p[7]=drs, p[8]=dws, p[0xB]=otc, p[0xD]=reset,
 * p[0xE]=status, p[0xF]=sync, 0x28/4=getctl, 0x10/4=ctl).
 *
 * The other call sites in display.c/gpu.c keep a `(u32 *)` word view of the
 * same pointer ON PURPOSE: a COMPONENT_REF sets MEM_IN_STRUCT_P (expr.c:4888)
 * where an INDIRECT_REF over a PLUS does not (expr.c:4567), which changes the
 * scheduler's alias classes -- respelling those already-matched bodies as member
 * accesses is a codegen change, not a cosmetic one. */
typedef struct GpuDevTable {
    /* 0x00 */ const char *rcsid;
    /* 0x04 */ void (*addque)();
    /* 0x08 */ s32 (*addque2)();
    /* 0x0C */ s32 (*clr)();
    /* 0x10 */ void (*ctl)();
    /* 0x14 */ s32 (*cwb)();
    /* 0x18 */ void (*cwc)();
    /* 0x1C */ s32 (*drs)();
    /* 0x20 */ s32 (*dws)();
    /* 0x24 */ s32 (*exeque)();
    /* 0x28 */ s32 (*getctl)();
    /* 0x2C */ s32 (*otc)();
    /* 0x30 */ s32 (*param)();
    /* 0x34 */ s32 (*reset)();
    /* 0x38 */ u32 (*status)();
    /* 0x3C */ s32 (*sync)();
} GpuDevTable;

extern GpuDevTable *g_gpu_dev_table;
extern s16 g_gpu_disp_x;
extern s16 g_gpu_disp_y;
extern u8 g_gpu_dither;
extern u8 g_gpu_type;

/* Functions */
extern void gpu_DisableDisplay(void);
extern void gpu_EnableDisplay(void);
extern void gpu_InitDisplay(void);

/* PsyQ libgpu packet queue (sys.c `static volatile struct QueueItem`): 64
 * records of 0x60 bytes {callback, argument pointer, word count, 21 data
 * words}. Evidence for the aggregate: the original code of _addque2 and
 * _exeque scales the queue index by 0x60 (x3 then sll 5) and adds it to
 * these addresses, and the copy loop parks &D_8010368C in a base register
 * and stores through base + i*4 + slot*0x60 -- one object addressed by
 * base + offset, not symbol adjacency. Replaces the splat per-word scalars
 * D_80103680 / D_80103684 / D_80103688 / D_8010368C (the +4/+8/+C rows stay
 * in the symbol config as aliases until _exeque leaves INCLUDE_ASM).
 * volatile: Sony's own qualifier on this object (the queue is drained by
 * _exeque from DMA-IRQ context); grant in volatile_extern_allowlist.txt. */
typedef struct GpuQueueItem {
    /* 0x00 */ s32 (*func)(s32 *, s32);
    /* 0x04 */ s32 *arg;
    /* 0x08 */ s32 count;
    /* 0x0C */ s32 data[21];
} GpuQueueItem; /* size 0x60 */

extern volatile GpuQueueItem D_80103680[64];

#endif /* GPU_H */
