/* REJECTED s3: dual-use temp_a0 form WITHOUT the var_a2 hoist.
   sandbox --disable all: score=6, build_insns=52 (== target 52).
   The instruction stream is STRUCTURALLY EXACT - same opcodes, same order,
   both `andi $v1,$a0,0xFFFF` present, correct delay slots. The ONLY residual
   is that the two symmetric mask pseudos take each other's seats
   ($a1 <-> $a2): a global.c allocno-priority tie (pseudo 75 = then-arm mask,
   3 refs / 19 insns, sorts ahead of pseudo 76 = else-arm mask, 3 refs / 21).
   Dead as a FINAL form, but this is the correct 52-instruction chassis to
   re-attack from - the fix must flip that tie without adding an instruction. */
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
  D_801078D8 = temp_v1_d8;
  D_800F1B10 = temp_v0_1b10 & ~temp_v1_d8;
  temp_v0_1b12 = D_800F1B12;
  temp_a0_da = temp_a0_da | var_a1;
  D_801078DA = temp_a0_da;
  D_800F1B12 = temp_v0_1b12 & ~temp_a0_da;
}
