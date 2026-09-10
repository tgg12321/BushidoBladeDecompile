/* func_8003DE14 - chassis h1 (grind s5, `enumerate` modality).
 * Score 42 / build_insns 173 (target 173).  This SUPERSEDES
 * chassis_f1_structure_exact_43.c: same structurally-exact inner loop (the
 * zero-pixel arm routes through the shared `advance_dst` tail, the colour arm
 * inlines dst++), one point better, found by exhaustive spelling enumeration
 * of the blend block.
 *
 * The only difference from f1 is the blend block's naming: r_src / g_src /
 * b_src / r_ch stay NAMED locals and the green and blue channel expressions
 * are written INLINE inside the final store (f1 named g_ch and b_shift and
 * assigned them separately).  `src++` also moved one statement earlier, out
 * of the innermost brace - measured byte-neutral (f1, f1_srcup and f1_srcdown
 * all score 43 / 173).
 *
 * ORDINARY C THROUGHOUT: no FAKE construct, no borrowed local, no
 * sanctioned-family claim needed.
 *
 * Residual 42: still the src/dst register seat (pseudo 108 -> $a2 in the
 * build, target wants $a3) plus the j/complement pair, both downstream of it.
 * s5 swept 4,488 spellings across four regions with ZERO hit - see
 * hypotheses.md [s5].
 *
 * Chassis: HEAD 2026-09-10 (post -mel / -msoft-float), sandbox --disable all.
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
                        src++;
                        {
                            s32 r_src = (pixel & 0x1F) << 3;
                            s32 g_src = ((u32)px >> 2) & 0xF8;
                            s32 b_src = ((u32)px >> 7) & 0xF8;
                            s32 r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
                            s32 g_ch = ((g_src * complement + g * factor) >> 10) & 0x3E0;
                            *dst = (pixel & 0x8000) | r_ch | g_ch | (((b_src * complement + b * factor) >> 5) & 0x7C00);
                        }
                    }
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
