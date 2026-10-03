#include "common.h"
#include "include_asm.h"
#include "bios.h"
#include "gpu.h"
#include "psx.h"


/* Forward declarations */
extern s32 VSync(s32);
extern s32 memcpy(s32, void *, s32);
extern void DeliverEvent(s32, s32);

/* Externs for globals */
extern volatile u32 *g_gpu_stat_reg;
extern volatile u32 *g_gpu_data_reg;
extern volatile u32 *g_gpu_dma_madr;
extern u32 *g_gpu_dma_bcr;
extern volatile u32 *g_gpu_dma_chcr;
extern u8 ctlbuf[];
extern s32 g_gpu_vcount;
extern s32 g_gpu_draw_count;
extern const char g_str_drawotag[];
extern const char g_str_drawsync[];
extern u32 D_80015EE8;
extern u32 D_80015FDC;

extern const char g_str_setdispmask[];

extern const char D_80015F2C[];
extern const char D_80015F38[];
extern const char D_80015F4C[];

/* --- Functions 0x8007B244 - 0x8007FF7C (text2 segment) --- */

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
extern const char g_str_clearimage[];
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
extern const char g_str_loadimage[];

void LoadImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_loadimage, a0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->dws, a0, 8, a1);
}
extern const char g_str_storeimage[];

void StoreImage(RECT *a0, u32 *a1) {
    checkRECT(g_str_storeimage, a0);
    g_gpu_dev_table->addque2(g_gpu_dev_table->drs, a0, 8, a1);
}
extern const char D_80015F74[];
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

extern const char g_str_clearotag[];
extern u32 g_gpu_ot_end;

