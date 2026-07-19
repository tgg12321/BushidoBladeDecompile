/* s11 rejected: GCC statement-expression wrap around a0<<4.
 * Form: `s32 v1 = ({ a0 << 4; });` in place of `s32 v1 = a0 << 4;`.
 * Rationale for probing: s10 killed do-while(0) wrap as a way to create
 * a cse-visible BB boundary; statement-expression is a distinct GCC
 * extension that lowers via a different C-front-end path (expr_stmt
 * inside a COMPOUND_STMT rvalue).  If GCC 2.7.2 emitted a
 * NOTE_INSN_BLOCK_BEG / BLOCK_END or otherwise fenced the value chain,
 * it could break cse.c's BB-scoped substitution (s7 mechanism).
 * Measured NEUTRAL (score=2, 83/83). GCC 2.7.2's c-parse.y collapses
 * `({ expr; })` to the bare expression at parse time when the body is a
 * single expression statement -- the resulting tree/RTL is IDENTICAL to
 * the baseline `s32 v1 = a0<<4;`.  No cse boundary is emitted; no LUID
 * shift.  Independently confirms s10's mechanism-agnostic conclusion:
 * no C-source-level lever emits a fresh cse basic block between i=a0
 * and v1=a0<<4 for this function.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = ({ a0 << 4; });
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    /* ... unchanged body ... */
}
