/* REJECTED s3: permuter output-160-1 (score 160). Moves the D_800F4E1A
 * store ABOVE u = 0x18, so the sh writes u's STALE value (uninitialized on
 * iteration 1, the limit byte afterwards) instead of 24 - SEMANTICS-BROKEN.
 * The permuter's reorder mutations do not preserve behavior; reject on
 * correctness regardless of score. Same defect in output-164-1 (plus a
 * var_s0 = var_s0; dead self-assign). */

typedef signed short s16;
typedef signed int s32;
typedef unsigned char u8;
typedef unsigned short u16;
extern u8 D_80101BCC;
extern s16 D_800F4E1A;
extern s16 D_800F4E1E;
extern s16 D_800F4E28[];
extern s16 D_800F4E2A;
extern s16 D_800F4E2C;
extern s16 D_800F4E2E;
extern u16 D_8010280A;
void func_8008B488(s32 *);
void func_800871D4(s32);
void func_800858D0(void)
{
  s32 buf[16];
  s16 var_s0;
  s32 offset;
  s16 ff;
  s32 u;
  buf[1] = 0x60093;
  var_s0 = 0;
  *((s16 *) (((u8 *) buf) + 0x14)) = 0x1000;
  *((s32 *) (((u8 *) buf) + 0x1C)) = 0x1000;
  *((u16 *) (((u8 *) buf) + 0x3A)) = 0x80FF;
  *((s16 *) (((u8 *) buf) + 0x08)) = 0;
  *((s16 *) (((u8 *) buf) + 0x0A)) = 0;
  *((s16 *) (((u8 *) buf) + 0x3C)) = 0x4000;
  if (D_80101BCC != 0)
  {
    ff = 0xFF;
    do
    {
      offset = var_s0 * 54;
      *((s16 *) (((u8 *) (&D_800F4E1A)) + offset)) = u;
      u = 0x18;
      *((s16 *) (((u8 *) (&D_800F4E1E)) + offset)) = 0;
      *((s16 *) (((u8 *) D_800F4E28) + offset)) = ff;
      *((s16 *) (((u8 *) (&D_800F4E2A)) + offset)) = 0;
      *((s16 *) (((u8 *) (&D_800F4E2C)) + offset)) = 0;
      *((s16 *) (((u8 *) (&D_800F4E2E)) + offset)) = ff;
      offset = 1;
      buf[0] = offset << var_s0;
      func_8008B488(buf);
      D_8010280A = var_s0;
      func_800871D4(1);
      var_s0 = var_s0 + 1;
      u = D_80101BCC;
    }
    while (var_s0 < u);
  }
}
