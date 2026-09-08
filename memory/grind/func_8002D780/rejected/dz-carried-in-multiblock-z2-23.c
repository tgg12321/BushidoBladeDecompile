/* func_8002D780 - grind candidate (s5 synthesis, 2026-09-08). Honest sandbox floor
 * 2/202 (s3/s4 left it at 7/202). Measured THIS session with these exact edits in
 * src/code6cac_b.c: `sandbox func_8002D780 --disable all` -> score 2, and
 * build_insns == target_insns == 202. Diff: tmp/grind/func_8002D780/s5/pairdiff_f4.txt
 * (the 5-floor intermediate) and the batch-G measurements in evidence.md [s5].
 *
 * What moved the floor this session (7 -> 5 -> 2):
 *  1. 7 -> 5, residual B (the `dist` copy) CLOSED.  Twenty-three copy spellings were
 *     dead across s1-s4 and every one of them was a SINGLE-DEF copy, which is exactly
 *     what cse.c make_regs_eqv folds.  The carrier `m` here is MULTIPLY-DEFINED (its
 *     declaration plus the same-value re-store inside the `dist >= 0` arm), which is
 *     the mechanism the byte-matched sibling func_8002D518 uses for the identical
 *     residual at src/code6cac_b.c:1244-1265.  Two things beyond the sibling's shape
 *     were load-bearing and cost s5 two batches to find:
 *       - the carrier must be DECLARED BEFORE `lzcr` (`s32 m = dist; s32 lzcr = 0;`).
 *         With `lzcr` first the copy folds again (measured d1 = 7); with the carrier
 *         first the copy survives, and reorg.c puts it in the `beqz` delay slot exactly
 *         as the target does (`move a0,s1`), leaving `move v1,zero` for the `bltz`
 *         slot.  Declaration order picks WHICH of the two insns reorg steals.
 *       - the second definition must sit in the SAME arm (not in an `else` arm): an
 *         `else { m = dist; }` also defeats cse but costs an extra `j` + a duplicated
 *         `li v0,-2` (204 insns, score 9, variant e1).
 *     `srlv v0,a0,v1` and the LZCS island input both read the copy afterwards, as the
 *     target does.
 *  2. 5 -> 2, the LZCR slot-address ORDER closed.  The target emits
 *     `addiu $v0,$sp,0x10` BETWEEN the `mtc2 $t4,$30` + nop/nop group and the
 *     `addu $t4,$v0,$zero` + `swc2 $31,0($t4)` group (asm/funcs/func_8002D780.s
 *     L141-147).  A single asm statement CANNOT produce that: an "r" operand's setup
 *     insn is always emitted before the whole asm insn, so s2/s3's one-statement island
 *     with `"r"(&sp_var)` hoisted the addiu above the mtc2.  Splitting the island into
 *     two asm statements -- the mtc2 island, then the swc2 island whose "r" operand is
 *     `&sp_var` -- puts the addiu exactly where the target has it.  The clobber split
 *     is not load-bearing between $12/$12-$15 orderings (g1 = g2 = g3 = 2) but SOME
 *     $13-$15 mention is (g4, `"$12"` on both, = 6): that is the same reload1.c
 *     bad_spill_regs effect the sibling clobber footprint buys, and the footprint stays
 *     the byte-matched sibling spelling (func_8002BC68 src/code6cac_b.c:762-767,
 *     func_8002BEA0 :825-830, judge ruling 2026-07-28 recorded in-line at :751-758).
 *
 * The ONE remaining residual (2 insns, unchanged since s3) is residual A: the test-3
 * subu pair.  The target emits `subu v1,t5,t1` (dx = x2 - x0) before `subu v0,a2,a3`
 * (az = cz - z0); we emit az first.  Every form that puts dx's statement before az's
 * DOES fix the order but permutes dz and dx between $a0 and $v1, which costs 7 more
 * insns (score 9 on this chassis; 14 on the s3 chassis).  Twelve declaration orders
 * were measured in s3 and another twenty-two forms in s5 (batches A, B, C, H, I, J):
 * only the shapes that keep az's statement before dx's hold the floor.
 *
 * DO NOT "clean up" the six test-3 difference locals or their declaration order, and do
 * NOT merge the two asm statements back together or drop the `m = dist;` re-store --
 * each is worth 2-7 insns and every alternative is measured in hypotheses.md.
 *
 * The three cop2 blocks are the owner-authorized canonical LZCS/LZCR + mvmva idiom
 * (.claude/rules/cop2-addressing-preamble-cluster.md).
 *
 * FAMILY NOTE for whoever reaches 0: this body carries ONE FAKE-annotated construct (the
 * same-value re-store of the local `m`), which is the dead-store family
 * (.claude/rules/dead-store-fake-exception.md) with an in-TU byte-matched precedent at
 * src/code6cac_b.c:1244-1265.  The six test-3 difference locals still need the
 * named-intermediate-vs-ordinary-C decision recorded in the s3 frontier before any
 * candidate-ready submission. */
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
                s32 ax;
                s32 dx;
                s32 az;
                s32 bx;
                s32 bz;
                ax = cx - x0;
                z2 = z2 - z0;
                dx = x2 - x0;
                az = cz - z0;
                bx = px - x0;
                bz = pz - z0;
                kc = z2 * ax - dx * az;
                kp = z2 * bx - dx * bz;
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
                /* FAKE: redundant same-value re-store of the LOCAL `m` (it already
                 * holds `dist` on entry to this arm), mechanism: cse.c make_regs_eqv --
                 * with a SINGLE def cse puts `m` and `dist` into one quantity, rewrites
                 * the asm input and the post-join `m >> shift` read to dist's register
                 * and deletes the copy insn outright (measured control: the same body
                 * without this line scores 7 at 202 insns and emits `nop` where the
                 * target has `move $a0,$s1`). The second def makes the pseudo
                 * multiply-defined across the join, invalidating that equivalence, so
                 * the copy survives cse and reorg.c steals it into the `beqz` delay
                 * slot; the redundant store itself is dropped before final output
                 * (build_insns 202 == target_insns 202, zero emitted bytes).
                 * lever-exhaustion: memory/grind/func_8002D780/hypotheses.md s1-s5 --
                 * twenty-three single-def copy spellings measured dead across four
                 * sessions (plain s32/u32 copies, outer-block and else-arm placements,
                 * asm-input-only, srlv-index-only, re-derived expression, embedded
                 * assignment in a condition) plus a 55,531-iteration decomp-permuter
                 * campaign; the same construct is the byte-matched sibling
                 * func_8002D518's closed residual at src/code6cac_b.c:1244-1265. */
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
