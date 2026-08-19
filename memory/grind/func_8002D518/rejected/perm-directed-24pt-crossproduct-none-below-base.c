/* REJECTED [s4, permuter modality] - the DIRECTED PERM cross-product chassis.
 *
 * This is the func_8002D518 region of tmp/perm_d518b/base.c, carrying three
 * PERM_GENERAL sites that between them enumerate 3 x 4 x 2 = 24 spellings:
 *   site 1 - the outer `disc < 0` arm's exit shape:
 *            `result = 0;`  |  `{ result = 0; goto done; }`  |  `return 0;`
 *   site 2 - the inner LZCS guard shape:
 *            `if (disc < 0) {lzcr=0;} else {island(ud);}`
 *            `if ((s32)ud < 0) {lzcr=0;} else {island(ud);}`
 *            `if (disc >= 0) {island(ud);} else {lzcr=0;}`
 *            `if (disc < 0) {lzcr=0;} else {island(disc);}`
 *   site 3 - the slow-path table index operand: `ud`  |  `(u32)disc`
 *
 * decomp-permuter enumerated ALL 24 combinations (campaign
 * s4-directed-guard-perm, tmp/perm_d518b) and exited. Permuter base score 35;
 * NONE of the 24 scored below 35. Six are exactly base-equivalent, eighteen are
 * strictly worse (230/235/430/545/605/740/800).
 *
 * WHY IT IS DEAD: these three axes had only ever been measured one at a time
 * (s2 and s3). The open question was whether they INTERACT - `disc`'s live
 * range, and therefore its global.c allocno priority, is a whole-block
 * property. They do not interact: the cross-product is closed, negative. Do not
 * re-open any of the three axes, individually or in combination.
 *
 * The floor-7 form itself is site1=`result = 0;`, site2=variant 1,
 * site3=`ud` - i.e. memory/grind/func_8002D518/candidate.c.
 */
s32 func_8002D518(s32 threshold, s32 r_sq, s32 *p1, s32 *p2) {
    s32 x1, z1, x2, z2;
    x1 = p1[0];
    x2 = p2[0];
    if (x1 < x2) {
        if (x2 < -threshold) return 0;
        if (threshold >= x1) goto z_check;
        return 0;
    } else {
        if (x1 < -threshold) return 0;
        if (threshold < x2) return 0;
    }
z_check:
    z1 = p1[1];
    z2 = p2[1];
    if (z1 < z2) {
        if (z2 < -threshold) return 0;
        if (threshold >= z1) goto dist_calc;
        return 0;
    } else {
        if (z1 < -threshold) return 0;
        if (threshold < z2) return 0;
    }
dist_calc:
    {
        s32 ax = p2[0] - p1[0];
        s32 x1r = p1[0];
        s32 az = p2[1] - p1[1];
        s32 z1r = p1[1];
        s32 ax_sq = ax * ax;
        s32 az_sq = az * az;
        s32 cx = ax * x1r;
        s32 cz = az * z1r;
        s32 x1_sq = x1r * x1r;
        s32 z1_sq = z1r * z1r;
        s32 dot2 = (cx + cz) * 2;
        s32 dot2_9 = dot2 >> 9;
        s32 c_val = ((x1_sq + z1_sq) - r_sq) >> 9;
        s32 dist_sq = ax_sq + az_sq;
        s32 disc = (dot2_9 * dot2_9) - ((dist_sq >> 9) * (c_val << 2));
        s32 sqrt_val;
        s32 result;
        if (disc < 0) {
            PERM_GENERAL(result = 0;, { result = 0; goto done; }, return 0;)
        } else {
            if ((u32)disc < 0x400u) {
                sqrt_val = (&D_8008D118)[disc] >> 3;
            } else {
                s32 sp_tmp;
                s32 lzcr;
                u32 ud = disc;
                PERM_GENERAL(if (disc < 0) { lzcr = 0; } else { __asm__ volatile("addu   $t4, %1, $zero\n" "mtc2   $t4, $30\n" "nop\n" "nop\n" "addu   $t4, $sp, $zero\n" "swc2   $31, 0($t4)\n" : "=m"(sp_tmp) : "r"(ud) : "$12"); lzcr = sp_tmp; }, if ((s32)ud < 0) { lzcr = 0; } else { __asm__ volatile("addu   $t4, %1, $zero\n" "mtc2   $t4, $30\n" "nop\n" "nop\n" "addu   $t4, $sp, $zero\n" "swc2   $31, 0($t4)\n" : "=m"(sp_tmp) : "r"(ud) : "$12"); lzcr = sp_tmp; }, if (disc >= 0) { __asm__ volatile("addu   $t4, %1, $zero\n" "mtc2   $t4, $30\n" "nop\n" "nop\n" "addu   $t4, $sp, $zero\n" "swc2   $31, 0($t4)\n" : "=m"(sp_tmp) : "r"(ud) : "$12"); lzcr = sp_tmp; } else { lzcr = 0; }, if (disc < 0) { lzcr = 0; } else { __asm__ volatile("addu   $t4, %1, $zero\n" "mtc2   $t4, $30\n" "nop\n" "nop\n" "addu   $t4, $sp, $zero\n" "swc2   $31, 0($t4)\n" : "=m"(sp_tmp) : "r"(disc) : "$12"); lzcr = sp_tmp; })
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tval = (&D_8008D118)[PERM_GENERAL(ud, (u32)disc) >> shift];
                    s32 half = (u32)shift >> 1;
                    sqrt_val = (u32)(tval << 16) >> (0x13 - half);
                }
            }
            {
                s32 neg_b = -dot2;
                s32 num1;
                s32 denom;
                s32 t1_val;
                s32 t2_val;
                sqrt_val <<= 9;
                num1 = (neg_b + sqrt_val) << 8;
                denom = dist_sq * 2;
                t1_val = num1 / denom;
                t2_val = ((neg_b - sqrt_val) << 8) / denom;
                result = 0;
                if (t1_val >= 0) {
                    result = t2_val < 0x101;
                }
            }
        }
done:
        return result;
    }
}
