s32 func_80037A20(s32 arg0, s32 arg1)
{
  register s32 *var_s0 asm("s0");
  register s32 var_s1 asm("s1");
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  sprintf(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (firstfile(sp10, var_s0) != 0)
  {
    __asm__("" : "=r"(var_s1) : "0"(var_s1));
    var_s1++;
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
    v0_val = nextfile(var_s0);
    var_s1 += 1;
    if (v0_val) goto loop;
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
