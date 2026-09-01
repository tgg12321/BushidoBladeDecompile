/* func_8002EA24 (src/code6cac_b.c) -- SESSION 21 (rederive): sandbox
 * `func_8002EA24 --disable all` = **0** at 104/104 insns, 0 rules, and the FULL
 * build verifies byte-identical: `verify-oracle` -> build_sha1 ==
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa == the locked oracle.
 *
 * This body carries **no FAKE construct and no coercion construct at all**.
 * Every local is an ordinary named intermediate holding a real, consumed value;
 * there is no variable reuse, no dead store, no constant holder, no pointer
 * alias, no volatile, no pad.  The only non-C content is the three canonical
 * GTE cop2 islands (`canonical func_8002EA24` -> ASM-PARTIAL, 8/104 insns:
 * lwc2/mvmva, swc2 x3, mtc2/swc2 LZCS), each in the shape already accepted
 * in-tree:
 *   - vector/mvmva islands: byte-identical in spelling to the MATCHED twin
 *     func_8002D320 (src/code6cac_b.c:870 and :879) -- operand address computed
 *     in C and bound via %0, template limited to the $t4 copy + lwc2/swc2 +
 *     the mvmva .word, no hardcoded $v0 addiu, no $2 clobber.
 *   - LZCS island: byte-identical in spelling to the user-authorized
 *     func_800274BC form (src/code6cac_b.c:292).
 *
 * WHAT SESSION 21 CHANGED (two edits against s20's b8 body, both ordinary C):
 *
 * 1. THE SUM-OF-SQUARES SPLIT (this is the edit that closed the 2-insn RA
 *    residual that survived sessions 4-20).  s20's body carried the parameter
 *    `threshold` as the carrier of `x*x` in order to donate a $a0 preference to
 *    it via global.c's expand_preferences, which is what pushed `neg_threshold`
 *    off $a0 and onto the target's $t1 -- but the carrier then owned $a2, so the
 *    first product came out `mflo a2` instead of the target's `mflo v0`
 *    (residual: ours[46] `mflo a2`, ours[49] `addu a0,a2,v1`).  Session 21
 *    reaches the SAME donation through an operand that costs no register seat:
 *
 *        sq = x * x + z * z;        <- plain sum, no carrier: product -> $v0
 *        if (r_sq < sq) return 0;
 *        a0_var = r_sq - sq;        <- a SECOND local, not a re-store into sq
 *
 *    Mechanism (global.c:828-871, expand_preferences): the insn `a0_var = r_sq -
 *    sq` sets allocno a0_var and carries REG_DEAD notes for BOTH `r_sq` and
 *    `sq`.  Because a0_var is BORN there, it conflicts with neither, so
 *    expand_preferences merges a0_var's preference set (which contains hard reg
 *    4 = $a0) into r_sq's.  r_sq is live from function entry through this insn,
 *    so it CONFLICTS with neg_threshold, and its priority (few refs over a long
 *    range) is far below neg_threshold's -- exactly the two conditions
 *    prune_preferences (global.c:915-928) requires to put $a0 into
 *    `regs_someone_prefers[neg_threshold]`, which find_reg then ORs into `used`
 *    (global.c:1001).  neg_threshold therefore skips $a0 and takes $t1.
 *    The old spelling `a0_var = r_sq - a0_var;` cannot do this: with a0_var as
 *    both source and dest it is live across its own set insn, so it CONFLICTS
 *    with r_sq and expand_preferences refuses the merge.  Splitting the
 *    sum-of-squares and the remaining-distance into two named locals is what a
 *    human would write anyway; it just happens to be the donation route that
 *    costs nothing.
 *
 * 2. THE FINAL RANGE TEST MERGED INTO ONE `||` (this removed the last
 *    coercion-class construct in the ledger).  Every candidate since session 4
 *    carried "L1": `if (y + a0_var < min_y) { z = 0; return z; }` -- a dead
 *    local borrowed to carry the return constant, to stop jump.c collapsing the
 *    final 0/1 diamond into slt/xori.  Session 21 measured five ordinary
 *    replacements (plain return, goto-label, result variable, inverted test,
 *    named sum: all score 3 at 102 insns) and then the one that works:
 *
 *        if (max_y < y - a0_var || y + a0_var < min_y) return 0;
 *        return 1;
 *
 *    Writing the last two bound checks as one short-circuit condition -- the
 *    same `||` shape the two earlier bound checks in this function already use
 *    -- keeps the diamond unfolded with no borrowed variable.  Score 0.
 *
 * MEASURED THIS SESSION (tmp/grind/func_8002EA24/s21/sweep1.txt, sweep2.txt):
 *   r0 s20-b8 control 2 | r1 plain-sum + split d **0** | r2 three-way split 21
 *   r3 donation+split 2 | r4 plain sum, no split 6 | r5 early -threshold 7
 *   r6 split+early-neg 3 | r7 addends swapped 2
 *   L1n merged final || **0** | L1h min_y-borrowed 0 (same construct family as
 *   L1, not used) | L1a plain return 3 | L1b goto label 3 | L1c result var 3
 *   | L1f inverted 3 | L1g named sum 12 | L1i all-goto 13 | L1j mixed exits 4
 */
s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {
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

    {
        s32 z;
        s32 a0_var;
        s32 sp_var;
        s32 min_y;
        s32 max_y;
        s32 y_low;
        s32 y;
        s32 x;
        s32 neg_threshold = -threshold;
        s32 sq;

        x = *(s32 *)(obj + 0x100);
        if (x < neg_threshold || threshold < x) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        sq = x * x + z * z;
        if (r_sq < sq) return 0;
        a0_var = r_sq - sq;

        if ((u32)a0_var < 0x400) {
            a0_var = (u32)*(((u8 *)&D_8008D118) + a0_var) >> 3;
        } else {
            s32 lzcr = 0;
            if (a0_var >= 0) {
                __asm__ volatile(
                    "addu $t4, %1, $zero\n"
                    "mtc2 $t4, $30\n"
                    "nop\n"
                    "nop\n"
                    "addu $t4, $sp, $zero\n"
                    "swc2 $31, 0($t4)"
                    : "=m"(sp_var) : "r"(a0_var) : "$12");
                lzcr = sp_var;
            }
            {
                s32 shift = 0x16 - (lzcr & ~1);
                s32 tbl = *(((u8 *)&D_8008D118) + ((u32)a0_var >> shift));
                a0_var = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));
            }
        }

        max_y = 0;
        min_y = 0;
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var || y + a0_var < min_y) return 0;
        return 1;
    }
}
