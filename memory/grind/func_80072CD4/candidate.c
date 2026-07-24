/* func_80072CD4 — best CLEAN (non-cheat) form, sandbox --disable all = 18, build_insns 78 (target 79).
 * NOTE: the imported floor=12 depends on the reviewer-FAIL do-while(0) barrier form
 * (rejected/plus4_first_dowhile.c) — NOT a legitimate pure-C floor. This is the best
 * legitimate form measured (s1). Mechanism grounded this session:
 *   - `int fc_const` hoisted before inner-if mirrors matched sibling func_80072BC4
 *     (which byte-matches via that shape). Places 0xFC in a held register.
 *   - Writing @0x14 as a DISTINCT `0xFC` literal (not fc_const) defeats CSE and forces
 *     a second 0xFC re-materialization → 77→78 insns, score 20→18. Target has this re-mat.
 *   - Still 1 insn short (78 vs 79) and the $v0/$v1 rotation is inverted vs target
 *     (target: var_v0 in $v0, 0xFC in $v1; our build: the reverse). RA-steer needed.
 */
s32 func_80072CD4(s32 arg0, GameObj *arg1)
{
  u8 var_v0;
  s32 new_var;
  int fc_const;
  initPolyG4(arg1);
  gpu_SetSemiTransp(arg1, 0);
  if (arg0 < 4)
  {
    fc_const = 0xFC;
    if ((*((s32 *) (((s32) D_800A35C4) + 8))) & 4)
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x1E;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xC8;
      var_v0 = 0x32;
    }
    else
    {
      *((u8 *) (((s32) arg1) + 5)) = 0xC3;
      *((u8 *) (((s32) arg1) + 6)) = 0x50;
      *((u8 *) (((s32) arg1) + 0xD)) = 0xDC;
      var_v0 = 0x46;
    }
    *((u8 *) (((s32) arg1) + 4)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xC)) = fc_const;
    *((u8 *) (((s32) arg1) + 0xE)) = var_v0;
    *((u8 *) (((s32) arg1) + 0x14)) = 0xFC;
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
