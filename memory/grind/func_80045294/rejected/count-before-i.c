/* s11 rejected: count decl placed BEFORE i decl, filling the last
 * untested slot in the {i, s4, count, s5} free-axis permutation cluster.
 * Order: sum, v1, s4, count, i, s5.
 * Measured NEUTRAL (score=2, 83/83). Confirms the s1/s3 free-axis
 * inference (count/i/s5 position is neutral as long as v1-before-i)
 * extends to this specific permutation.  No sched2 tie movement.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 v1 = a0 << 4;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
    s32 count = D_800A33AC;
    s32 i = a0;
    s32 s5 = s4 + a1;
    /* ... unchanged body ... */
}
