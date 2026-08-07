/* s99 rederive P1 — cross-symbol delta with D_800A1495 minuend (no explicit +1).
 * Applied to src/system.c line 384 in place of the h5 initializer.
 * Result: masked=4 (+2 vs h5), build_insns=161 (+1 insn).
 * KILL: replacing 1494 with 1495 in the SYMBOL_REF-SYMBOL_REF-diff DOES NOT
 *       preserve h5 basin — the RTL emission adds an extra insn AND regresses
 *       the pair-swap pattern. h5 basin depends specifically on the
 *       (D_800A1494 - D_800A125C) + 1 factorization (see s99 P2 which showed
 *       ((D_800A1494 + 1) - D_800A125C) IS INERT at masked=2, so it's the
 *       symbol-pair identity, not the +1 position, that's load-bearing).
 * This narrows s97's ledger-CONFIRMED insn 34/38 attribution: the flow-time
 * REG_N_REFS[p79]=5 profile requires the specific SYMBOL_REF pair 1494/125C,
 * not any arbitrary cross-symbol pair with the same numeric delta.
 */
  idx_1495 = (u8 *)((u8 *)tbl_125c + ((s32)&D_800A1495 - (s32)D_800A125C));
