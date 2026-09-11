/* func_8003DE14 - candidate (grind session 22, REDERIVE modality).
 *
 * SCORE 14 / 173 insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * NEW FLOOR: 26 (s13-s20) -> 16 (s21) -> 14 (this session).
 *
 * CHASSIS.  s21's u3 body (the s18 head-exact chassis + the two F1
 * combine-foldable chain extenders) with the BLEND ARM RE-DERIVED: the blue
 * channel no longer reuses its source variable for the second product, and the
 * GREEN and BLUE channel sums are staged through one shared `sum` local.
 *
 * THE TWO STRUCTURAL CHANGES (ablation, every form 173 build / 173 target):
 *     u3   s21 incumbent (blue reuses b_src for b*factor) ....... 16
 *     a2   u3, blue written as one expression, `bp` dropped ..... 15
 *     d2   a2 + shared `sum` local for the GREEN and BLUE sums .. 14   <- this
 *   counter-ablations: d1 `sum` for all three channels 30; f1 `sum` for red +
 *   green 30; f2 `sum` for red only 15 (single-use, combine folds it back);
 *   a1 blue product reuses b_src 38; a3 all three products reuse their source
 *   38/50; b1 fully inline arm, no channel locals 43; g1/g2 blue chained
 *   through ONE variable all the way to the mask 31/55; e1-e3 the output word
 *   accumulated into a named `out` 48/41/36.
 *
 * LEVER ACCOUNTING on this body (both s21 FAKE extenders still load-bearing):
 *     d2 as written ............................. 14
 *     d2 minus the g_src chain extender (c2) .... 23
 *     d2 minus the j chain extender (c3) ........ 21
 *     neither extender (c5) ..................... 28
 *
 * WHAT IS LEFT - 14 insns, and s22 proved they are ONE decision plus the trip
 * test.  Rows 106,107,108,111,112,114,115,118,119,123,124 are all downstream of
 * a single register choice: the blue temp (pseudo 139: `b_src` and the
 * b_src*complement product, which share a register in BOTH builds) takes $v0 in
 * ours and $a0 in the target.  With it on $a0 the three channel sums all fall to
 * $v0 (the target's rows 111/114/118) and b_shift keeps $a0.
 *   MECHANISM, read out of global.c with BB2_FINDREG_DEBUG=139 (the exact sets
 *   are in tmp/grind/func_8003DE14/s22/findreg_d2_139/stderr.log):
 *     conflicts(139)   = {3,5,6,7,8,16,29}   - 2 ($v0) and 4 ($a0) both absent
 *     someone_prefers  = {}                  - empty
 *     pass0_used       = conflicts + the not-yet-used regs; 2 is free, so
 *                        find_reg's first-fit loop returns 2.
 *   To land 4, regno 2 must enter pass0_used, i.e. EITHER some allocno holding
 *   $v0 must conflict with 139, OR a later-allocated allocno must *prefer* $v0
 *   (global.c set_preference only fires on a reg-reg copy where one side is
 *   already hard-numbered, so the preference route needs a locally-allocated
 *   $v0 pseudo copied into a global allocno).
 *   The cheap version of the conflict route is MEASURED DEAD: making the sums a
 *   multi-write global allocno (d1/f1) does put the sum on $v0 and does fix rows
 *   111/112, but it adds one allocno to the ordered list and every
 *   lower-priority allocno shifts one register (complement $t5->$t6, factor
 *   $t3->$t4, rp $t2->$t3), costing more than the 11 insns it buys.  The target
 *   therefore does NOT have an extra global allocno there: its sums are LOCAL
 *   quantities that local-alloc seats on $v0 BEFORE global alloc runs, which is
 *   what makes $v0 a hard-reg conflict for the blue temp.  In our build the red
 *   sum is instead merged by local-alloc's combine_regs into r_src's quantity
 *   (pseudo 123, nrefs 12 / livelen 13, $a1) - that merge is the thing to break.
 *
 * TRIP TEST - 3 of the 14 (rows 127/128/130).  Target lh $v0,4($s0) /
 * lh $v1,6($s0) / mult $v0,$v1; ours lh $v1,4 / lh $v0,6 / mult $v1,$v0.  s22
 * measured that the register pair is POSITIONAL, not expression-driven:
 * `rect[3] * rect[2]` (h1) swaps the two offsets but leaves $v1 on the first
 * load and $v0 on the second; `rect[2]*rect[3] > j` (h3) and `(s32)rect[2]*...`
 * (h2) are byte-identical to the incumbent latch; re-reading `total` in the
 * latch (h4) costs 2.
 *
 * ORDINARY-C STATUS UNCHANGED.  The two F1 chain extenders are still present and
 * still require the 2026-07-01 grant plus their annotations; this body is NOT
 * submittable as-is.  `sum` itself is an ordinary named intermediate (a real
 * value, written twice and read twice, no annotation needed).
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
                            s32 r_ch;
                            s32 g_ch;
                            s32 b_shift;
                            src++;
                            rp = r_src * complement;
                            r_src = r * factor;
                            r_ch = ((rp + r_src) >> 15) & 0x1F;
                            gp = g_src * complement;
                            g_src = g * factor;
                            sum = gp + g_src;
                            g_ch = (sum >> 10) & 0x3E0;
                            px = ((u32)px >> 7) & 0xF8;
                            px = px * complement;
                            sum = px + b * factor;
                            b_shift = ((sum + g_src) - g_src) >> 5;
                            *dst = (pixel & 0x8000) | r_ch | g_ch | (b_shift & 0x7C00);
                        }
                    }
                    dst++;
                loop_check:
                    j++;
                } while (j != rect[2] * rect[3]);
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
