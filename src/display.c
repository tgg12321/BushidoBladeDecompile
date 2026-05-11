#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "psx.h"

/* Padding NOP macro - emits NOP instructions between functions to match original layout */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")


/* Forward declarations */
extern s32 sys_VSync(s32);
extern s32 bb2_memcpy(s32, void *, s32);
extern void func_8008008C(s32, s32);

/* Externs for globals */
extern volatile u32 *g_gpu_stat_reg;
extern u32 *g_gpu_data_reg;
extern u32 *g_gpu_dma_madr;
extern u32 *g_gpu_dma_bcr;
extern volatile u32 *g_gpu_dma_chcr;
extern u8 g_gpu_color_table[];
extern u8 g_gpu_draw_env;
extern u8 g_gpu_disp_env;
extern u8 D_8009BE74;
extern s16 D_8009BE78;
extern s16 D_8009BE7A;
extern s32 g_gpu_vcount;
extern s32 g_gpu_draw_count;
extern u32 g_str_drawotag;
extern u32 g_str_drawsync;
extern u32 D_80015EE8;
extern u32 D_80015FDC;
extern u32 g_gpu_draw_mode;

extern u32 g_str_setdispmask;
extern u32 g_str_clearimage;

/* --- Functions 0x8007B244 - 0x8007FF7C (text2 segment) --- */

u32 func_8007B244(s32 a0) {
    u32 old;
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015EE8, a0);
    }
    old = g_gpu_draw_mode;
    g_gpu_draw_mode = a0;
    return old;
}
void gpu_SetDispMask(s32 a0) {
    u8 *p = &g_gpu_debug_level;
    if (*p >= 2) {
        g_gpu_debug_func(&g_str_setdispmask, a0);
    }
    if (!a0) {
        bb2_memset(p + 0x6A, -1, 0x14);
    }
    {
        u32 cmd = GP1_DISP_ENABLE;
        u32 *v0 = g_gpu_dev_table;
        if (a0) {
            cmd = 0x03000000;
        }
        ((void (*)(u32))v0[4])(cmd);
    }
}
void gpu_DrawSync(s32 a0) {
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&g_str_drawsync, a0);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(s32))v0[15])(a0);
    }
}
void func_8007B3A8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007B3A8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 g_str_clearimage;
extern s32 g_gpu_dev_table;
extern void func_8007B3A8(u8 *, s32);

void func_8007B4D0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    register s32 a asm("$19") = arg0;
    register s32 b asm("$18") = arg1;
    register s32 c asm("$17") = arg2;
    register s32 d asm("$16") = arg3;
    s32 *p;
    void (*fn)();
    s32 packed;
    func_8007B3A8(&g_str_clearimage, a);
    packed = ((d & 0xFF) << 16) | ((c & 0xFF) << 8);
    p = (s32 *)g_gpu_dev_table;
    fn = (void (*)())p[2];
    fn(p[3], a, 8, packed | (b & 0xFF));
}
void func_8007B564(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    register s32 a asm("$19") = arg0;
    register s32 b asm("$18") = arg1;
    register s32 c asm("$16") = arg2;
    register s32 d asm("$17") = arg3;
    s32 *p;
    void (*fn)();
    s32 packed;
    func_8007B3A8(&g_str_clearimage, a);
    packed = ((d & 0xFF) << 16) | ((c & 0xFF) << 8) | 0x80000000;
    p = (s32 *)g_gpu_dev_table;
    fn = (void (*)())p[2];
    fn(p[3], a, 8, packed | (b & 0xFF));
}
extern u32 g_str_loadimage;

void gpu_LoadImage(s32 a0, s32 a1) {
    u32 *v0;
    func_8007B3A8(&g_str_loadimage, a0);
    v0 = g_gpu_dev_table;
    ((void (*)(u32, s32, s32, s32))v0[2])(v0[8], a0, 8, a1);
}
extern u32 g_str_storeimage;

void gpu_StoreImage(s32 a0, s32 a1) {
    u32 *v0;
    func_8007B3A8(&g_str_storeimage, a0);
    v0 = g_gpu_dev_table;
    ((void (*)(u32, s32, s32, s32))v0[2])(v0[7], a0, 8, a1);
}
extern u8 D_80015F74;
extern s32 D_8009BF24;
extern s32 D_8009BF28;
extern s32 D_8009BF2C;

s32 func_8007B6C8(s32 *arg0, s16 arg1, s16 arg2) {
    s32 *p;
    s32 (*fn)();
    s32 packed;
    s32 *bf24;

    func_8007B3A8(&D_80015F74, (s32)arg0);
    if (((s16 *)arg0)[2] == 0) {
        return -1;
    }
    if (((s16 *)arg0)[3] == 0) {
        return -1;
    }
    packed = ((s32)arg2 << 16) | ((u32)arg1 & 0xFFFF);
    bf24 = &D_8009BF24;
    *bf24 = arg0[0];
    D_8009BF28 = packed;
    D_8009BF2C = arg0[1];
    p = (s32 *)g_gpu_dev_table;
    fn = (s32 (*)())p[2];
    return fn(p[6], (s32)bf24 - 8, 0x14, 0);
}
extern u32 g_str_clearotag;
extern u32 g_gpu_ot_end;

u32 *gpu_ClearOTag(u32 *a0, s32 a1) {
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&g_str_clearotag, a0, a1);
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

u32 *func_8007B844(u32 *ot, s32 n) {
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&D_80015F98, ot, n);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32 *, s32))v0[11])(ot, n);
    }
    *ot = ((u32)&g_gpu_ot_end) & 0xFFFFFF;
    return ot;
}
void gpu_SendPacket(u8 *a0) {
    u32 *dev = g_gpu_dev_table;
    u32 size = a0[3];
    ((void (*)(s32))dev[15])(0);
    dev = g_gpu_dev_table;
    ((void (*)(u32 *, u32))dev[5])(a0 + 4, size);
}
void gpu_DrawOTag(s32 a0) {
    if (g_gpu_debug_level >= 2) {
        g_gpu_debug_func(&g_str_drawotag, a0);
    }
    {
        u32 *v0 = g_gpu_dev_table;
        ((void (*)(u32, s32, s32, s32))v0[2])(v0[6], a0, 0, 0);
    }
}
extern u32 g_str_putdrawenv;
extern s32 func_8007C4B8(s32 *, s32 *);

typedef struct { s32 a, b, c, d; } _drawenv_q;
typedef struct { s32 a, b, c; } _drawenv_t;

