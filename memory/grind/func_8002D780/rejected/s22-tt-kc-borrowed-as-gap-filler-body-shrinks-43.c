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
/* s15 (2026-09-08, enumerate) - body UNCHANGED, floor re-measured 2/202 on HEAD e3895bb7.
 * Two exhaustive sweeps ran against this exact chassis and neither found anything below 2:
 *   - 2,080 in-block spellings of the test-3 region (name/inline x declaration order x
 *     kc/kp order x commutative operand swaps): 41 at 2, nothing lower. In-tree
 *     confirmation of the operator's out-of-tree 62,624-spelling class kill.
 *   - 816 DECLARATION-SCOPE variants (every subset of ax/dz/az/dx hoisted to the enclosing
 *     block as an uninitialised decl, values still assigned inside block 7, all outer and
 *     inner orders): 412 at 2, 204 at 4, 204 at 9 - the SAME 2:1:1 trichotomy at every
 *     hoist level, i.e. declaration scope is inert. An uninitialised decl emits no RTL, so
 *     the pseudo's birth/quantity order/LUID are still set by the assignment insn
 *     (local-alloc.c:1708 qty_compare_1). This closes the axis the class-kill memory named
 *     as the next instrument.
 * Do NOT re-enumerate the test-3 block or the scope axis. See memory/grind/func_8002D780/
 * evidence.md and hypotheses.md s15; the remaining region is the outer centroid/test-1
 * declaration list (16,384 valid orders, marked body at tmp/grind/func_8002D780/s15/
 * enum_base3.c). */
/* s16 (2026-09-08, structural) - body UNCHANGED, floor re-measured 2/202 on HEAD 82ab11bd.
 * s16's result is a complete MODEL of what is left, not a new spelling. Block 7 emits four
 * subus; sched1 groups them by which mult they feed (ax,dz -> mult1; dx,az -> mult2) and
 * within each pair falls through to INSN_LUID = source order (sched.c:2464). So the score
 * of any declaration order is a pure function of TWO BITS, measured exhaustively over all
 * 24 orders (tmp/grind/func_8002D780/s16/va.json + the twelve pairdiff_*.txt):
 *     ax<dz   dx<az   score   residual
 *      no      no       4     BOTH pairs transposed, all registers correct
 *      no      yes      2     ours[92:93] ax/dz swapped (the reorg delay-slot pair)
 *      yes     no       2     ours[96:97] dx/az swapped   <-- THIS BODY
 *      yes     yes      9     both emission orders correct, but the seats swap
 * The target is the (yes,yes) quadrant. It costs 9 rather than 2 because dz and dx then
 * tie EXACTLY in qty_compare_1 (local-alloc.c:1708) - 3 refs each, span 12 each, dz born 4
 * LUIDs earlier and dying 4 LUIDs earlier - so qsort seats dz first and $v1/$a0 swap
 * through the block. The entire remaining problem is: reach (yes,yes) and make dx beat dz
 * in qty_compare_1.
 * s16 killed five structural ways of trying: fresh kc/kp per test (inert alone, +34 for
 * both), hoisting the difference COMPUTATIONS out of block 7 (best 19; this is the axis
 * s15's uninitialised-declaration sweep could not reach), the block SHAPE of the three
 * tests (goto early-out and inner brace scope EXACTLY inert; hit-flag +6), the sign flip
 * of one product with a compensating `< 0` branch (best 21) and of both products (uniform
 * +22 in every shape), and PARTIAL inlining of one use to drop dz to 2 references (all
 * five modes exactly inert - cse refolds the duplicated subexpression). 528 variants;
 * every inert axis reproduces {2:12, 4:6, 9:6} byte-for-byte, which is itself the proof
 * that it never reaches the tie.
 * Do NOT re-run: the block-structure axis, the partial-inline axis, computation hoisting,
 * or any sign flip. See evidence.md / hypotheses.md s16. */
/* s17 (2026-09-08, synthesis) - body UNCHANGED, floor re-measured 2/202 on HEAD df6966b7.
 * s17 derived the residual end-to-end from local-alloc.c and the target asm instead of
 * from the ledger narrative, and the derivation is now complete: in the (T,T) quadrant
 * block 7 is 14 insns, dz is born at insn 2 and dies at insn 8, dx is born at insn 4 and
 * dies at insn 10, both have 3 references, so qty_compare_1 gives both exactly 2500 and
 * falls through to `return *q1 - *q2` (local-alloc.c:1719) - the QUANTITY number, handed
 * out by alloc_qty's next_qty++ (local-alloc.c:284) from block_alloc's forward insn scan
 * (local-alloc.c:1169-1175). In the (T,T) quadrant dz's subu always precedes dx's, so dz
 * always has the lower quantity number and always wins the seat. Only three levers can
 * change that: (L1) dx gets a 4th reference, (L2) dz's span grows by an insn placed
 * between insn 2 and insn 4, (L3) dx's last use moves to insn 9 or earlier.
 * s17 measured L3 dead (8 more kp-difference spellings, all exactly 9; 2 split-init forms
 * 32/36) and killed the last s16 frontier item, the outer declaration list, as a CLASS
 * kill (24 sampled orders, best 9, and the mechanism shows pseudo numbering is not an
 * input to any comparator in local-alloc.c) - so do NOT run the 16,384-variant sweep the
 * s15/s16 frontier proposed. A new axis, naming a PRODUCT rather than a difference to
 * reverse the multiply order, is inert on the first product and costs 25-32 on the second.
 * What is left is L1 and one unread artefact: block 7's four product pseudos do not
 * appear in the block-7 quantity table and have no QTYDBG-SUGG lines, so combine_regs has
 * already tied each of them into some other quantity. If the kp product can be made to
 * tie into dx's quantity, dx's refs go 3 -> 5 and its death extends to the kp subu:
 * priority 3125 vs dz's 2500, dx wins the seat, emission order untouched. See
 * evidence.md / hypotheses.md s17. */
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
        s32 kc = z0 * cx - x0 * cz;
        s32 kp = z0 * px - x0 * pz;

        if ((kc ^ kp) >= 0) {
            kc = z2 * cx - x2 * cz;
            kp = z2 * px - x2 * pz;
            if ((kc ^ kp) >= 0) {
                s32 ax = cx - x0;
                s32 dz = z2 - z0;
                kc = dz * ax;
                s32 dx = x2 - x0;
                s32 az = cz - z0;
                kc = kc - (dx * az);
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
