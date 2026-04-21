
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
extern u8 *D_800A147C;
extern s32 D_800161B8;
extern s32 D_800161C8;
extern void D_80016240;
extern u8 D_800A11D5;
extern s32 D_800A11DC[];
extern s32 D_800A125C[];
extern u8 D_800A1494;
extern s32 D_800A11B4;
extern s32 D_800A11B8;
extern void D_800F19A0;
extern void D_800F19A8;
extern s32 D_800F19B8;
extern s32 D_800F19BC;
extern void *D_800F19C0;
s32 cpu_side_move_dir_4(s32 a0, u8 *a1)
{
  int new_var;
  s32 v0;
  s32 cnt;
  u8 *idx_1494;
  u8 *idx_1495;
  s32 *tbl_125c;
  u8 saved;
  s32 status;
  u8 *src;
  u8 *dst;
  s32 i;
  u8 b;
  s32 temp;
  D_800F19B8 = sys_VSync(-1) + 0x3C0;
  tbl_125c = D_800A125C;
  idx_1494 = &D_800A1494;
  idx_1495 = idx_1494 + 1;
  D_800F19BC = 0;
  D_800F19C0 = &D_80016240;
  loop:
  v0 = sys_VSync(-1);

  if (D_800F19B8 < v0)
  {
    goto do_timeout;
  }
  cnt = D_800F19BC;
  D_800F19BC = cnt - -1;
  if (!(0x3C0000 < cnt))
  {
    goto success;
  }
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg4;
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

  new_var = 0xFF;
  if (sys_GetVblankCount() != 0)
  {
    saved = (*D_800A147C) & 3;
    poll:
    status = func_80080828();

    if (status != 0)
    {
      if (status & 4)
      {
        if (D_800A11B8 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B8)(*idx_1495, &D_800F19A8);
        }
      }
      if (status & 2)
      {
        if (D_800A11B4 != 0)
        {
          ((void (*)(u8, void *)) D_800A11B4)(*idx_1494, &D_800F19A0);
        }
      }
      goto poll;
    }
    *D_800A147C = saved;
  }
  temp = (*idx_1494) & new_var;
  if (((temp == 2) || (temp == 5)) != 0)
  {
    *idx_1494 = 2;
    dst = a1;
    src = (u8 *) (&D_800F19A0);
    i = 7;
    if (a1 != 0)
    {
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
    return temp;
  }
  if (a0 != 0)
  {
    return 0;
  }
  goto loop;
}
