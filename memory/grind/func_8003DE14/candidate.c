/* func_8003DE14 - candidate (grind session 23, REDERIVE modality).
 *
 * SCORE 12 / 173 insns on HEAD 2026-09-11 (post -mel, post -msoft-float).
 * FLOOR HISTORY: 26 (s13-s20) -> 16 (s21) -> 14 (s22) -> 12 (this session).
 *
 * WHAT CHANGED vs s22's d2.  The BLUE channel no longer has a `b_src` local at
 * all: `px` (the `pixel & 0xFFFF` carrier, dead after the green source shift)
 * is REUSED to carry both the blue source byte and the blue complement product:
 *
 *      px = ((u32)px >> 7) & 0xF8;      <- blue source, reusing px
 *      px = px * complement;            <- blue complement product, same carrier
 *      sum = px + b * factor;
 *
 * That is the sanctioned "variable reuse for codegen control" shape, and it
 * lands the target's t106/t107/t108 exactly (`andi $a0,$v0,0xF8` / `mult $a0,$t5`
 * / `mflo $a0`): the target keeps the blue source AND its product in px's own
 * register $a0, and only a C-level reuse of px produces that.
 *
 *   ABLATION on this chassis (every form 173 build / 173 target):
 *      d2   s22 incumbent (separate `b_src` local) ................ 14
 *      p1   px reused for the blue SOURCE only ................... 12
 *      p2   px reused for the source AND the product (this) ...... 12   <- kept
 *      p3   source+product folded into one statement ............. 14
 *      p4   px also carries the blue SUM ......................... 23
 *      p5   sum carries the product, px the sum .................. 46
 *   p2 is kept over p1 because p1 still emits `mflo $v0` at t108 (one extra
 *   wrong row absorbed by the edit distance); p2 makes t106-t108 byte-exact and
 *   leaves a strictly smaller, cleaner residual.
 *
 *   NOTE FOR THE LEDGER: s22 banked "px reused as the blue carrier" as KILLED
 *   (forms b1/c1, 43 and 15) - but that was measured on the u3 and a2 chassis,
 *   BEFORE d2's shared `sum` local existed.  The kill was instance-scoped and it
 *   is now VOID: on the d2 chassis the same construct is worth -2.
 *
 * THE RESIDUAL IS ONE REGISTER SEAT - all 12 rows.
 *   t111/t112  addu $v0,$t2,$a1 / sra $a1,$v0,15   (ours: addu $a1,$t2,$a1 ...)
 *   t114/t115  addu $v0,$t1,$v1 / sra $v1,$v0,10   (ours: addu $v1,$t1,$v1 ...)
 *   t117-t119  mflo $t7 / addu $v0,$a0,$t7 / sra $a0,$v0,5
 *   t123/t124  andi $v1,$a0,0x7C00 / or $v0,$v0,$v1
 *   t127/t128/t130  lh $v0,4($s0) / lh $v1,6($s0) / mult $v0,$v1
 *   In the TARGET $v0 is a free scratch across the whole inner-loop block: each
 *   channel sum is born in $v0 and dies one insn later, and the latch's first
 *   load takes $v0 too.  In OUR build $v0 is OCCUPIED for the whole block by ONE
 *   global allocno, and every value below it shifts one seat.
 *
 *   PASS ATTRIBUTION (read out of tmp/grind/func_8003DE14/dumps/*.lreg for this
 *   exact body, s23 - not inferred):
 *     - The $v0 occupant is the `b * factor` mulsi3 result (insn 253, reg 138):
 *       "Register 138 used 6 times across 9 insns in block 10; pref LO_REG, else
 *       GR_REGS".  Because reg_preferred_class is LO_REG and
 *       CLASS_LIKELY_SPILLED_P(LO_REG) is true, local-alloc.c:472 sets
 *       reg_qty = -1 for it, so local-alloc NEVER seats it; it becomes a global
 *       allocno (BB2_ALLOC_DEBUG ord=10, nrefs=6, livelen=10, pri=12000) and
 *       global.c's find_reg first-fit hands it regno 2.
 *     - The same line explains why the sums combine with their sources: r_src,
 *       g_src, px and pixel all print "dies in 2 places" (they are the reused
 *       C variables), so reg_n_deaths != 1 and they too are global allocnos;
 *       the channel sums are the only LOCAL quantities in sight, and they end up
 *       on whatever local-alloc has free - never $v0, because the global
 *       allocno's block-level liveness has already claimed it by the time the
 *       seats are printed.
 *   THE ONE THING TO BREAK: get regno 2 out of the b*factor allocno's reach,
 *   either by a LOCAL quantity that local-alloc seats on $v0 and that overlaps
 *   insns 253-260, or by dropping that allocno's priority below the allocnos
 *   that would then take $v0.  Naming `b * factor` as a C local does NOT do it -
 *   it turns the pseudo into an ordinary local (dies once, no LO_REG pref) and
 *   the whole arm re-seats: s1 47, s2/s3/s4 41, s5 49.
 *
 * ORDINARY-C STATUS UNCHANGED.  The two s21 F1 chain extenders are still
 * present and still load-bearing; this body is NOT submittable as-is.
 *   `((sum + g_src) - g_src)`  - the g_src price extender
 *   `((s32)dst_buf + j) - j`   - the j price extender
 * `sum` remains an ordinary named intermediate; the px reuse is the sanctioned
 * variable-reuse family and needs no annotation.

 * [s24 CORRECTION - read before spending the pass attribution above]
 *   (1) The engine's 173/173 counts are objdump ZERO-ELIDED counts.  Both this
 *       object and the reference are 0x2cc = 716 bytes = 179 instructions; the
 *       three `nop; nop` pairs after the mflo's already match.  Use
 *       tmp/grind/func_8003DE14/s24/ed2.py (objdump -dz, 1:1 aligned) to read
 *       the residual - s23's sxs.py mis-aligns by 6 rows from t97 onward.
 *   (2) The residual is exactly 12 rows and they are 4 seats of ONE shape: the
 *       target routes each channel sum (and the blue mask, and the latch's
 *       first load) through a FRESH register and writes the final value back
 *       into the operand's register one insn later; ours collapses both steps
 *       onto the operand's register.
 *   (3) BB2_ALLOC_DEBUG on THIS body shows pseudo 138 (b*factor) is seated in
 *       LO (hardreg 65) by global.c's first pass, ord=9 pri=13333; it reaches
 *       $v0 only via reload's retry_global_alloc (.greg "Register 138 now in
 *       2").  And BB2_QTY_DEBUG shows four block-10 LOCAL quantities already
 *       get $v0.  So "no local quantity can reach $v0" is false, and the
 *       find_reg first-fit story in the paragraph above names the wrong pass.

 * [s25 ENUMERATE - the arm's spelling space is swept out]
 *   1,496 complete bodies (9 axes: carrier choice per channel, which values are
 *   named locals, channel block order, src++ position) all bottom at THIS score,
 *   12, with the SAME 12 residual rows; 8 of them tie.  Held values: r*factor
 *   must reuse r_src (fresh 28), g*factor must reuse g_src (fresh 31), order must
 *   be RGB (GRB 28 / GBR 30), the blue source must reuse px (fresh 14), and the
 *   green AND blue sums must share one `sum` local (inline/fresh 15).  FREE at the
 *   floor: the blue complement product may take a fresh `bp`, the red sum may be a
 *   fresh named local, and src++ may sit anywhere in the arm.  Also swept: 20
 *   spellings of the loop bound (all 12; `while (j < total)` loses the re-read and
 *   builds 3 insns short, so the target re-reads rect[2]/rect[3] at the latch).
 *   And the .loop dump shows NO LICM divergence (loop.c calls all three *factor
 *   hoists "not desirable" in our build too).  The residual is therefore NOT in
 *   this arm's spelling - look at block structure / declaration scope / the
 *   allocator question in hypotheses.md's s26 frontier.

 * [s26 SYNTHESIS - the residual is a combine_regs CHAINING difference]
 *   Chassis re-measured today: this body is 12 / 173 build / 173 target.
 *   (1) KILL RE-AUDIT of the two F1 extenders, re-measured on this chassis:
 *       both present 12 | j removed 19 | g_src removed 40 | neither 38.  Both
 *       are still load-bearing, the ordinary-C floor of this chassis is 38, and
 *       the two INTERACT with opposite sign (removing only the g_src extender is
 *       worse than removing both), so never quote a single-lever delta again.
 *   (2) s24 frontier item 1 is ANSWERED: the sum-to-operand tie is
 *       local-alloc's combine_regs (local-alloc.c:1295 -> :1784) merging pseudos
 *       into ONE quantity, not find_free_reg scan order.  Block 10 has 21
 *       pseudos, 16 local-alloc-eligible, but only 7 real quantities; qty18
 *       prints refs=24 while its reg1=152 has only 6 refs - a four-pseudo merge.
 *   (3) local-alloc.c:470-476 is the gate: NONE of the arm's six products is a
 *       local quantity (the three complement products are "pref LO_REG" =
 *       CLASS_LIKELY_SPILLED_P; the three *factor products live in reused C
 *       variables that "die in 2 places").  Every local quantity in block 10 is
 *       a DERIVED value, and the three wrong seats ($v1/$a0/$a1) are exactly the
 *       three MULTI-pseudo merged quantities qty16/qty17/qty15.  Short
 *       single-pseudo quantities (qty0/5/10) ALREADY get $v0.
 *   (4) Two more spelling axes swept out at 12: the output-word or-chain (11
 *       bodies, min 12, only a named `hi = pixel & 0x8000` ties; associativity
 *       changes cost up to +33) and the channel-RESULT carriers (8 bodies, min
 *       12, every reuse costs +4 to +25) - the latter was the direct C attempt at
 *       the reg_n_deaths!=1 route and it does not pay here.
 *   => The lever for s27 is BLOCK STRUCTURE (make a sum live across a block
 *      boundary), not spelling.  See hypotheses.md's s27 frontier.
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