u32 *ClearOTag(u32 *a0, s32 a1) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_clearotag, a0, a1);
    }
    a1--;
    if (a1) {
        u32 mask = 0xFFFFFF;
        u32 himask = 0xFF000000;
        do {
            u32 *next;
            a1--;
            next = a0 + 1;
            ((u8 *)a0)[3] = 0;
            *a0 = (*a0 & himask) | ((u32)next & mask);
            a0 = next;
        } while (a1);
    }
    *a0 = (u32)&g_gpu_ot_end & 0xFFFFFF;
    return a0;
}
extern u32 D_80015F98;

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
    u32 *dev = (u32 *)g_gpu_dev_table;
    u32 size = a0[3];
    ((void (*)(s32))dev[15])(0);
    dev = (u32 *)g_gpu_dev_table;
    ((void (*)(u32 *, u32))dev[5])(a0 + 4, size);
}
void DrawOTag(u32 *a0) {
    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(g_str_drawotag, a0);
    }
    g_gpu_dev_table->addque2(g_gpu_dev_table->cwc, a0, 0, 0);
}
extern const char g_str_putdrawenv[];


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
    u32 *dev;

    if (g_gpu_ctx.debug_level >= 2) {
        GPU_printf(&D_80015FDC, arg0, env);
    }
    SetDrawEnv2(&env->dr_env, env);
    env->dr_env.tag = (env->dr_env.tag & 0xFF000000) | (arg0 & 0xFFFFFF);
    dev = (u32 *)g_gpu_dev_table;
    ((s32 (*)(u32, DR_ENV *, s32, s32))dev[2])(dev[6], &env->dr_env, 0x40, 0);
    g_gpu_ctx.draw_env = *env;
}
s32 GetDrawEnv(s32 a0) {
    memcpy(a0, &g_gpu_ctx.draw_env, 0x5C);
    return a0;
}
extern const char D_80015FF8[];
extern s32 GetVideoMode(void);
s32 get_dx(s16 *arg0);
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
            ? ((env->disp.y & 0xFFF) << 12) | (get_dx((s16 *)env) & 0xFFF) | 0x05000000
            : ((env->disp.y & 0x3FF) << 10) | (env->disp.x & 0x3FF) |
                  0x05000000);
    /* FAKE: volatile-qualified reads of the saved environment (8 casts, both rect
       compares). The shipped bytes load every one of these fields as lhu + sll 16 +
       sra 16 -- the un-folded extend GCC keeps only for a volatile halfword -- while
       the env-> side of the same compares is a plain lh; the non-volatile spelling
       folds to lh. Admitted on a cross-function citation (owner ruling Q100): the
       same use-site `*(volatile T *)&` read of a plain-RAM struct member, matched and
       self-marked in SOTN ("Why the volatile?"); SOTN's own PutDispEnv (3.3) compares
       words and has none.
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
           and its respellings add 4 insns.
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
    s32 (*func)(void) = ((s32 (**)(void))g_gpu_dev_table)[0xE];
    return (u32)func() >> 31;
}
void SetTexWindow(u8 *a0, s32 a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = get_tw(a1);
    *(u32 *)(a0 + 8) = 0;
}
void SetDrawArea(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = get_cs(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = get_ce((s32)(s16)((u16)a1[0] + (u16)a1[2] - 1), (s32)(s16)((u16)a1[1] + (u16)a1[3] - 1));
}
void SetDrawOffset(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = get_ofs(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = 0;
}
void SetPriority(u8 *a0, s32 a1, s32 a2) {
    u32 v0;
    a0[3] = 2;
    v0 = 0xE6000000;
    if (a1) {
        v0 = 0xE6000002;
    }
    if (a2) {
        v0 |= 1;
    }
    *(u32 *)(a0 + 4) = v0;
    *(u32 *)(a0 + 8) = 0;
}
void SetDrawMode(u8 *a0, s32 a1, s32 a2, u16 a3, s32 a4) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = get_mode(a1, a2, a3);
    *(u32 *)(a0 + 8) = get_tw(a4);
}
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
    s16 u;
    s16 v;
    u8 pad12[4];
    s16 ax;
    s16 ay;
    u16 cx;
    u16 cy;
    u8 flag;
    u8 r;
    u8 g;
    u8 b;
} Rect;
void SetDrawEnv(s32 *out, Rect *r)
{
  s32 *o = out; /* FAKE: prologue pair order — single forward-order param
                   alias (pointer-alias-fake-exception; owner ruling for the
                   twins SetDrawEnv / SetDrawEnv2). cc1 combine's single-use
                   entry-copy merge relocates arg0's `move s1,a0` past arg1's
                   `move s0,a1`, flipping the prologue save+def pair
                   emit order to match target (s0-pair first). An arg1
                   alias, K&R decl-block reversal or do-while(0) entry wrap
                   leaves the pair order unchanged. */
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  o[1] = get_cs(r->x, r->y);
  o[2] = get_ce((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));
  o[3] = get_ofs(r->u, r->v);
  o[4] = get_mode(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));
  o[5] = get_tw(((u8 *) r) + 12);
  o[6] = (s32) 0xE6000000;
  var_a3 = 7;
  if (r->flag != 0)
  {
    buf[0] = (u16) r->x;
    buf[1] = (u16) r->y;
    new_var = (s16) r->w;
    buf[2] = (u16) r->w;
    buf[3] = (u16) r->h;
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
    buf[0] -= (u16) r->u;
    buf[1] -= (u16) r->v;
    o[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
    o[var_a3++] = ((u32 *) buf)[0];
    o[var_a3++] = ((u32 *) buf)[1];
    buf[0] += (u16) r->u;
    buf[1] += (u16) r->v;
  }
  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);
}
void SetDrawEnv2(s32 *out, Rect *r)
{
  s32 *o = out; /* FAKE: prologue pair order — same param alias and
                   mechanism as SetDrawEnv above (owner ruling for the twins):
                   cc1 combine's single-use entry-copy merge relocates arg0's
                   `move s1,a0` past arg1's `move s0,a1`, flipping the
                   prologue save+def pair emit order to match target
                   (s0-pair first). */
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  o[1] = get_cs(r->x, r->y);
  o[2] = get_ce((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));
  o[3] = get_ofs(r->u, r->v);
  o[4] = get_mode(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));
  o[5] = get_tw(((u8 *) r) + 12);
  o[6] = (s32) 0xE6000000;
  var_a3 = 7;
  if (r->flag != 0)
  {
    buf[0] = (u16) r->x;
    buf[1] = (u16) r->y;
    new_var = (s16) r->w;
    buf[2] = (u16) r->w;
    buf[3] = (u16) r->h;
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
      buf[0] -= (u16) r->u;
      buf[1] -= (u16) r->v;
      o[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
      o[var_a3++] = ((u32 *) buf)[0];
      o[var_a3++] = ((u32 *) buf)[1];
      buf[0] += (u16) r->u;
      buf[1] += (u16) r->v;
    }
    else
    {
      o[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x02000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
      o[var_a3++] = ((u32 *) buf)[0];
      o[var_a3++] = ((u32 *) buf)[1];
    }
  }
  *(((s8 *) o) + 3) = (s8) (var_a3 - 1);
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
s32 get_tw(u8 *arg0) {
    if (arg0 != 0) {
        u32 tmp[4]; /* FAKE: written-never-read scratch (SOTN dra/62DEC.c sp70[4] family;
                       dead-vars-local-array carve-out) */
        u8 r, b1;
        s32 g, b2;
        u32 b15, re2, ret;
        r = arg0[0] >> 3;
        tmp[0] = r;
        g = ((-*(s16 *)(arg0 + 4)) & 0xFF) >> 3;
        tmp[2] = g;
        b1 = arg0[2] >> 3;
        tmp[1] = b1;
        b15 = (u32)b1 << 0xF;
        b2 = ((-*(s16 *)(arg0 + 6)) & 0xFF) >> 3;
        re2 = ((u32)r << 0xA) | 0xE2000000u;
        ret = b15 | re2 | ((u32)b2 << 5) | (u32)g;
        tmp[3] = b2;
        return ret;
    }
    return 0;
}
s32 get_dx(s16 *arg0) {
    s32 v1, a, t;
    switch (g_gpu_ctx.type) {
    case 1:
        if (g_gpu_ctx.reverse != 0) {
            t = 0x400;
            v1 = arg0[2];
            a = arg0[0];
        sub:
            t = t - v1;
            return t - a;
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != g_gpu_ctx.reverse) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            /* FAKE: wrap keeps the 0x400 load below the div chain so it
               fills the jump delay slot instead of hoisting to block top */
            do { t = 0x400; } while (0);
            goto sub;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
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
    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR;
    for (i = a1 - 1; i != -1; i--) {
        *(volatile u32 *)g_gpu_data_reg = *a0++;
    }
    return 0;
}
void _cwc(u32 a0) {
    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR_FIFO;
    *(volatile u32 *)g_gpu_dma_madr = a0;
    *(volatile u32 *)g_gpu_dma_bcr = 0;
    *(volatile u32 *)g_gpu_dma_chcr = DMA_GPU_LINKED_LIST;
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


s32 _exeque();                            /* extern */
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
/* GpuQueueItem and `extern volatile GpuQueueItem _que[64];` are declared in include/gpu.h. */

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
extern const char g_str_gpu_timeout[];
extern const char D_80016044[];
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
extern s32 _exeque();
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
    *(volatile s32 *)g_gpu_stat_reg = 0x10000007;
    if ((*(volatile s32 *)g_gpu_data_reg & 0xFFFFFF) != 2) {
        *(volatile s32 *)g_gpu_data_reg = (*(volatile s32 *)g_gpu_stat_reg & 0x3FFF) | 0xE1001000;
        (void)*(volatile s32 *)g_gpu_data_reg;
        if (!(*(volatile s32 *)g_gpu_stat_reg & 0x1000)) {
            return 0;
        }
        if (!(arg0 & 8)) {
            return 1;
        }
        *(volatile s32 *)g_gpu_stat_reg = 0x20000504;
        return 2;
    }
    if (!(arg0 & 8)) {
        return 3;
    }
    *(volatile s32 *)g_gpu_stat_reg = 0x09000001;
    return 4;
}
void memset(u8 *a0, u8 a1, s32 a2) {
    s32 i;
    for (i = a2 - 1; i != -1; i--) {
        *a0++ = a1;
    }
}
BIOS_A_FUNCTION(GPU_cw, 0x49);
extern s32 sin_1(s32);
s32 rsin(s32 a0) {
    s32 v;
    if (a0 < 0) {
        v = sin_1((-a0) & 0xFFF);
        return -v;
    }
    return sin_1(a0 & 0xFFF);
}
extern s16 rsin_tbl[];
extern s16 g_sin_lut_q3[];
extern s16 g_cos_lut_q2[];
extern s16 g_cos_lut_q4[];

s32 sin_1(s32 a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return rsin_tbl[a0];
        }
        return rsin_tbl[0x800 - a0];
    }
    if (a0 < 0xC01) {
        return -g_sin_lut_q3[a0];
    }
    return -rsin_tbl[0x1000 - a0];
}
s32 rcos(s32 a0) {
    if (a0 < 0) {
        a0 = -a0;
    }
    a0 = a0 & 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return rsin_tbl[0x400 - a0];
        }
        return -g_cos_lut_q2[a0];
    }
    if (a0 < 0xC01) {
        return -rsin_tbl[0xC00 - a0];
    }
    return g_cos_lut_q4[a0];
}

