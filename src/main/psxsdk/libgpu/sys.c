/* PsyQ 4.0 LIBGPU SYS: the GPU system layer (ResetGraph .. memset; $Id:
 * sys.c,v 1.129). .text 0x8007AE7C..0x8007DF10, a verbatim LIBSCAN module span
 * (docs/naming/libscan/matches.json), Q106 D3. One file across the old cut at
 * 0x8007B244, which was mid-module (Q106 D3). */
#include "common.h"
#include <psxsdk/libgpu.h>
#include "psx.h"

/* PsyQ libgpu device table (_gpucb in psyz's sys.c; psyz's "gpu" is the
 * pointer): 0x40 bytes of function pointers at _gpucb, reached through the
 * pointer D_8009BE6C (0x8009BE6C). Member names/offsets are the PsyQ ones. */
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

extern GpuDevTable *D_8009BE6C;

/* libgpu SYS state block: one 0x80-byte object at 0x8009BE74 (ResetGraph
 * clears 0x80 bytes from its base). Member names follow the API that owns each
 * field (GetGraphType, SetGraphQueue, SetGraphDebug, SetGraphReverse, ...).
 * unk08 is set by _addque2 and test-and-cleared by _exeque (also the DMA-2 IRQ
 * callback) before it calls drawsync_cb. */
typedef struct {
    u8 type;        /* +0x00 */
    u8 queue_mode;  /* +0x01 */
    u8 debug_level; /* +0x02 */
    u8 reverse;     /* +0x03 */
    s16 width;      /* +0x04 */
    s16 height;     /* +0x06 */
    /* +0x08 volatile: test-and-cleared by _exeque, also the DMA-2 IRQ callback
     * (Q95) */
    volatile s32 unk08;
    u32 drawsync_cb;  /* +0x0C */
    DRAWENV draw_env; /* +0x10 */
    DISPENV disp_env; /* +0x6C */
} GpuCtx;             /* 0x80 */

extern GpuCtx g_gpu_ctx;

/* PsyQ libgpu packet queue (sys.c `static volatile struct QueueItem`): 64
 * records of 0x60 bytes {callback, argument pointer, the callback's second
 * argument (a colour, a pixel pointer or 0), 21 data words}. volatile is
 * Sony's own qualifier (the queue is drained by _exeque from DMA-IRQ context;
 * Ruling-4 (legitimate-volatile-interrupt-touched) grant).
 */
typedef struct GpuQueueItem {
    /* 0x00 */ s32 (*func)(s32 *, s32);
    /* 0x04 */ s32 *arg;
    /* 0x08 */ s32 cb_arg; /* the callback's second argument */
    /* 0x0C */ s32 data[21];
} GpuQueueItem; /* size 0x60 */

extern volatile GpuQueueItem _que[64];

/* PsyQ libgpu sys.c DR_ENV packet buffer (the `_clr` split-clear / fill
 * packet): one tag word + up to 15 command words at 0x800F1858. The next
 * object (ctlbuf, 0x800F189C) starts at +0x44. */
typedef struct GpuDrEnv {
    /* 0x00 */ u32 tag;
    /* 0x04 */ u32 code[15];
} GpuDrEnv; /* size 0x40 */

extern GpuDrEnv D_800F1858;
#include <psxsdk/libetc.h>

/* .rodata 0x80015E28..0x8001605C: this module's strings; every C reader is in
 * this file (Q106 D4). */

/* the rcsid; only the device table _gpucb points here */
const char D_80015E28[52] =
    "$Id: sys.c,v 1.129 1996/12/25 03:36:20 noda Exp $\0\0\0";

const char D_80015E5C[32] = "ResetGraph:jtb=%08x,env=%08x\n\0\0\0";

const char D_80015E7C[20] = "ResetGraph(%d)...\n\0\0";

const char D_80015E90[24] = "SetGraphReverse(%d)...\n\0";

const char D_80015EA8[44] = "SetGraphDebug:level:%d,type:%d r"
                            "everse:%d\n\0\0";

