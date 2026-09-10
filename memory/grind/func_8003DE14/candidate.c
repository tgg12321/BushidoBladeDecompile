/* func_8003DE14 - best form as of grind session 3 (structural), honest floor
 * 31/173 (was 52 at the end of s2).  ORDINARY C THROUGHOUT: no /* FAKE *\/
 * construct, no borrowed local, no sanctioned-family claim needed.
 *
 * Inherited from s2 and still load-bearing:
 *   - `s32 complement = blend_base - factor;` DECLARED INSIDE the inner
 *     do-body.  It is loop-invariant, so loop.c hoists it back to the
 *     inner-loop preheader (the target's `subu $t5,$fp,$t3` at 8003DF30), but
 *     it is now the FIRST movable, so move_movables spends the first move on
 *     it and applies `threshold -= 3` (loop.c:1904); the `count - 1` comparand
 *     then fails the movable test and stays INLINE the way the target has it.
 *
 * Three s3 changes, each measured on this chassis (see evidence.md s3):
 *   1. target_color is spelled RED-OR-CONSTANT FIRST:
 *        (((u32)r >> 3) | (s32)-0x8000) | ((g & 0xF8) << 2) | ((b & 0xF8) << 7)
 *      fold-const.c's `associate:` block (split_tree, fold-const.c:882/3696)
 *      rewrites `(VAR|CON) | ARG1` into `VAR | (ARG1|CON)`, so this source
 *      spelling reaches RTL as `red | (green | -0x8000)` - the TARGET's tree.
 *      Writing the constant next to the green term (s2's y1 form) is what
 *      produced the build's wrong `(red|K)|green`.  The emitted color block is
 *      now byte-exact vs 8003DEA0-8003DED0.
 *   2. `i = 0;` is placed LATE (immediately before `if (count > 0)`) instead of
 *      early.  This shortens pseudo `i`'s live range enough to lift its
 *      global.c allocno priority above `count`'s (they sit 1% apart: 3277 vs
 *      3243 before the move), so `i` takes $s1 and `count` takes $s2 as in the
 *      target, and the `move sN,zero` also lands in the target's slot.
 *   3. The i == count-1 zero-pixel arm writes `*dst++ = pixel;` and jumps
 *      straight to the loop-check label instead of sharing the `advance_dst:
 *      dst++;` tail.  That lifts `dst`'s reg_n_refs enough to swap the
 *      src/dst cursor priorities, so src lands in $a3 and dst in $a2 as in the
 *      target (ra_solver inverse: the goal needed refs_up on dst by ~6).
 *
 * Residual 31 (three items, see hypotheses.md s3 frontier):
 *   (a) build_insns 172 vs target 173 - jump2 cross-jumps that arm's tail one
 *       instruction further than the target does;
 *   (b) `j`/`complement` still swapped (build t5=j/t4=complement, target
 *       t4=j/t5=complement) - ra_solver says REACHABLE with ONE atom;
 *   (c) blend-arm emission order (the mflo/srl interleave on the blue channel
 *       and the final or/andi pair) - a sched.c item, no count delta.
 *
 * Chassis: HEAD 2026-09-10 (post -mel / -msoft-float).  Score measured with
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
