/* REJECTED - [s3] Spelling the disc<0 arm as `return 0;` (target sets $v0 and jumps past the result move) is CROSS-JUMPED by jump2 into the entrance return-0 block, deleting the 2-insn block: 142 insns vs target 144, score 10 (v21 base 7). KILLED unless the cross-jump can be blocked. */
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

        if (disc < 0) return 0;
        {
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
