/* func_8002D518 — best form as of s1 (recon). Honest floor 30 (from 33 at
 * dispatch), build_insns 144 == target_insns 144. This exact text is IN PLACE
 * in src/code6cac_b.c at end of s1 — future sessions: verify src still carries
 * it before re-applying.
 *
 * Load-bearing s1 findings (details in evidence.md):
 *  - The LZCS/LZCR island is spelled in the func_800274BC-accepted canonical
 *    form (single __asm__ volatile, "=m"(sp_tmp), "r"(ud), "$12" clobber).
 *    In this form the sandbox does NOT strip it and the island's 7 insns
 *    byte-match target (addu $t4 encodings verified word-for-word).
 *    Function is a member of the 2026-08-17 cop2-addressing-preamble cluster
 *    ruling (.claude/rules/cop2-addressing-preamble-cluster.md:74, LZCS
 *    sub-family, 1 site) — at sandbox 0 the island takes the driver-executed
 *    authorization path; do NOT re-escalate.
 *  - `u32 ud = disc;` reproduces target's intent (target has a second register
 *    $a0 = copy of disc feeding the island + srlv) but GCC folds the copy
 *    (uses coalesce back to disc's register). Keeping the copy ALIVE is
 *    frontier item 1.
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
            result = 0;
        } else {
            if ((u32)disc < 0x400u) {
                sqrt_val = (&D_8008D118)[disc] >> 3;
            } else {
                s32 sp_tmp;
                s32 lzcr = 0;
                u32 ud = disc;
                if (disc >= 0) {
                    /* Hand-written GTE leading-zero-count block (LZCS in,
                     * LZCR out) — same island as authorized siblings
                     * func_8001A67C / func_800274BC: $t4 reused back-to-back
                     * for two unrelated values, 2 unfilled GTE delay nops,
                     * splat tags the cop2 ops "handwritten instruction". */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero\n"
                        "mtc2   $t4, $30\n"        /* LZCS <- disc */
                        "nop\n"
                        "nop\n"
                        "addu   $t4, $sp, $zero\n" /* &sp_tmp (at 0($sp)) */
                        "swc2   $31, 0($t4)\n"     /* sp_tmp <- LZCR */
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
                s32 sq = sqrt_val << 9;
                s32 neg_b = -dot2;
                s32 denom = dist_sq * 2;
                s32 t1_val = ((neg_b + sq) << 8) / denom;
                s32 t2_val = ((neg_b - sq) << 8) / denom;

                result = 0;
                if (t1_val >= 0) {
                    result = t2_val < 0x101;
                }
            }
        }
        return result;
    }
}
