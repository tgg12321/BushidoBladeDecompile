#include "common.h"
#include "include_asm.h"
#include "game.h"

/* Forward declarations */
extern s32 stage_GetId(void);
extern void sys_StubEmpty3(s32, s32, s32);
extern void *func_8004153C(void);
extern void obj_ClearAll(void);
extern void sys_StubEmpty2(void);
extern void obj_Clear(s32);

/* Externs for globals */
extern s32 g_game_mode;
extern s32 g_game_player_count;
extern s32 g_game_flag_b;
extern s32 g_game_flag_a;
extern s32 g_stage_init_tbl;
extern void func_8001924C(s32 *, s32);
extern void func_80045A28(s32, s32);
extern void func_80052A20(s32 *, s32 *, s16 *);
extern void func_80052C10(void);

/* Externs for globals */
extern u8 g_char_data;
extern s16 g_game_p1_ctrl;
extern u8 g_stage_collision[];
extern s32 D_80094A6C[];
extern u8 *D_800A3708;
extern s32 g_stage_light_pos;
extern s32 D_800A93B4;
extern s32 D_800A93B8;
extern s32 g_stage_light_dir;
extern s32 D_800A93C0;
extern s32 D_800A93C4;

/* --- Functions 0x8003F168 - 0x8004019C --- */
void stage_ExecInitFunc(void) {
    s32 v0 = *(s32 *)((u32)&g_stage_init_tbl + (stage_GetId() << 3));
    if (v0) {
        (*(void (**)(void))((u32)&g_stage_init_tbl + (stage_GetId() << 3)))();
    }
}
s32 game_GetMode(void) {
    return g_game_mode;
}

void *game_GetCharData(void) {
    return &g_char_data;
}

void game_SetControllerPorts(s32 a0) {
    if (a0) {
        g_game_p1_ctrl = 3;
        g_game_p2_ctrl = 2;
    } else {
        g_game_p1_ctrl = 0;
        g_game_p2_ctrl = 1;
    }
}

void game_SetPlayerCount(s32 a0) {
    if ((u32)a0 >= 2) {
        return;
    }
    if (a0 == g_game_player_count) {
        return;
    }
    g_game_player_count = a0;
    if (!a0) {
        game_SetControllerPorts(0);
    }
    g_game_mirror_mode = (s16)g_game_player_count;
}
s32 game_GetPlayerCount(void) {
    return g_game_player_count;
}
void stage_InitCollision(void) {
    s32 i, j;
    s32 col_center, row_center;
    s32 data;
    s32 adj_i;
    s32 adj_j;
    s32 *ptr = (s32 *)g_stage_collision;
    s32 count = 0xFF;

    do {
        *ptr = 0;
        count--;
        ptr++;
    } while (count >= 0);

    game_GetPlayerCount();

    col_center = (*(s32 *)(D_800A3708 + 0x4C) + 0x7D00) / 2000;
    row_center = (*(s32 *)(D_800A3708 + 0x54) + 0x7D00) / 2000;

    for (i = 0; i < 16; i++) {
        adj_i = i - 8;
        {
            u32 y = (u32)(row_center + adj_i);
            if (y < 0x20) {
                data = D_80094A6C[i];
                for (j = 0; j < 16; j++) {
                    adj_j = j - 8;
                    {
                        u32 x = (u32)(col_center + adj_j);
                        if (x < 0x20) {
                            s32 bits = (data >> ((15 - j) * 2)) & 3;
                            g_stage_collision[(y << 5) + x] |= bits;
                        }
                    }
                }
            }
        }
    }
}
void func_8003F388(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x4;
    }
}
void func_8003F3D4(s16 *a0) {
    s32 x = a0[0] + 0x10;
    s32 y = a0[2] + 0x10;
    if ((u32)x < 0x20 && (u32)y < 0x20) {
        g_stage_collision[y * 32 + x] |= 0x8;
    }
}
void func_8003F420(s32 a0, s32 a1) {
    s32 s3, s2, s1;
    int new_var;
    s32 s0;
    a0 += 0x7D00;
    a1 += 0x7D00;
    s3 = a0 / 2000;
    s1 = a0 - s3 * 2000;
    s0 = a1 / 2000;
    s2 = s0;
    s0 = a1 - s2 * 2000;
    if (s1 < 1000) {
        s1 = -1;
    } else {
        s1 = 1;
    }
    if (s0 < 1000) {
        s0 = (new_var = -1);
    } else {
        s0 = 1;
    }
    stage_SetCollision(s3, s2, 2);
    stage_SetCollision(s3 + s1, s2, 2);
    stage_SetCollision(s3, s2 + s0, 2);
    stage_SetCollision(s3 + s1, s2 + s0, 2);
}

