/* func_8003DE14 - MATCHING form (grind session 38, STRUCTURAL modality).
 *
 * SCORE 0 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float),
 * `sandbox func_8003DE14 --disable all`, rules_dropped 0.
 *
 * FLOOR HISTORY: 26 -> 16 (s21) -> 14 (s22) -> 12 (s23) -> 5 (s29) -> 4 (s31)
 * -> 2 (s32) -> 1 (s33-s35) -> 0 with a fresh multi-written `h` carrier (s36,
 * REJECTED by the Judge 2026-09-11 04:48) -> 2 with the ruling-sanctioned
 * `total` borrow (s36b/s37) -> 0 HERE, with NO carrier at all.
 *
 * WHAT CHANGED IN s38.  The latch carrier is GONE.  Sessions s32-s37 had been
 * attacking the last two rows (the two halfword loads of the inner loop bound)
 * by ESCAPING one of the two load pseudos out of its basic block, so that
 * local-alloc would skip it and global.c would seat it second.  Every carrier
 * that did this either violated the Judge's standing constraint (a fresh
 * multi-written local) or cost other rows (the `total` borrow loses the
 * emission-ORDER tie, because `birthing_insn_p` is literally
 * `reg_n_sets[dest] == 1` and `total` has two sets - s37 H37-1).
 *
 * s37 closed the no-carrier chassis in closed form instead (H37-3, a CLASS
 * kill): with both loads block-local the emission order is already the
 * target's, and the ONLY thing wrong is that `qty_compare_1`
 * (local-alloc.c:1669-1684) sorts the earlier-born rect[2] load SECOND,
 * because both loads die at the shared `mult` and the earlier birth means a
 * strictly larger `death - birth` denominator at equal `n_refs`.  s37 recorded
 * the exact escape condition - "a third in-block reference to the rect[2]
 * value" - and dismissed it as "an extra instruction the target does not
 * have".  It is NOT an extra instruction: s37's own H37-2 banked the
 * pass-ordering fact that `reg_n_refs` is computed in flow, which runs BEFORE
 * combine, so an algebraically-null detour that combine folds away still
 * raises the count.  This body spells that detour `+ rect[2] - rect[2]` on the
 * loop bound; the rect[2] pseudo goes from 6 weighted refs to 12, its priority
 * from 3 to 9 against the rect[3] load's 6, it sorts first, takes $v0, and the
 * latch becomes byte-exact at an unchanged 173 instructions.
 *
 * FAKE CONSTRUCTS PRESENT (3, all inside frozen SOTN-sanctioned families):
 *   (1) the bound's `+ rect[2] - rect[2]` detour -
 *       [[dead-store-fake-exception]] combine-foldable chain-extender clause
 *       (owner ruling 2026-07-01); zero emitted bytes verified (173 insns with
 *       and without).
 *   (2) the s21 `((s32)dst_buf + j) - j` extender on the LoadImage argument -
 *       same family, banked since s21 (7 pts; plain `(s32)dst_buf` is 7 worse).
 *   (3) `gm` - named intermediate for the green mask (s36, worth 15 pts: the
 *       same body with the mask inline scores 17).
 * The s36b/s37 `total` staging FAKE is REMOVED - this body needs no carrier.
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
                /* FAKE: the inner loop's bound is routed through the algebraically
                 * equivalent detour `+ rect[2] - rect[2]`, which combine folds back to the
                 * direct `rect[2] * rect[3]` with ZERO emitted bytes (173 build insns with
                 * and without it; verified against the target's 173).  Its only surviving
                 * effect is the extra reg_n_refs that flow.c records BEFORE the fold.
                 * Mechanism: local-alloc.c:1669-1684 `qty_compare_1` ranks the two
                 * block-local halfword loads of the bound by
                 * floor_log2(n_refs)*n_refs*size/(death-birth).  Both loads die at the
                 * shared `mult`, so the earlier-born rect[2] load has the strictly larger
                 * denominator: at the natural 2 refs each (weighted x3 for loop depth =
                 * 6) it scores floor_log2(6)*6/4 = 3 against the rect[3] load's
                 * floor_log2(6)*6/2 = 6, is sorted second, and is handed $v1 instead of the
                 * target's $v0.  The detour's two extra reads CSE onto the same pseudo, so
                 * flow counts 4 refs (weighted 12) and it scores floor_log2(12)*12/4 = 9 >
                 * 6, sorts first and takes $v0 - the target's map `lh $v0,4($s0)` /
                 * `lh $v1,6($s0)` / `mult $v0,$v1`.  MEASURED, not inferred:
                 * tmp/grind/func_8003DE14/s38/qty_win.log:623-624 prints
                 * `blk=11 ord=0 qty=0 reg1=147 birth=4 death=8 refs=12 got=2` and
                 * `ord=1 qty=1 reg1=150 birth=6 death=8 refs=6 got=3`; the same dump on the
                 * detour-free body (s37/qty_g6.log) prints refs=6/got=3 for reg1=147.  Same family and same
                 * mechanism as the s21 `((s32)dst_buf + j) - j` extender below
                 * ([[dead-store-fake-exception]] combine-foldable chain-extender clause,
                 * owner ruling 2026-07-01).  Lever-exhaustion:
                 * memory/grind/func_8003DE14/hypotheses.md s24-s37 - the latch's order and
                 * seats were driven to a closed form over 14 sessions (s32 block-locality,
                 * s33-s35 allocno-priority inequality, s36 the escaped-carrier rules, s37
                 * birthing_insn_p + the qty_compare_1 class kill that this detour is the
                 * measured answer to), across ~200 rejected spellings including every
                 * operand order, declaration order, for/while/do-while chassis, staged
                 * carrier and cross-block read site. */
                } while (j < rect[2] * rect[3] + rect[2] - rect[2]);
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
