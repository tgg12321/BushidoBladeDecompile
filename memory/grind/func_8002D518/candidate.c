/* func_8002D518 — best form as of s2 (structural). Honest floor 30,
 * build_insns 144 == target_insns 144. This exact text is IN PLACE in
 * src/code6cac_b.c at end of s2. Future sessions: s1's edits were NOT in src at
 * s2 dispatch (HEAD carried the old .word/register-pin island) — ALWAYS verify
 * src actually carries this text before trusting the ledger floor, and re-apply
 * from here if it does not.
 *
 * Load-bearing s2 finding (the frontier head from s1 is RESOLVED):
 *   The LZCS guard is spelled as a real if/else
 *     `if (disc < 0) { lzcr = 0; } else { <island>; lzcr = sp_tmp; }`
 *   rather than s1's "initialise lzcr = 0, then conditionally overwrite".
 *   In the plain-if shape GCC's *combine* pass deletes the `u32 ud = disc;`
 *   copy (combine's blocks are CODE_LABEL-bounded, and in the plain-if shape
 *   the copy and both of its uses sit in one label-to-label region). The
 *   if/else shape puts a CODE_LABEL between the copy and the island, combine
 *   can no longer propagate, and the copy SURVIVES — reorg then places it in
 *   the beqz delay slot exactly as target does:
 *       build idx 90  `move $4,$3`   ==  target 8002D680 `addu $a0,$a2,$zero`
 *   The copy lands in $a0, target's own register, and feeds BOTH the island
 *   input (idx 98) and the `srlv` (idx 109) exactly as target does. Cost: ZERO
 *   extra instructions (still 144). This is the free if/else escape documented
 *   in .claude/rules/cse-block-extension-controls-fold-span.md ("Spell the
 *   conditional as a real if/else rather than initialise-to-a-default, then
 *   conditionally overwrite ... ordinary C with a semantic reading"), not a
 *   coercion.
 *
 *   The island itself is the func_800274BC-accepted canonical form (single
 *   __asm__ volatile, "=m"(sp_tmp), "r"(ud), "$12" clobber); the sandbox does
 *   NOT strip this form, which is what restores insn parity 144 == 144.
 *   Function is a member of the 2026-08-17 cop2-addressing-preamble cluster
 *   ruling (.claude/rules/cop2-addressing-preamble-cluster.md:74, LZCS
 *   sub-family, 1 site) — at sandbox 0 the island takes the driver-executed
 *   authorization path; do NOT re-escalate.
 *
 * Remaining residual at floor 30 (see evidence.md [s2] for the full 154-line
 * alignment): indices 0-74 byte-identical incl. registers; the divergence head
 * is now (a) `disc` allocated to $3 where target uses $a2/$6, and (b) the
 * sched2 mult-shadow ordering flip at 75-78 (sched2 priorities 197=0x44 vs
 * 200=0x18; sched1 had them tied at 13).
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
                s32 lzcr;
                u32 ud = disc;
                if (disc < 0) {
                    lzcr = 0;
                } else {
                    /* Hand-written GTE leading-zero-count block (LZCS in,
                     * LZCR out) — same island as authorized siblings
                     * func_8001A67C / func_800274BC. */
                    __asm__ volatile(
                        "addu   $t4, %1, $zero
"
                        "mtc2   $t4, $30
"
                        "nop
"
                        "nop
"
                        "addu   $t4, $sp, $zero
"
                        "swc2   $31, 0($t4)
"
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
