/* REJECTED — m2c-reconstructed shape (semantically-correct restore of var_v1 in else arm).
   Sandbox score = 12 (WORSE than direct-form floor=10). GCC folds both `& 0xFFFF` (source is
   u16 extern, provably zero-extended by lhu) and additionally reshuffles RA in a way that adds
   diffs vs the in-place-modify shape.  s1 (2026-07-28). */
void func_800871D4(s32 a0_arg) {
  u32 var_v1;
  s32 var_a2, var_a1, temp_v0;
  u16 temp_v1_d8, temp_a0_da, temp_v0_1b10, temp_v0_1b12;

  var_v1 = D_8010280A & 0xFFFF;
  if (var_v1 < 0x10U) {
    var_a2 = 1 << var_v1;
    var_a1 = 0;
  } else {
    var_a2 = 0;
    var_a1 = 1 << (var_v1 - 0x10);
    var_v1 = D_8010280A & 0xFFFF;   /* restore — folded by cc1 */
  }
  temp_v0 = ((((var_v1 * 8) - var_v1) * 4) - var_v1) * 2;
  /* ...remainder identical to candidate.c... */
}
