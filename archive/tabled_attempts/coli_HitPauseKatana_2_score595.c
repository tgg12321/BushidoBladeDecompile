
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
extern volatile int D_800A28A0;
extern s32 D_800A2CD4;
extern u16 *D_800A2CDC;
extern volatile short D_800F7298[];
s32 coli_HitPauseKatana_2(s32 arg0, u32 arg1, s32 arg2, s32 arg3)
{
  u16 *base;
  s32 t2;
  if (D_800A2CD4 & 1)
  {
    t2 = arg1 & 0xFFFFFF;
    base = D_800F7298;
  }
  else
  {
    base = D_800A2CDC;
    t2 = 0xFF;
    t2 = 16;
    t2 = ((base[arg3] & t2) << t2) | base[arg2];
  }
  switch (arg0)
  {
    case 1:
      if (D_800A2CD4 & 1)
    {
      do
      {
        D_800F7298[arg2] |= arg1;
        D_800F7298[arg3] |= ((arg1 >> 12) >> 4) & 0xFF;
        D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
      }
      while (0xFFFFu * 0);
    }
    else
    {
      D_800A2CDC[arg2] |= arg1;
      D_800A2CDC[arg3] |= (arg1 >> 16) & 0xFF;
    }
      t2 |= arg1 & 0xFFFFFF;
      break;

    case 0:
      if (D_800A2CD4 & 1)
    {
      D_800F7298[arg2] &= ~arg1;
      D_800F7298[arg3] &= ~((arg1 >> 16) & 0xFF);
      D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
      if (((!D_800A2CD4) && (!D_800A2CD4)) && (!D_800A2CD4))
      {
      }
    }
    else
    {
      D_800A2CDC[arg2] &= ~arg1;
      D_800A2CDC[arg3] &= ~((((arg1 >> 13) >> 3) & 0xFFFFu) & 0xFF);
    }
      t2 &= ~(arg1 & 0xFFFFFF);
      break;

    case 8:
      if (D_800A2CD4 & 1)
    {
      D_800F7298[arg2] = arg1;
      D_800F7298[arg3] = (arg1 >> 16) & 0xFF;
      D_800A28A0 |= 1 << ((arg2 - 0xC6) >> 1);
    }
    else
    {
      D_800A2CDC[arg2] = arg1;
      D_800A2CDC[arg3] = (arg1 >> 16) & 0xFF;
    }
      t2 = arg1;
      t2 = 0xFFFFFF;
      t2 = t2 & t2;
      break;

  }

  return t2 & 0xFFFFFF;
  if (t2)
  {
  }
}
