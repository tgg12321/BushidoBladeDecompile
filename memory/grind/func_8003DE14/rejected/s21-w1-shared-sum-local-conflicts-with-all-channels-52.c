/* func_8003DE14 - s18 (forensics) chassis: HEAD REGION BYTE-EXACT, score 28.
 *
 * NOT the incumbent (the incumbent candidate.c is 26).  This body is banked
 * because it is the FIRST form in 18 sessions whose head region - the 4-insn
 * residual that has been on the frontier since s13 - is structurally identical
 * to the target, byte for byte except for ONE register-name swap.
 *
 * The only change vs candidate.c (s13 body) is that `s32 j = 0;` is hoisted out
 * of the `if (total > 0)` guard and written as a sibling declaration-initialiser
 * of the per-outer-iteration locals, immediately after `factor`.  Ordinary C.
 *
 * WHAT IT BUYS (measured, tmp/grind/func_8003DE14/s18/h1.txt vs asm/funcs):
 *   target 55-57 : mult v1,v0 / addiu a3,sp,0x10 / addiu a2,sp,0x410
 *   h1     2514-251c: mult v1,v0 / addiu a3,sp,16 / addiu a2,sp,1040     MATCH
 *   target 73-77 : mflo t3 / blez v1 / [slot] addu t4,zero,zero / subu t5,fp,t3
 *                  / addiu v0,s2,-1
 *   h1     2554-2564: mflo t3 / blez v1 / [slot] move t5,zero / subu t4,s8,t3
 *                  / addiu v0,s2,-1                        MATCH except t4<->t5
 * The incumbent instead emits `addiu a2,sp,1040` in the blez delay slot and
 * has NO addiu a2 at target row 57 - a 4-insn residual.
 *
 * WHAT IT COSTS: the j <-> complement hard-register seats invert.  h1 seats
 * j on $t5 and complement on $t4; the target (and the incumbent) seat j on $t4
 * and complement on $t5.  complement is read 3x in the blend arm
 * (mult a1,t5 / mult a0,t5 / mult v0,t5 in the target) and j twice in the latch,
 * so the swap costs ~6 rows and the net is 28 vs 26.
 *
 * THE WHOLE REMAINING HEAD QUESTION IS THEREFORE ONE global.c PRIORITY COMPARE
 * (BB2_ALLOC_DEBUG, tmp/grind/func_8003DE14/s18/alloc_h6/stderr.log):
 *   ord=15 pseudo=116 (complement) hardreg=12 ($t4) nrefs=11 livelen=54 pri=6111
 *   ord=16 pseudo=115 (j)          hardreg=13 ($t5) nrefs=11 livelen=59 pri=5593
 * Allocation order is by descending priority and each allocno takes the lowest
 * free hard register, so whichever of the pair is priced higher gets $t4.  In
 * the incumbent, j is defined INSIDE the guard, 5 insns later, which makes
 * livelen(j) < livelen(complement) and hands j $t4.  Hoisting j to buy the
 * delay slot lengthens j's live range past complement's and loses the seat.
 *
 * TO FLIP IT (arithmetic, pri = floor_log2(nrefs)*nrefs*10000/livelen):
 *   - nrefs(j) >= 14 at livelen 59  => 3*14*10000/59 = 7118 > 6111.  reg_n_refs
 *     is loop-depth weighted (flow.c), so ONE extra read of j at inner-loop
 *     depth 3 is worth +3 and is enough.  s18 found no byte-neutral spelling of
 *     such a read: `if (j < total)` / `if (j < rect[2]*rect[3])` as the guard
 *     both score 45 (they defeat the blez and emit slt).
 *   - or livelen(j) <= 54.  j's def is already the last insn of the head block
 *     (it is what reorg puts in the slot) and its last use is the latch, so the
 *     range cannot be shortened without moving the def back inside the guard,
 *     which is the incumbent.
 *   - or livelen(complement) >= 60.  complement's def is already the first insn
 *     of the preheader and its last use is the b-channel product, 3 insns before
 *     j's last use; the blend arm's statement order cannot push it 6 insns later
 *     (the b product must precede the b*factor overwrite of b_src).
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
                            s32 sum;
                            src++;
                            rp = r_src * complement;
                            r_src = r * factor;
                            sum = rp + r_src;
                            r_ch = (sum >> 15) & 0x1F;
                            gp = g_src * complement;
                            g_src = g * factor;
                            sum = gp + g_src;
                            g_ch = (sum >> 10) & 0x3E0;
                            bp = b_src * complement;
                            b_src = b * factor;
                            sum = ((bp + b_src) + g_src) - g_src;
                            b_shift = sum >> 5;
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
            LoadImage((s32)rect, ((s32)dst_buf + j) - j);
            DrawSync(0);
            i++;
        } while (i < count);
    }
}
