/* REJECTED (s3) — cheat-form, score 3 (build_insns 14). Do NOT re-propose.
 *
 * F3 mechanism (raise sum's loop-ref count to lift its allocno n_refs above
 * counter i's) CONFIRMED to flip RA: score drops 6 -> 3. But the only construct
 * that raises sum's refs is this redundant-arithmetic split of a single add.
 * arg0/2 + (arg0 - arg0/2) == arg0 for all int arg0 (faithful VALUE), and GCC
 * folds it back so build_insns stays 14 — i.e. the split is byte-neutral in
 * COUNT and exists ONLY to bump reg_n_refs(sum) during global allocno priority
 * (global.c:604 allocno_compare = flog2(nrefs)*nrefs/live_length).
 *
 * No human programmer writes `sum += arg0/2; sum += arg0 - arg0/2;` to compute
 * a triangular sum. Its sole purpose is to change GCC's allocno analysis with
 * no effect visible in the emitted output. That is a cheat by spelling
 * (no-new-park-categories "cheats by any spelling"; same species as the s2
 * off-early live-range-steering and the paren-reassoc n-refs/live steering).
 * It also does not even reach 0 (residual 3 = the delay-slot/subu scheduling).
 *
 * Contrast f3_sum_temp (`s32 t = sum + arg0; sum = t;`) which stays at 6 —
 * a bare extra read is DCE'd before n_refs counting, so no legitimate faithful
 * form raises sum's per-iteration refs. F3-as-legitimate-lever is therefore
 * KILLED: the triangular sum genuinely references the accumulator once per
 * iteration and no semantically-natural restructure changes that.
 */
s32 func_8004954C(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sum = 0;
    s32 i;
    for (i = 0; i < arg1; i++) {
        sum += arg0 / 2;
        sum += arg0 - arg0 / 2;
        arg0 -= 1;
    }
    return sum + (arg2 - arg1);
}
