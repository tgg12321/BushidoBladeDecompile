/* REJECTED s4: only the D_801078DA `or` moved up; the DA store and the
   D_800F1B12 write-back stay at the bottom, so the else-arm mask still dies
   at the same RTL insn and its live_length is unchanged.
   sandbox --disable all: score=6, build_insns=52.  The interleave has to move
   the whole DA group, not just the OR. */
void func_800871D4(s32 a0_arg)
{
  u32 temp_a0;
  u32 var_v1;
  s32 var_a2;
  s32 var_a1;
  s32 temp_v0;
  u16 temp_v1_d8;
  u16 temp_a0_da;
  u16 temp_v0_1b10;
  u16 temp_v0_1b12;

  temp_a0 = D_8010280A;
  var_v1 = temp_a0 & 0xFFFF;
  if (var_v1 < 0x10U)
  {
    var_a2 = 1 << var_v1;
    var_a1 = 0;
  }
  else
  {
    var_a2 = 0;
    var_v1 -= 0x10;
    var_a1 = 1 << var_v1;
    var_v1 = temp_a0 & 0xFFFF;
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  *((s8 *)((u8 *)&D_800F4E35 + temp_v0)) = 0;
  temp_v1_d8 = D_801078D8;
  temp_a0_da = D_801078DA;
  *((s16 *)((u8 *)&D_800F4E1C + temp_v0)) = 0;
  *((s16 *)((u8 *)&D_800F4E18 + temp_v0)) = 0;
  temp_v0_1b10 = D_800F1B10;
  temp_v1_d8 = temp_v1_d8 | var_a2;
  temp_a0_da = temp_a0_da | var_a1;
  D_801078D8 = temp_v1_d8;
  D_800F1B10 = temp_v0_1b10 & ~temp_v1_d8;
  temp_v0_1b12 = D_800F1B12;
  D_801078DA = temp_a0_da;
  D_800F1B12 = temp_v0_1b12 & ~temp_a0_da;
}
