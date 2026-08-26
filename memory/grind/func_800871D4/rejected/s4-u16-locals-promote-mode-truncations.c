/* REJECTED s4: both `raw` and `vc` declared u16, no explicit mask - the
   'a human would just use an unsigned short' reading.  GCC 2.7.2's PROMOTE_MODE
   keeps HImode locals in SImode registers and re-truncates after the in-place
   `vc -= 0x10`, so the form costs THREE extra instructions.
   sandbox --disable all: score=14, build_insns=55 (target 52). */
void func_800871D4(s32 a0_arg)
{
  u16 temp_a0;
  u16 var_v1;
  s32 var_a2;
  s32 var_a1;
  s32 temp_v0;
  u16 temp_v1_d8;
  u16 temp_a0_da;
  u16 temp_v0_1b10;
  u16 temp_v0_1b12;

  temp_a0 = D_8010280A;
  var_v1 = temp_a0;
  if (var_v1 < 0x10)
  {
    var_a2 = 1 << var_v1;
    var_a1 = 0;
  }
  else
  {
    var_a2 = 0;
    var_v1 -= 0x10;
    var_a1 = 1 << var_v1;
    var_v1 = temp_a0;
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  *((s8 *)((u8 *)&D_800F4E35 + temp_v0)) = 0;
  temp_v1_d8 = D_801078D8;
  temp_a0_da = D_801078DA;
  *((s16 *)((u8 *)&D_800F4E1C + temp_v0)) = 0;
  *((s16 *)((u8 *)&D_800F4E18 + temp_v0)) = 0;
  temp_v0_1b10 = D_800F1B10;
  temp_v0_1b12 = D_800F1B12;
  temp_v1_d8 = temp_v1_d8 | var_a2;
  temp_a0_da = temp_a0_da | var_a1;
  D_801078D8 = temp_v1_d8;
  D_801078DA = temp_a0_da;
  D_800F1B10 = temp_v0_1b10 & ~temp_v1_d8;
  D_800F1B12 = temp_v0_1b12 & ~temp_a0_da;
}
