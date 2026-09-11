/* func_8003DE14 - MATCHING candidate (grind session 36, ENUMERATE modality).
 *
 * SCORE 0 / 173 build insns on HEAD 2026-09-11 (post -mel, post -msoft-float),
 * measured with `sandbox func_8003DE14 --disable all` (cheat-stripped honest
 * distance).  FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12
 * (s23-s28) -> 5 (s29, s30) -> 4 (s31) -> 2 (s32) -> 1 (s33-s35) -> 0 (this
 * session).
 *
 * WHAT CLOSED IT.  s35 proved the entire residual was one inequality in
 * global.c's allocno ordering: with the red source read taken off `pixel` (the
 * target's own shape) the blend block's three channel pseudos compete for
 * $a0/$a1/$v1, and px keeps $a0 iff pri(px) > pri(red), i.e. red must reach
 * live length 22 at its pinned 24 references while px stays at 30/27.  s35
 * reached that with an OR re-association (2/173, red's `or` emitted one slot
 * late) and with a cross-block hoist of the red source read (3/173, the `sll`
 * emitted pre-branch).  This session enumerated the OR-chain region
 * exhaustively instead of hand-spelling it: the region was written in
 * fully-named form (sign / gm / bm / sr / srg) between ENUM markers,
 * tools/spelling_enum.py generated all 104 inline/declaration-order spellings
 * and tools/sweep_variants.py scored them in one pass.  18 of the 104 score
 * ZERO.  The minimal member of that family - and the one kept here - names
 * exactly ONE intermediate:
 *
 *     gm   = g_src & 0x3E0;
 *     *dst = (pixel & 0x8000) | r_src | gm | (px & 0x7C00);
 *
 * i.e. the target's own OR order (sign, red, green, blue) with the green mask
 * named.  BB2_ALLOC_DEBUG rows (tmp/grind/func_8003DE14/s36/alloc.log):
 *     with gm:     green 127 18/16 pri=45000 -> $v1   (target)
 *                  px    123 30/27 pri=44444 -> $a0   (target)
 *                  red   124 24/22 pri=43636 -> $a1   (target)
 *     without gm:  red   124 24/21 pri=45714 -> $a0   (WRONG, 17/173)
 *                  px    123 30/27 pri=44444 -> $a1
 * Naming the mask ends green's live range one insn earlier and pushes red's
 * last reference one insn later; that is the single unit of live length the
 * s35 inequality needed, bought without moving any expression across a block
 * boundary (so the pre-branch block keeps the target's lone
 * `andi $v0,$t0,0x1F`, which reorg.c fills into the `bnez` delay slot).
 *
 * BORROWS DO NOT WORK (s36 wave x, all measured this session): routing the
 * mask through an existing dead local - gp (22), rp (24), sum (43) - or
 * writing it back into g_src itself, `g_src = g_src & 0x3E0;` (17) or
 * `g_src &= 0x3E0;` (17), all fail.  The carrier must be a FRESH pseudo, which
 * is precisely prong (4) of the named-intermediate entry.
 *
 * FAKE CONSTRUCTS PRESENT (3, all inside frozen SOTN-sanctioned families; see
 * memory/grind/func_8003DE14/self_vet.md for the per-construct vet):
 *   (1) `gm` - named intermediate (this session, 17 pts).
 *
 * THE OR CHAIN IS ORDINARY C, NOT AN ENUMERATED ORDER.  The kept spelling is
 * plain left-to-right `sign | red | green | blue` with NO parentheses - the
 * natural channel order (the same order as the function's own r/g/b locals and
 * color_info[0..2]), and the order the target's own bytes were emitted in
 * (`andi $v0,$t0,0x8000` / `or $v0,$v0,$a1`(red) / `or $v0,$v0,$v1`(green) /
 * `andi $v1,$a0,0x7C00` / `or $v0,$v0,$v1`, asm/funcs/func_8003DE14.s:134-139).
 * [[or-tree-shape-shift]] "What IS allowed" lets a worker freely choose any
 * natural ordering; the parenthesised `(((v|R)|G)|B)` form the enumerator
 * emitted was verified byte-identical to the paren-free form, so no
 * non-natural grouping is committed and the 2026-08-20 carve-out is not needed.
 *
 * OPEN RULING QUESTION (why this session did NOT return candidate-ready).
 * The inherited `h` local (s32) is a FRESH local used as a staging carrier at
 * TWO sites - `h = target_color; *dst++ = h;` in the fast arm and
 * `while (j < rect[2] * (h = rect[3]))` in the latch - with BOTH staged values
 * real and immediately consumed (the latch assignment's value is the
 * multiplicand).  Zero dead code.  It is excluded from
 * [[staged-value-reused-variable]] by bound 2 (the carrier must be an EXISTING
 * variable; inventing one is not that family) and from the named-intermediate
 * entry by its multi-WRITE property (no-new-park-categories.md:204 ff).  It is
 * load-bearing: every alternative measured this session scores 2-43 (borrowing
 * the genuinely-dead existing `total` = 2; splitting h into two once-written
 * locals = 3; dropping either reference = 3; nine ordinary latch-expression
 * spellings = 3-5; hoisting sum/rp/gp/px/g_src/r_src to carry the latch =
 * 3-100).  The session therefore returns `ruling-request`; the body below is
 * the exact form to submit if the ruling allows `h`.
 *   (2) `h`  - staged value through a single local (s32, 3 pts; both of its
 *       references are load-bearing: dropping either measures 3/173).
 *   (3) the s21 `((s32)dst_buf + j) - j` chain extender (7 pts; plain
 *       `(s32)dst_buf` measures 7/173).
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
