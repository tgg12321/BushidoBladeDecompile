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
                /* FAKE: `h` is a single staging local used twice - it carries the
                 * fast arm's target_color into its store, and it carries the latch
                 * bound's rect[3] halfword.  Having BOTH references inside the inner
                 * loop (two basic blocks at loop depth 3) is what makes the latch's
                 * rect[3] pseudo non-block-local; mechanism: local-alloc.c:470-476
                 * skips any pseudo with reg_basic_block < 0, so local-alloc seats only
                 * the block-local rect[2] load ($v0, target) and global.c seats h
                 * afterwards ($v1, target), while the first scheduling pass emits the
                 * escaped pseudo's load SECOND - the target's `lh $v0,4` / `lh $v1,6`
                 * order; lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md
                 * s24-s31 (operand order, birth order, epilogue linkage, for-loop and
                 * pointer-alias waves) + s32 waves a-h. */
                do {
                    if (i == count - 1) {
                        u16 pixel = *src;
                        if (pixel == 0) {
                            *dst = pixel;
                            src++;
                            dst++;
                            goto loop_check;
                        }
                        total = target_color;
                        *dst++ = total;
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
                            /* FAKE: `gm` names the green channel's masked result so that
                             * g_src dies at the mask instead of at the store; mechanism:
                             * global.c allocno priority (prio = nrefs*40000/live_length,
                             * dumped via BB2_ALLOC_DEBUG) - naming gm takes green from
                             * 18 refs/livelen 18 to 18/16 and red from 24/21 to 24/22, so
                             * pri(px)=44444 > pri(red)=43636 and px is allocated $a0 with
                             * red $a1 and green $v1, the target's seat map; without the
                             * name red is 24/21=45714, outranks px, steals $a0 and the body
                             * scores 17.  lever-exhaustion:
                             * memory/grind/func_8003DE14/hypotheses.md s29-s35 (seat
                             * inequality, OR re-association, cross-block hoist, red-tail
                             * splits) + s36 waves x (borrowed carriers gp/sum/rp and the
                             * in-place `g_src = g_src & 0x3E0;` split all measured 17-43). */
                            s32 gm;
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
                            gm = g_src & 0x3E0;
                            *dst = (((pixel & 0x8000) | r_src) | gm) | (px & 0x7C00);
                        }
                    }
                    dst++;
                loop_check:
                    j++;
                } while (j < rect[2] * (total = rect[3]));
            }

            {
                s32 new_y = ((u16 *)rect)[1] + ((u16 *)rect)[3];
                ((u16 *)rect)[1] = new_y;
                if ((s16)new_y >= 0x200) {
                    rect[1] = saved_y;
                    ((u16 *)rect)[0] += ((u16 *)rect)[2];
                }
            }
            /* FAKE: j chain extender on the dst_buf argument (s21); mechanism:
             * combine.c folds the +j/-j pair away but flow.c's reg_n_refs for j is
             * counted before it, lifting j's allocno priority so the $t4/$t5 seat
             * pair matches; lever-exhaustion: memory/grind/func_8003DE14/
             * hypotheses.md s21-s30 (the extender-free chassis floors at 8). */
            LoadImage((s32)rect, ((s32)dst_buf + j) - j);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
