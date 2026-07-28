/* REJECTED s3(session-3, structural): score 8 (vs floor 5). Staging both
 * minuends through the EXISTING a2 (decl hoisted to function top; else-arm
 * keeps a2 = arg2 << 5).
 * Downstream allocation is PERFECT (dy=$v0, arm-1 /32 temp $a1, arm-2 $v1,
 * disc $v1 — all target), but the carrier itself lands $a2 and evicts the
 * arg2 param to $s3 in the prologue (early `move $s3,$a2` vs target's late
 * one). Mechanism (FINDREG a2c_fr77 + ALLOCDBG): carrier(77, pri 9333)
 * allocates at ord=11, one slot BEFORE disc(104, pri 9230); find_reg pass-0
 * avoids $v1 because regs_someone_prefers[77] = disc's full prefs {3,4,7}
 * (disc CONFLICTS with the carrier: a2's real segment lives from arg2<<5
 * through the (a2-disc) use in arm-2, overlapping disc's whole range).
 * STRUCTURALLY DEAD regardless of allocation order: the a2<->disc conflict
 * means the merged carrier can never share $v1 with disc, and target needs
 * BOTH the stage lw dest and disc in $v1. Artifacts:
 * tmp/grind/func_800200DC/s3/a2_carrier_8.diff.txt, a2c.{lreg,greg},
 * a2c_fr77.txt, a2c_alloc_raw.txt. */
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 a2;
    s32 dx;
    s32 dz;
    s32 dist;

    a2 = arg1[0]; /* FAKE: stage */
    dx = a2 - arg0[0];
    dz = arg1[2] - arg0[2];
    dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy;

        a2 = arg1[1]; /* FAKE: stage */
        dy = a2 - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 disc = arg2 * arg2 + arg3 * dy2;
            s32 a0;

            if (disc >= 0) {
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
