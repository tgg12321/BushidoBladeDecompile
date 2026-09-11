/* func_8003DE14 - candidate (grind session 12, structural modality).
 *
 * SCORE 28 / 173 insns on HEAD 2026-09-10 (post -mel, post -msoft-float).
 * This is the s11 body (score 29) with ONE structural change, and it drops the
 * floor by one AND retires the whole j/complement half of the residual:
 *
 *   s11:  s32 j = 0;                       <- in the outer do-body
 *         if (total > 0) {
 *             do {
 *                 s32 complement = blend_base - factor;   <- LICM-hoisted
 *
 *   s12:  if (total > 0) {
 *             s32 complement = blend_base - factor;       <- written where LICM
 *             s32 j = 0;                                     put it anyway
 *             do {
 *
 * WHY IT WORKS (pass attribution, BB2_ALLOC_DEBUG on tools/gcc-2.7.2/cc1,
 * tmp/grind/func_8003DE14/s12/d_W5/stderr.log):
 *   j (pseudo 115) and complement (pseudo 116) both carry nrefs 11, so
 *   allocno_compare (global.c:635-648) reduces to a pure live-length race and
 *   global.c:652-653 breaks an exact tie on ascending allocno.  Both pseudos are
 *   live across the whole pixel loop (each is used on every iteration and set
 *   only in the preheader), so their live LENGTHS differ by exactly the distance
 *   between their two set insns in the preheader - nothing inside the loop body
 *   can move either number.  LICM always appends a hoisted set at the END of the
 *   preheader (loop.c scan_loop emits before loop_start), so as long as
 *   complement's set is hoisted it is strictly BELOW `j = 0` and complement wins
 *   the race:  s11 measured j 59 / complement 54, i.e. complement seated first
 *   and taking $t4 where the target has j.
 *   Writing the subtraction inside `if (total > 0)` gives LICM nothing to hoist
 *   (it is already in the inner loop's preheader block) and lets ordinary
 *   statement order put `j = 0` BELOW it:
 *       ord=15 pseudo=115 hardreg=12 nrefs=11 livelen=54 pri=6111   (j -> $t4)
 *       ord=16 pseudo=116 hardreg=13 nrefs=11 livelen=55 pri=6000   (complement -> $t5)
 *   which is the target's seat.  Insns 70/71, 131 and 133 of the aligned diff
 *   are now byte-exact.
 *
 * ORDINARY C: the only change is where two locals are declared and initialised.
 * There is no annotation-bearing construct in this body at all.  (The inner
 * loop's three arm-local `dst++` copies are the s11 duplicated-statement shape,
 * .claude/rules/duplicated-statement-into-arms.md - unchanged from s11.)
 *
 * EQUIVALENT SPELLINGS, all measured 28 this session (tmp/grind/.../s12):
 *   W2  - `s32 j;` left in the outer do-body, `j = 0;` moved inside the if
 *   W7  - `s32 j;` declared at the top of the do-body
 *   H1  - dst declared before src
 *   H3  - total computed last in the outer body
 *   B1w/B3w/B7w/B8w - four of s11's blend reshapings on top of this chassis
 *
 * WHAT IS LEFT (28, sbs2.py aligned diff, tmp/grind/func_8003DE14/s12):
 *   (a) HEAD, 4 insns: the target initialises dst at insn 54 and fills the
 *       `blez` delay slot at 70 with `move t4,zero`; we sink `addiu a2,sp,1040`
 *       into that slot (reorg.c takes the closest movable insn before the
 *       branch, and `j = 0` is no longer that insn - it now lives below the
 *       branch).  This is the price of the flip and it is DIRECTLY COUPLED:
 *       H2 (j declared before complement inside the if) restores the target's
 *       delay slot and loses the seat, scoring 31.
 *   (b) BLEND ARM, target 88-123: register naming ($a0/$a1/$t7 vs our
 *       $v1/$a1/$v1) plus the blue channel's srl/andi/mult running at target
 *       104-107 where we run it at 109-112.  Twelve source reshapings were
 *       re-measured on THIS chassis (B1w..B12w) and none beat 28.
 *   (c) TRIP TEST, target 127-130: `lh v0,4(s0) / lh v1,6(s0) / mult v0,v1`
 *       against our `lh v1 / lh v0 / mult v1,v0` - the same two loads in the
 *       same order, with the two short-lived pseudos named the other way round.
 *
 * Chassis: HEAD 2026-09-10.  sandbox --disable all => score 28, build_insns 173,
 * target_insns 173.
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
                            s32 r_ch;
                            s32 g_ch;
                            s32 b_shift;
                            src++;
                            r_ch = ((r_src * complement + r * factor) >> 15) & 0x1F;
                            g_ch = ((g_src * complement + g * factor) >> 10) & 0x3E0;
                            b_shift = (b_src * complement + b * factor) >> 5;
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
