/* func_8003DE14 - candidate (grind session 32, FORENSICS modality).
 *
 * SCORE 2 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12 (s23-s28) -> 5 (s29,
 * s30) -> 4 (s31) -> 2 (this session).  THE ENTIRE LATCH GROUP IS NOW MATCHED;
 * the only residual is the red-shift pair.
 *
 * WHAT s32 FOUND.  s31 left the latch's two `lh` rows differing only in ORDER and
 * attributed the order to RTL birth order (statement order).  That attribution is
 * wrong: the dumps show the FIRST SCHEDULING PASS reorders them.  Body c04 of this
 * session puts the rect[2] load first in .combine (insn 298 = offset 4, insn 303 =
 * offset 6) and the .lreg dump - i.e. after `sched`, before local-alloc - has them
 * swapped.  The lever that decides the order is not statement position but whether
 * the destination pseudo is BLOCK-LOCAL:
 *   - the operand whose destination pseudo escapes local-alloc has its load emitted
 *     FIRST and is seated by global.c;
 *   - the remaining block-local quantity is seated by local-alloc and takes $v0.
 * So the target's `lh $v0,4($s0)` / `lh $v1,6($s0)` / `mult $v0,$v1` is exactly
 * "rect[2] block-local, rect[3] escaped": the rect[3] value must be carried by a
 * pseudo referenced in a SECOND basic block, with its load still inside the latch.
 * Every s31 way of doing that moved the load out of the latch or let loop.c/CSE
 * delete it; the way that works is a staging local whose other reference is also
 * INSIDE the inner loop - here the fast arm's `h = target_color; *dst++ = h;`.
 *
 * THE RESIDUAL IS 2 ROWS:
 *   red    ours `sra $v0,$v0,15` / `andi $a1,$v0,31`; target `sra $a1,$v0,15` /
 *          `andi $a1,$a1,31` - unchanged since s29.  combine_regs
 *          (local-alloc.c:1854-1897) ties the shift's destination to its dying
 *          source, the three-way shared `sum` that owns $v0.  s32 measured the new
 *          escape lever against it: making `sum`, `rp` or the shift's own
 *          destination cross-block costs 1-18 points and never moves the sra seat.
 *
 * FAKE CONSTRUCTS PRESENT: (1) the s21 j chain extender `((s32)dst_buf + j) - j`
 * in the LoadImage call (3 pts), (2) the `h` staging local (2 pts: it closes both
 * latch rows).  FAMILY QUESTION FOR THE NEXT SESSION: `h` is an INVENTED local
 * borrowed for a second value, which the family-selection table puts outside plain
 * variable-reuse (bound 2) and closest to staged-value-reused-variable
 * (.claude/rules/staged-value-reused-variable.md).  Read that rule end to end - and
 * consider a spelling in which an EXISTING local is the carrier (s32's h01 borrowed
 * `total` and measured 4) - before any candidate-ready submission.
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
                        h = target_color;
                        *dst++ = h;
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
                } while (j < rect[2] * (h = rect[3]));
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