/* Data blob D_8007E08C between math_Cos and func_8007E094 */
__asm__(
    ".section .text\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .include \"asm/funcs/D_8007E08C.s\"\n"
    "    .set reorder\n"
    "    .set at\n"
);

INCLUDE_ASM("asm/funcs", InitGeom);
INCLUDE_ASM("asm/funcs", SquareRoot0);
/* func_8007E1AC = LIBGTE MSC06 LoadAverage12 â€” verbatim-linked Sony PsyQ 4.0
 * object. Hand-written GTE asm; disassembler tags every
 * cop2 op "handwritten instruction". No pure-C form (mtc2/lwc2/gpf/gpl/mfc2/
 * swc2 have no C analog). Canonical body (cluster corroboration: siblings
 * MulMatrix2 and ApplyRotMatrix are hand-written asm too). */
INCLUDE_ASM("asm/funcs", LoadAverage12);
/* func_8007E1FC = LIBGTE MSC06 LoadAverage0 â€” verbatim-linked Sony PsyQ 4.0
 * object. Twin of func_8007E1AC differing only in the
 * gpf/gpl sf parameter (0 vs 1). Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", LoadAverage0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", LoadAverageShort12);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", LoadAverageShort0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", LoadAverageByte);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", LoadAverageCol);
INCLUDE_ASM("asm/funcs", SquareRoot12);
/* func_8007E4DC = LIBGTE MTX_000 MulMatrix0 â€” verbatim-linked Sony PsyQ 4.0
 * object. 3x3-mvmva matrix transform sibling of
 * MulMatrix2 (calc_fc_frame_8007EC5C). All the same hand-coded
 * signals: splat-tagged every cop2 op "handwritten instruction", hardcoded
 * `swc2 $11, 16($a2)` source reg, hand-scheduled cycle-N+1-mfc2 during
 * cycle-N-mvmva latency, per-cycle `lui $at, 0xFFFF` re-materialization,
 * addu $v0,$a2 pass-through-at-end. Canonical body. */
