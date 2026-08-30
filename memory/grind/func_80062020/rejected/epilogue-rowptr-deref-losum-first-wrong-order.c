/* REJECTED (mixes, but in the INVERSE arrangement to the target) — grind s8,
 * 2026-08-30, forensics modality.
 *
 * This is the family that FALSIFIES prong 2 of the s5/s6 "two-shape theorem"
 * ("symbol-keeping shapes fold K into the symbol for ALL columns and never form
 * a shared base"). The pointer-to-row deref spelling KEEPS the symbol and STILL
 * forms register bases — but the LO_SUM column is always the FIRST store in
 * source order, never the last:
 *
 *   (*(Tbl2 + n))[2] = 0;  ->  sw $0,Tbl2+8($ofs)     LO_SUM
 *   (*(Tbl2 + n))[1] = 0;  ->  addu ...; sw $0,0($r)  base+disp 0
 *   (*(Tbl2 + n))[0] = 0;  ->  addu ...; sw $0,0($r)  base+disp 0
 *
 * All six column orders and five spelling variants of the family were measured
 * standalone (tmp/grind/func_80062020/s7/falsif2_results.txt): the LO_SUM column
 * is ALWAYS the first-emitted one and the other two get their own bases derived
 * from it by `addu $r,$sym,-4/-8`, which costs more insns than the uniform
 * pointer body. The target needs the opposite (shared base+disp 8/4 FIRST, plain
 * LO_SUM LAST), so this family cannot reach it and is strictly worse than
 * candidate.c's floor 4. Mechanism (dump-proven, see falsif2_/dumps_ artifacts):
 * combine folds a SINGLE-USE address pseudo whose def chain ends in a symbolic
 * constant into the MEM; the first column's chain is single-use so it folds, the
 * later columns' chains are two-step (`addu -4`) so combine refuses them.
 *
 * Do not re-propose any spelling of `(*(A + n))[K]` / `*(*(A + n) + K)` /
 * `*((s32 *)(A + n) + K)` / `*(s32 *)((u8 *)(A + n) + 4*K)` / `A[n*3+K]`.
 */
