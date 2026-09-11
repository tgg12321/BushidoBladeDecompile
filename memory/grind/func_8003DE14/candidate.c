/* func_8003DE14 - candidate (grind session 21, forensics modality).
 *
 * SCORE 16 / 173 insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * This is a NEW FLOOR: the ledger stood at 26 for eight sessions (s13-s20).
 *
 * CHASSIS: the s18 h1 body (head region byte-exact, score 28) plus TWO
 * combine-foldable chain extenders (the sanctioned F1 family, owner ruling
 * 2026-07-01), each annotated in place below.  Both exploit the SAME measured
 * window: flow_analysis (toplev.c:2984) fills reg_n_refs / reg_live_length
 * ONCE, before combine_instructions (toplev.c:3004), and life analysis is
 * never re-run - so a reference on an insn combine later folds away is priced
 * permanently by global.c and costs zero bytes.
 *
 * ABLATION MATRIX (this session, every form 173 build / 173 target insns):
 *     base26  incumbent s13 body, no lift .................... 26
 *     h1      s18 head-exact body, no lift .................... 28
 *     q3      h1 + j lift (LoadImage arg) ..................... 23
 *     y1      h1 + g_src lift (blue sum) ...................... 22
 *     y2      base26 + g_src lift ............................. 19
 *     u3      h1 + BOTH lifts  <-- this body .................. 16
 * The two levers are independent and their effects compose.
 *
 * WHAT EACH LEVER BUYS
 *  (1) j lift.  s19 derived the window pri(j) must land in: (6111, 6964).
 *      s20 measured every depth-1 / depth-2 passenger site it could find and
 *      KILLED them - correctly, but only for sites BEFORE the inner loop,
 *      where cse2 knows j == 0 and folds the chain at tree/cse level.  The
 *      site AFTER the inner loop (j is the loop-exit value, not a constant)
 *      survives cse2: nrefs(j) 11 -> 15, livelen 59 -> 73, pri 5593 -> 6164,
 *      which is inside the window.  j takes ord=15 / hardreg 12 ($t4) and
 *      complement ord=16 / hardreg 13 ($t5) - the target's seats - while the
 *      head region stays byte-exact.  Residual falls 28 -> 23, and the ENTIRE
 *      head, prologue and guard are now byte-identical to asm/funcs.
 *  (2) g_src lift.  The blend arm's residual is one global.c ordering fact:
 *      px (pseudo 122, nrefs 12 / livelen 11 / pri 32727) is allocated before
 *      g_src (pseudo 126, nrefs 12 / livelen 12 / pri 30000), so px takes $v1
 *      and g_src takes $a0; the target has them the other way round.  A
 *      depth-3 passenger on g_src prices it at 60000, so g_src is allocated
 *      first and takes $v1 while px falls to $a0 and r_src keeps $a1.
 *      Residual falls 23 -> 16.
 *
 * WHAT IS LEFT (16 insns, tmp/grind/func_8003DE14/s21/u3_sxs.txt)
 *  (a) b_src SEAT, 13 insns (rows 106-108, 114-119, 122, 123).  b_src
 *      (pseudo 128, pri 32727) takes hardreg 2 ($v0); the target seats it on
 *      $a0 - the register px has just vacated (px dies at row 105, b_src is
 *      defined at row 106, so they do not conflict and CAN share).  In our
 *      build $v0 is free at b_src's allocation point, so global.c's
 *      lowest-free rule takes it.  Forcing $a0 needs a CONFLICTING allocno to
 *      hold $v0 across b_src's range.  Measured dead this session: lifting
 *      b_src itself (v1, still $v0 at ord=1), lifting px (v2), lifting r_src
 *      (v3), lifting bp (v4), one shared `sum` local across all three channels
 *      (w1/w2 - it DOES take $v0 but conflicts with everything and scores 52),
 *      and three per-channel `r_sum/g_sum/b_sum` locals (x1/x2 - combine folds
 *      them back, alloc table and score unchanged).
 *  (b) TRIP TEST, 3 insns (rows 127/128/130): target `lh v0,4(s0) /
 *      lh v1,6(s0) / mult v0,v1` vs ours `lh v1 / lh v0 / mult v1,v0` - two
 *      short-lived locally-allocated temps named the other way round.
 *
 * ORDINARY-C STATUS.  Both lifts are F1 chain extenders and therefore require
 * the 2026-07-01 grant plus these annotations; a ruling-request is the correct
 * next step BEFORE any candidate-ready that carries them.  No ordinary-C
 * carrier for either price has been found (see the lever-exhaustion pointers
 * on each annotation).
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
                            /* FAKE: combine-foldable chain extender on the
                             * blue sum; passenger `g_src`.  mechanism: the
                             * same flow.c-counts-then-combine-folds window -
                             * reg_n_refs(g_src) 12 -> 18 at loop-depth 3, so
                             * global.c prices g_src at 60000 (> px's 32727),
                             * allocates it first and seats it on $v1 while px
                             * falls to $a0, the target's pair.
                             * lever-exhaustion: s16 (1,483 carrier spellings),
                             * s21 t1-t4 (statement order inert), u1/u2 (decl
                             * hoist), w1/w2 + x1/x2 (named sums fold away). */
                            b_shift = (((bp + b_src) + g_src) - g_src) >> 5;
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
            /* FAKE: combine-foldable chain extender on the LoadImage dst
             * argument; passenger `j`.  mechanism: flow.c life analysis
             * (toplev.c:2984) counts the two extra `j` references at
             * loop-depth 2 and extends REG_LIVE_LENGTH(j) 59 -> 73 BEFORE
             * combine_instructions (toplev.c:3004) folds the chain away, so
             * global.c prices j at 6164 instead of 5593 and seats it on $t4.
             * lever-exhaustion: memory/grind/func_8003DE14/hypotheses.md s15
             * (3,966 spellings), s16, s19, s20 (p1/p5/p8 depth-1/2 sites all
             * cse2-folded), s21 (q1/q2/q4/r1/r3 carriers all out of window). */
            LoadImage((s32)rect, ((s32)dst_buf + j) - j);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
