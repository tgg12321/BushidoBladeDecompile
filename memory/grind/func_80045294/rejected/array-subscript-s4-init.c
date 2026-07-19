/* s45 REDERIVE PROBE - KILLED (NEUTRAL, score=2, 83/83)
 *
 * Sub-axis 17: expand-path via array-subscript syntax at s4's init.
 * Rewrite the s4 init from pointer-arith `*(s32*)((u8*)&SYM + v1)` to
 * array-subscript `((s32*)&SYM)[a0 << 2]` (element-index = a0*4, byte
 * offset = a0*16 == v1). This exercises expand_expr's ARRAY_REF path
 * rather than the INDIRECT_REF+PLUS_EXPR path.
 *
 * Prediction: probably neutral (GCC folds ARRAY_REF to (mem (plus
 * (ashift a0 4) (symbol_ref sym))) at fold-const before tree_LUID is
 * assigned).
 * Measured: score=2, target=83, build=83 -- NEUTRAL. Byte-identical
 * to baseline candidate.c. GCC 2.7.2 fold-const collapses the
 * ARRAY_REF-with-INT_CST-subscript pattern to the same PLUS_EXPR
 * pointer-arith tree at fold time, so the ashift/mem-plus RTL sequence
 * emitted at expand is identical. No LUID movement.
 *
 * Distinct rederive angle from s5's arithmetic-tree probes (which
 * varied the a0<<4 SUB-expression via (u32)a0*16u, a0*16, (u8*)0+a0,
 * ptrdiff) and from s39's addr-cast probes (which varied the BASE
 * cast via (u32)&D_800EED14 / (u32)a0 signature). This one varies the
 * DEREF SYNTAX (subscript vs. pointer-arith) at the outermost level.
 *
 * Adds a 17th independent rederive sub-axis to the ledger's tally,
 * corroborating the s26/s37 "m2c-via-any-flag converges on H1 shape"
 * closure from an independent, non-m2c evidence surface.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = ((s32 *)&D_800EED14)[a0 << 2];
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... (rest identical to baseline candidate.c) ... */
}
