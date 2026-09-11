/* s10 REJECTED: `dst = dst;` in the inner loop is deleted before flow_analysis
 * (cse/delete_trivially_dead_insns), so reg_n_refs for dst stays 26 and the
 * allocation is bit-identical to f1: score 43 / 173.  The working spelling is
 * an increment/decrement pair, which survives to flow and is folded by combine
 * (see candidate.c).  Measured HEAD 2026-09-10, f1 chassis, no other change. */
/* func_8003DE14 - THE f1 CHASSIS (grind session 4, permuter modality).
 *
 * Honest score 43 (WORSE than candidate.c's 31) but build_insns == 173 == the
 * TARGET's instruction count, and the arm structure of the inner loop is now
 * BYTE-STRUCTURALLY IDENTICAL to 8003DE14+0x12c..0x214.  This file is banked
 * because it, not candidate.c, is the chassis the next session should work.
 *
 * WHAT CHANGED vs candidate.c (one hunk):
 *   the zero-pixel arm of the `i == count - 1` branch goes from
 *       *dst++ = pixel; src++; goto loop_check;
 *   to
 *       *dst   = pixel; src++; goto advance_dst;
 *
 * WHY (read from the target's own disassembly, s4):
 *   target 0x210 is `advance_dst:` (addiu a2,a2,2) and 0x214 is `loop_check:`.
 *     0x13c  i==count-1 zero-pixel arm : sh v0,0(a2) ; j 0x210 ; addiu a3,a3,2
 *     0x148  i==count-1 colour arm     : sh s6,0(a2) ; addiu a2,a2,2 ;
 *                                        j 0x214 ; addiu a3,a3,2
 *     0x16c  blend zero-pixel arm      : sh t0,0(a2) ; j 0x210 ; addiu a3,a3,2
 *     0x20c  blend main path           : falls through into 0x210
 *   So the arm that inlines its own `dst++` is the TARGET-COLOUR arm, and the
 *   zero-pixel arm routes through the shared tail.  s3's d4 sweep put the
 *   inline increment on the WRONG arm: with both arms inlining it, jump2's
 *   cross-jumping merged the two identical `sh / addiu a2 / j / addiu a3`
 *   tails and ate the 173rd instruction (candidate.c is 172).  f1 puts it on
 *   the arm the target uses and the instruction count becomes exact.
 *
 * WHAT IS LEFT ON THIS CHASSIS (whole residual, from the s4 side-by-side):
 *   (a) src/dst are swapped: we get 108(src)->$a2, 109(dst)->$a3; the target
 *       has 108->$a3, 109->$a2.  This is the entire 43-vs-31 delta and it also
 *       drags the v0/v1/a0/a1 naming in the blend arm and the loop-bottom
 *       `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1` reload.
 *   (b) the j/complement pair is still $t5/$t4 instead of $t4/$t5.
 *   (c) the blend arm's mflo/srl interleave (unchanged since s3, H-s3-4).
 *
 * ra_solver on THIS chassis (tmp/ra_solver_work/func_8003DE14.model.json,
 * dispositions 25/25, sort order MATCH):
 *   108 refs 32 livelen ~59 priority 27118 -> $a2 ;  109 refs 26 livelen ~58
 *   priority 17931 -> $a3.  The gap is a floor_log2 threshold effect:
 *   floor_log2(32) = 5 but floor_log2(26) = 4.  inverse.py global --goal
 *   '{"108": 7, "109": 6}' returns minimal solution size 1 with EIGHT vectors,
 *   all of them either `refs_down pseudo 108: 32 -> <=26` or
 *   `refs_up pseudo 109: 26 -> >=32`.  No live-length or birth-order vector
 *   exists at size 1 - the swap is purely a reference-count problem.
 *
 * Chassis: HEAD 2026-09-10 (post -mel, post -msoft-float).  No FAKE construct,
 * no sanctioned-family claim: ordinary C throughout.
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
                    dst = dst;
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
