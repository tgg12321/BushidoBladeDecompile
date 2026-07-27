/* REJECTED (s3, p13) — case2 reaches the tail via a stub block
 * `sub0: t = 0x400; goto sub;` so sched1 can't hoist the li (own BB).
 * sandbox floor = 6 (build 47, +3 insns). t DOES get $v0 and case1 + the ==0
 * block become byte-exact — but the stub survives: reorg's
 * fill_simple_delay_slots fills case2's `j sub0` from its OWN thread first
 * (sra v1,v1,1 is eligible, immediately precedes the j), so the target-thread
 * steal (copy li + retarget to sub) never runs; sub0 remains as
 * [j sub; li-delay] (+2) and `ret:` needs a jump around the stub (+1, default
 * no longer falls into jr). For the steal to fire, case2's j slot would have
 * to find NO eligible own-thread insn — sra1/lh are always eligible.
 * Do NOT re-propose stub placements unless a way to block own-thread fill
 * is found. */
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
            goto sub0;
        }
        t = ((s32)((s16)(*((u16 *)arg0)))) / 2;
        goto ret;
    default:
        t = arg0[0];
    ret:
        return t;
    sub0:
        t = 0x400;
        goto sub;
    }
}
