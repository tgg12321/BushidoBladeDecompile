/* func_8003DE14 - candidate (grind session 33, REDERIVE modality).
 *
 * SCORE 1 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12 (s23-s28) -> 5 (s29,
 * s30) -> 4 (s31) -> 2 (s32) -> 1 (this session).  The red shift pair that had
 * been the standing residual since s29 is CLOSED; ONE row remains.
 *
 * WHAT s33 FOUND (the red pair was never an RA-tie problem).  s29-s32 all read
 * the red residual - ours `sra $v0,$v0,15` / `andi $a1,$v0,31` vs the target's
 * `sra $a1,$v0,15` / `andi $a1,$a1,31` - as combine_regs tying the shift's
 * destination to the dying three-way `sum`, and spent four sessions trying to
 * break that tie by making `sum` / `rp` / the destination escape local-alloc.
 * The actual cause is one level up: `r_src = (sum >> 15) & 0x1F;` is a SINGLE
 * expression, so expand creates a fresh single-set temp for the shift, and that
 * temp is what combine_regs ties to `sum`.  Splitting it into two statements -
 *     r_src = sum >> 15;
 *     r_src = r_src & 0x1F;
 * - makes the shift's destination the ALREADY-MULTI-SET pseudo r_src (it is
 * written twice before this, by the sll and by the mflo), so no tie can be
 * formed and both insns print into r_src's own seat.  That is byte-exactly the
 * target's shape, and our GREEN channel had been carrying it all along
 * (`g_src = sum >> 10;` with the mask deferred to the OR chain prints
 * `sra $v1,$v0,10` / `andi $v1,$v1,0x3E0`, which is the same two-register form).
 *
 * THE PRICE, AND HOW IT IS PAID.  The split adds two references to r_src (6 -> 8,
 * weighted 18 -> 24 at loop depth 3), which flips the $a0/$a1 seats of `px` and
 * `r_src` in global.c's allocno ordering.  BB2_ALLOC_DEBUG, red-split chassis
 * with nothing else changed:
 *     px    pseudo 123  nrefs=30 livelen=27 pri=44444   -> $a1   (WRONG)
 *     r_src pseudo 124  nrefs=24 livelen=21 pri=45714   -> $a0   (WRONG)
 * a 2.8% margin the wrong way, and the whole body scores 17.  Handing px ONE
 * more real reference restores the order and the score drops to 1.  This body
 * spends that reference on the red channel's source read: `(px & 0x1F) << 3`
 * instead of `(pixel & 0x1F) << 3` - px and pixel carry the same value there, so
 * it is ordinary C, and px goes to nrefs=33 / pri=61111, comfortably above r_src.
 *
 * THE RESIDUAL IS 1 ROW: target `andi $v0,$t0,0x1F` (the red source is read out
 * of `pixel`, $t0) against our `andi $v0,$a0,0x1F` (read out of `px`, $a0).  It
 * is the very reference we spent to win the seat, so the last point is exactly
 * "find a FREE way to give px one more reference (or r_src one more unit of live
 * length) and put the red source read back on `pixel`".
 *   - chassis_s33_pxsplit_1.c is the OTHER 1-point body: it keeps
 *     `(pixel & 0x1F) << 3` and buys the reference by splitting px's birth
 *     (`s32 px = pixel; px = px & 0xFFFF;`).  Its single wrong row is the mirror
 *     image - combine proves the 0xFFFF mask redundant against the lhu's
 *     nonzero_bits and folds the pair into `move $a0,$t0` where the target keeps
 *     `andi $a0,$t0,0xFFFF`.  Two independent 1-point forms, two different rows.
 *
 * FAKE CONSTRUCTS PRESENT: (1) the s21 j chain extender `((s32)dst_buf + j) - j`
 * in the LoadImage call (3 pts), (2) the s32 `h` staging local (2 pts, both latch
 * rows).  NOTHING s33 added is FAKE: the red split is a plain two-statement
 * spelling and `(px & 0x1F)` is a read of an in-scope local holding that value.
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
                            s32 r_src = (px & 0x1F) << 3;
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
