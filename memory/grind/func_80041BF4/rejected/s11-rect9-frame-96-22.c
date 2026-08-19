#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"


extern s16 D_800EED10[];
extern s32 D_800EED1C[];
extern s32 D_800EED18;
extern s32 D_800EED14;
extern s32 D_800EED00[];
extern s32 D_800A33AC;
extern s32 D_800A33A0;
extern s32 D_800A33A4;
extern s32 D_800A33A8;
extern u8 D_800A9D10;
extern void func_80049E1C(void);
extern void func_80052C10(void);
extern void func_80044098(s16);
extern void func_80044010(s32 *, s16);
extern s32 D_800A3240;
extern s32 D_800A3398;
extern s32 D_800A3244;
extern s16 D_800963EE;
extern void func_8003EDC0(s32 *, s32);
extern void func_80054410(s32 *);
extern s32 func_800457A0(s32);
extern void func_80041430(s32, s32);
extern s32 func_8004019C(s32 *, s32);
/* --- Functions 0x800401CC - 0x800466C0 (text1a segment, 126 funcs) --- */

extern s32 D_800A378C;
extern s32 D_800A3234;
extern s32 D_800A3378;
extern u8 D_800A9830;
extern u8 D_800A9920;
extern u16 D_80094AF4;
extern u8 D_80094B48[];
extern u8 D_80094D40[];
extern void SetDrawMove(s32, s16 *, s32, s32);

extern s32 g_player_ptrs[];
s32 func_8004153C(s32 a0) {
    return g_player_ptrs[a0];
}
s32 func_80041554(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        return ptr[4];
    }
    return -1;
}
s32 func_80041584(void) {
    s32 i;
    s32 ret = -1;
    for (i = 0; i < 3; i++) {
        if (g_player_ptrs[i] == 0) {
            ret = i;
            break;
        }
    }
    return ret;
}
extern void func_8004016C(s32);
extern void func_80045A50(s32);
void player_Destroy(s32 a0) {
    func_8004016C(a0);
    func_80045A50(a0);
    g_player_ptrs[a0] = 0;
}
extern s32 g_player_char_ids[];
void player_SetCharId(s32 a0, s32 a1) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        if ((val & 0x1F) != a1) {
            ptr[3] = -2;
        }
    }
    g_player_char_ids[a0] = a1;
}
s32 func_80041650(s32 a0) {
    s16 *ptr = (s16 *)g_player_ptrs[a0];
    if (ptr) {
        s32 val = ptr[1];
        return val & 0x1F;
    }
    return -1;
}
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void func_80041398(s32);

