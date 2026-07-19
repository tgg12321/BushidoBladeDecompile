
typedef signed int s32;
typedef signed short s16;
typedef unsigned char u8;
typedef unsigned int u32;
extern s16 D_800EED10[];
extern s32 D_800EED14;
extern s32 D_800EED18;
extern s32 D_800EED1C[];
extern s32 D_800A33A0;
extern s32 D_800A33A4;
extern s32 D_800A33AC;
extern void gpu_DrawSync(s32);
extern void func_800520B8(s32, s32, s32);
void func_80045294(s32 a0, s32 a1)
{
  s32 sum = 0;
  s32 i = a0;
  s32 *new_var;
  s32 v1 = a0 << 4;
  s32 s4 = *((s32 *) (((u8 *) (&D_800EED14)) + v1));
  s32 count = D_800A33AC;
  s32 s5 = s4 + a1;
  if (i < count)
  {
    do
    {
      s32 val = *((s32 *) (((u8 *) (&D_800EED18)) + v1));
      i = a0;
      v1 += 0x10;
      i += 1;
      sum += val;
    }
    while (i < count);
  }
  if (sum != 0)
  {
    s32 *ptr;
    s32 idx;
    gpu_DrawSync(0);
    func_800520B8(s4, s5, sum);
    i = a0;
    if (i < D_800A33AC)
    {
      v1 = i << 4;
      new_var = (s32 *) (((u8 *) (&D_800EED14)) + v1);
      ptr = new_var;
      idx = v1;
      do
      {
        *ptr += a1;
        {
          void (*fn)(s16, s32) = (void (*)(s16, s32)) (*((s32 *) (((u8 *) (&D_800EED1C)) + idx)));
          if (fn != 0)
          {
            fn(*((s16 *) (((u8 *) (&D_800EED10)) + idx)), a1);
          }
        }
        ptr = (s32 *) (((u8 *) ptr) + 0x10);
        idx += 0x10;
        i += 1;
      }
      while (i < D_800A33AC);
    }
  }
  D_800A33A0 += a1;
  D_800A33A4 -= a1;
}
