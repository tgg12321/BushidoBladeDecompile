/* func_8003DE14 - chassis q1, banked by grind session 8 (forensics).
 * Score 37 / 172 insns.  ORDINARY C THROUGHOUT (no /* FAKE *\/ construct, no
 * sanctioned-family claim needed): it is k1 with the two zero-pixel arms
 * routed through a shared `advance_src: src++;` tail and the blend arm given
 * an explicit `goto advance_dst;` so every path still advances src exactly
 * once.
 *
 * WHY IT IS BANKED (it scores 6 WORSE than candidate.c's 31 - score is not the
 * read-out here):  q1 is the first chassis in this grind that reaches the
 * TARGET CURSOR SEAT (src -> $a3, dst -> $a2) in ordinary C with NO extra dst
 * reference, i.e. without k8's duplicated `*dst++` statement.
 *
 *   ALLOCDBG (tools/ra_solver/extract.py):
 *     k1: src refs=32 live=59 pri=27118 -> $a2 | dst refs=26 live=58 -> $a3
 *     q1: src refs=26 live=58 pri=17931 -> $a3 | dst refs=26 live=57 -> $a2
 *
 * The mechanism is flow.c:2081 `reg_n_refs[regno] += loop_depth`: every
 * reference inside the per-pixel loop is weighted by depth 3, so ONE `src++`
 * statement is worth 6 weighted refs (2 RTL refs x 3).  src = (2 reads +
 * 4 incs) * 3 + 2 = 32 on k1; dropping to THREE pre-RA `src++` statements gives
 * 26, which is exactly the allocno_compare (global.c:643) threshold measured by
 * s7's cursor_thresh.py.
 *
 * REMAINING RESIDUAL (see s8 in hypotheses.md):
 *   (a) 172 vs the target's 173 instructions - the target's four `addiu $a3`
 *       are four separate pre-RA insns, and this chassis only has three;
 *   (b) the j/complement $t4/$t5 pair, unchanged from k1 (j live 56 /
 *       complement 51 here);
 *   (c) blend-block emission order + the v0/v1/a0/a1 rotation that rides on
 *       (b).
 *
 * Chassis: HEAD 2026-09-10 (post -mel / -msoft-float), measured with
 * `sandbox func_8003DE14 --disable all`.
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
                            goto advance_src;
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
                            goto advance_src;
                        }
                        src++;
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 b_src = ((u32)px >> 7) & 0xF8;
                            s32 r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
                            s32 g_ch = ((g_src * complement + g * factor) >> 10) & 0x3E0;
                            s32 b_shift = (b_src * complement + b * factor) >> 5;
                            *dst = (pixel & 0x8000) | r_ch | g_ch | (b_shift & 0x7C00);
                        }
                        goto advance_dst;
                    }
                advance_src:
                    src++;
                advance_dst:
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
