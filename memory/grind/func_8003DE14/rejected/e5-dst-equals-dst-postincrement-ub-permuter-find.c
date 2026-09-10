/* REJECTED (s4, permuter modality).  This is decomp-permuter's best find on the
 * d4 chassis (tmp/perm_3DE14_s4a/output-300-1, permuter score 300 vs base 360):
 * the zero-pixel arm respelled `dst = dst++; *dst = pixel;`.
 * MEASURED: score 30, build_insns 174.
 * REJECTED ON SEMANTICS, not on policy: `dst = dst++` has an unsequenced
 * self-assignment (undefined behaviour) and under this cc1 leaves dst UNCHANGED,
 * so the zero-pixel arm no longer advances the destination cursor - the function
 * computes a different image.  A permuter proposal, not a candidate.  Its ONE
 * useful datum is that the extra dst reference is what buys the point; every
 * semantics-preserving respelling of the same reference count (e1/e2/e4/e6)
 * folds away in RTL and measures 31/172 unchanged. */
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
                            dst = dst++;
                            *dst = pixel;
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
