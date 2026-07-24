/* REJECTED — carries two `do { } while (0)` scheduler barriers with no
 * semantic purpose (cheat-reviewer test #1 FAIL). Inherited from the original
 * 9-rule recipe. Reached sandbox --disable all = 11 (HEAD baseline 12), but
 * does NOT close: var_v0 still allocates to $v1 (target wants $v0) and 0xFC
 * to $v0 (target wants $v1) — the 9 regfix `subst`/`reorder` rules' job.
 * The 1-insn improvement comes only from the +4-before-0x14 store order plus
 * the do-while barriers, not from a clean register flip. Do NOT promote.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  u8 var_v0;
  s32 new_var;
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    var_v0 = 0x46;
    if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4)
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x1E;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xC8;
      do { } while (0);            /* CHEAT: empty scheduler barrier */
      var_v0 = 0x32;
    }
    else
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x50;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xDC;
    }
    new_var = 0xFC;
    *((u8 *) (((s32) arg1) + 4)) = new_var;
    *((u8 *) (((s32) arg1) + 0xC)) = new_var;
    *((u8 *) (((s32) arg1) + 0xE)) = var_v0;
    do { *((u8 *) (((s32) arg1) + 0x14)) = 0xFC; } while (0);  /* CHEAT */
    *((u8 *) (((s32) arg1) + 0x15)) = 0x82;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0x32;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0x28;
    *((u8 *) (((s32) arg1) + 0x16)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0xA;
  }
  else
  {
    new_var = (s32) arg1;
    *((u8 *) (new_var + 4)) = 0x10;
    *((u8 *) (((s32) arg1) + 5)) = 0x30;
    *((u8 *) (((s32) arg1) + 6)) = 0x60;
    *((u8 *) (((s32) arg1) + 0xD)) = 0;
    *((u8 *) (((s32) arg1) + 0xE)) = 0x40;
    *((u8 *) (((s32) arg1) + 0x14)) = 0x30;
    *((u8 *) (((s32) arg1) + 0x15)) = 0;
    *((u8 *) (((s32) arg1) + 0x16)) = 0x60;
    *((u8 *) (((s32) arg1) + 0xC)) = 0x18;
    *((u8 *) (((s32) arg1) + 0x1C)) = 0;
    *((u8 *) (((s32) arg1) + 0x1D)) = 0;
    *((u8 *) (((s32) arg1) + 0x1E)) = 0;
  }
  ot_Link(D_800A374C + 0x60, arg1);
  return (s32) (((u8 *) arg1) + 0x24);
}
