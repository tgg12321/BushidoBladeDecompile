/* s61 P1: idx_1495 = &D_800A1495; direct symbol reference honest respelling.
 * Sandbox masked=16, target_insns=160, build_insns=161 (+1 insn).
 * WORSE than the s8 probe1 form (idx_1494 + 1, masked=15, no extra insn).
 * Mechanism: &D_800A1495 emits a SEPARATE lui/addiu pair for a distinct
 * symbol rather than combine-folding to an addiu on the p77 (idx_1494)
 * pseudo. GCC's expand.c produces a fresh single-symref ADDR_EXPR whose
 * REG_EQUIV note declares the constant address of D_800A1495 (a distinct
 * symbol); local-alloc/global-alloc still see the +REG_EQUIV alloc-web
 * rotation (p78 demoted to last), AND the emitted asm has one more insn
 * because the split-symref cannot be substituted through addsi3_internal
 * to save a lui.
 */
  idx_1495 = &D_800A1495;
