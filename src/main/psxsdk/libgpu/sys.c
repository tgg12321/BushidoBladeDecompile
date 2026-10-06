/* PsyQ 4.0 LIBGPU SYS: the GPU system layer (ResetGraph .. memset; $Id: sys.c,v 1.129). .text
 * 0x8007AE7C..0x8007DF10, a verbatim LIBSCAN module span (docs/naming/libscan/matches.json), Q106
 * D3. */
/* One file across the old gpu.c|display.c cut at 0x8007B244, which was mid-module (Q106 D3). */
#include "common.h"
#include <psxsdk/libgpu.h>
#include "psx.h"

/* PsyQ libgpu device table ("gpu" in the SDK's sys.c): a 0x40-byte struct of
 * function pointers, the object at D_8009BE2C, reached through the pointer
 * g_gpu_dev_table (0x8009BE6C).  Member names/offsets are the PsyQ ones; every
 * index used across src/ maps onto them exactly (p[2]=addque2, p[3]=clr,
 * p[5]=cwb, p[6]=cwc, p[7]=drs, p[8]=dws, p[0xB]=otc, p[0xD]=reset,
 * p[0xE]=status, p[0xF]=sync, 0x28/4=getctl, 0x10/4=ctl).
 *
 * SetDispMask, DrawSync, ClearImage(2), LoadImage, StoreImage, DrawOTag and
 * PutDrawEnv call through the members (measured byte-identical), as do the
 * other users except ClearOTagR (its word view is the P7c debt row). */
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
 * (draw_env), GetDispEnv/PutDispEnv (disp_env). unk08 is set to 1 by _addque2 and
 * test-and-cleared by _exeque (also the DMA-2 IRQ callback) before it calls drawsync_cb. */
typedef struct {
    u8 type;         /* +0x00 */
    u8 queue_mode;   /* +0x01 */
    u8 debug_level;  /* +0x02 */
    u8 reverse;      /* +0x03 */
    s16 width;       /* +0x04 */
    s16 height;      /* +0x06 */
    volatile s32 unk08; /* +0x08 volatile: grant in volatile_extern_allowlist.txt */
    u32 drawsync_cb; /* +0x0C */
    DRAWENV draw_env; /* +0x10 */
    DISPENV disp_env; /* +0x6C */
} GpuCtx; /* 0x80 */

extern GpuCtx g_gpu_ctx;

/* PsyQ libgpu packet queue (sys.c `static volatile struct QueueItem`): 64
 * records of 0x60 bytes {callback, argument pointer, the callback's second
 * argument (a colour, a pixel pointer or 0), 21 data words}. Evidence for the aggregate: the original code of _addque2 and
 * _exeque scales the queue index by 0x60 (x3 then sll 5) and adds it to
 * these addresses, and the copy loop parks &D_8010368C in a base register
 * and stores through base + i*4 + slot*0x60 -- one object addressed by
 * base + offset, not symbol adjacency. Replaces the splat per-word scalars
 * D_80103680 / D_80103684 / D_80103688 / D_8010368C.
 * volatile: Sony's own qualifier on this object (the queue is drained by
 * _exeque from DMA-IRQ context); grant in volatile_extern_allowlist.txt. */
