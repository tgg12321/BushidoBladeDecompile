
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
extern s32 D_800A3368;
extern u32 *D_800A3820;
extern u8 D_800A4750[];
extern u8 D_800A6690[];
extern s16 D_800A7FE0[];
extern u16 D_800A87E0[];
extern u8 D_800A8FB0[];
s32 *func_8003EB84(s32 a0, s32 a1, s32 *out)
{
  s32 buf[32];
  s32 mask;
  s32 *p;
  s32 *p_lo;
  s32 *p_hi;
  int new_var3;
  s32 *p_end;
  s32 t4;
  s32 t1;
  s32 t2;
  s32 new_var;
  s16 *p_t6;
  s16 *p_t3;
  s32 *p_t5;
  s32 entry;
  s32 a1_val;
  u8 flag_byte;
  u16 lookup_val;
  s32 v1;
  u8 *p_a;
  u8 *p_b;
  int new_var2;
  new_var2 = 1;
  new_var3 = a0 >= 0x20;
  if (a0 >= 0)
  {
    if (new_var3)
    {
      mask = -1;
      goto fill;
    }
  }
  else
    if ((-a0) >= 0x20)
  {
    mask = -1;
    goto fill;
  }
  if (a0 < 0)
  {
    mask = (s32) (((u32) (-1)) >> (a0 + 0x1F));
  }
  else
    if (a0 == 0)
  {
    mask = 0;
  }
  else
  {
    mask = (-1) << (0x20 - a0);
  }
  fill:
  p = buf;

  p_lo = &buf[a1];
  entry = a1 + 0x1F;
  p_hi = &buf[entry];
  p_end = &buf[0x20];
  do
  {
    if ((((s32) p) >= ((s32) p_lo)) && (((s32) p) < ((s32) p_hi)))
    {
      *p = mask;
      goto next;
    }
    *p = -1;
    next:
    p++;

  }
  while (((s32) p) < ((s32) p_end));
  t4 = 0;
  p_t6 = D_800A7FE0;
  p_t5 = buf;
  do
  {
    t2 = *p_t5;
    if (t2 != 0)
    {
      t1 = 0;
      p_t3 = p_t6;
      do
      {
        if (t2 < 0)
        {
          entry = *p_t3;
          new_var = t4 << 5;
          if (entry >= 0)
          {
            a1_val = entry;
            mask = new_var + t1;
            flag_byte = D_800A8FB0[mask];
            inner_inner:
            lookup_val = *((u16 *) ((((s32) ((s16) a1_val)) << 1) + ((s32) D_800A87E0)));

            t1 = 0;
            v1 = lookup_val & 0x7FFF;
            a1_val++;
            if (v1 < D_800A3368)
            {
              p_a = (u8 *) (((s32) D_800A4750) + ((v1 << 3) << 1));
              p_a[6] = (s8) (flag_byte & 3);
              if ((flag_byte & 8) || ((flag_byte & 4) && (p_a[7] & 8)))
              {
                p_a[7] = p_a[7] | 1;
              }
              else
              {
                p_a[7] = p_a[7] & 0xFE;
              }
              {
                u32 *cur_3820 = D_800A3820;
                D_800A3820 = cur_3820 + 1;
                *cur_3820 = (u32) p_a;
              }
            }
            else
            {
              p_b = (u8 *) (((s32) D_800A6690) + ((v1 - D_800A3368) * 0x68));
              if (p_b[0x58] == t1)
              {
                *out = (s32) p_b;
                new_var2 = 1;
                out++;
                p_b[0x58] = new_var2;
              }
            }
            t2 <<= 1;
            if ((lookup_val & 0x8000) == 0)
            {
              goto inner_inner;
            }
          }
        }
        t1++;
        p_t3++;
      }
      while (t1 < 0x20);
    }
    p_t6 += 0x20;
    t4++;
    p_t5++;
  }
  while (t4 < 0x20);
  return out;
}
