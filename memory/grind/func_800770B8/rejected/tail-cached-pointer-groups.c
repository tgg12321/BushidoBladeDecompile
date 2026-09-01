/* REJECTED s1 (2026-09-01): chassis-style cached-pointer grouping in the tail
 * (p = D_800A36A0; two stores; p = D_800A36A0; ...). Hypothesis: target shares
 * one gp-load per statement group. MEASURED: 20 -> 29, build_insns 176 -> 174:
 * GCC 2.7.2 cse MERGES the repeated `p = D_800A36A0` reads (the intervening
 * byte stores through p do not invalidate the gp-rel load for it), so the
 * reload count DROPS below target. Direct global derefs per statement (the
 * floor-14 form) already produce the target's reload pattern exactly — the
 * tail rows 140-166 are byte-clean in the s1 posdiff. Do not re-propose.
 */
{
    u8 *p = D_800A36A0;
    *(s32 *)p = arg1;
    *(s8 *)(p + 0x65) = 0;
    p = D_800A36A0;
    *(u8 *)(p + 0x67) = 1;
    p = D_800A36A0;
    *(u8 *)(p + 0x66) = (&D_8009BD21)[*(u8 *)(p + 0x67) * 2];
}