void stage_SetCollision(s32 a0, s32 a1, s32 a2) {
    g_stage_collision[a1 * 32 + a0] = a2 & 3;
}

u32 stage_GetCollision(s32 a0, s32 a1) {
    return g_stage_collision[a1 * 32 + a0];
}

void stage_ClearLighting(void) {
    g_game_flag_b = 0;
    g_game_flag_a = 0;
    D_800A93B8 = 0;
    D_800A93B4 = 0;
    g_stage_light_pos = 0;
    D_800A93C4 = 0;
    D_800A93C0 = 0;
    g_stage_light_dir = 0;
}

void stage_SetLightPosDir(s32 a0, s32 a1, s32 a2) {
    (&g_stage_light_pos)[a2] = a0;
    (&g_stage_light_dir)[a2] = a1;
}

void stage_ApplyLighting(void) {
    sys_StubEmpty3(g_stage_light_pos, g_stage_light_dir, 0);
    sys_StubEmpty3(D_800A93B4, D_800A93C0, 1);
    sys_StubEmpty3(D_800A93B8, D_800A93C4, 2);
}

void func_8003F62C(s32 *a0) {
    s16 *s0;
    s32 *s1 = a0;
    s0 = (s16 *)s1[9];
    if (s0 == 0) return;
    if (s0[3]) {
        func_8004016C(*(s16 *)((u8 *)s1 + 4));
        func_8003F824(s1, 0);
    }
    if (s0[1]) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8(s0);
    func_8001924C((s32 *)((u8 *)s0 + 0x418), s0[0]);
    if (s0[1]) {
        func_80040068((u8 *)s0);
        s0[1] = 0;
    }
}
void func_8003F6D8(s16 *arg0) {
    volatile s32 _arg0_spill;
    volatile s32 _p0, _p1, _p2, _p3, _p4;
    volatile s32 sp28;
    s32 count;
    register void *temp_v0 asm("v0");
    register s32 var_s7 asm("s7");
    register s32 var_s6 asm("s6");
    register s32 var_s5 asm("s5");
    register void *temp_s4 asm("s4");
    register void *var_s3 asm("s3");
    register void *var_s2 asm("s2");
    register void *temp_s1 asm("s1");
    register s32 temp_s0 asm("s0");
    s32 var_fp;
    _arg0_spill = (s32)arg0;
    var_fp = 0;
    if (*arg0 > 0) {
        {
            register s32 init_a3 asm("a3");
            init_a3 = 8;
            sp28 = init_a3;
        }
        do {
            {
                register s32 reg_t0 asm("t0");
                register s32 reg_a3 asm("a3");
                asm("lw %0,%2; lw %1,%3" : "=r"(reg_t0), "=r"(reg_a3) : "m"(_arg0_spill), "m"(sp28));
                var_s7 = 0;
                asm("addu %0,%1,%2" : "=r"(temp_v0) : "r"(reg_t0), "r"(reg_a3));
            }
            count = *(s32 *)((u8 *)temp_v0 + 0x1C);
            temp_s4 = (void *)((u8 *)temp_v0 + 0x1C);
            if (count > 0) {
                var_s6 = 0x18;
                var_s5 = 0x84;
                var_s3 = temp_s4;
                asm("addu %0,%1,%2" : "=r"(var_s2) : "r"(temp_s4), "r"(var_s5));
L8003F750:
                {
                    register s32 *call_a1 asm("a1");
                    register s16 *call_a2 asm("a2");
                    call_a1 = (s32 *)var_s2;
                    temp_s1 = (void *)((u8 *)temp_s4 + var_s6);
                    call_a2 = (s16 *)temp_s1;
                    var_s6 += 0x20;
                    var_s5 += 0x10;
                    temp_s0 = *(s32 *)((u8 *)var_s3 + 4);
                    var_s3 = (void *)((u8 *)var_s3 + 4);
                    temp_s0 += 0x18;
                    func_80052A20((s32 *)temp_s0, call_a1, call_a2);
                    func_80052A20((s32 *)temp_s0, (s32 *)((u8 *)var_s2 + 8), (s16 *)((u8 *)temp_s1 + 0x10));
                }
                var_s2 = (void *)((u8 *)temp_s4 + var_s5);
                var_s7 += 1;
                if (var_s7 < *(s32 *)temp_s4) goto L8003F750;
            }
            {
                register s32 incr_t0 asm("t0");
                register s16 *end_a3 asm("a3");
                incr_t0 = sp28;
                end_a3 = (s16 *)_arg0_spill;
                incr_t0 += 0xD0;
                sp28 = incr_t0;
                var_fp += 1;
                if (var_fp >= *end_a3) break;
            }
        } while (1);
    }
}

