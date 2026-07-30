/* func_800477E8 — best known form as of grind session s2 (2026-07-30, structural).
 *
 *   honest floor (sandbox --disable all): 17   (UNCHANGED from s1)
 *   build_insns == target_insns == 170
 *
 * The FLOOR is the same as HEAD's, but this form is strictly more advanced and
 * is the base the next session should start from, because the composition of
 * the residual has changed completely:
 *
 *   HEAD   : loop1 seating WRONG (v1/a1 swapped), loop2 seating WRONG,
 *            tie A (t1val/t2) WRONG                            -> 6 + 6 + 5 = 17
 *   THIS   : loop1 seating EXACTLY TARGET for every local,
 *            loop2 seating WRONG, tie A WRONG                  -> 0 + 12 + 5 = 17
 *
 * Measured with the cc1 -da `.greg` allocation dump (tmp/grind/func_800477E8/s2/):
 *   loop1  a0->$a0(4)  a1->$a1(5)  a2->$a2(6)  a3->$a3(7)  t0->$t0(8)
 *          v1->$v1(3)  v0->$v0(2)  s0->$s0  s1val/s2val/s3val->$s1/$s2/$s3
 *   ALL of those equal target. Remaining diffs are ONLY:
 *     tie A : t1val->$t2(10) / t2->$t1(9)   -- target wants t1val=9, t2=10
 *     loop2 : val->$a3(7) ptr->$v1(3) w->$a1(5) r->$a2(6)
 *             -- target wants val=$v1(3) ptr=$a1(5) w=$a2(6) r=$a3(7)
 *
 * WHAT CHANGED vs HEAD, and why each change is load-bearing:
 *
 * 1. loop2's row counter / accumulator / stored value are now their OWN locals
 *    (`r`, `w`, `val`) instead of reusing loop1's `a3`, `a2`, `v1`. They are
 *    semantically unrelated quantities that m2c merged because they shared a
 *    hard register. This is what frees loop1's `v1` to win $v1: while `v1` is
 *    shared with loop2 it CONFLICTS with the loop2 walking pointer `ptr`, and
 *    `ptr` (a tight-range induction pseudo, priority ~1.6) always out-ranks a
 *    loop-spanning value pseudo (~0.26) in GCC 2.7.2 global.c allocno_compare,
 *    so `ptr` takes $v1 first, every time. Splitting removes the conflict.
 *    NOTE this OVERTURNS s1's K2 conclusion ("v1 must stay shared"): K2 read
 *    the SCORE (17->31) instead of the SEATING, and the seating shows the split
 *    is a prerequisite, not a regression.
 *
 * 2. `a3 = 0;` sits BETWEEN `t2 = 0x2C00;` and `a0 = 0;`. This is pure statement
 *    order among three initialisations and is worth 9 points: with `a3 = 0;`
 *    first (HEAD's order) a3 and t0 swap ($a3 gets $t0), and with `a3 = 0;` last
 *    the whole loop1 seating collapses (score 26). The mechanism is
 *    allocno_live_length: a3 has 6 refs to t0's 5, so a3 only wins the earlier
 *    register when its live range is short enough that
 *    floor_log2(6)*6/len(a3) > floor_log2(5)*5/len(t0). It is arbitrary-looking
 *    and a successor should try to reach the same seating from a natural loop
 *    shape instead (a real for() over the row counter) before proposing this to
 *    the Judge.
 *
 * KNOWN-DEAD, do not re-derive (s1 + s2, all measured):
 *   - splitting `ptr` between loop2 and loop3, or turning loop3 into an indexed
 *     loop: completely inert (identical .greg dump).
 *   - declaration order alone (v1 first, t1val last, a1 before a2): inert.
 *   - giving loop2 its own inner counter: catastrophic (17 -> 40).
 *   - hoisting `t2 = 0x2C00` above gpu_CalcClut: +2 insns (callee-save pair).
 *   - inlining the 0x2C00 literal: GCC re-CSEs it, seating unchanged (17 -> 18).
 *
 * OUTSTANDING JUDGE SURFACE: the `do { v0 = v1 | a1; } while (0);` below is
 * inherited from HEAD and is NOT `/* FAKE *\/`-annotated. Any session that
 * reaches distance 0 must annotate it per .claude/rules/do-while-zero-exception
 * or eliminate it before proposing the candidate.
 */

