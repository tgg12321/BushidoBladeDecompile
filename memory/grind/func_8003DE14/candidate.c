/* func_8003DE14 - candidate (grind session 11, rederive modality).
 *
 * SCORE 29 / 173 insns on HEAD 2026-09-10 (post -mel, post -msoft-float).
 * Same floor as the s10 candidate, but the s10 candidate's FAKE-family
 * `dst++; dst--;` round-trip is GONE: the +12 weighted-reference lift on the
 * dst pseudo that seats the cursors the way the target seats them is spelled
 * here as an ARM-LOCAL `dst++` in each of the three inner-loop arms, instead of
 * a shared `advance_dst:` label reached by goto from two of them.
 *
 * WHAT CHANGED vs memory/grind/func_8003DE14/candidate.c (the s10 form):
 *   - the `advance_dst:` label and both `goto advance_dst;` are deleted;
 *   - each of the two zero-pixel arms ends `*dst = pixel; src++; dst++;
 *     goto loop_check;`
 *   - the blend arm falls through to a plain `dst++;` before `loop_check:`.
 *   Three copies of one real statement, where s10 had one copy plus a
 *   two-statement net-zero round-trip.
 *
 * WHY IT REACHES THE SAME SEAT (pass attribution, ALLOCDBG read this session):
 *   flow_analysis (toplev.c:2983) counts reg_n_refs on the PRE-jump2 stream and
 *   weights each occurrence by loop_depth (flow.c:2081); the pixel loop is at
 *   depth 3, so one `dst++` is 2 occurrences x 3 = 6 weighted refs.  Replacing
 *   one shared copy with three arm-local copies is 18 instead of 6, i.e. dst's
 *   weighted reg_n_refs goes 26 -> 38 - exactly the count the s10 round-trip
 *   produced.  jump2's cross-jump (jump.c:2020, minimum-2-matching-insn tail
 *   merge) then re-merges the three tails, so the emitted stream is unchanged
 *   at 173 instructions.  Measured (BB2_ALLOC_DEBUG, tools/gcc-2.7.2/cc1,
 *   tmp/grind/func_8003DE14/s11/d_a1/stderr.log):
 *       ord=3 pseudo=109 hardreg=6 nrefs=38 livelen=60 pri=31666   (dst -> $a2)
 *       ord=4 pseudo=108 hardreg=7 nrefs=32 livelen=61 pri=26229   (src -> $a3)
 *   i.e. the TARGET's cursor seat, at the target's own instruction count.
 *
 * FAMILY: this is the duplicated-statement-into-arms shape
 * (.claude/rules/duplicated-statement-into-arms.md - "duplicating a REAL
 * statement into 2+ arms (instead of label-sharing) ... incl. when cross-jump
 * re-merges the copies to identical bytes and the effect is a reg_n_refs
 * priority lift").  A submitting session must read that rule end-to-end and
 * carry whatever annotation it mandates; it is NOT the dead-store family the
 * s10 round-trip fell into, and no net-zero / dead statement remains anywhere
 * in this body.
 *
 * NOTE: the natural-looking compression `*dst++ = pixel;` in the zero arms is
 * NOT equivalent here - it folds two insns away (171 insns, score 35) and is
 * banked as rejected/s11-star-dst-plusplus-in-zero-arms-folds-two-insns-35.c.
 * The split `*dst = pixel; ... dst++;` is load-bearing.
 *
 * WHAT IS LEFT (whole residual, unchanged from s10, sbs2.py aligned diff):
 *   target insns 0-69 and 134-172 byte-exact.  Residual is
 *     (b) j / complement seated $t5/$t4 where the target has $t4/$t5
 *         (target 70/71, 131, 133);
 *     (c) the blend arm's temp naming ($a0/$a1/$v0/$v1/$t7 vs our
 *         $v0/$v1/$a0/$a1) and the mflo/srl interleave (target 104-107 vs our
 *         109-112).
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
                            dst++;
                            goto loop_check;
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
                            dst++;
                            goto loop_check;
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
                    dst++;
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