INCLUDE_ASM("asm/funcs", MulMatrix0);
INCLUDE_ASM("asm/funcs", CompMatrix);
/* func_8007E74C = LIBGTE MTX_004 ApplyMatrixLV â€” verbatim-linked Sony PsyQ 4.0
 * object. Local-vector transform with pre-scaling via sign-
 * split (hi=x>>15, lo=x&0x7FFF), two mvmva cycles (hi 0,0,3,3,0 then lo
 * 1,0,3,3,0), post-scale hi result by 8 via signed <<3, sum + store. Hand-
 * coded evidence: the `sra $tN,$tM,15` split idiom and branch forms that
 * compiled C reaches only with register pins or asm rewriting. Canonical
 * body. */
INCLUDE_ASM("asm/funcs", ApplyMatrixLV);
/* func_8007E8AC â€” hand-written GTE mvmva vector-transform wrapper
 * (8007Exxx hand-asm cluster, sibling of MulMatrix2 / calc_fc_frame_8007EC5C;
 * owner-authorized per canonical-asm-authorization-recipe). Hand-coded evidence: the
 * lw encodings target $t0/$t1 (unreachable from compiled C without
 * forbidden pins), hand-placed GTE load-delay nop, return-pinned-at-end
 * addu $v0,$a2 pass-through, unfilled jr delay slot. cop2 ops splat-tagged
 * "handwritten instruction". */
