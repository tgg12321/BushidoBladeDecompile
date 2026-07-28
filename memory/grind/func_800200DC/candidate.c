/* func_800200DC — SANDBOX-0 candidate (s4, permuter). Base: the s3 floor-5
 * disc-carrier form (both subtraction minuends staged through the
 * currently-dead disc, sanctioned staged-value-reused-variable shape; disc
 * also reused for the sqrt result per s2). NEW in s4: the arm-2 (a2-disc)
 * temp is written into the dead dy (existing variable, single write, value
 * consumed immediately). Mechanism (global.c set_preference /
 * expand_preferences / find_reg, building on the s3 instrumented-cc1
 * evidence): disc dies at that subu, so its prefs (incl. {3}=$v1) merge into
 * dy and flow down the mult/divmodsi4 single_set pref edges to the /32
 * quotient, whose find_reg low-first override then takes $v1 (target)
 * instead of $a0; dy's own $v0 home gives the subu/mult dest the target
 * register. Exhaustion table (sandbox --disable all): natural spelling 5,
 * fresh named temp 5 (naming RTL-neutral), disc-reuse 2 (temp lands disc's
 * $v1 home; target wants $v0), dy-reuse 0. Zero-arm decl-order flip measured
 * NOT load-bearing. sandbox 0 verified 3x this session (168/168, 14 rules
 * stripped); independent rule-free single-function pipeline diff vs
 * asm/funcs/func_800200DC.s EMPTY (tmp/perm_200DC_s4). Found by directed
 * permuter campaign (label arm2-statement-family, 2803 iters, harvested);
 * campaign's own score-0 spelled it as ((dy = a2 - disc) * dist) embedded
 * assignment — refined to the two-statement form. */
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
                    /* FAKE: dead dy reused as the arm-2 (a2-disc) temp,
                     * mechanism: global.c set_preference/expand_preferences —
                     * disc dies at this subu so its {$v1} pref merges into dy
                     * and flows down the mult/divmodsi4 pref edges to the /32
                     * quotient, whose find_reg low-first override then takes
                     * $v1 (target); dy's own $v0 home matches the subu/mult.
                     * lever-exhaustion: memory/grind/func_800200DC/evidence.md
                     * §s4 (natural spelling 5, fresh named temp 5, disc-reuse
                     * 2, dy-reuse 0). */
                    dy = a2 - disc;
                    a0 = (dy * dist) / dy2 / 32;
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
