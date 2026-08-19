/* func_8002D518 - MATCHED FORM (s8, rederive). Honest sandbox distance 0
 * with all 33 regfix/asmfix rules dropped and cheat-asm stripped;
 * build_insns 144 == target_insns 144.
 *
 * s8 inherited the s7 form (floor 4) and closed the last two residual items
 * with two ordinary-C edits. The full derivation is in evidence.md (E14-E17).
 *
 *  EDIT 1 - the `ud` copy (3 slots, the frontier head since s1).
 *    Target keeps TWO registers for the discriminant across the LZCS island:
 *    $a2 (read by `bltz $a2,.L8002D6BC`) and $a0 (`addu $a0,$a2,$zero` at
 *    0x8002D680, read by the island and by the slow-path `srlv`). Seven
 *    sessions measured every plain-C placement of `u32 ud = disc;` folding
 *    away. MEASURED MECHANISM (tmp/grind/func_8002D518/s8/dumps_*): with a
 *    SINGLE assignment, cse.c's make_regs_eqv puts ud and disc in one quantity
 *    and rewrites every read to disc, then DELETES the copy insn - the .cse
 *    dump of the single-assignment control (v6_nodup) has no copy at all and
 *    the block runs code_label 240 -> insn 248 directly. Writing the
 *    assignment a SECOND time inside the `if (disc >= 0)` guard arm makes the
 *    pseudo multiply-defined across the join: the equivalence is invalidated,
 *    the post-join `ud >> shift` read keeps its own pseudo, and BOTH copies
 *    survive cse as `(set (reg 131) (reg 117))`. global_alloc then lands
 *    117 -> $a2 and 131 -> $a0 (target's pair) and the redundant second copy
 *    is dropped before final output, so insn parity holds at 144. This is the
 *    sanctioned duplicated-statement-into-arms family
 *    (.claude/rules/duplicated-statement-into-arms.md) and carries the
 *    required /* FAKE *\/ annotation at the duplicated copy.
 *
 *  EDIT 2 - slot 74 (1 slot). Target's `disc < 0` arm writes the RETURN
 *    register directly (`j .L8002D774` + `addu $v0,$zero,$zero` in the delay
 *    slot) and jumps PAST the join's `addu $v0,$a1,$zero`, whereas ours wrote
 *    $a1 and jumped INTO the join. Spelling the arm `return 0;` does not work
 *    (measured twice, s7 and s8/v5: jump.c cross-jumps the block into an
 *    earlier return-0 site and then inverts `bgez`->`bltz`, losing 2 insns,
 *    score 5/142). The fix is that `result` and the comparison flag are two
 *    DIFFERENT locals: the flag is computed in $a1 and copied into `result`
 *    ($v0) at the join, so the `disc < 0` arm's `result = 0` is already a
 *    write of $v0 and its `j` targets the epilogue. Ordinary named-
 *    intermediate C, no annotation needed.
 *
 * ALSO LOAD-BEARING (inherited from s1-s7, do not undo):
 *   - the `ud = disc; lzcr = 0;` ORDER (s8/v3 had them reversed: the two
 *     delay slots come out swapped, score 3);
 *   - s7's variable reuse - `disc` carries the discriminant, the square root
 *     and the <<9 result (target's $a2 does exactly that);
 *   - s3's named numerator `num1` assigned BEFORE `denom`;
 *   - the canonical GTE LZCS island in the func_800274BC-accepted form
 *     (single __asm__ volatile, "=m"(sp_tmp), "r"(ud), "$12" clobber);
 *     cluster .claude/rules/cop2-addressing-preamble-cluster.md:74;
 *   - the outer `if (disc < 0) { result = 0; } else { ... }` join shape.
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
                u32 ud;
                ud = disc;
                lzcr = 0;
                if (disc >= 0) {
                    /* FAKE: duplicated `ud = disc;` into the LZCS-guard arm,
                     * mechanism: cse.c make_regs_eqv - the second def makes
                     * pseudo `ud` multiply-defined, so the equivalence
                     * ud == disc established at the first copy is invalidated
                     * and cse can no longer rewrite the post-join `ud >> shift`
                     * read to disc's register; without it cse DELETES the copy
                     * outright (measured: v6_nodup, score 3, no `addu $a0,$a2`).
                     * lever-exhaustion: memory/grind/func_8002D518/hypotheses.md
                     * (s1-s7: 22 rejected forms, 33,881 permuter iterations,
                     * every plain-C copy placement measured folding). */
                    ud = disc;
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

                {
                    s32 flag = 0;
                    if (t1_val >= 0) {
                        flag = t2_val < 0x101;
                    }
                    result = flag;
                }
            }
        }
        return result;
    }
}
