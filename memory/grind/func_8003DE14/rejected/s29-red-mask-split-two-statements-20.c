/* s29 REJECT: r_src = sum >> 15; r_src = r_src & 0x1F; produces the target's two-insn red shape but swaps the red/green source carriers - 20 against the 5 control */
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
                s32 complement = blend_base - factor;
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
                            s32 sum;
                            s32 rp;
                            s32 gp;
                            src++;
                            rp = r_src * complement;
                            r_src = r * factor;
                            sum = rp + r_src;
                            r_src = sum >> 15;
                            r_src = r_src & 0x1F;
                            gp = g_src * complement;
                            g_src = g * factor;
                            sum = gp + g_src;
                            g_src = sum >> 10;
                            px = ((u32)px >> 7) & 0xF8;
                            px = px * complement;
                            sum = px + b * factor;
                            px = sum >> 5;
                            *dst = (pixel & 0x8000) | r_src | (g_src & 0x3E0) | (px & 0x7C00);
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
            LoadImage((s32)rect, ((s32)dst_buf + j) - j);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