const char D_80015ED4[20] = "SetGrapQue(%d)...\n\0\0";

const char D_80015EE8[28] = "DrawSyncCallback(%08x)...\n\0\0";

const char g_str_setdispmask[20] = "SetDispMask(%d)...\n\0";

const char g_str_drawsync[20] = "DrawSync(%d)...\n\0\0\0\0";

const char D_80015F2C[12] = "%s:bad RECT\0";

const char D_80015F38[20] = "(%d,%d)-(%d,%d)\n\0\0\0\0";

const char D_80015F4C[4] = "%s:\0";

const char g_str_clearimage[12] = "ClearImage\0\0";

const char g_str_loadimage[12] = "LoadImage\0\0\0";

const char g_str_storeimage[12] = "StoreImage\0\0";

const char D_80015F74[12] = "MoveImage\0\0\0";

const char g_str_clearotag[24] = "ClearOTag(%08x,%d)...\n\0\0";

const char D_80015F98[24] = "ClearOTagR(%08x,%d)...\n\0";

const char g_str_drawotag[20] = "DrawOTag(%08x)...\n\0\0";

const char g_str_putdrawenv[24] = "PutDrawEnv(%08x)...\n\0\0\0\0";

const char D_80015FDC[28] = "DrawOTagEnv(%08x,&08x)...\n\0\0";

const char D_80015FF8[24] = "PutDispEnv(%08x)...\n\0\0\0\0";

const char g_str_gpu_timeout[52] =
    "GPU timeout:que=%d,stat=%08x,chc"
    "r=%08x,madr=%08x,\0\0\0";

const char D_80016044[24] = "func=(%08x)(%08x,%08x)\n\0";

extern s32 memcpy(s32, void *, s32);
s32 get_mode(s32, s32, s32);
s32 get_cs(s16, s16);
s32 get_ce(s16, s16);
s32 get_ofs(s32, s32);

extern volatile u32 *GPU_STATUS;
extern volatile u32 *GPU_DATA;
extern volatile u32 *DMA2_MADR;
extern volatile u32 *DMA2_BCR;
extern volatile u32 *DMA2_CHCR;
extern u8 ctlbuf[];
extern s32 D_8009BF8C;
extern s32 D_8009BF90;

extern s32 _gpucb;
extern s32 D_8009BEF4[];
extern s32 D_8009BF08[];

u32 ResetGraph(s32 a0) {
    switch (a0 & 7) {
    case 0:
    case 3:
        printf(&D_80015E5C, &_gpucb, &g_gpu_ctx);
        /* fallthrough */
    case 5:
        memset(&g_gpu_ctx, 0, sizeof(g_gpu_ctx));
        ResetCallback();
        GPU_cw((u32)D_8009BE6C & 0xFFFFFF);
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
        D_8009BE6C->reset(1);
        break;
    }
}

u32 SetGraphReverse(s32 a0) {
    u32 old = g_gpu_ctx.reverse;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015E90, a0);
    }
    g_gpu_ctx.reverse = a0;
    D_8009BE6C->ctl(
        0x08000000 | (g_gpu_ctx.reverse ? 0x80 : 0) | D_8009BE6C->getctl(8));
    if (g_gpu_ctx.type == 2) {
        D_8009BE6C->ctl(0x20000000 | (g_gpu_ctx.reverse ? 0x501 : 0x504));
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
        D_8009BE6C->reset(1);
        g_gpu_ctx.queue_mode = a0;
        DMACallback(2, 0);
    }
    return old;
}

u32 GetGraphType(void) { return g_gpu_ctx.type; }

u32 GetGraphDebug(void) { return g_gpu_ctx.debug_level; }

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
    D_8009BE6C->ctl(a0 ? 0x03000000 : 0x03000001);
}

void DrawSync(s32 a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_drawsync, a0);
    }
    D_8009BE6C->sync(a0);
}