void func_80041688(s32 arg0, s32 arg1) {
    s32 *player;
    s32 i;
    u8 *p;
    u8 *q;
    s32 b, r, g, v;
    volatile s32 sp10[8];
    extern s32 func_800486FC(void);

    player = (s32 *)g_player_ptrs[arg0];
    if (player == NULL) return;

    p = (u8 *)player + 0x94;
    if (arg1) {
        p[1] |= 1;
    } else {
        p[1] &= ~1;
    }

    i = 1;
loop1:
    p += 0x68;
    if (*(s16 *)(p + 2) >= 0) {
        if (arg1) p[1] |= 1;
        else      p[1] &= ~1;
    }
    i++;
    if (i < 18) goto loop1;

    q = (u8 *)player + 0x10D5;
loop2:
    if (*(s32 *)(q + 0x57) == 0) goto after2;
    if (arg1) *q |= 1;
    else      *q &= ~1;
    q += 0x68;
    goto loop2;
after2:

    if (func_800486FC()) {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        v = func_8004881C(b, g, r);
        func_80041398((v << 16) | (v << 8) | v);
    } else {
        r = *((u8 *)player + 0x18);
        g = *((u8 *)player + 0x19);
        b = *((u8 *)player + 0x1A);
        func_80041398(b | ((r << 16) | (g << 8)));
    }
    (void)sp10;
}
typedef struct { s32 w[4]; } Block16;
extern s32 g_anim_func_table[];
extern void func_80052930(void *, void *, void *);
extern void func_80052A88(void *, void *, void *);
void func_800417D0(s32 *a0) {
    void (*func)(s32 *, s32 *);

    if (((s16 *)a0)[3] == 1) {
        return;
    }
    if (((s16 *)a0)[3] != 2) {
        func = (void (*)(s32 *, s32 *))g_anim_func_table[((s16 *)a0)[4]];
        func(a0 + 4, a0 + 14);
    }
    if ((s32 *)a0[3] != 0) {
        if (((s16 *)a0[3])[3] == 0) {
            func_800417D0((s32 *)a0[3]);
        }
        func_80052930((void *)((u8 *)((s32 *)a0[3]) + 0x18), (void *)((u8 *)a0 + 0x38), (void *)((u8 *)a0 + 0x18));
        func_80052A88((void *)((u8 *)((s32 *)a0[3]) + 0x18), (void *)((u8 *)a0 + 0x4C), (void *)((u8 *)a0 + 0x2C));
    } else {
        ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
        ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
    }
    ((s16 *)a0)[3] = 1;
}
typedef struct { s32 w[4]; } Block16;
extern s32 g_anim_func_table[];
void func_800418D0(s32 *a0) {
    s16 sp10[4];
    void (*func)(s16 *, s32 *);
    sp10[0] = -(u16)((u16 *)a0)[8];
    sp10[1] = -(u16)((u16 *)a0)[9];
    sp10[2] = -(u16)((u16 *)a0)[10];
    func = (void (*)(s16 *, s32 *))g_anim_func_table[((s16 *)a0)[4]];
    func(sp10, a0 + 14);
    ((Block16 *)(a0 + 6))[0] = ((Block16 *)(a0 + 14))[0];
    ((Block16 *)(a0 + 6))[1] = ((Block16 *)(a0 + 14))[1];
}
void func_80041988(s32 a0, s32 a1, s32 a2, s32 a3) {
    s32 mask_table;
    s32 bit;
    s32 i;
    s32 one;

    if ((u32)a0 >= 2) {
        return;
    }
    mask_table = D_80094D40[a1];
    bit = 0x10;
    i = 0;
    do {
        if (!(mask_table & bit) || !(a2 & bit)) {
            goto shift;
        }
        /* FAKE: constant-holder biasing RA (named-local-fake-exception,
         * SOTN `s16 three = 3;`). A literal `a0 == 1` materializes the 1 as a
         * compiler-generated pseudo (!REG_USERVAR_P) that loop.c move_movables
         * hoists to the preheader; reload then rematerializes it in-loop as $v1.
         * A named local (REG_USERVAR_P) is not movable, stays in-loop, and global
         * RA assigns $v0 — matching target's in-loop `addiu v0,zero,1`. The store
         * is live (feeds the compare). Measured exhaustion (memory/grind/func_80041988/):
         * literal if-chain = 2, switch(a0) = 2, holder = 0. */
        one = 1;
        if (a0 == 0) {
            goto case0;
        }
        if (a0 == one) {
            goto case1;
        }
        goto shift;
    case0:
        if (func_8003E2A0() == 0) {
            func_800480C0(a3, i + 1, 0, 0, -0x140, 0xE8);
        } else {
            func_80047EE8(a3, i + 1);
        }
        goto shift;
    case1:
        if (func_8003E2A0() == a0) {
            func_800480C0(a3, i + 1, 0x80, 0, -0x140, 0xE8);
        } else {
            func_80047FBC(a3, i + 1, 0x80, 0);
        }
    shift:
        bit >>= 1;
        i++;
    } while (i < 5);
}

