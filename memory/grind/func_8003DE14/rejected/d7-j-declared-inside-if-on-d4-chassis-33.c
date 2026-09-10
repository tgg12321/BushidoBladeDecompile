/* func_8003DE14 - best form as of grind session 2 (structural), honest floor 52/173.
 *
 * ORDINARY C THROUGHOUT. No /* FAKE *\/ construct, no borrowed local, no
 * sanctioned-family claim needed. s1's `total = count - 1;` borrow is GONE:
 * see hypotheses.md C1 for the loop.c:1631 threshold mechanism that replaced it.
 *
 * Two structural changes vs s1's 57-scoring candidate:
 *   1. `s32 complement = blend_base - factor;` is DECLARED INSIDE the inner
 *      do-body (it was outside, in the `if (total > 0)` prologue). It is still
 *      loop-invariant, so loop.c hoists it straight back to the inner-loop
 *      preheader - exactly where the target has `subu $t5,$fp,$t3` (8003DF30).
 *      The point is that it is now the FIRST movable in the inner loop's
 *      movables list, so move_movables spends the first move on it and applies
 *      `threshold -= 3` (loop.c:1904). The next movable - the `count - 1`
 *      comparand at insn 147 - then fails `threshold * savings * lifetime >=
 *      insn_count` (55 < 59) and is reported "not desirable", i.e. it stays
 *      INLINE at the top of the inner loop the way the target has it, and is no
 *      longer hoisted on out of the outer loop either.
 *   2. target_color is one flat single expression instead of `|=` accumulation.
 *
 * Result: build_insns == target_insns == 173 (s1's best had 174, an extra
 * `move s6,a0`), r/g/b land in the target's s5/s4/s3 and target_color in s6.
 *
 * Chassis: HEAD 2026-09-10 (post -mel / -msoft-float). Score measured with
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

            if (total > 0) {
                s32 j = 0;
                do {
                    s32 complement = blend_base - factor;
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst++ = pixel;
                            src++;
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