INCLUDE_ASM("asm/funcs", ApplyRotMatrix);
/* func_8007E8DC = LIBGTE MTX_00A ScaleMatrixL â€” verbatim-linked Sony PsyQ 4.0
 * object. In-place Q12 fixed-point column scale of a 3x3
 * matrix by 3 scalars (columns 0,1,2 x scalars *arg1[0/1/2]). Splat tags the
 * body handwritten; hardcoded $t0..$t5 packed register cadence + hand-scheduled
 * multu/mflo pairing + sw in jr delay slot are hand-coded signatures. Sibling
 * of ScaleMatrix (func_8007EDBC, canonical body per packed-multiply-cluster).
 * Canonical body. */
INCLUDE_ASM("asm/funcs", ScaleMatrixL);
/* func_8007EA0C = LIBGTE MTX_01 ApplyRotMatrixLV - verbatim-linked Sony PsyQ
 * 4.0 object. Sibling of ApplyMatrixLV (func_8007E74C):
 * sign-splits input vec into hi/lo halves (arithmetic split), runs mvmva
 * twice (hi 0,0,3,3,0 then lo 1,0,3,3,0), post-scales hi by <<3 with signed
 * preservation, sums and stores. No pure-C form reaches these bytes without
 * register pins or asm rewriting. Canonical body. */
INCLUDE_ASM("asm/funcs", ApplyRotMatrixLV);
/* func_8007EB4C = LIBGTE MTX_03 MulMatrix â€” verbatim-linked Sony PsyQ 4.0
 * object. In-place variant of the same 3-cycle mvmva
 * transform as func_8007E4DC / calc_fc_frame_8007EC5C: reads matrix + vec
 * from $a0 (out doubles as matrix-input buffer), writes result back to $a0.
 * All the calc_fc_frame (MulMatrix2) hand-coded signals hold. Canonical body. */
INCLUDE_ASM("asm/funcs", MulMatrix);
/* calc_fc_frame_8007EC5C: hand-coded GTE 3x3-mvmva matrix transform.
 * COMPLETED-INLINE-ASM-CANONICAL -- see
 * inline_asm_canonical.txt for justification. Disassembler annotates
 * every cop2 op as a handwritten instruction; final swc2 $11 uses a
 * hardcoded source reg; mvmva/mfc2/mtc2/nop pipeline is hand-scheduled
 * (cycle N+1 setup interleaves with cycle N latency); per-cycle lui $at
 * re-materialization is a hand-coded choice. No pure-C form reaches
 * these bytes. */
INCLUDE_ASM("asm/funcs", MulMatrix2);
/* func_8007ED6C = LIBGTE MTX_05 ApplyMatrix â€” verbatim-linked Sony PsyQ 4.0
 * object. Loads a 3x3 R matrix (5 packed s32 words) into
 * cop2 controls 0-4, transforms *a1 vec by RT matrix (mvmva 1,0,0,3,0),
 * writes result to *a2. Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", ApplyMatrix);

/* func_8007EDBC: hand-coded asm in the original PSY-Q source (display.c packed
 * fixed-point multiply -- 3x3-matrix column scale: 9 packed s16 values each
 * x coef[k%3] from arg1, >>12 Q12, repacked in place).
 * Hand-coded, NOT compiled C:
 *  - Leading `andi $t1,$t0,0xFFFF` before `sll 16; sra 16` is a REDUNDANT mask
 *    (the sll discards exactly the masked bits). GCC combine (combine.c:1458
 *    added_sets_2) elides a single-use redundant mask and keeps the 2nd-use
 *    insn for a multi-use one, so no pure-C form emits this andi without a
 *    stray instruction the target lacks (verified ~38 C forms).
 *  - cc1psx (Sony GCC 2.7.2.SN.1) is byte-identical to our fork here (both
 *    elide it) -- so the original was not compiled from C.
 *  - Cluster: sibling func_8007E8DC (jaccard=0.58) is inline asm; the 8007Exxx
 *    /8007Fxxx display.c region is documented hand-coded asm.
 * Owner-authorized on that evidence (cc1psx proof + combine.c + cluster).
 */
