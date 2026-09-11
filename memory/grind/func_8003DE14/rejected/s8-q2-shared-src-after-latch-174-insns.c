/* func_8003DE14 - chassis k1 (grind s6, `synthesis` modality).  Score 43 /
 * build_insns 173 (target 173).  ORDINARY C throughout - no FAKE construct.
 *
 * k1 = h1 with the blend block re-named to the TARGET's own shape, read off
 * asm/funcs/func_8003DE14.s 8003DF8C-8003E020: r_ch and g_ch are named locals
 * that carry their masks, the blue channel is a named local carrying only the
 * shift, and its 0x7C00 mask is written inline in the store.
 *
 * WHY THIS CHASSIS MATTERS EVEN THOUGH IT SCORES ONE WORSE THAN h1 (42):
 * it is the only 173-insn chassis whose src/dst seat is the CLASSIC SWAP.
 * On h1 the seat is src -> $a1, dst -> $a2 (dst already correct, src two
 * registers low).  The extra named blend intermediate creates one more
 * short-lived high-priority allocno, which takes $a1 and pushes the pair to
 * src -> $a2 / dst -> $a3 - one exclusion away from the target's
 * src -> $a3 / dst -> $a2.  ra_solver find_reg trace on this chassis:
 *   108 (src) pri 27118 hard_conf=[2,3,4,5,29] -> best 6 ($a2)
 *   109 (dst) pri 17931 hard_conf=[2,3,4,5,6,29] -> best 7 ($a3)
 * so the ONLY thing missing is that dst outrank src (see hypotheses [s6]).
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
                    }
                advance_dst:
                    dst++;
                loop_check:
                    j++;
                    goto latch;
                advance_src:
                    src++;
                    goto advance_dst;
                latch:
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
