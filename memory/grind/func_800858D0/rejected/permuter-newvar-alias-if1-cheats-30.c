/* REJECTED s3: permuter output-30-1 (score 30). Closing distance rides on
 * an invented pointer alias local (new_var = buf passed to the call), an
 * invented staging local for the limit read (new_var2), and an if (1) { }
 * always-true wrap - all forbidden-family shapes (alias/staging inventions
 * + if(1) wrap not sanctioned by do-while-zero carve-out). Reject. */

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
  int new_var2;
  s32 *new_var;
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
    if (1)
    {
    }
    ff = 0xFF;
    do
    {
      offset = var_s0 * 54;
      u = 0x18;
      *((s16 *) (((u8 *) (&D_800F4E1E)) + offset)) = 0;
      *((s16 *) (((u8 *) (&D_800F4E1A)) + offset)) = u;
      new_var = buf;
      *((s16 *) (((u8 *) D_800F4E28) + offset)) = ff;
      *((s16 *) (((u8 *) (&D_800F4E2A)) + offset)) = 0;
      *((s16 *) (((u8 *) (&D_800F4E2C)) + offset)) = 0;
      *((s16 *) (((u8 *) (&D_800F4E2E)) + offset)) = ff;
      offset = 1;
      buf[0] = offset << var_s0;
      func_8008B488(new_var);
      D_8010280A = var_s0;
      func_800871D4(1);
      var_s0 = var_s0 + 1;
      new_var2 = D_80101BCC;
      u = new_var2;
    }
    while (var_s0 < u);
  }
}