s32 *func_8007B9B0(s32 *arg0) {
    s32 *p;
    u32 *dev;
    _drawenv_q *src;
    _drawenv_q *dst;
    _drawenv_q *end;
    u8 *base = &g_gpu_debug_level;

    if (*base >= 2) {
        g_gpu_debug_func(&g_str_putdrawenv, arg0);
    }
    p = arg0 + 7;
    func_8007C4B8(p, arg0);
    arg0[7] |= 0xFFFFFF;
    dev = g_gpu_dev_table;
    ((s32 (*)(u32, s32 *, s32, s32))dev[2])(dev[6], p, 0x40, 0);

    dst = (_drawenv_q *)(base + 0xE);
    src = (_drawenv_q *)arg0;
    end = (_drawenv_q *)(arg0 + 0x14);
    do {
        *dst = *src;
        src++;
        dst++;
    } while (src != end);
    *(_drawenv_t *)dst = *(_drawenv_t *)src;
    return arg0;
}
void func_8007BAB4(s32 arg0, s32 *arg1) {
    s32 *p;
    u32 *dev;
    _drawenv_q *src;
    _drawenv_q *dst;
    _drawenv_q *end;
    u8 *base = &g_gpu_debug_level;

    if (*base >= 2) {
        g_gpu_debug_func(&D_80015FDC, arg0, arg1);
    }
    p = arg1 + 7;
    func_8007C4B8(p, arg1);
    arg1[7] = (arg1[7] & 0xFF000000) | (arg0 & 0xFFFFFF);
    dev = g_gpu_dev_table;
    ((s32 (*)(u32, s32 *, s32, s32))dev[2])(dev[6], p, 0x40, 0);

    dst = (_drawenv_q *)(base + 0xE);
    src = (_drawenv_q *)arg1;
    end = (_drawenv_q *)(arg1 + 0x14);
    do {
        *dst = *src;
        src++;
        dst++;
    } while (src != end);
    *(_drawenv_t *)dst = *(_drawenv_t *)src;
}
s32 gpu_GetDrawEnv(s32 a0) {
    bb2_memcpy(a0, &g_gpu_draw_env, 0x5C);
    return a0;
}
void func_8007BC08(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007BC08.s).
     * GPU display environment setup: sets up DISPENV registers (mode, hres,
     * vres, frame buffer addresses) based on arg0 fields. m2c output had
     * function-pointer parse errors. Pure-C decomp pending. */
    (void)arg0;
}
s32 gpu_GetDispEnv(s32 a0) {
    bb2_memcpy(a0, &g_gpu_disp_env, 0x14);
    return a0;
}
u32 gpu_IsDrawing(void) {
    s32 (*func)(void) = ((s32 (**)(void))g_gpu_dev_table)[0xE];
    return (u32)func() >> 31;
}
void initClearImage(u8 *a0, s32 a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C97C(a1);
    *(u32 *)(a0 + 8) = 0;
}
void initDrawArea(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C7A0(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = func_8007C86C((s32)(s16)((u16)a1[0] + (u16)a1[2] - 1), (s32)(s16)((u16)a1[1] + (u16)a1[3] - 1));
}
void initDrawOffset(u8 *a0, s16 *a1) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C938(a1[0], a1[1]);
    *(u32 *)(a0 + 8) = 0;
}
void initMaskBit(u8 *a0, s32 a1, s32 a2) {
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
void initTexPage(u8 *a0, s32 a1, s32 a2, u16 a3, s32 a4) {
    a0[3] = 2;
    *(u32 *)(a0 + 4) = func_8007C748(a1, a2, a3);
    *(u32 *)(a0 + 8) = func_8007C97C(a4);
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
void func_8007C2A0(s32 *out, Rect *r)
{
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  out[1] = func_8007C7A0(r->x, r->y);
  out[2] = func_8007C86C((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));
  out[3] = func_8007C938(r->u, r->v);
  out[4] = func_8007C748(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));
  out[5] = func_8007C97C(((u8 *) r) + 12);
  out[6] = (s32) 0xE6000000;
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
      if ((D_8009BE78 - 1) < new_var)
      {
        var_v0 = D_8009BE78 - 1;
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
      if ((D_8009BE7A - 1) < ((s16) buf[3]))
      {
        var_v0_2 = D_8009BE7A - 1;
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
    out[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
    out[var_a3++] = ((u32 *) buf)[0];
    out[var_a3++] = ((u32 *) buf)[1];
    buf[0] += (u16) r->u;
    buf[1] += (u16) r->v;
  }
  *(((s8 *) out) + 3) = (s8) (var_a3 - 1);
}
void func_8007C4B8(s32 *out, Rect *r)
{
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  out[1] = func_8007C7A0(r->x, r->y);
  out[2] = func_8007C86C((s16) ((((u16) r->w) + ((u16) r->x)) - 1), (s16) ((((u16) r->y) + ((u16) r->h)) - 1));
  out[3] = func_8007C938(r->u, r->v);
  out[4] = func_8007C748(*(((u8 *) r) + 23), *(((u8 *) r) + 22), *((u16 *) (((u8 *) r) + 20)));
  out[5] = func_8007C97C(((u8 *) r) + 12);
  out[6] = (s32) 0xE6000000;
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
      if ((D_8009BE78 - 1) < new_var)
      {
        var_v0 = D_8009BE78 - 1;
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
      if ((D_8009BE7A - 1) < ((s16) buf[3]))
      {
        var_v0_2 = D_8009BE7A - 1;
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
      out[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x60000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
      out[var_a3++] = ((u32 *) buf)[0];
      out[var_a3++] = ((u32 *) buf)[1];
      buf[0] += (u16) r->u;
      buf[1] += (u16) r->v;
    }
    else
    {
      out[var_a3++] = ((((*(((u8 *) r) + 27)) << 16) | 0x02000000) | ((*(((u8 *) r) + 26)) << 8)) | (*(((u8 *) r) + 25));
      out[var_a3++] = ((u32 *) buf)[0];
      out[var_a3++] = ((u32 *) buf)[1];
    }
  }
  *(((s8 *) out) + 3) = (s8) (var_a3 - 1);
}
s32 func_8007C748(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_v1;

    if ((u32) (D_8009BE74 - 1) < 2U) {
        var_v1 = 0xE1000000;
        if (arg1 != 0) {
            var_v1 = 0xE1000800;
        }
        arg2 = arg2 & 0x27FF;
        if (arg0 != 0) {
            arg2 |= 0x1000;
        }
    } else {
        var_v1 = 0xE1000000;
        if (arg1 != 0) {
            var_v1 = 0xE1000200;
        }
        arg2 = arg2 & 0x9FF;
        if (arg0 != 0) {
            arg2 |= 0x400;
        }
    }
    return var_v1 | arg2;
}
s32 func_8007C7A0(s16 arg0, s16 arg1)
{
    s16 var_a1;
    s16 var_v0_2;
    int new_var;
    s32 var_v0;
    int new_var2;
    s32 var_v1;

    new_var = arg0 >= 0;
    if (new_var) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
        var_a1 = arg1;
    }
    if (var_a1 >= 0) {
        if ((D_8009BE7A - 1) < var_a1) {
            var_a1 = D_8009BE7A - 1;
        }
    } else {
        var_a1 = 0;
    }
    var_a1 = var_a1 & 0xFFF;
    if (((u32)(D_8009BE74 - 1)) >= 2U) {
        new_var2 = var_v0_2;
        if (!D_8009BE7A) { }
        var_v1 = var_a1 & 0x3FF;
        var_v1 = var_v1 << 0xA;
        var_v0 = new_var2 & 0x3FF;
    } else {
        var_v1 = (var_a1 << 1) << 11;
        var_v0 = new_var2 & 0xFFF;
    }
    new_var2 = 0xE3000000;
    return var_v1 | (var_v0 | new_var2);
}
s32 func_8007C86C(s16 arg0, s16 arg1)
{
    s16 var_a1;
    s16 var_v0_2;
    int new_var;
    s32 var_v0;
    int new_var2;
    s32 var_v1;

    new_var = arg0 >= 0;
    if (new_var) {
        if ((D_8009BE78 - 1) < arg0) {
            var_v0_2 = D_8009BE78 - 1;
        } else {
            var_v0_2 = arg0;
        }
    } else {
        var_v0_2 = 0;
        var_a1 = arg1;
    }
    if (var_a1 >= 0) {
        if ((D_8009BE7A - 1) < var_a1) {
            var_a1 = D_8009BE7A - 1;
        }
    } else {
        var_a1 = 0;
    }
    var_a1 = var_a1 & 0xFFF;
    if (((u32)(D_8009BE74 - 1)) >= 2U) {
        new_var2 = var_v0_2;
        if (!D_8009BE7A) { }
        var_v1 = var_a1 & 0x3FF;
        var_v1 = var_v1 << 0xA;
        var_v0 = new_var2 & 0x3FF;
    } else {
        var_v1 = (var_a1 << 1) << 11;
        var_v0 = new_var2 & 0xFFF;
    }
    new_var2 = 0xE4000000;
    return var_v1 | (var_v0 | new_var2);
}
extern u8 g_gpu_type;
s32 func_8007C938(s32 arg0, s32 arg1) {
    register s32 var_v0 asm("$2");
    register s32 var_v1 asm("$3");
    var_v1 = arg1 & 0xFFF;
    if ((u32) (g_gpu_type - 1) >= 2U) {
        var_v1 = arg1 & 0x7FF;
        var_v1 = var_v1 << 0xB;
        var_v0 = arg0 & 0x7FF;
    } else {
        var_v1 = var_v1 << 0xC;
        var_v0 = arg0 & 0xFFF;
    }
    var_v0 = var_v0 | 0xE5000000;
    return var_v1 | var_v0;
}
s32 func_8007C97C(u8 *arg0) {
    s32 sp[4];
    register u8 *p asm("$4") = arg0;
    register s32 r asm("$5");
    register s32 b1 asm("$6");
    register s32 g asm("$2");
    register s32 b2 asm("$3");
    if (p == 0) {
        g = 0;
        return g;
    }
    r = p[0];
    r >>= 3;
    sp[0] = r;

    b1 = *(s16 *)(p + 4);
    b1 = -b1;
    b1 &= 0xFF;
    b1 >>= 3;
    sp[2] = b1;

    g = p[2];
    r <<= 10;
    g >>= 3;
    sp[1] = g;
    g <<= 15;

    b2 = *(s16 *)(p + 6);
    {
        register s32 e2 asm("$4") = 0xE2000000;
        r |= e2;
    }
    g |= r;

    b2 = -b2;
    b2 &= 0xFF;
    b2 >>= 3;
    {
        register s32 b2sh asm("$4") = b2 << 5;
        g |= b2sh;
    }
    g |= b1;
    sp[3] = b2;
    return g;
}
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    int new_var;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            s32 v1 = arg0[2];
            return (0x400 - v1) - arg0[0];
        }
        return arg0[0];
    case 2:
        if (0 != D_8009BE77) {
            s32 v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            new_var = 0x400;
            return (new_var - v1) - arg0[0];
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    default:
        return arg0[0];
    }
}
u32 func_8007CAB0(void) {
    return *g_gpu_stat_reg;
}
extern s32 motion_LoadPreCalcData_8007DC68();
extern s32 func_8007DC9C();
extern s32 *D_8009BF58;
extern s32 *D_8009BF5C;
extern s32 *D_8009BF60;
extern s32 *D_8009BF64;
s32 func_8007CAC8(s32 arg0, s32 arg1) {
    *D_8009BF64 |= 0x08000000;
    *D_8009BF60 = 0;
    *D_8009BF58 = (arg0 - 4) + (arg1 * 4);
    *D_8009BF5C = arg1;
    *D_8009BF60 = 0x11000002;
    asm volatile("");
    motion_LoadPreCalcData_8007DC68();
    if (*D_8009BF60 & 0x01000000) {
        do {
            if (func_8007DC9C() != 0) {
                return -1;
            }
        } while (*D_8009BF60 & 0x01000000);
    }
    return arg1;
}
void func_8007CBB0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007CBB0.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
typedef struct {
    s32 unk0;
    s16 x;
    s16 y;
} _GpuChunkHdr_CE0C;

s32 func_8007CE0C(_GpuChunkHdr_CE0C *arg0, s32 *arg1) {
    register s32 var_s5 asm("s5") = 0;
    s32 half_size;
    s32 big_size;
    s32 remainder;
    s16 x;
    s16 y;
    s32 v1_tmp;
    s32 a0_tmp;
    s32 v0_ext;

    motion_LoadPreCalcData_8007DC68();

    x = arg0->x;
    if (x < 0) goto x_neg;
    v1_tmp = x;
    if (D_8009BE78 < x) {
        v1_tmp = D_8009BE78;
    }
    goto x_done;
x_neg:
    v1_tmp = 0;
x_done:
    arg0->x = (s16)v1_tmp;

    y = arg0->y;
    if (y < 0) goto y_neg;
    a0_tmp = y;
    v0_ext = a0_tmp << 16;
    if (D_8009BE7A < y) {
        a0_tmp = D_8009BE7A;
        goto y_block_8;
    }
    goto y_done;
y_neg:
    a0_tmp = 0;
y_block_8:
    v0_ext = a0_tmp << 16;
y_done:
    arg0->y = (s16)a0_tmp;

    {
        s32 y_ext = v0_ext >> 16;
        s32 prod = (s32)arg0->x * y_ext + 1;
        half_size = prod / 2;
        big_size = prod >> 5;
    }

    if (half_size <= 0) {
        return -1;
    }

    remainder = (half_size - big_size * 16) - 1;

    if (!((*g_gpu_stat_reg) & 0x04000000)) {
        do {
            if (func_8007DC9C((u32 *)0xA0000000) != 0) {
                return -1;
            }
        } while (!((*g_gpu_stat_reg) & 0x04000000));
    }

    *g_gpu_stat_reg = 0x04000000;
    *g_gpu_data_reg = 0x01000000;
    *g_gpu_data_reg = (var_s5 != 0) ? 0xB0000000 : 0xA0000000;
    *g_gpu_data_reg = arg0->unk0;
    *g_gpu_data_reg = (s32)arg0->x;

    if (remainder != -1) {
        do {
            *g_gpu_data_reg = *arg1++;
            remainder--;
        } while (remainder != -1);
    }

    if (big_size != 0) {
        *g_gpu_stat_reg = 0x04000002;
        *g_gpu_dma_madr = (u32)arg1;
        *g_gpu_dma_bcr = (big_size << 16) | 0x10;
        *g_gpu_dma_chcr = 0x01000201;
    }

    return 0;
}

void func_8007D048(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007D048.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8007D2CC(u32 a0) {
    *g_gpu_stat_reg = a0;
    g_gpu_color_table[a0 >> 24] = a0;
}
u32 func_8007D2F4(s32 a0) {
    return g_gpu_color_table[a0];
}
s32 gpu_SendData(u32 *a0, s32 a1) {
    s32 i;
    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR;
    for (i = a1 - 1; i != -1; i--) {
        *(volatile u32 *)g_gpu_data_reg = *a0++;
    }
    return 0;
}
void gpu_StartDmaList(u32 a0) {
    *(volatile u32 *)g_gpu_stat_reg = GP1_DMA_DIR_FIFO;
    *(volatile u32 *)g_gpu_dma_madr = a0;
    *(volatile u32 *)g_gpu_dma_bcr = 0;
    *(volatile u32 *)g_gpu_dma_chcr = DMA_GPU_LINKED_LIST;
}
u32 gpu_GetInfo(u32 a0) {
    *g_gpu_stat_reg = a0 | GP1_GPU_INFO;
    return *g_gpu_data_reg & OT_ADDR_MASK;
}
void func_8007D3D4(s32 a0, s32 a1, s32 a2) {
    func_8007D3F8(a0, a1, 0, a2);
}
extern s32 *D_8009BF48;
extern s32 D_8009BF78;
extern s32 D_8009BF7C;
extern s32 motion_LoadPreCalcData_8007DC68();

s32 func_8007D6D8();                            /* extern */
s32 func_8007DC9C();                                /* extern */
s32 irq_AcknowledgeVblank(s32, s32 (*)()); /* extern */
s32 motion_make_table(s32);                         /* extern */
extern u8 D_8009BE75;
extern s32 D_8009BE7C;
extern s32 D_8009BE80;
extern s32 *D_8009BF54;
extern s32 (*D_8009BF68)(s32 *, s32);
extern s32 *D_8009BF6C;
extern s32 D_8009BF70;
extern s32 D_8009BF80;
extern s32 D_80103680;
extern s32 D_80103684;
extern s32 D_80103688;
extern s32 D_8010368C;

s32 func_8007D3F8(s32 (*arg0)(s32 *, s32), s32 *arg1, s32 arg2, s32 arg3) {
    s32 *var_a3;
    s32 temp_a0;
    s32 temp_a1;
    s32 var_a2;
    s32 var_v0;
    s32 var_v0_2;

    motion_LoadPreCalcData_8007DC68();
    goto check_top;
err_loop:
    if (func_8007DC9C() != 0) {
        return -1;
    }
    func_8007D6D8();
check_top:
    if (((D_8009BF78 + 1) & 0x3F) == D_8009BF7C) {
        goto err_loop;
    }
    {
        D_8009BF80 = motion_make_table(0);
        D_8009BE7C = 1;
        if ((D_8009BE75 == 0) || ((D_8009BF78 == D_8009BF7C) && !(*D_8009BF54 & 0x01000000) && (D_8009BE80 == 0))) {
            do {
            } while (!(*D_8009BF48 & 0x04000000));
            arg0(arg1, arg3);
            D_8009BF68 = arg0;
            D_8009BF6C = arg1;
            D_8009BF70 = arg3;
            motion_make_table(D_8009BF80);
            return 0;
        }
        irq_AcknowledgeVblank(2, func_8007D6D8);
        var_a2 = 0;
        if (arg2 != 0) {
            s32 v_shift;
            var_a3 = arg1;
            var_v0_2 = arg2;
loop_13:
            if (var_v0_2 < 0) {
                var_v0_2 += 3;
            }
            v_shift = var_v0_2 >> 2;
            temp_a0 = var_a2 * 4;
            if (var_a2 < v_shift) {
                temp_a1 = *var_a3;
                var_a3 += 1;
                var_a2 += 1;
                *(s32 *)(temp_a0 + ((D_8009BF78 * 0x60) + (s32)&D_8010368C)) = temp_a1;
                var_v0_2 = arg2;
                goto loop_13;
            }
            *(s32 **)((s32)&D_80103684 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = (s32 *)((*(volatile s32 *)&D_8009BF78 * 0x60) + (s32)&D_8010368C);
        } else {
            *(s32 **)((s32)&D_80103684 + (D_8009BF78 * 0x60)) = arg1;
        }
        *(s32 *)((s32)&D_80103688 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = arg3;
        *(s32 (**)(s32 *, s32))((s32)&D_80103680 + (*(volatile s32 *)&D_8009BF78 * 0x60)) = arg0;
        D_8009BF78 = (D_8009BF78 + 1) & 0x3F;
        motion_make_table(D_8009BF80);
        func_8007D6D8();
        var_v0 = (D_8009BF78 - D_8009BF7C) & 0x3F;
        return var_v0;
    }
}
void func_8007D6D8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007D6D8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern void bb2_memset(u8 *a0, u8 a1, s32 a2);
extern s32 motion_make_table(s32);
extern s32 func_8007DE08(s32);
extern volatile s32 *D_8009BF48;
extern s32 *D_8009BF54;
extern volatile s32 D_8009BF7C;
extern s32 D_8009BF78;
extern s32 D_8009BF88;
extern u8 D_800F189C[];
extern u8 D_80103680[];
extern s32 g_str_gpu_timeout;
extern s32 D_80016044;
extern u32 *g_gpu_dma_madr;
extern s32 *D_8009BF64;
extern s32 D_8009BF68[];
extern s32 D_8009BF6C;
extern s32 D_8009BF70;
extern s32 debug_printf();
s32 func_8007D9C4(s32 arg0) {
    D_8009BF88 = motion_make_table(0);
    D_8009BF7C = 0;
    D_8009BF78 = D_8009BF7C;
    switch (arg0 & 7) {
    case 5:
    case 0:
        *D_8009BF54 = 0x401;
        *D_8009BF64 |= 0x800;
        *D_8009BF48 = 0;
        bb2_memset(D_800F189C, 0, 0x100);
        bb2_memset(D_80103680, 0, 0x1800);
        break;
    case 1:
    case 3:
        *D_8009BF54 = 0x401;
        *D_8009BF64 |= 0x800;
        *D_8009BF48 = 0x02000000;
        *D_8009BF48 = 0x01000000;
        break;
    }
    motion_make_table(D_8009BF88);
    if (arg0 & 7) {
        return 0;
    }
    return func_8007DE08(arg0);
}
extern s32 func_8007D6D8();
s32 func_8007DB20(s32 arg0) {
    s32 temp_s0;
    s32 ret;

    if (arg0 == 0) {
        motion_LoadPreCalcData_8007DC68();
        while (D_8009BF78 != D_8009BF7C) {
            func_8007D6D8();
            if (func_8007DC9C() != 0) return -1;
        }
        while ((*g_gpu_dma_chcr & 0x01000000) || !(*g_gpu_stat_reg & 0x04000000)) {
            if (func_8007DC9C() != 0) return -1;
        }
        return 0;
    }
    temp_s0 = (D_8009BF78 - D_8009BF7C) & 0x3F;
    if (temp_s0 != 0) {
        func_8007D6D8();
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
void motion_LoadPreCalcData_8007DC68(void) {
    g_gpu_vcount = sys_VSync(-1) + 0xF0;
    g_gpu_draw_count = 0;
}
s32 func_8007DC9C(void) {
    volatile s32 *new_var2;
    s32 temp_v0;
    s32 temp_v1;
    s32 new_var;
    new_var2 = &D_8009BF7C;
    if ((g_gpu_vcount < sys_VSync(-1)) || ((temp_v1 = g_gpu_draw_count, g_gpu_draw_count = temp_v1 + 1, (temp_v1 > 0xF0000) != 0))) {
        new_var = *g_gpu_stat_reg;
        debug_printf(&g_str_gpu_timeout, (D_8009BF78 - D_8009BF7C) & 0x3F, *g_gpu_stat_reg, *g_gpu_dma_chcr, *g_gpu_dma_madr);
        (void)new_var;
        debug_printf(&D_80016044, D_8009BF68[0], D_8009BF6C, D_8009BF70);
        temp_v0 = motion_make_table(0);
        D_8009BF7C = 0;
        D_8009BF88 = temp_v0;
        D_8009BF78 = *new_var2;
        *g_gpu_dma_chcr = 0x401;
        *D_8009BF64 |= 0x800;
        *g_gpu_stat_reg = 0x02000000;
        *g_gpu_stat_reg = 0x01000000;
        motion_make_table(D_8009BF88);
        return -1;
    }
    return 0;
}s32 func_8007DE08(s32 arg0) {
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
void bb2_memset(u8 *a0, u8 a1, s32 a2) {
    s32 i;
    for (i = a2 - 1; i != -1; i--) {
        *a0++ = a1;
    }
}
__asm__(
    ".set noreorder\n"
    ".set noat\n"
    "glabel func_8007DF10\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x49\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* 1 NOP after func_8007DF10 */
extern s32 math_SinLookup(s32);
s32 math_Sin(s32 a0) {
    s32 v;
    if (a0 < 0) {
        v = math_SinLookup((-a0) & 0xFFF);
        return -v;
    }
    return math_SinLookup(a0 & 0xFFF);
}
extern s16 g_sin_lut_q1[];
extern s16 g_sin_lut_q3[];
extern s16 g_cos_lut_q2[];
extern s16 g_cos_lut_q4[];

s32 math_SinLookup(s32 a0) {
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return g_sin_lut_q1[a0];
        }
        return g_sin_lut_q1[0x800 - a0];
    }
    if (a0 < 0xC01) {
        return -g_sin_lut_q3[a0];
    }
    return -g_sin_lut_q1[0x1000 - a0];
}
s32 math_Cos(s32 a0) {
    if (a0 < 0) {
        a0 = -a0;
    }
    a0 = a0 & 0xFFF;
    if (a0 < 0x801) {
        if (a0 < 0x401) {
            return g_sin_lut_q1[0x400 - a0];
        }
        return -g_cos_lut_q2[a0];
    }
    if (a0 < 0xC01) {
        return -g_sin_lut_q1[0xC00 - a0];
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

__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8007E094\n"
    "    lui    $at, %hi(g_gte_saved_ra)\n"
    "    sw     $ra, %lo(g_gte_saved_ra)($at)\n"
    "    jal    func_8007FEDC\n"
    "    nop\n"
    "    lui    $ra, %hi(g_gte_saved_ra)\n"
    "    lw     $ra, %lo(g_gte_saved_ra)($ra)\n"
    "    nop\n"
    "    mfc0   $v0, $12\n"
    "    lui    $v1, 0x4000\n"
    "    or     $v0, $v0, $v1\n"
    "    mtc0   $v0, $12\n"
    "    nop\n"
    "    addiu  $t0, $zero, 0x155\n"
    "    ctc2   $t0, $29\n"
    "    nop\n"
    "    addiu  $t0, $zero, 0x100\n"
    "    ctc2   $t0, $30\n"
    "    nop\n"
    "    addiu  $t0, $zero, 0x3E8\n"
    "    ctc2   $t0, $26\n"
    "    nop\n"
    "    addiu  $t0, $zero, -0x1062\n"
    "    ctc2   $t0, $27\n"
    "    nop\n"
    "    lui    $t0, 0x0140\n"
    "    ctc2   $t0, $28\n"
    "    nop\n"
    "    ctc2   $zero, $24\n"
    "    ctc2   $zero, $25\n"
    "    nop\n"
    "    jr     $ra\n"
    "    nop\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
PAD_NOPS_2; /* 2 NOPs after func_8007E094 */
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8007E11C\n"
    "    mtc2   $a0, $30\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $v0, $31\n"
    "    addiu  $at, $zero, 0x20\n"
    "    beq    $v0, $at, .L8007E198\n"
    "    nop\n"
    "    andi   $t0, $v0, 0x1\n"
    "    addiu  $t2, $zero, -0x2\n"
    "    and    $t2, $v0, $t2\n"
    "    addiu  $t1, $zero, 0x1F\n"
    "    sub    $t1, $t1, $t2\n"
    "    sra    $t1, $t1, 1\n"
    "    addi   $t3, $t2, -0x18\n"
    "    bltz   $t3, .L8007E164\n"
    "    nop\n"
    "    sllv   $t4, $a0, $t3\n"
    "    b      .L8007E170\n"
    ".L8007E164:\n"
    "    addiu  $t3, $zero, 0x18\n"
    "    sub    $t3, $t3, $t2\n"
    "    srav   $t4, $a0, $t3\n"
    ".L8007E170:\n"
    "    addi   $t4, $t4, -0x40\n"
    "    sll    $t4, $t4, 1\n"
    "    lui    $t5, %hi(g_gte_sqrt_table)\n"
    "    addu   $t5, $t5, $t4\n"
    "    lh     $t5, %lo(g_gte_sqrt_table)($t5)\n"
    "    nop\n"
    "    sllv   $t5, $t5, $t1\n"
    "    srl    $v0, $t5, 12\n"
    "    jr     $ra\n"
    "    nop\n"
    ".L8007E198:\n"
    "    jr     $ra\n"
    "    addiu  $v0, $zero, 0x0\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
PAD_NOPS_3; /* 3 NOPs after func_8007E11C */
s32 func_8007E1AC(s32 *a0, s32 *a1, s32 a2, s32 a3, s32 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ volatile (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ volatile (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B98003D");             /* gpf 1 */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0xC8A90000" :: "r"(a1));  /* lwc2 $9, 0($a1) */
    __asm__ volatile (".word 0xC8AA0004" :: "r"(a1));  /* lwc2 $10, 4($a1) */
    __asm__ volatile (".word 0xC8AB0008" :: "r"(a1));  /* lwc2 $11, 8($a1) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4BA8003E");             /* gpl 1 */
    __asm__ volatile (".word 0x8FA80010");             /* lw $t0, 0x10($sp) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0xE9090000");             /* swc2 $9, 0($t0) */
    __asm__ volatile (".word 0xE90A0004");             /* swc2 $10, 4($t0) */
    __asm__ volatile (".word 0xE90B0008");             /* swc2 $11, 8($t0) */
    (void)out;
    return v0;
}
s32 func_8007E1FC(s32 *a0, s32 *a1, s32 a2, s32 a3, s32 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ volatile (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ volatile (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B90003D");             /* gpf 0 */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0xC8A90000" :: "r"(a1));  /* lwc2 $9, 0($a1) */
    __asm__ volatile (".word 0xC8AA0004" :: "r"(a1));  /* lwc2 $10, 4($a1) */
    __asm__ volatile (".word 0xC8AB0008" :: "r"(a1));  /* lwc2 $11, 8($a1) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4BA0003E");             /* gpl 0 */
    __asm__ volatile (".word 0x8FA80010");             /* lw $t0, 0x10($sp) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0xE9090000");             /* swc2 $9, 0($t0) */
    __asm__ volatile (".word 0xE90A0004");             /* swc2 $10, 4($t0) */
    __asm__ volatile (".word 0xE90B0008");             /* swc2 $11, 8($t0) */
    (void)out;
    return v0;
}
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
s32 func_8007E24C(s32 *a0, s32 *a1, s32 a2, s32 a3, s32 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x8C880000" :: "r"(a0));  /* lw $t0, 0($a0) */
    __asm__ volatile (".word 0x8C8A0004" :: "r"(a0));  /* lw $t2, 4($a0) */
    __asm__ volatile (".word 0x00084C03");             /* sra $t1, $t0, 16 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x314AFFFF");             /* andi $t2, $t2, 0xFFFF */
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B98003D");             /* gpf 1 */
    __asm__ volatile (".word 0x8CA80000" :: "r"(a1));  /* lw $t0, 0($a1) */
    __asm__ volatile (".word 0x8CAA0004" :: "r"(a1));  /* lw $t2, 4($a1) */
    __asm__ volatile (".word 0x00084C03");             /* sra $t1, $t0, 16 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x314AFFFF");             /* andi $t2, $t2, 0xFFFF */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4BA8003E");             /* gpl 1 */
    __asm__ volatile (".word 0x48084800");             /* mfc2 $t0, $9 */
    __asm__ volatile (".word 0x48095000");             /* mfc2 $t1, $10 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x00094C00");             /* sll $t1, $t1, 16 */
    __asm__ volatile (".word 0x01094025");             /* or $t0, $t0, $t1 */
    __asm__ volatile (".word 0x8FAD0010");             /* lw $t5, 0x10($sp) */
    __asm__ volatile (".word 0x480A5800");             /* mfc2 $t2, $11 */
    __asm__ volatile (".word 0xADA80000");             /* sw $t0, 0($t5) */
    __asm__ volatile (".word 0xADAA0004");             /* sw $t2, 4($t5) */
    (void)out;
    return v0;
}
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
s32 func_8007E2D4(s32 *a0, s32 *a1, s32 a2, s32 a3, s32 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x8C880000" :: "r"(a0));  /* lw $t0, 0($a0) */
    __asm__ volatile (".word 0x8C8A0004" :: "r"(a0));  /* lw $t2, 4($a0) */
    __asm__ volatile (".word 0x00084C03");             /* sra $t1, $t0, 16 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x314AFFFF");             /* andi $t2, $t2, 0xFFFF */
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B90003D");             /* gpf 0 */
    __asm__ volatile (".word 0x8CA80000" :: "r"(a1));  /* lw $t0, 0($a1) */
    __asm__ volatile (".word 0x8CAA0004" :: "r"(a1));  /* lw $t2, 4($a1) */
    __asm__ volatile (".word 0x00084C03");             /* sra $t1, $t0, 16 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x314AFFFF");             /* andi $t2, $t2, 0xFFFF */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4BA0003E");             /* gpl 0 */
    __asm__ volatile (".word 0x48084800");             /* mfc2 $t0, $9 */
    __asm__ volatile (".word 0x48095000");             /* mfc2 $t1, $10 */
    __asm__ volatile (".word 0x3108FFFF");             /* andi $t0, $t0, 0xFFFF */
    __asm__ volatile (".word 0x00094C00");             /* sll $t1, $t1, 16 */
    __asm__ volatile (".word 0x01094025");             /* or $t0, $t0, $t1 */
    __asm__ volatile (".word 0x8FAD0010");             /* lw $t5, 0x10($sp) */
    __asm__ volatile (".word 0x480A5800");             /* mfc2 $t2, $11 */
    __asm__ volatile (".word 0xADA80000");             /* sw $t0, 0($t5) */
    __asm__ volatile (".word 0xADAA0004");             /* sw $t2, 4($t5) */
    (void)out;
    return v0;
}
s32 func_8007E35C(u8 *a0, u8 *a1, s32 a2, s32 a3, u8 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x90880000" :: "r"(a0));  /* lbu $t0, 0($a0) */
    __asm__ volatile (".word 0x90890001" :: "r"(a0));  /* lbu $t1, 1($a0) */
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B90003D");             /* gpf 0 */
    __asm__ volatile (".word 0x90A80000" :: "r"(a1));  /* lbu $t0, 0($a1) */
    __asm__ volatile (".word 0x90A90001" :: "r"(a1));  /* lbu $t1, 1($a1) */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x240B000C");             /* addiu $t3, $zero, 0xC */
    __asm__ volatile (".word 0x4BA0003E");             /* gpl 0 */
    __asm__ volatile (".word 0x8FAD0010");             /* lw $t5, 0x10($sp) */
    __asm__ volatile (".word 0x4808C800");             /* mfc2 $t0, $25 */
    __asm__ volatile (".word 0x4809D000");             /* mfc2 $t1, $26 */
    __asm__ volatile (".word 0x01684007");             /* srav $t0, $t0, $t3 */
    __asm__ volatile (".word 0x01694807");             /* srav $t1, $t1, $t3 */
    __asm__ volatile (".word 0xA1A80000");             /* sb $t0, 0($t5) */
    __asm__ volatile (".word 0xA1A90001");             /* sb $t1, 1($t5) */
    (void)out;
    return v0;
}
s32 func_8007E3BC(u8 *a0, u8 *a1, s32 a2, s32 a3, u8 *out) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x90880000" :: "r"(a0));  /* lbu $t0, 0($a0) */
    __asm__ volatile (".word 0x90890001" :: "r"(a0));  /* lbu $t1, 1($a0) */
    __asm__ volatile (".word 0x908A0002" :: "r"(a0));  /* lbu $t2, 2($a0) */
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B90003D");             /* gpf 0 */
    __asm__ volatile (".word 0x90A80000" :: "r"(a1));  /* lbu $t0, 0($a1) */
    __asm__ volatile (".word 0x90A90001" :: "r"(a1));  /* lbu $t1, 1($a1) */
    __asm__ volatile (".word 0x90AA0002" :: "r"(a1));  /* lbu $t2, 2($a1) */
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile (".word 0x48884800");             /* mtc2 $t0, $9 */
    __asm__ volatile (".word 0x48895000");             /* mtc2 $t1, $10 */
    __asm__ volatile (".word 0x488A5800");             /* mtc2 $t2, $11 */
    __asm__ volatile (".word 0x240B000C");             /* addiu $t3, $zero, 0xC */
    __asm__ volatile (".word 0x4BA0003E");             /* gpl 0 */
    __asm__ volatile (".word 0x8FAD0010");             /* lw $t5, 0x10($sp) */
    __asm__ volatile (".word 0x4808C800");             /* mfc2 $t0, $25 */
    __asm__ volatile (".word 0x4809D000");             /* mfc2 $t1, $26 */
    __asm__ volatile (".word 0x480AD800");             /* mfc2 $t2, $27 */
    __asm__ volatile (".word 0x01684007");             /* srav $t0, $t0, $t3 */
    __asm__ volatile (".word 0x01694807");             /* srav $t1, $t1, $t3 */
    __asm__ volatile (".word 0x016A5007");             /* srav $t2, $t2, $t3 */
    __asm__ volatile (".word 0xA1A80000");             /* sb $t0, 0($t5) */
    __asm__ volatile (".word 0xA1A90001");             /* sb $t1, 1($t5) */
    __asm__ volatile (".word 0xA1AA0002");             /* sb $t2, 2($t5) */
    (void)out;
    return v0;
}
PAD_NOPS_1; /* 1 NOP after func_8007E1AC */
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8007E43C\n"
    "    mtc2   $a0, $30\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $v0, $31\n"
    "    addiu  $at, $zero, 0x20\n"
    "    beq    $v0, $at, .L8007E4C8\n"
    "    nop\n"
    "    andi   $t0, $v0, 0x1\n"
    "    addiu  $t2, $zero, -0x2\n"
    "    and    $t2, $v0, $t2\n"
    "    addiu  $t1, $zero, 0x13\n"
    "    sub    $t1, $t1, $t2\n"
    "    sra    $t1, $t1, 1\n"
    "    addi   $t3, $t2, -0x18\n"
    "    bltz   $t3, .L8007E484\n"
    "    nop\n"
    "    sllv   $t4, $a0, $t3\n"
    "    b      .L8007E490\n"
    ".L8007E484:\n"
    "    addiu  $t3, $zero, 0x18\n"
    "    sub    $t3, $t3, $t2\n"
    "    srav   $t4, $a0, $t3\n"
    ".L8007E490:\n"
    "    addi   $t4, $t4, -0x40\n"
    "    sll    $t4, $t4, 1\n"
    "    lui    $t5, %hi(g_gte_sqrt_table)\n"
    "    addu   $t5, $t5, $t4\n"
    "    lh     $t5, %lo(g_gte_sqrt_table)($t5)\n"
    "    nop\n"
    "    bltz   $t1, .L8007E4BC\n"
    "    nop\n"
    "    sllv   $v0, $t5, $t1\n"
    "    jr     $ra\n"
    "    nop\n"
    ".L8007E4BC:\n"
    "    neg    $t1, $t1\n"
    "    jr     $ra\n"
    "    srlv   $v0, $t5, $t1\n"
    ".L8007E4C8:\n"
    "    jr     $ra\n"
    "    addiu  $v0, $zero, 0x0\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
PAD_NOPS_3; /* 3 NOPs after func_8007E43C */
void func_8007E4DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007E4DC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
PAD_NOPS_1; /* 1 NOP after func_8007E4DC */
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "glabel func_8007E5EC\n"
    "    lw $t0, 0($a0)\n"
    "    lw $t1, 4($a0)\n"
    "    lw $t2, 8($a0)\n"
    "    lw $t3, 12($a0)\n"
    "    lw $t4, 16($a0)\n"
    "    ctc2 $t0, $0\n"
    "    ctc2 $t1, $1\n"
    "    ctc2 $t2, $2\n"
    "    ctc2 $t3, $3\n"
    "    ctc2 $t4, $4\n"
    "    lhu $t0, 0($a1)\n"
    "    lw $t1, 4($a1)\n"
    "    lw $t2, 12($a1)\n"
    "    lui $at, (0xFFFF0000 >> 16)\n"
    "    and $t1, $t1, $at\n"
    "    or $t0, $t0, $t1\n"
    "    mtc2 $t0, $0\n"
    "    mtc2 $t2, $1\n"
    "    nop\n"
    "    mvmva 1, 0, 0, 3, 0\n"
    "    lhu $t0, 2($a1)\n"
    "    lw $t1, 8($a1)\n"
    "    lh $t2, 14($a1)\n"
    "    sll $t1, $t1, 16\n"
    "    or $t0, $t0, $t1\n"
    "    mfc2 $t3, $9\n"
    "    mfc2 $t4, $10\n"
    "    mfc2 $t5, $11\n"
    "    mtc2 $t0, $0\n"
    "    mtc2 $t2, $1\n"
    "    nop\n"
    "    mvmva 1, 0, 0, 3, 0\n"
    "    lhu $t0, 4($a1)\n"
    "    lw $t1, 8($a1)\n"
    "    lw $t2, 16($a1)\n"
    "    lui $at, (0xFFFF0000 >> 16)\n"
    "    and $t1, $t1, $at\n"
    "    or $t0, $t0, $t1\n"
    "    mfc2 $t6, $9\n"
    "    mfc2 $t7, $10\n"
    "    mfc2 $t8, $11\n"
    "    mtc2 $t0, $0\n"
    "    mtc2 $t2, $1\n"
    "    nop\n"
    "    mvmva 1, 0, 0, 3, 0\n"
    "    andi $t3, $t3, 0xFFFF\n"
    "    sll $t6, $t6, 16\n"
    "    or $t6, $t6, $t3\n"
    "    sw $t6, 0($a2)\n"
    "    andi $t5, $t5, 0xFFFF\n"
    "    sll $t8, $t8, 16\n"
    "    or $t8, $t8, $t5\n"
    "    sw $t8, 12($a2)\n"
    "    mfc2 $t0, $9\n"
    "    mfc2 $t1, $10\n"
    "    swc2 $11, 16($a2)\n"
    "    lhu $t5, 20($a1)\n"
    "    lw $t6, 24($a1)\n"
    "    lw $t2, 28($a1)\n"
    "    sll $t6, $t6, 16\n"
    "    or $t5, $t5, $t6\n"
    "    mtc2 $t5, $0\n"
    "    mtc2 $t2, $1\n"
    "    nop\n"
    "    mvmva 1, 0, 0, 3, 0\n"
    "    sll $t4, $t4, 16\n"
    "    andi $t0, $t0, 0xFFFF\n"
    "    or $t0, $t0, $t4\n"
    "    sw $t0, 4($a2)\n"
    "    andi $t7, $t7, 0xFFFF\n"
    "    sll $t1, $t1, 16\n"
    "    or $t1, $t1, $t7\n"
    "    sw $t1, 8($a2)\n"
    "    mfc2 $t0, $25\n"
    "    mfc2 $t1, $26\n"
    "    mfc2 $t2, $27\n"
    "    lw $t3, 20($a0)\n"
    "    lw $t4, 24($a0)\n"
    "    lw $t5, 28($a0)\n"
    "    add $t0, $t0, $t3\n"
    "    add $t1, $t1, $t4\n"
    "    add $t2, $t2, $t5\n"
    "    sw $t0, 20($a2)\n"
    "    sw $t1, 24($a2)\n"
    "    sw $t2, 28($a2)\n"
    "    addu $v0, $a2, $zero\n"
    "    jr $ra\n"
    "    nop\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
void func_8007E74C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007E74C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
s32 *func_8007E8AC(s32 *a0, s32 *a1, s32 *a2) {
    register s32 *v0 asm("v0");
    __asm__ volatile (".word 0x8C880000" :: "r"(a0));   /* lw $t0, 0($a0) */
    __asm__ volatile (".word 0x8C890004" :: "r"(a0));   /* lw $t1, 4($a0) */
    __asm__ volatile (".word 0x48880000");               /* mtc2 $t0, $0 */
    __asm__ volatile (".word 0x48890800");               /* mtc2 $t1, $1 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");               /* mvmva 1,0,0,3,0 */
    __asm__ volatile (".word 0xE8A90000" :: "r"(a1));   /* swc2 $9, 0($a1) */
    __asm__ volatile (".word 0xE8AA0004" :: "r"(a1));   /* swc2 $10, 4($a1) */
    __asm__ volatile (".word 0xE8AB0008" :: "r"(a1));   /* swc2 $11, 8($a1) */
    __asm__ volatile ("addu %0, %1, $zero" : "=r"(v0) : "r"(a2));
    return v0;
}
void *func_8007E8DC(s32 *arg0, s32 *arg1) {
    register s32 t0 asm("t0");
    register s32 t1 asm("t1");
    register s32 t2 asm("t2");
    register s32 t3 asm("t3");
    register s32 t4 asm("t4");
    register s32 t5 asm("t5");
    s32 *v0;

    asm volatile("lw %0, 0(%1)" : "=r"(t0) : "r"(arg0));
    asm volatile("lw %0, 0(%1)" : "=r"(t3) : "r"(arg1));
    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "r"(t0));
    asm volatile("sll %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("sra %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t1), "r"(t3));
    asm volatile("sra %0, %1, 16" : "=r"(t2) : "r"(t0));
    asm volatile("lw %0, 4(%1)" : "=r"(t4) : "r"(arg1));
    asm volatile("lw %0, 8(%1)" : "=r"(t5) : "r"(arg1));
    asm volatile("lw %0, 4(%1)" : "=r"(t0) : "r"(arg0));
    asm volatile("addu %0, %1, $0" : "=r"(v0) : "r"(arg0));
    asm volatile("mflo %0" : "=r"(t1));
    asm volatile("sra %0, %1, 12" : "=r"(t1) : "0"(t1));
    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t2), "r"(t3));
    asm volatile("mflo %0" : "=r"(t2));
    asm volatile("sra %0, %1, 12" : "=r"(t2) : "0"(t2));
    asm volatile("sll %0, %1, 16" : "=r"(t2) : "0"(t2));
    asm volatile("or %0, %1, %2" : "=r"(t1) : "0"(t1), "r"(t2));
    asm volatile("sw %0, 0(%1)" : : "r"(t1), "r"(arg0));

    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "r"(t0));
    asm volatile("sll %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("sra %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t1), "r"(t3));
    asm volatile("sra %0, %1, 16" : "=r"(t2) : "r"(t0));
    asm volatile("lw %0, 8(%1)" : "=r"(t0) : "r"(arg0));
    asm volatile("mflo %0" : "=r"(t1));
    asm volatile("sra %0, %1, 12" : "=r"(t1) : "0"(t1));
    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t2), "r"(t4));
    asm volatile("mflo %0" : "=r"(t2));
    asm volatile("sra %0, %1, 12" : "=r"(t2) : "0"(t2));
    asm volatile("sll %0, %1, 16" : "=r"(t2) : "0"(t2));
    asm volatile("or %0, %1, %2" : "=r"(t1) : "0"(t1), "r"(t2));
    asm volatile("sw %0, 4(%1)" : : "r"(t1), "r"(arg0));

    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "r"(t0));
    asm volatile("sll %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("sra %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t1), "r"(t4));
    asm volatile("sra %0, %1, 16" : "=r"(t2) : "r"(t0));
    asm volatile("lw %0, 12(%1)" : "=r"(t0) : "r"(arg0));
    asm volatile("mflo %0" : "=r"(t1));
    asm volatile("sra %0, %1, 12" : "=r"(t1) : "0"(t1));
    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t2), "r"(t4));
    asm volatile("mflo %0" : "=r"(t2));
    asm volatile("sra %0, %1, 12" : "=r"(t2) : "0"(t2));
    asm volatile("sll %0, %1, 16" : "=r"(t2) : "0"(t2));
    asm volatile("or %0, %1, %2" : "=r"(t1) : "0"(t1), "r"(t2));
    asm volatile("sw %0, 8(%1)" : : "r"(t1), "r"(arg0));

    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "r"(t0));
    asm volatile("sll %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("sra %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t1), "r"(t5));
    asm volatile("sra %0, %1, 16" : "=r"(t2) : "r"(t0));
    asm volatile("lw %0, 16(%1)" : "=r"(t0) : "r"(arg0));
    asm volatile("mflo %0" : "=r"(t1));
    asm volatile("sra %0, %1, 12" : "=r"(t1) : "0"(t1));
    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t2), "r"(t5));
    asm volatile("mflo %0" : "=r"(t2));
    asm volatile("sra %0, %1, 12" : "=r"(t2) : "0"(t2));
    asm volatile("sll %0, %1, 16" : "=r"(t2) : "0"(t2));
    asm volatile("or %0, %1, %2" : "=r"(t1) : "0"(t1), "r"(t2));
    asm volatile("sw %0, 12(%1)" : : "r"(t1), "r"(arg0));

    asm volatile("andi %0, %1, 0xFFFF" : "=r"(t1) : "r"(t0));
    asm volatile("sll %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("sra %0, %1, 16" : "=r"(t1) : "0"(t1));
    asm volatile("multu %0, %1" : : "r"(t1), "r"(t5));
    asm volatile("mflo %0" : "=r"(t1));
    asm volatile("sra %0, %1, 12" : "=r"(t1) : "0"(t1));
    asm volatile("sw %0, 16(%1)" : : "r"(t1), "r"(arg0));

    return v0;
}
PAD_NOPS_3; /* 3 NOPs after func_8007E8DC */
void func_8007EA0C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007EA0C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
PAD_NOPS_2; /* 2 NOPs after func_8007EA0C */
void func_8007EB4C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007EB4C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
PAD_NOPS_1; /* 1 NOP after func_8007EB4C */
void calc_fc_frame_8007EC5C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/calc_fc_frame_8007EC5C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
PAD_NOPS_1; /* 1 NOP after calc_fc_frame_8007EC5C */
s32 *func_8007ED6C(s32 *a0, s32 *a1, s32 *a2) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 *v0 asm("v0");
    t0 = a0[0];
    t1 = a0[1];
    t2 = a0[2];
    t3 = a0[3];
    t4 = a0[4];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("lwc2 $0, 0(%0)" :: "r"(a1));
    __asm__ volatile ("lwc2 $1, 4(%0)" :: "r"(a1));
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A486012");               /* mvmva 1,0,0,3,0 */
    __asm__ volatile ("swc2 $25, 0(%0)" :: "r"(a2));
    __asm__ volatile (".word 0xE8DA0004" :: "r"(a2));   /* swc2 $26, 4($a2) */
    __asm__ volatile (".word 0xE8DB0008" :: "r"(a2));   /* swc2 $27, 8($a2) */
    __asm__ volatile ("move %0, %1" : "=r"(v0) : "r"(a2));
    return v0;
}