s32 func_800477E8(void) {
    s16 *s0;
    s32 s3val;
    s32 s2val;
    s32 s1val;
    s32 t1val;
    s32 a3;
    s32 a0;
    s32 a2;
    s32 a1;
    s32 t0;
    s32 v1;
    s32 t2;
    s32 v0;
    s32 *ptr;
    s32 *p;
    s32 w;
    s32 r;
    s32 val;

    s0 = D_800A33D0;
    s3val = gpu_CalcTPage(0, 0, 0x2C0, 0x1C0);
    s2val = gpu_CalcTPage(0, 0, 0x2C0, 0x180);
    s1val = gpu_CalcClut(0x10, 0x1E0);
    t1val = gpu_CalcClut(0x10, 0x1E0);
    t2 = 0x2C00;
    a3 = 0;
    a0 = 0;
    do {
        t0 = 0x1200;
        a2 = 0x13;
        a1 = 0;
        v1 = 1;
inner:
        if (a3 >= 5) {
            *s0 = s3val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = s1val;
            s0 += 1;
            v0 = -0xC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x3FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4000;
        } else {
            *s0 = s2val;
            s0 += 1;
            *s0 = t2;
            s0 += 1;
            *s0 = t1val;
            s0 += 1;
            v0 = -0x40C1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x4100;
            *s0 = v0;
            s0 += 1;
            v0 = -0x7FC1;
            *s0 = v0;
            s0 += 1;
            v0 = -0x8000;
        }
        *s0 = v0;
        s0 += 1;
        if (a3 & 1) {
            v0 = a2 | t0;
            *s0 = v0;
            s0 += 1;
            do { v0 = v1 | a1; } while (0);
        } else {
            v0 = v1 | a1;
            *s0 = v0;
            s0 += 1;
            v0 = a2 | t0;
        }
        *s0 = v0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        *s0 = 0;
        s0 += 1;
        v0 = -0x1000;
        *s0 = v0;
        s0 += 1;
        t0 += 0x100;
        a2 += 1;
        a1 += 0x100;
        a0 += 1;
        v1 += 1;
        if (a0 < 0x10) goto inner;
        a3 += 1;
        a0 = 0;
    } while (a3 < 8);

    {
        s32 *a0p;
        a0p = (s32 *)&D_800EF070;
        *(s8 *)a0p = 0xE;
        D_800EF07A = 4;
        D_800EF0BC = -0x2EE0;
        D_800EF071 = 0;
        D_800EF0C0 = 0;
        D_800EF0C4 = -0xFA0;
        D_800EF080 = 0;
        D_800EF082 = 0;
        D_800EF084 = 0;
        D_800EF078 = 0;
        D_800EF07C = 0;
        D_800EF076 = 0;
        func_800417D0(a0p);
    }

    r = 0;
    w = 0;
    ptr = &D_800EF59C[0];
outer2:
    val = w;
    a0 = 0x10;
    p = ptr + 0x10;
inner2:
    *p = val;
    a0--;
    p--;
    if (a0 >= 0) goto inner2;
    w += 0x7D0;
    r += 1;
    ptr += 0x11;
    if (r < 9) goto outer2;

    a0 = 0;
    ptr = &D_800EF558[0];
loop3:
    *ptr = (a0 << 7) & 0xFFF;
    a0++;
    ptr++;
    if (0x11 > a0) goto loop3;

    return (s32)s0 - (s32)D_800A33D0;
}
