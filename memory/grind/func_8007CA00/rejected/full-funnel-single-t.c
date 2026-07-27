/* REJECTED (s3, p11) — ALL paths funneled through one t + `ret: return t`.
 * sandbox floor = 9 (build 43). This form DISCOVERED the H1 cure (==0 division
 * flipped to target's lhu v0 / acc v1 / cross sra — because the ==0 block's
 * last insn becomes a pseudo-dest in its own BB, killing the combine backward
 * $v0 squeeze) but t itself lands in $a1 (conflicts hard 2 & 3 per greg:
 * sched1 hoists case2's t=0x400 above the div chain (backward-scheduler float,
 * no birthing promotion since t is multi-set) -> t overlaps the div's
 * local-$v0/$v1 hard ranges). Every t-touching site pays: li a1 x2,
 * subu a1 x2, sra a1,v1,1, lh a1, move v0,a1 in the jr delay.
 * Superseded by candidate.c (partial funnel: tail returns directly via a
 * BLOCK-LOCAL c; only ==0/case1==0/default go through t) = floor 4. */
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
            t = t - a;
            goto ret;
        }
        t = arg0[0];
        goto ret;
    case 2:
        if (0 != D_8009BE77) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            a = arg0[0];
            t = 0x400;
            goto sub;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    }
}