void checkRECT(const char *str, RECT *rect) {
    s16 w, x, y, h;
    if (g_gpu_ctx.debug_level == 1)
        goto level_1;
    if (g_gpu_ctx.debug_level == 2)
        goto level_2;
    goto end;
level_1:
    w = rect->w;
    if (w > g_gpu_ctx.width)
        goto bad;
    x = rect->x;
    if (w + x > g_gpu_ctx.width)
        goto bad;
    y = rect->y;
    if (y > g_gpu_ctx.height)
        goto bad;
    h = rect->h;
    if (y + h > g_gpu_ctx.height)
        goto bad;
    if (w <= 0)
        goto bad;
    if (x < 0)
        goto bad;
    if (y < 0)
        goto bad;
    if (h > 0)
        goto end;
bad:
    GPU_printf(D_80015F2C, str);
    GPU_printf(D_80015F38, rect->x, rect->y, rect->w, rect->h);
    goto end;
level_2:
    GPU_printf(D_80015F4C, str);
    GPU_printf(D_80015F38, rect->x, rect->y, rect->w, rect->h);
end:;
}

extern void checkRECT(const char *, RECT *);

s32 ClearImage(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {
    checkRECT(g_str_clearimage, arg0);
    return D_8009BE6C->addque2(
        D_8009BE6C->clr, arg0, 8,
        ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}

void ClearImage2(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {
    checkRECT(g_str_clearimage, arg0);
    D_8009BE6C->addque2(
        D_8009BE6C->clr, arg0, 8,
        0x80000000 | ((u32)arg3 << 16) | ((u32)arg2 << 8) | (u32)arg1);
}

s32 LoadImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_loadimage, a0);
    return D_8009BE6C->addque2(D_8009BE6C->dws, a0, 8, a1);
}

s32 StoreImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_storeimage, a0);
    return D_8009BE6C->addque2(D_8009BE6C->drs, a0, 8, a1);
}

extern u32 move_image[5];

s32 MoveImage(RECT *rect, int x, int y) {
    s32 packed;

    checkRECT(D_80015F74, rect);
    if (rect->w == 0 || rect->h == 0) {
        return -1;
    }
    packed = (u16)y << 16 | (u16)x;
    move_image[2] = *(u32 *)&rect->x;
    move_image[3] = packed;
    move_image[4] = *(u32 *)&rect->w;
    return D_8009BE6C->addque2(
        D_8009BE6C->cwc, move_image, sizeof(move_image), 0);
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
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015F98, ot, n);
    }
    D_8009BE6C->otc(ot, n);
    *ot = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return ot;
}

void DrawPrim(u8 *a0) {
    u32 size = a0[3];
    D_8009BE6C->sync(0);
    D_8009BE6C->cwb(a0 + 4, size);
}

void DrawOTag(u32 *a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_drawotag, a0);
    }
    D_8009BE6C->addque2(D_8009BE6C->cwc, a0, 0, 0);
}

DRAWENV *PutDrawEnv(DRAWENV *env) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_putdrawenv, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    env->dr_env.tag |= 0xFFFFFF;
    D_8009BE6C->addque2(D_8009BE6C->cwc, &env->dr_env, 0x40, 0);
    g_gpu_ctx.draw_env = *env;
    return env;
}

void DrawOTagEnv(s32 arg0, DRAWENV *env) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    env->dr_env.tag = (env->dr_env.tag & 0xFF000000) | (arg0 & 0xFFFFFF);
    D_8009BE6C->addque2(D_8009BE6C->cwc, &env->dr_env, 0x40, 0);
    g_gpu_ctx.draw_env = *env;
}

s32 GetDrawEnv(s32 a0) {
    memcpy(a0, &g_gpu_ctx.draw_env, 0x5C);
    return a0;
}

s32 get_dx(DISPENV *env);