extern s32 D_80094DF0[];
extern u8 D_80094E08[];
extern s16 D_800A9A20;
extern u16 D_800A9A24;
void func_80041AC8(s16 *arg0)
{
  s16 rect[4];
  s16 *var_s0;
  u16 *var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 v1_val;
  s16 *id_ptr;
  if (arg0[2] != 1)
  {
    return;
  }
  if (D_80094E08[arg0[4]] == 0xFF)
  {
    return;
  }
  /* id_ptr spelling: target's store->reload order (sh D_800A9A20 BEFORE
   * lh arg0[4]) is only producible when the reload is NOT spelled as plain
   * pointer-indexing (GCC 2.7.2 MEM_IN_STRUCT_P /s flag + sched.c escape
   * clause) — the original source provably used a non-indexed spelling.
   * One representative spelling sanctioned by user policy 2026-06-10; see
   * .claude/rules/proven-spelling-class-reconstruction.md. */
  id_ptr = &arg0[4];
  D_800A9A20 = arg0[4];
  var_s0 = (s16 *) D_80094DF0[D_80094E08[*id_ptr]];
  if (func_8003E2A0() != 1)
  {
    goto else_lbl;
  }
  var_s3 = -0x140;
  var_s2 = 0xF0;
  goto after_if;
  else_lbl:
  var_s3 = 0x80;

  var_s2 = 0;
  after_if:
  v1_val = (u16) (*var_s0);

  if ((*var_s0) >= 0)
  {
    s32 w = 0x10;
    s32 h = 1;
    var_s1 = &D_800A9A24;
    do
    {
      u16 v0_val;
      rect[0] = v1_val + var_s3;
      v0_val = (u16) var_s0[1];
      rect[2] = w;
      rect[3] = h;
      rect[1] = v0_val + var_s2;
      StoreImage(rect, var_s1);
      var_s0 += 2;
      var_s1 += 0x10;
      v1_val = (u16) (*var_s0);
    }
    while ((*var_s0) >= 0);
  }
  DrawSync(0);
}
extern void LoadImage(s32, s32);
extern void func_80048A7C(s16, s16, s32, s32, s32, s32);
extern s32 func_8003E2A0(void);
extern void func_8003E120(void);
void func_80041BF4(s32 a0, s32 a1, s32 a2)
{
  s32 *fp_ptr;
  s32 r;
  s32 g;
  s32 b;
  s32 outer;
  s32 xoff;
  s32 yoff;
  s16 *tbl;
  s32 idx;
  s32 x;
  int one;
  s16 rect[9];
  extern s32 func_800486FC(void);
  fp_ptr = (s32 *)func_8004153C(1);
  if (fp_ptr == 0) { return; }
  if ((*(((s16 *) fp_ptr) + 4)) != D_800A9A20) { return; }
  if (D_80094E08[*(((s16 *) fp_ptr) + 4)] == 0xFF) { return; }
  r = (a0 << 12) / 255;
  one = 1;
  g = (a1 << 12) / 255;
  b = (a2 << 12) / 255;
  if (func_800486FC()) {
    b = func_8004881C(r, g, b);
    g = b;
    r = b;
  }
  outer = 0;
  oloop:
  {
  if (outer == 0) {
    xoff = -0x140;
    yoff = 0xF0;
  } else {
    do { xoff = 0x80; } while (0);
    do { yoff = 0; } while (0);
  }
  tbl = *(s16 **)((u8 *) D_80094DF0 + (D_80094E08[*(((s16 *) fp_ptr) + 4)] << 2));
  idx = 0;
  goto test;
  again:
  {
    s32 off = idx << 5;
    idx++;
    rect[0] = x + xoff;
    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;
    rect[2] = 0x10;
    rect[3] = 1;
    LoadImage((s32)rect, (s32)((u8 *)&D_800A9A24 + off));
    DrawSync(0);
    tbl += 2;
    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);
  }
  test:
  x = *(u16 *) tbl;
  if ((s16) x >= 0) goto again;
  outer++;
  }
  if (outer < 2) goto oloop;
  if (func_8003E2A0() == one) { func_8003E120(); }
}
extern s16 g_anim_select[3];
extern Block16 D_800A9B28;
void func_80041E10(Block16 *a0, s32 a1) {
    g_anim_select[0] = (s16)((((a1 >> 16) & 0xFF) << 12) / 255);
    g_anim_select[1] = (s16)((((a1 >> 8) & 0xFF) << 12) / 255);
    g_anim_select[2] = (s16)(((a1 & 0xFF) << 12) / 255);
    D_800A9B28 = *a0;
}
extern s32 func_80052754(s32, s32, s32);
extern s32 ratan2(s32, s32);
extern s32 rcos(s32);
extern s32 rsin(s32);
extern s32 func_8004A1FC(s32);
extern s16 D_800F62E0[];
extern s16 D_800F6340[];
void func_80041EB0(s32 a0, s32 a1)
{
    s16 *fp_ptr;
    s32 outer;
    s32 *cam;
    s16 *tbl;
    s32 dx;
    s32 dy;
    s32 dz;
    s32 angle;
    s32 cos_val;
    s32 *ptr;

    fp_ptr = D_800F62E0;
    outer = 0;
    cam = (s32 *)&D_800A9B28;

    do {
        tbl = fp_ptr;
        if (outer == 0) {
            ptr = (s32 *)a0;
        } else {
            ptr = (s32 *)a1;
            tbl = D_800F6340;
        }

        if (ptr == 0) { goto skip; }
        if (g_anim_select[0] < 0) { goto skip; }

        dx = ptr[0] - cam[0];
        dy = ptr[1] - cam[1];
        dz = ptr[2] - cam[2];

        if (dx < 0) goto neg_dx;
        if (dx < 0x7000) goto check_dy;
        goto calc;
    neg_dx:
        if (-dx >= 0x7000) goto calc;
    check_dy:
        if (dy < 0) goto neg_dy;
        if (dy < 0x7000) goto check_dz;
        goto calc;
    neg_dy:
        if (-dy >= 0x7000) goto calc;
    check_dz:
        if (dz < 0) goto neg_dz;
        if (dz < 0x7000) goto dist_check;
        goto calc;
    neg_dz:
        if (-dz >= 0x7000) goto calc;

    dist_check:
        if (func_80052754(dx, dy, dz) > 0x17D7840) { goto skip; }

    calc:
        angle = ratan2(dx, dz);
        cos_val = rcos(angle);
        {
            s32 sin_val = rsin(angle);
            s32 cross = (cos_val * dz + sin_val * dx) >> 12;
            tbl[4] = (s16)-ratan2(dy, cross);
        }
        tbl[5] = (s16)angle;
        tbl[6] = 1;
        tbl[29] = g_anim_select[0];
        tbl[32] = g_anim_select[1];
        tbl[35] = g_anim_select[2];
        goto end_loop;

    skip:
        tbl[6] = 0;

    end_loop:
        outer++;
    } while (outer < 2);

    func_8004A1FC((s32)fp_ptr);
    func_8004A1FC((s32)D_800F6340);
}
extern s16 g_anim_counter;
extern s16 g_anim_select[3];
void func_800420D0(void) {
    extern s16 g_anim_hit_flags;
    g_anim_counter = 0;
    g_anim_hit_flags = 0;
    g_anim_select[0] = -1;
}
extern s16 g_anim_hit_flags[];
extern s32 g_anim_hit_data[];
void func_800420E8(s32 a0, s32 a1) {
    if (a0 < 2) {
        g_anim_hit_flags[a0] = 1;
        g_anim_hit_data[a0] = a1;
    }
}
extern s32 D_800A3388;
void func_8004211C(void) {
    s32 val = g_anim_hit_flags[0] * 2 + g_anim_counter;
    switch (val) {
    case 1:
        func_80041EB0(0, D_800A3388);
        break;
    case 2:
        func_80041EB0(g_anim_hit_data[0], 0);
        break;
    case 3:
        func_80041EB0(g_anim_hit_data[0], D_800A3388);
        break;
    }
}
extern void func_80041EB0(s32, s32);
void func_800421A4(void) {
    func_80041EB0(0, 0);
}
extern void func_800422BC(s32, s32, s32, s32);
extern void func_80042478(s32);
extern s16 D_800F6462;
extern s16 D_800F6342;
extern s16 D_800F62E2;
extern s16 D_800F6460;
extern s16 D_800F6340;
extern s16 D_800F62E0;
extern s32 StageLight[];
void func_800421C8(s32 a0) {
    s32 *p = (s32 *)((u8 *)StageLight + a0 * 24);
    s32 val;
    func_800422BC(a0, *p++, 0, 0);
    func_800422BC(a0, *p++, 0, 1);
    func_800422BC(a0, *p++, 1, 0);
    func_800422BC(a0, *p++, 1, 1);
    val = *p;
    D_800F6462 = val & 0xFFF;
    D_800F6342 = val & 0xFFF;
    D_800F62E2 = val & 0xFFF;
    val = *(s16 *)((u8 *)p + 2);
    D_800F6460 = val & 0xFFF;
    D_800F6340 = val & 0xFFF;
    D_800F62E0 = val & 0xFFF;
    func_80042478(*(s32 *)((u8 *)p + 4));
}
extern s16 D_800F6318[];
extern s16 D_800F631E;
extern s16 D_800F6324;
extern u8 D_800F6338;
extern u8 D_800F6339;
extern u8 D_800F633A;
extern s16 D_800F6378;
extern s16 D_800F637E;
extern s16 D_800F6384;
extern u8 D_800F6398;
extern u8 D_800F6399;
extern u8 D_800F639A;
extern s16 D_800F6498[];
extern s16 D_800F649E;
extern s16 D_800F64A4;
extern u8 D_800F64B8;
extern u8 D_800F64B9;
extern u8 D_800F64BA;
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void func_8004A1FC(s16 *);
void func_800422BC(s32 a0, s32 packed, s32 a2, s32 a3) {
    s32 r = (packed >> 16) & 0xFF;
    s32 g = (packed >> 8) & 0xFF;
    s32 b = packed & 0xFF;
    s16 r2;
    s16 g2;
    s16 *new_var;
    s16 b2;
    if (func_800486FC(a0)) {
        b = func_8004881C(r, g, b);
        g = b;
        r = b;
    }
    if (a3 != 0) {
        goto raw;
    }
    r2 = (r << 12) / 255;
    g2 = (g << 12) / 255;
    b2 = (b << 12) / 255;
    if (a2 != 0) {
        goto alt_scale;
    }
    new_var = D_800F6318;
    *new_var = r2;
    D_800F631E = g2;
    D_800F6324 = b2;
    func_8004A1FC((s16 *)(((u8 *)new_var) - 0x38));
    D_800F6378 = r2;
    D_800F637E = g2;
    D_800F6384 = b2;
    func_8004A1FC((s16 *)(((u8 *)new_var) + 0x28));
    goto out;
alt_scale:
    {
        s16 *p2 = D_800F6498;
        *p2 = r2;
        D_800F649E = g2;
        D_800F64A4 = b2;
        func_8004A1FC((s16 *)(((u8 *)p2) - 0x38));
    }
    goto out;
raw:
    if (a2 != 0) {
        goto alt_raw;
    }
    D_800F6338 = r;
    D_800F6339 = g;
    D_800F633A = b;
    D_800F6398 = r;
    D_800F6399 = g;
    D_800F639A = b;
    goto out;
alt_raw:
    D_800F64B8 = r;
    D_800F64B9 = g;
    D_800F64BA = b;
out:;
}
extern s32 func_800486FC(s32);
extern s32 func_8004881C(s32, s32, s32);
extern void disp_SetFramebufferMode(s32, s32, s32, s32);
extern void SetFarColor(s32, s32, s32);
void func_80042478(s32 a0) {
    s32 r = (a0 >> 16) & 0xFF;
    s32 g = (a0 >> 8) & 0xFF;
    s32 b = a0 & 0xFF;
    if (func_800486FC(a0)) {
        b = func_8004881C(r, g, b);
        g = b;
        r = b;
    }
    disp_SetFramebufferMode(1, r, g, b);
    SetFarColor(r, g, b);
}
