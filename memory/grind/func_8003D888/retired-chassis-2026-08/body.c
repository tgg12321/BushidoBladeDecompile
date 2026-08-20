s32 func_8003D888(u32 *a0_param, s32 a1_param)
{
  register u32 *a2 asm("$6");
  register s32 a3 asm("$7");
  register s32 v0 asm("$2");
  register u32 v1 asm("$3");
  register s32 a0 asm("$4");
  register s32 a1 asm("$5");

  __asm__ __volatile__("addu\t$6, $4, $0" : "=r"(a2));
  __asm__ __volatile__("addu\t$7, $5, $0" : "=r"(a3));
  a1 = *((s32 *)((u8 *)a2 + 8));
  if (a1 < a3) {
    a3 -= a1;
    a0 = 1;
    v0 = (a0 << a1) - 1;
    a0 = (a0 << a3) - 1;
    v1 = *((u32 *)((u8 *)a2 + 4));
    a1 = *(s32 *)a2;
    v1 &= v0;
    v0 = a1 + 4;
    *(u32 *)a2 = v0;
    v0 = 32 - a3;
    a1 = *(s32 *)a1;
    v1 <<= a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v0 = (u32)a1 >> v0;
    v0 &= a0;
    v1 |= v0;
    *((u32 *)((u8 *)a2 + 4)) = a1;
  } else {
    v1 = *((u32 *)((u8 *)a2 + 4));
    v0 = a1 - a3;
    *((s32 *)((u8 *)a2 + 8)) = v0;
    v1 >>= v0;
    v0 = 1;
    v0 = (v0 << a3) - 1;
    v1 &= v0;
  }
  return (s32)v1;
}
