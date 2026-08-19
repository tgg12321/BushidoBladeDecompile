/* s5 REJECTED - score 10 (WORSE than the 7 base), build_insns 144 == target 144.
 * Change: the discriminant's second product spelled `(c_val << 2) * (dist_sq >> 9)`
 * instead of `(dist_sq >> 9) * (c_val << 2)` - i.e. the two multiply operands
 * swapped (multiplication is commutative; semantically identical).
 * RATIONALE THAT MOTIVATED IT (now measured and killed): global.c set_preference()
 * takes XEXP(SET_SRC, 0) for a non-copy SET, so `(set (reg 121) (mult (reg 119)
 * (reg 120)))` gives allocno 121 a preference for whatever hard reg reg 119 got
 * from local-alloc - which is $3 (`;; Register 119 in 3.`). expand_preferences()
 * then merges 121's preference into 117 (disc) at the subu, because the subu
 * carries REG_DEAD reg 121 and 117/121 do not conflict. Swapping the operands was
 * meant to make reg 120 ($2, which 117 already conflicts with -> pruned) the
 * preference source, leaving 117 preference-free.
 * WHY IT DOES NOT HELP (the real lesson, do not re-derive): mips.h defines no
 * REG_ALLOC_ORDER, so find_reg's fallback scan is ascending hard-reg number.
 * With $2 hard-conflicted, the first free reg is $3 EITHER WAY - the preference
 * and the default scan agree. Removing the preference cannot move disc off $3;
 * only CONFLICTS can. (The measured 10 comes from the operand swap also flipping
 * which of the two sra/sll feeders is emitted first.)
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

        s32 disc = (dot2_9 * dot2_9) - ((c_val << 2) * (dist_sq >> 9));
        s32 sqrt_val;
        s32 result;

        if (disc < 0) {
            result = 0;
        } else {
            if ((u32)disc < 0x400u) {
                sqrt_val = (&D_8008D118)[disc] >> 3;
            } else {
                s32 sp_tmp;
                s32 lzcr;
                u32 ud = disc;
                if (disc < 0) {
                    lzcr = 0;
                } else {
                    /* Hand-written GTE leading-zero-count block (LZCS in,
                     * LZCR out) — same island as authorized siblings
                     * func_8001A67C / func_800274BC. */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"
                        "nop\n"
                        "nop\n"
                        "addu   $t4, $sp, $zero\n"
                        "swc2   $31, 0($t4)\n"
                        : "=m"(sp_tmp)
                        : "r"(ud)
                        : "$12");
                    lzcr = sp_tmp;
                }
                {
                    s32 shift = 0x16 - (lzcr & ~1);
                    s32 tval = (&D_8008D118)[ud >> shift];
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
        return result;
    }
}
