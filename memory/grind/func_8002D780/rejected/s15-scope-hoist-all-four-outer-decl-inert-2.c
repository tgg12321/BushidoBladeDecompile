/* func_8002D780 - grind candidate (s14 rederive, 2026-09-08).  Honest sandbox floor
 * 2/202, build_insns == target_insns == 202, measured THIS session with these exact
 * edits in src/code6cac_b.c (`sandbox func_8002D780 --disable all` -> 2).
 *
 * WHAT CHANGED IN s14.  Two things.  (1) The coupled sibling func_8002E6B0 reached
 * COMPLETED-C (src/code6cac_b.c:1332-1364) and it is the SAME point-in-triangle predicate
 * this function inlines with one vertex at the origin; its matched spelling names the two
 * edge differences per test (`s32 dz = ...; s32 dx = ...;`) and leaves the centroid/query
 * differences inline.  Transplanting that spelling here scores 2/202 with the residual on
 * the DELAY-SLOT pair instead of the block-7 pair (tmp/grind/func_8002D780/s14/
 * pairdiff_a_sibling_dz_dx.txt).  (2) The body below adopts the sibling's `dx` naming on
 * top of the s12 ax/dz/az chassis, which keeps the s12 residual (strictly the more
 * tractable of the two) while removing the duplicated inline `(x2 - x0)` subexpression, so
 * all four block-7 differences are ordinary once-written named CSE locals.
 *
 * THE RESIDUAL (unchanged from s12, tmp/grind/func_8002D780/s14/pairdiff_n1_ax_dz_az_dx.txt):
 *
 *   ours[96] subu v0,a2,a3 (az)      target[96] subu v1,t5,t1 (dx)
 *   ours[97] subu v1,t5,t1 (dx)      target[97] subu v0,a2,a3 (az)
 *   === 2 differing instructions ===
 *
 * Everything else in the function - the test-2 delay slot, every register in every block,
 * both mult operand orders - is already the target's.
 *
 * WHY THE FOUR DECLARATIONS ARE IN THIS ORDER (s14 re-attribution; the s12 header's
 * "sched2 ready-list" story is WRONG and is corrected here from the dumps).  sched1
 * already emits az before dx (tmp/grind/func_8002D780/s14/w_n1/code6cac_b.sched, insns
 * 179 ax, 182 dz, 191 mult1, 185 az, 188 dx, 193 mult2) and sched2 does not move them:
 * rank_for_schedule finds equal INSN_PRIORITY and equal dependence class against the mult
 * (RANKDBG last=193 y=188 cls=3 x=185 cls2=3 val=0 in w_n1/stderr_full.txt) and falls
 * through to INSN_LUID (sched.c:2464), i.e. to SOURCE ORDER.  So the emission order of the
 * az/dx pair is exactly the declaration order.
 *
 * And the declaration order is also what buys the register seats, which is the trap.
 * local-alloc's block-7 quantity table (BB2_QTY_DEBUG, w_n1 vs w_n6/stderr_full.txt):
 *   this body (az 3rd, dx 4th):  dz reg130 birth 4  death 16 refs 3 -> pri 2500, got $a0
 *                                dx reg132 birth 10 death 20 refs 3 -> pri 3000, got $v1
 *   target order (dx 3rd, az 4th): dz birth 4 death 16 -> 2500 ; dx birth 8 death 20 -> 2500
 * i.e. in the target's OWN emission order the two quantities tie exactly on
 * qty_compare_1 (local-alloc.c:1708-1719, which has no tie-break of its own), qsort leaves
 * them in quantity-number order, dz is seated first and takes $v1 while dx takes $a0 - the
 * reverse of the target, and the score goes 2 -> 9 (rejected/s14-decl-order-dx-before-az-
 * seats-swapped-9.c).  The `az` declaration sitting between dz's and dx's definitions is
 * the ONLY thing measured so far that shortens dx's live range enough to break that tie,
 * and it costs exactly the two instructions above.
 *
 * s14 also killed the obvious way out: naming or hoisting the kp/kc difference
 * subexpressions (px - x0, pz - z0) to change the birth/death arithmetic does nothing at
 * all, because sched1 sinks every difference back to just before its consuming mult.  Six
 * spellings, all 9/202, and the dumped block-7 birth/death/refs quadruples are BYTE
 * IDENTICAL between the plain target-order body and both hoisted bodies (w_n6, w_w2b,
 * w_w2c: dz 4/16/3, dx 8/20/3 in all three).  See rejected/s14-w2-*.c.
 *
 * The rest of the body is unchanged from s5/s10 and every line of it is load-bearing; see
 * candidate_alt_s5_named_locals.c for the LZCS/LZCR island split, the `m` carrier and the
 * clobber footprint.  Summary of what must not change:
 *  - the two cop2 islands in the sqrt block must stay SPLIT;
 *  - `s32 m = dist;` must be declared BEFORE `s32 lzcr` and re-stored inside the
 *    `dist >= 0` arm (the annotated FAKE construct below; worth 3 insns, s14 re-ablation);
 *  - the three cop2 blocks are the owner-authorized canonical LZCS/LZCR + mvmva idiom
 *    (.claude/rules/cop2-addressing-preamble-cluster.md).
 *
 * FAMILY NOTE for whoever reaches 0: this body carries one annotated construct, the
 * same-value re-store of the local `m` (dead-store family,
 * .claude/rules/dead-store-fake-exception.md, in-TU byte-matched precedent at
 * src/code6cac_b.c:1244-1265).  The four test-3 locals ax, dz, dx, az all hold REAL values
 * that appear in the target bytes; dz and dx are written once and read twice (ordinary CSE
 * variables, and exactly the sibling func_8002E6B0's matched spelling), while ax and az are
 * written once and read once, which is the named-intermediate shape described in
 * .claude/rules/narrow-byte-args-packed-call.md plus the 2026-08-17 clarification in
 * .claude/rules/no-new-park-categories.md.  A candidate-ready session must decide whether
 * the once-read pair needs the named-intermediate FAKE annotation (six prongs) or is
 * ordinary C, and should file a ruling-request if the answer is not clean. */
