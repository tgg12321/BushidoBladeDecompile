/* func_8003DE14 - best banked form (grind session 36b, ENUMERATE modality).
 *
 * SCORE 2 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float),
 * `sandbox func_8003DE14 --disable all`.  This is the best JUDGE-PERMISSIBLE
 * body known.  FLOOR HISTORY: 26 -> 16 (s21) -> 14 (s22) -> 12 (s23) -> 5
 * (s29) -> 4 (s31) -> 2 (s32) -> 1 (s33-s35) -> 0 with the `h` carrier (s36,
 * REJECTED by the Judge 2026-09-11 04:48) -> 2 here with `h` replaced by the
 * ruling-sanctioned `total` borrow.
 *
 * WHAT CHANGED FROM THE REJECTED BODY.  The Judge FAILed the fresh multi-written
 * local `h` (docs/grind/decisions.md, 2026-09-11 04:48) and ANSWERED the second
 * question in the same ruling: staging through the EXISTING, genuinely-dead-at-
 * that-point `total` IS inside [[staged-value-reused-variable]], provided each
 * site carries its own liveness sentence in the /* FAKE *\/ annotation.  This
 * body does exactly that, and this session additionally proved the FAST-ARM
 * staging site is unnecessary: `total` is borrowed at ONE site, the latch.
 * (Both sites staged = 2 as well; tmp/grind/func_8003DE14/s36b/base.c.)
 *
 * THE REMAINING 2 ROWS, IN CLOSED FORM.  They are the two halfword loads of the
 * loop bound.  Target: `lh $v0,4($s0)` then `lh $v1,6($s0)` then `mult $v0,$v1`.
 * Ours: `lh $v1,6($s0)` then `lh $v0,4($s0)` - the REGISTERS are already the
 * target's, only the emission ORDER is swapped.  Pass attribution is dumped,
 * not guessed (tmp/grind/func_8003DE14/s36b/dumps_{h,b5,tot}): after .combine
 * BOTH bodies have the rect[2] load first and the rect[3] load second, with
 * byte-identical RTL apart from the carrier's pseudo number (117 for `h`, 101
 * for `total`); the FIRST scheduling pass leaves that order alone when the
 * carrier is `h` and swaps it when the carrier is `total`.
 *
 * THE WALL, STATED AS TWO RULES THAT CANNOT BOTH BE SATISFIED BY THIS BORROW.
 *   (R1) the escaped carrier's load is scheduled FIRST when the carrier's other
 *        set lives in the OUTER row block (`total`), and SECOND when both of its
 *        sets live inside the inner loop (`h`);
 *   (R2) local-alloc seats the block-local load in $v0 and global.c seats the
 *        escaped carrier in $v1 (measured on every body this session).
 * The target needs the FIRST load in $v0, i.e. the block-local one first, i.e.
 * an escaped carrier whose other set is inside the inner loop.  `total`'s only
 * other set is its real job (the row area, read by the `if (total > 0)` guard),
 * which is necessarily in the outer block.  Carrying the OTHER operand instead
 * (`while (j < rect[3] * (total = rect[2]));`,
 * tmp/grind/func_8003DE14/s36b/waveD/d8_c07_swapmul.c) is the exact complement:
 * it scores 2 with the ORDER and the `mult $v0,$v1` correct and only the two lh
 * destination registers swapped.
 *
 * FAKE CONSTRUCTS PRESENT (3, all inside frozen SOTN-sanctioned families):
 *   (1) `total` staged at the latch - [[staged-value-reused-variable]], granted
 *       for this exact site by the Judge ruling of 2026-09-11 04:48.
 *   (2) `gm` - named intermediate for the green mask (s36, worth 15 pts: the
 *       same body with the mask inline scores 17).
 *   (3) the s21 `((s32)dst_buf + j) - j` chain extender (7 pts; plain
 *       `(s32)dst_buf` measures 7/173 worse).
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
                            *dst = (pixel & 0x8000) | r_src | gm | (px & 0x7C00);
                        }
                    }
                    dst++;
                loop_check:
                    j++;
                /* FAKE: the inner loop's bound re-reads rect[3] each iteration and the
                 * read is staged through the EXISTING local `total` instead of a compiler
                 * temp, so that the bound's height pseudo escapes its basic block.
                 * Per-site liveness (the one site, the latch): `total` last held the row
                 * area rect[2]*rect[3], whose only reader is the `if (total > 0)` guard
                 * ABOVE this loop; at the latch that value is dead, and `total` is
                 * re-assigned from rect[2]*rect[3] at the top of every outer row before
                 * anything reads it again, so no live value is clobbered and the staged
                 * value (rect[3]) is consumed immediately by the multiply that is the
                 * loop bound.  Zero dead code.  Mechanism: local-alloc.c:470-476 skips
                 * any pseudo with reg_basic_block < 0, so local-alloc seats only the
                 * block-local rect[2] load ($v0, target) and global.c seats the escaped
                 * height pseudo afterwards ($v1, target).  Lever-exhaustion:
                 * memory/grind/func_8003DE14/hypotheses.md s24-s32 (operand order, birth
                 * order, epilogue linkage, for-loop and pointer-alias waves) + s36 waves
                 * y/z/q/L/m/n + this session's waves A-E (48 latch/tail/declaration
                 * spellings, all >= 2). */
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
