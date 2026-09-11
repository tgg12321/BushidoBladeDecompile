/* func_8003DE14 - candidate (grind session 31, STRUCTURAL modality).
 *
 * SCORE 4 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12 (s23-s28) -> 5 (s29,
 * s30) -> 4 (this session).  The fully-ordinary-C (extender-free) best is still
 * 8 (chassis_s29_targetmap_ordinary_8.c); the ORDINARY-C best on THIS chassis is
 * 5 (`} while (j < rect[3] * rect[2]);`, s31 v04 - see below).
 *
 * WHAT CHANGED IN s31 - the row loop's latch.  s30 typed the whole residual as
 * RA and closed the latch group in closed form (qty_compare_1 prices the
 * SECOND-born of two equal-refs block-local quantities higher, so the second
 * load takes $v0).  s31 read that as a statement about C: the latch's two
 * halfword loads are born in SOURCE OPERAND ORDER, so which load owns $v0 is
 * chosen by which operand of the exit test's multiply is written first.
 *
 *   - `} while (j < rect[3] * rect[2]);` (s31 v04) flips the births and lands
 *     BOTH target seats (lh $v0,4 / lh $v1,6) - the two `lh` register rows are
 *     gone - but the multiply then reaches RTL as mult(off6, off4) and prints
 *     `mult $v1,$v0` where the target has `mult $v0,$v1`.  Net: still 5.
 *   - Staging rect[3] through `h` at the bottom of the row loop and testing
 *     `j < rect[2] * h` decouples the two: h's load is emitted (and born) FIRST,
 *     so the rect[2] load is the shorter-span quantity and takes $v0, while the
 *     multiply keeps its (rect[2], h) operand order and prints `mult $v0,$v1`.
 *     That is the target's multiply AND the target's two seats: 5 -> 4.
 *
 * THE RESIDUAL IS 4 ROWS, in two independent groups:
 *   latch  ours `lh $v1,6($s0)` / `lh $v0,4($s0)`; target `lh $v0,4($s0)` /
 *          `lh $v1,6($s0)`.  REGISTERS AND MULTIPLY NOW MATCH; only the two
 *          loads' ORDER differs.  Emission order == RTL birth order (verified:
 *          .sched/.sched2/.dbr/.s all carry the same order and gas does not
 *          reorder), and the first-born loses $v0 by qty_compare_1.  The target
 *          therefore has its FIRST-born load in $v0, which local-alloc cannot
 *          produce for two equal-refs block-local quantities - one of the loads
 *          must escape local-alloc (local-alloc.c:470-476) and be seated by
 *          global.c.  That is the top frontier item.
 *   red    ours `sra $v0,$v0,15` / `andi $a1,$v0,31`; target `sra $a1,$v0,15` /
 *          `andi $a1,$a1,31` - unchanged from s29/s30 (combine_regs ties the
 *          shift's destination to the dying three-way `sum`).
 *
 * FAKE CONSTRUCTS PRESENT: (1) the s21 j chain extender `((s32)dst_buf + j) - j`
 * in the LoadImage call (worth 3 points), (2) the new `h` latch-bound stage
 * (worth 1 point).  Both are annotated in the body.
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
                s32 complement = blend_base - factor;
                /* FAKE: `h` stages the latch bound's rect[3] halfword so the row loop's
                 * exit test reaches RTL as mult(rect[2]_load, h) with h's load BORN FIRST;
                 * mechanism: local-alloc.c:1660 qty_compare_1 prices the shorter-span
                 * (second-born) quantity higher, so staging rect[3] is what seats the
                 * rect[2] load in $v0 and keeps the multiply's operand order (off4,off6);
                 * lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s31 (latch
                 * operand-order, guard-shape, for-loop, cross-block-staging and
                 * epilogue-linkage waves) + s30 H-s30-4. */
                s32 h;
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
                            r_src = (sum >> 15) & 0x1F;
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
                    h = rect[3];
                } while (j < rect[2] * h);
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