INCLUDE_ASM("asm/funcs", ScaleMatrix);
INCLUDE_ASM("asm/funcs", SetRotMatrix);
INCLUDE_ASM("asm/funcs", SetColorMatrix);
INCLUDE_ASM("asm/funcs", SetTransMatrix);
INCLUDE_ASM("asm/funcs", ReadSZfifo3);
/* LIBGTE REG09: Sony's hand-written asm module, padded to 16 bytes (owner ruling Q104). */
INCLUDE_ASM("asm/funcs", ReadGeomScreen);
void SetBackColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile ("ctc2 %0, $13" :: "r"(a0));  /* ctc2 $a0, $13 */
    __asm__ volatile ("ctc2 %0, $14" :: "r"(a1));  /* ctc2 $a1, $14 */
    __asm__ volatile ("ctc2 %0, $15" :: "r"(a2));  /* ctc2 $a2, $15 */
}
void SetFarColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile ("ctc2 %0, $21" :: "r"(a0));  /* ctc2 $a0, $21 */
    __asm__ volatile ("ctc2 %0, $22" :: "r"(a1));  /* ctc2 $a1, $22 */
    __asm__ volatile ("ctc2 %0, $23" :: "r"(a2));  /* ctc2 $a2, $23 */
}
/* LIBGTE REG12 and REG13: hand-written asm modules, each padded to 16 bytes (owner ruling Q104). */
INCLUDE_ASM("asm/funcs", SetGeomOffset);
INCLUDE_ASM("asm/funcs", SetGeomScreen);
INCLUDE_ASM("asm/funcs", LightColor);
INCLUDE_ASM("asm/funcs", DpqColorLight);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", DpqColor3);
INCLUDE_ASM("asm/funcs", Intpl);
/* func_8007F0BC / func_8007F0E4 â€” hand-written GTE sqr leaf wrappers
 * (8007Fxxx cluster, same shape as ApplyRotMatrix / func_8007E8AC,
 * f980d67b): lwc2 x3 -> GTE delay nop -> sqr -> swc2 x3 -> jr with
 * hand-pinned `addu $v0,$a1,$zero` return in the delay slot. The return
 * pin is unreachable from compiled C (local-alloc copy-suggestion scan is
 * ascending, so the arg copy always wins the qty and the return copy
 * materializes at function head â€” measured across volatile/return-local
 * variants). swc2 ops splat-tagged "handwritten instruction". Owner-authorized. */
INCLUDE_ASM("asm/funcs", Square12);
INCLUDE_ASM("asm/funcs", Square0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", AverageZ3);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", AverageZ4);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", OuterProduct12);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", OuterProduct0);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", Lzc);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", RotTransPers);
/* func_8007F24C = LIBGTE SMP_03 RotTransPers3 â€” verbatim-linked Sony PsyQ 4.0
 * object. Triple perspective transform: lwc2 3 SXY0/SXY1/SXY2
 * pairs from *a0/*a1/*a2 -> rtpt -> swc2 SZ/SXY0/SXY1/SXY2 to *a3 & sp-loaded
 * pointers -> cfc2 FLAG to *(sp+0x1C) -> return mfc2 SZ3 >> 2 (folded into jr
 * delay slot). Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", RotTransPers3);
/* Original LIBGTE assembly; fixed-register ABI is explicit in the assembly body. */
INCLUDE_ASM("asm/funcs", RotTrans);
/* func_8007F2DC = LIBGTE CMB_00 RotTransPers4 â€” verbatim-linked Sony PsyQ 4.0
 * object. Triple perspective transform PLUS a 4th vertex
 * via rtps: rtpt on 3 SXY pairs, then rtps on the 4th (*a3). Combined FLAGs
 * OR'd; returns SZ3 >> 2. Hand-written GTE asm; canonical body. */