s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
    if (flag == 0) {
        s32 *vin;
        s32 *vout;
        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];
        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];
        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];
        vin = (s32 *)(obj + 0xF8);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "lwc2 $0, 0($t4)\n"
            "lwc2 $1, 4($t4)\n"
            "nop\n"
            "nop\n"
            ".word 0x4A486012"
            : : "r"(vin) : "$12", "memory");
        vout = (s32 *)(obj + 0x100);
        __asm__ volatile(
            "addu $t4, %0, $zero\n"
            "swc2 $25, 0($t4)\n"
            "swc2 $26, 4($t4)\n"
            "swc2 $27, 8($t4)"
            : : "r"(vout) : "$12", "memory");
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        if (y < -threshold || threshold < y) return 0;
    }

    {
        s32 x0 = *(s32 *)(obj + 0xA8);
        s32 x2 = *(s32 *)(obj + 0xB8);
        s32 z0 = *(s32 *)(obj + 0xAC);
        s32 z2 = *(s32 *)(obj + 0xBC);
        s32 cx = (x0 + x2) / 3;
        s32 cz = (z0 + z2) / 3;
        s32 px = *(s32 *)(obj + 0x100);
        s32 pz = *(s32 *)(obj + 0x104);
        s32 ax, dz, az, dx;
        s32 kc = z0 * cx - x0 * cz;
        s32 kp = z0 * px - x0 * pz;

        if ((kc ^ kp) >= 0) {
            kc = z2 * cx - x2 * cz;
            kp = z2 * px - x2 * pz;
            if ((kc ^ kp) >= 0) {
                ax = cx - x0;
                dz = z2 - z0;
                az = cz - z0;
                dx = x2 - x0;
                kc = (dz * ax) - (dx * az);
                kp = (dz * (px - x0)) - (dx * (pz - z0));
                if ((kc ^ kp) >= 0)
                    return 1;
            }
        }
    }

    {
        s32 y = *(s32 *)(obj + 0x108);
        s32 sp_var;
        s32 dist = r_sq - y * y;
        s32 sqrt_val;
        s32 *p118;
        s32 *p124;
        s32 *p10C;

        if ((u32)dist < 0x400) {
            sqrt_val = (u32)*((&D_8008D118) + dist) >> 3;
        } else {
            s32 m = dist;
            s32 lzcr = 0;
            if (dist >= 0) {
                /* FAKE: same-value re-store of the local `m`, mechanism: cse.c make_regs_eqv
                 * (a single-definition copy is folded; a second definition keeps the pseudo
                 * multiply-defined so the copy survives into local-alloc), lever-exhaustion:
                 * memory/grind/func_8002D780/hypotheses.md s1-s5, 14 copy spellings dead. */
                m = dist;
                __asm__ volatile(
                    "addu $t4, %0, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop"
                    : : "r"(m) : "$12");
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(&sp_var) : "$12", "$13", "$14", "$15");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *((&D_8008D118) + ((u32)m >> shift));
                sqrt_val = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        p118 = (s32 *)(obj + 0x118);
        p124 = (s32 *)(obj + 0x124);
        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);
        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);
        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);
        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p118, p124) != 0) return 1;

        p10C = (s32 *)(obj + 0x10C);
        p10C[0] = -*(s32 *)(obj + 0x100);
        p10C[1] = -*(s32 *)(obj + 0x104);
        if (func_8002D518(sqrt_val, dist, p10C, p118) != 0) return 1;

        if (func_8002D518(sqrt_val, dist, p10C, p124) != 0) return 1;
        return 0;
    }
}