typedef struct GpuQueueItem {
    /* 0x00 */ s32 (*func)(s32 *, s32);
    /* 0x04 */ s32 *arg;
    /* 0x08 */ s32 cb_arg;  /* the callback's second argument */
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
#include <psxsdk/libetc.h>

/* .rodata 0x80015E28..0x8001605C: this module's strings (moved from src/text1a_b_post_rodata.c, Q106
 * D4: every C reader is in this file, in link order; the leading rcsid is referenced only by SYS's
 * device table D_8009BE2C, asm/data/7D920.data.s). */

/* D_80015E28: 1 string(s), 52B @ 0x80015E28 (the rcsid; only the device table D_8009BE2C points here) */
const char D_80015E28[52] =
    "$Id: sys.c,v 1.129 1996/12/25 03:36:20 noda Exp $\0\0\0"
    ;

/* D_80015E5C: 1 string(s), 32B @ 0x80015E5C */
const char D_80015E5C[32] =
    "ResetGraph:jtb=%08x,env=%08x\n\0\0\0"
    ;

/* D_80015E7C: 1 string(s), 20B @ 0x80015E7C */
const char D_80015E7C[20] =
    "ResetGraph(%d)...\n\0\0"
    ;

/* D_80015E90: 1 string(s), 24B @ 0x80015E90 */
const char D_80015E90[24] =
    "SetGraphReverse(%d)...\n\0"
    ;

/* D_80015EA8: 1 string(s), 44B @ 0x80015EA8 */
const char D_80015EA8[44] =
    "SetGraphDebug:level:%d,type:%d r"
    "everse:%d\n\0\0"
    ;

/* D_80015ED4: 1 string(s), 20B @ 0x80015ED4 */
const char D_80015ED4[20] =
    "SetGrapQue(%d)...\n\0\0"
    ;

/* D_80015EE8: 1 string(s), 28B @ 0x80015EE8 */
const char D_80015EE8[28] =
    "DrawSyncCallback(%08x)...\n\0\0"
    ;

/* g_str_setdispmask: 1 string(s), 20B @ 0x80015F04 */
const char g_str_setdispmask[20] =
    "SetDispMask(%d)...\n\0"
    ;

/* g_str_drawsync: 1 string(s), 20B @ 0x80015F18 */
const char g_str_drawsync[20] =
    "DrawSync(%d)...\n\0\0\0\0"
    ;

/* D_80015F2C: 1 string(s), 12B @ 0x80015F2C */
const char D_80015F2C[12] =
    "%s:bad RECT\0"
    ;

/* D_80015F38: 1 string(s), 20B @ 0x80015F38 */
const char D_80015F38[20] =
    "(%d,%d)-(%d,%d)\n\0\0\0\0"
    ;

/* D_80015F4C: 1 string(s), 4B @ 0x80015F4C */
const char D_80015F4C[4] =
    "%s:\0"
    ;

/* g_str_clearimage: 1 string(s), 12B @ 0x80015F50 */
const char g_str_clearimage[12] =
    "ClearImage\0\0"
    ;

/* g_str_loadimage: 1 string(s), 12B @ 0x80015F5C */
const char g_str_loadimage[12] =
    "LoadImage\0\0\0"
    ;

/* g_str_storeimage: 1 string(s), 12B @ 0x80015F68 */
const char g_str_storeimage[12] =
    "StoreImage\0\0"
    ;

/* D_80015F74: 1 string(s), 12B @ 0x80015F74 */
const char D_80015F74[12] =
    "MoveImage\0\0\0"
    ;

/* g_str_clearotag: 1 string(s), 24B @ 0x80015F80 */
const char g_str_clearotag[24] =
    "ClearOTag(%08x,%d)...\n\0\0"
    ;

/* D_80015F98: 1 string(s), 24B @ 0x80015F98 */
const char D_80015F98[24] =
    "ClearOTagR(%08x,%d)...\n\0"
    ;

/* g_str_drawotag: 1 string(s), 20B @ 0x80015FB0 */
const char g_str_drawotag[20] =
    "DrawOTag(%08x)...\n\0\0"
    ;

/* g_str_putdrawenv: 1 string(s), 24B @ 0x80015FC4 */
const char g_str_putdrawenv[24] =
    "PutDrawEnv(%08x)...\n\0\0\0\0"
    ;

/* D_80015FDC: 1 string(s), 28B @ 0x80015FDC */
const char D_80015FDC[28] =
    "DrawOTagEnv(%08x,&08x)...\n\0\0"
    ;

/* D_80015FF8: 1 string(s), 24B @ 0x80015FF8 */
const char D_80015FF8[24] =
    "PutDispEnv(%08x)...\n\0\0\0\0"
    ;

/* g_str_gpu_timeout: 1 string(s), 52B @ 0x80016010 */
const char g_str_gpu_timeout[52] =
    "GPU timeout:que=%d,stat=%08x,chc"
    "r=%08x,madr=%08x,\0\0\0"
    ;

/* D_80016044: 1 string(s), 24B @ 0x80016044 */
const char D_80016044[24] =
    "func=(%08x)(%08x,%08x)\n\0"
    ;

/* Forward declarations */
extern s32 memcpy(s32, void *, s32);
s32 get_mode(s32, s32, s32);
s32 get_cs(s16, s16);
s32 get_ce(s16, s16);
s32 get_ofs(s32, s32);

/* Externs for globals */
extern volatile u32 *g_gpu_stat_reg;
extern volatile u32 *g_gpu_data_reg;
extern volatile u32 *g_gpu_dma_madr;
extern volatile u32 *g_gpu_dma_bcr;
extern volatile u32 *g_gpu_dma_chcr;
extern u8 ctlbuf[];
extern s32 g_gpu_vcount;
extern s32 g_gpu_draw_count;

/* Declarations from the file ResetGraph .. GetGraphDebug were split from (gpu.c). */
extern s32 D_8009BE2C;
extern s32 D_8009BEF4[];
extern s32 D_8009BF08[];

/* PsyQ LIBGPU sys.c v1.129: ResetGraph — verbatim-linked Sony object;
   C ref: sotn-decomp src/main/psxsdk/libgpu/sys.c */
u32 ResetGraph(s32 a0) {
    switch (a0 & 7) {
    case 0:
    case 3:
        printf(&D_80015E5C, &D_8009BE2C, &g_gpu_ctx);
        /* fallthrough */
    case 5:
        memset(&g_gpu_ctx, 0, sizeof(g_gpu_ctx));
        ResetCallback();
        GPU_cw((u32)g_gpu_dev_table & 0xFFFFFF);
        g_gpu_ctx.type = _reset(a0);
        g_gpu_ctx.queue_mode = 1;
        g_gpu_ctx.width = D_8009BEF4[g_gpu_ctx.type];
        g_gpu_ctx.height = D_8009BF08[g_gpu_ctx.type];
        memset(&g_gpu_ctx.draw_env, -1, sizeof(DRAWENV));
        memset(&g_gpu_ctx.disp_env, -1, sizeof(DISPENV));
        return g_gpu_ctx.type;
    default:
        if (g_gpu_ctx.debug_level >= 2) {
            GPU_printf(&D_80015E7C, a0);
        }
        g_gpu_dev_table->reset(1);
        break;
    }
}
u32 SetGraphReverse(s32 a0) {
    u32 old = g_gpu_ctx.reverse;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015E90, a0);
    }
    g_gpu_ctx.reverse = a0;
    g_gpu_dev_table->ctl(0x08000000 | (g_gpu_ctx.reverse ? 0x80 : 0) | g_gpu_dev_table->getctl(8));
    if (g_gpu_ctx.type == 2) {
        g_gpu_dev_table->ctl(0x20000000 | (g_gpu_ctx.reverse ? 0x501 : 0x504));
    }
    return old;
}

u32 SetGraphDebug(s32 a0) {
    u32 old = g_gpu_ctx.debug_level;
    g_gpu_ctx.debug_level = a0;
    if (g_gpu_ctx.debug_level) {
        GPU_printf(&D_80015EA8, g_gpu_ctx.debug_level, g_gpu_ctx.type,
                   g_gpu_ctx.reverse);
    }
    return old;
}
u32 SetGraphQueue(s32 a0) {
    u32 old = g_gpu_ctx.queue_mode;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015ED4, a0);
    }
    if (a0 != g_gpu_ctx.queue_mode) {
        g_gpu_dev_table->reset(1);
        g_gpu_ctx.queue_mode = a0;
        DMACallback(2, 0);
    }
    return old;
}

u32 GetGraphType(void) {
    return g_gpu_ctx.type;
}

u32 GetGraphDebug(void) {
    return g_gpu_ctx.debug_level;
}

u32 DrawSyncCallback(s32 a0) {
    u32 old;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015EE8, a0);
    }
    old = g_gpu_ctx.drawsync_cb;
    g_gpu_ctx.drawsync_cb = a0;
    return old;
}

