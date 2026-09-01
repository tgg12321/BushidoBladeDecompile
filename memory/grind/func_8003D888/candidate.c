/* s2 (structural, 2026-09-01): body unchanged, floor re-measured 13 on the current chassis.
   s2 named the exact gate: find_reg pass 0 excludes regs_someone_prefers, and the COUNT allocno's
   $a1 copy-preference is what keeps the avail/ptr/word carrier out of $a1 in every merged-carrier
   spelling (A/E/B measured 22/22/20). Second, independent gate: allocno_compare orders the count
   (9 refs / 24 insns) ahead of s (9 / 26), so s cannot take $a2 first. See evidence.md s2. */
/* func_8003D888 — s1 best form (recon 2026-08-31), honest floor 13 (sandbox --disable all, 37/37 insns).
   THIS EXACT BODY IS IN src/code6cac_c2.c in the s1 working tree.
   Bitstream reader: s[0]=word ptr, s[1]=current word, s[2]=bits available; returns next n bits.
   Key structural discovery: the original reuses the PARAM `n` as the ptr/word carrier in the
   if-arm (that is how $a1 legitimately carries ptr→word via the entry-copy preference), while a
   fresh local (`cnt` here) carries the request/need count. Remaining residual vs target:
   (a) avail↔cnt seating: ours avail→$a3/cnt→$t0, target avail→$a1/cnt→$a3;
   (b) our `subu` recomputes cnt = n - avail (cse substitutes n for cnt) where target decrements
       in place `subu a3,a3,a1`;
   (c) two if-arm schedule slots (`sllv r` sits after the `and` instead of after the word load;
       `li 32`/`lw word` order swapped). See evidence.md s1 for the full mechanism analysis. */
s32 func_8003D888(u32 *s, s32 n)
{
    s32 cnt = n;
    s32 avail = s[2];
    u32 r;

    if (avail < cnt) {
        u32 m1, m2;
        s32 shift;

        cnt -= avail;
        m1 = (1 << avail) - 1;
        m2 = (1 << cnt) - 1;
        r = s[1] & m1;
        n = s[0];
        s[0] = n + 4;
        shift = 32 - cnt;
        n = *(u32 *)n;
        r <<= cnt;
        s[2] = shift;
        r |= ((u32)n >> shift) & m2;
        s[1] = n;
    } else {
        s[2] = avail - cnt;
        r = (s[1] >> (avail - cnt)) & ((1 << cnt) - 1);
    }
    return r;
}