INCLUDE_ASM("asm/funcs", RotTransPers4);
/* motutil_GetWalkDir: hand-coded asm in original PSY-Q source.
 * Cluster sibling of func_8007F5EC (jaccard=0.68): same 3-axis Euler
 * rotation skeleton, different rotation-matrix coefficient signs
 * and different output-byte layout. 163 insns, zero spills, three
 * INT_MIN-guard idioms in succession. Scanner STRONG 3/5 (S2+S3+S5);
 * manual review confirmed hand-coded. Same cluster authorization. */
INCLUDE_ASM("asm/funcs", RotMatrix);
/* func_8007F5EC: hand-coded asm in original PSY-Q source.
 * 3-axis Euler rotation: reads X/Y/Z angles from arg0 (s16[3]),
 * looks up cos/sin for each, applies a 9-element 3D rotation chain
 * to arg1[0..0x10]. Manual signal review (scanner 3/5
 * but verified hand-coded): three INT_MIN-guard idioms, 163 insns
 * with zero spills despite 10+ live registers, hand-scheduled
 * multu/mflo where a 3-cycle gap holds a bgez+andi pair in the
 * pipeline stall window. Same cluster authorization scope as
 * func_8007F87C (RotMatrixX). */
INCLUDE_ASM("asm/funcs", RotMatrixZYX);
/* func_8007F87C: hand-coded asm in original PSY-Q source.
 * Evidence for the hand-coded classification:
 *   - Uniform 2-cycle multu/mflo pacing on EVERY mult/mflo pair
 *   - Front-loaded loads (6 args loaded interleaved with first 2 multus)
 *   - Tight register packing across 75-instruction kernel, no spills
 *   - INT_MIN-guard idiom: empty `if (a<0){}` body at .L8007F898
 *   - Cluster behavior: motutil_GetWalkDir, func_8007F5EC, func_8007FA1C,
 *     func_8007FBBC share the same skeletal shape.
 * Owner-authorized for this cluster. */
INCLUDE_ASM("asm/funcs", RotMatrixX);
/* func_8007FA1C: hand-coded asm in original PSY-Q source.
 * Sibling of func_8007F87C with mirrored sin negation and offsets
 * shifted to 0..0x10 (vs 6..0x10 for func_8007F87C). All 5 strong
 * signals confirmed (uniform multu pacing, front-loaded loads, tight
 * register packing, INT_MIN-guard idiom at .L8007FA38, cluster). Same
 * cluster authorization (commit 39e9bf0). */
INCLUDE_ASM("asm/funcs", RotMatrixY);
/* func_8007FBBC: hand-coded asm in original PSY-Q source.
 * Cluster sibling of func_8007F87C (jaccard=1.00 â€” structurally
 * identical, just different stride offsets 0..0xA). All 5 strong
 * signals confirmed by scan_hand_coded: uniform 2-cycle multu pacing,
 * empty-body INT_MIN-guard branch, 0 spills in 102 insns, 6-load burst
 * at insn 25, cluster sibling of two already-authorized functions.
 * Same cluster authorization. */
INCLUDE_ASM("asm/funcs", RotMatrixZ);
#define NULL ((void *)0)

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;
extern s16 ratan_tbl[];

/* PsyQ LIBGTE ratan: ratan2 â€” verbatim-linked Sony object (census
   2026-07-09); C ref: sotn-decomp psxsdk (table-lookup atan2) */