void SetDispMask(s32 a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_setdispmask, a0);
    }
    if (!a0) {
        memset(&g_gpu_ctx.disp_env, -1, 0x14);
    }
    g_gpu_dev_table->ctl(a0 ? 0x03000000 : 0x03000001);
}
void DrawSync(s32 a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_drawsync, a0);
    }
    g_gpu_dev_table->sync(a0);
}
void checkRECT(const char *str, RECT *rect) {
    s16 w, x, y, h;
    if (g_gpu_ctx.debug_level == 1) goto level_1;
    if (g_gpu_ctx.debug_level == 2) goto level_2;
    goto end;
level_1:
    w = rect->w;
    if (w > g_gpu_ctx.width) goto bad;
    x = rect->x;
    if (w + x > g_gpu_ctx.width) goto bad;
    y = rect->y;
    if (y > g_gpu_ctx.height) goto bad;
    h = rect->h;
    if (y + h > g_gpu_ctx.height) goto bad;
    if (w <= 0) goto bad;
    if (x < 0) goto bad;
    if (y < 0) goto bad;
    if (h > 0) goto end;
bad:
    GPU_printf(D_80015F2C, str);
    GPU_printf(D_80015F38, rect->x, rect->y, rect->w, rect->h);
    goto end;
level_2:
    GPU_printf(D_80015F4C, str);
    GPU_printf(D_80015F38, rect->x, rect->y, rect->w, rect->h);
end:
    ;
}
extern void checkRECT(const char *, RECT *);

void ClearImage(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {
    checkRECT(g_str_clearimage, arg0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->clr, arg0, 8, ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}
void ClearImage2(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {
    checkRECT(g_str_clearimage, arg0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->clr, arg0, 8,
                             0x80000000 | ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}

void LoadImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_loadimage, a0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->dws, a0, 8, a1);
}

void StoreImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_storeimage, a0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->drs, a0, 8, a1);
}
extern u32 g_gpu_move_param[5];

s32 MoveImage(RECT *rect, int x, int y) {
    s32 packed;

    checkRECT(D_80015F74, rect);
    if (rect->w == 0 || rect->h == 0) {
        return -1;
    }
    packed = (u16)y << 16 | (u16)x;
    g_gpu_move_param[2] = *(u32 *)&rect->x;
    g_gpu_move_param[3] = packed;
    g_gpu_move_param[4] = *(u32 *)&rect->w;
    return g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, g_gpu_move_param,
                                    sizeof(g_gpu_move_param), 0);
}

extern u32 g_gpu_ot_end;

u32 *ClearOTag(u32 *a0, s32 a1) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_clearotag, a0, a1);
    }
    while (--a1) {
        setlen(a0, 0);
        setaddr(a0, a0 + 1);
        a0++;
    }
    *a0 = (u32)&g_gpu_ot_end & 0xFFFFFF;
    return a0;
}

u32 *ClearOTagR(u32 *ot, s32 n) {
    u32 *new_var;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015F98, ot, n);
        new_var = ot; /* FAKE: cse.c make_regs_eqv beyond-block gate; flow-deleted pre-RA */
    }
    {
        u32 *v0 = (u32 *)g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    new_var = ot;
    *new_var = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return new_var;
}
void DrawPrim(u8 *a0) {
    u32 size = a0[3];
    g_gpu_dev_table->sync(0);
    g_gpu_dev_table->cwb(a0 + 4, size);
}
void DrawOTag(u32 *a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_drawotag, a0);
    }
    g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, a0, 0, 0);
}


DRAWENV *PutDrawEnv(DRAWENV *env) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_putdrawenv, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    env->dr_env.tag |= 0xFFFFFF;
    g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, &env->dr_env, 0x40, 0);
    g_gpu_ctx.draw_env = *env;
    return env;
}
void DrawOTagEnv(s32 arg0, DRAWENV *env) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    env->dr_env.tag = (env->dr_env.tag & 0xFF000000) | (arg0 & 0xFFFFFF);
    g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, &env->dr_env, 0x40, 0);
    g_gpu_ctx.draw_env = *env;
}
s32 GetDrawEnv(s32 a0) {
    memcpy(a0, &g_gpu_ctx.draw_env, 0x5C);
    return a0;
}
s32 get_dx(DISPENV *env);
/* PsyQ 4.0 LIBGPU SYS: PutDispEnv (verbatim-linked Sony object);
   C ref: SOTN src/main/psxsdk/libgpu/sys.c:336 @aa53500 (a PsyQ 3.3 build; structure only) */
