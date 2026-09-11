/* func_8003DE14 - s20 chassis: the incumbent's 26 reached with the declaration and
 * assignment orders of the j / complement pair SPLIT.  Score 26 / 173 on HEAD
 * 2026-09-10, identical to candidate.c.
 *
 * Guard block:   s32 j;  s32 complement;  complement = blend_base - factor;  j = 0;
 *
 * WHY IT IS BANKED.  It isolates the two levers s18/s19 conflated.  The pseudo NUMBER
 * follows declaration order (so here j = p115, complement = p116 - h1's mapping, not
 * the incumbent's), while reg_live_length follows ASSIGNMENT order (so here j still
 * gets livelen 54 / pri 6111 / $t4 and complement 55 / 6000 / $t5 - the incumbent's
 * seats).  Measured siblings: e2 (decl j,complement / assign j,complement) 30, e3
 * (decl complement,j / assign j,complement) 30.  Use this body, not candidate.c, when
 * a probe needs the incumbent's seats with h1's pseudo numbering - for example when
 * attacking the blend arm's 32727 priority TIE, which global.c:654 breaks by the lower
 * allocno number, i.e. by declaration order.
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
                s32 j;
                s32 complement;
                complement = blend_base - factor;
                j = 0;
                do {
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
                            s32 rp;
                            s32 gp;
                            s32 bp;
                            s32 r_ch;
                            s32 g_ch;
                            s32 b_shift;
                            src++;
                            rp = r_src * complement;
                            r_src = r * factor;
                            r_ch = ((rp + r_src) >> 15) & 0x1F;
                            gp = g_src * complement;
                            g_src = g * factor;
                            g_ch = ((gp + g_src) >> 10) & 0x3E0;
                            bp = b_src * complement;
                            b_src = b * factor;
                            b_shift = (bp + b_src) >> 5;
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
