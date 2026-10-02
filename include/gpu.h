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
 * restored below; see pre-slim-2026-10-01:.claude/rules/bitfield-direction-divergence.md. */
typedef struct {
    u32 addr : 24; /* LOW 24 bits: next-packet address */
    u32 len : 8;   /* HIGH 8 bits: packet word count */
} OTag;

/* Named globals */
extern void (*GPU_printf)();
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

/* PsyQ LIBGPU.H environment types (public header layouts). */
typedef struct {
    s16 x, y, w, h;
} RECT;

typedef struct {
    u32 tag;
    u32 code[15];
} DR_ENV; /* 0x40 */

typedef struct {
    RECT clip;     /* +0x00 */
    s16 ofs[2];    /* +0x08 */
    RECT tw;       /* +0x0C */
    u16 tpage;     /* +0x14 */
    u8 dtd;        /* +0x16 */
    u8 dfe;        /* +0x17 */
    u8 isbg;       /* +0x18 */
    u8 r0, g0, b0; /* +0x19 */
    DR_ENV dr_env; /* +0x1C */
} DRAWENV; /* 0x5C */

typedef struct {
    RECT disp;   /* +0x00 */
    RECT screen; /* +0x08 */
    u8 isinter;  /* +0x10 */
    u8 isrgb24;  /* +0x11 */
    u8 pad0;     /* +0x12 */
    u8 pad1;     /* +0x13 */
} DISPENV; /* 0x14 */

/* libgpu SYS state block: one 0x80-byte object at 0x8009BE74 (the one C handle
 * for these bytes). Evidence that it is one object: ResetGraph clears 0x80 bytes
 * from its base and then re-fills +0x10 (0x5C) and +0x6C (0x14); SetDispMask,
 * PutDrawEnv and DrawOTagEnv address disp_env / draw_env off the register that
 * holds &debug_level (+0x6A, +0xE), which cse's related-value addressing only
 * does for offsets of ONE symbol. Member names restate the API that owns each
 * field: GetGraphType/_reset (type), SetGraphQueue (queue_mode), SetGraphDebug
 * (debug_level), SetGraphReverse (reverse; get_dx mirrors x when set),
 * ResetGraph's per-type limit tables + the clamps in checkRECT/get_cs/_clr
 * (width/height), DrawSyncCallback (drawsync_cb), GetDrawEnv/PutDrawEnv
 * (draw_env), GetDispEnv/PutDispEnv (disp_env). unk08 is set to 1 by _addque2. */
typedef struct {
    u8 type;         /* +0x00 */
    u8 queue_mode;   /* +0x01 */
    u8 debug_level;  /* +0x02 */
    u8 reverse;      /* +0x03 */
    s16 width;       /* +0x04 */
    s16 height;      /* +0x06 */
    s32 unk08;       /* +0x08 */
    u32 drawsync_cb; /* +0x0C */
    DRAWENV draw_env; /* +0x10 */
    DISPENV disp_env; /* +0x6C */
} GpuCtx; /* 0x80 */

extern GpuCtx g_gpu_ctx;

/* Functions */
extern void gpu_SetDispMaskOn(void);
extern void gpu_ResetGraphMode1(void);
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

extern volatile GpuQueueItem _que[64];

/* PsyQ libgpu sys.c DR_ENV packet buffer (the `_clr` split-clear / fill
 * packet): one tag word + up to 15 command words at 0x800F1858.  Evidence for
 * the aggregate from the ORIGINAL code of _clr: it materialises &code[8]
 * (0x800F187C) into a base register and stores 0x03FFFFFF through it, and
 * the tag word carries that same address -- one object addressed by base +
 * offset, not thirteen adjacent scalars.  Replaces splat's per-word names
 * D_800F185C..D_800F1888 (retired from the symbol config; this aggregate is
 * the sole handle).  Stock PsyQ DR_ENV is 0x40 bytes;
 * the next object (g_gpu_color_table, 0x800F189C) starts at +0x44. */
typedef struct GpuDrEnv {
    /* 0x00 */ u32 tag;
    /* 0x04 */ u32 code[15];
} GpuDrEnv; /* size 0x40 */

extern GpuDrEnv D_800F1858;


/* PsyQ DR_MOVE: DMA tag word, then five GPU command words. */
typedef struct { u32 tag; u32 code[5]; } DR_MOVE;
/* One VRAM-scroll channel at D_800EF848 (0x134 bytes each). */
typedef struct {
    s32 phase;          /* +0x000 */
    DR_MOVE move[2][6]; /* +0x004: one bank per frame parity, two packets per level */
    s16 ctl[7];         /* +0x124: filled from D_80099C34 by func_80048F58 */
} MoveChannel;

extern MoveChannel D_800EF848[];
extern u16 D_80099C34[][7];

extern void SetDrawMove(DR_MOVE *, RECT *, u32, u32);
/* SDK OT_TYPE: one DMA tag word per table entry. */
extern u32 *D_800A378C;
extern DR_MOVE light_effect_col[31][2];
extern DR_MOVE D_800A4340[19][2];
extern DR_MOVE D_800A9830[2][10];

#endif /* GPU_H */