DISPENV *PutDispEnv(DISPENV *env) {
    s32 h_start, h_end;
    s32 v_start, v_end;
    s32 mode;

    mode = 0x08000000;
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(D_80015FF8, env);
    }
    D_8009BE6C->ctl(
        g_gpu_ctx.type == 1 || g_gpu_ctx.type == 2
            ? ((env->disp.y & 0xFFF) << 12) | (get_dx(env) & 0xFFF) | 0x05000000
            : ((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF) |
                  0x05000000);
    /* FAKE: volatile reads of the saved environment (8 casts) keep the target's
       lhu + sll + sra extend; plain reads fold to lh: score 71 (Q100).
       SOTN: src/main/psxsdk/libspu/s_m_m.c:48 @db41b28
     */
    if (!(*(volatile s16 *)&g_gpu_ctx.disp_env.screen.x == env->screen.x &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.y == env->screen.y &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.w == env->screen.w &&
          *(volatile s16 *)&g_gpu_ctx.disp_env.screen.h == env->screen.h)) {
        env->pad0 = GetVideoMode();
        h_start = env->screen.x * 10 + 0x260;
        v_start = env->screen.y + (env->pad0 ? 0x13 : 0x10);
        h_end = h_start + (env->screen.w ? env->screen.w * 10 : 2560);
        v_end = v_start + (env->screen.h ? env->screen.h : 240);
        /* each value clamped in place. SOTN: src/main/psxsdk/libgpu/sys.c:358
         * @aa53500 */
        h_start = h_start < 500 ? 500 : (h_start > 3290 ? 3290 : h_start);
        h_end = h_end < h_start + 0x50 ? h_start + 0x50
                                       : (h_end > 3290 ? 3290 : h_end);
        v_start =
            v_start < 0x10
                ? 0x10
                : (v_start > (env->pad0 ? 310 : 256) ? (env->pad0 ? 310 : 256)
                                                     : v_start);
        v_end = v_end < v_start + 2
                    ? v_start + 2
                    : (v_end > (env->pad0 ? 312 : 258) ? (env->pad0 ? 312 : 258)
                                                       : v_end);
        D_8009BE6C->ctl(
            ((h_end & 0xFFF) << 12) | 0x06000000 | (h_start & 0xFFF));
        D_8009BE6C->ctl(
            ((v_end & 0x3FF) << 10) | 0x07000000 | (v_start & 0x3FF));
    }
    /* isinter..pad1 compared as one word: the shipped bytes are a single lw at
       +0x10 on both sides. It is the same compare SOTN spells with its LOW()
       macro, i.e. the `*(s32 *)&` cast used here.
       SOTN: src/main/psxsdk/libgpu/sys.c:367 @aa53500 */
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
        /* FAKE: empty then-arm; the direct `if (env->disp.h > ...) mode |=
           0x24;` adds 4 insns (score 5). SOTN: src/main/psxsdk/libgpu/sys.c:394
           @aa53500 (same form). */
        if (env->disp.h <= (env->pad0 ? 288 : 256)) {
        } else {
            mode |= 0x24;
        }
        D_8009BE6C->ctl(mode);
    }
    memcpy((s32)&g_gpu_ctx.disp_env, env, sizeof(DISPENV));
    return env;
}

s32 GetDispEnv(s32 a0) {
    memcpy(a0, &g_gpu_ctx.disp_env, 0x14);
    return a0;
}

u32 GetODE(void) { return D_8009BE6C->status() >> 31; }

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

/* PsyQ: u_short *ofs; this build loads the two offsets signed (lh; u16 * gives
 * lhu). */
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

