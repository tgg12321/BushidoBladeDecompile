/* [s5 ADDENDUM - forensics modality, floor UNCHANGED at 7]
 * Re-applied to src/code6cac_b.c at s5 dispatch (HEAD did NOT carry it, for the
 * FIFTH consecutive session) and re-measured: score 7, build_insns 144 ==
 * target_insns 144. This file is still the best known form.
 *
 * s5 was pure pass-attribution forensics on the 6-slot `disc` residual. Result:
 * the s1-s4 frontier head ("get pseudo 117 / `disc` into $a2/$6 by lowering its
 * allocno priority") is KILLED IN CLOSED FORM. global.c find_reg blocks a hard
 * reg only via 117's own hard-reg conflicts or via a reg already held by a
 * CONFLICTING allocno; the only $5-holding allocno that conflicts with 117 is
 * 116 (`dist_sq`) at position 22 of 24 with priority 909, so reaching $6 by
 * order alone would need 117's live_length > 132 insns in a 144-insn function.
 * Two source probes were measured and banked as rejected:
 *   - multiply-operand swap (kills the $3 preference): score 10, WORSE.
 *   - `[(u32)disc >> shift]` instead of `[ud >> shift]`: score 7 (tie) but it
 *     makes cse merge allocno 132 (`ud`) away entirely, deleting the only
 *     allocno that can ever block $4 for disc. Strictly worse platform.
 * Also newly attributed: the pass that makes the inner LZCS guard read `ud`
 * instead of `disc` is cse.c's make_regs_eqv/qty_first_reg canonicalisation
 * (.rtl has `ge:SI (reg 117)`, .cse has `ge:SI (reg 132)`), NOT combine.
 * Full derivation incl. the validated allocno-priority table: evidence.md E1-E6.
 */
/* [s4 ADDENDUM - permuter modality, floor UNCHANGED at 7]
 * This form was re-applied to src/code6cac_b.c at s4 dispatch (HEAD did NOT
 * carry it, for the fourth consecutive session) and re-measured: score 7,
 * build_insns 144 == target_insns 144.
 * s4 then proved it is a STRICT LOCAL MINIMUM for decomp-permuter:
 *   - 33,881 random iterations on a validated full-TU workspace: ZERO finds,
 *     best-ever score == base (permuter metric 35). Neighbourhood is uphill.
 *   - an exhaustive 24-point directed PERM cross-product over {outer disc<0
 *     exit shape} x {inner LZCS guard shape} x {slow-path index operand}:
 *     none below base; six base-equivalent, eighteen worse.
 * So do NOT spend another session sampling this chassis. The residual 7 needs
 * the allocation mechanism attacked directly (global.c allocno priority for
 * pseudo 117), i.e. forensics, not search.
 */
/* func_8002D518 - best form as of s3 (structural). Honest floor 7,
 * build_insns 144 == target_insns 144. This exact text is IN PLACE in
 * src/code6cac_b.c at end of s3.
 *
 * CHASSIS WARNING (confirmed a THIRD time at s3 dispatch): src/code6cac_b.c did
 * NOT carry the previous session's candidate - HEAD still had the pre-s1
 * `register s32 t4_v asm("t4")` + `.word` island and measured floor 33. The
 * Grinder commits the ledger, not src/. ALWAYS re-apply this file to
 * src/code6cac_b.c and re-measure before trusting any ledger floor.
 * (Also: memory/grind/func_8002D518/candidate.c had been written with REAL
 * newlines inside the __asm__ string literals instead of \n escapes - i.e. it
 * was not compilable C as stored. s3 repaired it. Check that before splicing.)
 *
 * WHAT s3 CHANGED (floor 30 -> 7, at unchanged insn parity 144 == 144). Two
 * independent tail-shape edits, both ordinary C:
 *   1. `sqrt_val <<= 9;` IN PLACE instead of `s32 sq = sqrt_val << 9;`
 *      Target does `sll $a2,$a2,9` - the shifted value occupies sqrt_val's own
 *      register. Reusing the variable reproduces that; a fresh `sq` local does
 *      not (it lands in $a1/$5 and drags the whole divide chain with it).
 *   2. The first quotient's numerator is named (`num1 = (neg_b + sqrt_val) << 8;`
 *      assigned BEFORE `denom = dist_sq * 2;`). Target computes the denominator
 *      LAST before the div (`addu $a0,$v0,$a2 / sll $a0,8 / sll $v1,$a1,1`);
 *      naming the numerator and assigning it first puts the denom sll last.
 *   Measured separately: edit 1 alone = 21, edit 2 alone = 19, both = 7.
 *   These two between them fixed the ENTIRE downstream cascade the s1 residual
 *   map listed as items 2 and 3: dist_sq now allocates $a1 (was $6), sqrt_val
 *   $a2 (was $2), result $a1 (was $6), t1/t2 and both div scaffolds match, and
 *   the tail-twin ordering flip at 118-122 is GONE. The s2 attribution that
 *   said those were unreachable from source was correct about *statement
 *   reordering*; it was the VARIABLE (one fewer live pseudo in the divide
 *   chain) that moved them, not the order.
 *
 * INHERITED AND STILL LOAD-BEARING (do not undo):
 *   - The LZCS guard is a real if/else (`if (disc < 0) { lzcr = 0; } else
 *     { island; lzcr = sp_tmp; }`). In the "init lzcr = 0 then conditionally
 *     overwrite" shape - which is what the target's delay-slot layout literally
 *     looks like - GCC's *combine* deletes the `u32 ud = disc;` copy (combine's
 *     blocks are CODE_LABEL-bounded). s3 re-measured that literal target shape
 *     (v8/v9/v10) and the copy re-folds every time. The if/else is the free
 *     escape from .claude/rules/cse-block-extension-controls-fold-span.md.
 *   - The island is the func_800274BC-accepted canonical form (single
 *     __asm__ volatile, "=m"(sp_tmp), "r"(ud), "$12" clobber). The sandbox does
 *     not strip this form, which is what holds insn parity at 144.
 *     Cluster: .claude/rules/cop2-addressing-preamble-cluster.md:74 (LZCS
 *     sub-family, 1 site) - at sandbox 0 the island takes the driver-executed
 *     authorization path; do NOT re-escalate.
 *
 * THE ENTIRE REMAINING RESIDUAL IS 7 SLOTS, and it is TWO items:
 *   (a) 6 slots: `disc` is allocated $v1/$3 where target uses $a2/$6
 *       (slots 84 subu, 85 bgez, 86 sltiu, 90 the copy's source, 92 the table
 *       index addu, 96 the bltz). Nothing else about those instructions
 *       differs - same opcodes, same operand roles, same order.
 *   (b) 1 slot: slot 88. Target emits `addu $v0,$zero,$zero` and jumps to the
 *       epilogue LABEL (.L8002D774, past `addu $v0,$a1,$zero`) - i.e. the
 *       disc < 0 arm is a real `return 0;`. We emit `move $5,$0` and jump to
 *       the join. Spelling it `if (disc < 0) return 0;` produces exactly the
 *       target shape but jump2 CROSS-JUMPS the resulting 2-insn block into the
 *       entrance return-0 block at 1930, dropping to 142 insns (score 10).
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
