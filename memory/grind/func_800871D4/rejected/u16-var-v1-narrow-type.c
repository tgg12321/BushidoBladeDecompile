/* REJECTED s2: u16 var_v1 narrow-type.
 * Motivation: declare var_v1 as u16 to change codegen decisions vs u32.
 * Result: sandbox --disable all -> score=11 (WORSE than u32 floor=10).
 * build_insns=52 == target=52 (same insn count), but 11 mismatched positions.
 * Narrowing var_v1 does not steer GCC toward the oracle's $a0/$v1 split.
 */
void func_800871D4(s32 a0_arg)
{
  u16 var_v1;
  s32 var_a2;
  s32 var_a1;
  s32 temp_v0;
  u16 temp_v1_d8;
  u16 temp_a0_da;
  u16 temp_v0_1b10;
  u16 temp_v0_1b12;

  var_v1 = D_8010280A;
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
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  /* ... rest unchanged ... */
}
