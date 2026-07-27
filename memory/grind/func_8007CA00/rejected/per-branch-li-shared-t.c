/* REJECTED (s3, p9/p10) — per-branch `t = 0x400` with mid-block label after
 * case1's li (the "p9 shape" — structurally what target's original source must
 * have looked like). sandbox floor = 12 (build 43).
 * WHY DEAD (two independent proven mechanisms):
 * 1. sched.c: GCC 2.7.2 list-schedules BACKWARD; an isolated no-dependency
 *    constant-set floats to the BLOCK TOP unless promoted by adjust_priority's
 *    birthing_insn_p — which requires reg_n_sets[dest]==1. A shared t is set
 *    in BOTH branches (>=2 sets) -> never promoted -> case2's li hoists above
 *    the div chain -> t live across the div's local-$v0/$v1 ranges.
 * 2. global.c marks sets-before-deaths: t dying at the combine-folded
 *    (set $v0 (minus t a)) return conflicts with hard $v0 (greg dump:
 *    "75 conflicts: ... 2 3"). Both mechanisms force t -> $a1: li a1,
 *    subu a1,a1,v1, etc. (5-6 diffs) and the accumulator respelling
 *    t=t-v1; t=t-a; return t; (p10) produces IDENTICAL bytes (combine
 *    re-folds). Do NOT re-propose any form where one variable carries 0x400
 *    from both branches into the shared tail. A pure-C fix would need t
 *    single-set, which two branch inits make impossible; the only remaining
 *    spellings are constant-holder locals w/ coalesced copy — that is
 *    [[named-local-fake-exception]] territory (FAKE-annotated, last resort),
 *    not a free lever. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1, a, t;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            t = 0x400;
            v1 = arg0[2];
            a = arg0[0];
        sub:
            t = t - v1;
            return t - a;
        }
        return arg0[0];
    case 2:
        if (0 != D_8009BE77) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            t = 0x400;
            goto sub;
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    default:
        return arg0[0];
    }
}
