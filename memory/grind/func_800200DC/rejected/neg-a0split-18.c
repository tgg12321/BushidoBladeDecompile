/* REJECTED s3(session-3, structural): score 18 (vs floor 5). neg-carrier
 * (both minuends staged through neg, decl at function top) PLUS the zero-arm
 * first quotient split through a0 (`a0 = neg*dx; a0 = a0/denom;
 * arg4[0] = a0;`) to repair the zero-arm local-pool rotation that sank the
 * plain neg-carrier (26).
 * The zero arm DOES match under this spelling (a0 is global, forced to its
 * $a0 home = exactly target's mflo $a0/div/mflo $a0/sw $a0; dy stays $v0).
 * But the else arm rotates: with local-neg gone from the pool, the two
 * (a2+/-disc)*dist mflo products become block-LOCALS colored $v1 (nega0
 * local table: 94, 98 in 3), and via divmodsi4 set_preference (single_set
 * IGNORES the REG_UNUSED mod half — rtlanal.c:601 — so divmods ARE
 * preference/merge edges) both /32 rounding temps acquire LITERAL {3} prefs
 * (greg: ";; 112 preferences: 3", ";; 118 preferences: 3") and steal $v1;
 * disc loses its {3} pref (greg: 102 prefs {4,7} vs {3,4,7} in every intact
 * form), lands $a3, carrier lands $a1. The zero-arm local-neg@$v1 is a
 * load-bearing part of the target equilibrium: it blocks the zero-arm
 * quotient off $v1 AND (channel not yet fully traced) upholds disc's {3}.
 * NO existing variable can host the stages: disc={4}-leak (floor 5), a2=disc
 * conflict (8), neg/denom=local-pool membership load-bearing (26/18), dx dz
 * dist dy dy2 a0 = wrong target home for the stage lw dest ($s-regs/$v0/$a0)
 * or live-range conflict. Artifacts: s3/neg_a0split_18.diff.txt,
 * nega0.{lreg,greg}, nega0_fr{77,112,102}.txt, nega0_alloc_raw.txt,
 * negc.* (plain neg-carrier 26 dumps), prefweb.py. */
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 neg;
    s32 dx;
    s32 dz;
    s32 dist;

    neg = arg1[0]; /* FAKE: stage */
    dx = neg - arg0[0];
    dz = arg1[2] - arg0[2];
    dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy;
        s32 a0;

        neg = arg1[1]; /* FAKE: stage */
        dy = neg - arg0[1];

        if (dy == 0) {
            s32 denom;
            neg = -arg3;
            denom = arg2 * 2;
            a0 = neg * dx; /* FAKE: quotient staged through a0 (split-init) */
            a0 = a0 / denom;
            arg4[0] = a0;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
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
