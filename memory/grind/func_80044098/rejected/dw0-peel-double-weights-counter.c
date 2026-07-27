/* s2 KILL — do-while(0) wrapper around the peel (sanctioned family) is a
 * dead end here: the loop notes double the flow weight of the peel's COUNTER
 * refs too (counter 16@18 pri 35555 back above pointer 20@23 pri 34782 — the
 * v1/a0 flip REVERTS), and the peel stops cross-jump-merging entirely (body
 * emitted twice, 26 insns of wrong structure). Same result for m2 outside,
 * after, and inside the wrapper. */
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
            do {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            } while (0);
            while (a4 != m2) {
                *v1 -= (s32)a6;
                v1++;
                a4--;
            }
        }
    }
}
