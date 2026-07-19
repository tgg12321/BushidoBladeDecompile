/* s12 KILLED: sum=0 moved from position 0 to position 5 (after v1/s4/i/count/s5).
 * Result: score=2 -> 4, build_insns=83 unchanged. Two register-choice diffs surface;
 * the sll/move16 sched2 residual persists. sum's decl position IS NOT a free axis
 * despite the s1/s3 free-axis inference for {i,s4,count,s5}: pushing sum to LUID
 * after s5 rotates its RA (was $17, now different) which cascades into the
 * accumulator loop. Confirms sum-at-position-0 is load-bearing, distinct from the
 * free-axis cluster of {i,s4,count,s5} that only requires v1-before-i.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;
    s32 sum = 0;
    /* ... rest identical to candidate.c ... */
}
