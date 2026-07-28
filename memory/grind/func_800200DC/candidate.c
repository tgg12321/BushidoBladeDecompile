/* func_800200DC — floor-5 candidate (s3). disc-reuse (s2) + BOTH subtraction
 * minuends staged through the currently-dead disc (sanctioned
 * staged-value-reused-variable shape: existing var, live staged reads, FAKE-
 * annotated). Mechanism (proven with instrumented cc1 FINDREG/ALLOCDBG, s3):
 * a raw `dy = arg1[1] - arg0[1]` gives dy a $v1 preference via global.c
 * set_preference (minuend's local-alloc hard reg); find_reg's pref-override
 * then steals dy from $v0 to $v1 and rotates 3 regs (the old 6-insn residual).
 * Staging the minuend through a cross-block pseudo (unallocated at
 * set_preference time) kills the preference; dy stays $v0, arg2^2 takes $v1,
 * reload lands the disc-addu mflo in $t1 — all matching target. The dx-side
 * stage is load-bearing: disc's first segment overlaps arg1-ptr, blocking an
 * expand_preferences merge that otherwise leaks arg1-ptr's $a1 entry pref
 * into the arm-1 quotient (measured $t0 without it).
 * Residual 5 = arm-2 quotient lands $a0 instead of $v1: disc's own $a0 pref
 * (combine folds `sll $a0,disc,10` into the call-arg set) flows down the
 * dy->dy2->divres->quotient expand_preferences merge chain when disc dies at
 * the staged dy-subu; the quotient's override then takes free $a0.
 * A fresh two-write relay (y1) with empty prefs scores sandbox 0 (bytes
 * proven, artifact s3/final_y1) but was layer-1 FAILED as outside the
 * staged-value sanction (fresh variable) — OWNER RULING REQUESTED. */
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 disc;
    s32 dx;
    s32 dz;
    s32 dist;

    disc = arg1[0]; /* FAKE: stage the minuend through the currently-dead disc */
    dx = disc - arg0[0];
    dz = arg1[2] - arg0[2];
    dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy;

        disc = arg1[1]; /* FAKE: stage the minuend through the currently-dead disc */
        dy = disc - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 a0;

            disc = arg2 * arg2 + arg3 * dy2;

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