void func_8003F7F4(void) {
    obj_ClearAll();
    sys_StubEmpty2();
    g_game_flag_a = 0;
    g_game_flag_b = 0;
}
void tslPrintScreen(u8 *arg0, s32 arg1)
{
    register u8 *arg0_s4 asm("s4") = arg0;
    register s32 arg1_s7 asm("s7") = arg1;
    register u8 *s3 asm("s3");
    s16 *new_var;
    register s16 *s1 asm("s1");
    register u8 *s0 asm("s0");
    s32 s5;
    u8 *s2;
    s32 cur;
    u8 *dst;
    s32 flag;
    s32 ret;
    u8 *new_var2;
    u8 *new_var7;
    s32 new_var5;
    s16 new_var3;

    s3 = *((u8 **)(arg0_s4 + 0x24));
    if (s3 == 0) goto done;
    s1 = *((s16 **)(arg0_s4 + 0x28));
    s0 = s3 + 0x468;
    if (*s1 == -3) {
        *((s32 *)(arg0_s4 + 0x24)) = 0;
        goto done;
    }
    *((u16 *)(s3 + 0x0)) = 0;
    *((u16 *)(s3 + 0x2)) = 0;
    *((u16 *)(s3 + 0x4)) = 0;
    *((u16 *)(s3 + 0x6)) = 0;
    new_var3 = (s16)(*((u16 *)s1));
    if (*s1 == -3) goto check_arg1;
    s5 = 0;
    new_var5 = -2;
outer_loop:
    if (new_var3 == new_var5) goto past_marker;
    new_var = (s16 *)s3;
    if (*((s16 *)s3) >= 5) {
        func_80052C10();
    }
    cur = (*new_var) * 0xD0 + 8;
    s2 = s3 + cur;
    new_var2 = *((u8 **)(arg0_s4 + 0x1A34 + s5 * 4));
    *((u8 **)(s2 + 0x18)) = s0;
    s0 = new_var2;
    *((u8 **)(s2 + 0x14)) = s0;
    ret = (s32)func_8003FA24((s32 *)s2);
    dst = s3 + (s32)(*((s16 *)s3)) * 16;
    new_var7 = s2;
    do {
        s32 t0 = *((s32 *)(new_var7 + 0x4));
        s32 t1 = *((s32 *)(new_var7 + 0x8));
        s32 t2 = *((s32 *)(new_var7 + 0xC));
        s32 t3 = *((s32 *)(new_var7 + 0x10));

        *((s32 *)(dst + 0x418)) = t0;
        *((s32 *)(dst + 0x41C)) = t1;
        *((s32 *)(dst + 0x420)) = t2;
        *((s32 *)(dst + 0x424)) = t3;

        *s0 = 0xD;
        (*((u16 *)(s3 + 0x0)))++;
        flag = *((s16 *)s1);
        s0 = (u8 *)ret;
        if (flag < 0) goto check_ctrl;
        s1++;
walk_nonneg:
        cur = *((s16 *)s1);
        s1++;
        if (cur >= 0) goto walk_nonneg;
        s1--;
    } while (0);
check_ctrl:
    if (*((s16 *)s1) == -1) {
        s1++;
        func_8003FECC((s32 *)arg0_s4, (s32 *)new_var7, s1);
    }
    flag = *((s16 *)s1);
skip_marker:
    if (flag == new_var5) goto past_marker;
    s1++;
walk_nonmark:
    if (*((s16 *)s1) != new_var5) { s1++; goto walk_nonmark; }
past_marker:
    s1++;
    new_var3 = *((s16 *)s1);
    s5 = new_var3;
    if (s5 != -3) { s5++; goto outer_loop; }
check_arg1:
    if (arg1_s7 != 0) {
        func_80045A28(
            (s32)(*((s16 *)(arg0_s4 + 0x4))),
            (s32)(s0 - *((u8 **)(arg0_s4 + 0x1C))));
    }
done:
    return;
}
s32 func_8003FA24(void *a0) {
    (void)a0;
    return 0;
}
s16 *func_8003FE40(s16 *a0, s32 a1, s16 *a2) {
    register s32 i asm("a3");
    i = 0;
    if (a1 > i) {
        register s16 t0 asm("t0") = -256;
        register s16 *v1 asm("v1") = a0;
        for (i = 0; i < a1; i++) {
            *(s16 *)((u8 *)v1 + 6) = t0;
            v1 = (s16 *)((u8 *)v1 + 8);
        }
    }

    {
        register s32 count asm("v1");
        count = a2[0];
        a2++;
        if (count >= 0) {
            register s32 neg1 asm("t1") = -1;
            do {
                register int val asm("a1");
                val = a2[0];
                do { count--; } while(0);
                a2++;
                if (count != neg1) {
                    register s32 neg1b asm("t0") = -1;
                    do {
                        register s32 addr asm("v0");
                        i = a2[0];
                        a2++;
                        count--;
                        addr = (i << 3) + (s32)a0;
                        *(s16 *)(addr + 6) = val;
                    } while (count != neg1b);
                }
                count = a2[0];
                a2++;
            } while (count >= 0);
        }
    }
    return (s16 *)a2;
}

