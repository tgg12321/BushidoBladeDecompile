s32 func_80043D34(u8 *a0, s32 a1, s32 a2, s32 a3, s32 a4)
{
  register unsigned int t1 asm("$9");
  register s32 t0 asm("$8");
  register s32 v0 asm("$2");
  register unsigned int v1 asm("$3");

  a1 = (a1 << 16) >> 22;
  v0 = (a2 << 16) >> 24;
  a3 = (a3 << 16) >> 20;
  t1 = *((u16 *) (a0 + 0x1A));
  t0 = (t1 & 0xF) + a1;
  t0 &= 0xF;
  v1 = (t1 >> 4) & 1;
  v1 += v0;
  v1 &= 1;
  t1 &= 0xFFE0;
  v1 <<= 4;
  t1 |= v1;
  v0 = *((u8 *) (a0 + 0xD));
  t0 |= t1;
  *((u16 *) (a0 + 0x1A)) = t0;
  {
    register s32 la1 asm("$5");
    __asm__ __volatile__("lw\t$5, 16($29)" : "=r"(la1) : "r"(t0));
    a1 = la1;
  }
  v1 = *((u8 *) (a0 + 0x25));
  v0 += a2;
  *((u8 *) (a0 + 0xD)) = v0;
  v0 = *((u8 *) (a0 + 0x19));
  v1 += a2;
  *((u8 *) (a0 + 0x25)) = v1;
  v1 = *((u16 *) (a0 + 0xE));
  v0 += a2;
  *((u8 *) (a0 + 0x19)) = v0;
  __asm__ __volatile__("");
  a3 += v1 & 0x3F;
  a3 &= 0x3F;
  a1 += (v1 >> 6) & 0x1FF;
  a1 &= 0x1FF;
  v1 &= 0x8000;
  a1 <<= 6;
  v1 |= a1;
  a3 |= v1;
  *((u16 *) (a0 + 0xE)) = a3;
}
