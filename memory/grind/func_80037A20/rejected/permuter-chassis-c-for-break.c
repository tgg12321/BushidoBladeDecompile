typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern u8 D_80102810;
extern s32 g_str_memcard_fmt;
extern s32 D_800A38C8;
extern void func_80079A30(s32 *, s32, s32, s32);
extern s32 bios_firstfile_B(s32 *, s32 *);
extern s32 bios_nextfile_B(s32 *);
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    var_s1 = 1;
    for (;;)
    {
      var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
      v0_val = bios_nextfile_B(var_s0);
      var_s1 += 1;
      if (!v0_val)
      {
        var_s1 -= 1;
        break;
      }
    }
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