void SetDrawEnv(DR_ENV *out, DRAWENV *r) {
    /* FAKE: param alias flips the prologue's save/move pair order to the
       target's (s0 pair first); without it: score 4
       (pointer-alias-fake-exception; owner ruling for SetDrawEnv /
       SetDrawEnv2) */
    DR_ENV *o = out;
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
       (*(o->code + var_a3++ - 1)); the subscript o->code[var_a3++ - 1] adds the
       base first (addu aN,s1,aN for addu aN,aN,s1 at every push): score 3 in
       SetDrawEnv, 6 in SetDrawEnv2. */
    var_a3 = 7;
    if (r->isbg != 0) {
        buf[0] = r->clip.x;
        buf[1] = r->clip.y;
        new_var = r->clip.w;
        buf[2] = r->clip.w;
        buf[3] = r->clip.h;
        if (new_var >= 0) {
            if ((g_gpu_ctx.width - 1) < new_var) {
                var_v0 = g_gpu_ctx.width - 1;
            } else {
                var_v0 = new_var;
            }
        } else {
            var_v0 = 0;
        }
        buf[2] = (u16)var_v0;
        if (((s16)buf[3]) >= 0) {
            if ((g_gpu_ctx.height - 1) < ((s16)buf[3])) {
                var_v0_2 = g_gpu_ctx.height - 1;
            } else {
                var_v0_2 = (s16)buf[3];
            }
        } else {
            var_v0_2 = 0;
        }
        buf[3] = (u16)var_v0_2;
        buf[0] -= r->ofs[0];
        buf[1] -= r->ofs[1];
        *(o->code + var_a3++ - 1) =
            (((r->b0 << 16) | 0x60000000) | (r->g0 << 8)) | r->r0;
        *(o->code + var_a3++ - 1) = ((u32 *)buf)[0];
        *(o->code + var_a3++ - 1) = ((u32 *)buf)[1];
        buf[0] += r->ofs[0];
        buf[1] += r->ofs[1];
    }
    setlen(o, var_a3 - 1);
}

void SetDrawEnv2(DR_ENV *out, DRAWENV *r) {
    /* FAKE: param alias flips the prologue's save/move pair order, as in
       SetDrawEnv; without it: score 4
       (pointer-alias-fake-exception) */
    DR_ENV *o = out;
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
       (*(o->code + var_a3++ - 1)); the subscript o->code[var_a3++ - 1] adds the
       base first (addu aN,s1,aN for addu aN,aN,s1 at every push): score 3 in
       SetDrawEnv, 6 in SetDrawEnv2. */
    var_a3 = 7;
    if (r->isbg != 0) {
        buf[0] = r->clip.x;
        buf[1] = r->clip.y;
        new_var = r->clip.w;
        buf[2] = r->clip.w;
        buf[3] = r->clip.h;
        if (new_var >= 0) {
            if ((g_gpu_ctx.width - 1) < new_var) {
                var_v0 = g_gpu_ctx.width - 1;
            } else {
                var_v0 = new_var;
            }
        } else {
            var_v0 = 0;
        }
        buf[2] = (u16)var_v0;
        if (((s16)buf[3]) >= 0) {
            if ((g_gpu_ctx.height - 1) < ((s16)buf[3])) {
                var_v0_2 = g_gpu_ctx.height - 1;
            } else {
                var_v0_2 = (s16)buf[3];
            }
        } else {
            var_v0_2 = 0;
        }
        buf[3] = (u16)var_v0_2;
        if ((buf[0] & 0x3F) || (buf[2] & 0x3F)) {
            buf[0] -= r->ofs[0];
            buf[1] -= r->ofs[1];
            *(o->code + var_a3++ - 1) =
                (((r->b0 << 16) | 0x60000000) | (r->g0 << 8)) | r->r0;
            *(o->code + var_a3++ - 1) = ((u32 *)buf)[0];
            *(o->code + var_a3++ - 1) = ((u32 *)buf)[1];
            buf[0] += r->ofs[0];
            buf[1] += r->ofs[1];
        } else {
            *(o->code + var_a3++ - 1) =
                (((r->b0 << 16) | 0x02000000) | (r->g0 << 8)) | r->r0;
            *(o->code + var_a3++ - 1) = ((u32 *)buf)[0];
            *(o->code + var_a3++ - 1) = ((u32 *)buf)[1];
        }
    }
    setlen(o, var_a3 - 1);
}

