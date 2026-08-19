/* func_8002D518 - best form as of s7 (forensics). Honest floor 4,
 * build_insns 144 == target_insns 144. This exact text is IN PLACE in
 * src/code6cac_b.c at end of s7 (re-measured: score 4).
 *
 * CHASSIS WARNING (now confirmed a SEVENTH time): src/code6cac_b.c at s7
 * dispatch did NOT carry the s6 candidate - HEAD held a hybrid `sq` +
 * `[(u32)disc >> shift]` form. ALWAYS re-apply this file and re-measure.
 * tmp/grind/func_8002D518/s7/apply.py splices a form file into src/ by brace
 * matching; tmp/grind/func_8002D518/s7/align3.py prints a normalised
 * (reg-name / pseudo-mnemonic / nop / reloc insensitive) target-vs-build diff.
 *
 * WHAT s7 CHANGED (floor 7 -> 4, insn parity held at 144 == 144). TWO edits,
 * both ordinary C, both in already-sanctioned families:
 *
 *   1. VARIABLE REUSE: `sqrt_val` was deleted as a separate local and `disc`
 *      carries the square-root value too (`disc = table[disc] >> 3;`,
 *      `disc = (u32)(tval << 16) >> (0x13 - half);`, `disc <<= 9;`,
 *      `num1 = (neg_b + disc) << 8;`, `t2_val = ((neg_b - disc) << 8) / denom`).
 *      This is exactly what target's register file shows: $a2 holds the
 *      discriminant AND the square root AND the <<9 result. It is the same
 *      lever s3 used for `sqrt_val <<= 9` in place, applied one level up.
 *      EFFECT (the s5/E4 mechanism, corrected - see evidence E12): merging the
 *      two variables into one pseudo makes allocno 117 live across the whole
 *      lzcs/table region, which picks up HARD-REG conflicts from the
 *      local_alloc'd block-locals living there:
 *        base  `;; 117 conflicts: 108 116 117 2 29`
 *        s7    `;; 117 conflicts: 108 116 117 122 131 2 3 4 5 12 29 64 65 66`
 *      With hard regs 2,3,4,5 all blocked, global.c find_reg has no choice but
 *      $6 = $a2 = TARGET'S REGISTER, and 117 is still allocno position 1 so
 *      nothing else in the order has to move. All six `disc` slots close.
 *
 *   2. The inner LZCS guard is now the INIT-THEN-CONDITIONALLY-OVERWRITE shape
 *      (`lzcr = 0; if (disc >= 0) { island; lzcr = sp_tmp; }`), i.e. literally
 *      what target's `bltz $a2,.L8002D6BC` + fall-through island reads like.
 *      s2/s3 had rejected this shape because it deletes the `u32 ud = disc;`
 *      copy - and it still does (see the residual below) - but on the s7
 *      chassis it is worth 2 slots net: score 6/146 with the if/else guard,
 *      score 4/144 with the init guard.
 *
 * THE REMAINING RESIDUAL IS 4 SLOTS = TWO INDEPENDENT ITEMS:
 *   (a) 3 slots - the `u32 ud = disc;` copy. Target keeps TWO registers
 *       (`addu $a0,$a2,$zero` at 0x8002D680, then the island reads $a0 and the
 *       slow-path `srlv` reads $a0); we fold to one, so we are 1 insn short and
 *       the island's `addu $t4,%1,$zero` + the `srlv` read $6 instead of $4.
 *       s6/E7 + s7 re-confirmation: with a plain C copy cse's make_regs_eqv is
 *       strictly either/or, so this is NOT a C-level `u32 ud = disc;`.
 *       Hoisting the copy above the 0x400 test on the s7 chassis is
 *       score-NEUTRAL (4/144) - measured, banked as rejected.
 *   (b) 1 slot - slot 88 (`addu $v0,$zero,$zero` + jump to the epilogue label
 *       vs our `addu $a1,$zero,$zero` + jump to the join). Spelling the arm
 *       `return 0;` still loses 2 insns to jump2 cross-jumping on the s7
 *       chassis: score 8 / 142. Unchanged verdict from s6/E10-E11: this is a
 *       reorg.c delay-slot re-materialisation question, read .dbr for it.
 *
 * INHERITED AND STILL LOAD-BEARING (do not undo): the island is the
 * func_800274BC-accepted canonical form (single __asm__ volatile, "=m"(sp_tmp),
 * "r"(ud), "$12" clobber); s3's named numerator `num1` assigned BEFORE `denom`;
 * the outer `if (disc < 0) { result = 0; } else { ... }` join shape.
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
        s32 result;

        if (disc < 0) {
            result = 0;
        } else {
            if ((u32)disc < 0x400u) {
                disc = (&D_8008D118)[disc] >> 3;
            } else {
                s32 sp_tmp;
                s32 lzcr;
                u32 ud = disc;
                lzcr = 0;
                if (disc >= 0) {
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
                    disc = (u32)(tval << 16) >> (0x13 - half);
                }
            }

            {
                s32 neg_b = -dot2;
                s32 num1;
                s32 denom;
                s32 t1_val;
                s32 t2_val;
                disc <<= 9;
                num1 = (neg_b + disc) << 8;
                denom = dist_sq * 2;
                t1_val = num1 / denom;
                t2_val = ((neg_b - disc) << 8) / denom;

                result = 0;
                if (t1_val >= 0) {
                    result = t2_val < 0x101;
                }
            }
        }
        return result;
    }
}
