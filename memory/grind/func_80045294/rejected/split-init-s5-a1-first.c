/* s39 KILLED (score 2 -> 3).
 * Split-init on s5 with a1 as the seed value (mirror of s3's
 * split-init-s5.c which used s4 as the seed).  Form:
 *     s32 s5 = a1;
 *     s5 += s4;
 * Result: sandbox --disable all -> score=3, target_insns=83, build_insns=83
 * (+1 diff versus baseline; likely a $19/$20 addu operand-order swap on
 * insn 32, echoing s2's swap-s5-operands.c evidence that GCC 2.7.2 does
 * NOT commutatively canonicalize addu RHS operands at expand time).
 * Mechanism (union with s3): split-init on s5 introduces an extra copy
 * insn that displaces $21's block-0 addu, in addition to (for the a1-first
 * seed) reversing addu's operand order, producing 1 more diff than the
 * baseline non-split form. Both split-init variants of s5 (s4-first
 * banked s3, a1-first here) are harmful. Not a lever.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = a1;
    s5 += s4;
    /* ... rest identical to baseline ... */
}