s32 get_mode(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_v1;
    s32 var_v0;

    if ((u32)(g_gpu_ctx.type - 1) < 2U) {
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

/* Clamps both axes to g_gpu_ctx.width/height; SOTN's (different build) get_cs
 * clamps against constants. */
s32 get_cs(s16 x, s16 y) {
    x = x < 0 ? 0 : (x > g_gpu_ctx.width - 1 ? g_gpu_ctx.width - 1 : x);
    y = y < 0 ? 0 : (y > g_gpu_ctx.height - 1 ? g_gpu_ctx.height - 1 : y);
    if ((u32)(g_gpu_ctx.type - 1) < 2U) {
        return 0xE3000000 | ((y & 0xFFF) << 12) | (x & 0xFFF);
    } else {
        return 0xE3000000 | ((y & 0x3FF) << 10) | (x & 0x3FF);
    }
}

/* The get_cs twin, packet 0xE4000000. */
s32 get_ce(s16 x, s16 y) {
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
    if ((u32)(g_gpu_ctx.type - 1) >= 2U) {
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
        /* FAKE: written-never-read scratch (SOTN dra/62DEC.c sp70[4] family;
           dead-vars-local-array carve-out); without it: score 23 (25/33
           insns) */
        u32 tmp[4];
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
            do {
                t = 0x400;
            } while (0);
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

u32 _status(void) { return *GPU_STATUS; }

extern void set_alarm(void);
extern s32 get_alarm();
extern volatile s32 *DMA6_MADR;
extern volatile s32 *DMA6_BCR;
extern volatile s32 *DMA6_CHCR;
extern volatile s32 *DPCR;

s32 _otc(s32 arg0, s32 arg1) {
    *DPCR |= 0x08000000;
    *DMA6_CHCR = 0;
    *DMA6_MADR = (arg0 - 4) + (arg1 * 4);
    *DMA6_BCR = arg1;
    *DMA6_CHCR = 0x11000002;
    set_alarm();
    if (*DMA6_CHCR & 0x01000000) {
        do {
            if (get_alarm() != 0) {
                return -1;
            }
        } while (*DMA6_CHCR & 0x01000000);
    }
    return arg1;
}

void _cwc(u32 a0);
u32 _param(u32 a0);

/* _clr - the GPU-side body of ClearImage(): clamp the rect to the VRAM page,
 * build either a 12-word unaligned (mono rectangle) or 5-word aligned (VRAM
 * fill) packet in the DR_ENV buffer, and DMA it. */
s32 _clr(RECT *rect, u32 color) {
    u32 ptr;

    rect->w =
        rect->w < 0
            ? 0
            : (rect->w > g_gpu_ctx.width - 1 ? g_gpu_ctx.width - 1 : rect->w);
    rect->h =
        rect->h < 0
            ? 0
            : (rect->h > g_gpu_ctx.height - 1 ? g_gpu_ctx.height - 1 : rect->h);
    if (rect->x & 0x3F || rect->w & 0x3F) {
        /* unaligned clear: split in two packets */
        ptr = (u32)&D_800F1858.code[8];
        D_800F1858.tag = (ptr & 0xFFFFFF) | 0x08000000;
        D_800F1858.code[0] = 0xE3000000;
        D_800F1858.code[1] = 0xE4FFFFFF;
        D_800F1858.code[2] = 0xE5000000;
        D_800F1858.code[3] = 0xE6000000;
        D_800F1858.code[4] =
            0xE1000000 | *GPU_STATUS & 0x7FF | (color >> 0x1F) << 10;
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
        D_800F1858.code[1] =
            0xE1000000 | *GPU_STATUS & 0x7FF | (color >> 0x1F) << 10;
        D_800F1858.code[2] = (color & 0xFFFFFF) | 0x02000000;
        D_800F1858.code[3] = *(s32 *)&rect->x;
        D_800F1858.code[4] = *(s32 *)&rect->w;
    }
    _cwc((u32)&D_800F1858);
    return 0;
}

/* _dws - "data write short", the GPU-side body of LoadImage(): clamp the
 * destination rect to the VRAM page, push the CPU->VRAM copy command plus the
 * odd (non-multiple-of-16) leading words through GP0, then hand the
 * 16-word-aligned bulk to DMA channel 2. */
s32 _dws(RECT *rect, s32 *data) {
    s32 to_write;
    s32 size;
    s32 var_s0;
    s32 var_s4;

    var_s4 = 0;
    set_alarm();
    rect->w = rect->w < 0
                  ? 0
                  : (rect->w > g_gpu_ctx.width ? g_gpu_ctx.width : rect->w);
    rect->h = rect->h < 0
                  ? 0
                  : (rect->h > g_gpu_ctx.height ? g_gpu_ctx.height : rect->h);
    to_write = (rect->w * rect->h + 1) / 2;
    if (to_write <= 0) {
        return -1;
    }
    var_s0 = to_write % 16;
    size = to_write / 16;

    while (!(*GPU_STATUS & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *GPU_STATUS = 0x04000000;
    *GPU_DATA = 0x01000000;
    *GPU_DATA = var_s4 ? 0xB0000000 : 0xA0000000;
    *GPU_DATA = *(s32 *)&rect->x;
    *GPU_DATA = *(s32 *)&rect->w;

    while (--var_s0 != -1) {
        *GPU_DATA = *data++;
    }

    if (size) {
        *GPU_STATUS = 0x04000002;
        *DMA2_MADR = (u32)data;
        *DMA2_BCR = size << 16 | 0x10;
        *DMA2_CHCR = 0x01000201;
    }
    return 0;
}

/* _drs - "data read short", the GPU-side body of StoreImage(): as _dws, but
 * VRAM->CPU after waiting for the GPU to be ready to send. */
s32 _drs(RECT *rect, s32 *data) {
    s32 to_read;
    s32 size;
    s32 var_s0;

    set_alarm();
    rect->w = rect->w < 0
                  ? 0
                  : (rect->w > g_gpu_ctx.width ? g_gpu_ctx.width : rect->w);
    rect->h = rect->h < 0
                  ? 0
                  : (rect->h > g_gpu_ctx.height ? g_gpu_ctx.height : rect->h);
    to_read = (rect->w * rect->h + 1) / 2;
    if (to_read <= 0) {
        return -1;
    }
    var_s0 = to_read % 16;
    size = to_read / 16;

    while (!(*GPU_STATUS & 0x04000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    *GPU_STATUS = 0x04000000;
    *GPU_DATA = 0x01000000;
    *GPU_DATA = 0xC0000000;
    *GPU_DATA = *(s32 *)&rect->x;
    *GPU_DATA = *(s32 *)&rect->w;

    while (!(*GPU_STATUS & 0x08000000)) {
        if (get_alarm() != 0) {
            return -1;
        }
    }

    while (--var_s0 != -1) {
        *data++ = *GPU_DATA;
    }

    if (size) {
        *GPU_STATUS = 0x04000003;
        *DMA2_MADR = (u32)data;
        *DMA2_BCR = size << 16 | 0x10;
        *DMA2_CHCR = 0x01000200;
    }
    return 0;
}

void _ctl(u32 a0) {
    *GPU_STATUS = a0;
    ctlbuf[a0 >> 24] = a0;
}

u32 _getctl(s32 a0) { return ctlbuf[a0]; }

s32 _cwb(u32 *a0, s32 a1) {
    s32 i;
    *GPU_STATUS = GP1_DMA_DIR;
    for (i = a1 - 1; i != -1; i--) {
        *GPU_DATA = *a0++;
    }
    return 0;
}

void _cwc(u32 a0) {
    *GPU_STATUS = GP1_DMA_DIR_FIFO;
    *DMA2_MADR = a0;
    *DMA2_BCR = 0;
    *DMA2_CHCR = DMA_GPU_LINKED_LIST;
}

u32 _param(u32 a0) {
    *GPU_STATUS = a0 | GP1_GPU_INFO;
    return *GPU_DATA & OT_ADDR_MASK;
}

void _addque(s32 a0, s32 a1, s32 a2) { _addque2(a0, a1, 0, a2); }

extern volatile s32 _qin;
extern volatile s32 _qout;

s32 _exeque(void);
s32 get_alarm();                 /* extern */
s32 DMACallback(s32, s32 (*)()); /* extern */
s32 SetIntrMask(s32);            /* extern */
extern volatile s32 _qlog[];
extern s32 *D_8009BF6C;
extern s32 D_8009BF70;
extern s32 D_8009BF80;
extern s32 D_8009BF84;

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
        (_qin == _qout && !(*DMA2_CHCR & 0x01000000) &&
         g_gpu_ctx.drawsync_cb == 0)) {
        while (!(*GPU_STATUS & 0x04000000)) {
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

/* Drains the packet queue; when it is empty and a draw is pending, clears the
 * pending flag and calls the DrawSyncCallback. */
s32 _exeque(void) {
    if (*DMA2_CHCR & 0x01000000) {
        return 1;
    }
    D_8009BF84 = SetIntrMask(0);
    while (_qin != _qout && !(*DMA2_CHCR & 0x01000000)) {
        if (((_qout + 1) & 0x3F) == _qin && g_gpu_ctx.drawsync_cb == 0) {
            DMACallback(2, NULL);
        }
        while (!(*GPU_STATUS & 0x04000000)) {
        }
        _que[_qout].func(_que[_qout].arg, _que[_qout].cb_arg);
        _qlog[0] = (s32)_que[_qout].func;
        D_8009BF6C = _que[_qout].arg;
        /* FAKE: do-while(0) keeps this log store between the arg log store
         * and the _qout advance; without it both sink to the loop test: score
         * 10. */
        do {
            D_8009BF70 = _que[_qout].cb_arg;
        } while (0);
        _qout = (_qout + 1) & 0x3F;
    }
    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*DMA2_CHCR & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
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
extern volatile u32 *DMA2_MADR;
extern volatile int *DPCR;
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
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0;
        memset(ctlbuf, 0, 0x100);
        memset((u8 *)_que, 0, 0x1800);
        break;
    case 1:
    case 3:
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0x02000000;
        *GPU_STATUS = 0x01000000;
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
            if (get_alarm() != 0)
                return -1;
        }
        while ((*DMA2_CHCR & 0x01000000) || !(*GPU_STATUS & 0x04000000)) {
            if (get_alarm() != 0)
                return -1;
        }
        return 0;
    }
    temp_s0 = (_qin - _qout) & 0x3F;
    if (temp_s0 != 0) {
        _exeque();
    }
    if (!(*DMA2_CHCR & 0x01000000) && (*GPU_STATUS & 0x04000000)) {
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
    D_8009BF8C = VSync(-1) + 0xF0;
    D_8009BF90 = 0;
}

s32 get_alarm(void) {
    s32 temp_v0;
    if (D_8009BF8C < VSync(-1) || D_8009BF90++ > 0xF0000) {
        *GPU_STATUS;
        printf(g_str_gpu_timeout, (_qin - _qout) & 0x3F, *GPU_STATUS,
               *DMA2_CHCR, *DMA2_MADR);
        printf(D_80016044, _qlog[0], D_8009BF6C, D_8009BF70);
        temp_v0 = SetIntrMask(0);
        _qout = 0;
        D_8009BF88 = temp_v0;
        _qin = _qout;
        *DMA2_CHCR = 0x401;
        *DPCR |= 0x800;
        *GPU_STATUS = 0x02000000;
        *GPU_STATUS = 0x01000000;
        SetIntrMask(D_8009BF88);
        return -1;
    }
    return 0;
}

s32 _version(s32 arg0) {
    *GPU_STATUS = 0x10000007;
    if ((*GPU_DATA & 0xFFFFFF) != 2) {
        *GPU_DATA = (*GPU_STATUS & 0x3FFF) | 0xE1001000;
        (void)*GPU_DATA;
        if (!(*GPU_STATUS & 0x1000)) {
            return 0;
        }
        if (!(arg0 & 8)) {
            return 1;
        }
        *GPU_STATUS = 0x20000504;
        return 2;
    }
    if (!(arg0 & 8)) {
        return 3;
    }
    *GPU_STATUS = 0x09000001;
    return 4;
}

void memset(u8 *a0, u8 a1, s32 a2) {
    s32 i;
    for (i = a2 - 1; i != -1; i--) {
        *a0++ = a1;
    }
}
