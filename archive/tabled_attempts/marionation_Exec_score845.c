
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
extern volatile int sys_VSync(s32);
extern void tslTm2LoadImage_2(void *);
extern void debug_printf(void *, void *, s32, s32, s32);
extern void cdrom_ClearIrq(void);
extern s32 sys_GetVblankCount(void);
extern s32 func_80080828(void);
extern volatile u8 *g_cd_index_reg;
extern s32 D_800161B8;
extern s32 D_800161C8;
extern void D_80016248;
extern u8 D_800A11D5;
extern s32 D_800A11DC[];
extern s32 D_800A125C[];
extern u8 D_800A1494;
extern s32 g_cd_callback_a;
extern s32 g_cd_callback_b;
extern void D_800F19A0;
extern void D_800F19A8;
extern void D_800F19B0;
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
s32 marionation_Exec(s32 a0, u8 *a1)
{
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  u8 *idx_1496;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 new_var2;
  u8 result;
  u8 *src;
  u8 *dst;
  int new_var;
  s32 i;
  u8 b;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = 1 + idx_1494;
  idx_1496 = idx_1494 + 2;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016248;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt + 1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 arg4;
    ;
    arg4 = tbl_125c[idx_1494[0]];
    debug_printf(&D_800161C8, D_800F19C0, D_800A11DC[D_800A11D5], arg4, D_800A125C[idx_1494[1]]);
  }
  cdrom_ClearIrq();
  v0 = -1;
  goto check;
  success:
  v0 = 0;

  check:
  if (v0 != 0)
  {
    return -1;
  }

  if (sys_GetVblankCount() != 0)
  {
    saved = (*g_cd_index_reg) & 3;
    poll:
    status = func_80080828();

    if (status != 0)
    {
      if (status & 4)
      {
        if (g_cd_callback_b != 0)
        {
          ((void (*)(u8, void *)) g_cd_callback_b)(*idx_1495, &D_800F19A8);
        }
        ;
      }
      if (status & 2)
      {
        if (g_cd_callback_a)
        {
          ((void (*)(u8, void *)) g_cd_callback_a)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *g_cd_index_reg = saved;
  }
  result = *idx_1496;
  new_var = result != (D_800F19BC * 0);
  if (new_var)
  {
    *idx_1496 = 0;
    src = (u8 *) (&D_800F19B0);
    new_var = result;
    dst = a1;
    if (a1 != 0)
    {
      i = 7;
      do
      {
        b = *src;
        src++;
        i--;
        *dst = b;
        dst++;
      }
      while (i != (-1));
    }
    return new_var;
  }
  result = *(idx_1496 - 1);
  if (result != 0)
  {
    *(idx_1496 - 1) = 0;
    dst = a1;
    src = (u8 *) (&D_800F19A8);
    i = 7;
    if (a1 != 0)
    {
      do
      {
        new_var2 = (*dst = *src);
        b = new_var2;
        src++;
        i--;
        dst++;
      }
      while (i != (-1));
    }
    return result;
  }
  if (a0 != 0)
  {
    return 0;
  }
  goto loop;
}
