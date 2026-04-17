
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
extern s16 D_8008E194;
extern u8 D_80106A78;
extern s16 Judge;
u8 *coli_hit_body_weapon(u8 *arg0, s32 arg1)
{
  u8 *new_var3;
  s32 new_var2;
  u16 new_var5;
  register u8 *t1 asm("t1") = arg0;
  u8 *p;
  register u8 *a3 asm("a3");
  s32 i;
  u8 *t0;
  s32 v;
  volatile unsigned long new_var4;
  s32 new_var;
  s32 v1;
  a3 = &D_80106A78;
  i = 0;
  p = a3 + 0xA;
  do
  {
    if (((*((s16 *) (p - 8))) == (-1)) && ((*p) == 0xFF))
    {
      v = 1;
      goto found;
    }
    i++;
    p += 0x64;
    a3 += 0x64;
  }
  while (i < 12);
  v = 1;
  found:
  *((s8 *) (a3 + 0xA)) = i;

  *((s16 *) (a3 + 2)) = arg1;
  *((s8 *) (a3 + 7)) = 0;
  *((s8 *) (a3 + 8)) = 0;
  *((s8 *) (a3 + 4)) = v;
  *((s8 *) (a3 + 6)) = (s8) (*((u16 *) (t1 + 4)));
  *((s32 *) (a3 + 0x2C)) = *((s32 *) (t1 + 0xF4));
  v1 = *((s16 *) (t1 + 0x1A));
  *((s32 *) (a3 + 0x30)) = (*((s32 *) (t1 + 0xF8))) - (v1 / 32);
  *((s32 *) (a3 + 0x34)) = *((s32 *) (t1 + 0xFC));
  t0 = ((u8 *) (&D_8008E194)) + (2 * (arg1 * 7));
  *((s32 *) (a3 + 0x44)) = ((s32) ((*((&Judge) + ((*((u16 *) (t1 + 0x1CA))) & 0xFFF))) * (*((s16 *) (t0 + 4))))) >> 12;
  *((s32 *) (a3 + 0x48)) = *((s16 *) (t0 + 6));
  new_var = (s32) ((*((&Judge) + (((*((s16 *) (t1 + 0x1CA))) + 0x400) & 0xFFF))) * (*((s16 *) (t0 + 4))));
  *((s32 *) (a3 + 0x2C)) += *((s32 *) (a3 + 0x44));
  *((s32 *) (a3 + 0x30)) += *((s32 *) (a3 + 0x48));
  *((s32 *) (a3 + 0x4C)) = new_var >> 12;
  *((s32 *) (a3 + 0x2C)) += ((s32) (*((s32 *) (a3 + 0x44)))) / 2;
  *((s32 *) (a3 + 0x38)) = *((s32 *) (a3 + 0x2C));
  *((s32 *) (a3 + 0x30)) += ((s32) (*((s32 *) (a3 + 0x48)))) / 2;
  *((s32 *) (a3 + 0x34)) += *((s32 *) (a3 + 0x4C));
  *((s32 *) (a3 + 0x34)) += ((s32) (*((s32 *) (a3 + 0x4C)))) / 2;
  new_var2 = (new_var4 = *((s32 *) (a3 + 0x34)));
  *((s32 *) (a3 + 0x3C)) = *((s32 *) (a3 + 0x30));
  *((s32 *) (a3 + 0x40)) = new_var2;
  *((s16 *) (a3 + 0x54)) = 0;
  new_var2 = *((u16 *) (t1 + 0x1CA));
  *((s16 *) (a3 + 0x58)) = 0;
  *((u16 *) (a3 + 0x56)) = new_var2;
  {
    s16 sw = *((s16 *) (t0 + 0));
    if (sw == 1)
    {
      *((s16 *) (a3 + 0x5C)) = 0;
      *((s16 *) (a3 + 0x60)) = 0;
      *((s16 *) (a3 + 0x5E)) = *((u16 *) (t0 + 8));
    }
    else
      if (sw == 2)
    {
      new_var5 = *((u16 *) (t0 + 8));
      *((s16 *) (a3 + 0x5E)) = 0;
      *((s16 *) (a3 + 0x60)) = 0;
      *((s16 *) (a3 + 0x5C)) = new_var5;
    }
    else
      if (sw == 3)
    {
      *((s16 *) (a3 + 0x5C)) = 0;
      *((s16 *) (a3 + 0x60)) = 0;
      *((s16 *) (a3 + 0x5E)) = *((u16 *) (t0 + 8));
    }
    else
    {
      *((s16 *) (a3 + 0x5C)) = 0;
      *((s16 *) (a3 + 0x5E)) = 0;
      *((s16 *) (a3 + 0x60)) = 0;
    }
  }
  *((s32 *) (a3 + 0x50)) = 1;
  *((s8 *) (a3 + 5)) = 0;
  *((s16 *) (new_var3 = a3 + 0)) = 0;
  return a3;
}
