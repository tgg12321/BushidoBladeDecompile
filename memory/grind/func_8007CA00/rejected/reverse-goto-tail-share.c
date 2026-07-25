/* REJECTED (s2) — reverse-direction shared-tail goto. sandbox floor = 14 (WORSE
 * than 11). case1 falls through to `sub:` and case2/BE77!=0 gotos UP to it.
 * RESULT: build_insns dropped to 44 (target=44, the +1 duplicate subu is GONE —
 * case1 no longer jumps, so reorg has no delay slot to fill), BUT the shared
 * block absorbs the arg0[0] load + the `li 0x400`, forcing a reload and a bad
 * register cascade (arg0[2]->a1, arg0[0]->v1, extra nop). Net 14 diffs.
 * Confirms the frontier's "without a forced goto": ANY explicit label-sharing of
 * the CA40 tail degrades RA (H-A forward goto = 17, this reverse goto = 14).
 * The natural cross-jump already merges the full 2-subu suffix; the only H2 debt
 * is the reorg delay-slot-fill dup, which needs case1 fall-through WITHOUT
 * over-sharing the loads. */
extern u8 D_8009BE74;
extern u8 D_8009BE77;
s32 func_8007CA00(s16 *arg0) {
    s32 v1;
    switch (D_8009BE74) {
    case 1:
        if (D_8009BE77 != 0) {
            v1 = arg0[2];
        sub:
            return (0x400 - v1) - arg0[0];
        }
        return arg0[0];
    case 2:
        if (D_8009BE77 != 0) {
            v1 = ((s16)(*((u16 *)(arg0 + 2)))) / 2;
            goto sub;
        }
        return ((s32)((s16)(*((u16 *)arg0)))) / 2;
    default:
        return arg0[0];
    }
}