s32 *func_8007EDBC(volatile s32 *arg0, s32 *arg1)
{
  register s32 m0 asm("t3") = arg1[0];
  register s32 m1 asm("t4") = arg1[1];
  register s32 m2 asm("t5") = arg1[2];
  register s32 w asm("t0");
  register s32 t1 asm("t1");
  register s32 t2 asm("t2");
  s32 *ret;
  w = arg0[0];
  t1 = (s32) ((s16) (w & 0xFFFF));
  t2 = w >> 16;
  t1 = t1 * m0;
  t2 = t2 * m1;
  ret = (s32 *) arg0;
  t1 = (t1 >> 12) & 0xFFFF;
  t2 = (t2 >> 12) << 16;
  arg0[0] = t1 | t2;
  w = arg0[1];
  t1 = (s32) ((s16) (w & 0xFFFF));
  t2 = w >> 16;
  t1 = t1 * m2;
  t2 = t2 * m0;
  t1 = (t1 >> 12) & 0xFFFF;
  t2 = (t2 >> 12) << 16;
  arg0[1] = t1 | t2;
  w = arg0[2];
  t1 = (s32) ((s16) (w & 0xFFFF));
  t2 = w >> 16;
  t1 = t1 * m1;
  t2 = t2 * m2;
  t1 = (t1 >> 12) & 0xFFFF;
  t2 = (t2 >> 12) << 16;
  arg0[2] = t1 | t2;
  w = arg0[3];
  t1 = (s32) ((s16) (w & 0xFFFF));
  t2 = w >> 16;
  t1 = t1 * m0;
  t2 = t2 * m1;
  t1 = (t1 >> 12) & 0xFFFF;
  t2 = (t2 >> 12) << 16;
  arg0[3] = t1 | t2;
  w = arg0[4];
  t1 = (s32) ((s16) (w & 0xFFFF));
  t1 = t1 * m2;
  arg0++;
  arg0--;
  arg0[4] = t1 >> 12;
  return ret;
}
PAD_NOPS_3; /* 3 NOPs after func_8007EDBC */
void gte_SetRotMatrix(s32 *a0) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 t2 asm("t2") = a0[2];
    register s32 t3 asm("t3") = a0[3];
    register s32 t4 asm("t4") = a0[4];
    __asm__ volatile (".word 0x48C80000" :: "r"(t0));  /* ctc2 $t0, $0 */
    __asm__ volatile (".word 0x48C90800" :: "r"(t1));  /* ctc2 $t1, $1 */
    __asm__ volatile (".word 0x48CA1000" :: "r"(t2));  /* ctc2 $t2, $2 */
    __asm__ volatile (".word 0x48CB1800" :: "r"(t3));  /* ctc2 $t3, $3 */
    __asm__ volatile (".word 0x48CC2000" :: "r"(t4));  /* ctc2 $t4, $4 */
}
void gte_SetColorMatrix(s32 *a0) {
    register s32 t0 asm("t0") = a0[0];
    register s32 t1 asm("t1") = a0[1];
    register s32 t2 asm("t2") = a0[2];
    register s32 t3 asm("t3") = a0[3];
    register s32 t4 asm("t4") = a0[4];
    __asm__ volatile (".word 0x48C88000" :: "r"(t0));  /* ctc2 $t0, $16 */
    __asm__ volatile (".word 0x48C98800" :: "r"(t1));  /* ctc2 $t1, $17 */
    __asm__ volatile (".word 0x48CA9000" :: "r"(t2));  /* ctc2 $t2, $18 */
    __asm__ volatile (".word 0x48CB9800" :: "r"(t3));  /* ctc2 $t3, $19 */
    __asm__ volatile (".word 0x48CCA000" :: "r"(t4));  /* ctc2 $t4, $20 */
}
void gte_SetTransVector(s32 *a0) {
    register s32 t0 asm("t0") = a0[5];
    register s32 t1 asm("t1") = a0[6];
    register s32 t2 asm("t2") = a0[7];
    __asm__ volatile (".word 0x48C82800" :: "r"(t0));  /* ctc2 $t0, $5 */
    __asm__ volatile (".word 0x48C93000" :: "r"(t1));  /* ctc2 $t1, $6 */
    __asm__ volatile (".word 0x48CA3800" :: "r"(t2));  /* ctc2 $t2, $7 */
}
void gte_GetScreenXY(s32 *a0, s32 *a1, s32 *a2) {
    __asm__ volatile (".word 0xE8910000" :: "r"(a0));  /* swc2 $17, 0($a0) */
    __asm__ volatile (".word 0xE8B20000" :: "r"(a1));  /* swc2 $18, 0($a1) */
    __asm__ volatile (".word 0xE8D30000" :: "r"(a2));  /* swc2 $19, 0($a2) */
}
PAD_NOPS_3; /* 3 NOPs after gte_GetScreenXY */
s32 gte_GetH(void) { s32 ret; __asm__ volatile (".word 0x4842D000" : "=r" (ret)); return ret; }
PAD_NOPS_1; /* 1 NOP after gte_GetH */
void gte_SetBackColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile (".word 0x48C46800" :: "r"(a0));  /* ctc2 $a0, $13 */
    __asm__ volatile (".word 0x48C57000" :: "r"(a1));  /* ctc2 $a1, $14 */
    __asm__ volatile (".word 0x48C67800" :: "r"(a2));  /* ctc2 $a2, $15 */
}
void gte_SetFarColor(s32 a0, s32 a1, s32 a2) {
    a0 <<= 4;
    a1 <<= 4;
    a2 <<= 4;
    __asm__ volatile (".word 0x48C4A800" :: "r"(a0));  /* ctc2 $a0, $21 */
    __asm__ volatile (".word 0x48C5B000" :: "r"(a1));  /* ctc2 $a1, $22 */
    __asm__ volatile (".word 0x48C6B800" :: "r"(a2));  /* ctc2 $a2, $23 */
}
void gte_SetScreenOffset(s32 a0, s32 a1) {
    a0 <<= 16;
    a1 <<= 16;
    __asm__ volatile (".word 0x48C4C000" :: "r"(a0));  /* ctc2 $a0, $24 */
    __asm__ volatile (".word 0x48C5C800" :: "r"(a1));  /* ctc2 $a1, $25 */
}
PAD_NOPS_2; /* 2 NOPs after gte_SetScreenOffset */
void tslDmaDrawListDelAll(s32 a0) {
    __asm__ volatile (".word 0x48C4D000" :: "r"(a0));  /* ctc2 $a0, $26 */
}
PAD_NOPS_1; /* 1 NOP after tslDmaDrawListDelAll */
void func_8007F00C(s32 *a0, s32 *a1) {
    __asm__ volatile (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ volatile (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ volatile (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A4DA412");              /* mvmva 1,2,3,1,1 */
    __asm__ volatile (".word 0xE8A90000" :: "r"(a1));  /* swc2 $9, 0($a1) */
    __asm__ volatile (".word 0xE8AA0004" :: "r"(a1));  /* swc2 $10, 4($a1) */
    __asm__ volatile (".word 0xE8AB0008" :: "r"(a1));  /* swc2 $11, 8($a1) */
}
void func_8007F034(s32 *a0, s32 *a1, s32 a2, u32 *a3) {
    __asm__ volatile (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ volatile (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ volatile (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ volatile (".word 0xC8A60000" :: "r"(a1));  /* lwc2 $6, 0($a1) */
    __asm__ volatile (".word 0x48864000" :: "r"(a2));  /* mtc2 $a2, $8 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A680029");              /* dpcl */
    __asm__ volatile (".word 0xE8F60000" :: "r"(a3));  /* swc2 $22, 0($a3) */
}
void func_8007F05C(s32 *a0, s32 *a1, s32 *a2, s32 a3, s32 *o0, s32 *o1, s32 *o2) {
    __asm__ volatile (".word 0xC8940000" :: "r"(a0));  /* lwc2 $20, 0($a0) */
    __asm__ volatile (".word 0xC8B50000" :: "r"(a1));  /* lwc2 $21, 0($a1) */
    __asm__ volatile (".word 0xC8D60000" :: "r"(a2));  /* lwc2 $22, 0($a2) */
    __asm__ volatile (".word 0xC8C60000" :: "r"(a2));  /* lwc2 $6,  0($a2) */
    __asm__ volatile (".word 0x48874000" :: "r"(a3));  /* mtc2 $a3, $8 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4AF8002A");             /* dpct */
    __asm__ volatile (".word 0x8FA80010");             /* lw $t0, 0x10($sp) */
    __asm__ volatile (".word 0x8FA90014");             /* lw $t1, 0x14($sp) */
    __asm__ volatile (".word 0x8FAA0018");             /* lw $t2, 0x18($sp) */
    __asm__ volatile (".word 0xE9140000");             /* swc2 $20, 0($t0) */
    __asm__ volatile (".word 0xE9350000");             /* swc2 $21, 0($t1) */
    __asm__ volatile (".word 0xE9560000");             /* swc2 $22, 0($t2) */
    (void)o0; (void)o1; (void)o2;
}
void func_8007F098(s32 *a0, s32 a1, u32 *a2) {
    __asm__ volatile (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ volatile (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ volatile (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ volatile (".word 0x48854000" :: "r"(a1));  /* mtc2 $a1, $8 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A980011");              /* intpl */
    __asm__ volatile (".word 0xE8D60000" :: "r"(a2));  /* swc2 $22, 0($a2) */
}
s32 *func_8007F0BC(s32 *a0, s32 *a1) {
    __asm__ (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ ("nop");
    __asm__ (".word 0x4AA80428");              /* sqr 1 */
    __asm__ (".word 0xE8B90000" :: "r"(a1));  /* swc2 $25, 0($a1) */
    __asm__ (".word 0xE8BA0004" :: "r"(a1));  /* swc2 $26, 4($a1) */
    __asm__ (".word 0xE8BB0008" :: "r"(a1));  /* swc2 $27, 8($a1) */
    return a1;
}
s32 *func_8007F0E4(s32 *a0, s32 *a1) {
    __asm__ (".word 0xC8890000" :: "r"(a0));  /* lwc2 $9, 0($a0) */
    __asm__ (".word 0xC88A0004" :: "r"(a0));  /* lwc2 $10, 4($a0) */
    __asm__ (".word 0xC88B0008" :: "r"(a0));  /* lwc2 $11, 8($a0) */
    __asm__ ("nop");
    __asm__ (".word 0x4AA00428");              /* sqr 0 */
    __asm__ (".word 0xE8B90000" :: "r"(a1));  /* swc2 $25, 0($a1) */
    __asm__ (".word 0xE8BA0004" :: "r"(a1));  /* swc2 $26, 4($a1) */
    __asm__ (".word 0xE8BB0008" :: "r"(a1));  /* swc2 $27, 8($a1) */
    return a1;
}
s32 func_8007F10C(s32 a0, s32 a1, s32 a2) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x48848800" :: "r"(a0));  /* mtc2 $a0, $17 */
    __asm__ volatile (".word 0x48859000" :: "r"(a1));  /* mtc2 $a1, $18 */
    __asm__ volatile (".word 0x48869800" :: "r"(a2));  /* mtc2 $a2, $19 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B58002D");              /* avsz3 */
    __asm__ volatile (".word 0x48023800" : "=r"(v0));  /* mfc2 $v0, $7 */
    return v0;
}
__asm__(
    ".section .text\n"
    "    .set\tnoat\n"
    "    .set\tnoreorder\n"
    "    .set noat\n"
    "    .set noreorder\n"
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
s32 func_8007F12C(s32 a0, s32 a1, s32 a2, s32 a3) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x48848000" :: "r"(a0));  /* mtc2 $a0, $16 */
    __asm__ volatile (".word 0x48858800" :: "r"(a1));  /* mtc2 $a1, $17 */
    __asm__ volatile (".word 0x48869000" :: "r"(a2));  /* mtc2 $a2, $18 */
    __asm__ volatile (".word 0x48879800" :: "r"(a3));  /* mtc2 $a3, $19 */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B68002E");              /* avsz4 */
    __asm__ volatile (".word 0x48023800" : "=r"(v0));  /* mfc2 $v0, $7 */
    return v0;
}
void func_8007F150(s32 *a0, s16 *a1, s32 *a2) {
    __asm__ volatile (".word 0x484D0000");             /* cfc2 $t5, $0 */
    __asm__ volatile (".word 0x484E1000");             /* cfc2 $t6, $2 */
    __asm__ volatile (".word 0x484F2000");             /* cfc2 $t7, $4 */
    __asm__ volatile (".word 0x8C880000" :: "r"(a0));  /* lw $t0, 0($a0) */
    __asm__ volatile (".word 0x8C890004" :: "r"(a0));  /* lw $t1, 4($a0) */
    __asm__ volatile (".word 0x8C8A0008" :: "r"(a0));  /* lw $t2, 8($a0) */
    __asm__ volatile (".word 0x48C80000");             /* ctc2 $t0, $0 */
    __asm__ volatile (".word 0x48C91000");             /* ctc2 $t1, $2 */
    __asm__ volatile (".word 0x48CA2000");             /* ctc2 $t2, $4 */
    __asm__ volatile (".word 0xC8AB0008" :: "r"(a1));  /* lwc2 $11, 8($a1) */
    __asm__ volatile (".word 0xC8A90000" :: "r"(a1));  /* lwc2 $9,  0($a1) */
    __asm__ volatile (".word 0xC8AA0004" :: "r"(a1));  /* lwc2 $10, 4($a1) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B78000C");             /* op 1 */
    __asm__ volatile (".word 0xE8D90000" :: "r"(a2));  /* swc2 $25, 0($a2) */
    __asm__ volatile (".word 0xE8DA0004" :: "r"(a2));  /* swc2 $26, 4($a2) */
    __asm__ volatile (".word 0xE8DB0008" :: "r"(a2));  /* swc2 $27, 8($a2) */
    __asm__ volatile (".word 0x48CD0000");             /* ctc2 $t5, $0 */
    __asm__ volatile (".word 0x48CE1000");             /* ctc2 $t6, $2 */
    __asm__ volatile (".word 0x48CF2000");             /* ctc2 $t7, $4 */
}
void func_8007F1A8(s32 *a0, s16 *a1, s32 *a2) {
    __asm__ volatile (".word 0x484D0000");             /* cfc2 $t5, $0 */
    __asm__ volatile (".word 0x484E1000");             /* cfc2 $t6, $2 */
    __asm__ volatile (".word 0x484F2000");             /* cfc2 $t7, $4 */
    __asm__ volatile (".word 0x8C880000" :: "r"(a0));  /* lw $t0, 0($a0) */
    __asm__ volatile (".word 0x8C890004" :: "r"(a0));  /* lw $t1, 4($a0) */
    __asm__ volatile (".word 0x8C8A0008" :: "r"(a0));  /* lw $t2, 8($a0) */
    __asm__ volatile (".word 0x48C80000");             /* ctc2 $t0, $0 */
    __asm__ volatile (".word 0x48C91000");             /* ctc2 $t1, $2 */
    __asm__ volatile (".word 0x48CA2000");             /* ctc2 $t2, $4 */
    __asm__ volatile (".word 0xC8AB0008" :: "r"(a1));  /* lwc2 $11, 8($a1) */
    __asm__ volatile (".word 0xC8A90000" :: "r"(a1));  /* lwc2 $9,  0($a1) */
    __asm__ volatile (".word 0xC8AA0004" :: "r"(a1));  /* lwc2 $10, 4($a1) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4B70000C");             /* op 0 */
    __asm__ volatile (".word 0xE8D90000" :: "r"(a2));  /* swc2 $25, 0($a2) */
    __asm__ volatile (".word 0xE8DA0004" :: "r"(a2));  /* swc2 $26, 4($a2) */
    __asm__ volatile (".word 0xE8DB0008" :: "r"(a2));  /* swc2 $27, 8($a2) */
    __asm__ volatile (".word 0x48CD0000");             /* ctc2 $t5, $0 */
    __asm__ volatile (".word 0x48CE1000");             /* ctc2 $t6, $2 */
    __asm__ volatile (".word 0x48CF2000");             /* ctc2 $t7, $4 */
}
s32 func_8007F200(s32 a0) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0x4884F000" :: "r"(a0));  /* mtc2 $a0, $30 */
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4802F800" : "=r"(v0));  /* mfc2 $v0, $31 */
    return v0;
}
PAD_NOPS_1; /* 1 NOP after func_8007F200 */
s32 func_8007F21C(s32 *a0, s32 *a1, s32 *a2, s32 *a3) {
    register s32 v1 asm("v1");
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0xC8800000" :: "r"(a0));  /* lwc2 $0, 0($a0) */
    __asm__ volatile (".word 0xC8810004" :: "r"(a0));  /* lwc2 $1, 4($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A180001");              /* rtps */
    __asm__ volatile (".word 0xE8AE0000" :: "r"(a1));  /* swc2 $14, 0($a1) */
    __asm__ volatile (".word 0xE8C80000" :: "r"(a2));  /* swc2 $8, 0($a2) */
    __asm__ volatile (".word 0x4843F800" : "=r"(v1));  /* cfc2 $v1, $31 */
    __asm__ volatile (".word 0x48029800" : "=r"(v0));  /* mfc2 $v0, $19 */
    *a3 = v1;
    return v0 >> 2;
}
PAD_NOPS_1; /* 1 NOP after func_8007F21C */
s32 func_8007F24C(s16 *a0, s16 *a1, s16 *a2, s32 *a3, s32 *o0, s32 *o1, s32 *o2, s32 *o3) {
    register s32 v1 asm("v1");
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0xC8800000" :: "r"(a0));  /* lwc2 $0, 0($a0) */
    __asm__ volatile (".word 0xC8810004" :: "r"(a0));  /* lwc2 $1, 4($a0) */
    __asm__ volatile (".word 0xC8A20000" :: "r"(a1));  /* lwc2 $2, 0($a1) */
    __asm__ volatile (".word 0xC8A30004" :: "r"(a1));  /* lwc2 $3, 4($a1) */
    __asm__ volatile (".word 0xC8C40000" :: "r"(a2));  /* lwc2 $4, 0($a2) */
    __asm__ volatile (".word 0xC8C50004" :: "r"(a2));  /* lwc2 $5, 4($a2) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A280030");             /* rtpt */
    __asm__ volatile (".word 0x8FA80010");             /* lw $t0, 0x10($sp) */
    __asm__ volatile (".word 0x8FA90014");             /* lw $t1, 0x14($sp) */
    __asm__ volatile (".word 0x8FAA0018");             /* lw $t2, 0x18($sp) */
    __asm__ volatile (".word 0x8FAB001C");             /* lw $t3, 0x1C($sp) */
    __asm__ volatile (".word 0xE8EC0000" :: "r"(a3));  /* swc2 $12, 0($a3) */
    __asm__ volatile (".word 0xE90D0000");             /* swc2 $13, 0($t0) */
    __asm__ volatile (".word 0xE92E0000");             /* swc2 $14, 0($t1) */
    __asm__ volatile (".word 0xE9480000");             /* swc2 $8,  0($t2) */
    __asm__ volatile (".word 0x4843F800" : "=r"(v1));  /* cfc2 $v1, $31 */
    __asm__ volatile (".word 0x48029800" : "=r"(v0));  /* mfc2 $v0, $19 */
    __asm__ volatile (".word 0xAD630000");             /* sw $v1, 0x0($t3) */
    (void)o0; (void)o1; (void)o2; (void)o3;
    return v0 >> 2;
}
PAD_NOPS_3; /* 3 NOPs after func_8007F24C */
void func_8007F2AC(s32 *a0, s32 *a1, s32 *a2) {
    register s32 v0 asm("v0");
    __asm__ volatile (".word 0xC8800000" :: "r"(a0));  /* lwc2 $0, 0($a0) */
    __asm__ volatile (".word 0xC8810004" :: "r"(a0));  /* lwc2 $1, 4($a0) */
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");              /* mvmva 1,0,0,0,0 */
    __asm__ volatile (".word 0xE8B90000" :: "r"(a1));  /* swc2 $25, 0($a1) */
    __asm__ volatile (".word 0xE8BA0004" :: "r"(a1));  /* swc2 $26, 4($a1) */
    __asm__ volatile (".word 0xE8BB0008" :: "r"(a1));  /* swc2 $27, 8($a1) */
    __asm__ volatile (".word 0x4842F800" : "=r"(v0));  /* cfc2 $v0, $31 */
    *a2 = v0;
}
PAD_NOPS_2; /* 2 NOPs after func_8007F2AC */
void func_8007F2DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007F2DC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
PAD_NOPS_2; /* 2 NOPs after func_8007F2DC */
void *motutil_GetWalkDir(s16 *angle_ptr, void *arg1) {
    (void)angle_ptr;
    return arg1;
}
PAD_NOPS_1; /* 1 NOP after motutil_GetWalkDir */
void *func_8007F5EC(s16 *angle_ptr, void *arg1) {
    (void)angle_ptr;
    return arg1;
}
PAD_NOPS_1; /* 1 NOP after func_8007F5EC */
void *func_8007F87C(s32 angle, void *arg1) {
    (void)angle;
    return arg1;
}
PAD_NOPS_2; /* 2 NOPs after func_8007F87C */
void *func_8007FA1C(s32 angle, void *arg1) {
    (void)angle;
    return arg1;
}
PAD_NOPS_2; /* 2 NOPs after func_8007FA1C */
void *func_8007FBBC(s32 angle, void *arg1) {
    (void)angle;
    return arg1;
}
PAD_NOPS_2; /* 2 NOPs after func_8007FBBC */
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
typedef volatile u8 vu8;
typedef volatile s8 vs8;
typedef volatile u16 vu16;
typedef volatile s16 vs16;
typedef volatile u32 vu32;
typedef volatile s32 vs32;
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
extern s16 D_800A0928[];

s32 single_game_getEnemyCharId(s32 arg0, s32 arg1) {
    s32 var_v1;
    s32 var_v0;
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
        var_v1 = D_800A0928[idx];
    } else {
        if (var_a1 & 0x7FE00000) {
            idx = var_a1 / (var_a0 >> 0xA);
        } else {
            idx = (var_a1 << 0xA) / var_a0;
        }
        var_v1 = 0x400 - D_800A0928[idx];
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
    "glabel func_8007FEDC\n"
    "    lui $at, %hi(D_800A3658)\n"
    "    sw $ra, %lo(D_800A3658)($at)\n"
    "    jal EnterCriticalSection\n"
    "    nop\n"
    "    addiu $t2, $zero, 0xB0\n"
    "    jalr $t2\n"
    "    addiu $t1, $zero, 0x56\n"
    "    lui $t2, %hi(D_8007FF44)\n"
    "    lui $t1, %hi(func_8007FF7C)\n"
    "    lw $v0, 24($v0)\n"
    "    addiu $t2, $t2, %lo(D_8007FF44)\n"
    "    addiu $t1, $t1, %lo(func_8007FF7C)\n"
    ".L8007FF0C:\n"
    "    lw $v1, 0($t2)\n"
    "    addiu $t2, $t2, 0x4\n"
    "    addiu $v0, $v0, 0x4\n"
    "    bne $t2, $t1, .L8007FF0C\n"
    "    sw $v1, -4($v0)\n"
    "    jal func_80078FF0\n"
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
    "    .set\treorder\n"
    "    .set\tat\n"
    "    .set reorder\n"
    "    .set at\n"
);
PAD_NOPS_1; /* 1 NOP after func_8007FEDC */
extern s32 cdrom_CheckReady(s32);
extern s32 cdrom_SetCallbackA(s32);
extern s32 sys_SetVsyncMode(s32);
extern s32 sys_SetTimer(s32);
extern s32 D_80080014;
extern s32 D_8008003C;
extern s32 D_80080064;
extern u32 g_str_cdinit_fail;

s32 func_8007FF7C(void) {
    s32 retries = 4;
loop:
    if (cdrom_CheckReady(1) != 1) {
        retries--;
        if (retries != -1) goto loop;
        debug_printf(&g_str_cdinit_fail);
        return 0;
    }
    cdrom_SetCallbackA((s32)&D_80080014);
    cdrom_SetCallbackB((s32)&D_8008003C);
    sys_SetVsyncMode((s32)&D_80080064);
    sys_SetTimer(0);
    return 1;
}

void motion_SavePreCalcData_80080014(void) {
    func_8008008C(0xF0000003, 0x20);
}

void motion_SavePreCalcData_8008003C(void) {
    func_8008008C(0xF0000003, 0x40);
}

void motion_SavePreCalcData_80080064(void) {
    func_8008008C(0xF0000003, 0x40);
}
