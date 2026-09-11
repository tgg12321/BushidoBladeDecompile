/* func_8003DE14 - candidate (grind session 13, structural modality).
 *
 * SCORE 26 / 173 insns on HEAD 2026-09-10 (post -mel, post -msoft-float).
 * This is the s12 body (score 28) with ONE structural change inside the blend
 * arm: the r/g/b channel sums are staged through named per-channel variables
 * and each `X_src` variable is REUSED to hold that channel's `X * factor`
 * product.  Ordinary C throughout - no annotation-bearing construct anywhere in
 * the body.
 *
 *   s12:  s32 r_src = (pixel & 0x1F) << 3;      (and g_src, b_src)
 *         r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
 *
 *   s13:  s32 r_src = (pixel & 0x1F) << 3;      (and g_src, b_src)
 *         s32 rp;                                (and gp, bp)
 *         rp    = r_src * complement;
 *         r_src = r * factor;                    <- same C variable reused
 *         r_ch  = ((rp + r_src) >> 15) & 0x1F;
 *
 * WHY IT WORKS.  In GCC 2.7.2 a non-address-taken C local is exactly ONE pseudo
 * for its whole scope, so writing a second value into `r_src` forces the
 * shifted source component and the `r * factor` product to share one hard
 * register.  That is precisely what the target does: target insns 94/95 are
 * `sll a1,v0,0x3 / mult a1,t5` and target insn 98 is `mflo a1` - one register
 * ($a1) carrying both r-channel values.  s12's body spelled the shifted
 * component and the product as two distinct expressions, so they became two
 * pseudos and landed in $v0 and $a1.  Making them one variable makes rows 94
 * and 95 byte-exact and drops the floor 28 -> 26.
 *
 * THE MOST IMPORTANT FINDING OF THIS SESSION (correcting the s12 frontier):
 * the blend arm's instruction ORDER is already byte-for-byte the target's.
 * s12's frontier claimed "the target runs the blue channel's srl/andi/mult
 * before its first mflo (104-107) where we run it after (109-112)" - that was a
 * difflib alignment artifact.  A raw index-by-index side-by-side
 * (tmp/grind/func_8003DE14/s13/sxs.py) shows target[84..131] and ours[84..131]
 * carry IDENTICAL opcodes in identical slots; only the register NAMES differ.
 * There is no scheduling problem in the blend arm and nothing for
 * tools/sched_solver to solve there.
 *
 * WHAT IS LEFT (26, tmp/grind/func_8003DE14/s13):
 *   (a) HEAD, 4 insns.  Target: `addiu a2,sp,1040` (dst = dst_buf) is emitted
 *       at insn 54 next to `addiu a3,sp,16` (src = src_buf), and the `blez`
 *       delay slot at 70 is filled with `move t4,zero` (j = 0) taken from the
 *       FALL-THROUGH side of the branch; `subu t5,s8,t3` (complement) follows
 *       at 71.  Ours: reorg pulls `addiu a2,sp,1040` into the slot instead and
 *       `move t4,zero` lands at 71.  Six statement-order spellings were
 *       measured this session (E1..E6) and none restores the target's slot
 *       without losing the j/complement seat.
 *   (b) BLEND ARM, 19 insns: pure register naming, all of it cascading from ONE
 *       seat.  The zero-extended pixel (`px`, pseudo 122, a GLOBAL allocno with
 *       nrefs 12 / livelen 11 / pri 32727, allocated 3rd) takes $v1 in our
 *       build and $a0 in the target; every later channel register is whatever
 *       the free pool hands out around that choice.  See the frontier below.
 *   (c) TRIP TEST, 3 insns (target 127-130): `lh v0,4(s0) / lh v1,6(s0) /
 *       mult v0,v1` against our `lh v1 / lh v0 / mult v1,v0` - same loads in
 *       the same order, the two short-lived pseudos named the other way round.
 *
 * EQUIVALENT SPELLINGS, all measured 26 this session (tmp/grind/.../s13):
 *   D1  - the `px` user variable deleted entirely (u16 `pixel` used directly,
 *         the zero-extend becomes a CSE temp).  Byte-identical output to C2:
 *         whether the zero-extend is a user variable or a compiler temp does
 *         NOT move px's seat.
 *   D5  - D1 with explicit (u32) casts on the two shifts.
 *   E4  - `total` computed after src/dst/factor.
 *   E5  - dst declared before src.
 *   E6  - `total` deleted, the guard written `if (rect[2] * rect[3] > 0)`.
 *   F4  - g_src / b_src declared lazily at their first use.
 *
 * S14 (structural) re-measured this body at 26 / 173 insns on HEAD 2026-09-10
 * and left it unchanged as the best known form.  What s14 added is the exact
 * arithmetic of the remaining seat (BB2_ALLOC_DEBUG, s14/d_aac/stderr.log):
 * px = pseudo 122, nrefs 12 / livelen 11 / pri 32727, allocated 3rd, takes $v1
 * because p118 (`count - 1`, pri 90000) conflicts with it and already holds
 * $v0.  g_src = pseudo 126 at pri 30000 follows and takes $a0.  The target has
 * them the other way round, and r_src (pseudo 123, $a1) is already correct.
 * To flip: g_src needs livelen <= 10 (pri 36000) or nrefs >= 14 (35000), or px
 * needs livelen >= 13 (pri 27692 - which ties r_src, and global.c:652-653 gives
 * the tie to the lower pseudo number 122, producing exactly the target order
 * g_src, px, r_src).
 *
 * S14 measured dead: the whole 3x3 per-channel product/reuse lattice (27 forms,
 * min 26 at this shape and at AAC), all six channel-block orderings, all six
 * `src++` placements (byte-identical - sched1 normalises it), a single shared
 * `sum` local for the three channel totals (53), and deriving the b source from
 * the g source's shift to cut a px reference (combine refolds it; identical
 * bytes).  Forms in tmp/grind/func_8003DE14/s14/.
 *
 * Chassis: HEAD 2026-09-10.  sandbox --disable all => score 26, build_insns 173,
 * target_insns 173.
 *
 * S15 (enumerate) re-measured this body at 26 / 173 on HEAD 2026-09-10 and left
 * it unchanged as the best known form.  s15 swept SIX exhaustive spelling
 * families, 3,966 valid spellings, ZERO hit (tmp/grind/func_8003DE14/s15/):
 *   - all 1680 interleavings of the blend arms nine channel assignments
 *     (best 26, 196 byte-identical ties) - INCLUDING the order the targets own
 *     instruction stream exhibits (all three products, then the three sums),
 *     which ties rather than beats;
 *   - all 588 head-region spellings (hoisting j = 0 and/or complement out of
 *     the if (total > 0) guard x every def-before-use order of the four
 *     per-iteration statements): best 26, and every hoisted-j spelling is 40+;
 *   - all 96 cursor declaration-site spellings (src/dst at function scope):
 *     best 26, hoisting either cursor costs 33 to 97 points;
 *   - 7 respellings of i == count - 1 (best 27) and 8 of the inner-loop latch
 *     (best 26, operand order inert);
 *   - the full 550-spelling no-reuse (SSA) blend lattice: best 28, i.e. the
 *     per-channel variable reuse in this body is worth 2 points that naming and
 *     ordering alone never buy.
 *
 * S16 (synthesis) re-measured this body at 26 / 173 on HEAD 2026-09-10 and left it
 * unchanged as the best known form.  s16 corrected the residual's pseudo map and
 * proved the target's seats are REACHABLE but not affordable:
 *   - p121 = pixel, p122 = px, p123 = r_src, p126 = g_src, p128 = b_src.  b_src is
 *     NOT a local quantity (s14's reading): it is the second allocno of the 32727
 *     tie and it is what holds $v0, which the target leaves to local-alloc.
 *     Target seats: g_src $v1, px $a0, b_src $a0 (after px dies), r_src $a1.
 *   - Declaration-site RENUMBERING (declaration split from assignment) is a real,
 *     byte-neutral lever: all 384 spellings score 26, and the dumps show the pseudo
 *     numbers and the 32727 pair's seats really do move.  It cannot decide px vs
 *     g_src because those two are never tied (32727 vs 30000).
 *   - The CROSS-channel carrier lattice (1,483 liveness-checked spellings, alloc
 *     tables for every one in s16/carrier_alloc.json): 4 forms hit three of the four
 *     target seats exactly, 108 forms tie px and g_src.  None pays - seat-correct
 *     forms score 38, tie+renumber forms 34, and the lattice minimum is the
 *     incumbent's 26.  Steering the seat re-prices sched1 and re-interleaves the arm.
 * The open question is now narrow: raise pri(g_src) above 32727 (livelen <= 10 at
 * nrefs 12, or nrefs >= 15 at livelen <= 13) WITHOUT touching the AAA value->variable
 * mapping - which means the change has to come from outside the blend arm.
 *
 * S17 (solver) re-measured this body at 26 / 173 on HEAD 2026-09-10 and left it
 * unchanged as the best known form.  s17 ran the full solver triage and closed
 * the SCHEDULER axis outright:
 *   - inverse_compose.py classify (object mode, target = build/src/code6cac_c2.o,
 *     ours = the cheat-stripped sandbox .o) returns FIRST DIVERGENCE: RA.  The
 *     register-BLANKED instruction multisets of the two 173-insn streams are
 *     IDENTICAL, so there is no pre-RA (front end / cse / combine / loop) insn
 *     difference anywhere in the function - every one of the 26 points is a
 *     register name or a placement, never a different instruction.
 *   - tools/sched_solver (model extracted for code6cac_c2 at parity=True, 750
 *     blocks / 3989 picks) with the goal derived from the TARGET OBJECT reports
 *     exactly ONE block of func_8003DE14 whose goal differs from ours in either
 *     pass: block 10, the blend arm - and it reports that goal as NOT a
 *     topological order (8 violations in pass 2, 3 in pass 1), i.e. the target
 *     alignment mis-paired duplicate instruction text.  That is the same difflib
 *     artifact s13 identified by raw index-by-index comparison; there is no real
 *     order divergence in the blend arm.  Every other block - INCLUDING the
 *     outer-loop head block that carries the 4-insn blez-delay-slot residual -
 *     already schedules to the target's order in both sched1 and sched2.
 *   - Consequence for the s16 frontier: the head residual is NOT a sched1
 *     INSN_PRIORITY question (that probe is answered and dead), and the blend
 *     arm's interleaved cursor-bump / trip-test insns cannot be moved by any
 *     scheduler lever because our schedule already IS the target's.  The head
 *     rotation (`addiu a2,sp,1040` at target row 54 vs our blez delay slot at
 *     row 69) is produced downstream of the scheduler, in reorg.c's delay-slot
 *     fill, which the sched model explicitly does not cover.
 *
 * TOOLING NOTE (s17, reusable).  tools/sched_solver/mkasm.sh cannot be used on
 * this function: it predates --prefill-label-funcs (2026-09-04) and it runs the
 * FULL prologue_fix, so its .hon.s is not the sandbox's source state.  And even
 * the correct honest stream trips goalmap's same-source checksum, because maspsx
 * emits mult/mflo interlock nop PAIRS that objdump renders as `...` and
 * engine.score.normalized_insns therefore drops from BOTH streams (179 text
 * lines vs 173 object insns; verified symmetric - target and ours both carry
 * them at 0x25c8/0x25cc).  tmp/grind/func_8003DE14/s17/mkasm3.py rebuilds the
 * streams through engine.pipeline.c_pipeline_cmd with the sandbox's own
 * overrides, and tmp/grind/func_8003DE14/s17/perturb2.py wraps perturb.py with
 * the one-line goalmap patch (a nop inside a RUN of nops expands to 0 object
 * insns).  Use those two, not mkasm.sh, for any further solver work here.
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
            if (total > 0) {
                s32 complement = blend_base - factor;
                s32 j = 0;
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
                            b_shift = (bp + b_src) >> 5;
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
            LoadImage((s32)rect, (s32)dst_buf);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