void func_8003FECC(s32 *a0, s32 *a1, s16 *a2)
{
  s32 t2;
  u8 *new_var2;
  s16 v1;
  unsigned int t3;
  s32 *a3;
  s16 *new_var;
  s32 *t1;
  u16 t0;
  t2 = *((s32 *) (((u8 *) a1) + 0x1C));
  ;
  t0 = (u16) a2[0];
  a1 = (s32 *) (((u8 *) a1) + 0x1C);
  new_var = &a2[0];
  if (a2[(*new_var) * 0] == (-2))
  {
    goto end;
  }
  t3 = -2;
  a3 = (s32 *) ((t2 * 0x10) + (s32)(u8 *) a1);
  t1 = (s32 *) ((t2 * 4) + (s32)(u8 *) a1);
  loop:
  {
    a2++;
    {
      s32 v0;
      v1 = 1;
      v0 = (s32) (((u8 *) a0) + ((s32) ((((s16) t0) * 0x68) + 0x94)));
      t1[v1] = v0;
      a3++;
      a3--;
      t1++;
      t1--;
    }
    {
      u16 val = (u16) (*(a2++));
      t2++;
      *((u16 *) (((u8 *) a3) + 0x84)) = val;
    }
    *((u16 *) (((u8 *) a3) + 0x86)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x88)) = (u16) (*(a2++));
    *((u16 *) (((u8 *) a3) + 0x8C)) = (u16) (*(a2++));
    new_var2 = ((u8 *) a3) + 0x90;
    *((u16 *) (((u8 *) a3) + 0x8E)) = (u16) (*(a2++));
    *((u16 *) new_var2) = (u16) (*(a2++));
    {
      s32 val78 = (s32) (*(a2++));
      {
        s32 *p_next = (s32 *) (((u8 *) a3) + 0x10);
        a3 = p_next;
      }
      *((s32 *) (((u8 *) t1) + 0x78)) = val78;
    }
    t0 = (u16) (*a2);
    v1 = (s16) t0;
    t1 = (s32 *) (((u8 *) t1) + 4);
  }

  if (v1 != ((s16) t3))
  {
    goto loop;
  }
  end:
  a1[0] = t2;

  a1[5] = t2;
}
s32 func_8003FFA8(s32 a0) {
    if (a0 & 3) {
        a0 = (a0 + 3) & ~3;
    }
    return a0;
}
void func_8003FFC4(s32 *a0) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        v1[3] = 1;
    }
}
void func_8003FFE0(void) {
    s32 *v0 = (s32 *)func_8004153C();
    if (v0) {
        s16 *v1 = (s16 *)v0[9];
        if (v1) {
            v1[1] = 1;
        }
    }
}
void func_8004001C(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 1;
        a0[0xE + i * 0xD0] = 1;
    }
}
void func_80040068(u8 *a0) {
    s32 i;
    for (i = 0; i < *(s16 *)a0; i++) {
        a0[0x41A + i * 0x10] = 0;
        a0[0xE + i * 0xD0] = 0;
    }
}
void func_800400B0(s32 *a0, s32 a1) {
    s16 *v1 = (s16 *)a0[9];
    if (v1) {
        s32 i;
        for (i = 0; i < v1[0]; i++) {
            *(s32 *)((u8 *)v1 + i * 0xD0 + 0x34) = a1;
        }
    }
}
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            s0 = 0;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}

void func_8004016C(void) {
    void *v0 = func_8004153C();
    if (v0) {
        func_800400F8(v0);
    }
}

void func_8004019C(s32 *a0, s32 a1) {
    s32 *v1 = (s32 *)a0[9];
    if (v1) {
        v1 = (s32 *)((s32)v1 + a1);
        a0[9] = (s32)v1;
        a0[10] = a0[10] + a1;
        *(s16 *)((s32)v1 + 6) = 1;
    }
}
