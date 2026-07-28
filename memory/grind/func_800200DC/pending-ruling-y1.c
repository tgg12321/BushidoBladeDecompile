/* SANDBOX 0 FORM — PENDING OWNER RULING (s3, 2026-07-28).
 * `sandbox func_800200DC --disable all` = 0 (168/168, 14 rules stripped) with
 * this form in src/code6cac.c; independent rule-free pipeline diff = 0
 * (artifact s3/final_y1.diff.txt).
 * Layer-1 cheat-reviewer FAILED it: y1 is a FRESH variable invented to carry
 * the two staged minuends, which [[staged-value-reused-variable]] explicitly
 * excludes ("inventing a new variable just to have something to borrow is NOT
 * this rule"). Counter-precedent: the ALLOWED "named-intermediate" family
 * (SOTN randy chain, w_037.c) ships fresh named intermediates staging values;
 * the delta here is the TWO writes (multi-assignment property).
 * Body is byte-identical-producing; both stores are live (each read by the
 * following subtraction); no pins/volatile/asm/dead stores. */
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 y1;
    s32 dx;
    s32 dz;
    s32 dist;

    y1 = arg1[0]; /* FAKE: stage the minuends through y1 */
    dx = y1 - arg0[0];
    dz = arg1[2] - arg0[2];
    dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy;

        y1 = arg1[1]; /* FAKE: stage the minuends through y1 */
        dy = y1 - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 a0;
            s32 disc = arg2 * arg2 + arg3 * dy2;

            if (disc >= 0) {
                s32 a2;
                disc = func_8007E11C(disc << 10);
                a2 = arg2 << 5;
                a0 = ((a2 + disc) * dist) / dy2 / 32;

                if (a0 < 0) {
                    a0 = ((a2 - disc) * dist) / dy2 / 32;
                }
            } else {
                a0 = 300;
            }

            if (a0 >= 301) {
                a0 = 300;
            }

            arg4[0] = (a0 * dx) / dist;
            arg4[2] = (a0 * dz) / dist;
        }
    }
}
