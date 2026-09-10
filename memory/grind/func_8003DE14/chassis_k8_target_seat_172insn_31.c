/* func_8003DE14 - chassis k8 (grind s6, `synthesis` modality).  Score 31 /
 * build_insns 172 (target 173).  ORDINARY C throughout - no FAKE construct.
 *
 * k8 = k1 with the LAST-FRAME zero-pixel arm respelled `*dst++ = pixel;
 * src++; goto loop_check;` (the s3 "d4" dose, carried onto the k1 blend
 * shape).  That is +2 dst RTL references at inner-loop depth 3 = +6 weighted
 * reg_n_refs, taking dst from 26 to 32 refs, which crosses the floor_log2
 * step in global.c's allocno_compare and puts dst AHEAD of src:
 *   109 (dst) pri 27118 -> $a2      108 (src) pri 26666 -> $a3
 * i.e. THE TARGET'S SEAT, verified by tools/ra_solver simulate --trace on
 * tmp/grind/func_8003DE14/s6/k8.model.json (dispositions match).
 *
 * The price is the 173rd instruction: the arm's tail is now identical to the
 * colour arm's (`sh / addiu a2 / j loop_check / addiu a3`) and jump2
 * cross-jumps it away, so the body is 172 insns.  This file is banked as the
 * PROOF that the seat is reachable and exactly what it costs - it is not a
 * candidate.
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
