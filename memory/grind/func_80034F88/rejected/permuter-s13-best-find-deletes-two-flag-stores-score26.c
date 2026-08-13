void func_80034F88(void)
{
  s32 *p;
  u8 *new_var;
  s32 i;
  p = func_80077D00();
  {
    u8 *q = &D_80106A73;
    s32 v;
    s32 c;
    *q &= 0xF8;
    v = *(new_var = q);
    c = p[8] & 1;
    if (c)
    {
      c = v | 1;
    }
    else
    {
      c = v;
    }
    *new_var = c;
  }
  bb2_set_flag(p[8] & 2, 2);
  bb2_set_flag(p[8] & 4, 4);
  for (i = 0; i < 3; i++)
  {
    *((&D_80106A70) + i) = *((((u8 *) p) + i) + 0x17);
  }

}
