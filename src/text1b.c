#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

void func_80047ED0(s32 a0) {
    g_snd_volume += a0;
}

void AddTbpOfst_80047EE8(s32 arg0, s32 arg1)
{
    u32 *p;
    s16 new_var;
    s32 count;
    unsigned int new_var2;
    count = (s32) (arg1 << 16);
    count = arg0 + (count >> 14);
    p = (u32 *) count;
    p = (u32 *) (arg0 + (((*p) >> 2) << 2));
    count = *(p++);
    if (count != 0)
    {
        count--;
        do
        {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            word = *p;
            p = (u32 *) (((s32) p) + 4);
            a1v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a2v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            a3v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            new_var = a1v;
            new_var2 = word >> 2;
            v0v = (s16) (*((u16 *) p));
            p = (u32 *) (((s32) p) + 2);
            efc_buki_draw_zanzou(arg0 + (new_var2 << 2), new_var, a2v, a3v, v0v);
        }
        while ((count--) != 0);
    }
}
void InitHiraRmd_80047FBC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    u32 *p;
    s32 count;
    s32 new_var;
    p = (u32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(arg0 + (((*p) >> 2) << 2));
    count = *(p++);
    new_var = arg0;
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 v_plus_arg3;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v_plus_arg3 = (s32)v0v + sx_arg3;
            new_var2 = word >> 2;
            efc_buki_draw_zanzou(new_var + (new_var2 << 2),
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg2,
                          v_plus_arg3);
        } while ((count--) != 0);
    }
}
void InitHiraRmd_800480C0(s32 arg0, s32 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    register s32 saved_arg0 asm("$18");
    register s32 sx_arg2 asm("$22");
    register s32 sx_arg3 asm("$21");
    register s32 sx_arg4 asm("$20");
    register s32 sx_arg5 asm("$19");
    u32 *p;
    s32 count;
    p = (u32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(arg0 + (((*p) >> 2) << 2));
    count = *(p++);
    saved_arg0 = arg0;
    if (count != 0) {
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        sx_arg4 = arg4;
        sx_arg5 = arg5;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            s16 v0v;
            s32 v_plus;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v_plus = (s32)v0v + sx_arg5;
            new_var2 = word >> 2;
            efc_buki_draw_zanzou(saved_arg0 + (new_var2 << 2),
                          (s32)a1v + sx_arg2,
                          (s32)a2v + sx_arg3,
                          (s32)a3v + sx_arg4,
                          v_plus);
        } while ((count--) != 0);
    }
}
void func_800481E8(s32 arg0, s32 arg1)
{
    u32 *p;
    s32 count;
    s32 new_var;
    p = (u32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (u32 *)(arg0 + (((*p) >> 2) << 2));
    count = *(p++);
    new_var = arg0;
    if (count != 0) {
        count--;
        do {
            u32 word;
            s16 a1v;
            s16 a2v;
            s16 a3v;
            u16 v0v;
            s32 a0_for_call;
            unsigned int new_var2;
            word = *p;
            p = (u32 *)(((s32)p) + 4);
            a1v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a2v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            a3v = (s16)(*((u16 *)p));
            p = (u32 *)(((s32)p) + 2);
            v0v = *((u16 *)p);
            new_var2 = word >> 2;
            a0_for_call = new_var + (new_var2 << 2);
            if ((s32)a3v < 0x280) {
                p = (u32 *)(((s32)p) + 2);
                v0v += 1;
            }
            efc_buki_draw_zanzou(a0_for_call,
                          (s32)a1v,
                          (s32)a2v,
                          (s32)a3v,
                          (s32)(s16)v0v);
        } while ((count--) != 0);
    }
}
void efc_buki_draw_zanzou(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, u16 arg4) {
    s16 rect[4];
    s16 buf[512];
    u8 *p_alt;
    s32 flags;
    u32 dim;
    u32 dim2;
    s32 head = arg0[0];
    arg0 += 4;
    if (head != 0x10) return;
    flags = *(s32 *)arg0 & 8;
    arg0 += 4;
    if (flags != 0) {
        p_alt = arg0;
        arg0 = p_alt + (((u32)*(u32 *)p_alt >> 2) << 2);
        p_alt += 4;
    }
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    rect[3] = dim >> 16;
    rect[2] = dim;
    gpu_LoadImage(rect, (s32 *)(arg0 + 4));
    if (flags == 0) return;
    p_alt += 4;
    rect[0] = arg3;
    rect[1] = arg4;
    dim2 = *(u32 *)p_alt;
    p_alt += 4;
    rect[3] = dim2 >> 16;
    rect[2] = dim2;
    if (func_800486FC() != 0) {
        func_8004876C((s32)p_alt, rect[2], (s32)buf);
        gpu_LoadImage(rect, (s32 *)buf);
        gpu_DrawSync(0);
        return;
    }
    gpu_LoadImage(rect, (s32 *)p_alt);
}
extern s32 func_800484A0(s32, s32, s32);

void func_800483DC(s32 arg0, s32 arg1, s16 arg2, s16 arg3)
{
    s32 *p;
    s32 count;
    s32 new_var;
    p = (s32 *)(arg0 + (((s32)(arg1 << 16)) >> 14));
    p = (s32 *)(arg0 + (*p));
    count = *(p++);
    new_var = arg0;
    if (count != 0) {
        s32 sx_arg2;
        s32 sx_arg3;
        count--;
        sx_arg2 = arg2;
        sx_arg3 = arg3;
        do {
            s32 entry_off;
            u32 dx_u;
            u32 dy_u;
            s32 dx;
            s32 dy;
            entry_off = *p;
            p = (s32 *)(((s32)p) + 8);
            dx_u = *((u16 *)p);
            p = (s32 *)(((s32)p) + 2);
            dy_u = *((u16 *)p);
            p = (s32 *)(((s32)p) + 2);
            dx = ((s32)(dx_u << 16)) >> 16;
            dy = ((s32)(dy_u << 16)) >> 16;
            func_800484A0(new_var + entry_off, dx + sx_arg2, dy + sx_arg3);
        } while ((count--) != 0);
    }
}
void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {
    s16 rect[4];
    s16 buf[512];
    u32 dim;
    if (arg0[0] != 0x10) return;
    arg0 += 4;
    if ((*(s32 *)arg0 & 8) == 0) return;
    arg0 += 4;
    arg0 += 8;
    rect[0] = arg1;
    rect[1] = arg2;
    dim = *(u32 *)arg0;
    arg0 += 4;
    rect[3] = dim >> 16;
    rect[2] = dim;
    if (func_800486FC() != 0) {
        func_8004876C((s32)arg0, rect[2], (s32)buf);
    }
    gpu_LoadImage(rect, (s32)buf);
}
extern void func_800485EC(s32, s32, s32, s32, s32, s32);
s32 func_80048530(s32 arg0, s32 arg1, u32 arg2, s32 arg3) {
    u8 *p;
    s32 count;
    s32 entry_off;
    s32 a, b, c, d;
    u32 cv, dv;
    p = (u8 *)arg0 + ((s32 *)arg0)[arg1];
    asm volatile("" : "=r"(arg3) : "0"(arg3));
    count = *(s32 *)p;
    if (arg2 >= (u32)count) return -1;
    p += 4;
    p += arg2 * 0xC;
    entry_off = *(s32 *)p;
    p = p + 4;
    a = (s32)*(u16 *)p;
    p = p + 2;
    entry_off = entry_off + arg0;
    b = (s32)*(u16 *)p;
    p = p + 2;
    cv = *(u16 *)p;
    dv = *(u16 *)(p + 2);
    asm volatile("" : "=r"(cv) : "0"(cv));
    asm volatile("" : "=r"(dv) : "0"(dv));
    c = (s32)(s16)cv;
    d = (s32)(s16)dv;
    func_800485EC(entry_off, arg3, (s16)a, (s16)b, c, d);
    return count;
}
void func_800485EC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    register s32 s1 asm("s1") = arg0;
    register s32 s0 asm("s0") = arg1;
    register s32 s2 asm("s2") = arg2;
    register s32 s3 asm("s3") = arg3;
    s32 v1;
    s32 flags;
    v1 = *((u8 *) s1);
    if (v1 != 0x10) {
        return;
    }
    s1 += 4;
    flags = *((s32 *) s1);
    s1 += 4;
    *((s16 *) (s0 + 0)) = (s16)(flags & 7);
    if (flags & 8) {
        register u32 size asm("v0");
        v1 = s1;
        size = *((u32 *) v1);
        *((u16 *) (s0 + 0xA)) = arg4;
        *((u16 *) (s0 + 0xC)) = arg5;
        s1 = v1 + ((size >> 2) << 2);
        v1 += 8;
        *((u16 *) (s0 + 0x10)) = *((u16 *) (v1 + 2));
        *((s16 *) (s0 + 0xE)) = (s16)*((s32 *)v1);
        *((s32 *) (s0 + 0x1C)) = v1 + 4;
        *((s16 *) (s0 + 0x14)) = gpu_CalcClut(*((s16 *) (s0 + 0xA)), *((s16 *) (s0 + 0xC)));
    } else {
        *((s16 *) (s0 + 0x14)) = 0;
    }
    v1 = s1 + 8;
    asm volatile("" : "=r"(v1) : "0"(v1));
    *((s16 *) (s0 + 2)) = (s16)s2;
    *((s16 *) (s0 + 4)) = (s16)s3;
    *((u16 *) (s0 + 8)) = *((u16 *) (v1 + 2));
    *((s16 *) (s0 + 6)) = (s16)*((s32 *)v1);
    *((s32 *) (s0 + 0x18)) = v1 + 4;
    *((s16 *) (s0 + 0x12)) = gpu_CalcTPage(*((s16 *) (s0 + 0)), 0, (*((s16 *) (s0 + 2))) & 0xFFC0, (*((s16 *) (s0 + 4))) & 0xFF00);
}
extern s16 g_color_mode;
s32 file_GetFlag0(void);
s16 func_800486FC(void) {
    if (file_GetFlag0()) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
    return g_color_mode;
}
extern s16 g_color_mode;
void func_80048744(s32 a0) {
    if (a0) {
        g_color_mode = 1;
    } else {
        g_color_mode = 0;
    }
}
void func_8004876C(u16 *arg0, s32 arg1, u16 *arg2) {
    s32 pad[2];
    s32 temp_a3;
    s32 temp_v1;
    s32 var_t0;
    u16 *var_t1;
    s32 temp_a1;
    var_t1 = arg0;
    var_t0 = arg1 - 1;
    if (arg1 != 0) {
        do {
            temp_a1 = *var_t1;
            var_t1 += 1;
            var_t0 -= 1;
            temp_v1 = temp_a1 << 0x10;
            {
                s32 b;
                s32 g;
                temp_a3 = (temp_a1 & 0x1F) * 0x547;
                b = (temp_v1 >> 0x1A) & 0x1F;
                g = (temp_v1 >> 0xA) & 0xF800;
                asm volatile("" : : "r"(b));
                temp_a3 = ((temp_a3 + b * 0x2B8) + g) >> 0xC;
                temp_a3 &= 0x1F;
            }
            *arg2 = (u16) ((((temp_a1 & (~0x7FFF)) + (temp_a3 << 0xA)) + (temp_a3 << 5)) + temp_a3);
            arg2 += 1;
        } while (var_t0 != (-1));
    }
}
s32 func_8004881C(s32 arg0, s32 arg1, s32 arg2) {
    s32 a0_term = arg0 * 0x547;
    s32 sum = a0_term + (arg1 << 11);
    sum += arg2 * 0x2B8;
    return sum >> 12;
}

void saTan4FireDisp_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy)
{
  u16 src_buf[256];
  u16 dst_buf[256];
  s16 rect[4];
  volatile s32 unused;
  u16 *sp;
  u16 *dp;
  s32 i;
  u32 a3_val;
  u32 v1_val;
  s32 r5;
  s32 g5;
  s32 b5;
  s32 r8;
  s32 g8;
  s32 b8;
  register s32 alpha asm("a3");
  s32 lum;
  register s32 nr asm("a0");
  register s32 ng asm("a2");
  register s32 nb asm("a1");
  gpu_DrawSync(0);
  rect[0] = (s16) sx;
  rect[1] = (s16) sy;
  rect[2] = (s16) w;
  rect[3] = 1;
  gpu_StoreImage(rect, src_buf);
  gpu_DrawSync(0);
  sp = src_buf;
  dp = dst_buf;
  i = 0;
  if (w > 0)
  {
    do
    {
      a3_val = (u32) (*sp);
      v1_val = a3_val & 0xFFFF;
      r5 = v1_val == 0;
      if (r5)
      {
        *dp = (u16) a3_val;
        sp++;
      }
      else
      {
        r5 = a3_val & 0x1F;
        do
        {
          sp++;
          g5 = (v1_val >> 5) & 0x1F;
          b5 = (v1_val >> 10) & 0x1F;
          r8 = r5 << 3;
          g8 = g5 << 3;
          b8 = b5 << 3;
          alpha = a3_val & 0x8000;
        }
        while (0);
        switch (mode)
        {
          case 0:
            nr = (r8 * mr) >> 15;
            ng = (g8 * mg) >> 15;
            nb = (b8 * mb) >> 15;
            break;

          case 1: {
            int t = (r5 << 4) + r8;  /* t = r5*16 + r5*8 = r5*24 */
            t = (t << 6) + r8;        /* t = r5*1536 + r5*8 = r5*0x608 */
            t = (t << 3) - t;         /* t = r5*0x608 * 7 = r5*0x2A38 */
            i = 1856 * (b5 * 3);
            t = t + (g5 << 14);
            g8 = t + i;
            lum = g8 >> 15;
            i = lum;
            nr = (i * mr) >> 12;
            ng = (nr * mg) >> 12;
            nb = (nr * mb) >> 12;
            break;
          }

          default:
            r8 = r8 & 0x1F;
            nr = r8;
            ng = g8;
            nb = b8;
            break;

        }

        nr &= 0x1F;
        ng &= 0x1F;
        nb &= 0x1F;
        r5 = 5;
        *dp = (u16) (((alpha | nr) | (ng << r5)) | (nb << 10));
      }
      dp++;
      i++;
    }
    while (i < w);
  }
  rect[0] = (s16) dx;
  rect[1] = (s16) dy;
  gpu_LoadImage(rect, dst_buf);
  gpu_DrawSync(0);
}
void func_80048A7C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    saTan4FireDisp_80048864(0, arg0, arg1, arg2, arg3, arg4, arg5, arg0, arg1);
}
extern s32 snd_LoadBgm(u8);
extern s32 snd_PlayBgm(s32);
extern u8 D_80099BCC;
extern s32 D_800A33E0;
extern s32 D_800A33E4;
extern s32 func_8004153C(s32);
s32 mario_getMarioVoiceData_80048AD0(s32 arg0) {
    s32 temp_v0;
    u8 sound;
    u8 *base;
    register s32 delta asm("$6");
    register u8 *p asm("$3");
    register u8 *q asm("$5");
    register s32 i asm("$4");

    temp_v0 = func_8004153C(arg0);
    if (temp_v0 == 0) return 0;
    {
        s32 idx = *(s16 *)(temp_v0 + 8);
        D_800A33E0 = arg0;
        sound = (&D_80099BCC)[idx];
    }
    if (sound == 0xFF) return 0;
    base = (u8 *)snd_LoadBgm(sound);
    p = base + ((*(u32 *)(base + 8) >> 2) << 2);
    delta = (s32)(p - base);
    q = p + 0xA;
    D_800A33E4 = (s32)p;
    i = 0;
    do {
        *(s16 *)(q - 8) = i;
        *(s16 *)(q - 6) = 9;
        *p = 0xF;
        *(s8 *)(q - 9) = 0;
        *(s16 *)q = (s16)arg0;
        q += 0x68;
        i += 1;
        p += 0x68;
    } while (i < 0x11);
    snd_PlayBgm(delta + 0x6E8);
    return 1;
}
extern s32 g_snd_play_count;
void saTan5GetTakeCutAnimType(s32 a0) {
    g_snd_play_count += a0;
}
void func_80048BA4(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80048BA4.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 g_snd_ch_data[];
extern u16 g_snd_se_bank[];
extern void InitFadePanel(void);
void func_80048F58(s32 a0, s32 a1) {
    s32 s1 = a0;
    s32 s0 = a1;
    s32 i;
    u16 *src;
    u16 *dst;
    u8 *base;
    if (s0 > 0) {
        InitFadePanel();
    }
    base = g_snd_ch_data + s0 * 308;
    *(u32 *)base = 0;
    src = (u16 *)(g_snd_se_bank + s1 * 7);
    dst = (u16 *)(base + 0x124);
    i = 0;
    do {
        *dst = *src;
        src++;
        i++;
        dst++;
    } while (i < 7);
}
extern s32 D_800A36AC;
extern s32 D_800A378C;
extern s32 D_800EF848;
extern s32 initLoadImage(void *, s16 *, s32, s32);

void func_80048FFC(s32 arg0) {
    s16 sp10, sp12;
    u16 sp14;
    s16 sp16;
    s32 *sp18;
    s32 sp20, sp28, sp30, sp38;
    s32 sp40, sp48, sp50, sp58;
    s32 sp60, sp68;
    s32 sp70;
    s32 *base = (s32 *)((arg0 * 0x134) + (s32)&D_800EF848);
    s32 *cur;
    s32 var_s7;
    s32 a1_1, v0_1;

    var_s7 = *base;
    sp18 = (s32 *)((s32)base + 0x124);
    cur = (s32 *)((s32)base + ((D_800A36AC & 1) * 0x90) + 4);

    {
        u32 v1 = *(u16 *)((s32)base + 0x124);
        u32 v1_2 = *(u16 *)((s32)base + 0x126);
        s32 a0 = (s32)(s16)v1;
        sp20 = (s16)((s32)v1 & 0xFFC0);
        sp28 = (s16)((s32)v1_2 & 0xFF00);
        a1_1 = a0;
        if (a0 < 0) a1_1 = a0 + 0x3F;
        sp30 = (s16)(a0 - ((a1_1 >> 6) << 6));
        {
            s32 v1_3 = (s32)(s16)v1_2;
            v0_1 = v1_3;
            if (v1_3 < 0) v0_1 = v1_3 + 0xFF;
            sp38 = (s16)(v1_3 - ((v0_1 >> 8) << 8));
        }
    }
    sp60 = *(s16 *)((s32)base + 0x128);
    {
        u32 v1 = *(u16 *)((s32)base + 0x12C);
        s16 t2 = *(s16 *)((s32)base + 0x12A);
        s32 a0 = (s32)(s16)v1;
        u32 a2 = *(u16 *)((s32)base + 0x12E);
        sp40 = (s16)((s32)v1 & 0xFFC0);
        sp48 = (s16)((s32)a2 & 0xFF00);
        sp68 = t2;
        a1_1 = a0;
        if (a0 < 0) a1_1 = a0 + 0x3F;
        sp50 = (s16)(a0 - ((a1_1 >> 6) << 6));
        {
            s32 v1_3 = (s32)(s16)a2;
            v0_1 = v1_3;
            if (v1_3 < 0) v0_1 = v1_3 + 0xFF;
            sp58 = (s16)(v1_3 - ((v0_1 >> 8) << 8));
        }
    }
    sp70 = 0;
    do {
        s32 t_w = sp40 + sp50;
        s32 t_h = sp48 + sp58;
        s32 t_y = sp28 + sp38;
        s16 t_diff = sp68 - var_s7;
        sp10 = sp20 + sp30;
        sp12 = (s16)t_y;
        sp14 = (u16)sp60;
        sp16 = t_diff;
        initLoadImage((void *)cur, &sp10, t_w, t_h + var_s7);
        {
            s32 prev_s7 = var_s7;
            var_s7 = var_s7 >> 1;
            sp30 = sp30 >> 1;
            sp38 = sp38 >> 1;
            sp50 = sp50 >> 1;
            sp58 = sp58 >> 1;
            sp60 = sp60 >> 1;
            sp68 = sp68 >> 1;
            sp70 += 1;
            *cur = (*cur & 0xFF000000) | (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFFFFFF);
            *(s32 *)((s32)&D_800A378C + 0x3FFC) = (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFF000000) | ((s32)cur & 0xFFFFFF);
            sp12 = (s16)(t_y + t_diff);
            sp16 = (s16)prev_s7;
            initLoadImage((void *)((s32)cur + 0x18), &sp10, t_w, t_h);
            *(s32 *)((s32)cur + 0x18) = (*(s32 *)((s32)cur + 0x18) & 0xFF000000) | (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFFFFFF);
            *(s32 *)((s32)&D_800A378C + 0x3FFC) = (*(s32 *)((s32)&D_800A378C + 0x3FFC) & 0xFF000000) | (((s32)cur + 0x18) & 0xFFFFFF);
            cur = (s32 *)((s32)cur + 0x30);
        }
    } while (sp70 < 3);
    {
        s32 v = *base + *(s16 *)((s32)sp18 + 0xC);
        s16 mod_by = *(s16 *)((s32)sp18 + 6);
        *base = v;
        if (v >= mod_by) *base = v % mod_by;
    }
}
extern s16 D_800EF9F2;
extern s16 D_800EF9F4;
extern s16 D_800A33EA;
extern s16 D_800A33E8;
extern s32 D_800A33EC;
void gnd_close_8004939C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_800EF9F2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800EF9F4 = -2;
    D_800A33EA = -1;
    D_800A33E8 = -1;
    D_800A33EC = -1;
}
extern s32 InitFadePanel();
extern u8 D_80099CC8[];
extern u8 D_80099CC9[];
extern s32 D_800A33EC;
extern s16 D_800EF980[];
void func_800493E4(s32 arg0) {
    u8 temp_v1;
    s32 idx;

    D_800EF980[arg0] = 1;
    if (D_800A33EC == -1) {
        if (arg0 >= 0x33) {
            D_800A33EC = 1;
        } else {
            D_800A33EC = 0;
        }
    } else {
        if (D_800A33EC == 0 && !(arg0 < 0x33)) {
            InitFadePanel();
        }
        if (D_800A33EC == 1 && arg0 < 0x33) {
            InitFadePanel();
        }
    }
    idx = arg0 * 2;
    temp_v1 = D_80099CC8[idx];
    if (temp_v1 != 0xFF) {
        D_800EF980[temp_v1] = 1;
        do { } while (0);
        D_800EF980[D_80099CC9[idx]] = 1;
    }
}
extern s32 D_800A33EC;
extern s16 D_800A33E8;
extern void InitFadePanel(void);
void func_800494D4(s32 arg0, s32 arg1) {
    register s32 idx asm("$17") = arg0;
    s32 new_var;
    register s32 val asm("$16") = arg1;
    s32 cond;
    if (D_800A33EC == 0) {
        cond = val < 16;
    } else {
        cond = val < 8;
    }
    if (!cond) {
        InitFadePanel();
    }
    new_var = idx;
    if (((u32)new_var) >= 2U) {
        InitFadePanel();
    }
    (&D_800A33E8)[new_var] = (s16)val;
}
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2) {
    register s32 var_a3 asm("$7") = 0;
    register s32 var_v1 asm("$3") = 0;
    s32 var_a0 = arg0;
    if (arg1 > 0) {
        do {
            var_v1 += var_a0;
            var_a3 += 1;
            var_a0 -= 1;
        } while (var_a3 < arg1);
    }
    (void)var_a3;
    return var_v1 + (arg2 - arg1);
}
extern s16 D_80099C50[];
extern s16 D_800EF980[];
extern s32 D_800A33EC;
extern s16 D_800A33E8;
extern s16 D_800A33EA;
extern s32 D_800A324C;
extern s32 func_8004954C(s32, s32, s32);
extern s32 saTanMainDispGnd_80046020();
extern void efc_rob_set_type_particle(s32, s32, s16 *, s32);
extern s32 func_8003E120();
void func_80049584(s32 arg0) {
    register s32 var_s2 asm("$18") = arg0;
    register s32 var_s1 asm("$17") = 1;
    register s32 var_s0 asm("$16") = 0;
    register s16 *var_a1 asm("$5");
    register s16 *var_a2 asm("$6");
    s16 temp_a0;
    s16 *var_v1;
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a2_2;
    s32 var_a1_2;
    register s32 var_s0_3 asm("$16");
    s32 dummy[2];
    var_a1 = D_80099C50;
    var_a2 = D_800EF980;
    do {
        temp_a0 = *var_a2;
        if ((((u32) (~temp_a0)) >> 31) != (((u32) (~(*var_a1))) >> 31)) {
            var_s1 = 0;
        }
        var_a0_2 = temp_a0;
        *var_a1 = var_a0_2;
        var_a1++;
        var_s0++;
        var_a2++;
    } while (var_s0 < 0x3A);
    var_a0 = 0;
    var_s0 = 0;
    var_v1 = D_800EF980;
    do {
        if ((*var_v1) >= 0) {
            *var_v1 = (s16) var_a0;
            var_a0++;
        }
        var_s0++;
        var_v1++;
    } while (var_s0 < 0x3A);
    var_a0_2 = 8;
    if (D_800A33EC == 0) {
        var_a0_2 = 0x10;
    }
    var_a2_2 = D_800A33E8;
    if (var_a2_2 == (-1)) {
        var_a1_2 = D_800A33EA;
        if (var_a1_2 == var_a2_2) {
            var_s0_3 = 0x24;
            if (D_800A33EC == 0) {
                var_s0_3 = 0x88;
            }
            goto end;
        }
        var_a2_2 = var_a1_2;
    } else {
        var_a1_2 = var_a2_2;
        if ((D_800A33EA != (-1)) && (var_a2_2 != D_800A33EA)) {
            if (var_a2_2 < D_800A33EA) {
                var_a2_2 = D_800A33EA;
            } else {
                var_a1_2 = D_800A33EA;
            }
        }
    }
    var_s0_3 = func_8004954C(var_a0_2, var_a1_2, var_a2_2);
end:
    if (D_800A324C != var_s0_3) {
        D_800A324C = var_s0_3;
        var_s1 = 0;
    }
    if (var_s1 == 0) {
        saTanMainDispGnd_80046020();
        efc_rob_set_type_particle(D_800A33EC, var_s0_3, D_800EF980, var_s2);
        func_8003E120();
    }
    (void) var_s2;
}
void func_80049710(void) {
}
typedef struct { s32 f0, f1, f2, f3, f4, f5, f6, f7; } _struct_copy_func49718;
extern u8 *D_800A3820;
extern u8 *D_800A38B4;
extern s16 D_800EF980[];
extern s32 (*g_anim_func_table)(s16 *, s16 *);
extern u8 *func_8004153C(s32);
extern void InitFadePanel(void);
extern void func_8007E4DC(s16 *, s16 *, s16 *);
extern void func_8007ED6C(s32, s16 *, s32 *);

void func_80049718(s32 arg0, s32 arg1, s32 *arg2, s16 *arg3) {
    int new_var2;
    s16 sp10[3];
    s16 *p_anim;
    s32 var_s5;
    s32 var_s3;
    u8 *vehicle;
    int new_var;
    u8 *obj;
    u8 *part;
    u8 *new_var3;
    u8 *ot;
    p_anim = &D_800EF980[arg0];
    do {
        var_s3 = arg1;
        if ((*p_anim) < 0) {
            InitFadePanel();
        }
    } while (0);
    obj = D_800A38B4;
    var_s5 = 0;
    obj[0] = 0;
    if (0) { }
    obj[1] = 0;
    new_var = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 8)) = 0;
    *((s32 *) (obj + 0xC)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = (s16) new_var;
    if (arg1 != 0) {
        if (arg1 == 1) {
            u8 *p_arg3 = (u8 *) arg3;
            *((u16 *) (obj + 0x10)) = *((u16 *) (p_arg3 + 0));
            *((u16 *) (obj + 0x12)) = *((u16 *) (p_arg3 + 2));
            *((u16 *) (obj + 0x14)) = *((u16 *) (p_arg3 + 4));
            g_anim_func_table((s16 *) (obj + 0x10), (s16 *) (obj + 0x18));
            *((s32 *) (obj + 0x2C)) = arg2[0];
            *((s32 *) (obj + 0x30)) = arg2[1];
            *((s32 *) (obj + 0x34)) = arg2[2];
        } else {
            var_s3 = arg1 & 0x7FFF;
            new_var2 = var_s3 & 1;
            vehicle = func_8004153C(var_s3 >> 1);
            part = vehicle + ((new_var2 * 0x68) + 0x7E4);
            *((s32 *) (part + 0x4C)) = ((*((s32 *) (part + 0x4C))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x50)) = ((*((s32 *) (part + 0x50))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            *((s32 *) (part + 0x54)) = ((*((s32 *) (part + 0x54))) * (*((s16 *) (vehicle + 0x12)))) >> 12;
            p_anim = vehicle + 0x44;
            func_8007E4DC((s16 *) p_anim, (s16 *) (part + 0x38), (s16 *) (obj + 0x18));
            sp10[0] = (s16) (*((s32 *) (part + 0x4C)));
            sp10[1] = (s16) (*((s32 *) (part + 0x50)));
            sp10[2] = (s16) (*((s32 *) (part + 0x54)));
            func_8007ED6C((*((s32 *) (part + 0xC))) + 0x18, sp10, (s32 *) (obj + 0x2C));
            *((s32 *) (obj + 0x2C)) = (*((s32 *) (obj + 0x2C))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x2C)));
            *((s32 *) (obj + 0x30)) = (*((s32 *) (obj + 0x30))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x30)));
            *((s32 *) (obj + 0x34)) = (*((s32 *) (obj + 0x34))) + (*((s32 *) ((*((u8 **) (part + 0xC))) + 0x34)));
            var_s3 = var_s3 | 0x8000;
            *((_struct_copy_func49718 *) (part + 0x18)) = *((_struct_copy_func49718 *) (obj + 0x18));
            var_s5 = *((s16 *) (vehicle + 0x1A84));
        }
        ot = D_800A3820;
        D_800A3820 = ot + 4;
        *((u8 **) ot) = obj;
        obj += 0x68;
        if (var_s3 != 1) {
            s32 anim_v = D_800EF980[arg0];
            obj[0] = 3;
            obj[1] = 0;
            *((s32 *) (obj + 0x58)) = var_s5;
            new_var3 = D_800A3820;
            ot = new_var3;
            *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
            *((s16 *) (obj + 6)) = 1;
            *((s16 *) (obj + 4)) = 6;
            *((s16 *) (obj + 8)) = 0;
            *((s16 *) (obj + 0xA)) = 0;
            *((s16 *) (obj + 2)) = (s16) ((anim_v * 2) + 1);
            D_800A3820 = ot + 4;
            *((u8 **) ot) = obj;
            obj += 0x68;
        }
        D_800A38B4 = obj;
    }
}
extern u8 D_80099CC8[];
extern s16 D_80099D3C[];
extern u8 *D_800A3820;
extern u8 *D_800A38B4;
extern s16 D_800EF980[];
extern void func_800417D0(s32 *);
void func_80049A2C(s32 arg0, s32 arg1, s32 arg2) {
    u8 *new_var6;
    u8 *new_var5;
    s16 *new_var7;
    char new_var4;
    u8 temp_v1;
    u8 *new_var8;
    s16 *p_anim;
    s16 new_var2;
    s16 *src;
    int new_var3;
    u8 *obj;
    int new_var;
    u8 *vehicle;
    s16 a1_val;
    u8 *ot;
    s32 dummy[2];

    new_var6 = D_80099CC8;
    {
        u8 *p = new_var6 + (arg0 * 2);
        temp_v1 = p[arg2];
    }
    if (temp_v1 == 0xFF) {
        return;
    }
    new_var3 = 8;
    new_var8 = (u8 *) D_800EF980;
    p_anim = (s16 *) (new_var8 + (temp_v1 * 2));
    if ((*p_anim) < 0) {
        InitFadePanel();
    }
    vehicle = (u8 *) func_8004153C(arg1 >> 1);
    obj = D_800A38B4;
    obj[0] = 0;
    obj[1] = 0;
    a1_val = (*p_anim) * 2;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + new_var3)) = 0;
    *((s16 *) (obj + 0xA)) = 4;
    *((s16 *) (obj + 2)) = a1_val;
    src = &D_80099D3C[(arg1 & 1) * 6];
    *((s32 *) (obj + 0x4C)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    if (a1_val) {
    }
    src++;
    *((s32 *) (obj + 0x50)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((s32 *) (obj + 0x54)) = ((s32) ((*src) * (*((s16 *) (vehicle + 0x12))))) >> 12;
    src++;
    *((u16 *) (obj + 0x10)) = (u16) (*src);
    src++;
    *((u16 *) (obj + 0x12)) = (u16) (*src);
    new_var2 = src[1];
    *((s32 *) (obj + 0xC)) = (s32) (vehicle + 0x50C);
    *((s16 *) (obj + 6)) = 0;
    *((u16 *) (obj + 0x14)) = (u16) new_var2;
    func_800417D0((s32 *) obj);
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    obj += 0x68;
    new_var5 = obj + 0xA;
    a1_val = (*p_anim) * 2;
    obj[0] = 3;
    *((s32 *) (obj + 0xC)) = (s32) (obj - 0x68);
    obj[1] = 0;
    new_var7 = (s16 *) (obj + 6);
    *((s16 *) (obj + (new_var = new_var3))) = 0;
    *new_var7 = 1;
    *((s16 *) new_var5) = 0;
    *((s16 *) (obj + 4)) = 6;
    *((s16 *) (obj + 2)) = (s16) (a1_val + 1);
    *((s32 *) (obj + 0x58)) = (s32) (*((s16 *) (vehicle + 0x1A84)));
    ot = D_800A3820;
    D_800A3820 = ot + 4;
    *((u8 **) ot) = obj;
    D_800A38B4 = obj + 0x68;
    (void) dummy;
    (void) new_var4;
}
s32 func_80049C24(s32 arg0, s32 arg1) {
    s32 count;
    s32 temp_v0;
    s32 temp_a2;
    s32 var_s7;
    s32 v0;
    s32 var_fp;
    s32 var_s5;
    s32 var_s6;
    s32 var_s4;
    s32 var_s0;
    s32 var_s2;
    s32 var_s1;
    s32 var_s3;
    s32 v1;
    s32 hdr;
    s32 a0_arg;

    count = *(s32 *)arg0;
    var_s3 = arg1;
    temp_v0 = *(s32 *)(arg0 + (count * 4) + 4);
    temp_a2 = *(s32 *)(arg0 + 8);
    var_s7 = arg0 + temp_v0;
    v0 = *(s32 *)(arg0 + 4);
    var_fp = arg0 + v0;
    var_s5 = temp_a2 - v0;

    if (count >= 2) {
        var_s6 = arg0 + temp_a2;
        var_s4 = *(s32 *)(arg0 + 0xC) - temp_a2;
    } else {
        var_s6 = 0;
        var_s4 = 0;
    }

    var_s0 = D_800A33E8;
    var_s2 = D_800A33EA;
    var_s1 = var_s3 + 0xC;

    if (var_s0 == -1) {
        if (var_s2 == var_s0) {
            var_s0 = var_s2;
        } else {
            var_s2 = 0;
        }
    } else if (var_s2 == -1) {
        var_s0 = 0;
    } else if (var_s0 == var_s2) {
        var_s0 = 0;
        var_s2 = 0;
    } else if (var_s0 < var_s2) {
        var_s0 = 0;
        var_s2 = 1;
    } else if (var_s2 < var_s0) {
        var_s0 = 1;
        var_s2 = 0;
    } else {
        InitFadePanel();
    }

    v1 = var_s3;
    var_s3 += 4;
    hdr = ((u32)~var_s0) >> 31;
    if (var_s2 >= 0) {
        hdr += 1;
    }
    *(s32 *)v1 = hdr;

    if (var_s0 >= 0) {
        *(s32 *)var_s3 = 2;
        var_s3 += 4;
        if (var_s0 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        saTan5TakeGetPos_80045230(a0_arg);
        var_s1 += tslGlobalMemFree_8005C2A8(var_s1, 2, var_s7);
    }

    if (var_s2 >= 0) {
        *(s32 *)var_s3 = 5;
        if (var_s2 == 0) {
            func_800520B8(var_fp, var_s1, var_s5);
            a0_arg = var_s1 + var_s5;
        } else {
            func_800520B8(var_s6, var_s1, var_s4);
            a0_arg = var_s1 + var_s4;
        }
        saTan5TakeGetPos_80045230(a0_arg);
        var_s1 += tslGlobalMemFree_8005C2A8(var_s1, 5, var_s7);
    }
    return var_s1;
}
extern s16 D_80099CC2;
extern s32 D_800A324C;
void func_80049E1C(void) {
    s16 val = -1;
    s32 i = 0x39;
    s16 *p = &D_80099CC2;
    do {
        *p = val;
        i--;
        p--;
    } while (i >= 0);
    D_800A324C = -1;
}
extern s32 func_800418D0();
extern s32 func_8004A1FC();
extern void *D_800A3708;
extern void *D_800A370C;
extern u8 D_800FF638;
extern s8 D_800FF639;
extern s16 D_800FF640;
extern s32 D_800FF644;
extern s16 D_800FF648;
extern s16 D_800FF64A;
extern s16 D_800FF64C;
extern s32 D_800FF684;
extern s32 D_800FF688;
extern s32 D_800FF68C;
extern u8 D_80101DF0;
extern s8 D_80101DF1;
extern s16 D_80101DF8;
extern s32 D_80101DFC;
extern s16 D_80101E00;
extern s16 D_80101E02;
extern s16 D_80101E04;
extern s32 D_80101E3C;
extern s32 D_80101E40;
extern s32 D_80101E44;
void func_80049E4C(void) {
    u8 *p1 = &D_80101DF0;
    u8 *p2 = &D_800FF638;
    *p1 = 0x64;
    D_80101DF1 = 0;
    D_80101E00 = 0;
    D_80101E02 = 0;
    D_80101E04 = 0;
    D_80101E3C = 0;
    D_80101E40 = 0;
    D_80101E44 = 0;
    D_80101DFC = 0;
    D_80101DF8 = 5;
    func_800418D0(p1);
    *p2 = 0x65;
    D_800FF639 = 0;
    D_800FF648 = 0;
    D_800FF64A = 0;
    D_800FF64C = 0;
    D_800FF684 = 0;
    D_800FF688 = 0;
    D_800FF68C = 0;
    D_800FF644 = 0;
    D_800FF640 = 2;
    func_800418D0(p2);
    D_800A3708 = p1;
    D_800A370C = p2;
}
extern u8 D_800153F0;
extern u8 D_800F62E0;
extern s32 D_800F6318;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern void func_8004A09C(s32, u16 *);
extern void gte_SetColorMatrix(s32 *);
extern void gte_SetBackColor(s32, s32, s32);

void func_80049F4C(void) {
    s32 sp10[12];
    s32 i;
    u8 *base;
    __builtin_memcpy(sp10, &D_800153F0, 44);
    i = 0;
    base = &D_800F62E0;
    do {
        func_8004A09C((s32)base, (u16 *)sp10);
        i++;
        base += 0x60;
    } while (i < 8);
    gte_SetColorMatrix(&D_800F6318);
    gte_SetBackColor(D_800F6338, D_800F6339, D_800F633A);
}
void func_8004A09C(s32 arg0, u16 *arg1) {
    *(s16 *)(arg0 + 0x38) = *arg1++;
    *(s16 *)(arg0 + 0x3E) = *arg1++;
    *(s16 *)(arg0 + 0x44) = *arg1++;
    *(s16 *)(arg0 + 0x3A) = *arg1++;
    *(s16 *)(arg0 + 0x40) = *arg1++;
    *(s16 *)(arg0 + 0x46) = *arg1++;
    *(s16 *)(arg0 + 0x3C) = *arg1++;
    *(s16 *)(arg0 + 0x42) = *arg1++;
    {
        s16 v48 = *arg1++;
        *(s16 *)(arg0 + 0x18) = 0;
        *(s16 *)(arg0 + 0x1A) = 0;
        *(s16 *)(arg0 + 0x1C) = 0;
        *(s16 *)(arg0 + 0x1E) = 0;
        *(s16 *)(arg0 + 0x20) = 0;
        *(s16 *)(arg0 + 0x22) = 0;
        *(s16 *)(arg0 + 0x24) = 0;
        *(s16 *)(arg0 + 0x26) = 0;
        *(s16 *)(arg0 + 0x28) = 0;
        *(s16 *)(arg0 + 0x48) = v48;
    }
    *(s16 *)(arg0 + 0x00) = *arg1++;
    *(s16 *)(arg0 + 0x02) = *arg1++;
    *(s16 *)(arg0 + 0x04) = *arg1++;
    *(s16 *)(arg0 + 0x08) = *arg1++;
    *(s16 *)(arg0 + 0x0A) = *arg1++;
    *(s16 *)(arg0 + 0x0C) = *arg1++;
    *(s16 *)(arg0 + 0x10) = *arg1++;
    *(s16 *)(arg0 + 0x12) = *arg1++;
    func_8004A1FC();
    *(s16 *)(arg0 + 0x14) = *arg1++;
    *(s8 *)(arg0 + 0x58) = *arg1++;
    *(s8 *)(arg0 + 0x59) = *arg1++;
    *(s8 *)(arg0 + 0x5A) = *arg1;
    *(s16 *)(arg0 + 0x5C) = *(arg1 + 1);
}
extern s32 math_Cos();
extern s32 math_Sin();
void func_8004A1FC(arg0) s16 *arg0; {
    s16 i;
    s16 *p;
    s16 *out;
    s16 c0;
    s32 t;

    i = 0;
    do {
        p = arg0 + (s32)i * 4;
        if (p[2] != 0) {
            c0 = math_Cos(p[0]);
            t = ((math_Sin(p[1]) * c0) >> 12) * arg0[0x2E];
            out = arg0 + (s32)i * 3 + 12;
            out[0] = -t >> 12;
            t = math_Sin(p[0]) * arg0[0x2E];
            out[1] = t >> 12;
            t = ((math_Cos(p[1]) * c0) >> 12) * arg0[0x2E];
            out[2] = -t >> 12;
        } else {
            out = arg0 + (s32)i * 3 + 12;
            out[0] = 0;
            out[1] = 0;
            out[2] = 0;
        }
        i++;
    } while (i < 3);
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A348\n"
    "    addiu  $sp, $sp, -0x38\n"
    "    sw     $ra, 52($sp)\n"
    "    sw     $fp, 48($sp)\n"
    "    sw     $s7, 44($sp)\n"
    "    sw     $s6, 40($sp)\n"
    "    sw     $s5, 36($sp)\n"
    "    sw     $s4, 32($sp)\n"
    "    sw     $s3, 28($sp)\n"
    "    sw     $s2, 24($sp)\n"
    "    sw     $s1, 20($sp)\n"
    "    sw     $s0, 16($sp)\n"
    "    lui    $v0, %hi(Judge)\n"
    "    addiu  $v0, $v0, %lo(Judge)\n"
    "    lhu    $t0, 0($a0)\n"
    "    lhu    $t1, 2($a0)\n"
    "    lhu    $t2, 4($a0)\n"
    "    andi   $t0, $t0, 0xFFF\n"
    "    andi   $t1, $t1, 0xFFF\n"
    "    andi   $t2, $t2, 0xFFF\n"
    "    add    $t0, $t0, $t0\n"
    "    add    $t1, $t1, $t1\n"
    "    add    $t2, $t2, $t2\n"
    "    add    $t3, $v0, $t0\n"
    "    add    $t4, $v0, $t1\n"
    "    add    $t5, $v0, $t2\n"
    "    lh     $s0, 0($t3)\n"
    "    lh     $s1, 0($t4)\n"
    "    lh     $s2, 0($t5)\n"
    "    addi   $t0, $t0, 0x800\n"
    "    addi   $t1, $t1, 0x800\n"
    "    addi   $t2, $t2, 0x800\n"
    "    andi   $t0, $t0, 0x1FFE\n"
    "    andi   $t1, $t1, 0x1FFE\n"
    "    andi   $t2, $t2, 0x1FFE\n"
    "    add    $t0, $v0, $t0\n"
    "    add    $t1, $v0, $t1\n"
    "    add    $t2, $v0, $t2\n"
    "    lh     $t5, 0($t2)\n"
    "    lh     $s4, 0($t1)\n"
    "    lh     $s3, 0($t0)\n"
    "    mtc2   $t5, $8\n"
    "    mtc2   $s0, $9\n"
    "    mtc2   $s3, $10\n"
    "    mtc2   $s4, $11\n"
    "    mfc2   $s0, $9\n"
    "    nop\n"
    "    gpf    1\n"
    "    mult   $s0, $s4\n"
    "    neg    $t7, $s1\n"
    "    sh     $t7, 12($a1)\n"
    "    mfc2   $t6, $9\n"
    "    mfc2   $t7, $10\n"
    "    mfc2   $t8, $11\n"
    "    mtc2   $s2, $8\n"
    "    mtc2   $s0, $9\n"
    "    mtc2   $s3, $10\n"
    "    mtc2   $s4, $11\n"
    "    sh     $t8, 0($a1)\n"
    "    mflo   $t9\n"
    "    gpf    1\n"
    "    sra    $t9, $t9, 12\n"
    "    mult   $s3, $s4\n"
    "    mtc2   $s1, $8\n"
    "    sh     $t9, 14($a1)\n"
    "    mfc2   $t8, $9\n"
    "    mfc2   $t9, $10\n"
    "    mfc2   $a2, $11\n"
    "    mfc2   $s1, $8\n"
    "    mtc2   $t7, $25\n"
    "    neg    $a3, $t6\n"
    "    mtc2   $a3, $26\n"
    "    mtc2   $t8, $27\n"
    "    mtc2   $t7, $11\n"
    "    mflo   $a3\n"
    "    nop\n"
    "    gpl    1\n"
    "    sh     $a2, 6($a1)\n"
    "    mult   $s1, $t6\n"
    "    sra    $a3, $a3, 12\n"
    "    sh     $a3, 16($a1)\n"
    "    mfc2   $t6, $9\n"
    "    mfc2   $t7, $10\n"
    "    mfc2   $t8, $11\n"
    "    sh     $t6, 8($a1)\n"
    "    sh     $t7, 10($a1)\n"
    "    mflo   $a2\n"
    "    sh     $t8, 4($a1)\n"
    "    sra    $a2, $a2, 12\n"
    "    sub    $a2, $a2, $t9\n"
    "    sh     $a2, 2($a1)\n"
    "    lw     $ra, 52($sp)\n"
    "    lw     $fp, 48($sp)\n"
    "    lw     $s7, 44($sp)\n"
    "    lw     $s6, 40($sp)\n"
    "    lw     $s5, 36($sp)\n"
    "    lw     $s4, 32($sp)\n"
    "    lw     $s3, 28($sp)\n"
    "    lw     $s2, 24($sp)\n"
    "    lw     $s1, 20($sp)\n"
    "    lw     $s0, 16($sp)\n"
    "    jr     $ra\n"
    "    addiu  $sp, $sp, 0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A4E0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,52($sp)\n"
    "    sw   $fp,48($sp)\n"
    "    sw   $s7,44($sp)\n"
    "    sw   $s6,40($sp)\n"
    "    sw   $s5,36($sp)\n"
    "    sw   $s4,32($sp)\n"
    "    sw   $s3,28($sp)\n"
    "    sw   $s2,24($sp)\n"
    "    sw   $s1,20($sp)\n"
    "    sw   $s0,16($sp)\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    lui   $s7,%hi(D_80102C00)\n"
    "    addiu   $s7,$s7,%lo(D_80102C00)\n"
    "    beq   $v0,$s7,.L8004A5D0\n"
    "    lui   $s1,0x1F80\n"
    "    addu   $s5,$zero,$zero\n"
    "    lui   $fp,%hi(D_800FF610)\n"
    "    addiu   $fp,$fp,%lo(D_800FF610)\n"
    "    lw   $s4,28($s1)\n"
    ".L8004A534:\n"
    "    lw   $s2,0($s7)\n"
    "    addu   $s6,$fp,$zero\n"
    "    lbu   $t0,0($s2)\n"
    "    nop\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $at,%hi(jtbl_8001541C)\n"
    "    addu   $at,$at,$t0\n"
    "    lw   $t0,%lo(jtbl_8001541C)($at)\n"
    "    nop\n"
    "    jr   $t0\n"
    "    addiu   $s7,$s7,0x4\n"
    ".L8004A560:\n"
    "    lh   $t0,4($s2)\n"
    "    lh   $t1,2($s2)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    nop\n"
    ".L8004A58C:\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004A5D0\n"
    "    addi   $s0,$s0,0x4\n"
    "    sll   $a1,$a1,2\n"
    "    lui   $at,%hi(jtbl_8001545C)\n"
    "    addu   $at,$at,$a1\n"
    "    lw   $a1,%lo(jtbl_8001545C)($at)\n"
    "    nop\n"
    "    jr   $a1\n"
    "    nop\n"
    ".L8004A5C0:\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $t0,.L8004A58C\n"
    "    nop\n"
    "    .global .L8004A5D0\n"
    ".L8004A5D0:\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    nop\n"
    "    bne   $s7,$v0,.L8004A534\n"
    "    nop\n"
    "    lw   $ra,52($sp)\n"
    "    lw   $fp,48($sp)\n"
    "    lw   $s7,44($sp)\n"
    "    lw   $s6,40($sp)\n"
    "    lw   $s5,36($sp)\n"
    "    lw   $s4,32($sp)\n"
    "    lw   $s3,28($sp)\n"
    "    lw   $s2,24($sp)\n"
    "    lw   $s1,20($sp)\n"
    "    lw   $s0,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    "    .global .L8004A614\n"
    ".L8004A614:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A76C\n"
    "    nop\n"
    "    jal   calc_loc_mat_fw_8004A940\n"
    "    nop\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A644\n"
    ".L8004A644:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A808\n"
    "    nop\n"
    "    lbu   $t1,1($s2)\n"
    "    lui   $s4,%hi(D_80095328)\n"
    "    addiu   $s4,$s4,%lo(D_80095328)\n"
    "    andi   $t1,$t1,0x1\n"
    "    beqz   $t1,.L8004A680\n"
    "    nop\n"
    "    addi   $s4,$s4,0xB0\n"
    ".L8004A680:\n"
    "    jal   calc_loc_mat_fw_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A694\n"
    ".L8004A694:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    lui   $t0,0x50\n"
    "    lui   $t1,0x3C\n"
    "    ctc2   $t0,$24\n"
    "    ctc2   $t1,$25\n"
    "    jal   func_8004A76C\n"
    "    nop\n"
    "    lui   $t0,0x140\n"
    "    lui   $t1,0x78\n"
    "    ctc2   $t0,$24\n"
    "    ctc2   $t1,$25\n"
    "    lui   $s4,%hi(D_80095508)\n"
    "    addiu   $s4,$s4,%lo(D_80095508)\n"
    "    jal   calc_loc_mat_fw_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A6F0\n"
    ".L8004A6F0:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A938\n"
    "    nop\n"
    "    lui   $s4,%hi(D_80095328)\n"
    "    addiu   $s4,$s4,%lo(D_80095328)\n"
    "    nop\n"
    "    jal   calc_loc_mat_fw_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    "    .global .L8004A730\n"
    ".L8004A730:\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    addiu   $a1,$s1,0x20\n"
    "    addiu   $a2,$s1,0x2B4\n"
    "    jal   func_8004A808\n"
    "    nop\n"
    "    lui   $s4,%hi(D_80095488)\n"
    "    addiu   $s4,$s4,%lo(D_80095488)\n"
    "    jal   calc_loc_mat_fw_8004A940\n"
    "    nop\n"
    "    lw   $s4,28($s1)\n"
    "    j   .L8004A5C0\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A76C\n"
    "    addiu  $a0, $a0, -1\n"
    "    lwc2   $0, 0($s0)\n"
    "    lwc2   $1, 4($s0)\n"
    "    lwc2   $2, 8($s0)\n"
    "    lwc2   $3, 12($s0)\n"
    "    lwc2   $4, 16($s0)\n"
    "    lwc2   $5, 20($s0)\n"
    "    nop\n"
    "    nop\n"
    "    .word 0x4A280030\n"
    "    j      .L8004A7B0_a76c\n"
    "    addiu  $s0, $s0, 24\n"
    ".L8004A79C_a76c:\n"
    "    .word 0x4A280030\n"
    "    addiu  $a2, $a2, 6\n"
    "    sh     $t0, -6($a2)\n"
    "    sh     $t1, -4($a2)\n"
    "    sh     $t2, -2($a2)\n"
    ".L8004A7B0_a76c:\n"
    "    swc2   $12, 0($a1)\n"
    "    swc2   $13, 4($a1)\n"
    "    swc2   $14, 8($a1)\n"
    "    mfc2   $t0, $17\n"
    "    mfc2   $t1, $18\n"
    "    mfc2   $t2, $19\n"
    "    addiu  $a1, $a1, 12\n"
    "    beqz   $a0, .L8004A7F4_a76c\n"
    "    addiu  $a0, $a0, -1\n"
    "    lwc2   $0, 0($s0)\n"
    "    lwc2   $1, 4($s0)\n"
    "    lwc2   $2, 8($s0)\n"
    "    lwc2   $3, 12($s0)\n"
    "    lwc2   $4, 16($s0)\n"
    "    lwc2   $5, 20($s0)\n"
    "    j      .L8004A79C_a76c\n"
    "    addiu  $s0, $s0, 24\n"
    ".L8004A7F4_a76c:\n"
    "    sh     $t0, 0($a2)\n"
    "    sh     $t1, 2($a2)\n"
    "    sh     $t2, 4($a2)\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004A808\n"
    "    addiu  $a0, $a0, -3\n"
    "    lhu    $t6, 0($s0)\n"
    "    lhu    $t7, 2($s0)\n"
    "    lhu    $t8, 4($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $0\n"
    "    mtc2   $t8, $1\n"
    "    lhu    $t6, 6($s0)\n"
    "    lhu    $t7, 8($s0)\n"
    "    lhu    $t8, 10($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $2\n"
    "    mtc2   $t8, $3\n"
    "    lhu    $t6, 12($s0)\n"
    "    lhu    $t7, 14($s0)\n"
    "    lhu    $t8, 16($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $4\n"
    "    mtc2   $t8, $5\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    j      .L8004A888\n"
    "    addiu  $s0, $s0, 18\n"
    ".L8004A874:\n"
    "    rtpt\n"
    "    addiu  $a2, $a2, 6\n"
    "    sh     $t0, -6($a2)\n"
    "    sh     $t1, -4($a2)\n"
    "    sh     $t2, -2($a2)\n"
    ".L8004A888:\n"
    "    swc2   $12, 0($a1)\n"
    "    swc2   $13, 4($a1)\n"
    "    swc2   $14, 8($a1)\n"
    "    mfc2   $t0, $17\n"
    "    mfc2   $t1, $18\n"
    "    mfc2   $t2, $19\n"
    "    addiu  $a1, $a1, 12\n"
    "    blez   $a0, .L8004A90C\n"
    "    nop\n"
    "    addiu  $a0, $a0, -3\n"
    "    lhu    $t6, 0($s0)\n"
    "    lhu    $t7, 2($s0)\n"
    "    lhu    $t8, 4($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $0\n"
    "    mtc2   $t8, $1\n"
    "    lhu    $t6, 6($s0)\n"
    "    lhu    $t7, 8($s0)\n"
    "    lhu    $t8, 10($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $2\n"
    "    mtc2   $t8, $3\n"
    "    lhu    $t6, 12($s0)\n"
    "    lhu    $t7, 14($s0)\n"
    "    lhu    $t8, 16($s0)\n"
    "    sll    $t7, $t7, 16\n"
    "    or     $t6, $t6, $t7\n"
    "    mtc2   $t6, $4\n"
    "    mtc2   $t8, $5\n"
    "    j      .L8004A874\n"
    "    addiu  $s0, $s0, 18\n"
    ".L8004A90C:\n"
    "    sh     $t0, 0($a2)\n"
    "    sh     $t1, 2($a2)\n"
    "    sh     $t2, 4($a2)\n"
    "    add    $a0, $a0, $a0\n"
    "    add    $t0, $a0, $a0\n"
    "    add    $a0, $t0, $a0\n"
    "    add    $s0, $s0, $a0\n"
    "    andi   $t0, $s0, 3\n"
    "    add    $s0, $s0, $t0\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
void func_8004A938(void) {
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel calc_loc_mat_fw_8004A940\n"
    "    sw  $ra,4($s1)\n"
    "    lhu  $a0,0($s0)\n"
    "    addiu  $s0,$s0,2\n"
    "    beqz  $a0,.L8004A980\n"
    ".L8004A950:\n"
    "    lhu  $t0,0($s0)\n"
    "    addiu  $s0,$s0,2\n"
    "    sll  $t0,$t0,2\n"
    "    addu  $t0,$t0,$s4\n"
    "    lw  $t0,0($t0)\n"
    "    nop\n"
    "    jalr  $t0\n"
    "    nop\n"
    "    lhu  $a0,0($s0)\n"
    "    addiu  $s0,$s0,2\n"
    "    bnez  $a0,.L8004A950\n"
    "    nop\n"
    ".L8004A980:\n"
    "    lw  $ra,4($s1)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    "jlabel .L8004A990\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    slt  $t6,$t0,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t3,$t0,$t6\n"
    "    sub  $t3,$t3,$t6\n"
    "    slt  $t6,$t1,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t4,$t1,$t6\n"
    "    sub  $t4,$t4,$t6\n"
    "    slt  $t6,$t2,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t5,$t2,$t6\n"
    "    sub  $t5,$t5,$t6\n"
    "    or  $t3,$t3,$t4\n"
    "    or  $t3,$t3,$t5\n"
    "    addiu  $s5,$zero,2\n"
    "    slti  $t4,$t3,18944\n"
    "    ori  $at,$zero,42240\n"
    "    slt  $t5,$t3,$at\n"
    "    sub  $s5,$s5,$t4\n"
    "    sub  $s5,$s5,$t5\n"
    "    nop\n"
    "    srav  $t0,$t0,$s5\n"
    "    sh  $t0,32($s1)\n"
    "    srav  $t1,$t1,$s5\n"
    "    sh  $t1,34($s1)\n"
    "    srav  $t2,$t2,$s5\n"
    "    sh  $t2,36($s1)\n"
    "    lh  $t0,10($s2)\n"
    "    lui  $t2,%hi(D_800F62E0)\n"
    "    addiu  $t2,$t2,%lo(D_800F62E0)\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    addu  $s3,$t1,$t2\n"
    "    addiu  $a0,$s3,24\n"
    "    addiu  $a1,$s2,24\n"
    "    jal  func_80052930\n"
    "    addiu  $a2,$s1,72\n"
    "    lw  $t0,72($s1)\n"
    "    lw  $t1,76($s1)\n"
    "    lw  $t2,80($s1)\n"
    "    lw  $t3,84($s1)\n"
    "    lw  $t4,88($s1)\n"
    "    ctc2  $t0,$8\n"
    "    ctc2  $t1,$9\n"
    "    ctc2  $t2,$10\n"
    "    ctc2  $t3,$11\n"
    "    ctc2  $t4,$12\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    addu  $a0,$s6,$zero\n"
    "    addiu  $a1,$s2,24\n"
    "    jal  func_80052930\n"
    "    addiu  $a2,$s1,40\n"
    "    lw  $t0,0($s6)\n"
    "    lw  $t1,4($s6)\n"
    "    lw  $t2,8($s6)\n"
    "    lw  $t3,12($s6)\n"
    "    lw  $t4,16($s6)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    lwc2  $0,32($s1)\n"
    "    lwc2  $1,36($s1)\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    mvmva  1,0,0,0,0\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    mfc2  $t0,$9\n"
    "    mfc2  $t1,$10\n"
    "    mfc2  $t2,$11\n"
    "    sra  $t1,$t1,1\n"
    "    ctc2  $t0,$5\n"
    "    ctc2  $t1,$6\n"
    "    ctc2  $t2,$7\n"
    "    lh  $t0,40($s1)\n"
    "    lh  $t1,42($s1)\n"
    "    lh  $t2,44($s1)\n"
    "    lh  $t3,46($s1)\n"
    "    lh  $t4,48($s1)\n"
    "    lh  $t5,50($s1)\n"
    "    lh  $t6,52($s1)\n"
    "    lh  $t7,54($s1)\n"
    "    lh  $t8,56($s1)\n"
    "    srav  $t0,$t0,$s5\n"
    "    srav  $t1,$t1,$s5\n"
    "    srav  $t2,$t2,$s5\n"
    "    addi  $s5,$s5,1\n"
    "    srav  $t3,$t3,$s5\n"
    "    srav  $t4,$t4,$s5\n"
    "    srav  $t5,$t5,$s5\n"
    "    addi  $s5,$s5,-1\n"
    "    srav  $t6,$t6,$s5\n"
    "    srav  $t7,$t7,$s5\n"
    "    srav  $t8,$t8,$s5\n"
    "    andi  $t0,$t0,65535\n"
    "    sll  $t1,$t1,16\n"
    "    andi  $t2,$t2,65535\n"
    "    sll  $t3,$t3,16\n"
    "    andi  $t4,$t4,65535\n"
    "    sll  $t5,$t5,16\n"
    "    andi  $t6,$t6,65535\n"
    "    sll  $t7,$t7,16\n"
    "    or  $t0,$t0,$t1\n"
    "    or  $t2,$t2,$t3\n"
    "    or  $t4,$t4,$t5\n"
    "    or  $t6,$t6,$t7\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t2,$1\n"
    "    ctc2  $t4,$2\n"
    "    ctc2  $t6,$3\n"
    "    ctc2  $t8,$4\n"
    "    j  .L8004A560\n"
    "    nop\n"
    "jlabel .L8004ABD8\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    slt  $t6,$t0,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t3,$t0,$t6\n"
    "    sub  $t3,$t3,$t6\n"
    "    slt  $t6,$t1,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t4,$t1,$t6\n"
    "    sub  $t4,$t4,$t6\n"
    "    slt  $t6,$t2,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t5,$t2,$t6\n"
    "    sub  $t5,$t5,$t6\n"
    "    or  $t3,$t3,$t4\n"
    "    or  $t3,$t3,$t5\n"
    "    addiu  $s5,$zero,3\n"
    "    slti  $t4,$t3,11520\n"
    "    slti  $t5,$t3,23168\n"
    "    ori  $at,$zero,46340\n"
    "    slt  $t6,$t3,$at\n"
    "    sub  $s5,$s5,$t6\n"
    "    sub  $s5,$s5,$t5\n"
    "    sub  $s5,$s5,$t4\n"
    "    addiu  $s5,$zero,2\n"
    "    srav  $t0,$t0,$s5\n"
    "    sh  $t0,32($s1)\n"
    "    srav  $t1,$t1,$s5\n"
    "    sh  $t1,34($s1)\n"
    "    srav  $t2,$t2,$s5\n"
    "    sh  $t2,36($s1)\n"
    "    lw  $t0,0($s6)\n"
    "    lw  $t1,4($s6)\n"
    "    lw  $t2,8($s6)\n"
    "    lw  $t3,12($s6)\n"
    "    lw  $t4,16($s6)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    lwc2  $0,32($s1)\n"
    "    lwc2  $1,36($s1)\n"
    "    lh  $t0,10($s2)\n"
    "    nop\n"
    "    mvmva  1,0,0,0,0\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    lui  $t0,%hi(D_800F62E0)\n"
    "    addiu  $t0,$t0,%lo(D_800F62E0)\n"
    "    addu  $s3,$t1,$t0\n"
    "    mfc2  $t0,$9\n"
    "    mfc2  $t1,$10\n"
    "    mfc2  $t2,$11\n"
    "    sra  $t1,$t1,1\n"
    "    ctc2  $t0,$5\n"
    "    ctc2  $t1,$6\n"
    "    ctc2  $t2,$7\n"
    "    lw  $t0,24($s3)\n"
    "    lw  $t1,28($s3)\n"
    "    lw  $t2,32($s3)\n"
    "    lw  $t3,36($s3)\n"
    "    lw  $t4,40($s3)\n"
    "    ctc2  $t0,$8\n"
    "    ctc2  $t1,$9\n"
    "    ctc2  $t2,$10\n"
    "    ctc2  $t3,$11\n"
    "    ctc2  $t4,$12\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    lh  $t0,0($s6)\n"
    "    lh  $t1,2($s6)\n"
    "    lh  $t2,4($s6)\n"
    "    lh  $t3,6($s6)\n"
    "    lh  $t4,8($s6)\n"
    "    lh  $t5,10($s6)\n"
    "    lh  $t6,12($s6)\n"
    "    lh  $t7,14($s6)\n"
    "    lh  $t8,16($s6)\n"
    "    srav  $t0,$t0,$s5\n"
    "    srav  $t1,$t1,$s5\n"
    "    srav  $t2,$t2,$s5\n"
    "    addi  $s5,$s5,1\n"
    "    srav  $t3,$t3,$s5\n"
    "    srav  $t4,$t4,$s5\n"
    "    srav  $t5,$t5,$s5\n"
    "    addi  $s5,$s5,-1\n"
    "    srav  $t6,$t6,$s5\n"
    "    srav  $t7,$t7,$s5\n"
    "    srav  $t8,$t8,$s5\n"
    "    andi  $t0,$t0,65535\n"
    "    sll  $t1,$t1,16\n"
    "    andi  $t2,$t2,65535\n"
    "    sll  $t3,$t3,16\n"
    "    andi  $t4,$t4,65535\n"
    "    sll  $t5,$t5,16\n"
    "    andi  $t6,$t6,65535\n"
    "    sll  $t7,$t7,16\n"
    "    or  $t0,$t0,$t1\n"
    "    or  $t2,$t2,$t3\n"
    "    or  $t4,$t4,$t5\n"
    "    or  $t6,$t6,$t7\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t2,$1\n"
    "    ctc2  $t4,$2\n"
    "    ctc2  $t6,$3\n"
    "    ctc2  $t8,$4\n"
    "    j  .L8004A560\n"
    "    nop\n"
    "jlabel .L8004AE0C\n"
    "    lw  $a0,12($s2)\n"
    "    addiu  $a1,$s1,40\n"
    "    lw  $a2,88($s2)\n"
    "    jal  camera_Transform\n"
    "    addiu  $a0,$a0,24\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,60($s1)\n"
    "    lw  $t1,64($s1)\n"
    "    lw  $t2,68($s1)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    sra  $t0,$t0,2\n"
    "    sra  $t1,$t1,2\n"
    "    sra  $t2,$t2,2\n"
    "    sh  $t0,32($s1)\n"
    "    sh  $t1,34($s1)\n"
    "    sh  $t2,36($s1)\n"
    "    lw  $t0,0($s6)\n"
    "    lw  $t1,4($s6)\n"
    "    lw  $t2,8($s6)\n"
    "    lw  $t3,12($s6)\n"
    "    lw  $t4,16($s6)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    lw  $t0,32($s1)\n"
    "    lw  $t1,36($s1)\n"
    "    mtc2  $t0,$0\n"
    "    mtc2  $t1,$1\n"
    "    nop\n"
    "    nop\n"
    "    mvmva  1,0,0,0,0\n"
    "    nop\n"
    "    nop\n"
    "    swc2  $9,20($s6)\n"
    "    swc2  $10,24($s6)\n"
    "    swc2  $11,28($s6)\n"
    "    addu  $a0,$s6,$zero\n"
    "    jal  calc_fc_frame_8007EC5C\n"
    "    addiu  $a1,$s1,40\n"
    "    lw  $t0,20($s6)\n"
    "    lw  $t1,24($s6)\n"
    "    lw  $t2,28($s6)\n"
    "    sra  $t1,$t1,1\n"
    "    ctc2  $t0,$5\n"
    "    ctc2  $t1,$6\n"
    "    ctc2  $t2,$7\n"
    "    lh  $t0,40($s1)\n"
    "    lh  $t1,42($s1)\n"
    "    lh  $t2,44($s1)\n"
    "    lh  $t3,46($s1)\n"
    "    lh  $t4,48($s1)\n"
    "    lh  $t5,50($s1)\n"
    "    lh  $t6,52($s1)\n"
    "    lh  $t7,54($s1)\n"
    "    lh  $t8,56($s1)\n"
    "    sra  $t0,$t0,2\n"
    "    sra  $t1,$t1,2\n"
    "    sra  $t2,$t2,2\n"
    "    sra  $t3,$t3,3\n"
    "    sra  $t4,$t4,3\n"
    "    sra  $t5,$t5,3\n"
    "    sra  $t6,$t6,2\n"
    "    sra  $t7,$t7,2\n"
    "    sra  $t8,$t8,2\n"
    "    andi  $t0,$t0,65535\n"
    "    sll  $t1,$t1,16\n"
    "    andi  $t2,$t2,65535\n"
    "    sll  $t3,$t3,16\n"
    "    andi  $t4,$t4,65535\n"
    "    sll  $t5,$t5,16\n"
    "    andi  $t6,$t6,65535\n"
    "    sll  $t7,$t7,16\n"
    "    or  $t0,$t0,$t1\n"
    "    or  $t2,$t2,$t3\n"
    "    or  $t4,$t4,$t5\n"
    "    or  $t6,$t6,$t7\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t2,$1\n"
    "    ctc2  $t4,$2\n"
    "    ctc2  $t6,$3\n"
    "    ctc2  $t8,$4\n"
    "    j  .L8004A560\n"
    "    addiu  $s5,$zero,2\n"
    "jlabel .L8004AF84\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    slt  $t6,$t0,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t3,$t0,$t6\n"
    "    sub  $t3,$t3,$t6\n"
    "    slt  $t6,$t1,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t4,$t1,$t6\n"
    "    sub  $t4,$t4,$t6\n"
    "    slt  $t6,$t2,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t5,$t2,$t6\n"
    "    sub  $t5,$t5,$t6\n"
    "    or  $t3,$t3,$t4\n"
    "    or  $t3,$t3,$t5\n"
    "    addiu  $s5,$zero,2\n"
    "    slti  $t4,$t3,23040\n"
    "    ori  $at,$zero,46336\n"
    "    slt  $t5,$t3,$at\n"
    "    sub  $s5,$s5,$t4\n"
    "    sub  $s5,$s5,$t5\n"
    "    nop\n"
    "    lh  $t0,10($s2)\n"
    "    nop\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    lui  $t0,%hi(D_800F62E0)\n"
    "    addiu  $t0,$t0,%lo(D_800F62E0)\n"
    "    addu  $s3,$t1,$t0\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    lui  $at,8064\n"
    "    sw  $s5,8($at)\n"
    "    nop\n"
    "    lh  $t0,4($s2)\n"
    "    lh  $t1,2($s2)\n"
    "    sll  $t0,$t0,2\n"
    "    lui  $t2,%hi(D_80103608)\n"
    "    addiu  $t2,$t2,%lo(D_80103608)\n"
    "    addu  $t0,$t0,$t2\n"
    "    lw  $t0,0($t0)\n"
    "    sll  $t1,$t1,2\n"
    "    addu  $t0,$t0,$t1\n"
    "    lw  $a1,0($t0)\n"
    "    lw  $a0,88($s2)\n"
    "    lh  $a2,10($s2)\n"
    "    jal  func_800430E4\n"
    "    addu  $a3,$s2,$zero\n"
    "    addu  $s0,$v0,$zero\n"
    "    j  .L8004A5D0\n"
    "    nop\n"
    "jlabel .L8004B0C8\n"
    "    addu  $a0,$s7,$zero\n"
    "    addu  $a1,$s2,$zero\n"
    "    jal  func_8004C404\n"
    "    addu  $a2,$s6,$zero\n"
    "    addu  $s7,$v0,$zero\n"
    "    j  .L8004A5D0\n"
    "    nop\n"
    "jlabel .L8004B0E4\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    slt  $t6,$t0,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t3,$t0,$t6\n"
    "    sub  $t3,$t3,$t6\n"
    "    slt  $t6,$t1,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t4,$t1,$t6\n"
    "    sub  $t4,$t4,$t6\n"
    "    slt  $t6,$t2,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t5,$t2,$t6\n"
    "    sub  $t5,$t5,$t6\n"
    "    or  $t3,$t3,$t4\n"
    "    or  $t3,$t3,$t5\n"
    "    addiu  $s5,$zero,2\n"
    "    slti  $t4,$t3,18944\n"
    "    ori  $at,$zero,42240\n"
    "    slt  $t5,$t3,$at\n"
    "    sub  $s5,$s5,$t4\n"
    "    sub  $s5,$s5,$t5\n"
    "    nop\n"
    "    lh  $t0,10($s2)\n"
    "    lui  $t2,%hi(D_800F62E0)\n"
    "    addiu  $t2,$t2,%lo(D_800F62E0)\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    addu  $s3,$t1,$t2\n"
    "    addiu  $a0,$s3,24\n"
    "    addiu  $a1,$s2,24\n"
    "    jal  func_80052930\n"
    "    addiu  $a2,$s1,72\n"
    "    lw  $t0,72($s1)\n"
    "    lw  $t1,76($s1)\n"
    "    lw  $t2,80($s1)\n"
    "    lw  $t3,84($s1)\n"
    "    lw  $t4,88($s1)\n"
    "    ctc2  $t0,$8\n"
    "    ctc2  $t1,$9\n"
    "    ctc2  $t2,$10\n"
    "    ctc2  $t3,$11\n"
    "    ctc2  $t4,$12\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    lhu  $t2,4($s6)\n"
    "    lh  $t3,6($s6)\n"
    "    lh  $t4,8($s6)\n"
    "    lh  $t5,10($s6)\n"
    "    sra  $t3,$t3,1\n"
    "    sra  $t4,$t4,1\n"
    "    sra  $t5,$t5,1\n"
    "    lw  $t0,0($s6)\n"
    "    sll  $t3,$t3,16\n"
    "    andi  $t4,$t4,65535\n"
    "    sll  $t5,$t5,16\n"
    "    or  $t2,$t2,$t3\n"
    "    or  $t4,$t4,$t5\n"
    "    lw  $t6,12($s6)\n"
    "    lw  $t8,16($s6)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t2,$1\n"
    "    ctc2  $t4,$2\n"
    "    ctc2  $t6,$3\n"
    "    ctc2  $t8,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    addu  $a1,$s5,$zero\n"
    "    jal  func_8004DA74\n"
    "    addu  $a0,$s2,$zero\n"
    "    j  .L8004A5D0\n"
    "    nop\n"
    "jlabel .L8004B27C\n"
    "    lui  $v0,%hi(D_800FF558)\n"
    "    addiu  $v0,$v0,%lo(D_800FF558)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,20($v0)\n"
    "    lw  $t4,24($v0)\n"
    "    lw  $t5,28($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    slt  $t6,$t0,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t3,$t0,$t6\n"
    "    sub  $t3,$t3,$t6\n"
    "    slt  $t6,$t1,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t4,$t1,$t6\n"
    "    sub  $t4,$t4,$t6\n"
    "    slt  $t6,$t2,$zero\n"
    "    neg  $t6,$t6\n"
    "    xor  $t5,$t2,$t6\n"
    "    sub  $t5,$t5,$t6\n"
    "    or  $t3,$t3,$t4\n"
    "    or  $t3,$t3,$t5\n"
    "    addiu  $s5,$zero,2\n"
    "    slti  $t4,$t3,18944\n"
    "    ori  $at,$zero,42240\n"
    "    slt  $t5,$t3,$at\n"
    "    sub  $s5,$s5,$t4\n"
    "    sub  $s5,$s5,$t5\n"
    "    nop\n"
    "    addiu  $s5,$zero,1\n"
    "    srav  $t0,$t0,$s5\n"
    "    sh  $t0,32($s1)\n"
    "    srav  $t1,$t1,$s5\n"
    "    sh  $t1,34($s1)\n"
    "    srav  $t2,$t2,$s5\n"
    "    sh  $t2,36($s1)\n"
    "    lh  $t0,10($s2)\n"
    "    lui  $t2,%hi(D_800F62E0)\n"
    "    addiu  $t2,$t2,%lo(D_800F62E0)\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    addu  $s3,$t1,$t2\n"
    "    addiu  $a0,$s3,24\n"
    "    addiu  $a1,$s2,24\n"
    "    jal  func_80052930\n"
    "    addiu  $a2,$s1,72\n"
    "    lw  $t0,72($s1)\n"
    "    lw  $t1,76($s1)\n"
    "    lw  $t2,80($s1)\n"
    "    lw  $t3,84($s1)\n"
    "    lw  $t4,88($s1)\n"
    "    ctc2  $t0,$8\n"
    "    ctc2  $t1,$9\n"
    "    ctc2  $t2,$10\n"
    "    ctc2  $t3,$11\n"
    "    ctc2  $t4,$12\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    lui  $a0,%hi(D_800FF558)\n"
    "    addiu  $a0,$a0,%lo(D_800FF558)\n"
    "    addiu  $a1,$s2,24\n"
    "    jal  func_80052930\n"
    "    addiu  $a2,$s1,40\n"
    "    lui  $v0,%hi(D_800FF558)\n"
    "    addiu  $v0,$v0,%lo(D_800FF558)\n"
    "    nop\n"
    "    lw  $t0,0($v0)\n"
    "    lw  $t1,4($v0)\n"
    "    lw  $t2,8($v0)\n"
    "    lw  $t3,12($v0)\n"
    "    lw  $t4,16($v0)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    lwc2  $0,32($s1)\n"
    "    lwc2  $1,36($s1)\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    mvmva  1,0,0,0,0\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    mfc2  $t0,$9\n"
    "    mfc2  $t1,$10\n"
    "    mfc2  $t2,$11\n"
    "    sra  $t1,$t1,1\n"
    "    ctc2  $t0,$5\n"
    "    ctc2  $t1,$6\n"
    "    ctc2  $t2,$7\n"
    "    lh  $t0,40($s1)\n"
    "    lh  $t1,42($s1)\n"
    "    lh  $t2,44($s1)\n"
    "    lh  $t3,46($s1)\n"
    "    lh  $t4,48($s1)\n"
    "    lh  $t5,50($s1)\n"
    "    lh  $t6,52($s1)\n"
    "    lh  $t7,54($s1)\n"
    "    lh  $t8,56($s1)\n"
    "    srav  $t0,$t0,$s5\n"
    "    srav  $t1,$t1,$s5\n"
    "    srav  $t2,$t2,$s5\n"
    "    addi  $s5,$s5,1\n"
    "    srav  $t3,$t3,$s5\n"
    "    srav  $t4,$t4,$s5\n"
    "    srav  $t5,$t5,$s5\n"
    "    addi  $s5,$s5,-1\n"
    "    srav  $t6,$t6,$s5\n"
    "    srav  $t7,$t7,$s5\n"
    "    srav  $t8,$t8,$s5\n"
    "    andi  $t0,$t0,65535\n"
    "    sll  $t1,$t1,16\n"
    "    andi  $t2,$t2,65535\n"
    "    sll  $t3,$t3,16\n"
    "    andi  $t4,$t4,65535\n"
    "    sll  $t5,$t5,16\n"
    "    andi  $t6,$t6,65535\n"
    "    sll  $t7,$t7,16\n"
    "    or  $t0,$t0,$t1\n"
    "    or  $t2,$t2,$t3\n"
    "    or  $t4,$t4,$t5\n"
    "    or  $t6,$t6,$t7\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t2,$1\n"
    "    ctc2  $t4,$2\n"
    "    ctc2  $t6,$3\n"
    "    ctc2  $t8,$4\n"
    "    j  .L8004A560\n"
    "    nop\n"
    "jlabel .L8004B4D8\n"
    "    lui  $v0,%hi(D_800A3708)\n"
    "    lw  $v0,%lo(D_800A3708)($v0)\n"
    "    lw  $t0,44($s2)\n"
    "    lw  $t1,48($s2)\n"
    "    lw  $t2,52($s2)\n"
    "    lw  $t3,44($v0)\n"
    "    lw  $t4,48($v0)\n"
    "    lw  $t5,52($v0)\n"
    "    subu  $t0,$t0,$t3\n"
    "    subu  $t1,$t1,$t4\n"
    "    subu  $t2,$t2,$t5\n"
    "    lui  $s5,%hi(g_game_p2_ctrl)\n"
    "    lh  $s5,%lo(g_game_p2_ctrl)($s5)\n"
    "    nop\n"
    "    srav  $t0,$t0,$s5\n"
    "    sh  $t0,32($s1)\n"
    "    srav  $t1,$t1,$s5\n"
    "    sh  $t1,34($s1)\n"
    "    srav  $t2,$t2,$s5\n"
    "    sh  $t2,36($s1)\n"
    "    lw  $t0,0($s6)\n"
    "    lw  $t1,4($s6)\n"
    "    lw  $t2,8($s6)\n"
    "    lw  $t3,12($s6)\n"
    "    lw  $t4,16($s6)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    ctc2  $zero,$5\n"
    "    ctc2  $zero,$6\n"
    "    ctc2  $zero,$7\n"
    "    lwc2  $0,32($s1)\n"
    "    lwc2  $1,36($s1)\n"
    "    lh  $t0,10($s2)\n"
    "    nop\n"
    "    mvmva  1,0,0,0,0\n"
    "    sll  $t1,$t0,1\n"
    "    add  $t1,$t1,$t0\n"
    "    sll  $t1,$t1,5\n"
    "    lui  $t2,%hi(D_800F62E0)\n"
    "    addiu  $t2,$t2,%lo(D_800F62E0)\n"
    "    addu  $s3,$t1,$t2\n"
    "    mfc2  $t0,$9\n"
    "    mfc2  $t1,$10\n"
    "    mfc2  $t2,$11\n"
    "    sra  $t1,$t1,1\n"
    "    ctc2  $t0,$5\n"
    "    ctc2  $t1,$6\n"
    "    ctc2  $t2,$7\n"
    "    lw  $t0,24($s3)\n"
    "    lw  $t1,28($s3)\n"
    "    lw  $t2,32($s3)\n"
    "    lw  $t3,36($s3)\n"
    "    lw  $t4,40($s3)\n"
    "    ctc2  $t0,$8\n"
    "    ctc2  $t1,$9\n"
    "    ctc2  $t2,$10\n"
    "    ctc2  $t3,$11\n"
    "    ctc2  $t4,$12\n"
    "    lw  $t0,56($s3)\n"
    "    lw  $t1,60($s3)\n"
    "    lw  $t2,64($s3)\n"
    "    lw  $t3,68($s3)\n"
    "    lw  $t4,72($s3)\n"
    "    ctc2  $t0,$16\n"
    "    ctc2  $t1,$17\n"
    "    ctc2  $t2,$18\n"
    "    ctc2  $t3,$19\n"
    "    ctc2  $t4,$20\n"
    "    lw  $a0,88($s3)\n"
    "    addiu  $t1,$zero,4080\n"
    "    srl  $a1,$a0,4\n"
    "    srl  $a2,$a0,12\n"
    "    sll  $a0,$a0,4\n"
    "    and  $a0,$a0,$t1\n"
    "    and  $a1,$a1,$t1\n"
    "    and  $a2,$a2,$t1\n"
    "    ctc2  $a0,$13\n"
    "    ctc2  $a1,$14\n"
    "    ctc2  $a2,$15\n"
    "    lh  $t0,0($s6)\n"
    "    lh  $t1,2($s6)\n"
    "    lh  $t2,4($s6)\n"
    "    lh  $t3,6($s6)\n"
    "    lh  $t4,8($s6)\n"
    "    lh  $t5,10($s6)\n"
    "    lh  $t6,12($s6)\n"
    "    lh  $t7,14($s6)\n"
    "    lh  $t8,16($s6)\n"
    "    srav  $t0,$t0,$s5\n"
    "    srav  $t1,$t1,$s5\n"
    "    srav  $t2,$t2,$s5\n"
    "    addi  $s5,$s5,1\n"
    "    srav  $t3,$t3,$s5\n"
    "    srav  $t4,$t4,$s5\n"
    "    srav  $t5,$t5,$s5\n"
    "    addi  $s5,$s5,-1\n"
    "    srav  $t6,$t6,$s5\n"
    "    srav  $t7,$t7,$s5\n"
    "    srav  $t8,$t8,$s5\n"
    "    sh  $t0,40($s1)\n"
    "    sh  $t1,42($s1)\n"
    "    sh  $t2,44($s1)\n"
    "    sh  $t3,46($s1)\n"
    "    sh  $t4,48($s1)\n"
    "    sh  $t5,50($s1)\n"
    "    sh  $t6,52($s1)\n"
    "    sh  $t7,54($s1)\n"
    "    sh  $t8,56($s1)\n"
    "    lw  $t0,40($s1)\n"
    "    lw  $t1,44($s1)\n"
    "    lw  $t2,48($s1)\n"
    "    lw  $t3,52($s1)\n"
    "    lw  $t4,56($s1)\n"
    "    ctc2  $t0,$0\n"
    "    ctc2  $t1,$1\n"
    "    ctc2  $t2,$2\n"
    "    ctc2  $t3,$3\n"
    "    ctc2  $t4,$4\n"
    "    lui  $a1,%hi(g_game_timer)\n"
    "    lw  $a1,%lo(g_game_timer)($a1)\n"
    "    nop\n"
    "    sw  $a1,448($s1)\n"
    "    jal  func_80047BE0\n"
    "    addu  $a0,$s2,$zero\n"
    "    j  .L8004A5D0\n"
    "    nop\n"
    "    j  .L8004A560\n"
    "    nop\n"
    "    lui  $t0,8064\n"
    "    sw  $ra,488($t0)\n"
    "    sw  $fp,492($t0)\n"
    "    sw  $s7,496($t0)\n"
    "    sw  $s6,500($t0)\n"
    "    sw  $s5,504($t0)\n"
    "    sw  $s4,508($t0)\n"
    "    sw  $s3,512($t0)\n"
    "    sw  $s2,516($t0)\n"
    "    sw  $s1,520($t0)\n"
    "    sw  $s0,524($t0)\n"
    "    addu  $s7,$t0,$zero\n"
    "    sh  $a3,480($s7)\n"
    "    sw  $a1,472($s7)\n"
    "    addu  $s0,$a2,$zero\n"
    "    lw  $s2,12($s7)\n"
    "    lui  $s4,%hi(D_800A38B4)\n"
    "    lw  $s4,%lo(D_800A38B4)($s4)\n"
    "    addi  $s5,$s7,32\n"
    "    addi  $s6,$s7,272\n"
    "    addi  $s3,$s7,528\n"
    "    lui  $fp,255\n"
    "    ori  $fp,$fp,65535\n"
    "    addi  $a0,$a0,-1\n"
    ".L8004B744:\n"
    "    lb  $t1,481($s7)\n"
    "    lw  $t0,0($s0)\n"
    "    andi  $t1,$t1,1\n"
    "    sll  $t1,$t1,25\n"
    "    or  $t0,$t0,$t1\n"
    "    mtc2  $t0,$6\n"
    "    lw  $v0,12($s0)\n"
    "    sw  $a0,468($s7)\n"
    "    sll  $t4,$v0,2\n"
    "    andi  $t4,$t4,1020\n"
    "    addu  $t0,$t4,$s5\n"
    "    lw  $t0,0($t0)\n"
    "    srl  $t5,$v0,6\n"
    "    andi  $t5,$t5,1020\n"
    "    addu  $t1,$t5,$s5\n"
    "    lw  $t1,0($t1)\n"
    "    srl  $t6,$v0,14\n"
    "    andi  $t6,$t6,1020\n"
    "    addu  $t2,$t6,$s5\n"
    "    lw  $t2,0($t2)\n"
    "    mtc2  $t0,$12\n"
    "    mtc2  $t1,$13\n"
    "    mtc2  $t2,$14\n"
    "    and  $v1,$t0,$t1\n"
    "    and  $v1,$v1,$t2\n"
    "    nop\n"
    "    nclip\n"
    "    bltz  $v1,.L8004BB18\n"
    "    nop\n"
    "    andi  $v1,$v1,32768\n"
    "    mfc2  $v0,$24\n"
    "    bnez  $v1,.L8004BB18\n"
    "    nop\n"
    "    bgez  $v0,.L8004BB18\n"
    "    neg  $v1,$v0\n"
    "    sra  $a0,$t0,16\n"
    "    addi  $a0,$a0,-240\n"
    "    sra  $a1,$t1,16\n"
    "    addi  $a1,$a1,-240\n"
    "    or  $a0,$a0,$a1\n"
    "    sra  $a1,$t2,16\n"
    "    addi  $a1,$a1,-240\n"
    "    or  $a0,$a0,$a1\n"
    "    andi  $a0,$a0,32768\n"
    "    beqz  $a0,.L8004BB18\n"
    "    nop\n"
    "    srl  $t4,$t4,1\n"
    "    add  $a0,$t4,$s6\n"
    "    lh  $a0,0($a0)\n"
    "    srl  $t5,$t5,1\n"
    "    add  $a1,$t5,$s6\n"
    "    lh  $a1,0($a1)\n"
    "    srl  $t6,$t6,1\n"
    "    add  $a2,$t6,$s6\n"
    "    lh  $a2,0($a2)\n"
    "    slt  $v0,$a0,$a1\n"
    "    beqz  $v0,.L8004B834\n"
    "    slt  $v0,$a0,$a2\n"
    "    addu  $a0,$a1,$zero\n"
    "    slt  $v0,$a0,$a2\n"
    ".L8004B834:\n"
    "    beqz  $v0,.L8004B840\n"
    "    nop\n"
    "    addu  $a0,$a2,$zero\n"
    ".L8004B840:\n"
    "    addu  $v0,$a0,$zero\n"
    "    addu  $a3,$v0,$zero\n"
    "    lh  $t8,454($s7)\n"
    "    lw  $a2,24($s7)\n"
    "    lw  $a1,16($s7)\n"
    "    srlv  $v0,$v0,$t8\n"
    "    srl  $t8,$v0,11\n"
    "    andi  $v0,$v0,2047\n"
    "    addi  $v0,$v0,-4096\n"
    "    srav  $v0,$v0,$t8\n"
    "    andi  $v0,$v0,4095\n"
    "    slt  $at,$a1,$v0\n"
    "    bnez  $at,.L8004BB18\n"
    "    nop\n"
    "    slt  $at,$v0,$a2\n"
    "    bnez  $at,.L8004BB18\n"
    "    nop\n"
    "    lwc2  $0,16($s0)\n"
    "    lwc2  $1,20($s0)\n"
    "    lh  $a0,4($s0)\n"
    "    lh  $a1,0($s0)\n"
    "    sh  $a0,460($s7)\n"
    "    sh  $a1,462($s7)\n"
    "    ncs\n"
    "    slti  $a0,$v1,512\n"
    "    slti  $v1,$v1,768\n"
    "    add  $v1,$a0,$v1\n"
    "    lw  $a1,448($s7)\n"
    "    nop\n"
    "    andi  $a1,$a1,3\n"
    "    beqz  $a1,.L8004B900\n"
    "    addu  $a0,$zero,$zero\n"
    "    addi  $a2,$a1,-2\n"
    "    beqz  $a2,.L8004B904\n"
    "    nop\n"
    "    lb  $a0,480($s7)\n"
    "    nop\n"
    "    addi  $a0,$a0,-2\n"
    "    neg  $a0,$a0\n"
    "    addi  $a2,$a1,-1\n"
    "    beqz  $a2,.L8004B900\n"
    "    nop\n"
    "    lb  $a0,480($s7)\n"
    "    nop\n"
    "    slt  $a1,$a0,$v1\n"
    "    neg  $a1,$a1\n"
    "    sub  $a0,$v1,$a0\n"
    "    and  $a0,$a0,$a1\n"
    ".L8004B900:\n"
    "    addu  $v1,$a0,$zero\n"
    ".L8004B904:\n"
    "    lhu  $a0,6($s0)\n"
    "    lhu  $a1,8($s0)\n"
    "    lhu  $a2,10($s0)\n"
    "    beqz  $v1,.L8004B93C\n"
    "    addiu  $t8,$zero,255\n"
    "    srlv  $t8,$t8,$v1\n"
    "    sll  $t9,$t8,8\n"
    "    or  $t8,$t8,$t9\n"
    "    srlv  $a0,$a0,$v1\n"
    "    and  $a0,$a0,$t8\n"
    "    srlv  $a1,$a1,$v1\n"
    "    and  $a1,$a1,$t8\n"
    "    srlv  $a2,$a2,$v1\n"
    "    and  $a2,$a2,$t8\n"
    ".L8004B93C:\n"
    "    sh  $a0,482($s7)\n"
    "    sh  $a1,484($s7)\n"
    "    sh  $a2,486($s7)\n"
    "    lbu  $t8,2($s0)\n"
    "    lb  $a3,480($s7)\n"
    "    srl  $t8,$t8,1\n"
    "    andi  $t8,$t8,3\n"
    "    beqz  $t8,.L8004B96C\n"
    "    addi  $t8,$t8,-1\n"
    "    sub  $a3,$a3,$t8\n"
    "    bgtz  $a3,.L8004B9E8\n"
    "    nop\n"
    ".L8004B96C:\n"
    "    lh  $t4,460($s7)\n"
    "    lh  $t5,462($s7)\n"
    "    sw  $t0,8($s4)\n"
    "    sw  $t1,16($s4)\n"
    "    sw  $t2,24($s4)\n"
    "    sll  $t4,$t4,16\n"
    "    sll  $t5,$t5,16\n"
    "    lhu  $t0,482($s7)\n"
    "    lhu  $t1,484($s7)\n"
    "    lhu  $t2,486($s7)\n"
    "    or  $t0,$t0,$t4\n"
    "    or  $t1,$t1,$t5\n"
    "    sw  $t0,12($s4)\n"
    "    sw  $t1,20($s4)\n"
    "    sh  $t2,28($s4)\n"
    "    swc2  $22,4($s4)\n"
    "    sll  $v0,$v0,2\n"
    "    addu  $v0,$v0,$s2\n"
    "    lw  $t1,0($v0)\n"
    "    and  $t0,$s4,$fp\n"
    "    sw  $t0,0($v0)\n"
    "    lui  $t2,1792\n"
    "    or  $t1,$t1,$t2\n"
    "    sw  $t1,0($s4)\n"
    "    lw  $a0,468($s7)\n"
    "    addi  $s0,$s0,24\n"
    "    addi  $s4,$s4,32\n"
    "    bnez  $a0,.L8004B744\n"
    "    addi  $a0,$a0,-1\n"
    "    j  .L8004BB28\n"
    "    nop\n"
    ".L8004B9E8:\n"
    "    lw  $a0,448($s7)\n"
    "    nop\n"
    "    andi  $a0,$a0,4\n"
    "    beqz  $a0,.L8004BA48\n"
    "    nop\n"
    "    addu  $a0,$a3,$zero\n"
    "    lw  $a1,0($s0)\n"
    "    nop\n"
    "    lui  $at,255\n"
    "    ori  $at,$at,65535\n"
    "    or  $a1,$a1,$at\n"
    "    beqz  $a0,.L8004BA3C\n"
    "    nop\n"
    "    addiu  $at,$zero,-256\n"
    "    and  $a1,$a1,$at\n"
    "    addi  $a0,$a0,-1\n"
    "    beqz  $a0,.L8004BA3C\n"
    "    nop\n"
    "    lui  $at,65535\n"
    "    ori  $at,$at,16384\n"
    "    and  $a1,$a1,$at\n"
    ".L8004BA3C:\n"
    "    mtc2  $a1,$22\n"
    "    nop\n"
    "    nop\n"
    ".L8004BA48:\n"
    "    sll  $v0,$v0,2\n"
    "    addu  $v0,$v0,$s2\n"
    "    lw  $s1,0($v0)\n"
    "    sw  $v0,476($s7)\n"
    "    lui  $t0,1792\n"
    "    or  $s1,$s1,$t0\n"
    "    lw  $a1,472($s7)\n"
    "    swc2  $22,464($s7)\n"
    "    add  $t0,$t4,$t4\n"
    "    add  $t4,$t4,$t0\n"
    "    add  $t4,$t4,$a1\n"
    "    lh  $t0,0($t4)\n"
    "    lh  $t1,2($t4)\n"
    "    lh  $t2,4($t4)\n"
    "    lh  $t3,482($s7)\n"
    "    sh  $t0,8($s3)\n"
    "    sh  $t1,10($s3)\n"
    "    sh  $t2,12($s3)\n"
    "    sh  $t3,14($s3)\n"
    "    add  $t0,$t5,$t5\n"
    "    add  $t5,$t5,$t0\n"
    "    add  $t5,$t5,$a1\n"
    "    lh  $t0,0($t5)\n"
    "    lh  $t1,2($t5)\n"
    "    lh  $t2,4($t5)\n"
    "    lh  $t3,484($s7)\n"
    "    sh  $t0,16($s3)\n"
    "    sh  $t1,18($s3)\n"
    "    sh  $t2,20($s3)\n"
    "    sh  $t3,22($s3)\n"
    "    add  $t0,$t6,$t6\n"
    "    add  $t6,$t6,$t0\n"
    "    add  $t6,$t6,$a1\n"
    "    lh  $t0,0($t6)\n"
    "    lh  $t1,2($t6)\n"
    "    lh  $t2,4($t6)\n"
    "    lh  $t3,486($s7)\n"
    "    sh  $t0,24($s3)\n"
    "    sh  $t1,26($s3)\n"
    "    sh  $t2,28($s3)\n"
    "    sh  $t3,30($s3)\n"
    "    jal  func_8004BB68\n"
    "    addu  $a0,$a3,$zero\n"
    "    lw  $t0,476($s7)\n"
    "    and  $t1,$s1,$fp\n"
    "    sw  $t1,0($t0)\n"
    "    lw  $a0,468($s7)\n"
    "    addi  $s0,$s0,24\n"
    "    bnez  $a0,.L8004B744\n"
    "    addi  $a0,$a0,-1\n"
    "    j  .L8004BB28\n"
    "    nop\n"
    ".L8004BB18:\n"
    "    lw  $a0,468($s7)\n"
    "    addi  $s0,$s0,24\n"
    "    bnez  $a0,.L8004B744\n"
    "    addi  $a0,$a0,-1\n"
    ".L8004BB28:\n"
    "    lui  $at,%hi(D_800A38B4)\n"
    "    sw  $s4,%lo(D_800A38B4)($at)\n"
    "    addu  $v0,$s0,$zero\n"
    "    addu  $t0,$s7,$zero\n"
    "    lw  $ra,488($t0)\n"
    "    lw  $fp,492($t0)\n"
    "    lw  $s7,496($t0)\n"
    "    lw  $s6,500($t0)\n"
    "    lw  $s5,504($t0)\n"
    "    lw  $s4,508($t0)\n"
    "    lw  $s3,512($t0)\n"
    "    lw  $s2,516($t0)\n"
    "    lw  $s1,520($t0)\n"
    "    lw  $s0,524($t0)\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004BB68\n"
    "    bnez   $a0,.L8004BBF0\n"
    "    nop\n"
    "    lwc2   $0,8($s3)\n"
    "    lwc2   $1,12($s3)\n"
    "    lwc2   $2,16($s3)\n"
    "    lwc2   $3,20($s3)\n"
    "    lwc2   $4,24($s3)\n"
    "    lwc2   $5,28($s3)\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    lhu   $t4,460($s7)\n"
    "    lhu   $t5,462($s7)\n"
    "    lhu   $t0,14($s3)\n"
    "    lhu   $t1,22($s3)\n"
    "    lhu   $t2,30($s3)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    swc2   $12,8($s4)\n"
    "    swc2   $13,16($s4)\n"
    "    swc2   $14,24($s4)\n"
    "    lw   $t0,464($s7)\n"
    "    sw   $s1,0($s4)\n"
    "    sw   $t0,4($s4)\n"
    "    lui   $t1,0x700\n"
    "    and   $s1,$s4,$fp\n"
    "    or   $s1,$s1,$t1\n"
    "    jr   $ra\n"
    "    addi   $s4,$s4,0x20\n"
    ".L8004BBF0:\n"
    "    addi   $s3,$s3,0x28\n"
    "    addi   $a0,$a0,-0x1\n"
    "    sw   $ra,0($s3)\n"
    "    sh   $a0,4($s3)\n"
    "    lw   $t0,-32($s3)\n"
    "    lw   $t1,-28($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x18\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    addi   $a0,$s3,-0x18\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x8\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $t0,16($s3)\n"
    "    lw   $t1,20($s3)\n"
    "    sw   $t0,32($s3)\n"
    "    sw   $t1,36($s3)\n"
    "    lw   $t0,-16($s3)\n"
    "    lw   $t1,-12($s3)\n"
    "    sw   $t0,16($s3)\n"
    "    sw   $t1,20($s3)\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $t0,32($s3)\n"
    "    lw   $t1,36($s3)\n"
    "    sw   $t0,16($s3)\n"
    "    sw   $t1,20($s3)\n"
    "    lw   $t0,-24($s3)\n"
    "    lw   $t1,-20($s3)\n"
    "    sw   $t0,24($s3)\n"
    "    sw   $t1,28($s3)\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004BB68\n"
    "    nop\n"
    "    lw   $ra,0($s3)\n"
    "    nop\n"
    "    jr   $ra\n"
    "    addi   $s3,$s3,-0x28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel saTan2LineDraw\n"
    "    lui   $t0,0x1F80\n"
    "    sw   $ra,492($t0)\n"
    "    sw   $fp,496($t0)\n"
    "    sw   $s7,500($t0)\n"
    "    sw   $s6,504($t0)\n"
    "    sw   $s5,508($t0)\n"
    "    sw   $s4,512($t0)\n"
    "    sw   $s3,516($t0)\n"
    "    sw   $s2,520($t0)\n"
    "    sw   $s1,524($t0)\n"
    "    sw   $s0,528($t0)\n"
    "    addu   $s7,$t0,$zero\n"
    "    sh   $a3,480($s7)\n"
    "    sw   $a1,472($s7)\n"
    "    addu   $s0,$a2,$zero\n"
    "    lw   $s2,12($s7)\n"
    "    lui   $s4,%hi(D_800A38B4)\n"
    "    lw   $s4,%lo(D_800A38B4)($s4)\n"
    "    addi   $s5,$s7,32\n"
    "    addi   $s6,$s7,272\n"
    "    addi   $s3,$s7,532\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    addi   $a0,$a0,-0x1\n"
    ".L8004BD20:\n"
    "    lb   $t1,481($s7)\n"
    "    lw   $t0,0($s0)\n"
    "    andi   $t1,$t1,0x1\n"
    "    sll   $t1,$t1,25\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$6\n"
    "    srl   $t0,$t0,16\n"
    "    sh   $t0,452($s7)\n"
    "    lhu   $v1,16($s0)\n"
    "    lhu   $v0,14($s0)\n"
    "    sll   $v1,$v1,16\n"
    "    or   $v0,$v0,$v1\n"
    "    sw   $a0,468($s7)\n"
    "    sll   $t4,$v0,2\n"
    "    andi   $t4,$t4,0x3FC\n"
    "    addu   $t0,$t4,$s5\n"
    "    lw   $t0,0($t0)\n"
    "    srl   $t5,$v0,6\n"
    "    andi   $t5,$t5,0x3FC\n"
    "    addu   $t1,$t5,$s5\n"
    "    lw   $t1,0($t1)\n"
    "    srl   $t6,$v0,14\n"
    "    andi   $t6,$t6,0x3FC\n"
    "    addu   $t2,$t6,$s5\n"
    "    lw   $t2,0($t2)\n"
    "    srl   $t7,$v0,22\n"
    "    andi   $t7,$t7,0x3FC\n"
    "    addu   $t3,$t7,$s5\n"
    "    lw   $t3,0($t3)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $v1,$t0,$t1\n"
    "    and   $v1,$v1,$t2\n"
    "    and   $v1,$v1,$t3\n"
    "    nclip\n"
    "    bltz   $v1,.L8004C1A4\n"
    "    nop\n"
    "    andi   $v1,$v1,0x8000\n"
    "    mfc2   $v0,$24\n"
    "    bnez   $v1,.L8004C1A4\n"
    "    nop\n"
    "    bgez   $v0,.L8004C1A4\n"
    "    neg   $v1,$v0\n"
    "    sra   $a0,$t0,16\n"
    "    addi   $a0,$a0,-0xF0\n"
    "    sra   $a1,$t1,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    sra   $a1,$t2,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    sra   $a1,$t3,16\n"
    "    addi   $a1,$a1,-0xF0\n"
    "    or   $a0,$a0,$a1\n"
    "    andi   $a0,$a0,0x8000\n"
    "    beqz   $a0,.L8004C1A4\n"
    "    nop\n"
    "    srl   $t4,$t4,1\n"
    "    add   $a0,$t4,$s6\n"
    "    lh   $a0,0($a0)\n"
    "    srl   $t5,$t5,1\n"
    "    add   $a1,$t5,$s6\n"
    "    lh   $a1,0($a1)\n"
    "    srl   $t6,$t6,1\n"
    "    add   $a2,$t6,$s6\n"
    "    lh   $a2,0($a2)\n"
    "    srl   $t7,$t7,1\n"
    "    add   $a3,$t7,$s6\n"
    "    lh   $a3,0($a3)\n"
    "    slt   $v0,$a0,$a1\n"
    "    beqz   $v0,.L8004BE4C\n"
    "    slt   $v0,$a0,$a2\n"
    "    addu   $a0,$a1,$zero\n"
    "    slt   $v0,$a0,$a2\n"
    ".L8004BE4C:\n"
    "    beqz   $v0,.L8004BE5C\n"
    "    slt   $v0,$a0,$a3\n"
    "    addu   $a0,$a2,$zero\n"
    "    slt   $v0,$a0,$a3\n"
    ".L8004BE5C:\n"
    "    beqz   $v0,.L8004BE68\n"
    "    nop\n"
    "    addu   $a0,$a3,$zero\n"
    ".L8004BE68:\n"
    "    addu   $v0,$a0,$zero\n"
    "    addu   $a3,$v0,$zero\n"
    "    lbu   $t8,2($s0)\n"
    "    nop\n"
    "    andi   $t8,$t8,0x1\n"
    "    beqz   $t8,.L8004BE88\n"
    "    nop\n"
    "    addi   $v0,$v0,0x3E8\n"
    ".L8004BE88:\n"
    "    lh   $t8,454($s7)\n"
    "    lw   $a1,16($s7)\n"
    "    lw   $a2,24($s7)\n"
    "    srlv   $v0,$v0,$t8\n"
    "    srl   $t8,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t8\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$a1,$v0\n"
    "    bnez   $at,.L8004C1A4\n"
    "    nop\n"
    "    slt   $at,$v0,$a2\n"
    "    bnez   $at,.L8004C1A4\n"
    "    nop\n"
    "    lhu   $a1,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $a1,$1\n"
    "    lh   $a0,4($s0)\n"
    "    lh   $a1,0($s0)\n"
    "    sh   $a0,460($s7)\n"
    "    sh   $a1,462($s7)\n"
    "    ncs\n"
    "    slti   $a0,$v1,0x400\n"
    "    slti   $v1,$v1,0x700\n"
    "    add   $v1,$a0,$v1\n"
    "    lw   $a1,448($s7)\n"
    "    nop\n"
    "    andi   $a1,$a1,0x3\n"
    "    beqz   $a1,.L8004BF44\n"
    "    addu   $a0,$zero,$zero\n"
    "    addi   $a2,$a1,-0x2\n"
    "    beqz   $a2,.L8004BF48\n"
    "    nop\n"
    "    lb   $a0,480($s7)\n"
    "    nop\n"
    "    addi   $a0,$a0,-0x2\n"
    "    neg   $a0,$a0\n"
    "    addi   $a2,$a1,-0x1\n"
    "    beqz   $a2,.L8004BF44\n"
    "    nop\n"
    "    lb   $a0,480($s7)\n"
    "    nop\n"
    "    slt   $a1,$a0,$v1\n"
    "    neg   $a1,$a1\n"
    "    sub   $a0,$v1,$a0\n"
    "    and   $a0,$a0,$a1\n"
    ".L8004BF44:\n"
    "    addu   $v1,$a0,$zero\n"
    ".L8004BF48:\n"
    "    lhu   $a0,6($s0)\n"
    "    lhu   $a1,8($s0)\n"
    "    lhu   $a2,10($s0)\n"
    "    lhu   $a3,12($s0)\n"
    "    beqz   $v1,.L8004BF8C\n"
    "    addiu   $t8,$zero,0xFF\n"
    "    srlv   $t8,$t8,$v1\n"
    "    sll   $t9,$t8,8\n"
    "    or   $t8,$t8,$t9\n"
    "    srlv   $a0,$a0,$v1\n"
    "    and   $a0,$a0,$t8\n"
    "    srlv   $a1,$a1,$v1\n"
    "    and   $a1,$a1,$t8\n"
    "    srlv   $a2,$a2,$v1\n"
    "    and   $a2,$a2,$t8\n"
    "    srlv   $a3,$a3,$v1\n"
    "    and   $a3,$a3,$t8\n"
    ".L8004BF8C:\n"
    "    sh   $a0,482($s7)\n"
    "    sh   $a1,484($s7)\n"
    "    sh   $a2,486($s7)\n"
    "    sh   $a3,488($s7)\n"
    "    lbu   $t8,2($s0)\n"
    "    lb   $a3,480($s7)\n"
    "    srl   $t8,$t8,1\n"
    "    andi   $t8,$t8,0x3\n"
    "    beqz   $t8,.L8004BFC0\n"
    "    addi   $t8,$t8,-0x1\n"
    "    sub   $a3,$a3,$t8\n"
    "    bgtz   $a3,.L8004C048\n"
    "    nop\n"
    ".L8004BFC0:\n"
    "    lh   $t4,460($s7)\n"
    "    lh   $t5,462($s7)\n"
    "    sw   $t0,8($s4)\n"
    "    sw   $t1,16($s4)\n"
    "    sw   $t2,24($s4)\n"
    "    sw   $t3,32($s4)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,482($s7)\n"
    "    lhu   $t1,484($s7)\n"
    "    lhu   $t2,486($s7)\n"
    "    lhu   $t3,488($s7)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    sh   $t3,36($s4)\n"
    "    swc2   $22,4($s4)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s2\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s4,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    lui   $t2,0x900\n"
    "    or   $t1,$t1,$t2\n"
    "    sw   $t1,0($s4)\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    addi   $s4,$s4,0x28\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    "    j   .L8004C1B4\n"
    "    nop\n"
    ".L8004C048:\n"
    "    lw   $a0,448($s7)\n"
    "    nop\n"
    "    andi   $a0,$a0,0x4\n"
    "    beqz   $a0,.L8004C0A8\n"
    "    nop\n"
    "    addu   $a0,$a3,$zero\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    lui   $at,0xFF\n"
    "    ori   $at,$at,65535\n"
    "    or   $a1,$a1,$at\n"
    "    beqz   $a0,.L8004C09C\n"
    "    nop\n"
    "    addiu   $at,$zero,-0x100\n"
    "    and   $a1,$a1,$at\n"
    "    addi   $a0,$a0,-0x1\n"
    "    beqz   $a0,.L8004C09C\n"
    "    nop\n"
    "    lui   $at,0xFFFF\n"
    "    ori   $at,$at,16384\n"
    "    and   $a1,$a1,$at\n"
    ".L8004C09C:\n"
    "    mtc2   $a1,$22\n"
    "    nop\n"
    "    nop\n"
    ".L8004C0A8:\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s2\n"
    "    lw   $s1,0($v0)\n"
    "    sw   $v0,476($s7)\n"
    "    lui   $t0,0x900\n"
    "    or   $s1,$s1,$t0\n"
    "    lw   $a1,472($s7)\n"
    "    swc2   $22,464($s7)\n"
    "    add   $t0,$t4,$t4\n"
    "    add   $t4,$t4,$t0\n"
    "    add   $t4,$t4,$a1\n"
    "    lh   $t0,0($t4)\n"
    "    lh   $t1,2($t4)\n"
    "    lh   $t2,4($t4)\n"
    "    lh   $t3,482($s7)\n"
    "    sh   $t0,8($s3)\n"
    "    sh   $t1,10($s3)\n"
    "    sh   $t2,12($s3)\n"
    "    sh   $t3,14($s3)\n"
    "    add   $t0,$t5,$t5\n"
    "    add   $t5,$t5,$t0\n"
    "    add   $t5,$t5,$a1\n"
    "    lh   $t0,0($t5)\n"
    "    lh   $t1,2($t5)\n"
    "    lh   $t2,4($t5)\n"
    "    lh   $t3,484($s7)\n"
    "    sh   $t0,16($s3)\n"
    "    sh   $t1,18($s3)\n"
    "    sh   $t2,20($s3)\n"
    "    sh   $t3,22($s3)\n"
    "    add   $t0,$t6,$t6\n"
    "    add   $t6,$t6,$t0\n"
    "    add   $t6,$t6,$a1\n"
    "    lh   $t0,0($t6)\n"
    "    lh   $t1,2($t6)\n"
    "    lh   $t2,4($t6)\n"
    "    lh   $t3,486($s7)\n"
    "    sh   $t0,24($s3)\n"
    "    sh   $t1,26($s3)\n"
    "    sh   $t2,28($s3)\n"
    "    sh   $t3,30($s3)\n"
    "    add   $t0,$t7,$t7\n"
    "    add   $t7,$t7,$t0\n"
    "    add   $t7,$t7,$a1\n"
    "    lh   $t0,0($t7)\n"
    "    lh   $t1,2($t7)\n"
    "    lh   $t2,4($t7)\n"
    "    lh   $t3,488($s7)\n"
    "    sh   $t0,32($s3)\n"
    "    sh   $t1,34($s3)\n"
    "    sh   $t2,36($s3)\n"
    "    sh   $t3,38($s3)\n"
    "    jal   func_8004C1F4\n"
    "    addu   $a0,$a3,$zero\n"
    "    lw   $t0,476($s7)\n"
    "    and   $t1,$s1,$fp\n"
    "    sw   $t1,0($t0)\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    "    j   .L8004C1B4\n"
    "    nop\n"
    ".L8004C1A4:\n"
    "    lw   $a0,468($s7)\n"
    "    addi   $s0,$s0,0x18\n"
    "    bnez   $a0,.L8004BD20\n"
    "    addi   $a0,$a0,-0x1\n"
    ".L8004C1B4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s4,%lo(D_800A38B4)($at)\n"
    "    addu   $v0,$s0,$zero\n"
    "    addu   $t0,$s7,$zero\n"
    "    lw   $ra,492($t0)\n"
    "    lw   $fp,496($t0)\n"
    "    lw   $s7,500($t0)\n"
    "    lw   $s6,504($t0)\n"
    "    lw   $s5,508($t0)\n"
    "    lw   $s4,512($t0)\n"
    "    lw   $s3,516($t0)\n"
    "    lw   $s2,520($t0)\n"
    "    lw   $s1,524($t0)\n"
    "    lw   $s0,528($t0)\n"
    "    jr   $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C1F4\n"
    "    bnez   $a0,.L8004C298\n"
    "    nop\n"
    "    lwc2   $0,8($s3)\n"
    "    lwc2   $1,12($s3)\n"
    "    lwc2   $2,16($s3)\n"
    "    lwc2   $3,20($s3)\n"
    "    lwc2   $4,24($s3)\n"
    "    lwc2   $5,28($s3)\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    lhu   $t4,460($s7)\n"
    "    lhu   $t5,462($s7)\n"
    "    lhu   $t0,14($s3)\n"
    "    lhu   $t1,22($s3)\n"
    "    lhu   $t2,30($s3)\n"
    "    lhu   $t3,38($s3)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s4)\n"
    "    sw   $t1,20($s4)\n"
    "    sh   $t2,28($s4)\n"
    "    sh   $t3,36($s4)\n"
    "    swc2   $12,8($s4)\n"
    "    swc2   $13,16($s4)\n"
    "    swc2   $14,24($s4)\n"
    "    lwc2   $0,32($s3)\n"
    "    lwc2   $1,36($s3)\n"
    "    lw   $t0,464($s7)\n"
    "    nop\n"
    "    rtps\n"
    "    sw   $t0,4($s4)\n"
    "    sw   $s1,0($s4)\n"
    "    swc2   $14,32($s4)\n"
    "    lui   $t1,0x900\n"
    "    and   $s1,$s4,$fp\n"
    "    or   $s1,$s1,$t1\n"
    "    jr   $ra\n"
    "    addi   $s4,$s4,0x28\n"
    ".L8004C298:\n"
    "    addi   $s3,$s3,0x28\n"
    "    addi   $a0,$a0,-0x1\n"
    "    sw   $ra,0($s3)\n"
    "    sh   $a0,4($s3)\n"
    "    lw   $t0,-32($s3)\n"
    "    lw   $t1,-28($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x18\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x20\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-24($s3)\n"
    "    lw   $t1,-20($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x18\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-8($s3)\n"
    "    lw   $t1,-4($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x10\n"
    "    addi   $a1,$s3,-0x8\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x10\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $t0,-16($s3)\n"
    "    lw   $t1,-12($s3)\n"
    "    sw   $t0,8($s3)\n"
    "    sw   $t1,12($s3)\n"
    "    addi   $a0,$s3,-0x20\n"
    "    addi   $a1,$s3,-0x10\n"
    "    jal   func_8004C388\n"
    "    addi   $a2,$s3,0x18\n"
    "    lh   $a0,4($s3)\n"
    "    jal   func_8004C1F4\n"
    "    nop\n"
    "    lw   $ra,0($s3)\n"
    "    nop\n"
    "    jr   $ra\n"
    "    addi   $s3,$s3,-0x28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C388\n"
    "    lh  $t0,0($a0)\n"
    "    lh  $t1,2($a0)\n"
    "    lh  $t2,4($a0)\n"
    "    lh  $t3,0($a1)\n"
    "    lh  $t4,2($a1)\n"
    "    lh  $t5,4($a1)\n"
    "    add  $t0,$t0,$t3\n"
    "    add  $t1,$t1,$t4\n"
    "    add  $t2,$t2,$t5\n"
    "    sra  $t0,$t0,1\n"
    "    sra  $t1,$t1,1\n"
    "    sra  $t2,$t2,1\n"
    "    sh  $t0,0($a2)\n"
    "    sh  $t1,2($a2)\n"
    "    sh  $t2,4($a2)\n"
    "    lhu  $t3,6($a0)\n"
    "    lhu  $t4,6($a1)\n"
    "    andi  $t0,$t3,65280\n"
    "    andi  $t1,$t4,65280\n"
    "    andi  $t3,$t3,255\n"
    "    andi  $t4,$t4,255\n"
    "    add  $t3,$t3,$t4\n"
    "    add  $t0,$t0,$t1\n"
    "    srl  $t3,$t3,1\n"
    "    srl  $t0,$t0,1\n"
    "    andi  $t3,$t3,255\n"
    "    andi  $t0,$t0,65280\n"
    "    or  $t3,$t3,$t0\n"
    "    jr  $ra\n"
    "    sh  $t3,6($a2)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_8004C388 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C404\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    addu   $s2,$a0,$zero\n"
    "    addu   $s1,$a1,$zero\n"
    "    lui   $s3,0x1F80\n"
    "    lui   $s4,%hi(g_game_p2_ctrl)\n"
    "    lh   $s4,%lo(g_game_p2_ctrl)($s4)\n"
    "    addiu   $t0,$zero,0x3\n"
    "    sub   $t0,$t0,$s4\n"
    "    sh   $t0,454($s3)\n"
    "    lui   $t1,%hi(D_80103624)\n"
    "    lw   $t1,%lo(D_80103624)($t1)\n"
    "    nop\n"
    "    sw   $t1,456($s3)\n"
    "    lui   $a0,%hi(g_game_timer)\n"
    "    lw   $a0,%lo(g_game_timer)($a0)\n"
    "    nop\n"
    "    sw   $a0,448($s3)\n"
    "    lw   $t0,0($a2)\n"
    "    lw   $t1,4($a2)\n"
    "    lw   $t2,8($a2)\n"
    "    lw   $t3,12($a2)\n"
    "    lh   $t4,16($a2)\n"
    "    sw   $t0,392($s3)\n"
    "    sw   $t1,396($s3)\n"
    "    sw   $t2,400($s3)\n"
    "    sw   $t3,404($s3)\n"
    "    sh   $t4,408($s3)\n"
    "    addi   $s4,$s4,0x10\n"
    "    srav   $t0,$t0,$s4\n"
    "    srav   $t1,$t1,$s4\n"
    "    srav   $t2,$t2,$s4\n"
    "    srav   $t3,$t3,$s4\n"
    "    addi   $s4,$s4,-0x10\n"
    "    lh   $t5,392($s3)\n"
    "    lh   $t6,396($s3)\n"
    "    lh   $t7,400($s3)\n"
    "    lh   $t8,404($s3)\n"
    "    srav   $t4,$t4,$s4\n"
    "    srav   $t5,$t5,$s4\n"
    "    srav   $t6,$t6,$s4\n"
    "    srav   $t7,$t7,$s4\n"
    "    srav   $t8,$t8,$s4\n"
    "    sra   $t1,$t1,1\n"
    "    sra   $t7,$t7,1\n"
    "    sra   $t2,$t2,1\n"
    "    sh   $t5,412($s3)\n"
    "    sh   $t0,414($s3)\n"
    "    sh   $t6,416($s3)\n"
    "    sh   $t1,418($s3)\n"
    "    sh   $t7,420($s3)\n"
    "    sh   $t2,422($s3)\n"
    "    sh   $t8,424($s3)\n"
    "    sh   $t3,426($s3)\n"
    "    sh   $t4,428($s3)\n"
    "    lui   $t0,%hi(D_800A3708)\n"
    "    lw   $t0,%lo(D_800A3708)($t0)\n"
    "    nop\n"
    "    lw   $t1,44($t0)\n"
    "    lw   $t2,48($t0)\n"
    "    lw   $t3,52($t0)\n"
    "    sw   $t1,436($s3)\n"
    "    sw   $t2,440($s3)\n"
    "    sw   $t3,444($s3)\n"
    "    addiu   $t0,$zero,0x4\n"
    "    lui   $t2,%hi(D_800F62E0)\n"
    "    addiu   $t2,$t2,%lo(D_800F62E0)\n"
    "    sll   $t1,$t0,1\n"
    "    add   $t1,$t1,$t0\n"
    "    sll   $t1,$t1,5\n"
    "    addu   $t0,$t1,$t2\n"
    "    lw   $t1,24($t0)\n"
    "    lw   $t2,28($t0)\n"
    "    lw   $t3,32($t0)\n"
    "    lw   $t4,36($t0)\n"
    "    lw   $t5,40($t0)\n"
    "    ctc2   $t1,$8\n"
    "    ctc2   $t2,$9\n"
    "    ctc2   $t3,$10\n"
    "    ctc2   $t4,$11\n"
    "    ctc2   $t5,$12\n"
    "    lw   $t6,88($t0)\n"
    "    lw   $t1,56($t0)\n"
    "    lw   $t2,60($t0)\n"
    "    lw   $t3,64($t0)\n"
    "    lw   $t4,68($t0)\n"
    "    lw   $t5,72($t0)\n"
    "    ctc2   $t1,$16\n"
    "    ctc2   $t2,$17\n"
    "    ctc2   $t3,$18\n"
    "    ctc2   $t4,$19\n"
    "    ctc2   $t5,$20\n"
    "    addiu   $t0,$zero,0xFF0\n"
    "    sll   $t1,$t6,4\n"
    "    srl   $t2,$t6,4\n"
    "    srl   $t3,$t6,12\n"
    "    and   $t1,$t1,$t0\n"
    "    and   $t2,$t2,$t0\n"
    "    and   $t3,$t3,$t0\n"
    "    ctc2   $t1,$13\n"
    "    ctc2   $t2,$14\n"
    "    ctc2   $t3,$15\n"
    ".L8004C5BC:\n"
    "    lw   $t0,436($s3)\n"
    "    lw   $t1,440($s3)\n"
    "    lw   $t2,444($s3)\n"
    "    lw   $t3,8($s1)\n"
    "    addu   $t4,$zero,$zero\n"
    "    lw   $t5,12($s1)\n"
    "    sub   $t3,$t3,$t0\n"
    "    sub   $t4,$t4,$t1\n"
    "    sub   $t5,$t5,$t2\n"
    "    srav   $t3,$t3,$s4\n"
    "    srav   $t4,$t4,$s4\n"
    "    srav   $t5,$t5,$s4\n"
    "    andi   $t3,$t3,0xFFFF\n"
    "    sll   $t4,$t4,16\n"
    "    or   $t3,$t3,$t4\n"
    "    mtc2   $t3,$0\n"
    "    mtc2   $t5,$1\n"
    "    lw   $t0,392($s3)\n"
    "    lw   $t1,396($s3)\n"
    "    lw   $t2,400($s3)\n"
    "    lw   $t3,404($s3)\n"
    "    lw   $t4,408($s3)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $t0,$9\n"
    "    mfc2   $t1,$10\n"
    "    mfc2   $t2,$11\n"
    "    sra   $t1,$t1,1\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    lw   $t0,412($s3)\n"
    "    lw   $t1,416($s3)\n"
    "    lw   $t2,420($s3)\n"
    "    lw   $t3,424($s3)\n"
    "    lw   $t4,428($s3)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    lh   $t0,2($s1)\n"
    "    lh   $t1,4($s1)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    lh   $s7,6($s1)\n"
    "    nop\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004C930\n"
    "    addi   $s0,$s0,0x4\n"
    "    sw   $s0,432($s3)\n"
    "    addiu   $t0,$zero,0x4\n"
    "    bne   $t0,$a0,.L8004C7A8\n"
    "    nop\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    lhu   $t6,18($s0)\n"
    "    lhu   $t7,20($s0)\n"
    "    rtpt\n"
    "    lhu   $t8,22($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    addi   $s0,$s0,0x18\n"
    "    addiu   $s5,$s3,%lo(D_1F800020)\n"
    "    addiu   $s6,$s3,%lo(D_1F800110)\n"
    "    swc2   $12,0($s5)\n"
    "    swc2   $13,4($s5)\n"
    "    swc2   $14,8($s5)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    sh   $t0,0($s6)\n"
    "    sh   $t1,2($s6)\n"
    "    sh   $t2,4($s6)\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,12($s5)\n"
    "    sh   $t0,6($s6)\n"
    "    bgez   $zero,.L8004C8D0\n"
    "    nop\n"
    ".L8004C7A8:\n"
    "    addiu   $a0,$a0,-0x3\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    addiu   $s5,$s3,%lo(D_1F800020)\n"
    "    addiu   $s6,$s3,%lo(D_1F800110)\n"
    "    rtpt\n"
    "    j   .L8004C828\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004C814:\n"
    "    rtpt\n"
    "    addiu   $s6,$s6,0x6\n"
    "    sh   $t0,-6($s6)\n"
    "    sh   $t1,-4($s6)\n"
    "    sh   $t2,-2($s6)\n"
    ".L8004C828:\n"
    "    swc2   $12,0($s5)\n"
    "    swc2   $13,4($s5)\n"
    "    swc2   $14,8($s5)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    addiu   $s5,$s5,0xC\n"
    "    blez   $a0,.L8004C8AC\n"
    "    nop\n"
    "    addiu   $a0,$a0,-0x3\n"
    "    lhu   $t6,0($s0)\n"
    "    lhu   $t7,2($s0)\n"
    "    lhu   $t8,4($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$0\n"
    "    mtc2   $t8,$1\n"
    "    lhu   $t6,6($s0)\n"
    "    lhu   $t7,8($s0)\n"
    "    lhu   $t8,10($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$2\n"
    "    mtc2   $t8,$3\n"
    "    lhu   $t6,12($s0)\n"
    "    lhu   $t7,14($s0)\n"
    "    lhu   $t8,16($s0)\n"
    "    sll   $t7,$t7,16\n"
    "    or   $t6,$t6,$t7\n"
    "    mtc2   $t6,$4\n"
    "    mtc2   $t8,$5\n"
    "    j   .L8004C814\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004C8AC:\n"
    "    sh   $t0,0($s6)\n"
    "    sh   $t1,2($s6)\n"
    "    sh   $t2,4($s6)\n"
    "    add   $a0,$a0,$a0\n"
    "    add   $t0,$a0,$a0\n"
    "    add   $a0,$t0,$a0\n"
    "    add   $s0,$s0,$a0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    ".L8004C8D0:\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    ".L8004C8D8:\n"
    "    lhu   $t0,0($s0)\n"
    "    lw   $a1,432($s3)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    srl   $t0,$t0,3\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $at,%hi(D_80015470)\n"
    "    addu   $at,$at,$t0\n"
    "    lw   $t0,%lo(D_80015470)($at)\n"
    "    addu   $a2,$s0,$zero\n"
    "    jalr   $t0\n"
    "    addu   $a3,$s7,$zero\n"
    "    addu   $s0,$v0,$zero\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $a0,.L8004C8D8\n"
    "    nop\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    beqz   $t0,.L8004C930\n"
    "    nop\n"
    "    j   InitFadePanel\n"
    "    nop\n"
    ".L8004C930:\n"
    "    lui   $v0,%hi(D_800A3820)\n"
    "    lw   $v0,%lo(D_800A3820)($v0)\n"
    "    nop\n"
    "    beq   $v0,$s2,.L8004C964\n"
    "    nop\n"
    "    lw   $s1,0($s2)\n"
    "    nop\n"
    "    lh   $v0,0($s1)\n"
    "    nop\n"
    "    addiu   $at,$zero,0xC\n"
    "    beq   $v0,$at,.L8004C5BC\n"
    "    addi   $s2,$s2,0x4\n"
    "    addi   $s2,$s2,-0x4\n"
    ".L8004C964:\n"
    "    addu   $v0,$s2,$zero\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004C994\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CB5C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004C9F4:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004CB50\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004CB50\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004CB50\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CB50\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$0\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$1\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004CB50:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004C9F4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CB5C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CB8C\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CD80\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004CBEC:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004CD74\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004CD74\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004CD74\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CD74\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$1\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$0\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004CD74:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004CBEC\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CD80:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CDB0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004CFB0\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004CE14:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004CFA4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004CFA4\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004CFA4\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004CFA4\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,24($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    xor   $t2,$t2,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,28($s0)\n"
    "    lw   $t1,32($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004CFA4:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004CE14\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004CFB0:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004CFE0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D214\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D044:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004D208\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004D208\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004D208\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004D208\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t2,20($s0)\n"
    "    lw   $t1,24($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$0\n"
    "    mtc2   $t1,$1\n"
    "    addiu   $at,$zero,-0x50\n"
    "    xor   $t0,$a3,$at\n"
    "    add   $v0,$v0,$t0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lw   $t0,28($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,32($s0)\n"
    "    lw   $t1,36($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,40($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004D208:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004D044\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D214:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D244\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D3F4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D2A8:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004D3E8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004D3E8\n"
    "    nop\n"
    "    bnez   $t6,.L8004D3E8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004D3E8\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004D3E8\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004D3E8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004D2A8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D3F4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D424\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D604\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004D484:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004D5F8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004D5F8\n"
    "    nop\n"
    "    bnez   $t8,.L8004D5F8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004D5F8\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004D5F8\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004D5F8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004D484\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D604:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D634\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004D808\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D698:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004D7FC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004D7FC\n"
    "    nop\n"
    "    bnez   $t6,.L8004D7FC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004D7FC\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004D7FC\n"
    "    nop\n"
    "    lwc2   $0,16($s0)\n"
    "    lw   $t0,20($s0)\n"
    "    lwc2   $2,24($s0)\n"
    "    mtc2   $t0,$1\n"
    "    lwc2   $4,28($s0)\n"
    "    lwc2   $5,32($s0)\n"
    "    srl   $t0,$t0,16\n"
    "    mtc2   $t0,$3\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004D7FC:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004D698\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004D808:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004D838\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004DA44\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004D89C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004DA38\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004DA38\n"
    "    nop\n"
    "    bnez   $t8,.L8004DA38\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004DA38\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004DA38\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t9,24($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t9,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lwc2   $0,28($s0)\n"
    "    srl   $t9,$t9,16\n"
    "    mtc2   $t9,$1\n"
    "    lwc2   $2,32($s0)\n"
    "    lw   $t9,36($s0)\n"
    "    lwc2   $4,40($s0)\n"
    "    mtc2   $t9,$3\n"
    "    srl   $t8,$t9,16\n"
    "    mtc2   $t8,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004DA38:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004D89C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004DA44:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004DA74\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    addu   $s1,$a0,$zero\n"
    "    addu   $s5,$a1,$zero\n"
    "    lh   $t0,4($s1)\n"
    "    lh   $t1,2($s1)\n"
    "    sll   $t0,$t0,2\n"
    "    lui   $t2,%hi(D_80103608)\n"
    "    addiu   $t2,$t2,%lo(D_80103608)\n"
    "    addu   $t0,$t0,$t2\n"
    "    lw   $t0,0($t0)\n"
    "    sll   $t1,$t1,2\n"
    "    addu   $t0,$t0,$t1\n"
    "    lw   $s0,0($t0)\n"
    "    nop\n"
    "    lw   $a1,0($s0)\n"
    "    nop\n"
    "    andi   $a0,$a1,0xFFFF\n"
    "    srl   $a1,$a1,16\n"
    "    beqz   $a0,.L8004DD84\n"
    "    addi   $s0,$s0,0x4\n"
    "    add   $t0,$a0,$a0\n"
    "    add   $t0,$t0,$a0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s0,$s0,$t0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    "    lw   $s2,96($s1)\n"
    "    lui   $v0,%hi(D_800A3708)\n"
    "    lw   $v0,%lo(D_800A3708)($v0)\n"
    "    addiu   $a0,$a0,-0x1\n"
    "    lw   $t3,44($v0)\n"
    "    lw   $t4,48($v0)\n"
    "    lw   $t5,52($v0)\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t2,$1\n"
    "    lw   $t0,64($s2)\n"
    "    lw   $t1,68($s2)\n"
    "    lw   $t2,72($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$2\n"
    "    mtc2   $t2,$3\n"
    "    lw   $t0,128($s2)\n"
    "    lw   $t1,132($s2)\n"
    "    lw   $t2,136($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t2,$5\n"
    "    lui   $s3,0x1F80\n"
    "    ori   $s3,$s3,32\n"
    "    lui   $s4,0x1F80\n"
    "    ori   $s4,$s4,692\n"
    "    rtpt\n"
    "    j   .L8004DC18\n"
    "    addi   $s2,$s2,0xC0\n"
    ".L8004DC04:\n"
    "    rtpt\n"
    "    addiu   $s4,$s4,0x6\n"
    "    sh   $t6,-6($s4)\n"
    "    sh   $t7,-4($s4)\n"
    "    sh   $t8,-2($s4)\n"
    ".L8004DC18:\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    mfc2   $t6,$17\n"
    "    mfc2   $t7,$18\n"
    "    mfc2   $t8,$19\n"
    "    addiu   $s3,$s3,0xC\n"
    "    beqz   $a0,.L8004DD10\n"
    "    addiu   $a0,$a0,-0x1\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t2,$1\n"
    "    lw   $t0,64($s2)\n"
    "    lw   $t1,68($s2)\n"
    "    lw   $t2,72($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$2\n"
    "    mtc2   $t2,$3\n"
    "    lw   $t0,128($s2)\n"
    "    lw   $t1,132($s2)\n"
    "    lw   $t2,136($s2)\n"
    "    sra   $t0,$t0,7\n"
    "    sra   $t1,$t1,7\n"
    "    sra   $t2,$t2,7\n"
    "    sub   $t0,$t0,$t3\n"
    "    sub   $t1,$t1,$t4\n"
    "    sub   $t2,$t2,$t5\n"
    "    srav   $t0,$t0,$s5\n"
    "    srav   $t1,$t1,$s5\n"
    "    srav   $t2,$t2,$s5\n"
    "    sll   $t1,$t1,16\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    or   $t0,$t0,$t1\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t2,$5\n"
    "    j   .L8004DC04\n"
    "    addiu   $s2,$s2,0xC0\n"
    ".L8004DD10:\n"
    "    sh   $t6,0($s4)\n"
    "    sh   $t7,2($s4)\n"
    "    sh   $t8,4($s4)\n"
    "    addiu   $t0,$zero,0x3\n"
    "    subu   $t0,$t0,$s5\n"
    "    lui   $at,0x1F80\n"
    "    sw   $t0,8($at)\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    ".L8004DD34:\n"
    "    lhu   $t0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    srl   $t0,$t0,3\n"
    "    sll   $t0,$t0,2\n"
    "    lbu   $t1,1($s1)\n"
    "    lui   $t2,%hi(D_80015480)\n"
    "    addiu   $t2,$t2,%lo(D_80015480)\n"
    "    add   $t0,$t0,$t2\n"
    "    andi   $t1,$t1,0x1\n"
    "    beqz   $t1,.L8004DD64\n"
    "    nop\n"
    "    addi   $t0,$t0,0x10\n"
    ".L8004DD64:\n"
    "    lw   $t0,0($t0)\n"
    "    addu   $a1,$s0,$zero\n"
    "    jalr   $t0\n"
    "    nop\n"
    "    lhu   $a0,0($s0)\n"
    "    addiu   $s0,$s0,0x2\n"
    "    bnez   $a0,.L8004DD34\n"
    "    nop\n"
    ".L8004DD84:\n"
    "    addu   $v0,$s1,$zero\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004DDB4\n"
    "    addiu   $sp,$sp,-0x8C\n"
    "    sw   $ra,136($sp)\n"
    "    sw   $fp,132($sp)\n"
    "    sw   $s7,128($sp)\n"
    "    sw   $s6,124($sp)\n"
    "    sw   $s5,120($sp)\n"
    "    sw   $s4,116($sp)\n"
    "    sw   $s3,112($sp)\n"
    "    sw   $s2,108($sp)\n"
    "    sw   $s1,104($sp)\n"
    "    sw   $s0,100($sp)\n"
    "    addu   $fp,$a0,$zero\n"
    "    addu   $s0,$a1,$zero\n"
    "    addu   $s7,$a2,$zero\n"
    "    addu   $s1,$a3,$zero\n"
    "    addiu   $s5,$zero,0xC\n"
    "    lui   $s6,0x1F80\n"
    "    ori   $s6,$s6,32\n"
    "    addu   $s3,$s6,$zero\n"
    "    addi   $s4,$s6,0x224\n"
    "    addi   $a0,$s0,0x2\n"
    "    jal   func_80054440\n"
    "    addu   $a1,$fp,$zero\n"
    "    addi   $s0,$v0,-0x2\n"
    ".L8004DE14:\n"
    "    lui   $a0,%hi(D_800A3708)\n"
    "    lw   $a0,%lo(D_800A3708)($a0)\n"
    "    lw   $s2,0($fp)\n"
    "    lw   $t0,44($a0)\n"
    "    lw   $t1,48($a0)\n"
    "    lw   $t2,52($a0)\n"
    "    lw   $t3,20($s2)\n"
    "    lw   $t4,24($s2)\n"
    "    lw   $t5,28($s2)\n"
    "    sub   $t3,$t3,$t0\n"
    "    sub   $t4,$t4,$t1\n"
    "    sub   $t5,$t5,$t2\n"
    "    srav   $t3,$t3,$s1\n"
    "    srav   $t4,$t4,$s1\n"
    "    srav   $t5,$t5,$s1\n"
    "    sw   $t3,872($s6)\n"
    "    sw   $t4,876($s6)\n"
    "    sw   $t5,880($s6)\n"
    "    addi   $v0,$s6,0x340\n"
    "    lh   $t0,0($s2)\n"
    "    lh   $t1,2($s2)\n"
    "    lh   $t2,4($s2)\n"
    "    lh   $t3,6($s2)\n"
    "    lh   $t4,8($s2)\n"
    "    lh   $t5,10($s2)\n"
    "    lh   $t6,12($s2)\n"
    "    lh   $t7,14($s2)\n"
    "    lh   $t8,16($s2)\n"
    "    srav   $t0,$t0,$s1\n"
    "    srav   $t1,$t1,$s1\n"
    "    srav   $t2,$t2,$s1\n"
    "    srav   $t3,$t3,$s1\n"
    "    srav   $t4,$t4,$s1\n"
    "    srav   $t5,$t5,$s1\n"
    "    srav   $t6,$t6,$s1\n"
    "    srav   $t7,$t7,$s1\n"
    "    srav   $t8,$t8,$s1\n"
    "    sh   $t0,0($v0)\n"
    "    sh   $t1,2($v0)\n"
    "    sh   $t2,4($v0)\n"
    "    sh   $t3,6($v0)\n"
    "    sh   $t4,8($v0)\n"
    "    sh   $t5,10($v0)\n"
    "    sh   $t6,12($v0)\n"
    "    sh   $t7,14($v0)\n"
    "    sh   $t8,16($v0)\n"
    "    addu   $s2,$v0,$zero\n"
    "    addi   $fp,$fp,0x4\n"
    "    addu   $a0,$s7,$zero\n"
    "    addu   $a1,$s2,$zero\n"
    "    jal   func_80052930\n"
    "    addi   $a2,$s6,0x354\n"
    "    lw   $t0,0($s7)\n"
    "    lw   $t1,4($s7)\n"
    "    lw   $t2,8($s7)\n"
    "    lw   $t3,12($s7)\n"
    "    lw   $t4,16($s7)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    lw   $t0,872($s6)\n"
    "    lw   $t1,876($s6)\n"
    "    lw   $t2,880($s6)\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    nop\n"
    "    nop\n"
    "    swc2   $25,884($s6)\n"
    "    swc2   $26,888($s6)\n"
    "    swc2   $27,892($s6)\n"
    "    nop\n"
    "    addi   $s0,$s0,0x2\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E01C\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    "    lw   $t0,0($s2)\n"
    "    lw   $t1,4($s2)\n"
    "    lw   $t2,8($s2)\n"
    "    lw   $t3,12($s2)\n"
    "    lw   $t4,16($s2)\n"
    "    lw   $t5,872($s6)\n"
    "    lw   $t6,876($s6)\n"
    "    lw   $t7,880($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $t5,$5\n"
    "    ctc2   $t6,$6\n"
    "    ctc2   $t7,$7\n"
    ".L8004DFB4:\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lh   $t0,6($s0)\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    lh   $t1,8($s0)\n"
    "    lh   $t2,10($s0)\n"
    "    addi   $s0,$s0,0xC\n"
    "    addi   $s3,$s3,0x4\n"
    "    addi   $s4,$s4,0x2\n"
    "    mfc2   $t3,$25\n"
    "    mfc2   $t4,$26\n"
    "    mfc2   $t5,$27\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    sh   $t0,-4($s3)\n"
    "    sh   $t1,-2($s3)\n"
    "    sh   $t2,-2($s4)\n"
    "    bnez   $v1,.L8004DFB4\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E01C:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E140\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E030:\n"
    "    lhu   $t0,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    lh   $t2,6($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lh   $t0,0($s0)\n"
    "    lh   $t3,8($s0)\n"
    "    lh   $t4,10($s0)\n"
    "    lh   $t5,12($s0)\n"
    "    add   $t0,$t0,$t0\n"
    "    addi   $t9,$s6,0x224\n"
    "    add   $t9,$t9,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t8,$s6,$t0\n"
    "    lh   $t0,0($t8)\n"
    "    lh   $t1,2($t8)\n"
    "    lh   $t2,0($t9)\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    lw   $t3,0($s2)\n"
    "    lw   $t4,4($s2)\n"
    "    lw   $t5,8($s2)\n"
    "    lw   $t6,12($s2)\n"
    "    lw   $t7,16($s2)\n"
    "    ctc2   $t3,$0\n"
    "    ctc2   $t4,$1\n"
    "    ctc2   $t5,$2\n"
    "    ctc2   $t6,$3\n"
    "    ctc2   $t7,$4\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    lw   $t0,0($s7)\n"
    "    lw   $t1,4($s7)\n"
    "    lw   $t2,8($s7)\n"
    "    lw   $t3,12($s7)\n"
    "    lw   $t4,16($s7)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    andi   $t0,$t0,0xFFFF\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    addi   $s0,$s0,0xE\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,0($t8)\n"
    "    sh   $t0,0($t9)\n"
    "    bnez   $v1,.L8004E030\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E140:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E1DC\n"
    "    addi   $s0,$s0,0x2\n"
    "    addi   $v1,$v1,-0x1\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $zero,$5\n"
    "    ctc2   $zero,$6\n"
    "    ctc2   $zero,$7\n"
    ".L8004E188:\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    nop\n"
    "    nop\n"
    "    mvmva   1,0,0,0,0\n"
    "    addi   $s0,$s0,0x6\n"
    "    addi   $s3,$s3,0x4\n"
    "    addi   $s4,$s4,0x2\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    sh   $t0,-4($s3)\n"
    "    sh   $t1,-2($s3)\n"
    "    sh   $t2,-2($s4)\n"
    "    bnez   $v1,.L8004E188\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E1DC:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E2A0\n"
    "    addi   $s0,$s0,0x2\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    lw   $t3,884($s6)\n"
    "    lw   $t4,888($s6)\n"
    "    lw   $t5,892($s6)\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E224:\n"
    "    lhu   $t0,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    lh   $t2,6($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    lh   $t0,0($s0)\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    add   $t0,$t0,$t0\n"
    "    addi   $t9,$s6,0x224\n"
    "    add   $t9,$t9,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t8,$s6,$t0\n"
    "    lh   $t0,0($t8)\n"
    "    lh   $t1,2($t8)\n"
    "    lh   $t2,0($t9)\n"
    "    add   $t0,$t0,$t3\n"
    "    add   $t1,$t1,$t4\n"
    "    add   $t2,$t2,$t5\n"
    "    ctc2   $t0,$5\n"
    "    ctc2   $t1,$6\n"
    "    ctc2   $t2,$7\n"
    "    nop\n"
    "    nop\n"
    "    rtps\n"
    "    addi   $s0,$s0,0x8\n"
    "    mfc2   $t0,$19\n"
    "    swc2   $14,0($t8)\n"
    "    sh   $t0,0($t9)\n"
    "    bnez   $v1,.L8004E224\n"
    "    addi   $v1,$v1,-0x1\n"
    ".L8004E2A0:\n"
    "    lh   $v1,0($s0)\n"
    "    nop\n"
    "    beqz   $v1,.L8004E428\n"
    "    addi   $s0,$s0,0x2\n"
    "    lw   $t0,852($s6)\n"
    "    lw   $t1,856($s6)\n"
    "    lw   $t2,860($s6)\n"
    "    lw   $t3,864($s6)\n"
    "    lw   $t4,868($s6)\n"
    "    lw   $t5,884($s6)\n"
    "    lw   $t6,888($s6)\n"
    "    lw   $t7,892($s6)\n"
    "    ctc2   $t0,$0\n"
    "    ctc2   $t1,$1\n"
    "    ctc2   $t2,$2\n"
    "    ctc2   $t3,$3\n"
    "    ctc2   $t4,$4\n"
    "    ctc2   $t5,$5\n"
    "    ctc2   $t6,$6\n"
    "    ctc2   $t7,$7\n"
    "    addiu   $v1,$v1,-0x3\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$0\n"
    "    mtc2   $t2,$1\n"
    "    lhu   $t0,6($s0)\n"
    "    lh   $t1,8($s0)\n"
    "    lh   $t2,10($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$2\n"
    "    mtc2   $t2,$3\n"
    "    lhu   $t0,12($s0)\n"
    "    lh   $t1,14($s0)\n"
    "    lh   $t2,16($s0)\n"
    "    sll   $t1,$t1,16\n"
    "    or   $t1,$t1,$t0\n"
    "    mtc2   $t1,$4\n"
    "    mtc2   $t2,$5\n"
    "    nop\n"
    "    nop\n"
    "    rtpt\n"
    "    j   .L8004E370\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004E35C:\n"
    "    rtpt\n"
    "    sh   $t0,0($s4)\n"
    "    sh   $t1,2($s4)\n"
    "    sh   $t2,4($s4)\n"
    "    addiu   $s4,$s4,0x6\n"
    ".L8004E370:\n"
    "    blez   $v1,.L8004E3F4\n"
    "    nop\n"
    "    addiu   $v1,$v1,-0x3\n"
    "    lhu   $t0,0($s0)\n"
    "    lh   $t2,2($s0)\n"
    "    lh   $t1,4($s0)\n"
    "    sll   $t2,$t2,16\n"
    "    or   $t0,$t0,$t2\n"
    "    lhu   $t2,6($s0)\n"
    "    lh   $t4,8($s0)\n"
    "    lh   $t3,10($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    or   $t2,$t2,$t4\n"
    "    lhu   $t4,12($s0)\n"
    "    lh   $t6,14($s0)\n"
    "    lh   $t5,16($s0)\n"
    "    sll   $t6,$t6,16\n"
    "    or   $t4,$t4,$t6\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    addiu   $s3,$s3,0xC\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t3,$3\n"
    "    mtc2   $t4,$4\n"
    "    mtc2   $t5,$5\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    j   .L8004E35C\n"
    "    addiu   $s0,$s0,0x12\n"
    ".L8004E3F4:\n"
    "    swc2   $12,0($s3)\n"
    "    swc2   $13,4($s3)\n"
    "    swc2   $14,8($s3)\n"
    "    mfc2   $t0,$17\n"
    "    mfc2   $t1,$18\n"
    "    mfc2   $t2,$19\n"
    "    sh   $t0,0($s4)\n"
    "    sh   $t1,2($s4)\n"
    "    sh   $t2,4($s4)\n"
    "    add   $v1,$v1,$v1\n"
    "    add   $t0,$v1,$v1\n"
    "    add   $v1,$t0,$v1\n"
    "    add   $s0,$s0,$v1\n"
    ".L8004E428:\n"
    "    addi   $s0,$s0,0x2\n"
    "    addiu   $t1,$zero,0x4\n"
    "    andi   $t0,$s0,0x3\n"
    "    sub   $t0,$t1,$t0\n"
    "    andi   $t0,$s0,0x3\n"
    "    add   $s0,$s0,$t0\n"
    "    addi   $s4,$s6,-0x20\n"
    "    lw   $s3,28($s4)\n"
    ".L8004E448:\n"
    "    lh   $a0,0($s0)\n"
    "    addi   $s0,$s0,0x2\n"
    "    beqz   $a0,.L8004E480\n"
    "    lhu   $t0,0($s0)\n"
    "    addi   $s0,$s0,0x2\n"
    "    sll   $t0,$t0,2\n"
    "    add   $t0,$t0,$s3\n"
    "    lw   $t0,0($t0)\n"
    "    nop\n"
    "    jalr   $t0\n"
    "    nop\n"
    "    nop\n"
    "    j   .L8004E448\n"
    "    nop\n"
    ".L8004E480:\n"
    "    beqz   $s5,.L8004E52C\n"
    "    ori   $s2,$zero,0xFFFF\n"
    ".L8004E488:\n"
    "    lhu   $t0,0($s0)\n"
    "    lhu   $t1,2($s0)\n"
    "    beq   $t0,$s2,.L8004E500\n"
    "    addu   $t4,$s6,$zero\n"
    "    addi   $t6,$s6,0x224\n"
    "    lh   $t3,4($s0)\n"
    "    addu   $t5,$t4,$zero\n"
    "    addu   $t7,$t6,$zero\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t1,$t1,$t1\n"
    "    add   $t6,$t6,$t0\n"
    "    add   $t7,$t7,$t1\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $t1,$t1,$t1\n"
    "    add   $t4,$t4,$t0\n"
    "    add   $t5,$t5,$t1\n"
    "    addi   $s0,$s0,0x6\n"
    "    addi   $t3,$t3,-0x1\n"
    ".L8004E4D0:\n"
    "    lw   $t0,0($t4)\n"
    "    lh   $t1,0($t6)\n"
    "    sw   $t0,0($t5)\n"
    "    sh   $t1,0($t7)\n"
    "    addi   $t4,$t4,-0x4\n"
    "    addi   $t5,$t5,0x4\n"
    "    addi   $t6,$t6,-0x2\n"
    "    addi   $t7,$t7,0x2\n"
    "    bnez   $t3,.L8004E4D0\n"
    "    addi   $t3,$t3,-0x1\n"
    "    j   .L8004E488\n"
    "    nop\n"
    ".L8004E500:\n"
    "    addi   $s0,$s0,0x2\n"
    "    lh   $t0,0($s0)\n"
    "    addu   $s3,$s6,$zero\n"
    "    addi   $s4,$s6,0x224\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s4,$s4,$t0\n"
    "    add   $t0,$t0,$t0\n"
    "    add   $s3,$s3,$t0\n"
    "    addi   $s0,$s0,0x2\n"
    "    bgez   $zero,.L8004DE14\n"
    "    addi   $s5,$s5,-0x1\n"
    ".L8004E52C:\n"
    "    addu   $v0,$s0,$zero\n"
    "    lw   $ra,136($sp)\n"
    "    lw   $fp,132($sp)\n"
    "    lw   $s7,128($sp)\n"
    "    lw   $s6,124($sp)\n"
    "    lw   $s5,120($sp)\n"
    "    lw   $s4,116($sp)\n"
    "    lw   $s3,112($sp)\n"
    "    lw   $s2,108($sp)\n"
    "    lw   $s1,104($sp)\n"
    "    lw   $s0,100($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x8C\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_8004DDB4 */
void func_8004E564(void) {
}
void func_8004E56C(void) {
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004E574\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004E7B0\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004E5D8:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,8($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004E7A4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004E7A4\n"
    "    nop\n"
    "    bnez   $t6,.L8004E7A4\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004E7A4\n"
    "    sll   $v0,$v0,2\n"
    "    lw   $a0,16($s0)\n"
    "    addiu   $t0,$zero,0x555\n"
    "    mtc2   $t0,$8\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    srl   $t4,$a0,18\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    nop\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    srl   $t4,$a0,8\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    gpf   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    sll   $t4,$a0,2\n"
    "    andi   $t4,$t4,0xFFC\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    add   $t4,$t4,$t0\n"
    "    lw   $t4,0($t4)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    gpl   1\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lhu   $t2,8($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004E7A4:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004E5D8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004E7B0:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004E7E4\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EA94\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004E848:\n"
    "    lw   $a0,16($s0)\n"
    "    lwc2   $6,8($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004EA88\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004EA88\n"
    "    nop\n"
    "    bnez   $t8,.L8004EA88\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004EA88\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lw   $a2,20($s0)\n"
    "    lh   $a3,14($s0)\n"
    "    addu   $a0,$a2,$zero\n"
    "    andi   $a0,$a0,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    addu   $a1,$a2,$zero\n"
    "    andi   $a1,$a1,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    andi   $a2,$a2,0x3FF\n"
    "    addiu   $t0,$zero,0x400\n"
    "    mtc2   $t0,$8\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    sll   $a0,$a0,2\n"
    "    add   $a0,$a0,$t0\n"
    "    lw   $t4,0($a0)\n"
    "    nop\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a1,$a1,2\n"
    "    add   $a1,$a1,$t0\n"
    "    lw   $t4,0($a1)\n"
    "    gpf   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a2,$a2,2\n"
    "    add   $a2,$a2,$t0\n"
    "    lw   $t4,0($a2)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    sll   $a3,$a3,2\n"
    "    add   $a3,$a3,$t0\n"
    "    lw   $t4,0($a3)\n"
    "    gpl   1\n"
    "    srl   $t2,$t4,4\n"
    "    srl   $t3,$t4,12\n"
    "    sll   $t1,$t4,4\n"
    "    andi   $t1,$t1,0xFF0\n"
    "    andi   $t2,$t2,0xFF0\n"
    "    andi   $t3,$t3,0xFF0\n"
    "    mtc2   $t1,$9\n"
    "    mtc2   $t2,$10\n"
    "    mtc2   $t3,$11\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    lh   $t3,12($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004EA88:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004E848\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EA94:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004EAC8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EC94\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004EB2C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004EC88\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004EC88\n"
    "    nop\n"
    "    bnez   $t6,.L8004EC88\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004EC88\n"
    "    sll   $v0,$v0,2\n"
    "    lw   $a0,16($s0)\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    srl   $t1,$a0,18\n"
    "    srl   $t2,$a0,8\n"
    "    sll   $a0,$a0,2\n"
    "    andi   $t1,$t1,0xFFC\n"
    "    andi   $t2,$t2,0xFFC\n"
    "    andi   $a0,$a0,0xFFC\n"
    "    add   $t1,$t1,$t0\n"
    "    add   $t2,$t2,$t0\n"
    "    add   $a0,$a0,$t0\n"
    "    lw   $t0,0($t1)\n"
    "    lw   $t1,0($t2)\n"
    "    lw   $t2,0($a0)\n"
    "    lui   $a0,0x3400\n"
    "    or   $t0,$t0,$a0\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,28($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004EC88:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004EB2C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EC94:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004ECC8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004EEDC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004ED2C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004EED0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004EED0\n"
    "    nop\n"
    "    bnez   $t8,.L8004EED0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004EED0\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $a2,20($s0)\n"
    "    lh   $a3,14($s0)\n"
    "    addu   $a0,$a2,$zero\n"
    "    andi   $a0,$a0,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    addu   $a1,$a2,$zero\n"
    "    andi   $a1,$a1,0x3FF\n"
    "    srl   $a2,$a2,10\n"
    "    andi   $a2,$a2,0x3FF\n"
    "    lui   $t0,%hi(D_800F2B70)\n"
    "    addiu   $t0,$t0,%lo(D_800F2B70)\n"
    "    sll   $a0,$a0,2\n"
    "    sll   $a1,$a1,2\n"
    "    sll   $a2,$a2,2\n"
    "    sll   $a3,$a3,2\n"
    "    add   $a0,$a0,$t0\n"
    "    add   $a1,$a1,$t0\n"
    "    add   $a2,$a2,$t0\n"
    "    add   $a3,$a3,$t0\n"
    "    lw   $t0,0($a0)\n"
    "    lw   $t1,0($a1)\n"
    "    lw   $t2,0($a2)\n"
    "    lw   $t3,0($a3)\n"
    "    lui   $a0,0x3C00\n"
    "    or   $t0,$t0,$a0\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,28($s6)\n"
    "    sw   $t3,40($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lw   $t0,0($s0)\n"
    "    lw   $t1,4($s0)\n"
    "    lh   $t2,8($s0)\n"
    "    lh   $t3,12($s0)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004EED0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004ED2C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004EEDC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004EF10\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F0CC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004EF70:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F0C0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004F0C0\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004F0C0\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F0C0\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$0\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004F0C0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004EF70\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F0CC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F0FC\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F2E4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F15C:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004F2D8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004F2D8\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004F2D8\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F2D8\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    mtc2   $t0,$1\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t1,$0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    ncs\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    swc2   $22,4($s6)\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004F2D8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004F15C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F2E4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F314\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F50C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004F378:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F500\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bnez   $t6,.L8004F500\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L8004F500\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F500\n"
    "    nop\n"
    "    lw   $t0,16($s0)\n"
    "    lw   $t1,20($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,24($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    xor   $t2,$t2,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,28($s0)\n"
    "    lw   $t1,32($s0)\n"
    "    xor   $t0,$t0,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004F500:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8004F378\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F50C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F53C\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F768\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8004F5A0:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004F75C\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bnez   $t8,.L8004F75C\n"
    "    slt   $a3,$v0,$zero\n"
    "    addi   $a3,$a3,-0x1\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $t3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$t3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004F75C\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8004F75C\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t2,20($s0)\n"
    "    lw   $t1,24($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$0\n"
    "    mtc2   $t1,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lw   $t0,28($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$0\n"
    "    mtc2   $t1,$1\n"
    "    lw   $t2,32($s0)\n"
    "    lw   $t1,36($s0)\n"
    "    xor   $t2,$t2,$a3\n"
    "    xor   $t1,$t1,$a3\n"
    "    mtc2   $t2,$2\n"
    "    mtc2   $t1,$3\n"
    "    lw   $t0,40($s0)\n"
    "    srl   $t1,$t1,16\n"
    "    mtc2   $t0,$4\n"
    "    mtc2   $t1,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L8004F75C:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8004F5A0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F768:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F798\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004F940\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F7F8:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004F934\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004F934\n"
    "    nop\n"
    "    bnez   $t6,.L8004F934\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004F934\n"
    "    sll   $v0,$v0,2\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004F934\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    lui   $t4,0x2580\n"
    "    ori   $t4,$t4,32896\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sw   $t4,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004F934:\n"
    "    addiu   $s0,$s0,0x10\n"
    "    bnez   $s3,.L8004F7F8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004F940:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004F970\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FB44\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004F9D0:\n"
    "    lhu   $a1,16($s0)\n"
    "    lhu   $a0,14($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004FB38\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004FB38\n"
    "    nop\n"
    "    bnez   $t8,.L8004FB38\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004FB38\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    lui   $a0,0x1F80\n"
    "    lw   $a0,24($a0)\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    slt   $at,$v0,$a0\n"
    "    bnez   $at,.L8004FB38\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    lui   $t4,0x2D80\n"
    "    ori   $t4,$t4,32896\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    sw   $t4,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004FB38:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L8004F9D0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FB44:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FB74\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FD10\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FBD4:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L8004FD04\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L8004FD04\n"
    "    nop\n"
    "    bnez   $t6,.L8004FD04\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L8004FD04\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x20\n"
    ".L8004FD04:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004FBD4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FD10:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FD40\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8004FF10\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FDA0:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8004FF04\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8004FF04\n"
    "    nop\n"
    "    bnez   $t8,.L8004FF04\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8004FF04\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    swc2   $22,4($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L8004FF04:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L8004FDA0\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8004FF10:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_8004FF40\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800500EC\n"
    "    lh   $s7,96($s2)\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x700\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8004FFA8:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800500DC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L800500DC\n"
    "    nop\n"
    "    bnez   $t6,.L800500DC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,16($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,24($s6)\n"
    "    blez   $v0,.L800500DC\n"
    "    sll   $v0,$v0,2\n"
    "    lwc2   $0,16($s0)\n"
    "    lwc2   $1,20($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    mtc2   $s7,$8\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    gpf   1\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L800500DC:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    addiu   $s6,$s6,0x20\n"
    "    bnez   $s3,.L8004FFA8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800500EC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050120\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050300\n"
    "    lh   $s7,96($s2)\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050188:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L800502F0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L800502F0\n"
    "    nop\n"
    "    bnez   $t8,.L800502F0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L800502F0\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,16($s6)\n"
    "    sw   $t2,24($s6)\n"
    "    sw   $t7,32($s6)\n"
    "    lhu   $t0,18($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t0,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    mtc2   $s7,$8\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sh   $t2,28($s6)\n"
    "    sh   $t3,36($s6)\n"
    "    gpf   1\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L800502F0:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    addiu   $s6,$s6,0x28\n"
    "    bnez   $s3,.L80050188\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050300:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050334\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050508\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x900\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L80050398:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800504FC\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L800504FC\n"
    "    nop\n"
    "    bnez   $t6,.L800504FC\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,20($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,32($s6)\n"
    "    blez   $v0,.L800504FC\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L800504FC\n"
    "    nop\n"
    "    lwc2   $0,16($s0)\n"
    "    lw   $t0,20($s0)\n"
    "    lwc2   $2,24($s0)\n"
    "    mtc2   $t0,$1\n"
    "    lwc2   $4,28($s0)\n"
    "    lwc2   $5,32($s0)\n"
    "    srl   $t0,$t0,16\n"
    "    mtc2   $t0,$3\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nct\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,4($s6)\n"
    "    swc2   $21,16($s6)\n"
    "    swc2   $22,28($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x28\n"
    ".L800504FC:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L80050398\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050508:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050538\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050744\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0xC00\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,0($s0)\n"
    ".L8005059C:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050738\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050738\n"
    "    nop\n"
    "    bnez   $t8,.L80050738\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80050738\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80050738\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,44($s6)\n"
    "    lw   $t9,24($s0)\n"
    "    lwc2   $0,20($s0)\n"
    "    mtc2   $t9,$1\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    ncs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    swc2   $22,4($s6)\n"
    "    lwc2   $0,28($s0)\n"
    "    srl   $t9,$t9,16\n"
    "    mtc2   $t9,$1\n"
    "    lwc2   $2,32($s0)\n"
    "    lw   $t9,36($s0)\n"
    "    lwc2   $4,40($s0)\n"
    "    mtc2   $t9,$3\n"
    "    srl   $t8,$t9,16\n"
    "    mtc2   $t8,$5\n"
    "    lh   $t4,4($s0)\n"
    "    lh   $t5,0($s0)\n"
    "    sll   $t4,$t4,16\n"
    "    sll   $t5,$t5,16\n"
    "    nct\n"
    "    lhu   $t0,6($s0)\n"
    "    lhu   $t1,8($s0)\n"
    "    lhu   $t2,10($s0)\n"
    "    lhu   $t3,12($s0)\n"
    "    or   $t0,$t0,$t4\n"
    "    or   $t1,$t1,$t5\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    sh   $t2,36($s6)\n"
    "    sh   $t3,48($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $20,16($s6)\n"
    "    swc2   $21,28($s6)\n"
    "    swc2   $22,40($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x34\n"
    ".L80050738:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L8005059C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050744:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050774\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800508D8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L800507D4:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L800508C8\n"
    "    andi   $t6,$t6,0x8000\n"
    "    mfc2   $v0,$24\n"
    "    bnez   $t6,.L800508C8\n"
    "    nop\n"
    "    bgez   $v0,.L800508C8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L800508C8\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $t0,0x2000\n"
    "    sw   $t0,4($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    ".L800508C8:\n"
    "    addiu   $s0,$s0,0x4\n"
    "    addiu   $s6,$s6,0x14\n"
    "    bnez   $s3,.L800507D4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800508D8:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050908\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050A88\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050968:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050A78\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050A78\n"
    "    nop\n"
    "    bnez   $t8,.L80050A78\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80050A78\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    lui   $t0,0x2800\n"
    "    sw   $t0,4($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    ".L80050A78:\n"
    "    addiu   $s0,$s0,0x4\n"
    "    addiu   $s6,$s6,0x18\n"
    "    bnez   $s3,.L80050968\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050A88:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050AB8\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050C34\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050B1C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80050C28\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80050C28\n"
    "    nop\n"
    "    bnez   $t6,.L80050C28\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050C28\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80050C28:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L80050B1C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050C34:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050C68\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050E2C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050CCC:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80050E20\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80050E20\n"
    "    nop\n"
    "    bnez   $t8,.L80050E20\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050E20\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80050E20:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80050CCC\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050E2C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80050E60\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80050FDC\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80050EC4:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80050FD0\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80050FD0\n"
    "    nop\n"
    "    bnez   $t6,.L80050FD0\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80050FD0\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80050FD0:\n"
    "    addiu   $s0,$s0,0x14\n"
    "    bnez   $s3,.L80050EC4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80050FDC:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051010\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L800511D4\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,580\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051074:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L800511C8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L800511C8\n"
    "    nop\n"
    "    bnez   $t8,.L800511C8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L800511C8\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L800511C8:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051074\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L800511D4:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051208\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051380\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051268:\n"
    "    lw   $a0,12($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051374\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051374\n"
    "    nop\n"
    "    bnez   $t6,.L80051374\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051374\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80051374:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051268\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051380:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800513B0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8005157C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051410:\n"
    "    lhu   $a0,14($s0)\n"
    "    lhu   $a1,16($s0)\n"
    "    lwc2   $6,0($s0)\n"
    "    sll   $a1,$a1,16\n"
    "    or   $a0,$a0,$a1\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051570\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051570\n"
    "    nop\n"
    "    bnez   $t8,.L80051570\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051570\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80051570:\n"
    "    addiu   $s0,$s0,0x18\n"
    "    bnez   $s3,.L80051410\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8005157C:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800515AC\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051724\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x600\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L8005160C:\n"
    "    lw   $a0,12($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051718\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051718\n"
    "    nop\n"
    "    bnez   $t6,.L80051718\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t0,20($s6)\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    sll   $v0,$v0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051718\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    lui   $t0,0x4C80\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,24($s6)\n"
    "    addiu   $s6,$s6,0x1C\n"
    ".L80051718:\n"
    "    addiu   $s0,$s0,0x24\n"
    "    bnez   $s3,.L8005160C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051724:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051754\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051914\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $s6,%hi(D_800A38B4)\n"
    "    lw   $s6,%lo(D_800A38B4)($s6)\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L800517B4:\n"
    "    lw   $a0,16($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051908\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051908\n"
    "    nop\n"
    "    bnez   $t8,.L80051908\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    slti   $t9,$v0,0x1\n"
    "    bnez   $t9,.L80051908\n"
    "    nop\n"
    "    sw   $t1,8($s6)\n"
    "    sw   $t0,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t2,32($s6)\n"
    "    sw   $t7,36($s6)\n"
    "    sw   $t1,40($s6)\n"
    "    lui   $t0,0x4880\n"
    "    ori   $t0,$t0,32896\n"
    "    lui   $t1,0x5555\n"
    "    ori   $t1,$t1,21845\n"
    "    sw   $t0,4($s6)\n"
    "    sw   $t1,20($s6)\n"
    "    sw   $t0,28($s6)\n"
    "    sw   $t1,44($s6)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    addiu   $s6,$s6,0x18\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    addiu   $s6,$s6,0x18\n"
    ".L80051908:\n"
    "    addiu   $s0,$s0,0x2C\n"
    "    bnez   $s3,.L800517B4\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051914:\n"
    "    lui   $at,%hi(D_800A38B4)\n"
    "    sw   $s6,%lo(D_800A38B4)($at)\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051944\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051AD8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x14\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x28\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L800519B8:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051AC8\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051AC8\n"
    "    nop\n"
    "    bnez   $t6,.L80051AC8\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L80051AC8\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051AC8\n"
    "    nop\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    nccs\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L80051AC8:\n"
    "    addiu   $s0,$s0,0x38\n"
    "    addiu   $s6,$s6,0x38\n"
    "    bnez   $s3,.L800519B8\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051AD8:\n"
    "    addi   $s0,$s0,-0x28\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051B04\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,48($sp)\n"
    "    sw   $fp,44($sp)\n"
    "    sw   $s7,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051CD8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,0x1F80\n"
    "    lw   $s4,12($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x18\n"
    "    and   $t1,$t0,$t1\n"
    "    add   $s6,$s0,$t1\n"
    "    addi   $s0,$s0,0x30\n"
    "    addiu   $t1,$zero,0xC\n"
    "    and   $t1,$t0,$t1\n"
    "    addi   $t1,$t1,0x8\n"
    "    add   $s7,$s0,$t1\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    ".L80051B88:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L80051CC4\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L80051CC4\n"
    "    nop\n"
    "    bnez   $t8,.L80051CC4\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L80051CC4\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051CC4\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    lw   $t2,4($s0)\n"
    "    nop\n"
    "    add   $v0,$v0,$t2\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    nop\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s7,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    lui   $at,0x200\n"
    "    or   $t1,$t1,$at\n"
    "    sw   $t1,0($s7)\n"
    ".L80051CC4:\n"
    "    addiu   $s0,$s0,0x50\n"
    "    addiu   $s6,$s6,0x50\n"
    "    addiu   $s7,$s7,0x50\n"
    "    bnez   $s3,.L80051B88\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051CD8:\n"
    "    addi   $s0,$s0,-0x30\n"
    "    lw   $ra,48($sp)\n"
    "    lw   $fp,44($sp)\n"
    "    lw   $s7,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051D08\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L80051EA8\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,%hi(D_800A38D8)\n"
    "    lw   $s4,%lo(D_800A38D8)($s4)\n"
    "    lui   $s5,0x400\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x14\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x28\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L80051D7C:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t6,$t0,$t1\n"
    "    and   $t6,$t6,$t2\n"
    "    nclip\n"
    "    bltz   $t6,.L80051E98\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t6,$t6,0x8000\n"
    "    bgez   $v0,.L80051E98\n"
    "    nop\n"
    "    bnez   $t6,.L80051E98\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    mtc2   $a0,$17\n"
    "    mtc2   $a1,$18\n"
    "    mtc2   $a2,$19\n"
    "    sw   $t0,8($s6)\n"
    "    nop\n"
    "    avsz3\n"
    "    sw   $t1,12($s6)\n"
    "    mfc2   $v0,$7\n"
    "    sw   $t2,16($s6)\n"
    "    blez   $v0,.L80051E98\n"
    "    sll   $v0,$v0,2\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L80051E98\n"
    "    nop\n"
    "    addi   $v0,$v0,-0x3E8\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    nccs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    srl   $v0,$v0,4\n"
    "    andi   $v0,$v0,0xFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L80051E98:\n"
    "    addiu   $s0,$s0,0x38\n"
    "    addiu   $s6,$s6,0x38\n"
    "    bnez   $s3,.L80051D7C\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L80051EA8:\n"
    "    addi   $s0,$s0,-0x28\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80051ED4\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s6,36($sp)\n"
    "    sw   $s5,32($sp)\n"
    "    sw   $s4,28($sp)\n"
    "    sw   $s3,24($sp)\n"
    "    sw   $s2,20($sp)\n"
    "    sw   $s1,16($sp)\n"
    "    beqz   $a0,.L8005208C\n"
    "    addiu   $s3,$a0,-0x1\n"
    "    lui   $s1,0x1F80\n"
    "    ori   $s1,$s1,32\n"
    "    lui   $s2,0x1F80\n"
    "    ori   $s2,$s2,692\n"
    "    lui   $s4,%hi(D_800A38D8)\n"
    "    lw   $s4,%lo(D_800A38D8)($s4)\n"
    "    lui   $s5,0x500\n"
    "    lui   $fp,0xFF\n"
    "    ori   $fp,$fp,65535\n"
    "    lui   $t0,0x1F80\n"
    "    lw   $t0,20($t0)\n"
    "    addiu   $t1,$zero,0x18\n"
    "    and   $t0,$t0,$t1\n"
    "    add   $s6,$s0,$t0\n"
    "    addi   $s0,$s0,0x30\n"
    "    lui   $v1,0x1F80\n"
    "    lw   $v1,8($v1)\n"
    "    lwc2   $6,4($s6)\n"
    ".L80051F48:\n"
    "    lw   $a0,0($s0)\n"
    "    nop\n"
    "    sll   $a1,$a0,2\n"
    "    andi   $t3,$a1,0x3FC\n"
    "    addu   $a1,$t3,$s1\n"
    "    lw   $t0,0($a1)\n"
    "    srl   $a1,$a0,6\n"
    "    andi   $t4,$a1,0x3FC\n"
    "    addu   $a1,$t4,$s1\n"
    "    lw   $t1,0($a1)\n"
    "    srl   $a1,$a0,14\n"
    "    andi   $t5,$a1,0x3FC\n"
    "    addu   $a1,$t5,$s1\n"
    "    lw   $t2,0($a1)\n"
    "    srl   $a1,$a0,22\n"
    "    andi   $t6,$a1,0x3FC\n"
    "    addu   $a1,$t6,$s1\n"
    "    lw   $t7,0($a1)\n"
    "    mtc2   $t0,$12\n"
    "    mtc2   $t1,$13\n"
    "    mtc2   $t2,$14\n"
    "    and   $t8,$t0,$t1\n"
    "    and   $t8,$t8,$t2\n"
    "    and   $t8,$t8,$t7\n"
    "    nclip\n"
    "    bltz   $t8,.L8005207C\n"
    "    mfc2   $v0,$24\n"
    "    andi   $t8,$t8,0x8000\n"
    "    bgez   $v0,.L8005207C\n"
    "    nop\n"
    "    bnez   $t8,.L8005207C\n"
    "    srl   $v0,$t3,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a0,0($v0)\n"
    "    srl   $v0,$t4,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a1,0($v0)\n"
    "    srl   $v0,$t5,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a2,0($v0)\n"
    "    srl   $v0,$t6,1\n"
    "    addu   $v0,$v0,$s2\n"
    "    lh   $a3,0($v0)\n"
    "    add   $a0,$a0,$a1\n"
    "    add   $a0,$a0,$a2\n"
    "    add   $a0,$a0,$a3\n"
    "    sra   $v0,$a0,2\n"
    "    blez   $v0,.L8005207C\n"
    "    lui   $at,0x1\n"
    "    slt   $t9,$v0,$at\n"
    "    beqz   $t9,.L8005207C\n"
    "    nop\n"
    "    sw   $t0,8($s6)\n"
    "    sw   $t1,12($s6)\n"
    "    sw   $t2,16($s6)\n"
    "    sw   $t7,20($s6)\n"
    "    addi   $v0,$v0,-0x3E8\n"
    "    lwc2   $0,4($s0)\n"
    "    lwc2   $1,8($s0)\n"
    "    lwc2   $6,12($s0)\n"
    "    srlv   $v0,$v0,$v1\n"
    "    srl   $t0,$v0,11\n"
    "    andi   $v0,$v0,0x7FF\n"
    "    nccs\n"
    "    addi   $v0,$v0,-0x1000\n"
    "    srav   $v0,$v0,$t0\n"
    "    andi   $v0,$v0,0xFFF\n"
    "    srl   $v0,$v0,4\n"
    "    andi   $v0,$v0,0xFF\n"
    "    sll   $v0,$v0,2\n"
    "    addu   $v0,$v0,$s4\n"
    "    lw   $t1,0($v0)\n"
    "    and   $t0,$s6,$fp\n"
    "    sw   $t0,0($v0)\n"
    "    or   $t1,$t1,$s5\n"
    "    sw   $t1,0($s6)\n"
    "    swc2   $22,4($s6)\n"
    ".L8005207C:\n"
    "    addiu   $s0,$s0,0x40\n"
    "    addiu   $s6,$s6,0x40\n"
    "    bnez   $s3,.L80051F48\n"
    "    addiu   $s3,$s3,-0x1\n"
    ".L8005208C:\n"
    "    addi   $s0,$s0,-0x30\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s6,36($sp)\n"
    "    lw   $s5,32($sp)\n"
    "    lw   $s4,28($sp)\n"
    "    lw   $s3,24($sp)\n"
    "    lw   $s2,20($sp)\n"
    "    lw   $s1,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800520B8\n"
    "    slt  $at,$a0,$a1\n"
    "    bnez  $at,.L80052248\n"
    "    nop\n"
    "    addiu  $sp,$sp,-24\n"
    "    sw  $s1,20($sp)\n"
    "    sw  $s0,16($sp)\n"
    "    beqz  $a2,.L80052238\n"
    "    andi  $t0,$a2,3\n"
    "    slt  $t0,$zero,$t0\n"
    "    srl  $a2,$a2,2\n"
    "    add  $a2,$a2,$t0\n"
    "    andi  $v0,$a2,15\n"
    "    srl  $a2,$a2,4\n"
    "    beqz  $a2,.L8005218C\n"
    "    addi  $a2,$a2,-1\n"
    ".L800520F4:\n"
    "    lw  $t0,0($a0)\n"
    "    lw  $t1,4($a0)\n"
    "    lw  $t2,8($a0)\n"
    "    lw  $t3,12($a0)\n"
    "    lw  $t4,16($a0)\n"
    "    lw  $t5,20($a0)\n"
    "    lw  $t6,24($a0)\n"
    "    lw  $t7,28($a0)\n"
    "    sw  $t0,0($a1)\n"
    "    sw  $t1,4($a1)\n"
    "    sw  $t2,8($a1)\n"
    "    sw  $t3,12($a1)\n"
    "    sw  $t4,16($a1)\n"
    "    sw  $t5,20($a1)\n"
    "    sw  $t6,24($a1)\n"
    "    sw  $t7,28($a1)\n"
    "    lw  $t0,32($a0)\n"
    "    lw  $t1,36($a0)\n"
    "    lw  $t2,40($a0)\n"
    "    lw  $t3,44($a0)\n"
    "    lw  $t4,48($a0)\n"
    "    lw  $t5,52($a0)\n"
    "    lw  $t6,56($a0)\n"
    "    lw  $t7,60($a0)\n"
    "    sw  $t0,32($a1)\n"
    "    sw  $t1,36($a1)\n"
    "    sw  $t2,40($a1)\n"
    "    sw  $t3,44($a1)\n"
    "    sw  $t4,48($a1)\n"
    "    sw  $t5,52($a1)\n"
    "    sw  $t6,56($a1)\n"
    "    sw  $t7,60($a1)\n"
    "    addi  $a0,$a0,64\n"
    "    addi  $a1,$a1,64\n"
    "    bnez  $a2,.L800520F4\n"
    "    addi  $a2,$a2,-1\n"
    "    beqz  $v0,.L80052238\n"
    "    nop\n"
    ".L8005218C:\n"
    "    addiu  $v1,$zero,15\n"
    "    sub  $v1,$v1,$v0\n"
    "    sll  $v1,$v1,2\n"
    "    lui  $t9,%hi(D_800521AC)\n"
    "    addiu  $t9,$t9,%lo(D_800521AC)\n"
    "    add  $t9,$t9,$v1\n"
    "    jr  $t9\n"
    "    nop\n"
    "alabel D_800521AC\n"
    "    lw  $t0,56($a0)\n"
    "    lw  $t1,52($a0)\n"
    "    lw  $t2,48($a0)\n"
    "    lw  $t3,44($a0)\n"
    "    lw  $t4,40($a0)\n"
    "    lw  $t5,36($a0)\n"
    "    lw  $t6,32($a0)\n"
    "    lw  $t7,28($a0)\n"
    "    lw  $t8,24($a0)\n"
    "    lw  $t9,20($a0)\n"
    "    lw  $a3,16($a0)\n"
    "    lw  $a2,12($a0)\n"
    "    lw  $v0,8($a0)\n"
    "    lw  $s1,4($a0)\n"
    "    lw  $s0,0($a0)\n"
    "    lui  $a0,%hi(D_800521FC)\n"
    "    addiu  $a0,$a0,%lo(D_800521FC)\n"
    "    add  $a0,$a0,$v1\n"
    "    jr  $a0\n"
    "    nop\n"
    "alabel D_800521FC\n"
    "    sw  $t0,56($a1)\n"
    "    sw  $t1,52($a1)\n"
    "    sw  $t2,48($a1)\n"
    "    sw  $t3,44($a1)\n"
    "    sw  $t4,40($a1)\n"
    "    sw  $t5,36($a1)\n"
    "    sw  $t6,32($a1)\n"
    "    sw  $t7,28($a1)\n"
    "    sw  $t8,24($a1)\n"
    "    sw  $t9,20($a1)\n"
    "    sw  $a3,16($a1)\n"
    "    sw  $a2,12($a1)\n"
    "    sw  $v0,8($a1)\n"
    "    sw  $s1,4($a1)\n"
    "    sw  $s0,0($a1)\n"
    ".L80052238:\n"
    "    lw  $s1,20($sp)\n"
    "    lw  $s0,16($sp)\n"
    "    jr  $ra\n"
    "    addiu  $sp,$sp,24\n"
    ".L80052248:\n"
    "    addiu  $sp,$sp,-24\n"
    "    sw  $s1,20($sp)\n"
    "    sw  $s0,16($sp)\n"
    "    beqz  $a2,.L800523D0\n"
    "    andi  $t0,$a2,3\n"
    "    slt  $t0,$zero,$t0\n"
    "    srl  $a2,$a2,2\n"
    "    add  $a2,$a2,$t0\n"
    "    sll  $t0,$a2,2\n"
    "    add  $a0,$a0,$t0\n"
    "    add  $a1,$a1,$t0\n"
    "    addi  $a0,$a0,-4\n"
    "    addi  $a1,$a1,-4\n"
    "    andi  $v0,$a2,15\n"
    "    srl  $a2,$a2,4\n"
    "    beqz  $a2,.L80052324\n"
    "    addi  $a2,$a2,-1\n"
    ".L8005228C:\n"
    "    lw  $t0,0($a0)\n"
    "    lw  $t1,-4($a0)\n"
    "    lw  $t2,-8($a0)\n"
    "    lw  $t3,-12($a0)\n"
    "    lw  $t4,-16($a0)\n"
    "    lw  $t5,-20($a0)\n"
    "    lw  $t6,-24($a0)\n"
    "    lw  $t7,-28($a0)\n"
    "    sw  $t0,0($a1)\n"
    "    sw  $t1,-4($a1)\n"
    "    sw  $t2,-8($a1)\n"
    "    sw  $t3,-12($a1)\n"
    "    sw  $t4,-16($a1)\n"
    "    sw  $t5,-20($a1)\n"
    "    sw  $t6,-24($a1)\n"
    "    sw  $t7,-28($a1)\n"
    "    lw  $t0,-32($a0)\n"
    "    lw  $t1,-36($a0)\n"
    "    lw  $t2,-40($a0)\n"
    "    lw  $t3,-44($a0)\n"
    "    lw  $t4,-48($a0)\n"
    "    lw  $t5,-52($a0)\n"
    "    lw  $t6,-56($a0)\n"
    "    lw  $t7,-60($a0)\n"
    "    sw  $t0,-32($a1)\n"
    "    sw  $t1,-36($a1)\n"
    "    sw  $t2,-40($a1)\n"
    "    sw  $t3,-44($a1)\n"
    "    sw  $t4,-48($a1)\n"
    "    sw  $t5,-52($a1)\n"
    "    sw  $t6,-56($a1)\n"
    "    sw  $t7,-60($a1)\n"
    "    addi  $a0,$a0,-64\n"
    "    addi  $a1,$a1,-64\n"
    "    bnez  $a2,.L8005228C\n"
    "    addi  $a2,$a2,-1\n"
    "    beqz  $v0,.L800523D0\n"
    "    nop\n"
    ".L80052324:\n"
    "    addiu  $v1,$zero,15\n"
    "    sub  $v1,$v1,$v0\n"
    "    sll  $v1,$v1,2\n"
    "    lui  $t9,%hi(D_80052344)\n"
    "    addiu  $t9,$t9,%lo(D_80052344)\n"
    "    add  $t9,$t9,$v1\n"
    "    jr  $t9\n"
    "    nop\n"
    "alabel D_80052344\n"
    "    lw  $t0,-56($a0)\n"
    "    lw  $t1,-52($a0)\n"
    "    lw  $t2,-48($a0)\n"
    "    lw  $t3,-44($a0)\n"
    "    lw  $t4,-40($a0)\n"
    "    lw  $t5,-36($a0)\n"
    "    lw  $t6,-32($a0)\n"
    "    lw  $t7,-28($a0)\n"
    "    lw  $t8,-24($a0)\n"
    "    lw  $t9,-20($a0)\n"
    "    lw  $a3,-16($a0)\n"
    "    lw  $a2,-12($a0)\n"
    "    lw  $v0,-8($a0)\n"
    "    lw  $s1,-4($a0)\n"
    "    lw  $s0,0($a0)\n"
    "    lui  $a0,%hi(D_80052394)\n"
    "    addiu  $a0,$a0,%lo(D_80052394)\n"
    "    add  $a0,$a0,$v1\n"
    "    jr  $a0\n"
    "    nop\n"
    "alabel D_80052394\n"
    "    sw  $t0,-56($a1)\n"
    "    sw  $t1,-52($a1)\n"
    "    sw  $t2,-48($a1)\n"
    "    sw  $t3,-44($a1)\n"
    "    sw  $t4,-40($a1)\n"
    "    sw  $t5,-36($a1)\n"
    "    sw  $t6,-32($a1)\n"
    "    sw  $t7,-28($a1)\n"
    "    sw  $t8,-24($a1)\n"
    "    sw  $t9,-20($a1)\n"
    "    sw  $a3,-16($a1)\n"
    "    sw  $a2,-12($a1)\n"
    "    sw  $v0,-8($a1)\n"
    "    sw  $s1,-4($a1)\n"
    "    sw  $s0,0($a1)\n"
    ".L800523D0:\n"
    "    lw  $s1,20($sp)\n"
    "    lw  $s0,16($sp)\n"
    "    jr  $ra\n"
    "    addiu  $sp,$sp,24\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800523E0\n"
    "    addiu   $sp,$sp,-0x38\n"
    "    sw   $ra,44($sp)\n"
    "    sw   $fp,40($sp)\n"
    "    sw   $s5,36($sp)\n"
    "    sw   $s4,32($sp)\n"
    "    sw   $s3,28($sp)\n"
    "    sw   $s2,24($sp)\n"
    "    sw   $s1,20($sp)\n"
    "    sw   $s0,16($sp)\n"
    "    addu   $s0,$a0,$zero\n"
    "    addu   $s1,$a1,$zero\n"
    "    addu   $s3,$a2,$zero\n"
    "    addu   $s2,$a3,$zero\n"
    "    addiu   $s4,$zero,0x1000\n"
    "    sub   $s4,$s4,$s3\n"
    "    lh   $t0,0($s0)\n"
    "    lh   $t1,2($s0)\n"
    "    lh   $t2,4($s0)\n"
    "    mtc2   $s4,$8\n"
    "    mtc2   $t0,$9\n"
    "    mtc2   $t1,$10\n"
    "    mtc2   $t2,$11\n"
    "    lh   $t3,0($s1)\n"
    "    lh   $t4,2($s1)\n"
    "    gpf   1\n"
    "    lh   $t5,4($s1)\n"
    "    lh   $a0,6($s0)\n"
    "    lh   $a1,8($s0)\n"
    "    lh   $a2,10($s0)\n"
    "    mtc2   $s3,$8\n"
    "    mtc2   $t3,$9\n"
    "    mtc2   $t4,$10\n"
    "    mtc2   $t5,$11\n"
    "    lh   $s5,6($s1)\n"
    "    lh   $fp,8($s1)\n"
    "    gpl   1\n"
    "    lh   $a3,10($s1)\n"
    "    nop\n"
    "    jal   func_800525D8\n"
    "    nop\n"
    "    mtc2   $s4,$8\n"
    "    mtc2   $a0,$9\n"
    "    mtc2   $a1,$10\n"
    "    mtc2   $a2,$11\n"
    "    addu   $a0,$t0,$zero\n"
    "    addu   $a1,$t1,$zero\n"
    "    gpf   1\n"
    "    addu   $a2,$t2,$zero\n"
    "    sh   $a0,0($s2)\n"
    "    sh   $a1,2($s2)\n"
    "    sh   $a2,4($s2)\n"
    "    mtc2   $s3,$8\n"
    "    mtc2   $s5,$9\n"
    "    mtc2   $fp,$10\n"
    "    mtc2   $a3,$11\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $t0,$9\n"
    "    mfc2   $t1,$10\n"
    "    mfc2   $t2,$11\n"
    "    mult   $a0,$t0\n"
    "    mflo   $t3\n"
    "    nop\n"
    "    nop\n"
    "    mult   $a1,$t1\n"
    "    mflo   $t4\n"
    "    nop\n"
    "    nop\n"
    "    mult   $a2,$t2\n"
    "    mflo   $t5\n"
    "    nop\n"
    "    add   $t3,$t3,$t4\n"
    "    add   $t3,$t3,$t5\n"
    "    sra   $t3,$t3,12\n"
    "    neg   $t3,$t3\n"
    "    mtc2   $t3,$8\n"
    "    mtc2   $a0,$9\n"
    "    mtc2   $a1,$10\n"
    "    mtc2   $a2,$11\n"
    "    mtc2   $t0,$25\n"
    "    mtc2   $t1,$26\n"
    "    mtc2   $t2,$27\n"
    "    nop\n"
    "    nop\n"
    "    gpl   1\n"
    "    nop\n"
    "    nop\n"
    "    jal   func_800525D8\n"
    "    nop\n"
    "    cfc2   $t3,$0\n"
    "    cfc2   $t4,$2\n"
    "    cfc2   $t5,$4\n"
    "    ctc2   $a0,$0\n"
    "    ctc2   $a1,$2\n"
    "    ctc2   $a2,$4\n"
    "    mtc2   $t0,$9\n"
    "    mtc2   $t1,$10\n"
    "    mtc2   $t2,$11\n"
    "    nop\n"
    "    nop\n"
    "    op   1\n"
    "    sh   $t0,6($s2)\n"
    "    sh   $t1,8($s2)\n"
    "    sh   $t2,10($s2)\n"
    "    mfc2   $t0,$25\n"
    "    mfc2   $t1,$26\n"
    "    mfc2   $t2,$27\n"
    "    sh   $t0,12($s2)\n"
    "    sh   $t1,14($s2)\n"
    "    sh   $t2,16($s2)\n"
    "    ctc2   $t3,$0\n"
    "    ctc2   $t4,$2\n"
    "    ctc2   $t5,$4\n"
    "    lw   $ra,44($sp)\n"
    "    lw   $fp,40($sp)\n"
    "    lw   $s5,36($sp)\n"
    "    lw   $s4,32($sp)\n"
    "    lw   $s3,28($sp)\n"
    "    lw   $s2,24($sp)\n"
    "    lw   $s1,20($sp)\n"
    "    lw   $s0,16($sp)\n"
    "    jr   $ra\n"
    "    addiu   $sp,$sp,0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800525D8\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t1, $10\n"
    "    mfc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    sqr    0\n"
    "    nop\n"
    "    mfc2   $t3, $25\n"
    "    mfc2   $t4, $26\n"
    "    mfc2   $t5, $27\n"
    "    add    $t3, $t3, $t4\n"
    "    add    $v0, $t3, $t5\n"
    "    mtc2   $v0, $30\n"
    "    nop\n"
    "    nop\n"
    "    mfc2   $v1, $31\n"
    "    nop\n"
    "    andi   $v1, $v1, 0x1E\n"
    "    addiu  $t6, $zero, 0x1F\n"
    "    sub    $t6, $t6, $v1\n"
    "    addi   $t3, $v1, -0x18\n"
    "    bltz   $t3, .L8005263C\n"
    "    sra    $t6, $t6, 1\n"
    "    j      .L80052648\n"
    "    sllv   $t4, $v0, $t3\n"
    ".L8005263C:\n"
    "    addiu  $t3, $zero, 0x18\n"
    "    sub    $t3, $t3, $v1\n"
    "    srav   $t4, $v0, $t3\n"
    ".L80052648:\n"
    "    addi   $t4, $t4, -0x40\n"
    "    sll    $t4, $t4, 1\n"
    "    lui    $t5, %hi(D_800154A0)\n"
    "    addiu  $t5, $t5, %lo(D_800154A0)\n"
    "    addu   $t5, $t5, $t4\n"
    "    lh     $t5, 0($t5)\n"
    "    nop\n"
    "    mtc2   $t5, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpf    0\n"
    "    mfc2   $t0, $25\n"
    "    mfc2   $t1, $26\n"
    "    mfc2   $t2, $27\n"
    "    srav   $t0, $t0, $t6\n"
    "    srav   $t1, $t1, $t6\n"
    "    srav   $t2, $t2, $t6\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800526A0\n"
    "    mtc2   $a0, $30\n"
    "    beqz   $a0, .L80052710\n"
    "    addiu  $t0, $zero, 0x1F\n"
    "    mfc2   $v0, $31\n"
    "    lui    $t1, %hi(D_80015620)\n"
    "    addiu  $t1, $t1, %lo(D_80015620)\n"
    "    andi   $v0, $v0, 0x1E\n"
    "    addi   $t0, $v0, -0x18\n"
    "    bltz   $t0, .L800526E0\n"
    "    nop\n"
    "    sll    $v0, $a0, 1\n"
    "    add    $v0, $v0, $t1\n"
    "    lh     $v0, 0($v0)\n"
    "    nop\n"
    "    jr     $ra\n"
    "    srl    $v0, $v0, 9\n"
    ".L800526E0:\n"
    "    addiu  $t0, $zero, 0x18\n"
    "    sub    $t0, $t0, $v0\n"
    "    srav   $t0, $a0, $t0\n"
    "    sll    $t0, $t0, 1\n"
    "    add    $t0, $t0, $t1\n"
    "    lh     $t0, 0($t0)\n"
    "    addiu  $t1, $zero, 0x1F\n"
    "    sub    $t1, $t1, $v0\n"
    "    sra    $t1, $t1, 1\n"
    "    sllv   $v0, $t0, $t1\n"
    "    jr     $ra\n"
    "    srl    $v0, $v0, 12\n"
    ".L80052710:\n"
    "    jr     $ra\n"
    "    addu   $v0, $zero, $zero\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_2; /* padding after func_800526A0 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052720\n"
    "    mtc2   $a0, $9\n"
    "    mtc2   $a1, $10\n"
    "    mtc2   $a2, $11\n"
    "    nop\n"
    "    nop\n"
    "    sqr    0\n"
    "    nop\n"
    "    mfc2   $t0, $25\n"
    "    mfc2   $t1, $26\n"
    "    mfc2   $t2, $27\n"
    "    add    $a0, $t0, $t1\n"
    "    j      func_800526A0\n"
    "    add    $a0, $a0, $t2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
s32 func_80052754(s32 arg0, s32 arg1, s32 arg2) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    __asm__ volatile ("mtc2 %0, $9" :: "r"(arg0));
    __asm__ volatile ("mtc2 %0, $10" :: "r"(arg1));
    __asm__ volatile ("mtc2 %0, $11" :: "r"(arg2));
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4AA00428");
    __asm__ volatile ("nop");
    __asm__ volatile ("mfc2 %0, $25" : "=r"(t0));
    __asm__ volatile ("mfc2 %0, $26" : "=r"(t1));
    __asm__ volatile ("mfc2 %0, $27" : "=r"(t2));
    return t0 + t1 + t2;
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052788\n"
    "    lh     $t0, 0($a0)\n"
    "    lh     $t1, 2($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    ori    $t3, $zero, 0x1000\n"
    "    sub    $t3, $t3, $a2\n"
    "    mtc2   $t3, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lh     $t0, 0($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t1, 2($a1)\n"
    "    lh     $t2, 4($a1)\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpl    1\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t0, $9\n"
    "    mfc2   $t1, $10\n"
    "    mfc2   $t2, $11\n"
    "    sh     $t0, 0($a3)\n"
    "    sh     $t1, 2($a3)\n"
    "    jr     $ra\n"
    "    sh     $t2, 4($a3)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800527FC\n"
    "    lw     $t0, 0($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    srl    $t1, $t0, 16\n"
    "    ori    $v0, $zero, 0x1000\n"
    "    sub    $v0, $v0, $a2\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 0($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t2, 4($a1)\n"
    "    sra    $t1, $t0, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t1, 8($a0)\n"
    "    nop\n"
    "    gpl    1\n"
    "    lh     $t0, 6($a0)\n"
    "    srl    $t2, $t1, 16\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 0($a3)\n"
    "    sh     $t4, 2($a3)\n"
    "    sh     $t5, 4($a3)\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t1, 8($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t0, 6($a1)\n"
    "    srl    $t2, $t1, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 12($a0)\n"
    "    nop\n"
    "    gpl    1\n"
    "    lh     $t2, 16($a0)\n"
    "    srl    $t1, $t0, 16\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 6($a3)\n"
    "    sh     $t4, 8($a3)\n"
    "    sh     $t5, 10($a3)\n"
    "    mtc2   $v0, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    lw     $t0, 12($a1)\n"
    "    nop\n"
    "    gpf    1\n"
    "    lh     $t2, 16($a1)\n"
    "    srl    $t1, $t0, 16\n"
    "    mtc2   $a2, $8\n"
    "    mtc2   $t0, $9\n"
    "    mtc2   $t1, $10\n"
    "    mtc2   $t2, $11\n"
    "    nop\n"
    "    nop\n"
    "    gpl    1\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t3, $9\n"
    "    mfc2   $t4, $10\n"
    "    mfc2   $t5, $11\n"
    "    sh     $t3, 12($a3)\n"
    "    sh     $t4, 14($a3)\n"
    "    jr     $ra\n"
    "    sh     $t5, 16($a3)\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
void func_80052930(s32 *mat, s32 *vec, s16 *out) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    register u32 mask asm("$25");
    register s32 v0 asm("$2");
    register s32 v1 asm("$3");
    t0 = mat[0];
    t1 = mat[1];
    t2 = mat[2];
    t3 = mat[3];
    t4 = mat[4];
    __asm__ volatile ("lui %0, 0xFFFF" : "=r"(mask));
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 $zero, $5");
    __asm__ volatile ("ctc2 $zero, $6");
    __asm__ volatile ("ctc2 $zero, $7");
    t0 = vec[0];
    t1 = vec[1];
    t2 = vec[2];
    t3 = vec[3];
    t4 = vec[4];
    t5 = t1 & mask;
    t6 = t0 & 0xFFFF;
    t5 = t5 | t6;
    __asm__ volatile ("mtc2 %0, $0" :: "r"(t5));
    t6 = t3 & 0xFFFF;
    __asm__ volatile ("mtc2 %0, $1" :: "r"(t6));
    v0 = t2 << 16;
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");
    v1 = ((u32)t0) >> 16;
    v0 = v0 | v1;
    v1 = ((u32)t3) >> 16;
    __asm__ volatile ("mfc2 %0, $9" : "=r"(t5));
    __asm__ volatile ("mfc2 %0, $10" : "=r"(t6));
    __asm__ volatile ("mfc2 %0, $11" : "=r"(t7));
    __asm__ volatile ("mtc2 %0, $0" :: "r"(v0));
    __asm__ volatile ("mtc2 %0, $1" :: "r"(v1));
    out[0] = (s16)t5;
    out[3] = (s16)t6;
    out[6] = (s16)t7;
    __asm__ volatile (".word 0x4A480012");
    v0 = t2 & mask;
    v1 = t1 & 0xFFFF;
    v0 = v0 | v1;
    v1 = t4 & 0xFFFF;
    __asm__ volatile ("mfc2 %0, $9" : "=r"(t5));
    __asm__ volatile ("mfc2 %0, $10" : "=r"(t6));
    __asm__ volatile ("mfc2 %0, $11" : "=r"(t7));
    __asm__ volatile ("mtc2 %0, $0" :: "r"(v0));
    __asm__ volatile ("mtc2 %0, $1" :: "r"(v1));
    out[1] = (s16)t5;
    out[4] = (s16)t6;
    out[7] = (s16)t7;
    __asm__ volatile (".word 0x4A480012");
    __asm__ volatile ("mfc2 %0, $9" : "=r"(t5));
    __asm__ volatile ("mfc2 %0, $10" : "=r"(t6));
    __asm__ volatile ("mfc2 %0, $11" : "=r"(t7));
    out[2] = (s16)t5;
    out[5] = (s16)t6;
    out[8] = (s16)t7;
}
void game_2d_CheckLifeGaugeNoDisp(s32 *matrix, s32 *vec, s16 *out) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    t5 = matrix[5];
    t6 = matrix[6];
    t7 = matrix[7];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 %0, $5" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $6" :: "r"(t6));
    __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
    __asm__ volatile ("lwc2 $0, 0(%0)" :: "r"(vec));
    __asm__ volatile ("lwc2 $1, 4(%0)" :: "r"(vec));
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");               /* mvmva 1,0,0,0,0 */
    __asm__ volatile ("nop");
    __asm__ volatile ("swc2 $9, 0(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $10, 4(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $11, 8(%0)" :: "r"(out));
}
void func_80052A88(s32 *matrix, s32 *vec, s32 *out) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    t5 = matrix[5];
    t6 = matrix[6];
    t7 = matrix[7];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    t1 = ((s16 *)vec)[2];
    t0 = ((u16 *)vec)[0] | (t1 << 16);
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 %0, $5" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $6" :: "r"(t6));
    __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
    __asm__ volatile ("mtc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("lwc2 $1, 8(%0)" :: "r"(vec));
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");               /* mvmva 1,0,0,0,0 */
    __asm__ volatile ("nop");
    __asm__ volatile ("swc2 $9, 0(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $10, 4(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $11, 8(%0)" :: "r"(out));
}
void func_80052B00(s32 *matrix) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    t5 = matrix[5];
    t6 = matrix[6];
    t7 = matrix[7];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 %0, $5" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $6" :: "r"(t6));
    __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
}
void func_80052B44(s32 *matrix) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 $0, $5");
    __asm__ volatile ("ctc2 $0, $6");
    __asm__ volatile ("ctc2 $0, $7");
}
void func_80052B7C(s32 *matrix5, s16 *tr3, s32 *vec, s32 *out) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    register s32 t2 asm("$10");
    register s32 t3 asm("$11");
    register s32 t4 asm("$12");
    register s32 t5 asm("$13");
    register s32 t6 asm("$14");
    register s32 t7 asm("$15");
    t0 = matrix5[0];
    t1 = matrix5[1];
    t2 = matrix5[2];
    t3 = matrix5[3];
    t4 = matrix5[4];
    t5 = tr3[0];
    t6 = tr3[1];
    t7 = tr3[2];
    __asm__ volatile ("ctc2 %0, $0" :: "r"(t0));
    __asm__ volatile ("ctc2 %0, $1" :: "r"(t1));
    __asm__ volatile ("ctc2 %0, $2" :: "r"(t2));
    __asm__ volatile ("ctc2 %0, $3" :: "r"(t3));
    __asm__ volatile ("ctc2 %0, $4" :: "r"(t4));
    __asm__ volatile ("ctc2 %0, $5" :: "r"(t5));
    __asm__ volatile ("ctc2 %0, $6" :: "r"(t6));
    __asm__ volatile ("ctc2 %0, $7" :: "r"(t7));
    __asm__ volatile ("lwc2 $0, 0(%0)" :: "r"(vec));
    __asm__ volatile ("lwc2 $1, 4(%0)" :: "r"(vec));
    __asm__ volatile ("nop");
    __asm__ volatile ("nop");
    __asm__ volatile (".word 0x4A480012");               /* mvmva 1,0,0,0,0 */
    __asm__ volatile ("nop");
    __asm__ volatile ("swc2 $9, 0(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $10, 4(%0)" :: "r"(out));
    __asm__ volatile ("swc2 $11, 8(%0)" :: "r"(out));
}
void func_80052BE4(u8 *a0) {
    register u32 t0 asm("$8");
    register u32 t1 asm("$9");
    register u32 t2 asm("$10");
    __asm__ volatile (".word 0x4848A800" : "=r"(t0));  /* cfc2 $t0, $21 */
    __asm__ volatile (".word 0x4849B000" : "=r"(t1));  /* cfc2 $t1, $22 */
    __asm__ volatile (".word 0x484AB800" : "=r"(t2));  /* cfc2 $t2, $23 */
    a0[0] = t0 >> 4;
    a0[1] = t1 >> 4;
    a0[2] = t2 >> 4;
    __asm__ volatile ("" ::: "memory");
}
void InitFadePanel(void) {
    *(volatile s32 *)0x1F800400 = 0;
}
PAD_NOPS_1; /* padding after InitFadePanel */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052C28\n"
    "    addiu  $t0,$zero,3\n"
    "    sub  $a1,$t0,$a1\n"
    "    srlv  $a0,$a0,$a1\n"
    "    srl  $t0,$a0,11\n"
    "    andi  $a0,$a0,2047\n"
    "    addi  $a0,$a0,-4096\n"
    "    srav  $a0,$a0,$t0\n"
    "    jr  $ra\n"
    "    andi  $v0,$a0,4095\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80052C4C\n"
    "    lhu    $t0, 6($a0)\n"
    "    lh     $t1, 8($a0)\n"
    "    lhu    $t2, 10($a0)\n"
    "    lh     $t3, 12($a0)\n"
    "    lhu    $t4, 14($a0)\n"
    "    lh     $t5, 16($a0)\n"
    "    sll    $t1, $t1, 16\n"
    "    sll    $t3, $t3, 16\n"
    "    sll    $t5, $t5, 16\n"
    "    or     $t0, $t0, $t1\n"
    "    or     $t2, $t2, $t3\n"
    "    or     $t4, $t4, $t5\n"
    "    ctc2   $t0, $0\n"
    "    ctc2   $t2, $1\n"
    "    ctc2   $t4, $2\n"
    "    ctc2   $zero, $5\n"
    "    ctc2   $zero, $6\n"
    "    ctc2   $zero, $7\n"
    "    lh     $t0, 0($a0)\n"
    "    lh     $t1, 2($a0)\n"
    "    lh     $t2, 4($a0)\n"
    "    sub    $a1, $a1, $t0\n"
    "    sub    $a2, $a2, $t1\n"
    "    sub    $a3, $a3, $t2\n"
    "    andi   $a1, $a1, 0xFFFF\n"
    "    sll    $a2, $a2, 16\n"
    "    or     $a1, $a1, $a2\n"
    "    mtc2   $a1, $0\n"
    "    mtc2   $a3, $1\n"
    "    nop\n"
    "    nop\n"
    "    rtv0tr\n"
    "    jr     $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
void func_80052CD4(s32 *a0, s32 *a1) {
    register s32 t0 asm("$8");
    register s32 t1 asm("$9");
    __asm__ volatile (".word 0x48084800" : "=r"(t0));  /* mfc2 $t0, $9 */
    __asm__ volatile (".word 0x48095000" : "=r"(t1));  /* mfc2 $t1, $10 */
    *a0 = t0 >> 2;
    *a1 = t1 >> 2;
    __asm__ volatile ("" ::: "memory");
}
PAD_NOPS_3; /* padding after func_80052CD4 */
s32 func_80052D00(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80052D00.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern s32 func_80052754(s32, s32, s32);
extern s32 func_80052D00();
extern void func_80053754();
extern void func_80053E9C();
extern u8 D_800EFA00;
extern u8 D_800EF9F8;
extern s32 D_800A33F4;
typedef struct { s32 a, b, c, d; } _S16_53304;
void func_80053304(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3) {
    u8 *p;
    s32 a, b, new_var, c;
    D_800A33F4 = (s32)&D_800EF9F8;
    *(_S16_53304 *)&D_800EFA00 = *(_S16_53304 *)arg0;
    *(_S16_53304 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53304 *)arg1;
    if (func_80052754(
            *(s32 *)((u8 *)D_800A33F4 + 0x18) - *(s32 *)((u8 *)D_800A33F4 + 0x8),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C) - *(s32 *)((u8 *)D_800A33F4 + 0xC),
            *(s32 *)((u8 *)D_800A33F4 + 0x20) - *(s32 *)((u8 *)D_800A33F4 + 0x10)) <= 0x9C3F) {
        p = (u8 *)D_800A33F4;
        new_var = *(s32 *)(p + 0x1C);
        a = *(s32 *)(p + 0x8);
        b = *(s32 *)(p + 0xC);
        *(s32 *)(p + 0x5C) = (s32)func_80053754;
        c = *(s32 *)(p + 0x10);
        *(s32 *)(p + 0x8)  = a - ((*(s32 *)(p + 0x18) - a) << 1);
        *(s32 *)(p + 0xC)  = b - ((new_var - b) << 1);
        *(s32 *)(p + 0x10) = c - ((*(s32 *)(p + 0x20) - c) << 1);
    } else {
        *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    }
    func_80052D00(arg2, arg3);
}
extern s32 func_80052754(s32, s32, s32, s32);
extern s32 func_80052D00(s32, s32);
extern void func_80053754();
extern void func_80053E9C();
extern u8 *D_800A33F4;
typedef struct { s32 a, b, c, d; } _S16_5344C;
void func_8005344C(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 arg4) {
    s32 vx, vy, vz;
    s32 bx, by, bz;
    D_800A33F4 = (u8 *)arg4;
    *(_S16_5344C *)((u8 *)D_800A33F4 + 8) = *(_S16_5344C *)arg0;
    *(_S16_5344C *)((u8 *)D_800A33F4 + 0x18) = *(_S16_5344C *)arg1;
    if (func_80052754(
            *(s32 *)((u8 *)D_800A33F4 + 0x18) - *(s32 *)((u8 *)D_800A33F4 + 0x8),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C) - *(s32 *)((u8 *)D_800A33F4 + 0xC),
            *(s32 *)((u8 *)D_800A33F4 + 0x20) - *(s32 *)((u8 *)D_800A33F4 + 0x10),
            *(s32 *)((u8 *)D_800A33F4 + 0x1C)) <= 0x9C3F) {
        bx = *(s32 *)((u8 *)D_800A33F4 + 0x18);
        vx = *(s32 *)((u8 *)D_800A33F4 + 0x8);
        by = *(s32 *)((u8 *)D_800A33F4 + 0x1C);
        vy = *(s32 *)((u8 *)D_800A33F4 + 0xC);
        vz = *(s32 *)((u8 *)D_800A33F4 + 0x10);
        *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053754;
        bz = *(s32 *)((u8 *)D_800A33F4 + 0x20);
        *(s32 *)((u8 *)D_800A33F4 + 0x8) = vx - ((bx - vx) << 1);
        *(s32 *)((u8 *)D_800A33F4 + 0xC) = vy - ((by - vy) << 1);
        do { *(s32 *)((u8 *)D_800A33F4 + 0x10) = vz - ((bz - vz) << 1); } while (0);
    } else {
        *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    }
    func_80052D00(arg2, arg3);
}
extern s32 func_80052D00();
extern void func_80053E9C();
extern u8 D_800EFA00;
extern u8 D_800EF9F8;
extern s32 D_800A33F4;
typedef struct { s32 a, b, c, d; } _S16_53584;
void func_80053584(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3) {
    D_800A33F4 = (s32)&D_800EF9F8;
    *(_S16_53584 *)&D_800EFA00 = *(_S16_53584 *)arg0;
    *(_S16_53584 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53584 *)arg1;
    *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    func_80052D00(arg2, arg3);
}
typedef struct { s32 a, b, c, d; } _S16_53614;
void camera_check_inside_screen_rob_dpos(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 arg4) {
    D_800A33F4 = arg4;
    *(_S16_53614 *)((u8 *)D_800A33F4 + 8) = *(_S16_53614 *)arg0;
    *(_S16_53614 *)((u8 *)D_800A33F4 + 0x18) = *(_S16_53614 *)arg1;
    *(s32 *)((u8 *)D_800A33F4 + 0x5C) = (s32)func_80053E9C;
    func_80052D00(arg2, arg3);
}
extern u8 *D_800A33F4;
extern u16 D_800A33F8;
s32 func_80053694(s32 *arg0, s16 *arg1) {
    u8 *p = D_800A33F4;
    s32 t;
    s32 result = 0;
    if (*(s32 *)(p + 0) != 0x7FFFFFFF) {
        t = (*(s16 *)(p + 0x48) * 0x7D0) - 0x7D00;
        arg0[0] = *(s32 *)(p + 0x38) + t;
        arg0[1] = *(s32 *)(p + 0x3C);
        t = (*(s16 *)(p + 0x4A) * 0x7D0) - 0x7D00;
        arg0[2] = *(s32 *)(p + 0x40) + t;
        arg1[0] = *(s32 *)(p + 0x28) >> 2;
        arg1[1] = *(s32 *)(p + 0x2C) >> 2;
        arg1[2] = *(s32 *)(p + 0x30) >> 2;
        result = 1;
        D_800A33F8 = *(u16 *)(p + 4);
    }
    return result;
}
void func_80053754(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80053754.s).
     * 466 inst, 40 branches, 1 jal, 127 loads, 69 stores. m2c output had
     * type errors. Pure-C decomp pending. */
    (void)arg0; (void)arg1;
}
void func_80053E9C(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80053E9C.s).
     * 349 inst, 36 branches, 3 jal, 91 loads, 49 stores. m2c output had
     * type/operand errors. Pure-C decomp pending. */
    (void)arg0; (void)arg1;
}
extern s32 D_800A33F0;
void func_80054410(s32 a0) {
    D_800A33F0 = a0;
}
void func_8005441C(s32 a0) {
    D_800A33F0 += a0;
}
extern s16 D_800A33F8;
s16 func_80054434(void) {
    return D_800A33F8;
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80054440\n"
    "    addiu  $sp, $sp, -0x38\n"
    "    sw     $ra, 40($sp)\n"
    "    sw     $fp, 36($sp)\n"
    "    sw     $s5, 32($sp)\n"
    "    sw     $s4, 28($sp)\n"
    "    sw     $s3, 24($sp)\n"
    "    sw     $s2, 20($sp)\n"
    "    sw     $s1, 16($sp)\n"
    "    addu   $s1, $a0, $zero\n"
    "    addu   $s2, $a1, $zero\n"
    "    lui    $s4, %hi(D_800A3828)\n"
    "    lw     $s4, %lo(D_800A3828)($s4)\n"
    "    nop\n"
    "    addi   $s4, $s4, 24\n"
    "    lui    $s5, 0x1F80\n"
    "    mtc2   $zero, $6\n"
    "    lui    $s3, %hi(D_800F2B70)\n"
    "    addiu  $s3, $s3, %lo(D_800F2B70)\n"
    "    bgez   $zero, .L800545B8\n"
    "    nop\n"
    ".L80054490:\n"
    "    addi   $a2, $s5, 0x0020\n"
    "    sll    $t0, $v0, 2\n"
    "    add    $a1, $s2, $t0\n"
    "    lw     $a1, 0($a1)\n"
    "    jal    func_80052930\n"
    "    addu   $a0, $s4, $zero\n"
    "    lw     $t0, 32($s5)\n"
    "    lw     $t1, 36($s5)\n"
    "    lw     $t2, 40($s5)\n"
    "    lw     $t3, 44($s5)\n"
    "    lw     $t4, 48($s5)\n"
    "    ctc2   $t0, $8\n"
    "    ctc2   $t1, $9\n"
    "    ctc2   $t2, $10\n"
    "    ctc2   $t3, $11\n"
    "    ctc2   $t4, $12\n"
    "    lh     $v0, 0($s1)\n"
    "    addi   $s1, $s1, 2\n"
    "    lhu    $t0, 0($s1)\n"
    "    lh     $t2, 2($s1)\n"
    "    lh     $t1, 4($s1)\n"
    "    sll    $t2, $t2, 16\n"
    "    or     $t0, $t0, $t2\n"
    "    lhu    $t2, 6($s1)\n"
    "    lh     $t4, 8($s1)\n"
    "    lh     $t3, 10($s1)\n"
    "    sll    $t4, $t4, 16\n"
    "    or     $t2, $t2, $t4\n"
    "    lhu    $t4, 12($s1)\n"
    "    lh     $t6, 14($s1)\n"
    "    lh     $t5, 16($s1)\n"
    "    sll    $t6, $t6, 16\n"
    "    or     $t4, $t4, $t6\n"
    "    bgez   $zero, .L80054568\n"
    "    nop\n"
    ".L8005451C:\n"
    "    lhu    $t0, 0($s1)\n"
    "    lh     $t2, 2($s1)\n"
    "    lh     $t1, 4($s1)\n"
    "    sll    $t2, $t2, 16\n"
    "    or     $t0, $t0, $t2\n"
    "    lhu    $t2, 6($s1)\n"
    "    lh     $t4, 8($s1)\n"
    "    lh     $t3, 10($s1)\n"
    "    sll    $t4, $t4, 16\n"
    "    or     $t2, $t2, $t4\n"
    "    lhu    $t4, 12($s1)\n"
    "    lh     $t6, 14($s1)\n"
    "    lh     $t5, 16($s1)\n"
    "    sll    $t6, $t6, 16\n"
    "    or     $t4, $t4, $t6\n"
    "    swc2   $20, 0($s3)\n"
    "    swc2   $21, 4($s3)\n"
    "    swc2   $22, 8($s3)\n"
    "    addi   $s3, $s3, 12\n"
    ".L80054568:\n"
    "    mtc2   $t0, $0\n"
    "    mtc2   $t1, $1\n"
    "    mtc2   $t2, $2\n"
    "    mtc2   $t3, $3\n"
    "    mtc2   $t4, $4\n"
    "    mtc2   $t5, $5\n"
    "    addi   $v0, $v0, -3\n"
    "    nop\n"
    "    nct\n"
    "    bgtz   $v0, .L8005451C\n"
    "    addi   $s1, $s1, 18\n"
    "    swc2   $20, 0($s3)\n"
    "    swc2   $21, 4($s3)\n"
    "    swc2   $22, 8($s3)\n"
    "    addi   $s3, $s3, 12\n"
    "    add    $v0, $v0, $v0\n"
    "    add    $t0, $v0, $v0\n"
    "    add    $s3, $s3, $t0\n"
    "    add    $t0, $t0, $v0\n"
    "    add    $s1, $s1, $t0\n"
    ".L800545B8:\n"
    "    lh     $v0, 0($s1)\n"
    "    nop\n"
    "    addi   $t0, $v0, 1\n"
    "    bnez   $t0, .L80054490\n"
    "    addi   $s1, $s1, 2\n"
    "    addu   $v0, $s1, $zero\n"
    "    lw     $ra, 40($sp)\n"
    "    lw     $fp, 36($sp)\n"
    "    lw     $s5, 32($sp)\n"
    "    lw     $s4, 28($sp)\n"
    "    lw     $s3, 24($sp)\n"
    "    lw     $s2, 20($sp)\n"
    "    lw     $s1, 16($sp)\n"
    "    jr     $ra\n"
    "    addiu  $sp, $sp, 0x38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel myRobGeneiOpen\n"
    "    lui  $t0,19584\n"
    "alabel D_800545F8\n"
    "    ori  $t0,$t0,32896\n"
    "alabel D_800545FC\n"
    "    lui  $t0,18560\n"
    "alabel D_80054600\n"
    "    ori  $t0,$t0,32896\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
void func_80054604(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80054604.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)arg4; (void)arg5; (void)arg6;
}
extern s16 InfoPosYTbl1[];
extern void func_80054604(s32, s32, s32, s32, s32, s32, s32);
void func_80054884(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7) {
    func_80054604(InfoPosYTbl1[a0] + a1 - 0x131, a2, a3, a4, a5, a6, a7);
}
void gpu_DrawSync(s32);
void func_8004659C(s32);
void snd_StopSelection(void);
void katinuki_game_setData_800548DC(void) {
    gpu_DrawSync(0);
    func_8004659C(-1);
    snd_StopSelection();
}
s32 func_8005490C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005490C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern u32 D_80102C00;
extern u16 D_800A38D6;
extern s32 D_800A374C;
extern s32 D_800A3808;
extern s32 D_800A378C;
extern s32 func_8005490C(void);
extern void leaf_muki_awase_800444E0(void);
s32 func_80054F68(void) {
    s32 v3;
    s32 s0;
    D_800A3820 = &D_80102C00;
    v3 = D_800A374C;
    D_800A38D6 = D_800A38D6 + 1;
    D_800A3808 = v3;
    D_800A378C = v3 + 0x10;
    s0 = func_8005490C();
    leaf_muki_awase_800444E0();
    return s0;
}
extern s32 D_800EFB14;
extern s32 D_800EFB18;
extern s32 D_800EFB1C;
extern s32 D_800EFB20;
extern s32 D_800EFB24;
extern s32 D_800EFB28;
void func_80054FDC(s32 a0) {
    s32 *p = &D_800EFB14;
    *p = a0 + *p;
    D_800EFB18 = a0 + D_800EFB18;
    if (D_800EFB1C) {
        D_800EFB1C = a0 + D_800EFB1C;
    }
    if (D_800EFB20) {
        D_800EFB20 = a0 + D_800EFB20;
    }
    if (D_800EFB24) {
        D_800EFB24 = a0 + D_800EFB24;
    }
    if (D_800EFB28) {
        D_800EFB28 = a0 + D_800EFB28;
    }
}
extern s32 D_800EFB0C;
s32* func_8005507C(void) {
    return &D_800EFB0C;
}
extern s32 D_80101E1C;
s32* func_8005508C(void) {
    return &D_80101E1C;
}
void func_8005509C(s32 arg0) {
    s32 i;
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    i = 0;
    do {
        *(p + 0x415) = 0;
        *(p + 0x414) = 0;
        p += 2;
    } while (++i < 8);
}
void func_800550E8(s32 arg0) {
    s32 i;
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    i = 0;
    do {
        i += 1;
        *(p + 0x415) = *(p + 0x415) >> 1;
        p += 2;
    } while (i < 8);
}
void single_game_SetStatusUpData(s32 arg0, s32 arg1, s32 arg2) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/single_game_SetStatusUpData.s).
     * 516 inst, 57 branches, 5 jal, 63 loads, 72 stores. m2c output had
     * type errors. Pure-C decomp pending. */
    (void)arg0; (void)arg1; (void)arg2;
}
extern u16 D_80099D88;

s32 func_80055948(u8 *arg0) {
    register u8 *p asm("a2");
    u8 ctr;
    s32 t;
    s32 mask;
    s32 dx, dy, dz;
    register s32 sentinel asm("t0");

    ctr = arg0[0x3B8];
    p = *(u8 **)(arg0 + 0x3B4);
    if (ctr != 0) {
        if (*(s16 *)(arg0 + 0x46) == 0) {
            arg0[0x3B8] = ctr - 1;
        }
        goto ret_3c8;
    }
    if (arg0[0x3BC] != 0) {
        goto check_loop;
    }
    {
        u8 idx = arg0[0x443];
        if (idx == 22) goto check_loop;
        if ((*(&D_80099D88 + idx * 12) & 0xBF00) != 0) goto check_loop;
        {
            s32 limit;
            if (*(s32 *)(arg0 + 0x430) & 0x200) {
                limit = (*(s16 *)(arg0 + 0x43C) < 0x801);
            } else {
                limit = (*(s16 *)(arg0 + 0x43C) < 0x401);
            }
            if (limit == 0) goto reset_ret_neg1;
        }
        if (*(s32 *)(arg0 + 0x430) & 0x800) goto reset_ret_neg1;
        if ((u32)(arg0[0x425] - 1) < 2U) goto reset_ret_neg1;
        if (arg0[0x442] != 0) goto reset_ret_neg1;
        {
            u8 *other = *(u8 **)arg0;
            if (*(u16 *)(other + 0x6A) == 0x2D) goto reset_ret_neg1;
            dx = *(s32 *)(other + 0xF4) - *(s16 *)(arg0 + 0x40E);
            dy = *(s32 *)(other + 0xFC) - *(s16 *)(arg0 + 0x410);
            dz = *(s16 *)(arg0 + 0x412);
            sentinel = 0x80;
            if ((dz * dz) >= ((dx * dx) + (dy * dy))) goto loop_init;
        }
    }
    goto reset_ret_neg1;
check_loop:
    if (arg0[0x3BC] != 1) goto loop_init;
    if (*(s32 *)(arg0 + 0x430) & 0x800) {
        sentinel = 0x80;
        goto loop;
    }
reset_ret_neg1:
    *(s32 *)(arg0 + 0x3B4) = 0;
    return -1;
sentinel_reset:
    *(s32 *)(arg0 + 0x3B4) = 0;
    goto ret_3c8;
loop_init:
    sentinel = 0x80;
loop:
    while (1) {
        t = *p;
        p += 1;
        if (t & 0x80) {
            if (t == sentinel) goto sentinel_reset;
            *(s32 *)(arg0 + 0x3B4) = (s32)p;
            arg0[0x3B8] = (t & 0x7F) - 1;
            goto ret_3c8;
        }
        mask = 1 << (t & 0xF);
        if (t & 0x10) {
            *(s32 *)(arg0 + 0x3C8) &= ~mask;
        } else {
            *(s32 *)(arg0 + 0x3C8) |= mask;
        }
    }
ret_3c8:
    return *(s32 *)(arg0 + 0x3C8);
}
void func_80055B44(u8 *a0, s32 a1, s32 a2, s32 a3) {
    *(s32 *)(a0 + 0x3B4) = a1;
    a0[0x3BC] = (u8)a2;
    a0[0x3B8] = (u8)a3;
    *(s32 *)(a0 + 0x3C8) = 0;
    *(s32 *)(a0 + 0x3CC) = -1;
}
void calc_loc_mat_fw_80055B60(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/calc_loc_mat_fw_80055B60.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_80056CB8(s32 arg0) {
    /* Bind locals to specific callee-save regs to match target's allocation. */
    register s32 r_arg0 asm("$23") = arg0;     /* $s7 */
    register s32 var_s6 asm("$22");            /* $s6 - used by asmfix-slice */
    s32 var_fp;                                 /* GCC picks; clobber forces save */
    /* Target struct base lands at sp+0x18 (8-byte param-save gap above sp+0x10). */
    struct {
        s32 sp18, sp1C, sp20, _g0;
        s32 sp28, sp2C, sp30, _g1;
        s32 sp38, sp3C, sp40, _g2;
        s32 sp48, sp4C, sp50, _g3;
        s32 sp58, sp5C, sp60, _g4;
        s32 *sp68;
        s32 _g5;
        s32 *sp70;
        s32 _g6;
        s32 sp78;
    } f;
    s32 var_s0_2;
    s16 *p_pos1, *p_pos2;
    s32 angle_val;
    s32 var_s1;
    s32 var_s2, var_s3;
    s32 temp_s0;
    s32 temp_v1_3;
    u16 obj_type;
    s32 r1, r2;
#define sp18 f.sp18
#define sp1C f.sp1C
#define sp20 f.sp20
#define sp28 f.sp28
#define sp2C f.sp2C
#define sp30 f.sp30
#define sp38 f.sp38
#define sp3C f.sp3C
#define sp40 f.sp40
#define sp48 f.sp48
#define sp4C f.sp4C
#define sp50 f.sp50
#define sp58 f.sp58
#define sp5C f.sp5C
#define sp60 f.sp60
#define sp68 f.sp68
#define sp70 f.sp70
#define sp78 f.sp78
    {
        register s32 _low2 asm("$3");
        s32 _guard;
        s32 _hi3e8 = *(u16 *)(r_arg0 + 0x3E8);
        __asm__("andi %0,%1,0x3" : "=r"(_low2) : "r"(_hi3e8));
        var_s6 = _low2 << 1;
        __asm__ __volatile__("addiu %0,$0,1" : "=r"(_guard));
        if (_guard != 0) {
            sp60 = var_s6;
            sp68 = (s32 *)&sp28;
            sp70 = (s32 *)&sp58;
            sp78 = 0x1F8002B8;
            var_fp = _low2 << 2;
            do {
                /* Body replaced wholesale by asmfix-slice. Stub keeps GCC saving
                 * callee-saves and allocating struct stack slots. */
                __asm__ volatile ("" : : "r"(var_fp) : "$16","$17","$18","$19","$20","$21","memory");
                sp18 = 0; sp1C = 0; sp20 = 0;
                sp28 = 0; sp2C = 0; sp30 = 0;
                sp38 = 0; sp3C = 0; sp40 = 0;
                sp48 = 0; sp4C = 0; sp50 = 0;
                sp58 = 0; sp5C = 0;
                camera_check_inside_screen_rob_dpos((s32 *)&sp18, sp68, (s32)&sp38, (s32)sp70, var_fp);
                camera_check_inside_screen_rob_dpos((s32 *)&sp18, sp68, (s32)&sp48, (s32)sp70, sp78);
                *(s8 *)(r_arg0 + 0x444 + var_s6) = 0;
                var_s6 += 1;
                var_fp += 2;
            } while (var_s6 < sp60 + 2);
        }
    }
}
#undef sp18
#undef sp1C
#undef sp20
#undef sp28
#undef sp2C
#undef sp30
#undef sp38
#undef sp3C
#undef sp40
#undef sp48
#undef sp4C
#undef sp50
#undef sp58
#undef sp5C
#undef sp60
#undef sp68
#undef sp70
#undef sp78
extern u8 D_8009A830;
extern s8 D_8009A838;
extern u8 D_8009A840;
s32 ang_hosei_80056FE8(s32 arg0) {
    s32 a2 = *((s32 *) arg0);
    s32 a3 = *((u8 *) ((*((s32 *) (a2 + 0x58))) + 3));
    s32 base = a3 * 40;
    s32 var_v0;
    if ((*((u8 *) (a2 + 0xA3))) != 0xFF) {
        if ((var_v0 = *((s16 *) (arg0 + 0x5E))) == 0) {
            var_v0 = (*((u8 *) (((s32) (&D_8009A830)) + (*((s16 *) (a2 + 0xE)))))) * 2;
        } else {
            var_v0 = (*((s8 *) (((s32) (&D_8009A838)) + (*((s16 *) (a2 + 0xE)))))) * 8;
        }
    } else {
        var_v0 = (*((u8 *) (((s32) (&D_8009A840)) + (*((s16 *) (a2 + 0x14)))))) * 2;
    }
    {
        register s32 partial asm("$5") = base + var_v0;
        asm volatile("" : : "r"(partial));
        return partial + (*((s16 *) ((*((s32 *) arg0)) + 0x40A))) + 0x12C;
    }
}
extern s32 single_game_getEnemyCharId(s32, s32);
extern s32 func_800233AC(void *, s32 *);
extern s32 D_8009AA50[];

s32 func_80057094(void *arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 sp10;
    s32 temp_s0;
    s32 temp_v0;
    s32 temp_v1;
    s32 var_v0;

    temp_s0 = single_game_getEnemyCharId(D_800F6608 - *(s32 *)((s32)arg0 + 0xF4), D_800F6610 - *(s32 *)((s32)arg0 + 0xFC));
    var_v0 = temp_s0 - single_game_getEnemyCharId(arg1 - *(s32 *)((s32)arg0 + 0xF4), arg2 - *(s32 *)((s32)arg0 + 0xFC));
    var_v0 -= 0x100;
    temp_v0 = (s32)var_v0 >> 9;
    temp_v1 = temp_v0 & 7;
    sp10 = temp_v1;
    if ((arg3 == 0) && !(temp_v0 & 1)) {
        if (*(u16 *)((s32)arg0 + 0x3E8) & 0x10) {
            sp10 = temp_v1 + 1;
            if (sp10 >= 8) {
                sp10 = 0;
            }
        } else {
            sp10 = temp_v1 - 1;
            if (sp10 < 0) {
                sp10 = 7;
            }
        }
    }
    var_v0 = D_8009AA50[sp10 & 7];
    if (arg3 == 1) {
        var_v0 |= 4;
    }
    if (func_800233AC(arg0, &sp10) != 0) {
        var_v0 |= 8;
    }
    return var_v0;
}
void func_800571C0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800571C0.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8005763C(s32 arg0, s32 arg1, s32 arg2, s32 arg3,
                   s32 arg4, s32 arg5, s32 arg6, s32 arg7,
                   s32 *arg8, s32 *arg9) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005763C.s).
     * Pure C decomp: line/segment intersection (clip-to-rect) routine,
     * 66 branches, leaf function. Pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    (void)arg4; (void)arg5; (void)arg6; (void)arg7;
    (void)arg8; (void)arg9;
}
void func_80057ACC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80057ACC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 single_game_getEnemyCharId(s32, s32);
extern s16 Judge;
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    register unsigned short next_idx asm("s3");
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    p = (s16 *)((s32)table + (((s32)(prev_idx << 16) >> 16) << 2));
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((s32)(*(s16 **)(arg0 + 4)) + (((s32)(next_idx << 16) >> 16) << 2));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}

void func_80057E84(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80057E84.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_80058580(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80058580.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s16 D_800A3400;
extern s32 D_800A3408;
extern s32 D_800EFB38;
extern s32 D_800EFB78;
extern s32 D_800EFB7C;
extern s32 D_800EFC38;
extern void DispStuff(void);
extern s32 SetBloodSpot(s32);
extern s32 memcard_SetSlot(s32);
extern s32 sys_Shutdown(void);
extern s32 title_mv_exec2(s32);
extern s32 func_80085E4C(s32, s32);
extern s32 func_80085EE4(s32);
extern s32 func_80085F98(void);
void func_8005B43C(void) {
    s32 *p1;
    s32 *p2;
    s32 i;
    u8 *q;
    s32 j;

    i = 0;
    p1 = &D_800EFB38;
    p2 = &D_800EFC38;
    do {
        *p2 = 0;
        *p1 = 0;
        p1 += 1;
        i += 1;
        p2 += 1;
    } while (i < 0x10);
    sys_Shutdown();
    title_mv_exec2(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    memcard_SetSlot(0);
    SetBloodSpot(1);
    {
        register s32 v asm("$5") = 0x7F;
        q = (u8 *)&D_800EFB78;
        j = 0;
        do {
            *(s32 *)((u8 *)&D_800EFB78 + j) = 0;
            *(s8 *)(q + 5) = v;
            *(s8 *)((u8 *)&D_800EFB7C + j) = v;
            j += 8;
            q += 8;
        } while (j < 0xC0);
    }
    DispStuff();
    D_800A3408 = 0;
    D_800A3400 = 0;
}
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void irq_ProcessPending(void);
void spu_Reset(void);
extern s32 D_800EFB38[];
extern s32 D_800EFC38[];
extern s32 D_800A3408;
void get_point_value(void) {
    s32 i;
    s32 *a0;
    s32 *v1;
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    irq_ProcessPending();
    spu_Reset();
    i = 0;
    a0 = D_800EFB38;
    v1 = D_800EFC38;
    do {
        *v1 = 0;
        *a0 = 0;
        a0++;
        i++;
        v1++;
    } while (i < 0x10);
    D_800A3408 = 0;
}

void func_800858D0(s32);
void func_8005B58C(void) {
    func_800858D0(0);
}
extern void func_800858D0(s32);
extern void func_80086130(s32, s32, s32);
extern u32 D_800EFB78[];
extern u8 D_800EFB7C[];
void obj_InitChars(void) {
    s32 s1;
    s32 s3;
    u8 *s2;
    s32 s0;
    func_800858D0(0);
    s1 = 0;
    s3 = 0x7F;
    s2 = (u8 *)D_800EFB78;
    s0 = 0;
    do {
        *(u32 *)((u8 *)D_800EFB78 + s0) = 0;
        s2[5] = s3;
        *((u8 *)D_800EFB7C + s0) = s3;
        func_80086130((s16)s1, 0, 0);
        s2 += 8;
        s1++;
        s0 += 8;
    } while (s1 < 24);
}
void func_800858D0(s32);
void func_80087F64(s32);
extern s32 D_800EFC38;
extern s32 D_80015470;
extern s16 Judge;
extern u8 D_80099BCC;
extern u8 D_80099CC8;
extern u8 D_80099CC9;
extern u16 D_80099D88;
extern u8 D_80099D8B;
extern u8 D_80099D8C;
extern u8 D_80099D8D;
extern u8 D_80099D8E;
extern u8 D_80099D8F;
extern u8 D_80099D94;
extern u8 D_80099D9C;
extern u8 D_80099D9D;
extern u8 D_8009A088;
extern u8 D_8009A820;
extern u8 D_8009A821;
extern u8 D_8009A830;
extern s8 D_8009A838;
extern u8 D_8009A840;
extern u8 D_8009A850;
extern u8 D_8009A851;
extern u8 D_8009A852;
extern u8 D_8009A853;
extern u16 D_8009A8CA;
extern s32 D_8009AA50[];
extern u8 D_8009AD18;
extern u8 D_8009B14E;
extern s16 D_8009B16C;
extern s16 D_8009B17C;
extern s16 D_8009B18C;
extern s16 D_8009B2BC;
extern s16 D_8009B2BE;
extern s16 D_8009B2C4;
extern u16 D_8009B450;
extern u16 D_8009B452;
extern u8 D_8009B48E;
extern u8 D_8009B58C;
extern u8 D_8009BA60;
extern s32 chractar_use_pset_combo_id_table;
extern s32 D_8009BC04;
extern s32 D_8009BC08;
extern u8 D_8009BC0C;
extern u8 D_8009BC0D;
extern s32 D_8009BC1C;
extern u8 D_8009BC38;
extern u8 D_8009BC40;
extern u8 D_8009BC41;
extern u8 D_8009BC44;
extern u8 D_8009BC72;
extern u8 D_8009BC76;
extern u8 D_8009BC7C;
extern s16 D_8009BC94;
extern s16 D_8009BC96;
extern u16 D_8009BCC4;
extern u16 D_8009BCC6;
extern s16 D_8009BCD0;
extern s16 D_8009BCD2;
extern u8 D_8009BCE4;
extern u8 D_8009BCF8;
extern u8 D_8009BCF9;
extern u8 D_8009BD20;
extern u8 D_8009BD21;
extern s32 D_8009BD38;
extern u8 D_8009BD3C;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern s32 D_8009BD44;
extern u8 D_8009BD58;
extern u8 D_8009BD59;
extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;
extern s32 D_8009BD84;
extern s32 D_8009BD88;
extern u8 D_800A3270;
extern s32 D_800A32C8;
extern s16 D_800A33E8;
extern s16 D_800A3438;
extern s32 D_800A344C;
extern s16 D_800A34E8;
extern s16 D_800A3530;
extern s16 D_800A3534;
extern s16 D_800A3540;
extern s16 D_800A3544;
extern u8 D_800A3560;
extern u8 D_800A3561;
extern u8 D_800A3562;
extern s16 D_800A3588;
extern s16 D_800A358C;
extern s16 D_800A3590;
extern s16 D_800A3594;
extern s32 D_800A3618;
extern s32 D_800A3628;
extern s32 D_800A362C;
extern s32 D_800A3638;
extern s32 D_800A3708;
extern s32 D_800A370C;
extern s32 D_800A3828;
extern s32 D_800A38B4;
extern s32 D_800A38D8;
extern s16 D_800EF980[];
extern s16 D_800EF9F4;
extern s32 D_800EFC44;
extern s32 D_800EFC50;
extern u16 D_800EFC8A;
extern s16 D_800F0B78;
extern s16 D_800F0B7C;
extern s16 D_800F0B98;
extern s16 D_800F0BA4;
extern s16 D_800F0BB2;
extern s16 D_800F0BCC;
extern s16 D_800F0BEC;
extern s32 D_800F0C10;
extern s32 D_800F0C14;
extern s32 D_800F0C18;
extern s32 D_800F0CA0;
extern s32 D_800F0CA4;
extern s32 D_800F0CA8;
extern s32 D_800F0CAC;
extern s32 D_800F0CB0;
extern s32 D_800F0CB4;
extern s32 D_800F0CB8;
extern s32 D_800F0CBC;
extern s32 D_800F0CC0;
extern s32 D_800F0CC4;
extern s32 D_800F0CC8;
extern s32 D_800F0CCC;
extern s32 D_800F0CD0;
extern s32 D_800F0CD4;
extern s32 D_800F0CD8;
extern s32 D_800F0CDC;
extern s32 D_800F0CE0;
extern s32 D_800F0CE4;
extern s32 D_800F0CE8;
extern s32 D_800F0CEC;
extern s32 D_800F0CF0;
extern s32 D_800F0CF4;
extern s32 D_800F0CF8;
extern s32 D_800F0CFC;
extern s32 D_800F0D18;
extern s32 D_800F0D1C;
extern s32 D_800F0D20;
extern s32 D_800F0D24;
extern s32 D_800F0D28;
extern s32 D_800F0D2C;
extern s32 D_800F0D30;
extern s32 D_800F0D34;
extern s32 D_800F0D38;
extern s32 D_800F0D3C;
extern s32 D_800F0D40;
extern s32 D_800F0D44;
extern s32 D_800F0D48;
extern s32 D_800F0D4C;
extern s32 D_800F0D50;
extern s32 D_800F0D54;
extern s32 D_800F0D58;
extern s32 D_800F0D5C;
extern s32 D_800F0D60;
extern s32 D_800F0D64;
extern s32 D_800F0D68;
extern s32 D_800F0D6C;
extern s32 D_800F0D70;
extern s32 D_800F0D74;
extern s32 D_800F0D78;
extern s32 D_800F0D7C;
extern s32 videoDec;
extern s32 D_800F0E38;
extern s32 D_800F0E3C;
extern s32 D_800F0E40;
extern s32 D_800F0EC8;
extern s32 D_800F0ECC;
extern s32 D_800F0ED0;
extern s32 D_800F0FB8;
extern s32 D_800F0FBC;
extern s32 D_800F0FC0;
extern s16 D_800F1000;
extern s16 D_800F1002;
extern s16 D_800F1004;
extern s16 D_800F10A0;
extern s16 D_800F10A2;
extern s16 D_800F10A4;
extern s32 D_800F10D0;
extern s32 D_800F10D4;
extern s32 D_800F10E0;
extern s32 D_800F10E4;
extern s32 D_800F10E8;
extern s32 D_800F10EC;
extern s32 D_800F10F0;
extern s32 D_800F10F4;
extern s32 D_800F10F8;
extern s32 D_800F10FC;
extern s32 D_800F1100;
extern s32 D_800F1104;
extern s32 D_800F1108;
extern s32 D_800F110C;
extern s32 D_800F1110;
extern s32 D_800F1114;
extern s32 D_800F1118;
extern s32 D_800F111C;
extern s32 D_800F1138;
extern s32 D_800F1140;
extern s32 D_800F1144;
extern s32 D_800F1148;
extern u8 D_800F1150;
extern s32 D_800F1178;
extern s32 D_800F117C;
extern s32 D_800F1180;
extern s32 D_800F1198;
extern s32 D_800F119C;
extern s32 D_800F11A0;
extern s32 D_800F1850;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern s32 g_anim_func_table;
extern u8 D_800F74A4;
extern u8 D_800F74A5;
extern u8 D_800FB534;
extern u8 D_800FB535;
extern s16 D_800FF558;
extern s16 D_800FF55A;
extern s16 D_800FF55C;
extern s16 D_800FF55E;
extern s16 D_800FF560;
extern s16 D_800FF562;
extern s16 D_800FF564;
extern s16 D_800FF566;
extern s16 D_800FF568;
extern s32 D_800FF570;
extern s32 D_800FF574;
extern u8 D_800FF639;
extern s16 D_800FF640;
extern s32 D_800FF644;
extern s16 D_800FF648;
extern s16 D_800FF64A;
extern s16 D_800FF64C;
extern s32 D_800FF684;
extern s32 D_800FF688;
extern s32 D_800FF68C;
extern u8 D_80101DF1;
extern s16 D_80101DF8;
extern s32 D_80101DFC;
extern u16 D_80101E02;
extern u16 D_80101E04;
extern s32 D_80101E3C;
extern s32 D_80101E40;
extern s32 D_80101E44;
extern s32 D_80103624;
extern s32 D_800EFB38;
void GetAllocPacketSize(s32 a0) {
    s32 v;
    func_800858D0(0);
    v = a0 * 2 + a0 + 1;
    func_80087F64((s16)v);
    *(s32*)((u8*)&D_800EFC38 + (v * 4)) = 0;
    *(s32*)((u8*)&D_800EFB38 + (v * 4)) = 0;
}
extern s32 D_800EFC40;
extern s32 D_800EFB40;
extern s32 D_800EFC4C;
extern s32 D_800EFB4C;
void func_800858D0(s32);
void func_80087F64(s32);
void func_8005B6AC(void) {
    func_800858D0(0);
    func_80087F64(2);
    D_800EFC40 = 0;
    D_800EFB40 = 0;
    func_80087F64(5);
    D_800EFC4C = 0;
    D_800EFB4C = 0;
}
extern s32 D_800EFC3C;
extern s32 D_800EFB3C;
void func_80087F64(s32);
void func_8005B6FC(void) {
    func_80087F64(1);
    D_800EFC3C = 0;
    D_800EFB3C = 0;
}
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void func_80087F64(s16);
void obj_InitChars(void);
extern s32 D_800EFB3C[];
extern s32 D_800EFC3C[];
extern s32 D_800A3408;
void obj_InitAll(void) {
    s32 s0;
    s32 *s2;
    s32 *s1;
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
    s2 = D_800EFB3C;
    s1 = D_800EFC3C;
    for (s0 = 1; s0 < 0x10; s0++) {
        func_80087F64((s16)s0);
        *s1 = 0;
        *s2 = 0;
        s2++;
        s1++;
    }
    D_800A3408 = 0;
    obj_InitChars();
}

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
extern s32 func_80036EA8();
extern s32 func_80036F28();
extern s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);

s32 debug_printf(s32 *, s32);               /* extern */
s32 game_FrameLoop();                           /* extern */
s32 replay_camera_Init(s32, s32);               /* extern */
s32 title_mv_exec2(s32);                    /* extern */
extern s32 D_800158B4;
extern s32 D_800A3404;
extern s32 D_800A3408;
extern s32 D_800A340C;

void func_8005B7C4(s32 arg0) {
    s32 temp_v0;
    u32 temp_s0;

    title_mv_exec2(0);
    debug_printf(&D_800158B4, arg0);
    game_FrameLoop();
    temp_v0 = func_80036EA8(2, 1);
    replay_camera_Init(temp_v0, arg0);
    temp_s0 = func_80036F28(temp_v0);
    game_FrameLoop();
    D_800A3408 = 0;
    D_800A340C = 0x1010;
    D_800A3404 = 0x1010;
    tslGlobalMemFree_8005C2A8((GameObj *) arg0, 0, arg0 + temp_s0);
    D_800A340C = D_800A3404;
}
extern s32 D_800EFC58;
extern s32 D_800EFB58;
extern s32 D_800EFC48;
extern s32 D_800EFB48;
void func_800858D0(s32);
void func_80087F64(s32);
void obj_InitPair(void) {
    func_800858D0(0);
    func_80087F64(8);
    D_800EFC58 = 0;
    D_800EFB58 = 0;
    func_80087F64(4);
    D_800EFC48 = 0;
    D_800EFB48 = 0;
}
extern s32 func_80036EA8(s32, s32);
extern s32 func_80036F28(s32);
extern s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);
extern void obj_InitPair(void);
extern void func_800858D0(s32);
extern void replay_camera_Init(s32, s32);
s32 func_8005B8B8(s32 arg0) {
    s32 t0, t1, ret_a, t0_2, t1_2;

    obj_InitPair();
    func_800858D0(0);
    t0 = func_80036EA8(2, 0x5D);
    game_FrameLoop();
    replay_camera_Init(t0, arg0);
    t1 = func_80036F28(t0);
    game_FrameLoop();
    asm volatile("" : "=r"(t0) : "0"(t0));
    ret_a = tslGlobalMemFree_8005C2A8(arg0, 8, arg0 + t1);
    t0_2 = func_80036EA8(2, 0x5E);
    game_FrameLoop();
    replay_camera_Init(t0_2, arg0 + ret_a);
    t1_2 = func_80036F28(t0_2) + ret_a;
    game_FrameLoop();
    return tslGlobalMemFree_8005C2A8(arg0 + ret_a, 4, arg0 + t1_2) + ret_a;
}
void saFidLoad(s32, s32);
void motion_LoadPreCalcData_8005B98C(s32 a0) {
    saFidLoad(a0, 8);
    saFidLoad(a0, 4);
}
extern s32 D_800EFC5C;
extern s32 D_800EFB5C;
void func_800858D0(s32);
void func_80087F64(s32);
void obj_InitTask(void) {
    func_800858D0(0);
    func_80087F64(9);
    D_800EFC5C = 0;
    D_800EFB5C = 0;
}
void obj_InitTask(void);
s32 func_80036EA8(s32, s32);
s32 game_FrameLoop(void);
void replay_camera_Init(s32, s32);
s32 func_80036F28(s32);
s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);
void obj_InitTaskCamera(s32 a0) {
    s32 s1;
    obj_InitTask();
    s1 = func_80036EA8(2, 8);
    game_FrameLoop();
    replay_camera_Init(s1, a0);
    s1 = func_80036F28(s1);
    game_FrameLoop();
    tslGlobalMemFree_8005C2A8(a0, 9, a0 + s1);
}
void saFidLoad(s32, s32);
void obj_ExecTask(s32 a0) {
    saFidLoad(a0, 9);
}
s32 func_8005BA8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005BA8C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern void title_mv_exec2(s32);
extern void saFidLoad(s32, s32);
extern s32 D_800EFC50;
extern s32 D_800EFC44;
extern u8 D_8009AD18;
void func_8005BD30(s32 arg0) {
    u8 count;
    s32 i;
    title_mv_exec2(0);
    count = (D_800EFC50 == D_800EFC44) ? 2 : 3;
    if (count != 0) {
        i = 0;
        do {
            u8 byte = (&D_8009AD18)[i & 0xFF];
            saFidLoad(arg0, byte);
            i += 1;
        } while ((u32)(i & 0xFF) < (u32)count);
    }
    if (count == 2) {
        D_800EFC50 = D_800EFC44;
    }
}
extern u32 D_800EFC38[];
extern u32 D_800EFB38[];
extern u8 D_8009AD18[];
extern void func_80087F64(s32);
void func_8005BDF0(void) {
    u32 *s3 = D_800EFC38;
    u32 *s2 = D_800EFB38;
    u8 *s0 = D_8009AD18;
    u8 *s1 = (u8 *)((s32)s0 + 3);
    do {
        func_80087F64(*s0);
        s3[*s0] = 0;
        s2[*s0] = 0;
        s0++;
    } while ((s32)s0 < (s32)s1);
}
extern s32 D_8009AD1C;
extern s32 title_mv_exec2(s32);
extern s32 func_80085F98();
extern s32 func_80085EE4(s16);
extern s32 func_80085E4C(s16, s16);
extern s32 func_80085FB8();
extern s32 md_game_check_change_main_mode_katinuki(s16);
s32 mario_getMarioVoiceData_8005BE84(s32 arg0) {
    register s32 arg_save asm("$16") = arg0;
    s32 result;
    s16 *p;
    title_mv_exec2(0);
    {
        register s32 base asm("$3");
        base = (s32)&D_8009AD1C;
        p = (s16 *)(base + (arg_save * 4));
    }
    if (*p >= 0) {
        s16 temp_a0;
        arg_save = arg_save << 1;
        func_80085F98();
        func_80085EE4(0);
        func_80085E4C(0, 0);
        result = func_80085EE4(*p);
        md_game_check_change_main_mode_katinuki(*p);
        temp_a0 = arg_save + 1;
        func_80085E4C(temp_a0, temp_a0);
        func_80085FB8();
    } else {
        result = -1;
    }
    return (s16)result;
}
void func_800858D0(s32);
void func_80085F98(void);
void func_80085EE4(s32);
void func_80085E4C(s32, s32);
void obj_Reset(void) {
    func_800858D0(0);
    func_80085F98();
    func_80085EE4(0);
    func_80085E4C(0, 0);
}
extern s32 func_80087F64();
extern s32 tslCDFileRead();
extern s32 coli_CheckBukiPreHit_800880B8();
extern s32 func_8008AD64();
extern s32 func_8008ADC4();
extern s32 func_8008AE24();
extern s32 func_8008AEB0();
extern void func_800858D0(s32);
extern s32 *D_800EFC38[];
extern s32 D_800EFB38[];
s32 func_8005BF78(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    func_800858D0(0);
    func_80087F64((s16) arg1);
    func_8008AE24(arg3);
    func_8008AD64(arg0, D_800EFC38[arg1][3]);
    func_8008AEB0(1);
    func_8008AE24(arg2);
    func_8008ADC4(arg0, D_800EFC38[arg1][3]);
    func_8008AEB0(1);
    coli_CheckBukiPreHit_800880B8(D_800EFC38[arg1][1], (s16) arg1, arg2);
    tslCDFileRead((s16) arg1);
    D_800EFB38[arg1] = arg2;
    return arg2 + D_800EFC38[arg1][3];
}
void func_8005C074(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005C074.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
s32 tslGlobalMemFree_8005C2A8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/tslGlobalMemFree_8005C2A8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern s32 D_800EFC38;
extern s32 func_80087F64(s32);
extern s16 coli_CheckBukiPreHit_800880B8(s32, s32, s32);
extern s16 tslCDFileRead(s16);
s32 saFidLoad(s32 arg0, s16 arg1) {
    s32 **p;
    s32 *v;
    s32 *vv;
    s16 ret;
    title_mv_exec2(0);
    p = (s32 **)((u8 *)&D_800EFC38 + arg1 * 4);
    v = *p;
    if (v != 0) {
        v = (s32 *)((u8 *)v + arg0);
        *p = v;
        *v = *v + arg0;
        vv = *(volatile s32 **)p;
        *(s32 *)((u8 *)vv + 4) = *(s32 *)((u8 *)vv + 4) + arg0;
        func_80087F64(arg1);
        ret = coli_CheckBukiPreHit_800880B8(*(s32 *)((u8 *)*p + 4), arg1, *(s32 *)((u8 *)&D_800EFB38 + arg1 * 4));
        if (ret == arg1) {
            return (s16)tslCDFileRead(ret);
        }
        return ret;
    }
    return (s16)-1;
}
extern s32 D_800A3404;
void func_80087F64(s32);
void coli_CheckBukiPreHit_80088088(s32, s16, s32);
s32 func_800884C4(s32, s16);
s16 func_8005C5A8(s32 *a0, s16 a1) {
    s32 *s1 = a0;
    s16 s0 = a1;
    func_80087F64(s0);
    coli_CheckBukiPreHit_80088088(s1[1], s0, D_800A3404);
    *(s32 *)(s1[1] + 8) = s0;
    return (s16)func_800884C4(s1[2], s0);
}
void func_80083BE4(s32, s32);
void func_800858D0(s32);
void memcard_ClearBusy(void);
void memcard_SetData(s32);
void func_8005C614(void) {
    func_80083BE4(0x7F, 0x7F);
    func_800858D0(0);
    memcard_ClearBusy();
    memcard_SetData(0);
}
extern s32 D_8009AA70;
extern s32 D_800EFB78;
extern u8 D_800EFB7C;
extern u8 D_800EFB7D;
void func_8005C650(s32 a0, s32 a1, s32 a2) {
    s16 a3 = 0;
    s32 *base = (s32 *)((u8 *)&D_8009AA70 + a0 * 4);
    do {
        s32 off = a3 * 8;
        if (!*(s32 *)((u8 *)&D_800EFB78 + off)) {
            *(s32 *)((u8 *)&D_800EFB78 + off) = (s32)base;
            *((u8 *)&D_800EFB7C + off) = (u8)a1;
            *((u8 *)&D_800EFB7D + off) = (u8)a2;
            return;
        }
        a3 = (s16)(a3 + 1);
    } while ((s16)a3 < 0x18);
}
s32 func_8005C6D0(void) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005C6D0.s).
     * Pure-C decomp pending future purification work. */
    return 0;
}
s32 func_8005C8A8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005C8A8.s).
     * 753 inst, 69 branches, 29 jal, 80 loads, 162 stores. m2c output had
     * duplicate sp variable declarations. Pure-C decomp pending. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern s32 func_80073728();
extern s32 D_8009B2C8;
extern s32 D_8009B340;
extern s32 D_8009B358;
typedef struct {
    void *p0;
    s32 *p1;
    s32 pad08;
    s32 ret;
    s32 zero10;
    s32 one14;
    s32 zero18;
    s32 zero1C;
    s32 c20;
    s32 c24;
    s8 byte28;
} S46C;
void func_8005D46C(s32 arg0, s32 arg1) {
    S46C s;
    s32 stride;
    s32 ret;
    s32 idx;
    idx = arg1;
    if (arg1 > 0) {
        idx = arg1 - 1;
    }
    stride = idx * 0x3C;
    s.byte28 = 0;
    s.p0 = (void *)((u8 *)(&D_8009B2C8) + stride);
    s.p1 = &D_8009B340;
    s.c24 = 0x100;
    s.c20 = 0x100;
    s.zero1C = 0;
    s.zero18 = 0;
    s.zero10 = 0;
    s.one14 = 1;
    s.ret = arg0;
    ret = func_80073728((GameObj *)(&s), 0);
    s.byte28 = 0;
    s.p0 = (void *)(((u8 *)(&D_8009B2C8) + stride) + 0xC);
    s.p1 = &D_8009B358;
    s.c24 = 0x100;
    s.c20 = 0x100;
    s.zero1C = 0;
    s.zero18 = 0;
    s.zero10 = 0;
    s.one14 = 1;
    s.ret = ret;
    func_80073728((GameObj *)(&s), 0);
}
s32 gnd_land_hit_char_tsuba(s32 arg0, s32 arg1) {
    extern s32 func_80079154(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;
    S46C s;
    s32 v3;
    s32 v0;
    s32 i;
    u32 r5;
    u32 r4;
    s32 ret;
    s32 stride;
    s32 a0_offset;
    s32 a2_offset;
    s32 c100;
    s32 c1;
    s32 *p_b388;
    u8 *p_b2e0;
    s32 *base_offset;

    v3 = D_800A326C;
    v0 = v3;
    if (v3 < 0) v0 = v3 + 3;
    D_800A326C -= (v0 >> 2) * 4;

    ret = arg0;
    if (arg1 > 0) arg1 -= 1;
    i = 0;

    D_800A3418 ^= func_80079154();
    r5 = ((u32)(D_800A3418 * 0x260)) >> 0xF;
    D_800A3418 ^= func_80079154();
    r4 = ((u32)(D_800A3418 * 0xDC)) >> 0xF;

    if (((D_800A326C + 1) * 2) > 0) {
        c100 = 0x100;
        c1 = 1;
        stride = arg1 * 0x3C;
        p_b2e0 = (u8 *)&D_8009B2E0;
        base_offset = (s32 *)(p_b2e0 + stride + 0xC);
        p_b388 = &D_8009B388;
        do {
            s.byte28 = 0;
            s.p0 = (void *)(p_b2e0 + stride);
            s.p1 = p_b388;
            D_800A3418 ^= func_80079154();
            i += 1;
            s.c24 = c100;
            s.c20 = c100;
            D_800A3418 ^= func_80079154();
            a0_offset = (s32)r5 - 0x19 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= func_80079154();
            a2_offset = (s32)r4 - 0xC + ((u32)(D_800A3418 * 0x19) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((GameObj *)&s, 0);

            s.byte28 = 0;
            s.c24 = c100;
            s.c20 = c100;
            s.p1 = &D_8009B390;
            s.p0 = (void *)((u8 *)base_offset + (D_800A3418 & 1) * 0xC);
            D_800A3418 ^= func_80079154();
            a0_offset = (s32)r5 - 0x32 + ((u32)(D_800A3418 * 0x64) >> 0xF);
            s.zero18 = a0_offset;
            D_800A3418 ^= func_80079154();
            a2_offset = (s32)r4 - 0x19 + ((u32)(D_800A3418 * 0x32) >> 0xF);
            s.zero10 = 0;
            s.one14 = c1;
            s.ret = ret;
            s.zero1C = a2_offset;
            ret = func_80073728((GameObj *)&s, 0);
        } while (i < ((D_800A326C + 1) * 2));
    }
    D_800A326C += 1;
    return ret;
}
s32 func_8005D814(s16 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005D814.s).
     * 545 inst, 46 branches, 12 jal, 45 loads, 86 stores. m2c output had
     * duplicate sp variable declarations. Pure-C decomp pending. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern s16 D_8009B488;
extern s8 D_8009B48E;

s32 func_8005E098(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005E098.s).
     * Pure C decomp pending. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
s32 func_8005E098(s32, s32, s32, s32);
s32 func_8005E51C(s32 a0, s32 a1, s32 a2) {
    return func_8005E098(-1, a0 - 1, a1, a2);
}
s32 func_8005E54C(s32 arg0, s32 arg1, s32 arg2) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005E54C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2;
    return 0;
}
void func_8005F1C8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005F1C8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_8009B610;
extern s32 D_8009B634;
extern s32 D_8009B63C;
extern s32 D_8009B660;
extern s32 D_8009B670;
extern s32 D_8009B678;
s32 func_8005FA98(s32 arg0, s32 arg1, s32 arg2) {
    S46C s;
    s32 ret;
    s32 start = arg1;
    s32 end = arg1 + 0x190;

    s.c20 = 0x200;
    s.c24 = 0x100;
    s.p0 = (void *)((u8 *)(&D_8009B63C) + (arg0 * 0xC));
    s.byte28 = 0;
    s.zero1C = 0;
    s.zero18 = 0;
    s.zero10 = 0;
    s.one14 = arg2;
    switch (arg0) {
    case 0:
        s.p1 = &D_8009B660;
        break;
    case 1:
        s.p1 = &D_8009B670;
        break;
    case 2:
        s.p1 = &D_8009B678;
        break;
    }
    s.ret = start;
    ret = func_80073728((GameObj *)(&s), 0);
    s.p0 = (void *)((u8 *)(&D_8009B610) + (arg0 * 0xC));
    s.p1 = &D_8009B634;
    s.ret = ret;
    func_80073728((GameObj *)(&s), 0);
    return end - arg1;
}
extern u8 D_800A327C[8];
extern u8 D_800A3284[8];
extern s32 D_800A3278;

void func_8005FBC8(s32 arg0, u8 *arg1) {
    u8 r1[8], r2[8];
    s32 s0;
    s0 = func_80036EA8(2, arg0 + 0x33);
    replay_camera_Init(s0, (s32)arg1);
    game_FrameLoop();
    func_80036F28(s0);
    __builtin_memcpy(r1, D_800A327C, 8);
    __builtin_memcpy(r2, D_800A3284, 8);
    gpu_LoadImage((s32)r1, (s32)(arg1 + 0x40));
    gpu_DrawSync(0);
    gpu_LoadImage((s32)r2, (s32)(arg1 + 0x14));
    gpu_DrawSync(0);
    D_800A3278 = 0;
}
extern s32 D_800A36AC;
extern s32 D_800A374C;
extern s32 D_800A3278;
extern s32 D_8009B698;
extern s32 D_8009B6B0;
extern s32 D_800F7438;
extern s32 initDrawArea(s32, u16 *);
extern s32 initPolyG4(s32);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 gpu_SetSemiTransp(s32, s32);
extern s32 func_8007352C(s32);
extern s32 saMotionSet(s32, s32);

s32 func_8005FC9C(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8005FC9C.s).
     * Pure C decomp pending; m2c output incomplete (M2C_ERROR markers). */
    (void)arg0; (void)arg1;
    return 0;
}
typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 zero1C;
    s32 pad20;
    s32 pad24;
    s8 byte28;
    s8 padpad[7];
    s16 d0;
    s16 d1;
} S60C8;
extern s32 D_8009B6F0;
extern s32 D_8009B6FC;
extern s32 D_8009B708;
extern s32 D_8009B758;
s32 func_800600C8(s32 arg0, s32 arg1, s32 arg2)
{
    S60C8 s;
    s32 dist_off = arg1 + 0xB4;
    s32 end_off = arg1 + 0xC0;
    s32 cur_tex = arg1;
    s32 i;
    s32 width;
    s16 hi;

    s.p0 = &D_8009B6F0;
    s.byte28 = 0;
    s.zero10 = 0;
    s.zero1C = 0 & 0xFFFFu;
    s.arg2 = arg2;
    if (arg0 < 0xA) {
        width = 0x93;
        if ((!hi) && (!hi)) {}
    } else {
        width = 0xA3;
    }
    s.width = width;
    s.p1 = &D_8009B758;
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    s.p0 = &D_8009B6FC;
    s.d1 = (s.d0 = arg0);
    s.d0 = ((s16)arg0) % 10;
    hi = ((s16)arg0) / 10;
    s.d1 = hi % 10;
    i = 0;
loop_60C8:
    s.p1 = (s32 *)((s32)&D_8009B708 + ((&s.d0)[i] * 8));
    if (arg0 < 0xA) {
        width = 0x64;
    } else {
        width = (((1 - i) << 2) << 3) + 0x54;
    }
    s.width = width;
    s.in_tex = cur_tex;
    cur_tex = func_8007352C((s32)&s);
    if (s.d1 != 0) {
        i += 1;
        if (i < 2) goto loop_60C8;
    }
    initTexPage(dist_off, 1, 0, saMotionSet((s32 *)&D_8009B6F0, 0), 0);
    ot_Link(D_800A374C + (arg2 * 4), dist_off);
    return end_off - arg1;
}
extern u8 D_800A3294[8];
extern u8 D_800A329C[8];
extern u8 D_800A32A4[8];
extern u8 D_800A32AC[8];

void func_800602AC(s32 arg0, s32 *arg1) {
    u8 r1[8], r2[8], r3[8], r4[8];
    s32 s1;
    u8 *p;
    s1 = func_80036EA8(2, arg0 + 0x3D);
    replay_camera_Init(s1, (s32)arg1);
    game_FrameLoop();
    func_80036F28(s1);
    arg1[0] = arg1[0] + (s32)arg1;
    arg1[1] = arg1[1] + (s32)arg1;
    __builtin_memcpy(r1, D_800A3294, 8);
    __builtin_memcpy(r2, D_800A329C, 8);
    p = (u8 *)arg1[0];
    gpu_LoadImage((s32)r1, (s32)(p + 0x40));
    gpu_DrawSync(0);
    gpu_LoadImage((s32)r2, (s32)(p + 0x14));
    gpu_DrawSync(0);
    __builtin_memcpy(r3, D_800A32A4, 8);
    __builtin_memcpy(r4, D_800A32AC, 8);
    arg1 = (s32 *)arg1[1];
    gpu_LoadImage((s32)r3, (s32)((u8 *)arg1 + 0x60));
    gpu_DrawSync(0);
    gpu_LoadImage((s32)r4, (s32)((u8 *)arg1 + 0x14));
    gpu_DrawSync(0);
}
extern s32 saMotionSet();
extern s32 func_8007352C();
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 D_8009B7AC;
extern s32 D_8009B7B8;
extern s32 D_8009B7C4;
extern u16 D_8009B850;
extern u8 D_8009BD24;
extern s32 D_800A328C;
typedef struct {
    s32 *p_geom;
    s32 *p_static;
    s32 arg1_field;
    s32 pad0C;
    s32 zero10;
    s32 arg2_field;
    s32 width;
    s32 height;
    s32 pad20;
    s32 pad24;
    s8 byte28;
} S414;
s32 func_80060414(s16 arg0, s32 arg1, s32 arg2) {
    S414 s;
    s32 dist_off;
    s32 end_off;
    s32 new_var;
    new_var = arg1;
    dist_off = new_var + 0x14;
    end_off = new_var + 0x2C;
    s.byte28 = 0;
    s.zero10 = 0;
    s.arg2_field = arg2;
    s.width = (((u16)(*((&D_8009B850) + (arg0 & 0x7FFF)))) >> 7) + 0x37;
    s.height = ((*((&D_8009B850) + (arg0 & 0x7FFF))) & 0x7F) + 0x2A;
    if (arg0 & 0x8000) {
        s.p_geom = &D_8009B7AC;
    } else if (((u8)D_8009BD24) < 0xC) {
        s.p_geom = &D_8009B7B8;
    } else {
        s.p_geom = &D_8009B7C4;
    }
    s.p_static = &D_800A328C;
    s.arg1_field = new_var;
    func_8007352C((s32)(&s));
    initTexPage(dist_off, 1, 0, saMotionSet((s32)s.p_geom, 0), 0);
    ot_Link(D_800A374C + (arg2 * 4), dist_off);
    return end_off - arg1;
}
extern s32 D_8009B3B0;
extern s32 D_8009B770;
extern s32 D_8009B7A0;
extern s32 D_8009B7D0;
extern s32 D_8009B7D8;
extern s32 D_8009B800;
extern s32 D_8009B820;
extern s32 D_8009B840;
typedef struct {
    s32 *p_geom;
    s32 *p_static;
    s32 arg1_field;
    s32 pad0C;
    s32 zero10;
    s32 arg2_field;
    s32 width;
    s32 height;
    s32 pad20;
    s32 pad24;
    u8 byte28;
    u8 byte29;
    u8 byte2A;
    u8 byte2B;
} S544;
s32 func_80060544(s32 arg0, s32 arg1) {
    s32 new_var;
    S544 s;
    s32 end_off;
    s32 mid_off;
    s32 i;
    s32 j;
    s32 idx;
    s32 new_var4;
    s32 new_var6;
    s32 prev;
    s32 *p0;
    S544 *new_var2;
    s32 *p1;
    int new_var3;
    prev = arg0;
    mid_off = arg0 + 0x4EC;
    end_off = arg0 + 0x5F4;
    s.byte28 = 0;
    s.zero10 = 0;
    s.arg2_field = arg1;
    new_var3 = arg0 + 0x5DC;
    new_var6 = end_off;
    s.pad20 = 0x200;
    s.pad24 = 0x100;
    s.height = 0;
    s.width = 0;
    i = 0;
    idx = 0;
    do {
        s.p_geom = (s32 *)(((s32)(&D_8009B770)) + idx);
        if (i < 3) {
            if (i > 0) {
                goto S800;
            }
            if (i == 0) {
                goto S7D8;
            }
            goto Skip;
        }
        if (i == 3) {
            goto Case3;
        }
        goto Skip;
    S7D8:
        s.p_static = &D_8009B7D8;
        goto Skip;
    S800:
        s.p_static = &D_8009B800;
        goto Skip;
    Case3:
        s.p_static = &D_8009B7D0;
        s.pad0C = mid_off;
        mid_off = func_80073728(&s, 0);
    Skip:
        if (i != 3) {
            s.arg1_field = prev;
            prev = func_8007352C(&s);
        }
        i += 1;
        idx += 0xC;
    } while (i < 4);
    s.p_geom = &D_8009B7A0;
    s.p_static = &D_8009B820;
    s.arg1_field = prev;
    new_var4 = new_var6;
    new_var2 = &s;
    prev = func_8007352C(new_var2);
    j = 0;
    p1 = &D_8009B840;
    new_var = arg1;
    p0 = &D_8009B3B0;
    s.byte29 = 0xFF;
    s.byte2B = 0x10;
    s.byte2A = 0x10;
    s.byte28 = 1;
    do {
        s.p_geom = p0;
        s.p_static = p1;
        s.arg1_field = prev;
        prev = func_8007352C(&s);
        p1 = (s32 *)(((s32)p1) + 8);
        j += 1;
        p0 = (s32 *)(((s32)p0) + 0xC);
    } while (j < 2);
    initTexPage(new_var3, 1, 0, saMotionSet((s32)s.p_geom, 0), 0);
    ot_Link(D_800A374C + (new_var * 4), new_var3);
    new_var3 = new_var4;
    return new_var3 - arg0;
}
extern u16 D_800A32B6;
extern u16 D_800A32B4;
void func_80060758(void) {
    D_800A32B6 = 0;
    D_800A32B4 = 0;
}
extern s32 D_8009B0C0;
extern s32 D_800A374C;
extern void func_8006D808(s32 *, s32 *, s32 *, s32, s32);
extern s32 gpu_SetSemiTransp(void *, s32);
extern s32 initTexPage(void *, s32, s32, s32, s32);
extern s32 initTile(void *);
extern s32 ot_Link(s32, void *);
s32 func_80060768(s32 arg0, s32 arg1, s32 arg2) {
    s32 sp18;
    s32 sp1C;
    s32 t1;
    s32 t2;
    u16 cur1;
    u16 cur2;
    s32 end_off;
    s32 tile_off;
    u16 *p_b4 = &D_800A32B4;
    u16 *p_b6 = &D_800A32B6;

    tile_off = arg0 + 0x7D0;
    end_off = arg0 + 0xAC8;
    sp18 = arg0;
    sp1C = arg0 + 0x870;
    func_8006D808(&sp18, &sp1C, &D_8009B0C0, arg1, arg2);
    if ((u32)arg2 < 3U) {
        initTile((void *)tile_off);
        *(u8 *)(arg0 + 0x7D4) = 0xFF;
        *(s16 *)(arg0 + 0x7D8) = 0x6A;
        *(u8 *)(arg0 + 0x7D5) = 0;
        *(u8 *)(arg0 + 0x7D6) = 0;
        *(s16 *)(arg0 + 0x7DA) = (s16)(arg2 * 0x1A + 0x5B);
        cur1 = D_800A32B4;
        D_800A32B4 = cur1 + 1;
        t1 = (s32)((s16)cur1) * 0x1AA;
        *(s16 *)(arg0 + 0x7DE) = 2;
        *(s16 *)(arg0 + 0x7DC) = (s16)(t1 / 0x1E);
        if ((s16)*p_b4 >= 0x1F) {
            D_800A32B4 = 0x1E;
        }
        gpu_SetSemiTransp((void *)tile_off, 0);
        ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
        tile_off = arg0 + 0x7E0;
    }
    initTile((void *)tile_off);
    *(u8 *)(tile_off + 4) = 0xFF;
    *(s16 *)(tile_off + 8) = 0x9E;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xA) = 0xBD;
    cur2 = D_800A32B6;
    D_800A32B6 = cur2 + 1;
    t2 = (s32)((s16)cur2) * 0x144;
    *(s16 *)(tile_off + 0xE) = 2;
    *(s16 *)(tile_off + 0xC) = (s16)(t2 / 0x1E);
    if ((s16)*p_b6 >= 0x1F) {
        D_800A32B6 = 0x1E;
    }
    gpu_SetSemiTransp((void *)tile_off, 0);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
    tile_off += 0x10;

    initTile((void *)tile_off);
    *(s16 *)(tile_off + 8) = 0x3F;
    *(s16 *)(tile_off + 0xA) = 0x2D;
    *(s16 *)(tile_off + 0xC) = 0x202;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x6C;
    gpu_SetSemiTransp((void *)tile_off, 1);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
    tile_off += 0x10;

    initTile((void *)tile_off);
    *(s16 *)(tile_off + 8) = 0x92;
    *(s16 *)(tile_off + 0xA) = 0xAA;
    *(s16 *)(tile_off + 0xC) = 0x15C;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x1A;
    gpu_SetSemiTransp((void *)tile_off, 1);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);

    initTexPage((void *)sp1C, 1, 0, 0, 0);
    ot_Link(D_800A374C + arg1 * 4, (void *)sp1C);
    sp1C += 0xC;
    return end_off - arg0;
}
void func_80060A68(void) {
    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    register s32 outer asm("$3");
    u16 idx;
    u16 temp_a1;
    s32 result;
    volatile s32 _frame_pad[2];

    outer = D_800A3468;
    idx = *(u16 *)outer;
    *(s32 *)((s32)&D_800F10D0 + idx * 4) = 0;
    (void)_frame_pad;

    *(s32 *)(outer + 0x20) = *(s32 *)(*(s32 *)(outer + 0xC) + 0);
    *(s32 *)(outer + 0x24) = *(s32 *)(*(s32 *)(outer + 0xC) + 4);
    *(s32 *)(outer + 0x28) = *(s32 *)(*(s32 *)(outer + 0xC) + 8);

    *(u16 *)(outer + 0x18) = *(u16 *)(*(s32 *)(outer + 0x10) + 0);
    D_800A3478 = outer + 0x18;
    *(u16 *)(outer + 0x1A) = *(u16 *)(*(s32 *)(outer + 0x10) + 2);
    temp_a1 = *(u16 *)(*(s32 *)(outer + 0x10) + 4);
    D_800A347C = outer + 0x20;
    *(u16 *)(outer + 0x1C) = temp_a1;

    result = ((s32 (*)(s32, s32)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))(idx, temp_a1);
    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;

    if (*(s32 *)D_800A3468 & 0x200000) {
        D_800A32BC = 0xA;
    }
}
void func_80060B70(void) {
    extern s32 D_800A3468;
    extern s32 D_800A346C;
    extern s32 D_800A3470;
    extern s32 D_800A3474;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    extern void func_80061FAC(s32, s32, s32);
    s32 outer;
    s32 dst_u16;
    s32 dst_s32;
    u16 idx;
    s32 result;

    outer = D_800A3468;
    dst_u16 = (s32)D_800A346C;
    *(u16 *)(dst_u16 + 0) = *(u16 *)(*(s32 *)(outer + 4) + 0);
    *(u16 *)(dst_u16 + 2) = *(u16 *)(*(s32 *)(outer + 4) + 2);
    *(u16 *)(dst_u16 + 4) = *(u16 *)(*(s32 *)(outer + 4) + 4);

    dst_s32 = (s32)D_800A3470;
    *(s32 *)(dst_s32 + 0) = *(s32 *)(*(s32 *)(outer + 8) + 0);
    *(s32 *)(dst_s32 + 4) = *(s32 *)(*(s32 *)(outer + 8) + 4);
    *(s32 *)(dst_s32 + 8) = *(s32 *)(*(s32 *)(outer + 8) + 8);

    func_80061FAC(dst_u16, dst_s32, D_800A3474);

    idx = *(u16 *)D_800A3468;
    result = ((s32 (*)(void)) *(s32 *)((s32)&chractar_use_pset_combo_id_table
              + (*(u8 *)((s32)&D_8009BA60 + idx) + *(s32 *)((s32)&D_800F10D0 + idx * 4)) * 4))();

    *(s8 *)*(s32 *)((s32)D_800A3468 + 0x14) = result;
}
extern s32 D_800F10D0[];
extern u8 D_800F1150[];
extern s16 D_800A345E;
extern s16 D_800A345C;
extern s32 D_800A3458;
extern s32 D_800A3454;
extern s32 D_800A3450;
extern s32 D_800A344C;
extern s32 D_800A3460;
extern s32 D_800A3444;
extern s32 D_800A3448;
void saTan1GaugeInit(void) {
    s32 i = 0;
    s32 *p = D_800F10D0;
    do {
        *p = 0;
        D_800F1150[i] = 0;
        i++;
        p++;
    } while (i < 0x1C);
    D_800A345E = 0;
    D_800A345C = 0;
    D_800A3458 = 0;
    D_800A3454 = 0;
    D_800A3450 = 0;
    D_800A344C = 0;
    D_800A3460 = 0;
    D_800A3444 = 0;
    D_800A3448 = 0;
}

extern s32 D_800A3420;
extern s32 D_800A3424;

s32 func_80060CB8(s32 arg0, s32 arg1) {
    typedef struct {
        s16 sp10;
        s16 sp12;
        s16 sp14;
        s16 sp16;
    } SLocal;
    SLocal s;
    s32 v;
    s32 ret;

    game_FrameLoop();
    v = D_8009BD38 & 0xF;
    if (v == 0) {
        replay_camera_Init(func_80036EA8(2, 0x3C), arg0);
    } else if (v == 3) {
        replay_camera_Init(func_80036EA8(2, 0x2F), arg0);
    } else if (v == 2) {
        replay_camera_Init(func_80036EA8(2, 0x30), arg0);
    } else if (v == 5) {
        replay_camera_Init(func_80036EA8(2, 0x31), arg0);
    } else {
        replay_camera_Init(func_80036EA8(2, 0), arg0);
    }
    game_FrameLoop();
    s.sp10 = 0x380;
    s.sp12 = 0;
    s.sp14 = 0x80;
    s.sp16 = 0x1DC;
    gpu_DrawSync(0);
    gpu_LoadImage(&s.sp10, arg0);
    gpu_DrawSync(0);
    s.sp14 = 0x70;
    s.sp12 = 0x1DC;
    s.sp16 = 0x24;
    gpu_LoadImage(&s.sp10, arg0 + 0x1DC00);
    gpu_DrawSync(0);
    saTan1GaugeInit();
    func_80079184(func_80079154());
    ret = arg1 + 0x4650;
    D_800A3420 = arg1;
    D_800A3424 = ret;
    return ret + 0x4650;
}
extern s32 D_800A3420;
extern s32 D_800A3424;
extern volatile s32 D_800A37D4;
extern s32 D_800A3720;
void gnd_get_fog(s32 arg0) {
    s32 v0;
    if (arg0 != 0) {
        v0 = D_800A3424;
    } else {
        v0 = D_800A3420;
    }
    D_800A37D4 = v0;
    D_800A3720 = D_800A37D4;
}
extern s32 D_800A3468;
extern s32 D_800A346C;
extern s32 D_800A3470;
extern s32 D_800A3474;
extern s32 D_800A3480;
extern s32 D_800A3484;
extern s32 D_800A3488;
extern s32 D_800A348C;
extern s32 D_800A3490;
extern s32 D_800A3494;
extern s32 D_800A3498;
extern s32 D_800A349C;
extern s32 D_800A34A0;
extern s32 D_800A34A4;
extern s32 D_800A34A8;
extern s32 D_800A34AC;
extern s32 D_800A34B0;
extern s32 D_800A34B4;
extern s32 D_800A34B8;
extern s32 D_800A34BC;
extern s32 D_800A34C0;
extern s32 D_800A34C4;
extern s32 D_800A34C8;
extern s32 D_800A34CC;
extern s32 D_800A34D0;
extern s32 D_800A34D4;
extern s32 D_800A34D8;
extern s32 D_800A34DC;
extern s32 D_800A34E0;
extern s32 D_800A34E4;
extern s32 D_800A34E8;
extern s32 D_800A34EC;
void func_80060E38(s32 arg0, s32 arg1) {
    D_800A3468 = 0x1F800000;
    D_800A346C = 0x1F800018;
    D_800A3470 = 0x1F800020;
    D_800A3474 = 0x1F800030;
    D_800A3488 = 0x1F800050;
    D_800A3490 = 0x1F800058;
    D_800A3494 = 0x1F80005C;
    D_800A3498 = 0x1F800060;
    D_800A349C = 0x1F800062;
    D_800A34A0 = 0x1F800064;
    D_800A34A4 = 0x1F800066;
    D_800A34A8 = 0x1F800068;
    D_800A34AC = 0x1F80006A;
    D_800A34B0 = 0x1F80006C;
    D_800A34B4 = 0x1F800070;
    D_800A34B8 = 0x1F800074;
    D_800A34BC = 0x1F800080;
    D_800A34C0 = 0x1F800082;
    D_800A34C4 = 0x1F800084;
    D_800A34C8 = 0x1F800088;
    D_800A34CC = 0x1F80008C;
    D_800A34D0 = 0x1F800090;
    D_800A34D4 = 0x1F800098;
    D_800A34D8 = 0x1F80009A;
    D_800A34DC = 0x1F80009C;
    D_800A34E0 = 0x1F80009E;
    D_800A34E4 = 0x1F8000A0;
    D_800A34E8 = 0x1F8000A4;
    D_800A3480 = 0x1F8000A8;
    D_800A3484 = 0x1F8000AC;
    D_800A348C = 0x1F8000B0;
    D_800A34EC = 0x1F8000B8;
    *(s32 *)0x1F800004 = arg0;
    *(s32 *)0x1F800008 = arg1;
}
extern s32 func_80041E10();
extern s32 leaf_muki_awase_800421A4();
extern s32 func_80060B70();
extern s32 func_80060E38();
extern s32 debug_printf(s32 *, s32);
extern s32 D_800158E0;
extern s32 D_800A32BC;
extern s32 D_800A3464;
extern s32 *D_800A3468;
extern s32 D_800A3720;
extern s32 D_800A37D4;
extern s32 D_800F1140;
extern u8 D_800F1150;
void func_80061064(void) {
    s32 temp_a1;
    s32 i;
    func_80060E38();
    i = 0;
    do {
        *(s32 **)((s32)D_800A3468 + 0x14) = (s32 *)(i + (s32)&D_800F1150);
        if (*((u8 *)&D_800F1150 + i) != 0) {
            *(s32 *)D_800A3468 = i;
            func_80060B70();
        }
        i += 1;
    } while (i < 0x1C);
    if (D_800A32BC >= 2) {
        D_800A32BC -= 1;
        func_80041E10(&D_800F1140, D_800A3464);
    } else if (D_800A32BC == 1) {
        leaf_muki_awase_800421A4();
        D_800A32BC = 0;
    }
    temp_a1 = (s32)-((D_800A37D4 - D_800A3720) * 0x33333333) >> 3;
    if (temp_a1 >= 0x1C2) {
        debug_printf(&D_800158E0, temp_a1 - 0x1C2);
    }
}
extern s32 D_800A32BC;
void saTan1GaugeInit(void);
void leaf_muki_awase_800421A4(void);
void game_Cleanup(void) {
    saTan1GaugeInit();
    leaf_muki_awase_800421A4();
    D_800A32BC = 0;
}
extern u8 D_800F116A;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
void func_800611A4(s32 *arg0, s32 *arg1) {
    u16 sp[3];
    u16 new_var;
    s32 *v1 = (s32 *) (&D_800F116C);
    register s32 t asm("$2");
    register s32 mask asm("$3");
    sp[0] = *((u16 *) (((s32) arg1) + 0));
    sp[1] = *((u16 *) (((s32) arg1) + 2));
    D_800A3468 = (s32) v1;
    new_var = *((u16 *) (((s32) arg1) + 4));
    D_800F117C = (s32) (&sp[0]);
    D_800F1178 = (s32) arg0;
    D_800F1180 = (s32) (&D_800F116A);
    *v1 = 0x21001A;
    sp[2] = new_var;
    func_80060A68();
    t = arg0[0];
    D_800F1140 = t;
    t = arg0[1];
    D_800F1144 = t;
    mask = 0xFFFFEF;
    D_800A3464 = mask;
    t = arg0[2];
    D_800F1148 = t;
}
extern volatile u8 D_800F1159;
void func_80061250(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1159)[0] != 0) {
        if ((&D_800F1159)[1] != 0) {
            (&D_800F1159)[1] = 0;
            (&D_800F1159)[0] = 0;
        }
        if ((&D_800F1159)[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)&D_800F1159;
    *(s32 *)D_800A3468 = 0x210009;
    goto end;
check_one_zero:
    if ((&D_800F1159)[1] == 0) {
        D_800F1180 = (s32)((&D_800F1159) + 1);
        *v1 = 0x21000A;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF0060;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern u8 D_800F1154;
extern s32 D_800A3464;
extern s32 D_800A3468;
extern s32 D_800F116C;
extern s32 D_800F1178;
extern s32 D_800F1180;
s32 func_8006133C(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F1154;
    *v1 = 0x210004;
    func_80060A68();
    t = a0[0]; D_800F1140 = t;
    t = a0[1]; D_800F1144 = t;
    mask = 0x8080FF;
    D_800A3464 = mask;
    t = a0[2]; D_800F1148 = t;
    return 1;
}
extern u8 D_800F115B;
s32 func_800613C8(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
    *v1 = 0x21000B;
    func_80060A68();
    t = a0[0]; D_800F1140 = t;
    t = a0[1]; D_800F1144 = t;
    mask = 0x8080FF;
    D_800A3464 = mask;
    t = a0[2]; D_800F1148 = t;
    return 16;
}
extern u8 D_800F115B;
extern s32 D_800A3464;
extern s32 D_800A3468;
extern s32 D_800F116C;
extern s32 D_800F1178;
extern s32 D_800F1180;
s32 func_80061454(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
    *v1 = 0x29000B;
    func_80060A68();
    t = a0[0]; D_800F1140 = t;
    t = a0[1]; D_800F1144 = t;
    mask = 0x8080FF;
    D_800A3464 = mask;
    t = a0[2]; D_800F1148 = t;
    return 8;
}
s32 func_800614E0(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F115B;
    *v1 = 0x31000B;
    func_80060A68();
    t = a0[0]; D_800F1140 = t;
    t = a0[1]; D_800F1144 = t;
    mask = 0x8080FF;
    D_800A3464 = mask;
    t = a0[2]; D_800F1148 = t;
    return 5;
}
extern u8 D_800F1154;
void func_8006156C(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if ((&D_800F1154)[1] != 0) {
        if ((&D_800F1154)[2] != 0) {
            (&D_800F1154)[2] = 0;
            (&D_800F1154)[1] = 0;
        }
        if ((&D_800F1154)[1] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)((&D_800F1154) + 1);
    *(s32 *)D_800A3468 = 0x210005;
    goto end;
check_one_zero:
    if ((&D_800F1154)[2] == 0) {
        D_800F1180 = (s32)((&D_800F1154) + 2);
        *v1 = 0x210006;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF8080;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern u8 D_800F115C;
extern s32 D_800F116C;
extern s32 D_800A3464;
extern s32 D_800A3468;
void func_80061658(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000C;
        p = &D_800F115C;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000D;
        p = &D_800F115C + 1;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0x10FFFF;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
void func_80061710(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    u8 *p;
    s32 val;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    switch (arg1) {
    case 0:
        val = 0x21000E;
        p = &D_800F115C + 2;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    case 1:
        val = 0x21000F;
        p = &D_800F115C + 3;
        *p = 0;
        D_800F1180 = (s32)p;
        *v1 = val;
        break;
    }
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0x10FF10;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern u8 D_800F1160[];
void func_800617C8(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[0] != 0) {
        if (D_800F1160[1] != 0) {
            D_800F1160[1] = 0;
            D_800F1160[0] = 0;
        }
        if (D_800F1160[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1160;
    *(s32 *)D_800A3468 = 0x210010;
    goto end;
check_one_zero:
    if (D_800F1160[1] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 1);
        *v1 = 0x210011;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xC06013;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern u8 D_800F1152[];
extern s32 D_800F117C;
void func_800618B4(s32 *arg0, s32 arg1) {
    s32 *v1 = (s32 *)&D_800F116C;
    u8 *new_var;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    D_800F117C = arg1;
    new_var = D_800F1152;
    if (new_var[0] != 0) {
        if (D_800F1152[1] != 0) {
            D_800F1152[1] = 0;
            D_800F1152[0] = 0;
        }
        if (new_var[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1152;
    *(s32 *)D_800A3468 = 0x210002;
    goto end;
check_one_zero:
    if (D_800F1152[1] == 0) {
        D_800F1180 = (s32)(new_var + 1);
        *v1 = 0x210003;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF0000;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern s32 D_800F116C;
extern s32 D_800A3468;
extern s32 D_800F1178;
extern s32 D_800F1180;
extern s32 D_800F1158;
void func_80060A68(void);
void func_800619A4(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)&D_800F1158;
    *v1 = 0x10008;
    func_80060A68();
}

extern s32 D_800F116C;
extern s32 D_800A3468;
extern s32 D_800F1178;
extern s32 D_800F1180;
extern u8 D_800F1154[];
void func_80060A68(void);
void func_800619F0(s32 *a0) {
    s32 *v1 = (s32 *)&D_800F116C;
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)a0;
    D_800F1180 = (s32)(D_800F1154 + 3);
    *v1 = 0x10007;
    func_80060A68();
}

extern s32 D_800F117C;
extern u8 D_800F1151;
void func_80061A3C(s32 *a0, s16 a1, s32 a2, s32 a3) {
    s32 *v1 = (s32 *)&D_800F116C;
    s16 sp[4];

    D_800A3468 = (s32)v1;
    sp[2] = 0;
    sp[0] = 0;
    sp[1] = a1;
    D_800F1178 = (s32)a0;
    D_800F117C = (s32)sp;
    if (a3 == 0) {
        D_800F1180 = (s32)&D_800F1150;
        *v1 = a2 + 0x10000;
    } else {
        D_800F1180 = (s32)&D_800F1151;
        *v1 = a2 + 0x10001;
    }
    func_80060A68();
}
extern u8 D_800F1164[];
void func_80061ACC(s32 *arg0, s32 arg1) {
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)&D_800F116C;
    D_800F1178 = (s32)arg0;
    D_800F117C = arg1;
    if (D_800F1164[0] != 0) {
        if (D_800F1164[1] != 0) {
            D_800F1164[1] = 0;
            D_800F1164[0] = 0;
        }
        if (D_800F1164[0] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164;
    *(s32 *)D_800A3468 = 0x210014;
    func_80060A68();
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164 - 0xD;
    *(s32 *)D_800A3468 = 0x10007;
    goto end;
check_one_zero:
    if (D_800F1164[1] == 0) {
        D_800F1180 = (s32)(D_800F1164 + 1);
        *(s32 *)D_800A3468 = 0x210015;
        func_80060A68();
        *(s32 *)((s32)D_800A3468 + 0x14) = (s32)D_800F1164 - 0xD;
        *(s32 *)D_800A3468 = 0x10007;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF8080;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern s16 D_800A34F0;
extern s16 D_800A34F2;
void func_80061C00(s32 arg0, s16 arg1, s32 arg2) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20[3];
    u8 sp30[32];
    s32 sp50;

    D_800A3468 = (s32)&D_800F116C;
    if (arg2 != 1) {
        arg2 = 0;
    }
    sp10[1] = -0xA00;
    sp10[0] = 0;
    sp10[2] = 0xA00;
    sp20[2] = 0;
    sp20[1] = 0;
    sp20[0] = 0;
    sp18[1] = arg1;
    sp18[2] = 0;
    sp18[0] = 0;
    motutil_GetWalkDir(sp18, sp30);
    *(s32 *)(sp30 + 0x1C) = 0;
    *(s32 *)(sp30 + 0x18) = 0;
    *(s32 *)(sp30 + 0x14) = 0;
    gte_SetRotMatrix(sp30);
    gte_SetTransVector(sp30);
    func_8007F2AC(sp10, sp20, &sp50);
    sp10[0] = (s16)sp20[0];
    sp10[1] = (s16)sp20[1];
    sp10[2] = (s16)sp20[2];
    *(s32 *)(D_800A3468 + 0xC) = arg0;
    *(s32 *)(D_800A3468 + 0x10) = (s32)sp10;
    if ((D_800F1164 + 2)[0] != 0) {
        if ((D_800F1164 + 2)[1] != 0) {
            (D_800F1164 + 2)[1] = 0;
            (D_800F1164 + 2)[0] = 0;
        }
        if ((D_800F1164 + 2)[0] != 0) goto check_one_zero;
    }
    *(s32 *)(D_800A3468 + 0x14) = (s32)(D_800F1164 + 2);
    *(s32 *)D_800A3468 = 0x10016;
    D_800A34F0 = arg2;
    goto end;
check_one_zero:
    if ((D_800F1164 + 2)[1] == 0) {
        *(s32 *)(D_800A3468 + 0x14) = (s32)(D_800F1164 + 3);
        *(s32 *)D_800A3468 = 0x10017;
        D_800A34F2 = arg2;
    }
end:
    func_80060A68();
}
extern u8 D_800F1168[];
void func_8007F2AC(s16 *, s32 *, s32 *);
void *motutil_GetWalkDir(s16 *, u8 *);
void gte_SetRotMatrix(u8 *);
void gte_SetTransVector(u8 *);
void func_80061D74(s32 arg0, s16 arg1) {
    s16 sp10[4];
    s16 sp18[4];
    s32 sp20[3];
    u8 sp30[32];
    s32 sp50;

    D_800A3468 = (s32)&D_800F116C;
    sp10[1] = -0xA00;
    sp10[0] = 0;
    sp10[2] = 0xA00;
    sp20[2] = 0;
    sp20[1] = 0;
    sp20[0] = 0;
    sp18[2] = 0;
    sp18[1] = arg1;
    sp18[0] = 0;
    motutil_GetWalkDir(sp18, sp30);
    *(s32 *)(sp30 + 0x1C) = 0;
    *(s32 *)(sp30 + 0x18) = 0;
    *(s32 *)(sp30 + 0x14) = 0;
    gte_SetRotMatrix(sp30);
    gte_SetTransVector(sp30);
    func_8007F2AC(sp10, sp20, &sp50);
    sp10[0] = (s16)sp20[0];
    sp10[1] = (s16)sp20[1];
    sp10[2] = (s16)sp20[2];
    *(s32 *)(D_800A3468 + 0xC) = arg0;
    *(s32 *)(D_800A3468 + 0x10) = (s32)sp10;
    if (D_800F1168[0] != 0) {
        if (D_800F1168[1] != 0) {
            D_800F1168[1] = 0;
            D_800F1168[0] = 0;
        }
        if (D_800F1168[0] != 0) goto check_one_zero;
    }
    *(s32 *)(D_800A3468 + 0x14) = (s32)D_800F1168;
    *(s32 *)D_800A3468 = 0x10018;
    goto end;
check_one_zero:
    if (D_800F1168[1] == 0) {
        *(s32 *)(D_800A3468 + 0x14) = (s32)(D_800F1168 + 1);
        *(s32 *)D_800A3468 = 0x10019;
    }
end:
    func_80060A68();
}
void func_80061EC0(s32 *arg0) {
    s32 *v1 = (s32 *)&D_800F116C;
    register s32 t asm("$2");
    register s32 mask asm("$3");
    D_800A3468 = (s32)v1;
    D_800F1178 = (s32)arg0;
    if (D_800F1160[2] != 0) {
        if (D_800F1160[3] != 0) {
            D_800F1160[3] = 0;
            D_800F1160[2] = 0;
        }
        if (D_800F1160[2] != 0) goto check_one_zero;
    }
    *(s32 *)((s32)D_800A3468 + 0x14) = (s32)(D_800F1160 + 2);
    *(s32 *)D_800A3468 = 0x210012;
    goto end;
check_one_zero:
    if (D_800F1160[3] == 0) {
        D_800F1180 = (s32)(D_800F1160 + 3);
        *v1 = 0x210013;
    }
end:
    func_80060A68();
    t = arg0[0]; D_800F1140 = t;
    t = arg0[1]; D_800F1144 = t;
    mask = 0xFF00FF;
    D_800A3464 = mask;
    t = arg0[2]; D_800F1148 = t;
}
extern s32 D_800A34EC;
extern u8 D_8009BB74[];
void *motutil_GetWalkDir(u16*, u8*);
void func_8007E8DC(u8*, u8*);
void gte_SetRotMatrix(u8*);
void func_80061FAC(u16 *a0, s32 a1, u8 *a2) {
    u16 *v1 = a0;
    u16 *dest = (u16 *)D_800A34EC;
    u8 *s0 = a2;
    dest[0] = v1[0];
    dest[1] = v1[1];
    dest[2] = v1[2];
    motutil_GetWalkDir(dest, s0);
    *(s32 *)(s0 + 0x1C) = 0;
    *(s32 *)(s0 + 0x18) = 0;
    *(s32 *)(s0 + 0x14) = 0;
    func_8007E8DC(s0, D_8009BB74);
    gte_SetRotMatrix(s0);
}
extern s32 D_800A32B8;
extern s32 D_800F1198;
extern s32 D_800F119C;
extern s32 D_800F11A0;
void func_80062020(s32 *arg0) {
    register s32 *a0 asm("$4") = arg0;
    register s32 i asm("$5") = 0;
    register s32 ofs asm("$3") = 0;
    register s32 t asm("$2");
    s32 *p;
    s32 i12;
    D_800A32B8 = 0;
    t = a0[0];
    if ((t & 1) == 0) goto end;
    do {
        t = a0[0];
        *(s32 *)((u8 *)&D_800F1198 + ofs) = t;
        t = a0[1];
        i = i + 1;
        *(s32 *)((u8 *)&D_800F119C + ofs) = t;
        t = a0[2];
        a0 = (s32 *)((u8 *)a0 + 12);
        *(s32 *)((u8 *)&D_800F11A0 + ofs) = t;
        t = a0[0];
        ofs = ofs + 12;
    } while ((t & 1) != 0);
end:
    i12 = i + i;
    i12 = i12 + i;
    i12 = i12 << 2;
    p = (s32 *)((u8 *)&D_800F1198 + i12);
    p[2] = 0;
    p[1] = 0;
    *(s32 *)((u8 *)&D_800F1198 + i12) = 0;
}
void func_800620B8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800620B8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
s32 func_8006288C(void) {
    extern s32 D_800A3460;
    extern volatile s32 D_800A347C;
    extern volatile s32 D_800A3478;
    extern s16 D_800F0C04;
    extern s32 D_800F0FB8;
    extern s32 D_800F0FBC;
    extern s32 D_800F0FC0;
    extern s32 D_800F10A0;
    extern s32 D_800F10A2;
    extern s32 D_800F10A4;
    extern s32 D_800F1138;
    int new_var;
    s16 *flag_p;
    s32 *src_a;
    u16 *src_b;
    s32 i;
    s32 off_s32;
    s32 off_s16;
    s32 mask;

    i = 0;
    flag_p = &D_800F0C04;
    off_s16 = 0;
    off_s32 = 0;
    src_a = (s32 *)D_800A347C;
    src_b = (u16 *)D_800A3478;
    D_800F1138 = 1;
    new_var = 1;
loop_top:
    mask = new_var << i;
    if (!(D_800A3460 & mask)) {
        *(s32 *)((s32)&D_800F0FB8 + off_s32) = src_a[0];
        *(s32 *)((s32)&D_800F0FBC + off_s32) = src_a[1];
        *(s32 *)((s32)&D_800F0FC0 + off_s32) = src_a[2];
        *(u16 *)((s32)&D_800F10A0 + off_s16) = src_b[0];
        *(u16 *)((s32)&D_800F10A2 + off_s16) = src_b[1];
        D_800A3460 |= mask;
        *(u16 *)((s32)&D_800F10A4 + off_s16) = src_b[2];
        *flag_p = 0;
    } else {
        flag_p++;
        off_s16 += 8;
        i++;
        off_s32 += 0xC;
        if (i < 6) goto loop_top;
    }
    return 1;
}
void func_8006295C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006295C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern volatile s32 D_800A347C;

s32 func_80062FEC(void) {
    s32 i;
    s32 bit;
    s32 idx;
    s32 word_off;
    u8 *new_var;
    s32 *src;
    D_800F10F0 = 1;
    i = 0;
    bit = 1;
    do {
        if ((D_800A3448 & (bit << i)) == 0) {
            D_800A3448 |= bit << i;
            idx = i * 2;
            goto found;
        }
        i++;
    } while (i < 12);
    idx = i * 2;
found:
    src = (s32 *) D_800A347C;
    word_off = (idx + i) << 2;
    *((s32 *) (((u8 *) (&D_800F0E38)) + word_off)) = src[0];
    *((s32 *) (((u8 *) (&D_800F0E3C)) + word_off)) = src[1];
    do { } while (0);
    *((s32 *) (((u8 *) (&D_800F0E40)) + word_off)) = src[2];
    *((s16 *) (((u8 *) (new_var = &D_800F0BEC)) + idx)) = 0;
    return 1;
}
void func_80063084(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80063084.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A3468;
extern s32 D_800F10D0;
extern s16 D_800A345C;
u8 func_80063BD0(s32);
u8 func_80063AF0(void) {
    s32 *v1 = (s32 *)D_800A3468;
    D_800F10D0 = 1;
    D_800A345C = (*v1 >> 17) & 3;
    return func_80063BD0(0);
}
extern s32 D_800F10D4;
extern s16 D_800A345E;
u8 func_80063B34(void) {
    s32 *v1 = (s32 *)D_800A3468;
    D_800F10D4 = 1;
    D_800A345E = (*v1 >> 17) & 3;
    return func_80063BD0(1);
}
extern s32 D_800A3480;
extern s16 D_800A345C;
s32 func_80063E10(s32);
u8 func_80063B78(void) {
    *(s32 *)D_800A3480 = D_800A345C;
    return func_80063E10(0);
}
extern s16 D_800A345E;
u8 func_80063BA4(void) {
    *(s32 *)D_800A3480 = D_800A345E;
    return func_80063E10(1);
}
u8 func_80063BD0(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80063BD0.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
    return 0;
}
s32 func_80063E10(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80063E10.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
void func_800644FC(s32 *arg0, s32 arg1, s32 arg2)
{
    s32 i;
    s32 ptr;
    s32 *bits_p;
    s32 mul50;
    s32 dummy_pad;
    __asm__ volatile ("" : "=m"(dummy_pad));
    __asm__ volatile ("" : "=m"(dummy_pad));
    if ((*arg0) > 0) {
        bits_p = (s32 *)((s32)&D_800A3454 + (arg2 << 2));
        mul50 = (arg2 + (arg2 << 2)) << 4;
        ptr = arg1;
        i = 0;
        do {
            register s32 one asm("$2") = 1;
            register s32 mask asm("$2");
            mask = one << i;
            if ((*bits_p) & mask) {
                u8 *base;
                __asm__ volatile ("" : "=r"(base) : "0"(&D_800F1000));
                motutil_GetWalkDir((u16 *)(base + mul50 + (i << 3)), (u8 *)ptr);
            }
            i += 1;
            ptr += 0x20;
        } while (i < (*arg0));
    }
}
extern s32 func_80079154(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    register s32 s3 asm("$19");
    s32 mask;
    s32 a0;
    s32 s0;
    s32 idx2;
    s32 cont;
    s3 = 0;
    D_800F10EC = 1;
    do {
        a0 = 0;
loop_inner:
        s0 = s3 + a0;
        {
            register s32 one asm("$3") = 1;
            mask = one << s0;
        }
        a0 += 1;
        if (!(D_800A3444 & mask)) {
            idx2 = s0 << 1;
            *((s32 *)(((s32)(&D_800F0D78)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[0] + (func_80079154() & 0xFF)) - 0x7F;
            s3 += 4;
            *((s32 *)(((s32)(&D_800F0D7C)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[1] + (func_80079154() & 0xFF)) - 0x7F;
            *((s32 *)(((s32)(&videoDec)) + ((idx2 + s0) << 2))) = (((s32 *)D_800A347C)[2] + (func_80079154() & 0xFF)) - 0x7F;
            {
                s32 last = func_80079154();
                s32 bits = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                D_800A3444 = bits | mask;
            }
            do { } while (0);
            cont = s3 < 0xF;
        } else {
            if (a0 >= 4) {
                s3 += 4;
                cont = s3 < 0xF;
            } else {
                goto loop_inner;
            }
        }
    } while (cont != 0);
    return 1;
}
void func_800646E8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800646E8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CA0;
extern volatile s32 D_800F0CA4;
extern volatile s32 D_800F0CA8;
extern volatile s32 D_800F10E0;
extern volatile s16 D_800F0BA8;
void func_80064E90(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CA0 = p[0];
    D_800F0CA4 = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E0 = 1;
    D_800F0BA8 = 0;
    D_800F0CA8 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CAC;
extern volatile s32 D_800F0CB0;
extern volatile s32 D_800F0CB4;
extern volatile s32 D_800F10E4;
extern volatile s16 D_800F0BAA;
void func_80064ED8(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CAC = p[0];
    D_800F0CB0 = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E4 = 1;
    D_800F0BAA = 0;
    D_800F0CB4 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CB8;
extern volatile s32 D_800F0CBC;
extern volatile s32 D_800F0CC0;
extern volatile s32 D_800F10E8;
extern volatile s16 D_800F0BAC;
void func_80064F20(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CB8 = p[0];
    D_800F0CBC = p[1];
    p = (volatile s32 *)p[2];
    D_800F10E8 = 1;
    D_800F0BAC = 0;
    D_800F0CC0 = (s32)p;
}
extern void *D_800A347C;
extern s32 D_800F0CC4;
extern s32 D_800F0CC8;
extern s32 D_800F0CCC;
extern s32 D_800F10F4;
extern s16 D_800F0BAE;

s32 func_80064F68(void) {
    void *p = D_800A347C;
    s32 last;
    D_800F0CC4 = *(s32 *)((s32)p + 0);
    D_800F0CC8 = *(s32 *)((s32)p + 4);
    last = *(s32 *)((s32)p + 8);
    D_800F10F4 = 1;
    D_800F0BAE = 0x40;
    D_800F0CCC = last;
    return 1;
}
extern s32 D_800F0CD0;
extern s32 D_800F0CD4;
extern s32 D_800F0CD8;
extern s32 D_800F10F8;
extern s16 D_800F0BB0;

s32 func_80064FB4(void) {
    void *p = D_800A347C;
    s32 last;
    D_800F0CD0 = *(s32 *)((s32)p + 0);
    D_800F0CD4 = *(s32 *)((s32)p + 4);
    last = *(s32 *)((s32)p + 8);
    D_800F10F8 = 1;
    D_800F0BB0 = 0x40;
    D_800F0CD8 = last;
    return 1;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800A3468;
extern volatile s32 D_800F0CDC;
extern volatile s32 D_800F0CE0;
extern volatile s32 D_800F0CE4;
extern volatile s32 D_800F10FC;
extern volatile s16 D_800F0BB2;
extern volatile s16 D_800A3440;
s32 func_80065000(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    volatile s32 *q = (volatile s32 *)D_800A3468;
    D_800F0CDC = p[0];
    D_800F0CE0 = p[1];
    p = (volatile s32 *)p[2];
    D_800F10FC = 1;
    D_800F0BB2 = 0;
    D_800F0CE4 = (s32)p;
    D_800A3440 = (q[0] >> 19) & 3;
    return 1;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CE8;
extern volatile s32 D_800F0CEC;
extern volatile s32 D_800F0CF0;
extern volatile s32 D_800F1100;
extern volatile s16 D_800F0BB4;
void func_8006505C(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CE8 = p[0];
    D_800F0CEC = p[1];
    p = (volatile s32 *)p[2];
    D_800F1100 = 1;
    D_800F0BB4 = 0;
    D_800F0CF0 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0CF4;
extern volatile s32 D_800F0CF8;
extern volatile s32 D_800F0CFC;
extern volatile s32 D_800F1104;
extern volatile s16 D_800F0BB6;
void func_800650A4(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0CF4 = p[0];
    D_800F0CF8 = p[1];
    p = (volatile s32 *)p[2];
    D_800F1104 = 1;
    D_800F0BB6 = 0;
    D_800F0CFC = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D18;
extern volatile s32 D_800F0D1C;
extern volatile s32 D_800F0D20;
extern volatile s32 D_800F1108;
extern volatile s16 D_800F0BBC;
void func_800650EC(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D18 = p[0];
    D_800F0D1C = p[1];
    p = (volatile s32 *)p[2];
    D_800F1108 = 1;
    D_800F0BBC = 0;
    D_800F0D20 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D24;
extern volatile s32 D_800F0D28;
extern volatile s32 D_800F0D2C;
extern volatile s32 D_800F110C;
extern volatile s16 D_800F0BBE;
void func_80065134(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D24 = p[0];
    D_800F0D28 = p[1];
    p = (volatile s32 *)p[2];
    D_800F110C = 1;
    D_800F0BBE = 0;
    D_800F0D2C = (s32)p;
}
extern u16 D_800F0BC0;
extern u16 D_800F0BC4;
extern s32 D_800F0D38;
extern s32 D_800F0D48;
extern s32 D_800F0D4C;
extern s32 D_800F0D50;
extern s32 D_800F1110;
void func_8006517C(void) {
    register volatile s32 *p asm("$3") = (volatile s32 *)D_800A347C;
    register s32 t asm("$2");
    t = p[0]; D_800F0D30 = t;
    t = p[1]; D_800F0D34 = t;
    t = p[2];
    D_800F0BC0 = 0;
    D_800F0D38 = t;
    t = p[0]; D_800F0D48 = t;
    t = p[1]; D_800F0D4C = t;
    p = (volatile s32 *)p[2];
    D_800F1110 = 1;
    D_800F0BC4 = 0;
    D_800F0D50 = (s32)p;
}
extern u16 D_800F0BC2;
extern u16 D_800F0BC6;
extern s32 D_800F0D44;
extern s32 D_800F0D54;
extern s32 D_800F0D58;
extern s32 D_800F0D5C;
extern s32 D_800F1114;
void func_800651F0(void) {
    register volatile s32 *p asm("$3") = (volatile s32 *)D_800A347C;
    register s32 t asm("$2");
    t = p[0]; D_800F0D3C = t;
    t = p[1]; D_800F0D40 = t;
    t = p[2];
    D_800F0BC2 = 0;
    D_800F0D44 = t;
    t = p[0]; D_800F0D54 = t;
    t = p[1]; D_800F0D58 = t;
    p = (volatile s32 *)p[2];
    D_800F1114 = 1;
    D_800F0BC6 = 0;
    D_800F0D5C = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D60;
extern volatile s32 D_800F0D64;
extern volatile s32 D_800F0D68;
extern volatile s32 D_800F1118;
extern volatile s16 D_800F0BC8;
void func_80065264(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D60 = p[0];
    D_800F0D64 = p[1];
    p = (volatile s32 *)p[2];
    D_800F1118 = 1;
    D_800F0BC8 = 0;
    D_800F0D68 = (s32)p;
}
extern volatile s32 D_800A347C;
extern volatile s32 D_800F0D6C;
extern volatile s32 D_800F0D70;
extern volatile s32 D_800F0D74;
extern volatile s32 D_800F111C;
extern volatile s16 D_800F0BCA;
void func_800652AC(void) {
    volatile s32 *p = (volatile s32 *)D_800A347C;
    D_800F0D6C = p[0];
    D_800F0D70 = p[1];
    p = (volatile s32 *)p[2];
    D_800F111C = 1;
    D_800F0BCA = 0;
    D_800F0D74 = (s32)p;
}
extern s16 D_800F0BA8;
u8 motion_SetExMotion(s32);
u8 func_800652F4(void) {
    u8 v0 = motion_SetExMotion(0);
    s16 *p = &D_800F0BA8;
    s16 v1 = *p;
    v1 += 0x1FF;
    *p = v1;
    if ((s16)v1 < 0x1001) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAA;
u8 func_80065344(void) {
    u8 v0 = motion_SetExMotion(1);
    s16 *p = &D_800F0BAA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAC;
u8 func_80065394(void) {
    u8 v0 = motion_SetExMotion(2);
    s16 *p = &D_800F0BAC;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BAE;
u8 func_800653E4(void) {
    u8 v0 = motion_SetExMotion(3);
    s16 *p = &D_800F0BAE;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB0;
u8 func_80065434(void) {
    u8 v0 = motion_SetExMotion(4);
    s16 *p = &D_800F0BB0;
    s16 v1 = *p;
    v1 += 0x19;
    *p = v1;
    if ((s16)v1 < 0xC9) {
        return v0;
    }
    return 0;
}
extern s32 *D_800A3484;
s32 func_80065484(void) {
    unsigned int temp_v1;
    s32 ret;
    *D_800A3484 = (s32)*(s16 *)&D_800A3440;
    ret = motion_SetExMotion(5);
    temp_v1 = *D_800A3484;
    switch (temp_v1) {
    case 0: {
        u16 *p = (u16 *)&D_800F0BB2;
        *p = *p + 0x1FF;
        break;
    }
    case 1: {
        u16 *p = (u16 *)&D_800F0BB2;
        *p = *p + 0x3FE;
        break;
    }
    case 2: {
        u16 *p = (u16 *)&D_800F0BB2;
        *p = *p + 0x5FD;
        break;
    }
    }
    if (*(s16 *)&D_800F0BB2 < 0x2001) {
        return ret & 0xFF;
    }
    return 0;
}
extern s16 D_800F0BB4;
u8 func_80065540(void) {
    u8 v0 = motion_SetExMotion(6);
    s16 *p = &D_800F0BB4;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BB6;
u8 func_80065590(void) {
    u8 v0 = motion_SetExMotion(7);
    s16 *p = &D_800F0BB6;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBC;
u8 func_800655E0(void) {
    u8 v0 = motion_SetExMotion(0xA);
    s16 *p = &D_800F0BBC;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BBE;
u8 func_80065630(void) {
    u8 v0 = motion_SetExMotion(0xB);
    s16 *p = &D_800F0BBE;
    s16 v1 = *p;
    v1 += 0x32;
    *p = v1;
    if ((s16)v1 < 0x100) {
        return v0;
    }
    return 0;
}
extern s32 motion_SetExMotion(s32);
extern u16 D_800F0BC0;
extern s16 D_800F0BC4;
s32 func_80065680(void) {
    u16 *v1;
    s32 v0;
    motion_SetExMotion(0xC);
    v1 = &D_800F0BC0;
    v0 = *v1 + 1;
    *v1 = v0;
    v0 = motion_SetExMotion(0xE);
    D_800F0BC4 = D_800F0BC4 + 1;
    if ((s16)D_800F0BC4 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern u16 D_800F0BC2;
extern u16 D_800F0BC6;
s32 func_800656EC(void) {
    u16 *s0 = &D_800F0BC2;
    s32 v0;
    motion_SetExMotion(0xD);
    *s0 = *s0 + 1;
    v0 = motion_SetExMotion(0xF);
    D_800F0BC6 = D_800F0BC6 + 1;
    if ((s16)*s0 < 11) {
        return v0 & 0xFF;
    }
    return 0;
}
extern s16 D_800F0BC8;
u8 func_80065760(void) {
    u8 v0 = motion_SetExMotion(0x10);
    s16 *p = &D_800F0BC8;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
extern s16 D_800F0BCA;
u8 func_800657B0(void) {
    u8 v0 = motion_SetExMotion(0x11);
    s16 *p = &D_800F0BCA;
    s16 v1 = *p;
    v1 += 0x1C6;
    *p = v1;
    if ((s16)v1 < 0x11C8) {
        return v0;
    }
    return 0;
}
u8 motion_SetExMotion(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/motion_SetExMotion.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
    return 0;
}
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EC0(void) {
    u8 ret = func_80067200(0, 0, 0);
    D_800F10D8 = 1;
    return ret;
}
extern s32 D_800F10D8;
u8 func_80067200(s32, s32, s32);
u8 func_80066EF4(void) {
    u8 ret = func_80067200(0, 0, 1);
    D_800F10D8 = 2;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F28(void) {
    u8 ret = func_80067200(1, 1, 0);
    D_800F10DC = 1;
    return ret;
}
extern s32 D_800F10DC;
u8 func_80067200(s32, s32, s32);
u8 func_80066F5C(void) {
    u8 ret = func_80067200(1, 1, 1);
    D_800F10DC = 2;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066F90(void) {
    u8 ret = func_80067200(2, 1, 0);
    D_800F1120 = 1;
    return ret;
}
extern s32 D_800F1120;
u8 func_80067200(s32, s32, s32);
u8 func_80066FC4(void) {
    u8 ret = func_80067200(2, 1, 1);
    D_800F1120 = 2;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_80066FF8(void) {
    u8 ret = func_80067200(3, 0, 0);
    D_800F1124 = 1;
    return ret;
}
extern s32 D_800F1124;
u8 func_80067200(s32, s32, s32);
u8 func_8006702C(void) {
    u8 ret = func_80067200(3, 0, 1);
    D_800F1124 = 2;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067060(void) {
    u8 ret = func_80067200(4, 2, 0);
    D_800F1128 = 1;
    return ret;
}
extern s32 D_800F1128;
u8 func_80067200(s32, s32, s32);
u8 func_80067094(void) {
    u8 ret = func_80067200(4, 2, 1);
    D_800F1128 = 2;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670C8(void) {
    u8 ret = func_80067200(5, 3, 0);
    D_800F112C = 1;
    return ret;
}
extern s32 D_800F112C;
u8 func_80067200(s32, s32, s32);
u8 func_800670FC(void) {
    u8 ret = func_80067200(5, 3, 1);
    D_800F112C = 2;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067130(void) {
    u8 ret = func_80067200(6, 3, 0);
    D_800F1130 = 1;
    return ret;
}
extern s32 D_800F1130;
u8 func_80067200(s32, s32, s32);
u8 func_80067164(void) {
    u8 ret = func_80067200(6, 3, 1);
    D_800F1130 = 2;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_80067198(void) {
    u8 ret = func_80067200(7, 2, 0);
    D_800F1134 = 1;
    return ret;
}
extern s32 D_800F1134;
u8 func_80067200(s32, s32, s32);
u8 func_800671CC(void) {
    u8 ret = func_80067200(7, 2, 1);
    D_800F1134 = 2;
    return ret;
}
u8 func_80067200(s32 arg0, s32 arg1, s32 arg2) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80067200.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2;
    return 0;
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800676C8(void) {
    func_800678A8(0, 0);
    func_80067D14(0, 0);
    return func_80068D88(0, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067704(void) {
    func_800678A8(1, 1);
    func_80067D14(1, 1);
    return func_80068D88(1, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067740(void) {
    func_800678A8(2, 1);
    func_80067D14(2, 1);
    return func_80068D88(2, 1);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006777C(void) {
    func_800678A8(3, 0);
    func_80067D14(3, 0);
    return func_80068D88(3, 0);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677B8(void) {
    func_800678A8(4, 2);
    func_80067D14(4, 2);
    return func_80068D88(4, 2);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_800677F4(void) {
    func_800678A8(5, 3);
    func_80067D14(5, 3);
    return func_80068D88(5, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_80067830(void) {
    func_800678A8(6, 3);
    func_80067D14(6, 3);
    return func_80068D88(6, 3);
}
u8 func_800678A8(s32, s32);
u8 func_80067D14(s32, s32);
u8 func_80068D88(s32, s32);
u8 func_8006786C(void) {
    func_800678A8(7, 2);
    func_80067D14(7, 2);
    return func_80068D88(7, 2);
}
u8 func_800678A8(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800678A8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1;
    return 0;
}
u8 func_80067D14(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80067D14.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1;
    return 0;
}
u8 func_80068D88(s32 arg0, s32 arg1) {
    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3724;
    extern s32 D_800A34E4;
    extern s32 D_800A34E8;
    extern s32 D_800A374C;
    s32 outer = D_800A34EC;
    register s32 prev_init asm("$7") = D_800A37D4;
    s16 *p_idx = (s16 *)(outer + 0x6E);
    s32 *p_prev = (s32 *)(outer + 0x7C);
    s32 *p_cur = (s32 *)(outer + 0x80);
    s16 *p_matrix = (s16 *)(outer + 0x8C);
    register s32 cur_init asm("$5");
    s32 strength_red;
    s32 var_t3;
    (void)arg0; (void)arg1;

    D_800A3724 = outer + 0x1AC;
    cur_init = *p_cur;
    strength_red = -((cur_init - prev_init) * 0x33333333) >> 3;

    if (strength_red != 0) {
        s32 cur_loc;
        s32 prev_loc;

        *p_cur = prev_init;
        __asm__ volatile("" : : : "memory");
        cur_loc = *p_cur;
        *p_prev = cur_init;
        __asm__ volatile("" : : : "memory");
        prev_loc = *p_prev;
        var_t3 = 1;
        *p_idx = 0;

        if ((u32)cur_loc < (u32)prev_loc) {
            register s32 *p_a asm("$4");
            register s32 *p_b asm("$5");
            do {
                s32 idx_s = *p_idx;
                u32 entry = *(u16 *)((s32)p_matrix + idx_s * 2);
                p_a = (s32 *)(D_800A374C + (s32)(entry * 4));
                D_800A34E4 = (s32)p_a;
                p_b = (s32 *)*p_cur;
                D_800A34E8 = (s32)p_b;
                *p_b = (*p_b & 0xFF000000) | (*p_a & 0xFFFFFF);

                p_a = (s32 *)D_800A34E4;
                *p_a = (D_800A34E8 & 0xFFFFFF) | (*p_a & 0xFF000000);

                *p_cur += 0x28;
                *(u16 *)p_idx = *(u16 *)p_idx + 1;
            } while ((u32)*p_cur < (u32)*p_prev);
        }
        D_800A37D4 = *p_prev;
    } else {
        var_t3 = 0;
    }

    return var_t3;
}
void func_80068ECC(s32 arg0) {
    s32 *p = &D_8009BC04;
    register s32 v asm("$2");
    v = *p;
    v &= ~0x1; v |= arg0 & 0x1;
    v &= ~0x2; v |= arg0 & 0x2;
    v &= ~0x4; v |= arg0 & 0x4;
    asm volatile("" : "=r"(arg0) : "0"(arg0));
    v &= ~0x8; v |= ((u32)arg0 >> 1) & 0x8;
    asm volatile("" : "=r"(arg0) : "0"(arg0));
    v &= ~0x10; v |= ((u32)arg0 >> 1) & 0x10;
    asm volatile("" : "=r"(arg0) : "0"(arg0));
    v &= ~0x20; v |= ((u32)arg0 >> 1) & 0x20;
    v &= ~0x40; v |= (arg0 << 3) & 0x40;
    v &= ~0x80; v |= arg0 & 0x80;
    *p = v;
}
void func_80068F70(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80068F70.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A3524;
extern s32 D_800A34FC;
extern s32 D_800A372C;
extern s32 D_800A351C;
void func_8006E8CC(s32);
s32 *func_80069120(s32 a0) {
    s32 *v0 = (s32 *)D_800A3524;
    u8 *v1 = (u8 *)D_800A34FC;
    if (v1[0x30] != (v0[8] & 1)) {
        func_8006E8CC(D_800A372C);
    }
    v0 = (s32 *)D_800A3524;
    v1 = (u8 *)D_800A34FC;
    v1[0x30] = (u8)(v0[8] & 1);
    return (s32 *)((u8 *)D_800A351C + a0 * 44);
}

void func_8006920C(s32 *, s32);
s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);
s32 efc_rob_Close(s32 *a0) {
    s32 i = 0;
    s32 *p = &a0[5];
    do {
        func_8006920C(a0, *p);
        p++;
        i++;
    } while (i < 12);
    tslGlobalMemFree_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
void func_8006920C(s32 *a0, s32 a1) {
    s32 *p = (s32 *)a1;
    if (!*p) return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)a0;
        }
        p++;
    }
}
extern s32 func_8005C650();
extern void func_80069E18(s32, s32);
extern s32 func_8006E390();
extern s32 D_800A3514;
extern s32 D_800A3518;
s32 func_80069250(s32 arg0, s32 arg1) {
    s32 sp10[10];
    D_800A3514 = 0;
    func_8006E390(sp10, &D_800A3518);
    func_80069E18(sp10, 0);
    if ((arg1 & 0x400040) != 0) {
        func_8005C650(1, 0x7F, 0x7F);
        return 1;
    }
    return 0;
}
extern u32 D_800A32D0;
s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    register s32 sum asm("$10");
    s32 i;
    s32 a3_off;
    register s32 bitpos asm("$9");
    u32 *p;
    u32 maskA;
    u32 maskB;
    u32 v;
    s32 c;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    a3_off = 0;
    bitpos = 0;
    p = &D_800A32D0;

    do {
        s32 idx4;
        arg3 = (s16 *)((s32)arg3 + a3_off);
        v = i * 4;
        maskB = *p << arg1;
        idx4 = v;
        maskA = *(u32 *)((s32)&D_800A32C8 + idx4) << arg1;
        if (*arg2 == 0) {
            v = *arg0;
            if (v & maskA) {
                *arg3 = 1;
            } else if (v & maskB) {
                c = -1;
                *arg3 = c;
            }
        } else {
            v = *arg0;
            if (v & maskA) {
                c = 6;
                *arg2 = c;
            } else if (v & maskB) {
                c = -6;
                *arg3 = c;
            }
            sval = *arg2;
            if (sval >= 6) {
                sum += 1 << bitpos;
                *arg3 = 0;
                *arg2 = 0;
            } else if (sval < -5) {
                c = 2;
                sum += c << bitpos;
                *arg3 = 0;
                *arg2 = 0;
            }
        }
        a3_off += 2;
        bitpos += 0x10;
        p++;
        i++;
        *arg2 = (u16)*arg2 + (u16)*arg3;
        arg2++;
    } while (i < 2);

    return sum;
}
s32 func_800693CC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800693CC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
    return 0;
}
extern void initTile(u8 *p);
extern void gpu_SetSemiTransp(u8 *p, s32 semi);
extern void ot_Link(u32 *ot, u32 *prim);
void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2) {
    u8 *p = (u8 *) arg0->field_18;

    initTile(p);
    p[4] = 0xFF;
    p[5] = 0xFF;
    p[6] = 0xFF;
    p[4] = 0x80;
    p[5] = 0x80;
    p[6] = 0x80;
    *(u16 *)(p + 8)  = arg1[0];
    *(u16 *)(p + 10) = arg1[1];
    *(u16 *)(p + 12) = arg1[2];
    *(u16 *)(p + 14) = arg1[3];
    gpu_SetSemiTransp(p, 0);
    ot_Link((u32 *) D_800A374C + arg2, (u32 *)p);
    p += 0x10;

    initTile(p);
    p[4] = 0x80;
    p[5] = 0x80;
    p[6] = 0x80;
    *(u16 *)(p + 8)  = arg1[0];
    *(u16 *)(p + 10) = arg1[1] - 1;
    *(u16 *)(p + 12) = arg1[2];
    *(u16 *)(p + 14) = arg1[3];
    gpu_SetSemiTransp(p, 1);
    ot_Link((u32 *) D_800A374C + arg2, (u32 *)p);
    p += 0x10;

    initTile(p);
    p[4] = 0x40;
    p[5] = 0x40;
    p[6] = 0x40;
    *(u16 *)(p + 8)  = arg1[0];
    *(u16 *)(p + 10) = arg1[1] - 2;
    *(u16 *)(p + 12) = arg1[2];
    *(u16 *)(p + 14) = arg1[3];
    gpu_SetSemiTransp(p, 1);
    ot_Link((u32 *) D_800A374C + arg2, (u32 *)p);
    p += 0x10;

    arg0->field_18 = (s32) p;
}
s32 *saTan2GaugeInit_80077D00(void);
void func_80069A30(u8 *a0) {
    s32 *p = saTan2GaugeInit_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 0x22;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x4C;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
        v0 = 0x6C;
    }
    a0[6] = (u8)v0;
}
s32 *saTan2GaugeInit_80077D00(void);
void func_80069A8C(u8 *a0) {
    s32 *p = saTan2GaugeInit_80077D00();
    s32 v0;
    if (p[8] & 1) {
        v0 = 8;
        a0[4] = (u8)v0;
        a0[5] = (u8)v0;
    } else {
        v0 = 0x31;
        a0[4] = 0;
        a0[5] = 0;
    }
    a0[6] = (u8)v0;
}
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40;
} S_69AE4;

extern s32 saMotionSet(s32, s32);
extern s32 func_8007352C(s32);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern void initPolyF4(u8 *p);
extern void func_80069A8C(u8 *p);

void func_80069AE4(s32 *arg0, s32 mode, s32 unused_arg) {
    u8 *p;
    u8 *poly;
    s32 *qbase;
    s32 *q;
    s32 i;
    S_69AE4 s;

    p = (u8 *)arg0[6];

    if (mode == 2) {
        initTile(p);
        func_80069A30(p);
        *(s16 *)(p + 8)  = 0x4E;
        *(s16 *)(p + 10) = 0x30;
        *(s16 *)(p + 12) = 0xCC;
        *(s16 *)(p + 14) = 0xB0;
        gpu_SetSemiTransp(p, 1);
        ot_Link((u32 *)(D_800A374C + 0x44), (u32 *)p);
        p += 0x10;
        initTile(p);
        func_80069A30(p);
        *(s16 *)(p + 8)  = 0x166;
        *(s16 *)(p + 10) = 0x30;
        *(s16 *)(p + 12) = 0xCC;
        *(s16 *)(p + 14) = 0xB0;
    } else if (mode == 1) {
        initTile(p);
        func_80069A30(p);
        *(s16 *)(p + 8)  = 0x3F;
        *(s16 *)(p + 10) = 0x30;
        *(s16 *)(p + 12) = 0x202;
        *(s16 *)(p + 14) = 0xB0;
    } else {
        initTile(p);
        func_80069A30(p);
        *(s16 *)(p + 8)  = 0x130;
        *(s16 *)(p + 10) = 0x3A;
        *(s16 *)(p + 12) = 0x126;
        *(s16 *)(p + 14) = 0xAB;
    }
    gpu_SetSemiTransp(p, 1);
    ot_Link((u32 *)(D_800A374C + 0x44), (u32 *)p);
    p += 0x10;
    arg0[6] = (s32)p;

    s.sp2C = 0x12;
    s.sp40 = 0;
    s.sp28 = 0;
    qbase = *(s32 **)(arg0[1] + 0x34);
    s.sp30 = 0;
    s.sp34 = 0;
    q = qbase;
    i = 0;
    do {
        s32 v = *q;
        q++;
        s.sp18 = v;
        s.sp1C = v + 0xC;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)&s.sp18);
        i++;
    } while (i < 3);

    {
        s32 first = qbase[0];
        s.sp18 = first;
        initTexPage(arg0[7], 1, 0, saMotionSet(first, 0), 0);
    }
    ot_Link(D_800A374C + 0x48, arg0[7]);

    poly = (u8 *)arg0[3];
    arg0[7] += 0xC;
    initPolyF4(poly);
    func_80069A8C(poly);
    *(s16 *)(poly + 8)  = 0;
    *(s16 *)(poly + 10) = 0xB9;
    *(s16 *)(poly + 12) = 0x122;
    *(s16 *)(poly + 14) = 0;
    *(s16 *)(poly + 16) = 0;
    *(s16 *)(poly + 18) = 0xEF;
    *(s16 *)(poly + 20) = 0x122;
    *(s16 *)(poly + 22) = 0xEF;
    gpu_SetSemiTransp(poly, 0);
    ot_Link(D_800A374C + 0x4C, (s32)poly);
    poly += 0x18;

    initPolyF4(poly);
    func_80069A8C(poly);
    *(s16 *)(poly + 8)  = 0x15E;
    *(s16 *)(poly + 12) = 0x27F;
    *(s16 *)(poly + 10) = 0;
    *(s16 *)(poly + 14) = 0;
    *(s16 *)(poly + 16) = 0x15E;
    *(s16 *)(poly + 18) = 0xEF;
    *(s16 *)(poly + 20) = 0x27F;
    *(s16 *)(poly + 22) = 0x36;
    gpu_SetSemiTransp(poly, 0);
    ot_Link(D_800A374C + 0x4C, (s32)poly);
    poly += 0x18;

    initPolyF4(poly);
    func_80069A8C(poly);
    *(s16 *)(poly + 8)  = 0x122;
    *(s16 *)(poly + 10) = 0;
    *(s16 *)(poly + 12) = 0x15E;
    *(s16 *)(poly + 14) = 0;
    *(s16 *)(poly + 16) = 0x122;
    *(s16 *)(poly + 18) = 0xEF;
    *(s16 *)(poly + 20) = 0x15E;
    *(s16 *)(poly + 22) = 0xEF;
    gpu_SetSemiTransp(poly, 0);
    ot_Link(D_800A374C + 0x4C, (s32)poly);
    poly += 0x18;

    arg0[3] = (s32)poly;
}

typedef struct {
    s32 *p0;
    s32 *p1;
    s32 in_tex;
    s32 pad0C;
    s32 zero10;
    s32 arg2;
    s32 width;
    s32 zero1C;
    s32 pad20;
    s32 pad24;
    s8 byte28;
} S69E18;
void func_80069E18(s32 arg0) {
    extern s32 D_800A374C;
    s32 tile;
    s32 ptr;
    S69E18 s;
    s32 motion_v0;

    tile = *(s32 *)(arg0 + 0x18);
    initTile(tile);
    *(u8 *)(tile + 4) = 0xFF;
    *(u8 *)(tile + 5) = 0xFF;
    *(u8 *)(tile + 6) = 0xFF;
    *(s16 *)(tile + 0xC) = 0x280;
    *(s16 *)(tile + 8) = 0;
    *(s16 *)(tile + 0xA) = 0;
    *(s16 *)(tile + 0xE) = 0xF0;
    gpu_SetSemiTransp(tile, 0);
    ot_Link(D_800A374C + 0x50, tile);
    *(s32 *)(arg0 + 0x18) = tile + 0x10;

    ptr = *(s32 *)(*(s32 *)(arg0 + 4) + 0x14);
    s.arg2 = 0x10;
    s.zero10 = 0;
    s.width = 0;
    s.zero1C = 0;
    s.byte28 = 0;

    s.p0 = (s32 *)*(s32 *)ptr;
    initTexPage(*(s32 *)(arg0 + 0x1C), 1, 0, saMotionSet((s32)s.p0, 0), 0);
    ot_Link(D_800A374C + 0x44, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;

    {
        register s32 p0v asm("$2") = (s32)s.p0;
        register s32 p1v asm("$3") = p0v + 0xC;
        s.p1 = (s32 *)p1v;
        (void)p0v;
    }
    s.in_tex = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);

    s.p0 = (s32 *)*(s32 *)(ptr + 4);
    {
        register s32 p1v asm("$3") = (s32)s.p0 + 0xC;
        s.p1 = (s32 *)p1v;
    }
    s.in_tex = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);

    s.p0 = (s32 *)*(s32 *)(ptr + 8);
    {
        register s32 p1v asm("$3") = (s32)s.p0 + 0xC;
        s.p1 = (s32 *)p1v;
    }
    s.in_tex = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
}
void func_80069F80(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80069F80.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006A1A0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006A1A0.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 saMotionSet(s32, s32);
extern s32 func_8007352C(s32);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
void func_8006A3CC(s32 *arg0, u8 *arg1) {
    s32 v;
    *(s32 *)(arg1 + 0) = *(s32 *)(*(s32 *)(*(s32 *)((u8 *)arg0 + 4) + 0x1C) + 0x10);
    v = *(volatile s32 *)(arg1 + 0);
    *(s32 *)(arg1 + 0x18) = 0;
    *(s32 *)(arg1 + 0x1C) = 0;
    *(s8 *)(arg1 + 0x28) = 0;
    *(s32 *)(arg1 + 0x10) = 0;
    *(s32 *)(arg1 + 0x14) = 1;
    *(s32 *)(arg1 + 4) = v + 0xC;
    *(s32 *)(arg1 + 8) = arg0[5];
    arg0[5] = func_8007352C((s32)arg1);
    initTexPage(arg0[7], 1, 0, saMotionSet(*(s32 *)(arg1 + 0), 0), 0);
    ot_Link(D_800A374C + 4, arg0[7]);
    arg0[7] += 0xC;
}
extern s32 func_80073728(s32, s32);
void func_8006A494(s32 *arg0, u8 *arg1) {
    s32 v;
    *(s32 *)(arg1 + 0) = *(s32 *)(*(s32 *)(*(s32 *)((u8 *)arg0 + 4) + 0x1C) + 0x24);
    v = *(volatile s32 *)(arg1 + 0);
    *(s32 *)(arg1 + 0x18) = 0;
    *(s32 *)(arg1 + 0x1C) = 0;
    *(s8 *)(arg1 + 0x28) = 0;
    *(s32 *)(arg1 + 0x10) = 0;
    *(s32 *)(arg1 + 0x14) = 1;
    *(s32 *)(arg1 + 0x20) = 0x100;
    *(s32 *)(arg1 + 0x24) = 0x100;
    *(s32 *)(arg1 + 4) = v + 0xC;
    *(s32 *)(arg1 + 0xC) = arg0[2];
    arg0[2] = func_80073728((s32)arg1, 0);
    initTexPage(arg0[7], 1, 0, saMotionSet(*(s32 *)(arg1 + 0), 0), 0);
    ot_Link(D_800A374C + 4, arg0[7]);
    arg0[7] += 0xC;
}
void saTan3GaugeMain_8006A564(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saTan3GaugeMain_8006A564.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006A880(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006A880.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006B120(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006B120.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006B578(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006B578.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A36AC;
extern u8 g_disp_fb_base;
void func_8006B898(s32 arg0, s32 arg1) {
    s32 sp10[10];
    u8 *t;
    D_800A3514 += 1;
    t = ((D_800A36AC & 1) * 0x4090) + &g_disp_fb_base;
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006B120(sp10);
    func_8006B578(&arg0, &arg1);
}
extern u32 D_800A34F8;
extern s32 D_800A350C;
s32 func_8006B92C(s32 *unused, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_v0;
    register u32 var_v1 asm("v1");
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = ((a0 >> 13) & 7) + 1;
            D_800A34F8 = var_v1 | ((var_v0 & 7) << 13);
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        } else {
            var_v1 = a0 & 0xFFFF1FFF;
            var_v0 = ((a0 >> 13) & 7) - 1;
            D_800A34F8 = var_v1 | ((var_v0 & 7) << 13);
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }

    idx = (D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) | (((((D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
    return var_s0;
}

void func_8006BB68(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006BB68.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
typedef unsigned char u8_lcl;
typedef signed char s8_lcl;

extern s32 D_800A34FC;
extern s32 D_800A36E0;
extern s32 D_800A36E4;
extern s32 D_800A374C;
extern s32 saMotionSet();
extern s32 func_8007352C();
extern void initTexPage();
extern void ot_Link();

typedef struct { s32 a; s32 b; } _Pair_BD28;

void func_8006BD28(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    s32 i, j, n;
    s32 v;
    s32 base;
    s32 offset;
    _Pair_BD28 *p;

    offset = arg0 * 8;
    base = *(s32 *)(*(s32 *)((s32)D_800A34FC + 0x24) + 0x20);

    *(s8 *)(arg2 + 0x2B) = 0x30;
    *(s8 *)(arg2 + 0x2A) = 0x30;
    *(s8 *)(arg2 + 0x29) = 0x30;

    i = 0;
    do {
        s32 idx = i * 4 + base;
        __asm__ __volatile__("" : "=r"(idx) : "0"(idx));
        v = *(s32 *)(idx + offset);
        *(s32 *)(arg2 + 0) = v;
        if (v == -1) return;

        n = 1;
        if (arg0 == 0x12) {
            n = 3;
        }

        j = 0;
        if (n != 0) {
            do {
                if (arg0 == 0x12 && j != arg3 && j != 2) {
                    *(s8 *)(arg2 + 0x28) = 1;
                } else {
                    *(s8 *)(arg2 + 0x28) = 0;
                }
                *(s32 *)(arg2 + 0x18) = 0;
                *(s32 *)(arg2 + 0x1C) = arg1;
                *(s32 *)(arg2 + 0x14) = 8;
                *(s32 *)(arg2 + 0x10) = 0;
                *(s32 *)(arg2 + 8) = D_800A36E4;
                p = (_Pair_BD28 *)((s32)*(s32 *)(arg2 + 0) + 0xC);
                *(s32 *)(arg2 + 4) = (s32)&p[j];
                D_800A36E4 = func_8007352C(arg2);
                j++;
            } while (j < n);
        }

        initTexPage(D_800A36E0, 1, 0, saMotionSet(*(s32 *)(arg2 + 0), 0), 0);
        ot_Link(D_800A374C + 0x20, D_800A36E0);
        i++;
        D_800A36E0 += 12;
    } while (i < 2);
}
void func_8006BEC4(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006BEC4.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1;
}
extern void func_8006BB68(s32);
extern s32 func_8006B92C();
void func_8006C168(s32 arg0, s32 arg1) {
    s32 sp10[22];
    u8 *t;
    D_800A3514 += 1;
    t = ((D_800A36AC & 1) * 0x4090) + &g_disp_fb_base;
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006BB68(sp10);
    func_8006B92C(&arg0, &arg1);
}
s32 func_8006C168(s32, s32);
s32 saTan2InfoInit_8006C1FC(s32 a0, s32 a1) {
    return func_8006C168(a0, a1);
}
void saTan4GaugeMain(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saTan4GaugeMain.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
extern s32 D_800A34FC;
extern s32 D_800A3524;

void func_8006CBD4(s32 arg0, s32 arg1) {
    s32 code;
    s16 i;
    s32 mask;

    if (arg1 & (0x10 << (arg0 * 16))) {
        code = 1;
    } else if (arg1 & (0x40 << (arg0 * 16))) {
        code = 2;
    } else if (arg1 & (0x80 << (arg0 * 16))) {
        code = 3;
    } else if (arg1 & (0x20 << (arg0 * 16))) {
        code = 0;
    }

    mask = (1 << code) << (arg0 * 4);

    for (i = 0; i < 3; i++) {
        if (i == *(s16 *)((u8 *)D_800A34FC + (arg0 * 2) + 0x28)) {
            *((u8 *)D_800A3524 + i + 0x17) |= mask;
        } else {
            *((u8 *)D_800A3524 + i + 0x17) &= ~mask;
        }
    }
}
void func_8006CCC8(s32 arg0, s32 arg1, s32 arg2) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006CCC8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2;
}
s32 func_8006CFBC(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006CFBC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
    return 0;
}
extern s32 D_800A34FC;
void cpu_reset_dir(void) {
    s16 *v1 = (s16 *)D_800A34FC;
    v1[0x15] = 5;
    v1[0x14] = 5;
}
extern void saTan4GaugeMain(s32);
extern s32 func_8006CFBC(s32);
extern void func_8006CCC8(s32, s32, s32);
void func_8006D338(s32 arg0, s32 arg1) {
    s32 sp10[22];
    u8 *t;
    s32 r;
    D_800A3514 += 1;
    t = ((D_800A36AC & 1) * 0x4090) + &g_disp_fb_base;
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 2, t);
    saTan4GaugeMain(sp10);
    r = func_8006CFBC(sp10);
    func_8006CCC8(&arg0, &arg1, (s32)((r << 16) >> 16));
}
void func_8006D3DC(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006D3DC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
extern s32 D_800A350C;
extern void *D_800A3524;
extern u16 D_800A3528;
s32 func_8006D5D4(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 result = 0;
    s32 ret;
    s32 *p;
    s32 v;
    s32 sval;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    if ((ret >> 16) == 1) {
        D_800A3528 = D_800A3528 + 1;
        func_8005C650(0, 0x7F, 0x7F);
    } else if ((ret >> 16) == 2) {
        D_800A3528 = D_800A3528 - 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    if ((s16) D_800A3528 < 0) {
        D_800A3528 = 2;
    }
    D_800A3528 = (s16) D_800A3528 % 3;
    if (arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        result = -1;
    } else if (arg1 & 0x400040) {
        func_8005C650(1, 0x7F, 0x7F);
        sval = (s16) D_800A3528;
        if (sval == 2) {
            result = -1;
        } else {
            p = (s32 *)((s32)D_800A3524 + 0x14);
            v = *p;
            *p = (v & 0xFFFDFFFF) | ((sval & 1) << 17);
            result = 1;
        }
    }
    return result;
}
extern s32 D_800A3514;
extern s32 D_800A3518;
extern s32 D_800A36AC;
extern s32 D_800F7438;
extern s32 func_8006E390(s32, s32);
extern s32 func_80069AE4(s32, s32, s32);
extern void func_8006D3DC(s32);
extern s32 func_8006D5D4(s32, u32);
extern s32 func_8005C6D0(void);
s32 func_8006D74C(s32 arg0, s32 arg1) {
    s32 sp_buf[22];
    s32 result;
    s32 ptr_offset;
    D_800A3514 += 1;
    ptr_offset = ((D_800A36AC & 1) * 0x4090) + (s32)&D_800F7438;
    func_8006E390((s32)&sp_buf[0], (s32)&D_800A3518);
    func_80069AE4((s32)&sp_buf[0], 1, ptr_offset);
    func_8006D3DC((s32)&sp_buf[0]);
    result = func_8006D5D4(arg0, arg1);
    func_8005C6D0();
    return result;
}
extern s32 D_800A352C;
s32 func_8006D7FC(void) {
    D_800A352C = 0;
    return 1;
}
void func_8006D808(s32 *arg0, s32 *arg1, s32 *arg2, s32 arg3, s32 arg4) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006D808.s).
     * 355 inst, 39 branches, 14 jal, 56 loads, 65 stores. m2c output had
     * type errors. Pure-C decomp pending. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)arg4;
}
void func_8006DD94(s32 arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006DD94.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
extern s32 func_800692C0();
extern s32 D_800A350C;

s32 func_8006DF68(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 ret;
    s32 result = 0;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    if (((ret >> 16) & 0xFF) != 0) {
        D_800A352C += 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    D_800A352C &= 1;
    if ((arg1 & 0x100010) != 0) {
        result = -1;
        func_8005C650(2, 0x7F, 0x7F);
    } else if ((arg1 & 0x400040) != 0) {
        func_8005C650(1, 0x7F, 0x7F);
        if (D_800A352C != 0) {
            result = -1;
        } else {
            result = 1;
        }
    }
    func_8005C6D0();
    return result;
}
extern void func_8006DD94(s32);
extern s32 func_8006DF68();
void func_8006E068(s32 arg0, s32 arg1) {
    s32 sp10[22];
    u8 *t;
    D_800A3514 += 1;
    t = ((D_800A36AC & 1) * 0x4090) + &g_disp_fb_base;
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006DD94(sp10);
    func_8006DF68(arg0, arg1);
}
void func_8006E10C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006E10C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A3518;
extern s32 D_800F7438;
extern u8 D_800A32E0[8];
extern void func_8007B9B0(s32);
extern void func_8007BC08(s32);
extern void func_8007B4D0(s32, s32, s32, s32);
extern void gpu_SetDispMask(s32);
extern void gpu_DrawSync(s32);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
s32 func_8006E2A8(void) {
    u8 rect[8];
    s32 base;
    gpu_SetDispMask(0);
    base = ((D_800A3518 & 1) * 0x4090) + (s32)&D_800F7438;
    func_8007B9B0(base);
    base = ((D_800A3518 & 1) * 0x4090) + (s32)&D_800F7438 + 0x5C;
    func_8007BC08(base);
    gpu_DrawSync(0);
    __builtin_memcpy(rect, D_800A32E0, 8);
    func_8007B4D0((s32)rect, 0, 0, 0);
    gpu_DrawSync(0);
    disp_SetFramebufferMode(1, 0, 0, 0);
    gpu_SetDispMask(1);
    return 1;
}
extern s32 D_800A34FC;
extern s32 D_800A3520;
s32 *func_80069120(s32);
void func_8006E390(s32 *a0, s32 *a1) {
    s32 *s0 = a0;
    s32 *v0;
    a1[0]++;
    v0 = func_80069120(a1[0] & 1);
    s0[1] = ((s32 *)D_800A34FC)[9];
    s0[3] = v0[1];
    s0[2] = v0[0];
    s0[4] = v0[2];
    s0[5] = v0[4];
    s0[6] = v0[3];
    s0[7] = v0[5];
    s0[8] = v0[6];
    D_800A3520 = (s32)v0;
    s0[9] = v0[7];
}

void func_8006E440(s32 *a0) {
    s32 *p = a0;
    if (*p == -1) return;
    while (*p != -1) {
        *p = *p + (s32)a0;
        p++;
    }
}
s32 saMotionSet(u8 *a0, s32 a1) {
    s32 v0 = a0[0] & 0xFE1F;
    s32 v1 = a0[1] << 7;
    return v0 + v1 + a1;
}
s32 func_8006E49C(s32 arg0, s32 *arg1) {
    s32 a;
    s32 b;
    int new_var2;
    int new_var;
    s32 c;
    arg1[0] = arg0;
    a = (new_var = arg0 + 0x9C40);
    arg1[2] = a + 0x5DC0;
    arg1[1] = a;
    b = a + 0x6838;
    arg1[3] = a + 0x61F8;
    arg1[5] = b + 0x1B58;
    arg1[6] = b + 0x1DB0;
    arg1[7] = b + 0x1E28;
    arg1[8] = b + 0x1EA0;
    arg1[4] = b;
    arg1[0xB] = b + 0x1FB0;
    new_var2 = 0x1FB0;
    a = (b + new_var2) + 0x9C40;
    arg1[0xD] = a + 0x5DC0;
    arg1[0xC] = a;
    c = a + 0x6838;
    arg1[0xE] = a + 0x61F8;
    arg1[0x10] = c + 0x1B58;
    arg1[0x11] = c + 0x1DB0;
    arg1[0x12] = c + 0x1E28;
    arg1[0x13] = c + 0x1EA0;
    arg1[0xF] = c;
    return c + new_var2;
}
void func_8006E534(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006E534.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A35AC;
s32 func_8006E8AC(s32 a0) {
    return D_800A35AC + a0 * 44;
}
s32* saTan2GaugeInit_80077D00(void);
void gpu_DrawSync(s32);
void gpu_LoadImage(s16*, s32);
void func_8006E8CC(s32 *a0) {
    s32 *p;
    s32 data;
    s16 rect[4];
    p = saTan2GaugeInit_80077D00();
    if (p[8] & 1) {
        data = a0[4];
    } else {
        data = a0[3];
    }
    rect[0] = 0;
    rect[1] = 0x1E0;
    rect[2] = 0x280;
    rect[3] = 0x20;
    gpu_DrawSync(0);
    gpu_LoadImage(rect, data);
    gpu_DrawSync(0);
}
void func_8006E950(s32 *a0, s32 *a1) {
    s32 *s1 = a1;
    s32 s2;
    s32 s3;
    s32 s0;
    s32 s0_addr;
    s32 v0;
    s16 rect[4];

    s0_addr = (s32)a0;
    game_FrameLoop();
    v0 = func_80036EA8(2, s0_addr);
    replay_camera_Init(v0, (s32)s1);
    game_FrameLoop();
    s2 = 0x280;
    func_8006E440(s1);

    s3 = ((s32 *)((unsigned char *)s1 + 8))[0];
    s0 = 0x1DC;

    rect[0] = (s16)s2;
    rect[1] = 0;
    rect[2] = 0x180;
    rect[3] = (s16)s0;
    gpu_DrawSync(0);
    gpu_LoadImage(rect, s3);

    rect[2] = 0x170;
    rect[0] = (s16)s2;
    rect[1] = (s16)s0;
    rect[3] = 0x24;
    gpu_DrawSync(0);
    gpu_LoadImage(rect, s3 + 0x59400);

    func_8006E8CC(s1);
}
void func_8006920C(s32 *, s32);
s32 tslGlobalMemFree_8005C2A8(s32, s32, s32);
s32 efc_buki_ZanzouClose(s32 *a0) {
    func_8006920C(a0, a0[21]);
    func_8006920C(a0, a0[22]);
    func_8006920C(a0, a0[23]);
    func_8006920C(a0, a0[24]);
    func_8006920C(a0, a0[25]);
    func_8006920C(a0, a0[26]);
    func_8006920C(a0, a0[27]);
    func_8006920C(a0, a0[28]);
    func_8006920C(a0, a0[29]);
    tslGlobalMemFree_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
extern s32 D_8009BC1C;
extern s32 D_800A3548;
extern s32 D_800A354C;
extern s16 D_800A3580;
extern s32 D_800A35A0;
extern void *D_800A35A4;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern s32 D_800A35C0;
extern void *D_800A35C4;
void func_8006EC0C(void);
void func_8006F528(s32 *);
s32 func_8006EACC(s32 arg0, s32 arg1) {
    s32 sp10[10];
    s32 *temp_v0;
    s32 temp_v1;

    D_800A35C0 = ((D_800A36AC & 1) * 0x4090) + (s32)&D_800F7438;
    D_800A3548 = arg0;
    D_800A354C = arg1;
    if (D_800A35BC == 2) {
        D_800A354C = arg1 & 0xFFFF;
    }
    func_8006EC0C();
    temp_v1 = ((s32 *)D_800A35C4)[3] + 1;
    ((s32 *)D_800A35C4)[2] = ((s32 *)D_800A35C4)[2] + 1;
    ((s32 *)D_800A35C4)[3] = temp_v1;
    temp_v0 = (s32 *)func_8006E8AC(temp_v1 & 1);
    sp10[0] = D_800A35A8;
    sp10[1] = temp_v0[0];
    sp10[3] = temp_v0[2];
    sp10[4] = temp_v0[4];
    sp10[5] = temp_v0[3];
    sp10[6] = temp_v0[5];
    sp10[7] = temp_v0[6];
    D_800A35A4 = temp_v0;
    sp10[8] = temp_v0[7];
    sp10[9] = temp_v0[8];
    if ((u16)(D_800A3580 - 2) >= 2U) {
        func_8006F528(sp10);
    }
    ((void (*)(s32 *))(&D_8009BC1C)[D_800A3580])(sp10);
    return D_800A35A0;
}
extern s16 D_800A3570;
extern u16 D_800A3578;
extern s16 D_800A3584;
void func_8006EC0C(void) {
    s32 state = D_800A3578;  /* lhu gp-rel; regfix converts to lbu at idx 1 */

    if (state == 2) goto fade_out;
    if (state < 3) {
        if (state == 1) goto ramp_up;
        goto done;
    }
    if (state == 3) goto ramp_up;
    if (state == 4) goto fade_out;
    goto done;

ramp_up:
    D_800A3570 = (s16)(D_800A3570 + 0x20);
    if ((s32)(s16)D_800A3570 < 0x1E8) goto done;
    {
        s16 v3584 = D_800A3584;
        u16 word = D_800A3578;
        D_800A3570 = 0x1E8;
        D_800A3580 = v3584;
        if ((word >> 8) != 0) goto done;
        D_800A3578 = word + 1;
    }
    goto done;

fade_out:
    if (D_800A3570 == 0x1E8) {
        func_8005C650(5, 0x7F, 0x7F);
    }
    D_800A3570 = (s16)(D_800A3570 - 0x20);
    if ((s32)(s16)D_800A3570 > 0) goto done;
    D_800A3570 = 0;
    D_800A3578 = 0;

done: ;
}
void func_8006ECF4(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006ECF4.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern volatile u16 D_800A3550;
extern s32 gpu_SetSemiTransp(s32, s32);
extern s32 initTile(s32);
void func_8006F038(s32 arg0) {
    s32 temp_s0;
    s32 v;

    temp_s0 = *((s32 *)(((s32)arg0) + 0x14));
    initTile(temp_s0);
    v = (s8)D_800A3550;
    *((s16 *)(((s32)temp_s0) + 8)) = 0;
    *((s8 *)(((s32)temp_s0) + 4)) = v;
    *((s16 *)(((s32)temp_s0) + 0xA)) = 0;
    v = (s8)D_800A3550;
    *((s8 *)(((s32)temp_s0) + 5)) = v;
    *((s16 *)(((s32)temp_s0) + 0xC)) = 0x280;
    v = (s8)D_800A3550;
    *((s8 *)(((s32)temp_s0) + 6)) = v;
    *((s16 *)(((s32)temp_s0) + 0xE)) = 0xF0;
    gpu_SetSemiTransp(temp_s0, 1);
    {
        s32 saved = temp_s0;
        ot_Link(D_800A374C, temp_s0);
        temp_s0 = saved + 0x10;
    }
    *((s32 *)(((s32)arg0) + 0x14)) = temp_s0;
    initTexPage(*((s32 *)(((s32)arg0) + 0x18)), 1, 0, 0x40, 0);
    ot_Link(D_800A374C, *((s32 *)(((s32)arg0) + 0x18)));
    *((s32 *)(((s32)arg0) + 0x18)) = (s32)(*((s32 *)(((s32)arg0) + 0x18)) + 0xC);
}
void func_8006F100(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006F100.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006F528(s32 *arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006F528.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
void func_8006F97C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8006F97C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void replay_camera_attack(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/replay_camera_attack.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A3558;
extern u8 D_800A3560;
extern s16 D_800A3590;
extern s32 D_800A35A8;
extern s32 D_800A35B0;
extern s32 D_800A35BC;
extern s32 D_800A374C;
extern s32 func_8007352C(s32 *prim);
extern s32 saMotionSet(s32, s32);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 func_80069898(s32 a0, s32 *p, s32 mode);
extern void motion_ShiftControl(s32 a0, s32 *prim);
extern void func_8006ECF4(s32);
extern void func_80072E10(s32);
extern void saTan3GaugeMain_80073200(s32);

typedef struct PrimC70 {
    s32 p_geom;
    s32 p_static;
    s32 link;
    s32 pad0C;
    s32 zero10;
    s32 code;
    s32 mode;
    s32 zero1C;
    s32 width;
    s32 height;
    u8  byte28;
} PrimC70;

typedef struct IconC70 {
    s16 sp48;
    s16 sp4A;
    s16 sp4C;
    s16 sp4E;
} IconC70;

void func_80070C70(s32 arg0) {
    register s32 c60 asm("$20") = 0x60;
    PrimC70 prim;
    IconC70 icon;
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 var_s3;

    prim.zero10 = 0;
    prim.mode = 0;
    prim.zero1C = 0;
    prim.width = 0x100;
    prim.height = 0x100;
    prim.byte28 = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    prim.zero1C = 0;
    prim.mode = 0;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 4);
    var_s0 = 0;
    prim.p_static = prim.p_geom + 0xC;
    { s32 _c1; __asm__ __volatile__("addiu %0,$0,1" : "=r"(_c1)); prim.code = _c1; }
    prim.link = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    initTexPage(*(s32 *)(arg0 + 0x18), 1, 0, saMotionSet(prim.p_geom, c60), 0);
    ot_Link(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    icon.sp4C = 0xE7;
    icon.sp48 = 0xCC;
    icon.sp4A = 0x25;
    icon.sp4E = 1;
    func_80069898(arg0, (s32 *)&icon, 1);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    var_s3 = 0xA;
    prim.p_static = prim.p_geom + 0x48;
    do {
        prim.mode = var_s0 << 6;
        prim.code = var_s3;
        prim.link = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        var_s0 += 1;
        prim.p_geom += 0xC;
    } while (var_s0 < 6);
    prim.p_geom = *(s32 *)(ctx_or_var_s2);
    initTexPage(*(s32 *)(arg0 + 0x18), 1, 0, saMotionSet(prim.p_geom, c60), 0);
    ot_Link(D_800A374C + 0x28, *(s32 *)(arg0 + 0x18));
    var_s0 = 0;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    prim.p_geom = *(s32 *)(ctx_or_var_s2 + 8);
    if ((s32)(D_800A35B0 + ((s16)D_800A3558 + 1)) > 0) {
        var_s3 = 0x50;
        ctx_or_var_s2 = 0;
        do {
            u8 code = (&D_800A3560)[ctx_or_var_s2];
            s32 c5, c10, c2;
            s16 *p3590;
            __asm__ __volatile__("addiu %0,$0,5" : "=r"(c5));
            __asm__ __volatile__("addiu %0,$0,16" : "=r"(c10));
            __asm__ __volatile__("addiu %0,$0,2" : "=r"(c2));
            __asm__ __volatile__("la %0,D_800A3590" : "=r"(p3590));
            if ((code != c5) && (code != c10)) {
                s32 t = prim.p_geom + 0xC;
                prim.p_static = t;
                prim.p_static = t + (p3590[var_s0 * 2] << 4);
                if (((D_800A35B0 + (s16)(u16)D_800A3558) != 0) || (D_800A35BC == c2)) {
                    prim.mode = var_s3;
                } else {
                    s32 c105;
                    __asm__ __volatile__("addiu %0,$0,261" : "=r"(c105));
                    prim.mode = c105;
                }
                { s32 _c1; __asm__ __volatile__("addiu %0,$0,1" : "=r"(_c1)); prim.code = _c1; }
                prim.link = *(s32 *)(arg0 + 0x10);
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
            var_s3 += 0x16C;
            var_s0 += 1;
            ctx_or_var_s2 += 3;
        } while (var_s0 < (s32)(D_800A35B0 + ((s16)D_800A3558 + 1)));
    }
    initTexPage(*(s32 *)(arg0 + 0x18), 1, 0, saMotionSet(prim.p_geom, c60), 0);
    ot_Link(D_800A374C + 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    motion_ShiftControl(arg0, (s32 *)&prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    saTan3GaugeMain_80073200(arg0);
}
void motion_ShiftControl(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/motion_ShiftControl.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 D_800A3561;
extern u8 D_8009BC7C[];
s32 func_80071C20(void) {
    s32 v1;
    v1 = 3;
    if (D_8009BC7C[D_800A3561] & 2) {
        v1 = 9;
    }
    return v1;
}
void func_80071C4C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80071C4C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A35A8;
void func_800720FC(s32, s32, s32);
void func_80072084(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1A], 0);
}
void func_800720AC(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1B], 1);
}
void func_800720D4(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1C], 2);
}
void func_800720FC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800720FC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_800A374C;
extern s32 gpu_SetSemiTransp(GameObj *, s32);
extern s32 initPolyG4(GameObj *);
extern s32 ot_Link(s32, GameObj *);
extern void *D_800A35C4;
s32 func_80072BC4(s32 arg0, GameObj *arg1) {
    register u8 var_v0 asm("$2");
    int fc_const;

    initPolyG4(arg1);
    gpu_SetSemiTransp(arg1, 0);
    fc_const = 0xFC;
    if (arg0 < 4) {
        *(u8 *)((s32)(arg1) + 4) = 0;
        *(u8 *)((s32)(arg1) + 5) = 0;
        *(u8 *)((s32)(arg1) + 6) = 0;
        *(u8 *)((s32)(arg1) + 0xC) = fc_const;
        *(u8 *)((s32)(arg1) + 0xD) = 0x82;
        *(u8 *)((s32)(arg1) + 0xE) = 0;
        *(u8 *)((s32)(arg1) + 0x14) = fc_const;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 0x1D) = 0xC3;
            var_v0 = 0x1E;
        } else {
            *(u8 *)((s32)(arg1) + 0x1D) = 0xC3;
            var_v0 = 0x50;
        }
        *(u8 *)((s32)(arg1) + 0x1C) = fc_const;
        *(u8 *)((s32)(arg1) + 0x1E) = var_v0;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0;
        *(u8 *)((s32)(arg1) + 5) = 0;
        *(u8 *)((s32)(arg1) + 6) = 0;
        *(u8 *)((s32)(arg1) + 0xC) = 0x40;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x80;
        *(u8 *)((s32)(arg1) + 0x14) = 0x50;
        *(u8 *)((s32)(arg1) + 0x15) = 0xA0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x40;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x10;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x40;
        *(u8 *)((s32)(arg1) + 0x1E) = 0x80;
    }
    ot_Link(D_800A374C + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  u8 var_v0;
  s32 new_var;
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    var_v0 = 0x46;
    if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4)
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x1E;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xC8;
      do
      {
      }
      while (0);
      var_v0 = 0x32;
    }
    else
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x50;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xDC;
    }
    new_var = 0xFC;
    *((u8 *) (((s32) arg1) + 0xC)) = new_var;
    *((u8 *) (((s32) arg1) + 0xE)) = var_v0;
    *((u8 *) (((s32) arg1) + 0x14)) = new_var;
    do
    {
      *((u8 *) (((s32) arg1) + 4)) = 0xFC;
    }
    while (0);
    *((u8 *) (((s32) arg1) + 0x15)) = 0x82;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0x32;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0x28;
    *((u8 *) (((s32) arg1) + 0x16)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0xA;
  }
  else
  {
    new_var = (s32) arg1;
    *((u8 *) (new_var + 4)) = 0x10;
    *((u8 *) (((s32) arg1) + 5)) = 0x30;
    *((u8 *) (((s32) arg1) + 6)) = 0x60;
    *((u8 *) (((s32) arg1) + 0xD)) = 0;
    *((u8 *) (((s32) arg1) + 0xE)) = 0x40;
    *((u8 *) (((s32) arg1) + 0x14)) = 0x30;
    *((u8 *) (((s32) arg1) + 0x15)) = 0;
    *((u8 *) (((s32) arg1) + 0x16)) = 0x60;
    *((u8 *) (((s32) arg1) + 0xC)) = 0x18;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0;
  }
  ot_Link(D_800A374C + 0x60, arg1);
  return (s32) (((u8 *) arg1) + 0x24);
}
extern s32 func_80073060(s32);
extern s32 func_80072CD4(s32, GameObj *);
extern s16 D_800A3580;
void func_80072E10(s32 arg0) {
    GameObj *p;
    func_80073060(arg0);
    p = *(GameObj **)((s32)arg0 + 0xC);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x08) = 0x50;
    *(s16 *)((s32)p + 0x0A) = 0x32;
    *(s16 *)((s32)p + 0x10) = 0x50;
    *(s16 *)((s32)p + 0x12) = 0x52;
    *(s16 *)((s32)p + 0x18) = 0x140;
    *(s16 *)((s32)p + 0x1A) = 0x32;
    *(s16 *)((s32)p + 0x20) = 0x140;
    *(s16 *)((s32)p + 0x22) = 0x52;
    p = (GameObj *)func_80072BC4((s32)D_800A3580, p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x08) = 0x231;
    *(s16 *)((s32)p + 0x0A) = 0x32;
    *(s16 *)((s32)p + 0x10) = 0x231;
    *(s16 *)((s32)p + 0x12) = 0x52;
    *(s16 *)((s32)p + 0x18) = 0x140;
    *(s16 *)((s32)p + 0x1A) = 0x32;
    *(s16 *)((s32)p + 0x20) = 0x140;
    *(s16 *)((s32)p + 0x22) = 0x52;
    p = (GameObj *)func_80072BC4((s32)D_800A3580, p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x08) = 0x140;
    *(s16 *)((s32)p + 0x0A) = 0x52;
    *(s16 *)((s32)p + 0x10) = 0x140;
    *(s16 *)((s32)p + 0x12) = 0x71;
    *(s16 *)((s32)p + 0x18) = 0x50;
    *(s16 *)((s32)p + 0x1A) = 0x52;
    *(s16 *)((s32)p + 0x20) = 0x50;
    *(s16 *)((s32)p + 0x22) = 0x71;
    p = (GameObj *)func_80072CD4((s32)D_800A3580, p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x08) = 0x140;
    *(s16 *)((s32)p + 0x0A) = 0x52;
    *(s16 *)((s32)p + 0x10) = 0x140;
    *(s16 *)((s32)p + 0x12) = 0x71;
    *(s16 *)((s32)p + 0x18) = 0x231;
    *(s16 *)((s32)p + 0x1A) = 0x52;
    *(s16 *)((s32)p + 0x20) = 0x231;
    *(s16 *)((s32)p + 0x22) = 0x71;
    p = (GameObj *)func_80072CD4((s32)D_800A3580, p);
    *(GameObj **)((s32)arg0 + 0xC) = p;
}
void initTile(s32);
void gpu_SetSemiTransp(s32, s32);
extern s32 D_800A374C;
void ot_Link(s32, s32);
s32 *func_80072F30(s32 a0, u8 *a1) {
    initTile((s32)a1);
    if (a0 < 4) {
        a1[4] = 0x9E;
        a1[5] = 0x64;
        a1[6] = 0;
        gpu_SetSemiTransp((s32)a1, 1);
    } else {
        a1[4] = 0x28;
        a1[5] = 0x28;
        a1[6] = 0x18;
        gpu_SetSemiTransp((s32)a1, 0);
    }
    ot_Link(D_800A374C + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
extern s16 D_800A3580;
s32 *func_80072FCC(s32 ignored, u8 *a1) {
    initTile((s32)a1);
    if (D_800A3580 < 4) {
        a1[4] = 0x46;
        a1[5] = 0x24;
        a1[6] = 0x0A;
        gpu_SetSemiTransp((s32)a1, 1);
    } else {
        a1[4] = 0;
        a1[5] = 0;
        a1[6] = 0;
        gpu_SetSemiTransp((s32)a1, 0);
    }
    ot_Link(D_800A374C + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
void func_80073060(s32 arg0) {
    GameObj *p;
    s32 i;
    s32 j;
    p = *(GameObj **)((s32)arg0 + 0x14);
    j = 0x6F;
    do {
        if (D_800A3580) {}
        *(s16 *)((s32)p + 0x8) = j;
        j += 0x20;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xC) = 2;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    } while (j < 0x210);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xA) = 0x41;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 1;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xA) = 0x61;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 1;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    if (D_800A3580) {}
    *(s16 *)((s32)p + 0xA) = 0x50;
    *(s16 *)((s32)p + 0x8) = 0x51;
    *(s16 *)((s32)p + 0xC) = 0x1E0;
    *(s16 *)((s32)p + 0xE) = 2;
    p = (GameObj *)func_80072F30((s32)D_800A3580, (u8 *)p);
    i = 0;
    j = 0x6A;
    do {
        s32 v;
        *(s16 *)((s32)p + 0x8) = j;
        j += 0x21;
        if (D_800A3580) {}
        v = 5 - i;
        i += 1;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xC) = v;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072FCC((s32)D_800A3580, (u8 *)p);
    } while (i < 5);
    i = 0;
    j = 0x211;
    do {
        s32 v;
        *(s16 *)((s32)p + 0x8) = j;
        j -= 0x20;
        if (D_800A3580) {}
        v = 5 - i;
        i += 1;
        *(s16 *)((s32)p + 0xC) = v;
        *(s16 *)((s32)p + 0xA) = 0x32;
        *(s16 *)((s32)p + 0xE) = 0x3F;
        p = (GameObj *)func_80072FCC((s32)D_800A3580, (u8 *)p);
    } while (i < 5);
    *(GameObj **)((s32)arg0 + 0x14) = p;
}
void saTan3GaugeMain_80073200(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saTan3GaugeMain_80073200.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 initSprt(s32, s16);
extern s32 gpu_SetRawTexture(s32, s32);
extern s32 gpu_SetSemiTransp(s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 DispSleepMenuTex(s32 *, s32);
extern s32 gpu_CalcClut(u16, u16);
extern s32 D_800159A0;

typedef struct EnvA {
    s32 *header;
    s8  *table;
    s32  out;
    s32  pad0C;
    s32  semi;
    u32  ot_idx;
    s32  x;
    s32  y;
    s32  pad20, pad24;
    u8   has_color;
    u8   col_r;
    u8   col_g;
    u8   col_b;
} EnvA;

s32 func_8007352C(EnvA *_env) {
    register EnvA *env asm("s4") = _env;
    register s32 *hdr asm("s6") = env->header;
    register s32 buf asm("s5") = env->out;
    s32 fp;
    register s32 i asm("s7");
    register s32 dst asm("s3");
    s32 v_tail;
    s8 *e;
    s32 a1, x, y, v1;
    s32 raw;
    s32 saved_buf;

    {
        register s32 j asm("v1");
        raw = gpu_CalcClut(*((u16 *)hdr + 2), *((u16 *)hdr + 3));
        j = (s32)*((u8 *)hdr + 2) - 1;
        fp = raw & 0xFFFF;
        if (j < 0) goto exit;
        i = j;
    }
    dst = buf + 6;

loop_top:
    e = env->table + ((i << 16) >> 13);
    a1 = *(s16 *)(e + 0);
    x = a1 + env->x;
    y = *(s16 *)(e + 2) + env->y;
    {
        s32 x_check = x + (s32)*((u8 *)e + 6);
        v1 = y + (s32)*((u8 *)e + 7);
        if (x_check <= 0) goto next_compute_v_tail;
    }
    v_tail = i - 1;
    if (x >= 0x280) goto merge_v_tail;
    if (y >= 0xF0) goto next_compute_v_tail;
    v_tail = i - 1;
    if (v1 <= 0) goto merge_v_tail;
    initSprt(buf, (s16)a1);
    *(s16 *)(dst + 8) = (s16)fp;
    *(s16 *)(dst + 2) = (s16)x;
    *(s16 *)(dst + 4) = (s16)y;
    *(s8 *)(dst + 6) = (s8)((s32)*((u8 *)e + 4) + (s32)*((u8 *)hdr + 8));
    *(s8 *)(dst + 7) = (s8)((s32)*((u8 *)e + 5) + (s32)*((u8 *)hdr + 0xA));
    *(s16 *)(dst + 0xA) = (s16)*((u8 *)e + 6);
    *(s16 *)(dst + 0xC) = (s16)*((u8 *)e + 7);
    if (env->has_color != 0) {
        gpu_SetRawTexture(buf, 0);
        *((u8 *)dst - 2) = env->col_r;
        *((u8 *)dst - 1) = env->col_g;
        *((u8 *)dst + 0) = env->col_b;
    } else {
        gpu_SetRawTexture(buf, 1);
    }
    gpu_SetSemiTransp(buf, env->semi);
    if (env->ot_idx >= 0x1006U) {
        env->ot_idx = 1U;
        DispSleepMenuTex(&D_800159A0, buf);
    }
    saved_buf = buf;
    dst += 0x14;
    buf += 0x14;
    ot_Link(D_800A374C + env->ot_idx * 4, saved_buf);
next_compute_v_tail:
    v_tail = i - 1;
merge_v_tail:
    i = v_tail;
    if ((v_tail << 16) >= 0) goto loop_top;
exit:
    return buf;
}

s32 func_80073728(s32 arg0, s32 arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80073728.s).
     * 340 inst, 31 branches, 6 jal, 88 loads, 57 stores. m2c output had
     * structure-member errors. Pure-C decomp pending. */
    (void)arg0; (void)arg1;
    return 0;
}
void func_80073C78(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80073C78.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern void initPolyF4(s32);
extern s32 func_80069A8C(s32);
extern s32 func_8007352C(s32 *);

void func_80074220(s32 *arg0, s32 arg1) {
    s32 sp[12];
    s32 *p;
    s32 i;
    s32 *temp_s2;
    s32 v;
    s32 t;
    s32 q;
    s32 a3;

    if (arg1 != 0) goto skip_init;
    t = arg0[5];
    initTile(t);
    func_80069A30(t);
    *(s16 *)(t + 8) = 0x3F;
    *(s16 *)(t + 0xA) = 0x30;
    *(s16 *)(t + 0xC) = 0x202;
    *(s16 *)(t + 0xE) = 0xB0;
    gpu_SetSemiTransp(t, 1);
    ot_Link(D_800A374C + 0x78, t);
    arg0[5] = t + 0x10;
skip_init:
    sp[5] = 0x1F;
    *(s8 *)((s32)&sp[0] + 0x28) = 0;
    sp[4] = 0;
    temp_s2 = *(s32 **)((s32)arg0[0] + 0x38);
    sp[6] = 0;
    sp[7] = 0;
    p = temp_s2;
    i = 0;
    do {
        v = *p++;
        sp[0] = v;
        sp[1] = v + 0xC;
        sp[2] = arg0[4];
        arg0[4] = func_8007352C(sp);
        i++;
    } while (i < 3);

    {
        s32 _t = *temp_s2;
        sp[0] = _t;
        a3 = saMotionSet(_t, 0);
    }
    initTexPage(arg0[6], 1, 0, a3, 0);
    ot_Link(D_800A374C + 0x7C, arg0[6]);
    q = arg0[2];
    arg0[6] = arg0[6] + 0xC;
    initPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0x8) = 0;
    *(s16 *)(q + 0xA) = 0xB9;
    *(s16 *)(q + 0xC) = 0x122;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x14) = 0x122;
    *(s16 *)(q + 0x16) = 0xEF;
    gpu_SetSemiTransp(q, 0);
    ot_Link(D_800A374C + 0x80, q);
    q += 0x18;

    initPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0xC) = 0x27F;
    *(s16 *)(q + 0x14) = 0x27F;
    *(s16 *)(q + 0x8) = 0x15E;
    *(s16 *)(q + 0xA) = 0;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0x15E;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x16) = 0x36;
    gpu_SetSemiTransp(q, 0);
    ot_Link(D_800A374C + 0x80, q);
    q += 0x18;

    initPolyF4(q);
    func_80069A8C(q);
    *(s16 *)(q + 0x8) = 0x122;
    *(s16 *)(q + 0xA) = 0;
    *(s16 *)(q + 0xC) = 0x15E;
    *(s16 *)(q + 0xE) = 0;
    *(s16 *)(q + 0x10) = 0x122;
    *(s16 *)(q + 0x12) = 0xEF;
    *(s16 *)(q + 0x14) = 0x15E;
    *(s16 *)(q + 0x16) = 0xEF;
    gpu_SetSemiTransp(q, 0);
    ot_Link(D_800A374C + 0x80, q);
    q += 0x18;

    arg0[2] = q;
}
void func_80074488(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80074488.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_800747D8(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800747D8.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;

/* opaque - to satisfy GameObj * pointer-arg signatures without struct */
typedef struct GameObj { s32 dummy; } GameObj;
extern s32 gpu_SetSemiTransp(GameObj *, s32);
extern s32 initTile(GameObj *);
extern s32 ot_Link(s32, GameObj *);
extern u8 *D_800A36A0;
extern s32 D_800A374C;

void func_80074B18(s32 arg0, s32 arg1, s32 arg2) {
    s32 var_v1;
    s32 var_s3;
    s32 var_fp;
    s32 var_s4;
    s32 var_s5;
    s32 var_s7;
    register s32 var_s6 asm("s6");
    s32 var_s2;
    register s32 var_s1 asm("s1");
    register s32 var_s0 asm("s0");
    s32 v0;
    s32 a0_loc;

    var_v1 = 5;
    if (arg2 != 0) {
        var_v1 = 8;
    }
    a0_loc = (s32)D_800A36A0;
    var_s3 = *(s32 *)(arg0 + 0x14);
    var_fp = 0;
    if (((s32) *(u8 *)(a0_loc + 0x65)) + 3 != 0) {
        s32 sp28 = var_v1;
        s32 sp30 = (var_v1 > 0);
        var_s6 = ((arg1 << 4) - arg1) << 4;
        do {
            var_s2 = *(s32 *)(*(s32 *)(a0_loc + 4) + 0x3C);
            var_s4 = 0;
            if (sp30 != 0) {
                var_s5 = (((var_fp << 16) >> 16) << 4) + ((var_fp << 16) >> 16);
                var_s7 = var_s5 << 1;
                var_s1 = var_s2 + 2;
                var_s0 = var_s3 + 0xA;
                do {
                    initTile((GameObj *)var_s3);
                    *(u8 *)(var_s0 - 6) = *(u8 *)(var_s1 + 6);
                    *(u8 *)(var_s0 - 5) = *(u8 *)(var_s1 + 7);
                    *(u8 *)(var_s0 - 4) = *(u8 *)(var_s1 + 8);
                    *(u16 *)(var_s0 + 2) = *(u16 *)(var_s1 + 2);
                    *(u16 *)(var_s0 + 4) = *(u16 *)(var_s1 + 4);
                    gpu_SetSemiTransp((GameObj *)var_s3, 0);
                    if (arg2 != 0) {
                        *(s16 *)(var_s0 - 2) = *(u16 *)var_s2 + var_s6;
                        v0 = *(u16 *)var_s1 + var_s7 + 0x2B;
                    } else {
                        *(s16 *)(var_s0 - 2) = *(u16 *)var_s2 + var_s6;
                        v0 = *(u16 *)var_s1 + var_s5 + 0x7C;
                    }
                    *(s16 *)var_s0 = v0;
                    if (arg1 != 0) {
                        a0_loc = 0x15;
                    } else {
                        a0_loc = 0xB;
                    }
                    {
                        s32 a3_loc = var_s3;
                        var_s0 += 0x10;
                        var_s3 += 0x10;
                        var_s1 += 0xC;
                        var_s2 += 0xC;
                        ot_Link(D_800A374C + (a0_loc << 2), (GameObj *)a3_loc);
                    }
                    var_s4 += 1;
                } while ((s16)var_s4 < sp28);
            }
            var_fp += 1;
            a0_loc = (s32)D_800A36A0;
        } while ((s16)var_fp < (((s32) *(u8 *)(a0_loc + 0x65)) + 3));
    }
    *(s32 *)(arg0 + 0x14) = var_s3;
}
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40;
} S_80074D2C;

void func_80074D2C(s32 arg0, s32 arg1, s32 arg2) {
    S_80074D2C s;
    s32 var_s1;
    s32 sp18_val;
    s32 inner_ptr;

    var_s1 = 0xC;
    s.sp28 = 0;
    s.sp40 = 0;
    inner_ptr = *(s32 *)((s32)*(s32 *)arg0 + 0x1C);
    sp18_val = *(s32 *)(((arg2 << 16) >> 14) + inner_ptr);
    s.sp30 = arg1 * 0xF0;
    s.sp34 = 0;
    s.sp18 = sp18_val;
    s.sp1C = sp18_val + 0xC;
    if (arg1 != 0) {
        var_s1 = 0x16;
    }
    s.sp2C = var_s1;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    initTexPage(*(s32 *)(arg0 + 0x18), 1, 0, saMotionSet(s.sp18, 0), 0);
    ot_Link(D_800A374C + var_s1 * 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) += 0xC;
}
void func_80074E08(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80074E08.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 *D_800A36A0;
void func_8007526C(void) {
    register u8 *p asm("a0");
    register s32 i asm("a2");

    i = 0;
    p = D_800A36A0;
    do {
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = 0xC8;
                *(u16 *)(p + 0xC) = 0xC8;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
        p += 2;
    } while (i < 2);
}
void saTan1GaugeMain(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/saTan1GaugeMain.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 *D_800A36A0;
extern s16 D_800A35D0;
extern s32 func_8005C650(s32, s32, s32);
extern s32 func_800692C0();
void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    u8 *base;
    s16 *p;

    base = D_800A36A0;
    if (*(s32 *)(base + 0x10) != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, (s16 *)(base + (arg1 * 4 + 0x40)), (&D_800A35D0) + (arg1 * 2)) >> 16) != 0) {
        ;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
        *(u8 *)(D_800A36A0 + arg1 + 0x68) = *(u8 *)(D_800A36A0 + arg1 + 0x68) + 1;
        base = D_800A36A0;
        *(u8 *)(base + arg1 + 0x68) &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    base = D_800A36A0;
    p = p;
    *(s16 *)(base + arg1 * 2 + 0x3C) = *(u8 *)(base + arg1 + 0x68);
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            base = D_800A36A0;
            ;
            ((s16 *)(base + i * 2))[0x38 / 2] = 0;
            ((s16 *)(base + i * 2))[0x10 / 2] = 1;
            ((s16 *)(base + i * 2))[0x18 / 2] = 2;
        }
        base = D_800A36A0;
        *(u8 *)(base + ((arg1 + 1) & 1) + 0x68) = (*(u8 *)(base + arg1 + 0x68) + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (arg1 != 0) {
            p = (s16 *)(base + 0x14);
        } else {
            p = (s16 *)(base + 0x16);
        }
        if (*p == 1) {
            for (i = 0; i < 2; i++) {
                ;
                ((s16 *)(base + i * 2))[0x38 / 2] = 0;
                ((s16 *)(base + i * 2))[0x10 / 2] = 3;
                ((s16 *)(base + i * 2))[0x18 / 2] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
void func_80075830(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80075830.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_800759D0(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800759D0.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_80075F80(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80075F80.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8007636C(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_8007636C.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_800768DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_800768DC.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_80076D74(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80076D74.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
void func_8006920C(s32 *, s32);
s32 func_80076FF8(s32 *a0) {
    func_8006920C(a0, a0[5]);
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    func_8006920C(a0, a0[11]);
    func_8006920C(a0, a0[12]);
    func_8006920C(a0, a0[13]);
    func_8006920C(a0, a0[14]);
    return a0[1];
}
extern s32 D_800A35D8;
s32 func_80077098(s32 a0) {
    return D_800A35D8 + a0 * 44;
}

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
extern u8 *D_800A36A0;
extern s32 D_800A35D8;
extern s8 D_800A35DC;
extern u8 D_8009BCE4;
extern u8 D_8009BD21;
extern s16 D_800A35D0;
extern s32 D_800A374C;
extern s32 func_8007B844(s32, s32);
extern s32 snd_StopAll(void);
extern s32 func_8006E950(s32, s32 *);
extern s32 func_80076FF8(s32 *);
extern u8 *func_8006E49C(s32, s32);
void func_800770B8(s32 arg0_in, s32 arg1, long arg2)
{
  register s32 arg0 asm("$16") = arg0_in;
  u16 sp[2];
  s32 *p_old;
  u8 *p;
  s32 r;
  s16 t0;
  s16 a2;
  p_old = (s32 *) (arg0 + 0x58);
  __asm__ volatile("" : : "r"(arg1) : "memory");
  sp[0] = 0;
  sp[1] = 0;
  func_8007B844(D_800A374C, 0x1008);
  D_800A35D8 = arg0;
  snd_StopAll();
  func_8006E950(6, p_old);
  r = func_80076FF8(p_old);
  {
    s32 *prev = p_old;
    p_old = (s32 *) func_8006E49C(r, D_800A35D8);
    D_800A36A0 = (u8 *) p_old;
    *((s32 *) ((u8 *) p_old + 4)) = (s32) prev;
    *((s32 *) ((u8 *) p_old + 0x30)) = 0;
    *((s16 *) ((u8 *) p_old + 0x34)) = 0;
    p = (u8 *) p_old;
  }
  t0 = 0;
  a2 = 0;
  do
  {
    u8 *base = D_800A36A0;
    *((s16 *) ((base + (t0 * 2)) + 0x10)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x8)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0xC)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x14)) = 0;
    *((s16 *) ((base + (t0 * 2)) + 0x3C)) = 0;
    {
      s16 *p_d0 = &(&D_800A35D0)[t0 * 2];
      p_d0[1] = 0;
      p_d0[0] = 0;
    }
    *((s16 *) ((base + (t0 * 4)) + 0x42)) = 0;
    *((s16 *) ((base + (t0 * 4)) + 0x40)) = 0;
    *((u8 *) ((base + t0) + 0x68)) = (u8) t0;
    {
      s16 *p_6a = (s16 *) ((D_800A36A0 + (t0 * 10)) + 0x6A);
      s16 *p_7e = (s16 *) ((D_800A36A0 + (t0 * 10)) + 0x7E);
      do
      {
        p_6a[a2] = -1;
        p_7e[a2] = 0;
        a2 = (s16) (a2 + 1);
      }
      while (a2 < 5);
    }
    a2 = 0;
    *((s16 *) ((D_800A36A0 + (t0 * 2)) + 0x5C)) = 0;
    *((s16 *) ((D_800A36A0 + (t0 * 2)) + 0x60)) = 5;
    do
    {
      s16 idx = (s16) (a2 + (t0 * 10));
      (&D_8009BCE4)[idx] = (u8) ((&D_8009BCE4)[idx] & 0xF2);
      if ((arg2 & (1 << idx)) != 0)
      {
        (&D_8009BCE4)[idx] = (u8) ((&D_8009BCE4)[idx] | 1);
        sp[t0] += 1;
      }
      a2 = (s16) (a2 + 1);
    }
    while (a2 < 0xA);
    t0 = (s16) (t0 + 1);
    a2 = 0;
  }
  while (t0 < 2);
  p = D_800A36A0;
  *((s32 *) (p + 0x20)) = 0;
  *((s32 *) (p + 0x1C)) = 0;
  if (((s16) sp[0]) < ((s16) sp[1]))
  {
    *((s8 *) (p + 0x64)) = (s8) (((s16) sp[0]) - 3);
  }
  else
  {
    *((s8 *) (p + 0x64)) = (s8) (((s16) sp[1]) - 3);
  }
  if ((*((u8 *) (D_800A36A0 + 0x64))) >= 3)
  {
    *((u8 *) (D_800A36A0 + 0x64)) = 2;
  }
  p = D_800A36A0;
  *((s32 *) p) = arg1;
  *((s8 *) (p + 0x65)) = 0;
  p = D_800A36A0;
  *((u8 *) (p + 0x67)) = 1;
  p = D_800A36A0;
  *((u8 *) (p + 0x66)) = (&D_8009BD21)[(*((u8 *) (p + 0x67))) * 2];
  arg0 = 1;
  D_800A35DC = arg0;
}
void func_80077374(s32 arg0, s32 *arg1) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80077374.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1;
}
extern s32 D_800A36AC;
extern s32 *func_80077098(s32);
extern void func_80077374(s32, s32 *);
extern s32 *D_800A36A0;
extern s32 D_800F7438;
typedef struct {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
} S7724;
void func_80077724(s32 arg0, s32 arg1) {
    S7724 s;
    s32 *p;
    s32 temp_v1;
    *(s32 **)((s32)D_800A36A0 + 0x24) = (s32 *)(((D_800A36AC & 1) * 0x4090) + (s32)&D_800F7438);
    temp_v1 = *(s32 *)((s32)D_800A36A0 + 0x30) + 1;
    *(u16 *)((s32)D_800A36A0 + 0x34) = (u16)(*(u16 *)((s32)D_800A36A0 + 0x34) + 1);
    *(s32 *)((s32)D_800A36A0 + 0x30) = temp_v1;
    p = func_80077098(temp_v1 & 1);
    *(s32 **)((s32)D_800A36A0 + 0x2C) = p;
    s.sp10 = *(s32 *)((s32)D_800A36A0 + 4);
    s.sp14 = p[0];
    s.sp18 = p[1];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    s.sp34 = p[8];
    func_80077374(arg1, &s.sp10);
}
extern s32 D_800A35E4;
void func_80068F70(s32, s32 *);
extern s32 D_8009BD24;
s32 disp_SetFramebufferMode(s32, s32, s32, s32);
s32 func_80077820(s32 a0) {
    func_80068F70(a0, (s32 *)&D_8009BD24);
    disp_SetFramebufferMode(1, 0, 0, 0);
    D_800A35E4 = 0;
    return 1;
}

extern s32 D_800A35E4;
s32 func_80069250(void);
s32 func_80077860(void) {
    if (func_80069250() == 1) {
        D_800A35E4 = 0;
        return 1;
    }
    return 0;
}
s32 func_80077894(void) {
    register s32 ret asm("$16") = 0;
    register s32 result asm("$5") = func_800693CC();
    register s32 mask asm("$2");
    register s32 *p asm("$4");
    register s32 cur asm("$3");
    register s32 bits asm("$2");
    if (result < 0) goto fail;
    mask = -0x10;
    p = (s32 *)&D_8009BD38;
    ret = 1;
    cur = *p;
    D_800A35E4 = 0;
    cur = cur & mask;
    bits = result & 0xF;
    cur = cur | bits;
    *p = cur;
    goto end;
fail:
    if (result == -2) ret = -1;
end:
    return ret;
}
extern s32 D_800A35E0;
s32 func_80077904(void) {
    s32 i;

    D_800A35E4 = 0;
    i = (D_8009BD38 & 0xF) * 2;
    D_800A35E0 = *((u8 *)&D_8009BD59 + i);
    return *((u8 *)&D_8009BD58 + i);
}
extern s32 D_800A35E8;
void func_80077940(s32 arg0) {
    D_800A35E8 = (arg0 & 0x3FF) + ((u32) (arg0 & 0x3FF000) >> 2) + ((u32) (arg0 & 0x01000000) >> 4) + ((u32) (arg0 & 0x04000000) >> 5);
}
extern s32 D_800A35E0;
extern s32 D_800A35E8;
extern u8 D_8009BD24[];
void func_8006E534(s32, s32, u8*, s32);
s32 func_80077984(s32 a0) {
    func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8);
    disp_SetFramebufferMode(1, 0, 0, 0);
    return 1;
}
s32 func_8006EACC(void);
void func_8005B6FC(void);
s32 func_800779C8(void) {
    s32 ret = func_8006EACC();
    if (ret) {
        func_8005B6FC();
    }
    return ret;
}
extern s32 D_800A35E4;
void func_8006D74C(s32, s32);
void saTanMainDispGnd_80077A04(s32 a0, s32 a1) {
    D_800A35E4 = 0;
    func_8006D74C(a0, a1);
}
extern s32 D_800A35E4;
void disp_SetFramebufferMode(s32, s32, s32, s32);
s32 func_8006D7FC(void);
void func_80077A28(void) {
    D_800A35E4 = 0;
    disp_SetFramebufferMode(1, 0, 0, 0);
    func_8006D7FC();
}
void func_8006E068(void);
void saTan2InfoInit_80077A60(void) {
    func_8006E068();
}
extern s32 D_800A35E8;
extern s32 D_8009BD24;
void func_800770B8(s32, s32 *, s32);
s32 disp_SetFramebufferMode(s32, s32, s32, s32);
s32 func_80077A80(s32 a0) {
    func_800770B8(a0, (s32 *)&D_8009BD24, D_800A35E8);
    disp_SetFramebufferMode(1, 0, 0, 0);
    return 1;
}

void func_80077724(void);
void saTan2InfoInit_80077AC0(void) {
    func_80077724();
}
void func_8006E10C(void);
void func_80077AE0(void) {
    func_8006E10C();
}
void func_8006E2A8(void);
void func_80077B00(void) {
    func_8006E2A8();
}
extern s32 D_800A35E4;
void saTan2GaugeInit_80077B20(void) {
    D_800A35E4 = 1;
}
void func_80077B30(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80077B30.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern s32 D_8009BD24;
s32* saTan2GaugeInit_80077D00(void) {
    return &D_8009BD24;
}
void func_8006920C(s32*, s32);
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
extern s32 D_800A35F4;
extern s32 D_800A35F0;
extern s32 D_800A35F8;
extern s32 D_800A35FC;
extern s32 D_800A3600;
s32 func_80077D74(s32 a0) {
    return D_800A35F4 + a0 * 44;
}
void func_80077D94(s32 *arg0) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80077D94.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0;
}
s32 camera_get_rot_normal_rad(s32 arg0) {
    s32 s0;
    s32 r;

    func_8007B844(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A35F4 = arg0;
    D_800A35F8 = s0;
    func_8006E950(0x32, s0);
    r = func_80077D10(s0);
    func_8006E49C(r, D_800A35F4);
    D_800A35FC = 0;
    D_800A35F0 = 0;
    D_800A3600 = 0;
    return 1;
}
extern s32 *func_80077D74(s32);
extern void func_80077D94(s32 *);
extern s32 D_800A35F0;
extern s32 *D_800A35F8;
extern s32 D_800A35FC;
typedef struct {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
} S855C;
s32 func_8007855C(s32 arg0) {
    S855C s;
    s32 *p;
    s32 c;
    c = D_800A35FC + 1;
    D_800A35FC = c;
    p = func_80077D74(c & 1);
    s.sp14 = p[0];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    if (D_800A35F0 > 0 && (arg0 & 0x40)) {
        return 1;
    }
    func_80077D94(&s.sp10);
    {
        s32 *p_struct = *(s32 **)((s32)D_800A35F8 + 0x34);
        s32 new_val = D_800A35F0 + 1;
        int cond = new_val < *(s16 *)((s32)p_struct + 0xA);
        D_800A35F0 = new_val;
        return cond ? 0 : 1;
    }
}
s32 func_80078628(s32 *a0) {
    return a0[1];
}
extern s32 D_800A360C;
s32 func_80078634(s32 a0) {
    return D_800A360C + a0 * 44;
}
extern s32 saMotionSet(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 initTexPage(s32, s32, s32, s32, s32);
extern s32 ot_Link(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 D_800A374C;

typedef struct {
    s32 a;       /* sp18 - 0x00 */
    s32 b;       /* sp1C - 0x04 */
    s32 c;       /* sp20 - 0x08 */
    s32 d;       /* sp24 - 0x0C - unused */
    s32 e;       /* sp28 - 0x10 */
    s32 f;       /* sp2C - 0x14 */
    s32 g;       /* sp30 - 0x18 */
    s32 h;       /* sp34 - 0x1C */
    s32 i;       /* sp38 - 0x20 - unused */
    s32 j;       /* sp3C - 0x24 - unused */
    u8 cd_flag;  /* sp40 - 0x28 */
    u8 r;        /* sp41 - 0x29 */
    u8 g_;       /* sp42 - 0x2A */
    u8 b_;       /* sp43 - 0x2B */
} S78654;

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 v;
    s32 *var_s0;
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s.cd_flag = 1;
            v = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if ((s32)(v << 16) < 0) {
                v = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) v));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C(&s.a);
        initTexPage(arg0[5], 1, 0, saMotionSet(s.a, zero), 0);
        ot_Link(D_800A374C + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    initTexPage(arg0[5], 1, 0, saMotionSet(s.a, zero), 0);
    ot_Link(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}
extern s32 D_800A3610;
extern s32 D_800A3614;
extern s32 D_800A3304;
extern s32 D_800A3608;
extern s32 func_80078628(s32);
s32 func_80078824(s32 arg0) {
    s32 s0;
    s32 r;

    func_8007B844(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A360C = arg0;
    D_800A3610 = s0;
    func_8006E950(0x5F, s0);
    r = func_80078628(s0);
    func_8006E49C(r, D_800A360C);
    D_800A3304 = 0;
    D_800A3608 = 0;
    D_800A3614 = 0;
    disp_SetFramebufferMode(1, 0, 0, 0);
    return 1;
}
extern s32 D_800A3304;
extern s32 D_800A3608;
s32 *func_80078634(s32);
void func_80078654(s32 *);
s32 func_800788B0(void) {
    s32 buf[8];
    s32 *v0;
    D_800A3304++;
    v0 = func_80078634(D_800A3304 & 1);
    buf[0] = v0[0];
    buf[2] = v0[2];
    buf[3] = v0[4];
    buf[4] = v0[3];
    buf[5] = v0[5];
    buf[6] = v0[6];
    buf[7] = v0[7];
    func_80078654(buf);
    D_800A3608++;
    return D_800A3608 >= 0xB40;
}

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078948\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel func_80078948\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078948 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078958\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x70\n"
    "endlabel func_80078958\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078958 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078968\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9F\n"
    "endlabel func_80078968\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078968 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078978\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x8\n"
    "endlabel func_80078978\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078978 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078988\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9\n"
    "endlabel func_80078988\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078988 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078998\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xB\n"
    "endlabel func_80078998\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078998 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789A8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xC\n"
    "endlabel func_800789A8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789A8 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel EnterCriticalSection\n"
    "    addiu  $a0,$zero,1\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel ExitCriticalSection\n"
    "    addiu  $a0,$zero,2\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789D8\n"
    "    addu  $v0,$sp,$zero\n"
    "    jr  $ra\n"
    "    addu  $sp,$a0,$zero\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789D8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789E8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x32\n"
    "endlabel func_800789E8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789E8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800789F8\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x34\n"
    "endlabel func_800789F8\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789F8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A08\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x35\n"
    "endlabel func_80078A08\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A08 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A18\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x36\n"
    "endlabel func_80078A18\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A18 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A28\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x41\n"
    "endlabel func_80078A28\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A28 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A38\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x42\n"
    "endlabel func_80078A38\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A38 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A48\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel func_80078A48\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A48 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078A58\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x5B\n"
    "endlabel func_80078A58\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078A58 */
s32 func_80078A68(s32 arg0, s32 arg1, s32 arg2) {
    s32 a3;
    s32 t0;
    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
    a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    base = (t0 * 0x10) + D_8009BD6C;
    *((s16 *) (base + 4)) = 0;
    *((s16 *) (base + 8)) = (s16) arg1;
    if (((u32) t0) < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        v0 = arg2 & 0x1000;
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else {
        v0 = arg2 & 0x1000;
        if (t0 == 2) {
            ;
            if (!(arg2 & 1)) {
                a3 = 0x248;
            }
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    *((s16 *) (((t0 * 0x10) + D_8009BD6C) + 4)) = a3;
    return 1;
}
s32 func_80078B04(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    s32 result;
    if (v < 3) {
        result = *(u16 *)(D_8009BD6C + v * 0x10);
    } else {
        result = 0;
    }
    return result;
}
s32 func_80078B3C(s32 arg0) {
    s32 v;
    s32 *base;
    v = arg0 & 0xFFFF;
    base = D_8009BD68;
    base[1] = base[1] | (&D_8009BD70)[v];
    return v < 3;
}
s32 func_80078B70(s32 arg0) {
    s32 v;
    s32 *base;
    v = arg0 & 0xFFFF;
    base = D_8009BD68;
    base[1] = base[1] & ~(&D_8009BD70)[v];
    return 1;
}
s32 func_80078BA8(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    s32 result = 1;
    if (v < 3) {
        *(volatile u16 *)(D_8009BD6C + v * 0x10) = 0;
    } else {
        result = 0;
    }
    return result;
}
extern s32 D_8009BD80;
void func_80078BE0(s32 a0) {
    D_8009BD80 = a0;
}
extern s32 D_8009BD80;
s32 func_80078BF0(void) {
    return D_8009BD80;
}
void func_800790C0(void);
void EnterCriticalSection(void);
void func_80078F88(void);
void ExitCriticalSection(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F30(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C00(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    EnterCriticalSection();
    func_80078F88();
    ExitCriticalSection();
    func_80078A58(0);
    func_80078DA0();
    func_80078F30(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_800790C0(void);
void EnterCriticalSection(void);
void func_80078F88(void);
void ExitCriticalSection(void);
void func_80078A58(s32);
s32 func_80078DA0(void);
void func_80078F00(s32, s32, s32, s32);
void func_80079028(void);
extern s32 D_8009BD80;
void func_80078C9C(s32 a0, s32 a1, s32 a2, s32 a3) {
    func_800790C0();
    EnterCriticalSection();
    func_80078F88();
    ExitCriticalSection();
    func_80078A58(0);
    func_80078DA0();
    func_80078F00(a0, a1, a2, a3);
    func_80079028();
    D_8009BD80 = 1;
}
void func_80078F10(void);
void func_80078A58(s32);
void func_80078F60(void);
void func_80078D38(void) {
    func_80078F10();
    func_80078A58(0);
    func_80078F60();
}
extern s32 D_8009BD80;
void func_80078F74(void);
void func_80078F20(void);
s32 func_80078E20(void);
void pad_Init(void) {
    func_80078F74();
    func_80078F20();
    func_80078E20();
    D_8009BD80 = 0;
}
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void func_80078F50(s32, u32 *);
extern void func_80078F40(s32, u32 *);
extern void func_80078E58(void);
extern s32 func_80078EC0(void);
extern u32 D_800F183C;
extern u32 D_800F1840;
extern u32 D_800F1838;
extern u32 D_800F1844;
s32 func_80078DA0(void) {
    u32 *v1 = &D_800F183C;
    u32 *s0 = v1 - 1;
    EnterCriticalSection();
    *v1 = (u32)func_80078E58;
    D_800F1840 = (u32)func_80078EC0;
    D_800F1838 = 0;
    D_800F1844 = 0;
    func_80078F50(1, s0);
    func_80078F40(1, s0);
    ExitCriticalSection();
    return 1;
}
void EnterCriticalSection(void);
void func_80078F50(s32, s16*);
void ExitCriticalSection(void);
extern s16 D_800F1838;
s32 func_80078E20(void) {
    EnterCriticalSection();
    func_80078F50(1, &D_800F1838);
    ExitCriticalSection();
    return 1;
}
s32 func_80078E58(void) {
    volatile s32 i;
    s32 pad[2];
    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;
    i = 10;
    i = i - 1;
    if (i != -1) {
        do {
            i = i - 1;
        } while (i != -1);
    }
    return 0;
}
s32 func_80078EC0(void) {
    register s32 *p asm("$3") = (s32 *)D_8009BD88;
    register volatile s32 ret asm("$2") = 0;
    if ((p[1] & 1) == 0) goto end;
    if ((p[0] & 1) != 0) {
        ret = 1;
        goto end;
    }
    ret = 1;
    ret = 0;
end:
    return ret;
}
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F00\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x12\n"
    "endlabel func_80078F00\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F00 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F10\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x13\n"
    "endlabel func_80078F10\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F10 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F20\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x14\n"
    "endlabel func_80078F20\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F20 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F30\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x15\n"
    "endlabel func_80078F30\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F30 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F40\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x2\n"
    "endlabel func_80078F40\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F40 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F50\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x3\n"
    "endlabel func_80078F50\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_80078F50 */
extern void (*jtbl_800A3620)(void);
void func_80078F60(void) {
    jtbl_800A3620();
}
extern void (*jtbl_800A3624)(void);
void func_80078F74(void) {
    jtbl_800A3624();
}
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078F88\n"
    "    lui  $at,%hi(D_800A3618)\n"
    "    sw  $ra,%lo(D_800A3618)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $at,%hi(jtbl_800A3620)\n"
    "    addi  $v1,$v0,2180\n"
    "    sw  $v1,%lo(jtbl_800A3620)($at)\n"
    "    lui  $at,%hi(jtbl_800A3624)\n"
    "    addi  $v1,$v0,2196\n"
    "    addiu  $t1,$zero,11\n"
    "    sw  $v1,%lo(jtbl_800A3624)($at)\n"
    ".L80078FC4:\n"
    "    addiu  $t1,$t1,-1\n"
    "    sw  $zero,1428($v0)\n"
    "    bnez  $t1,.L80078FC4\n"
    "    addiu  $v0,$v0,4\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3618)\n"
    "    lw  $ra,%lo(D_800A3618)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80078FF0\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x44\n"
    "    nop\n"
    "    lui   $t1, %hi(D_800A362C)\n"
    "    lw    $t1, %lo(D_800A362C)($t1)\n"
    "    addiu $sp, $sp, -24\n"
    "    sw    $ra, 20($sp)\n"
    "    jalr  $t1\n"
    "    nop\n"
    "    lw    $ra, 20($sp)\n"
    "    addiu $sp, $sp, 24\n"
    "    jr    $ra\n"
    "    nop\n"
    "endlabel func_80078FF0\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_80079028\n"
    "    lui  $at,%hi(D_800A3628)\n"
    "    sw  $ra,%lo(D_800A3628)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $t2,%hi(func_800790A4)\n"
    "    lui  $t1,%hi(D_800790B4)\n"
    "    lui  $at,%hi(D_800A362C)\n"
    "    addi  $v1,$v0,1952\n"
    "    addiu  $t2,$t2,%lo(func_800790A4)\n"
    "    addiu  $t1,$t1,%lo(D_800790B4)\n"
    "    sw  $v1,%lo(D_800A362C)($at)\n"
    ".L80079064:\n"
    "    lw  $v1,0($t2)\n"
    "    addiu  $t2,$t2,4\n"
    "    sw  $v1,984($v0)\n"
    "    addiu  $v0,$v0,4\n"
    "    bne  $t2,$t1,.L80079064\n"
    "    sw  $v1,1244($v0)\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3628)\n"
    "    lw  $ra,%lo(D_800A3628)($ra)\n"
    "    lui  $v0,%hi(D_800A362C)\n"
    "    lw  $v0,%lo(D_800A362C)($v0)\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800790A4\n"
    "    and  $v0,$v0,$s5\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800790C0\n"
    "    lui  $at,%hi(D_800A3638)\n"
    "    sw  $ra,%lo(D_800A3638)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    addiu  $t2,$zero,9\n"
    "    addi  $v1,$v0,1580\n"
    ".L800790E8:\n"
    "    addiu  $t2,$t2,-1\n"
    "    sw  $zero,0($v1)\n"
    "    bnez  $t2,.L800790E8\n"
    "    addiu  $v1,$v1,4\n"
    "    jal  func_80078FF0\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3638)\n"
    "    lw  $ra,%lo(D_800A3638)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800790C0 */
u8* bb2_memcpy(u8 *dst, u8 *src, s32 len) {
    u8 *ret;
    if (!dst) {
        return 0;
    }
    ret = dst;
    while (len > 0) {
        *dst = *src;
        src++;
        len--;
        dst++;
    }
    return ret;
}
extern u32 D_800F1848;
s32 func_80079154(void) {
    D_800F1848 = D_800F1848 * 0x41C64E6D + 0x3039;
    return (D_800F1848 >> 16) & 0x7FFF;
}
extern s32 D_800F1848;
void func_80079184(s32 a0) {
    D_800F1848 = a0;
}
u8 *func_80079194(u8 *a0, u8 *a1) {
    u8 *v1;
    if (!a0) {
        return 0;
    }
    if (!a1) {
        return 0;
    }
    v1 = a0;
    while ((*a0++ = *a1++) != 0) {
    }
    return v1;
}
s32 func_800791D8(u8 *a0) {
    s32 v1 = 0;
    if (!a0) {
        return 0;
    }
    while (*a0++ != 0) {
        v1++;
    }
    return v1;
}
void debug_printf(s32 fmt, s32 a, ...) {
    func_80079244(1, fmt, &a);
}
void func_80079244(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80079244.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
extern u8 D_8009BD8D;
u8 func_800798CC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}
extern u8 D_8009BD8D;
u8 func_800798FC(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
u8 *func_8007992C(u8 *buf, s32 ch, s32 len) {
    if (buf == 0) return 0;
    if (len <= 0) return 0;
    len--;
    goto check;
found:
    return buf - 1;
check:
    if (len < 0) return 0;
    ch &= 0xFF;
loop:
    if (*buf == ch) goto found;
    ++buf;
    --len;
    if (len >= 0) goto loop;
    return 0;
}
void func_80078A08(s32, u8 *, s32);
void func_8007997C(s8 arg0) {
    u8 sp10;
    s32 temp_a0;

    sp10 = arg0;
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 == 9) goto loop;
    if (temp_a0 == 0xA) {
        func_8007997C(0xD);
        D_800F1850 = 0;
        goto tail;
    }
    goto def;
loop:
    func_8007997C(0x20);
    if ((D_800F1850 & 7) == 0) return;
    goto loop;
def:
    if ((&D_8009BD8D)[temp_a0] & 0x97) {
        D_800F1850 += 1;
    }
tail:
    func_80078A08(1, &sp10, 1);
}
void func_80079A30(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    /* Body replaced by asmfix replace_with_asmfile (asm/funcs/func_80079A30.s).
     * Pure-C decomp pending future purification work. */
    (void)arg0; (void)arg1; (void)arg2; (void)arg3;
}
