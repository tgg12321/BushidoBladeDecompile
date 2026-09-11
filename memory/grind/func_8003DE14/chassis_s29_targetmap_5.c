/* func_8003DE14 - candidate (grind session 29, SYNTHESIS modality).
 *
 * SCORE 5 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12 (s23-s28) -> 5 (this
 * session).  The extender-free (fully ordinary-C) best on the same chassis is 8
 * (chassis_s29_targetmap_ordinary_8.c), down from the 38 that stood since s26.
 *
 * WHAT CHANGED - the target's register map was read off the residual and spelled
 * directly.  s24 banked the p2 residual as "in the TARGET every intermediate goes
 * to a fresh scratch register and the final channel value is written back into the
 * operand's register one instruction later".  Read as C rather than as allocator
 * behaviour, that sentence describes TWO ordinary C constructs at once:
 *
 *   (a) ONE variable carries all three channel SUMS  (s27's three-way `sum`:
 *       three defs / three deaths -> local-alloc.c:470-476 makes it ineligible,
 *       so it is a global allocno and owns $v0 across the arm, which is what
 *       forces `b * factor` into $t7 through reload's ascending retry scan), and
 *   (b) each channel RESULT is written back into ITS OWN SOURCE CARRIER -
 *       `r_src = (sum >> 15) & 0x1F`, `g_src = sum >> 10`, `px = sum >> 5` -
 *       which is exactly the target's `sra $a1,$v0,15` / `sra $v1,$v0,10` /
 *       `sra $a0,$v0,5` (the shift lands in the source's dead seat).
 *
 * (b) is also the honest replacement for the s21 F1 g_src chain extender: the
 * write-back gives r_src / g_src / px three real defs and three real deaths each
 * (.lreg: 123 "18 times across 20 insns ... dies in 3 places", 126 "18 across 18
 * ... dies in 3", 122 "30 across 27 ... dies in 3"), so the reference counts that
 * the dead `((sum + g_src) - g_src)` identity used to buy are now bought by real
 * code.  THE g_src CHAIN EXTENDER IS GONE FROM THIS BODY.
 *
 * The green channel deliberately masks in the OR (`g_src = sum >> 10;` then
 * `(g_src & 0x3E0)`) while the red channel masks in the shift statement.  The
 * asymmetry is load-bearing and measured: making both symmetric (either both in
 * the shift statement or both in the or) swaps the red and green source carriers
 * $a1 <-> $v1 and costs 12-17 points (s29 wave 3, 32-body cross product).
 *
 * THE RESIDUAL IS 5 ROWS, in two independent groups:
 *   t118/t119  sra $a1,$v0,15 / andi $a1,$a1,31   (ours: sra $v0,$v0,15 /
 *              andi $a1,$v0,31) - the red shift's intermediate is seated on the
 *              sum's own register instead of on r_src's dead seat.  Eight
 *              spellings of the red statement tie at 5; splitting it into
 *              `r_src = sum >> 15; r_src = r_src & 0x1F;` produces the target's
 *              two-insn shape but swaps the red/green carriers (20).
 *   t133/t134/t136  lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1  (ours: the two
 *              lh destinations swapped).  Latch-condition spelling is inert here
 *              (>, != 0, operand swap, signed casts all tie at 5); the priority
 *              arithmetic in qty_compare_1 favours the SHORTER-span second load
 *              for $v0 (2*6/2 = 60000 vs 2*6/3 = 40000).
 *
 * ORDINARY-C STATUS.  One FAKE construct remains, the s21 j chain extender
 * `((s32)dst_buf + j) - j` in the LoadImage call, worth 3 points here (5 vs 8).
 * Its whole job is the $t4/$t5 seat pair (j vs complement).  s29 found an ORDINARY
 * substitute - declaring `s32 j = 0;` inside the `if (total > 0)` block - which
 * fixes those two seats for free but sinks the `addiu $a2,$sp,1040` dst-cursor
 * init past the guard (3 rows).  Reconciling those two is the top frontier item.
 * Everything else in this body is the sanctioned variable-reuse family plus
 * ordinary named intermediates.
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