s32 ratan2(s32 arg0, s32 arg1) {
    s32 var_v1;
    s32 var_a0;
    s32 var_a1;
    s32 var_a2;
    s32 var_a3;
    s32 idx;

    var_a0 = arg0;
    var_a1 = arg1;
    var_a2 = 0;
    var_a3 = 0;
    if (var_a1 < 0) {
        var_a2 = 1;
        var_a1 = -var_a1;
    }
    if (var_a0 < 0) {
        var_a3 = 1;
        var_a0 = -var_a0;
    }
    if (var_a1 == 0 && var_a0 == 0) {
        return 0;
    }
    if (var_a0 < var_a1) {
        if (var_a0 & 0x7FE00000) {
            idx = var_a0 / (var_a1 >> 0xA);
        } else {
            idx = (var_a0 << 0xA) / var_a1;
        }
        var_v1 = ratan_tbl[idx];
    } else {
        if (var_a1 & 0x7FE00000) {
            idx = var_a1 / (var_a0 >> 0xA);
        } else {
            idx = (var_a1 << 0xA) / var_a0;
        }
        var_v1 = 0x400 - ratan_tbl[idx];
    }
    if (var_a2 != 0) {
        var_v1 = 0x800 - var_v1;
    }
    if (var_a3 != 0) {
        var_v1 = -var_v1;
    }
    return var_v1;
}
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel _patch_gte\n"
    "    lui $at, %hi(D_800A3658)\n"
    "    sw $ra, %lo(D_800A3658)($at)\n"
    "    jal EnterCriticalSection\n"
    "    nop\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jalr $t2\n"
    "    addiu $t1, $zero, 0x56\n"
    "    lui $t2, %hi(D_8007FF44)\n"
    "    lui $t1, %hi(CdInit)\n"
    "    lw $v0, 24($v0)\n"
    "    addiu $t2, $t2, %lo(D_8007FF44)\n"
    "    addiu $t1, $t1, %lo(CdInit)\n"
    ".L8007FF0C:\n"
    "    lw $v1, 0($t2)\n"
    "    addiu $t2, $t2, 0x4\n"
    "    addiu $v0, $v0, 0x4\n"
    "    bne $t2, $t1, .L8007FF0C\n"
    "    sw $v1, -4($v0)\n"
    "    jal FlushCache\n"
    "    nop\n"
    "    jal ExitCriticalSection\n"
    "    nop\n"
    "    lui $ra, %hi(D_800A3658)\n"
    "    lw $ra, %lo(D_800A3658)($ra)\n"
    "    nop\n"
    "    jr $ra\n"
    "    nop\n"
    ".globl D_8007FF44\n"
    "D_8007FF44:\n"
    "    nop\n"
    "    nop\n"
    "    addiu $k0, $zero, 0x100\n"
    "    lw $k0, 8($k0)\n"
    "    nop\n"
    "    lw $k0, 0($k0)\n"
    "    nop\n"
    "    addi $k0, $k0, 0x8\n"
    "    sw $at, 4($k0)\n"
    "    sw $v0, 8($k0)\n"
    "    sw $v1, 12($k0)\n"
    "    sw $ra, 124($k0)\n"
    "    .word 0x40026800\n"
    "    nop\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
extern s32 CdReset(s32);
extern s32 CdSyncCallback(s32);
extern s32 CdReadCallback(s32);
extern s32 CdReadMode(s32);
void def_cbsync(void);
void def_cbready(void);
void def_cbread(void);



extern const char g_str_cdinit_fail[];

s32 CdInit(void) {
    s32 retries = 4;
loop:
    if (CdReset(1) != 1) {
        retries--;
        if (retries != -1) goto loop;
        printf(g_str_cdinit_fail);
        return 0;
    }
    CdSyncCallback((s32)&def_cbsync);
    CdReadyCallback((s32)&def_cbready);
    CdReadCallback((s32)&def_cbread);
    CdReadMode(0);
    return 1;
}

void def_cbsync(void) {
    DeliverEvent(0xF0000003, 0x20);
}

void def_cbready(void) {
    DeliverEvent(0xF0000003, 0x40);
}

void def_cbread(void) {
    DeliverEvent(0xF0000003, 0x40);
}
