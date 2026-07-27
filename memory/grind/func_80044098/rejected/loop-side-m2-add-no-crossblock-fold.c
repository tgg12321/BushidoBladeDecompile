/* s2 KILL — spelling the LOOP's decrement as a4 = a4 + m2 (to lift the
 * const-holder's refs while keeping the peel body plain) does NOT fold:
 * combine is intra-basic-block only, the li m2,-1 sits in the preheader and
 * the addu in the loop body -> emitted loop keeps `addu $4,$4,$5` (wrong
 * bytes vs target addiu a0,a0,-1). The IN-block variant (peel-side a4+m2,
 * pP) folds fine but severs the li's dependence pre-sched1, so the li still
 * schedules into the peel tail and the j-stub persists. Both placements
 * measured. */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 hdr;
    s32 a4;
    s32 *a6;
    s32 m2;

    v1 = D_80103608[a0];
    hdr = *(v1 - 1);
    a6 = v1 - 1;
    if (hdr & 0x8000) {
        a4 = hdr & 0x7FFF;
        *(v1 - 1) = a4;
        a4 = a4 - 1;
        if (a4 != -1) {
            m2 = -1;
            *v1 -= (s32)a6;
            v1++;
            a4--;
            while (a4 != m2) {
                *v1 -= (s32)a6;
                v1++;
                a4 = a4 + m2;
            }
        }
    }
}
