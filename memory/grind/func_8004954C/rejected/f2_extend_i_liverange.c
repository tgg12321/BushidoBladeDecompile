/* REJECTED (s3) — F2 hypothesis measured DEAD. Do NOT re-propose.
 *
 * F2 theory (evidence.md): extending counter i's live-length up to the return
 * would equalize live_length with sum, and the allocno-number tiebreak
 * (sum=75 < i=76) would then allocate sum first -> sum gets $v1 (match).
 *
 * MEASURED: using i post-loop (`return sum + (arg2 - i)`) raises the score to
 * 7 (WORSE than the floor-6 baseline), not lower. Bound-var decoupling
 * (`bound=arg1` for the test, i in the return) also -> 7. The forced-correct
 * faithful form (`if (arg1<=0) i=arg1;`) -> 10 / build_insns 17 (extra branch,
 * broken structure).
 *
 * Why F2 fails (two-axis mechanism, s1): equalizing live_length does NOT flip
 * the priority because i STILL outranks sum on the OTHER axis — frequency
 * (i has 3 loop refs vs sum's 2). Worse, keeping i live to the return forces i
 * to hold its own hard register through the epilogue instead of dying at the
 * loop, so the subtraction can no longer emit as target's `subu v0,a2,a1`
 * (arg1) — it emits against i's register. Net: +1 diff, not -.
 *
 * Also NOT semantically faithful: at loop exit i==arg1 only when arg1>0; for
 * arg1<=0 the loop never runs and i=0 != arg1, so `arg2 - i` != `arg2 - arg1`.
 * There is no faithful post-loop use of i that equals arg1 unconditionally.
 * F2 is KILLED on both counts (raises score AND unfaithful).
 */
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        sum += arg0;
        arg0 -= 1;
    }
    return sum + (arg2 - i);   /* score 7, and unfaithful for arg1<=0 */
}