DISPENV *PutDispEnv(DISPENV *env) {
    s32 h_start, h_end;
    s32 v_start, v_end;
    s32 mode;

    mode = 0x08000000;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(D_80015FF8, env);
    }
    g_gpu_dev_table->ctl(
        g_gpu_ctx.type == 1 || g_gpu_ctx.type == 2
            ? ((env->disp.y & 0xFFF) << 12) | (get_dx(env) & 0xFFF) | 0x05000000
            : ((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF) |
                  0x05000000);
    /* FAKE: volatile-qualified reads of the saved environment (8 casts, both rect
       compares). The shipped bytes load every one of these fields as lhu + sll 16 +
       sra 16 -- the un-folded extend GCC keeps only for a volatile halfword -- while
       the env-> side of the same compares is a plain lh; the non-volatile spelling
       folds to lh. Admitted on a cross-function citation (owner ruling Q100): the
       same use-site `*(volatile T *)&` read of a plain-RAM struct member, matched and
       self-marked in SOTN ("Why the volatile?"); SOTN's own PutDispEnv (3.3) compares
       words and has none. The plain reads: score 71.
       SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @db41b28 */
    if (!(*(volatile s16 *)&g_gpu_ctx.disp_env.screen.x == env->screen.x &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.y == env->screen.y &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.w == env->screen.w &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.h == env->screen.h)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        /* each value clamped in place. SOTN: src/main/psxsdk/libgpu/sys.c:358 @aa53500 */
        h_start = h_start < 500 ? 500 : (h_start > 3290 ? 3290 : h_start);
        h_end = h_end < h_start + 0x50 ? h_start + 0x50
                                       : (h_end > 3290 ? 3290 : h_end);
        v_start = v_start < 0x10 ? 0x10
                : (v_start > (env->pad0 ? 310 : 256) ? (env->pad0 ? 310 : 256)
                                                     : v_start);
        v_end = v_end < v_start + 2 ? v_start + 2
              : (v_end > (env->pad0 ? 312 : 258) ? (env->pad0 ? 312 : 258)
                                                 : v_end);
        g_gpu_dev_table->ctl(
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        g_gpu_dev_table->ctl(
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    /* isinter..pad1 compared as one word: the shipped bytes are a single lw at
       +0x10 on both sides. SOTN: src/main/psxsdk/libgpu/sys.c:367 @aa53500
       (the same compare of the saved and new environment, through LOW() =
       `*(s32 *)&`, SOTN include/common.h:73). */
    if (*(s32 *)&g_gpu_ctx.disp_env.isinter != *(s32 *)&env->isinter ||
        !(*(volatile s16 *)&g_gpu_ctx.disp_env.disp.x == env->disp.x &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.disp.y == env->disp.y &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.disp.w == env->disp.w &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.disp.h == env->disp.h)) {
        env->pad0 = GetVideoMode();
        if (env->pad0 == 1) {
            mode |= 0x8;
        }
        if (env->isrgb24) {
            mode |= 0x10;
        }
        if (env->isinter) {
            mode |= 0x20;
        }
        if (g_gpu_ctx.reverse) {
            mode |= 0x80;
        }
        if (env->disp.w > 280) {
            if (env->disp.w <= 352) {
                mode |= 1;
            } else if (env->disp.w <= 400) {
                mode |= 0x40;
            } else if (env->disp.w <= 560) {
                mode |= 2;
            } else {
                mode |= 3;
            }
        }
        /* FAKE: empty then-arm; the direct `if (env->disp.h > ...) mode |= 0x24;`
           and its respellings add 4 insns (score 5).
           SOTN: src/main/psxsdk/libgpu/sys.c:394 @aa53500 (same statement, same form). */
        if (env->disp.h <= (env->pad0 ? 288 : 256)) {
        } else {
            mode |= 0x24;
        }
        g_gpu_dev_table->ctl(mode);
    }
    memcpy((s32)&g_gpu_ctx.disp_env, env, sizeof(DISPENV));
    return env;
}
s32 GetDispEnv(s32 a0) {
    memcpy(a0, &g_gpu_ctx.disp_env, 0x14);
    return a0;
}
u32 GetODE(void) {
    return g_gpu_dev_table->status() >> 31;
}
void SetTexWindow(DR_TWIN *p, RECT *tw) {
    setlen(p, 2);
    p->code[0] = get_tw(tw);
    p->code[1] = 0;
}
void SetDrawArea(DR_AREA *p, RECT *r) {
    setlen(p, 2);
    p->code[0] = get_cs(r->x, r->y);
    p->code[1] = get_ce(r->x + r->w - 1, r->y + r->h - 1);
}
/* PsyQ: u_short *ofs; this build loads the two offsets signed (lh; u16 * gives lhu). */
void SetDrawOffset(DR_OFFSET *p, s16 *ofs) {
    setlen(p, 2);
    p->code[0] = get_ofs(ofs[0], ofs[1]);
    p->code[1] = 0;
}
void SetPriority(DR_PRIO *p, s32 a1, s32 a2) {
    u32 v0;
    setlen(p, 2);
    v0 = 0xE6000000;
    if (a1) {
        v0 = 0xE6000002;
    }
    if (a2) {
        v0 |= 1;
    }
    p->code[0] = v0;
    p->code[1] = 0;
}
void SetDrawMode(DR_MODE *p, s32 dfe, s32 dtd, s32 tpage, RECT *tw) {
    setlen(p, 2);
    p->code[0] = get_mode(dfe, dtd, (u16)tpage);
    p->code[1] = get_tw(tw);
}
void SetDrawEnv(DR_ENV *out, DRAWENV *r)
{
  DR_ENV *o = out; /* FAKE: prologue pair order — single forward-order param
                   alias (pointer-alias-fake-exception; owner ruling for the
                   twins SetDrawEnv / SetDrawEnv2). cc1 combine's single-use
                   entry-copy merge relocates arg0's `move s1,a0` past arg1's
                   `move s0,a1`, flipping the prologue save+def pair
                   emit order to match target (s0-pair first). An arg1
                   alias, K&R decl-block reversal or do-while(0) entry wrap
                   leaves the pair order unchanged. Without it: score 4. */
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  o->code[0] = get_cs(r->clip.x, r->clip.y);
  o->code[1] = get_ce(r->clip.w + r->clip.x - 1, r->clip.y + r->clip.h - 1);
  o->code[2] = get_ofs(r->ofs[0], r->ofs[1]);
  o->code[3] = get_mode(r->dfe, r->dtd, r->tpage);
  o->code[4] = get_tw(&r->tw);
  o->code[5] = 0xE6000000;
  /* FAKE: var_a3 counts the tag word, and the pushes are pointer arithmetic
     (*(o->code + var_a3++ - 1)); the subscript o->code[var_a3++ - 1] adds the base
     first (addu aN,s1,aN for addu aN,aN,s1 at every push): score 3 in SetDrawEnv,
     6 in SetDrawEnv2. */
  var_a3 = 7;
  if (r->isbg != 0)
  {
    buf[0] = r->clip.x;
    buf[1] = r->clip.y;
    new_var = r->clip.w;
    buf[2] = r->clip.w;
    buf[3] = r->clip.h;
    if (new_var >= 0)
    {
      if ((g_gpu_ctx.width - 1) < new_var)
      {
        var_v0 = g_gpu_ctx.width - 1;
      }
      else
      {
        var_v0 = new_var;
      }
    }
    else
    {
      var_v0 = 0;
    }
    buf[2] = (u16) var_v0;
    if (((s16) buf[3]) >= 0)
    {
      if ((g_gpu_ctx.height - 1) < ((s16) buf[3]))
      {
        var_v0_2 = g_gpu_ctx.height - 1;
      }
      else
      {
        var_v0_2 = (s16) buf[3];
      }
    }
    else
    {
      var_v0_2 = 0;
    }
    buf[3] = (u16) var_v0_2;
    buf[0] -= r->ofs[0];
    buf[1] -= r->ofs[1];
    *(o->code + var_a3++ - 1) = (((r->b0 << 16) | 0x60000000) | (r->g0 << 8)) | r->r0;
    *(o->code + var_a3++ - 1) = ((u32 *) buf)[0];
    *(o->code + var_a3++ - 1) = ((u32 *) buf)[1];
    buf[0] += r->ofs[0];
    buf[1] += r->ofs[1];
  }
  setlen(o, var_a3 - 1);
}
void SetDrawEnv2(DR_ENV *out, DRAWENV *r)
{
  DR_ENV *o = out; /* FAKE: prologue pair order — same param alias and
                   mechanism as SetDrawEnv above (owner ruling for the twins):
                   cc1 combine's single-use entry-copy merge relocates arg0's
                   `move s1,a0` past arg1's `move s0,a1`, flipping the
                   prologue save+def pair emit order to match target
                   (s0-pair first). Without it: score 4. */
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  o->code[0] = get_cs(r->clip.x, r->clip.y);
  o->code[1] = get_ce(r->clip.w + r->clip.x - 1, r->clip.y + r->clip.h - 1);
  o->code[2] = get_ofs(r->ofs[0], r->ofs[1]);
  o->code[3] = get_mode(r->dfe, r->dtd, r->tpage);
  o->code[4] = get_tw(&r->tw);
  o->code[5] = 0xE6000000;
  /* FAKE: var_a3 counts the tag word, and the pushes are pointer arithmetic
     (*(o->code + var_a3++ - 1)); the subscript o->code[var_a3++ - 1] adds the base
     first (addu aN,s1,aN for addu aN,aN,s1 at every push): score 3 in SetDrawEnv,
     6 in SetDrawEnv2. */
  var_a3 = 7;
  if (r->isbg != 0)
  {
    buf[0] = r->clip.x;
    buf[1] = r->clip.y;
    new_var = r->clip.w;
    buf[2] = r->clip.w;
    buf[3] = r->clip.h;
    if (new_var >= 0)
    {
      if ((g_gpu_ctx.width - 1) < new_var)
      {
        var_v0 = g_gpu_ctx.width - 1;
      }
      else
      {
        var_v0 = new_var;
      }
    }
    else
    {
      var_v0 = 0;
    }
    buf[2] = (u16) var_v0;
    if (((s16) buf[3]) >= 0)
    {
      if ((g_gpu_ctx.height - 1) < ((s16) buf[3]))
      {
        var_v0_2 = g_gpu_ctx.height - 1;
      }
      else
      {
        var_v0_2 = (s16) buf[3];
      }
    }
    else
    {
      var_v0_2 = 0;
    }
    buf[3] = (u16) var_v0_2;
    if ((buf[0] & 0x3F) || (buf[2] & 0x3F))
    {
      buf[0] -= r->ofs[0];
      buf[1] -= r->ofs[1];
      *(o->code + var_a3++ - 1) = (((r->b0 << 16) | 0x60000000) | (r->g0 << 8)) | r->r0;
      *(o->code + var_a3++ - 1) = ((u32 *) buf)[0];
      *(o->code + var_a3++ - 1) = ((u32 *) buf)[1];
      buf[0] += r->ofs[0];
      buf[1] += r->ofs[1];
    }
    else
    {
      *(o->code + var_a3++ - 1) = (((r->b0 << 16) | 0x02000000) | (r->g0 << 8)) | r->r0;
      *(o->code + var_a3++ - 1) = ((u32 *) buf)[0];
      *(o->code + var_a3++ - 1) = ((u32 *) buf)[1];
    }
  }
  setlen(o, var_a3 - 1);
}
s32 get_mode(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_v1;
    s32 var_v0;

    if ((u32) (g_gpu_ctx.type - 1) < 2U) {
        var_v1 = 0xE1000000;
        if (arg1 != 0) {
            var_v1 = 0xE1000800;
        }
        if (arg0 != 0) {
            var_v0 = (arg2 & 0x27FF) | 0x1000;
        } else {
            var_v0 = arg2 & 0x27FF;
        }
    } else {
        var_v1 = 0xE1000000;
        if (arg1 != 0) {
            var_v1 = 0xE1000200;
        }
        var_v0 = arg2 & 0x9FF;
        if (arg0 != 0) {
            var_v0 |= 0x400;
        }
    }
    return var_v1 | var_v0;
}
/* PsyQ libgpu get_cs (verbatim-linked Sony object).
 * Body: the published psxsdk clamp idiom (sotn-decomp
 * src/main/psxsdk/libgpu/sys.c house style) with THIS library build's limits
 * and dispatch — clamping both axes against the halfword globals
 * g_gpu_ctx.width/g_gpu_ctx.height and dispatching on the g_gpu_ctx.type range check.
 * It is NOT SOTN's get_cs verbatim: that build clamps against constants and
 * dispatches on a boolean global (different library build). */
s32 get_cs(s16 x, s16 y)
{
    x = x < 0 ? 0 : (x > g_gpu_ctx.width - 1 ? g_gpu_ctx.width - 1 : x);
    y = y < 0 ? 0 : (y > g_gpu_ctx.height - 1 ? g_gpu_ctx.height - 1 : y);
    if ((u32)(g_gpu_ctx.type - 1) < 2U) {
        return 0xE3000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE3000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}
/* PsyQ libgpu get_ce, the get_cs twin (verbatim-linked Sony object, census
 * 2026-07-09). Same published psxsdk clamp idiom as get_cs above, with
 * the packet constant 0xE4000000; the limits (g_gpu_ctx.width/g_gpu_ctx.height), the
 * dispatch (g_gpu_ctx.type range check) and both arms' masks/shifts were read off
 * THIS function's own target bytes (asm/funcs/get_ce.s), not assumed
 * symmetric. Not SOTN's get_ce verbatim — different library build. */
s32 get_ce(s16 x, s16 y)
{
    x = x < 0 ? 0 : (x > g_gpu_ctx.width - 1 ? g_gpu_ctx.width - 1 : x);
    y = y < 0 ? 0 : (y > g_gpu_ctx.height - 1 ? g_gpu_ctx.height - 1 : y);
    if ((u32)(g_gpu_ctx.type - 1) < 2U) {
        return 0xE4000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE4000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}
s32 get_ofs(s32 arg0, s32 arg1) {
    s32 var_v0;
    s32 var_v1;
    int new_var2;
    var_v1 = arg1 & 0xFFF;
    new_var2 = arg0;
    if ((u32) (g_gpu_ctx.type - 1) >= 2U) {
        var_v1 = arg1 & 0x7FF;
        var_v1 = var_v1 << 0xB;
        var_v0 = new_var2 & 0x7FF;
    } else {
        var_v1 = var_v1 << 0xC;
        var_v0 = new_var2 & 0xFFF;
    }
    new_var2 = 0xE5000000;
    return var_v1 | (var_v0 | new_var2);
}
s32 get_tw(RECT *tw) {
    if (tw != 0) {
        u32 tmp[4]; /* FAKE: written-never-read scratch (SOTN dra/62DEC.c sp70[4] family;
                       dead-vars-local-array carve-out) */
        u8 r, b1;
        s32 g, b2;
        u32 b15, re2, ret;
        r = (tw->x & 0xFF) >> 3;
        tmp[0] = r;
        g = ((-tw->w) & 0xFF) >> 3;
        tmp[2] = g;
        b1 = (tw->y & 0xFF) >> 3;
        tmp[1] = b1;
        b15 = (u32)b1 << 0xF;
        b2 = ((-tw->h) & 0xFF) >> 3;
        re2 = ((u32)r << 0xA) | 0xE2000000u;
        ret = b15 | re2 | ((u32)b2 << 5) | (u32)g;
        tmp[3] = b2;
        return ret;
    }
    return 0;
}
s32 get_dx(DISPENV *env) {
    s32 v1, a, t;
    switch (g_gpu_ctx.type) {
    case 1:
        if (g_gpu_ctx.reverse != 0) {
            t = 0x400;
            v1 = env->disp.w;
            a = env->disp.x;
        sub:
            t = t - v1;
            return t - a;
        }
        t = env->disp.x;
        goto ret;
    case 2:
        if (0 != g_gpu_ctx.reverse) {
            v1 = env->disp.w / 2;
            a = env->disp.x;
            /* FAKE: wrap keeps the 0x400 load below the div chain so it
               fills the jump delay slot instead of hoisting to block top
               (score 10) */
            do { t = 0x400; } while (0);
            goto sub;
        }
        t = env->disp.x / 2;
        goto ret;
    default:
        t = env->disp.x;
    ret:
        return t;
    }
}
u32 _status(void) {
    return *g_gpu_stat_reg;
}
extern void set_alarm(void);
extern s32 get_alarm();
extern volatile s32 *D_8009BF58;
extern volatile s32 *D_8009BF5C;
extern volatile s32 *D_8009BF60;
extern volatile s32 *D_8009BF64;
s32 _otc(s32 arg0, s32 arg1) {
    *D_8009BF64 |= 0x08000000;
    *D_8009BF60 = 0;
    *D_8009BF58 = (arg0 - 4) + (arg1 * 4);
    *D_8009BF5C = arg1;
    *D_8009BF60 = 0x11000002;
    set_alarm();
    if (*D_8009BF60 & 0x01000000) {
        do {
            if (get_alarm() != 0) {
                return -1;
            }
        } while (*D_8009BF60 & 0x01000000);
    }
    return arg1;
}

void _cwc(u32 a0);
u32 _param(u32 a0);

/* PsyQ libgpu sys.c `_clr` - the GPU-side body of ClearImage(): clamp the
 * rect to the VRAM page, build either a 12-word unaligned (mono rectangle)
 * or 5-word aligned (VRAM fill) packet in the DR_ENV buffer, and DMA it.
 * Spelling follows psyz decomp/src/libgpu/sys.c:706-741 (PsyQ 4.0). */
s32 _clr(RECT *rect, u32 color) {
    u32 ptr;

    rect->w = rect->w < 0 ? 0 : (rect->w > g_gpu_ctx.width - 1 ? g_gpu_ctx.width - 1 : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > g_gpu_ctx.height - 1 ? g_gpu_ctx.height - 1 : rect->h);
    if (rect->x & 0x3F || rect->w & 0x3F) {
        /* unaligned clear: split in two packets */
        ptr = (u32)&D_800F1858.code[8];
        D_800F1858.tag = (ptr & 0xFFFFFF) | 0x08000000;
        D_800F1858.code[0] = 0xE3000000;
        D_800F1858.code[1] = 0xE4FFFFFF;
        D_800F1858.code[2] = 0xE5000000;
        D_800F1858.code[3] = 0xE6000000;
        D_800F1858.code[4] = 0xE1000000 | *g_gpu_stat_reg & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[5] = (color & 0xFFFFFF) | 0x60000000;
        D_800F1858.code[6] = *(s32 *)&rect->x;
        D_800F1858.code[7] = *(s32 *)&rect->w;
        D_800F1858.code[8] = 0xFFFFFF | 0x03000000;
        D_800F1858.code[9] = _param(3) | 0xE3000000;
        D_800F1858.code[10] = _param(4) | 0xE4000000;
        D_800F1858.code[11] = _param(5) | 0xE5000000;
    } else {
        /* aligned clear */
        D_800F1858.tag = 0xFFFFFF | 0x05000000;
        D_800F1858.code[0] = 0xE6000000;
        D_800F1858.code[1] = 0xE1000000 | *g_gpu_stat_reg & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[2] = (color & 0xFFFFFF) | 0x02000000;
        D_800F1858.code[3] = *(s32 *)&rect->x;
        D_800F1858.code[4] = *(s32 *)&rect->w;
    }
    _cwc((u32)&D_800F1858);
    return 0;
}

/* PsyQ libgpu sys.c `_dws` - "data write short", the GPU-side body of
 * LoadImage(): clamp the destination rect to the VRAM page, push the
 * CPU->VRAM copy command plus the odd (non-multiple-of-16) leading words
 * through GP0, then hand the 16-word-aligned bulk to DMA channel 2.
 * Reconstructed from the two version-correct matching decomps of this same
 * Sony function: sotn-decomp src/main/psxsdk/libgpu/sys.c:608-655 (PSX,
 * GCC 2.7.2) and psyz decomp/src/libgpu/sys.c:745-785 (PsyQ 4.0).  Both ship
 * the same `var_s4` transfer-direction selector and the same `% 16` / `/ 16`
 * split; the spelling here follows them. */
s32 _dws(RECT *rect, s32 *data) {
    s32 to_write;
    s32 size;
    s32 var_s0;
    s32 var_s4;

    var_s4 = 0;
    set_alarm();
    rect->w = rect->w < 0 ? 0 : (rect->w > g_gpu_ctx.width ? g_gpu_ctx.width : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > g_gpu_ctx.height ? g_gpu_ctx.height : rect->h);
    to_write = (rect->w * rect->h + 1) / 2;
    if (to_write <= 0) {
        return -1;
    }
    var_s0 = to_write % 16;
    size = to_write / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = var_s4 ? 0xB0000000 : 0xA0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (--var_s0 != -1) {
        *g_gpu_data_reg = *data++;
    }

    if (size) {
        *g_gpu_stat_reg = 0x04000002;
        *g_gpu_dma_madr = (u32)data;
        *g_gpu_dma_bcr = size << 16 | 0x10;
        *g_gpu_dma_chcr = 0x01000201;
    }
    return 0;
}

/* PsyQ libgpu sys.c `_drs` - "data read short", the GPU-side body of
 * StoreImage(): clamp the source rect to the VRAM page, push the VRAM->CPU
 * copy command through GP0, wait for the GPU to be ready to send, read the
 * odd (non-multiple-of-16) leading words through GP0, then hand the
 * 16-word-aligned bulk to DMA channel 2.  Same shape as _dws above; spelling
 * follows psyz decomp/src/libgpu/sys.c:787-833 (PsyQ 4.0). */
s32 _drs(RECT *rect, s32 *data) {
    s32 to_read;
    s32 size;
    s32 var_s0;

    set_alarm();
    rect->w = rect->w < 0 ? 0 : (rect->w > g_gpu_ctx.width ? g_gpu_ctx.width : rect->w);
    rect->h = rect->h < 0 ? 0 : (rect->h > g_gpu_ctx.height ? g_gpu_ctx.height : rect->h);
    to_read = (rect->w * rect->h + 1) / 2;
    if (to_read <= 0) {
        return -1;
    }
    var_s0 = to_read % 16;
    size = to_read / 16;

    while (!(*g_gpu_stat_reg & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = 0xC0000000;
    *g_gpu_data_reg = *(s32 *)&rect->x;
    *g_gpu_data_reg = *(s32 *)&rect->w;

    while (!(*g_gpu_stat_reg & 0x08000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    while (--var_s0 != -1) {
        *data++ = *g_gpu_data_reg;
    }

    if (size) {
        *g_gpu_stat_reg = 0x04000003;
        *g_gpu_dma_madr = (u32)data;
        *g_gpu_dma_bcr = size << 16 | 0x10;
        *g_gpu_dma_chcr = 0x01000200;
    }
    return 0;
}
void _ctl(u32 a0) {
    *g_gpu_stat_reg = a0;
    ctlbuf[a0 >> 24] = a0;
}
u32 _getctl(s32 a0) {
    return ctlbuf[a0];
}
s32 _cwb(u32 *a0, s32 a1) {
    s32 i;
    *g_gpu_stat_reg = GP1_DMA_DIR;
    for (i = a1 - 1; i != -1; i--) {
        *g_gpu_data_reg = *a0++;
    }
    return 0;
}
void _cwc(u32 a0) {
    *g_gpu_stat_reg = GP1_DMA_DIR_FIFO;
    *g_gpu_dma_madr = a0;
    *g_gpu_dma_bcr = 0;
    *g_gpu_dma_chcr = DMA_GPU_LINKED_LIST;
}
u32 _param(u32 a0) {
    *g_gpu_stat_reg = a0 | GP1_GPU_INFO;
    return *g_gpu_data_reg & OT_ADDR_MASK;
}
void _addque(s32 a0, s32 a1, s32 a2) {
    _addque2(a0, a1, 0, a2);
}
extern volatile s32 _qin;
extern volatile s32 _qout;


s32 _exeque(void);
s32 get_alarm();                                /* extern */
s32 DMACallback(s32, s32 (*)()); /* extern */
s32 SetIntrMask(s32);                         /* extern */
extern volatile s32 _qlog[];
extern s32 *D_8009BF6C;
extern s32 D_8009BF70;
extern s32 D_8009BF80;
extern s32 D_8009BF84;

/* ADDQUE2-BEGIN */
/* LIBGPU/SYS `_addque2` — reference sotn-decomp src/main/psxsdk/libgpu/sys.c:744
 * (older library revision: per-store re-index of the volatile queue head,
 * 0x60-byte slots = func / arg / cb_arg / 21 data words). */
/* GpuQueueItem and `extern volatile GpuQueueItem _que[64];` are declared at the top of this file. */

s32 _addque2(s32 (*func)(s32 *, s32), s32 *arg, s32 len, s32 cb_arg) {
    s32 i;

    set_alarm();
    while (((_qin + 1) & 0x3F) == _qout) {
        if (get_alarm() != 0) {
            return -1;
        }
        _exeque();
    }
    D_8009BF80 = SetIntrMask(0);
    g_gpu_ctx.unk08 = 1;
    if (g_gpu_ctx.queue_mode == 0 ||
        (_qin == _qout && !(*g_gpu_dma_chcr & 0x01000000) && g_gpu_ctx.drawsync_cb == 0)) {
        while (!(*g_gpu_stat_reg & 0x04000000)) {
        }
        func(arg, cb_arg);
        _qlog[0] = (s32)func;
        D_8009BF6C = arg;
        D_8009BF70 = cb_arg;
        SetIntrMask(D_8009BF80);
        return 0;
    }
    DMACallback(2, _exeque);
    if (len != 0) {
        for (i = 0; i < len / 4; i++) {
            _que[_qin].data[i] = arg[i];
        }
        _que[_qin].arg = _que[_qin].data;
    } else {
        _que[_qin].arg = arg;
    }
    _que[_qin].cb_arg = cb_arg;
    _que[_qin].func = func;
    _qin = (_qin + 1) & 0x3F;
    SetIntrMask(D_8009BF80);
    _exeque();
    return (_qin - _qout) & 0x3F;
}
/* ADDQUE2-END */
/* PsyQ 4.0 LIBGPU SYS: _exeque — verbatim-linked Sony object; C ref:
 * sotn-decomp src/main/psxsdk/libgpu/sys.c:797 is an older revision (null-func reset path,
 * CheckCallback tail) and was not adopted. Drains the packet queue; when it is empty and a
 * draw is pending, clears the pending flag and calls the DrawSyncCallback. */
s32 _exeque(void) {
    if (*g_gpu_dma_chcr & 0x01000000) {
        return 1;
    }
    D_8009BF84 = SetIntrMask(0);
    while (_qin != _qout && !(*g_gpu_dma_chcr & 0x01000000)) {
        if (((_qout + 1) & 0x3F) == _qin && g_gpu_ctx.drawsync_cb == 0) {
            DMACallback(2, NULL);
        }
        while (!(*g_gpu_stat_reg & 0x04000000)) {
        }
        _que[_qout].func(_que[_qout].arg, _que[_qout].cb_arg);
        _qlog[0] = (s32)_que[_qout].func;
        D_8009BF6C = _que[_qout].arg;
        /* FAKE: do-while(0) — its loop notes keep this log store between the arg log store
         * and the _qout advance; without it sched sinks both log stores to the loop test. */
        do {
            D_8009BF70 = _que[_qout].cb_arg;
        } while (0);
        _qout = (_qout + 1) & 0x3F;
    }
    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*g_gpu_dma_chcr & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0) {
        g_gpu_ctx.unk08 = 0;
        ((void (*)(void))g_gpu_ctx.drawsync_cb)();
    }
    return (_qin - _qout) & 0x3F;
}
extern void memset(u8 *a0, u8 a1, s32 a2);
extern s32 SetIntrMask(s32);
extern s32 _version(s32);
extern volatile s32 _qout;
extern volatile s32 _qin;
extern s32 D_8009BF88;
extern u8 ctlbuf[];
extern volatile u32 *g_gpu_dma_madr;
extern volatile int *D_8009BF64;
extern volatile s32 _qlog[];

extern s32 D_8009BF70;
extern s32 printf();
s32 _reset(s32 arg0) {
    D_8009BF88 = SetIntrMask(0);
    _qout = 0;
    _qin = _qout;
    switch (arg0 & 7) {
    case 5:
    case 0:
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0;
        memset(ctlbuf, 0, 0x100);
        memset((u8 *)_que, 0, 0x1800);
        break;
    case 1:
    case 3:
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        break;
    }
    SetIntrMask(D_8009BF88);
    if (arg0 & 7) {
        return 0;
    }
    return _version(arg0);
}
s32 _sync(s32 arg0) {
    s32 temp_s0;
    s32 ret;

    if (arg0 == 0) {
        set_alarm();
        while (_qin != _qout) {
            _exeque();
            if (get_alarm() != 0) return -1;
        }
        while ((*g_gpu_dma_chcr & 0x01000000) || !(*g_gpu_stat_reg & 0x04000000)) {
            if (get_alarm() != 0) return -1;
        }
        return 0;
    }
    temp_s0 = (_qin - _qout) & 0x3F;
    if (temp_s0 != 0) {
        _exeque();
    }
    if (!(*g_gpu_dma_chcr & 0x01000000) && (*g_gpu_stat_reg & 0x04000000)) {
        ret = temp_s0;
    } else {
        if (temp_s0 != 0) {
            ret = temp_s0;
        } else {
            return 1;
        }
    }
    return ret;
}
void set_alarm(void) {
    g_gpu_vcount = VSync(-1) + 0xF0;
    g_gpu_draw_count = 0;
}
s32 get_alarm(void) {
    s32 temp_v0;
    if (g_gpu_vcount < VSync(-1) || g_gpu_draw_count++ > 0xF0000) {
        *g_gpu_stat_reg;
        printf(g_str_gpu_timeout, (_qin - _qout) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        printf(D_80016044, _qlog[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        _qout = 0;
        D_8009BF88 = temp_v0;
        _qin = _qout;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}
s32 _version(s32 arg0) {
    *g_gpu_stat_reg = 0x10000007;
    if ((*g_gpu_data_reg & 0xFFFFFF) != 2) {
        *g_gpu_data_reg = (*g_gpu_stat_reg & 0x3FFF) | 0xE1001000;
        (void)*g_gpu_data_reg;
        if (!(*g_gpu_stat_reg & 0x1000)) {
            return 0;
        }
        if (!(arg0 & 8)) {
            return 1;
        }
        *g_gpu_stat_reg = 0x20000504;
        return 2;
    }
    if (!(arg0 & 8)) {
        return 3;
    }
    *g_gpu_stat_reg = 0x09000001;
    return 4;
}
void memset(u8 *a0, u8 a1, s32 a2) {
    s32 i;
    for (i = a2 - 1; i != -1; i--) {
        *a0++ = a1;
    }
}
