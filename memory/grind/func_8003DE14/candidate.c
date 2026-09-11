/* func_8003DE14 - candidate (grind session 10, forensics modality).
 *
 * SCORE 29 / 173 insns on HEAD 2026-09-10 (post -mel, post -msoft-float).
 * NEW FLOOR: the previous best was 31 (and that form was 172 insns, one short).
 * This is the f1 chassis (which is the target's emitted arm topology, confirmed
 * independently by m2c in s9) PLUS one two-statement dst round-trip.
 *
 * WHAT IS DIFFERENT vs chassis_f1_structure_exact_43.c: exactly one hunk, in the
 * shared inner-loop tail --
 *       advance_dst:
 *           dst++;
 *   becomes
 *       advance_dst:
 *           dst++;
 *           dst++;
 *           dst--;
 *
 * WHY IT WORKS (pass attribution, dumps read this session, not guessed):
 *   toplev.c:2983 runs flow_analysis (which sets reg_n_refs and reg_live_length)
 *   BEFORE toplev.c:3004 combine_instructions.  In this function combine deletes
 *   19 of the 140 post-flow insns, so the allocator's reference counts are taken
 *   on a stream 19 insns LONGER than the emitted one.  The extra `dst++; dst--;`
 *   adds FOUR occurrences of the dst pseudo at inner-loop depth 3
 *   (flow.c:2081 weights every reference by loop_depth), lifting dst's weighted
 *   reg_n_refs from 26 to 38; combine then folds (dst+2)-2 back to dst and the
 *   emitted stream is unchanged at 173 instructions.
 *   allocno_compare (global.c:635) then ranks dst 38/58 -> pri 32758 ABOVE
 *   src 32/59 -> pri 27118, so dst is allocated first and find_reg's ascending
 *   scan hands it $a2; src takes $a3.  That is the TARGET's cursor seat, which
 *   three prior sessions could not reach from either direction.
 *   Measured (BB2_ALLOC_DEBUG, tools/gcc-2.7.2/cc1):
 *       ord=2 pseudo=109 hardreg=6 nrefs=38 livelen=58 pri=32758   (dst -> $a2)
 *       ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=59 pri=27118   (src -> $a3)
 *
 * !!! THIS FORM IS NOT SUBMITTABLE AS IT STANDS !!!
 *   `dst++; dst--;` is a net-zero pair of dead stores to a LOCAL.  Under the
 *   frozen family list that is the dead-store family
 *   (.claude/rules/dead-store-fake-exception.md), which REQUIRES a /* FAKE *(/)
 *   annotation carrying what + a named GCC-pass mechanism + lever-exhaustion,
 *   and the rule's own text is about a same-value RE-STORE / self-assign, not
 *   about an increment/decrement pair - a session that wants to submit this must
 *   either (a) get a ruling that the +1/-1 pair is inside that family, or
 *   (b) find an ORDINARY-C statement that puts >= 32 weighted references on dst
 *   (>= +6, i.e. >= 2 more depth-3 occurrences) without adding an emitted insn.
 *   The self-assign spelling `dst = dst;` was measured this session and does NOT
 *   work: it is deleted before flow_analysis, refs stay 26, score stays 43.
 *
 * WHAT IS LEFT (whole residual, sbs2.py aligned diff vs the target object):
 *   the cursor halves are now byte-exact - insns 0-69 and 134-172 all match.
 *   The 29 is entirely inside the blend block and is the SAME two sub-problems
 *   s7 identified:
 *     (b) the j/complement pair is seated $t5/$t4, the target has $t4/$t5;
 *     (c) the blend arm's temp naming ($a0/$a1/$v0/$v1) and the mflo/srl
 *         interleave (target does the blue channel srl/andi/mult BEFORE the
 *         first mflo; we do it after).
 *
 * Chassis: HEAD 2026-09-10.  sandbox --disable all => score 29, build_insns 173,
 * target_insns 173.
 */
void func_8003DE14(s16 *rect, s32 count) {
    u16 src_buf[0x200];
    u16 dst_buf[0x200];
    u8 color_info[0x20];
    s32 i;
    s32 saved_y;
    s32 r;
    s32 g;
    s32 b;
    s32 target_color;

    DrawSync(0);
    count--;
    StoreImage((s32 *)rect, src_buf);
    DrawSync(0);
    ((u16 *)rect)[1] -= ((u16 *)rect)[3];
    LoadImage((s32)rect, (s32)src_buf);
    saved_y = rect[1];
    rect[1] = ((u16 *)rect)[3] + saved_y;
    func_80052BE4(color_info);

    r = color_info[0];
    g = color_info[1];
    b = color_info[2];
    target_color = (((u32)r >> 3) | (s32)-0x8000) | ((g & 0xF8) << 2) | ((b & 0xF8) << 7);

    i = 0;
    if (count > 0) {
        s32 blend_base = 0x1000;
        do {
            s32 total = rect[2] * rect[3];
            u16 *src = src_buf;
            u16 *dst = dst_buf;
            s32 factor = ((i + 1) << 12) / count;
            s32 j = 0;

            if (total > 0) {
                do {
                    s32 complement = blend_base - factor;
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst = pixel;
                            src++;
                            goto advance_dst;
                        }
                        *dst++ = target_color;
                        src++;
                        goto loop_check;
                    }
                    {
                        u16 pixel = *src;
                        s32 px = pixel & 0xFFFF;
                        if (px == 0) {
                            *dst = pixel;
                            src++;
                            goto advance_dst;
                        }
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 b_src = ((u32)px >> 7) & 0xF8;
                            s32 r_ch;
                            s32 g_ch;
                            s32 b_shift;
                            src++;
                            r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
                            g_ch = ((g_src * complement + g * factor) >> 10) & 0x3E0;
                            b_shift = (b_src * complement + b * factor) >> 5;
                            *dst = (pixel & 0x8000) | r_ch | g_ch | (b_shift & 0x7C00);
                        }
                    }
                advance_dst:
                    dst++;
                    dst++;
                    dst--;
                loop_check:
                    j++;
                } while (j < rect[2] * rect[3]);
            }

            {
                s32 new_y = ((u16 *)rect)[1] + ((u16 *)rect)[3];
                ((u16 *)rect)[1] = new_y;
                if ((s16)new_y >= 0x200) {
                    rect[1] = saved_y;
                    ((u16 *)rect)[0] += ((u16 *)rect)[2];
                }
            }
            LoadImage((s32)rect, (s32)dst_buf);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
