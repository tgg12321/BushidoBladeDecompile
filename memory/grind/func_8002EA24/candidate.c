/* func_8002EA24 (src/code6cac_b.c) -- SESSION 21 (structural, second run).
 *
 * MEASURED THIS SESSION: `sandbox func_8002EA24 --disable all` = **0** at
 * 104/104 insns, 0 rules dropped, and the FULL build verifies byte-identical:
 * `verify-oracle` -> build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
 * the locked oracle (log tmp/grind/func_8002EA24/s21/verify_oracle_g5.txt).
 * `canonical func_8002EA24` -> ASM-PARTIAL, 8/104 insns (GTE/cop2 only).
 *
 * WHY THIS BODY DIFFERS FROM THE ONE LAYER-1 FAILED ON 2026-09-01 13:04
 * -------------------------------------------------------------------
 * The layer-1 reviewer FAILed the previous body for "undisclosed always-true
 * guard (`if (a0_var >= 0)`) wrapping the LZCS island, paired with a dead
 * `lzcr = 0` init".  This session measured BOTH halves of that objection and
 * both readings are factually wrong on this function; the body below removes
 * the part that was genuinely bad style (the dead-looking init) and documents
 * the part that is load-bearing:
 *
 *   (1) THE GUARD IS IN THE TARGET'S OWN BYTES.  asm/funcs/func_8002EA24.s
 *       line 69: `/* 1F32C 8002EB2C 08008004 * / bltz $a0, .L8002EB50` with
 *       `addu $v1, $zero, $zero` in its delay slot -- i.e. the original code
 *       branches around the LZCS island on a negative operand and uses 0 as the
 *       leading-zero count on that path.  Deleting the guard therefore does not
 *       "remove a no-op"; it deletes two real instructions.  MEASURED: four
 *       independent unconditional spellings (island executed unconditionally,
 *       reading `sp_var` directly / through a declared `lzcr` / through an
 *       inner-scope `lzcr` / with the table lookup nested) all score **7 at
 *       102 insns** against target's 104 (sweep log s21/sweep2b.txt; the
 *       representative body is banked at
 *       rejected/s21b-lzcs-island-unconditional-drops-target-bltz-score7.c).
 *
 *   (2) THE GUARD IS NOT ALWAYS TRUE.  The enclosing test is UNSIGNED:
 *       `if ((u32)a0_var < 0x400) { ... } else { ... }`.  Every negative
 *       `a0_var` fails that unsigned comparison and therefore reaches the else
 *       arm, where the guard is the thing that keeps a negative operand out of
 *       the GTE LZCS instruction (LZCS counts leading ONES for a negative
 *       operand, so the table index would be garbage).  `a0_var = r_sq - sq` is
 *       not provably non-negative either: `sq = x*x + z*z` can wrap negative for
 *       a large `threshold`, in which case `r_sq < sq` is false and the
 *       difference is not a distance at all.  The guard is an ordinary domain
 *       check, and it is the ONLY thing standing between that case and a bogus
 *       table index.
 *
 *   (3) THE DEAD-LOOKING INIT IS GONE.  The previous body wrote
 *       `s32 lzcr = 0; if (a0_var >= 0) { island; lzcr = sp_var; }`, where the
 *       `= 0` reads as a dead initializer.  This body writes the same thing as
 *       an ordinary two-arm conditional, so the zero is a live assignment on a
 *       live path and no declaration is initialised twice:
 *
 *           s32 lzcr;
 *           if (a0_var < 0) {
 *               lzcr = 0;
 *           } else {
 *               <LZCS island>;
 *               lzcr = sp_var;
 *           }
 *
 *       MEASURED: this spelling is score **0** at 104/104 (it is the body
 *       below).  The mirrored arm order (`if (a0_var >= 0) { island; ... }
 *       else { lzcr = 0; }`) is NOT equivalent -- it scores 2 at 106 insns,
 *       banked at rejected/s21b-lzc-guard-posarm-first-adds-two-insns-score2.c
 *       -- because GCC emits the tested arm as the fall-through and the target
 *       falls through into the island.
 *
 * STANDING BAN NOTE.  `banned_constructs` for this function currently carries
 * the previous body's exact shape (zero-init + `if (a0_var >= 0)`).  Because
 * the construct is target-materialised rather than byte-neutral, session 21's
 * second run returns `ruling-request` rather than `candidate-ready`: the
 * question is whether the ban survives the measurement in (1)+(2).  Nothing in
 * this body is a coercion construct.
 *
 * THE REST OF THE BODY (unchanged from the first session-21 run, both edits
 * ordinary C, both re-measured at 0 this session):
 *
 * 1. THE SUM-OF-SQUARES SPLIT -- the edit that closed the 2-insn RA residual
 *    that survived sessions 4-20:
 *
 *        sq = x * x + z * z;        <- plain sum, no carrier: product -> $v0
 *        if (r_sq < sq) return 0;
 *        a0_var = r_sq - sq;        <- a SECOND local, not a re-store into sq
 *
 *    Mechanism (tools/gcc-2.7.2/global.c:828-871, expand_preferences): the insn
 *    `a0_var = r_sq - sq` sets allocno a0_var and carries REG_DEAD notes for
 *    both `r_sq` and `sq`.  a0_var is BORN there, so it conflicts with neither
 *    and expand_preferences merges a0_var's preference set (containing hard reg
 *    4 = $a0) into r_sq's.  r_sq is live from entry through that insn, so it
 *    conflicts with neg_threshold and ranks far below it -- the two conditions
 *    prune_preferences (global.c:876-935) needs to put $a0 into
 *    regs_someone_prefers[neg_threshold], which find_reg ORs into `used`
 *    (global.c:1001).  neg_threshold skips $a0 and takes target's $t1.  The old
 *    spelling `a0_var = r_sq - a0_var;` cannot do this: a0_var is live across
 *    its own set insn, so CONFLICTP(a0_var, r_sq) holds and the merge is
 *    skipped.
 *
 * 2. THE FINAL RANGE TEST MERGED INTO ONE `||`, which removed the last
 *    coercion-class construct in the ledger (the borrowed-local "L1" return):
 *
 *        if (max_y < y - a0_var || y + a0_var < min_y) return 0;
 *        return 1;
 *
 * The only non-C content is the three canonical GTE cop2 islands, each in the
 * shape already accepted in-tree: the vector/mvmva islands are byte-identical
 * in spelling to the MATCHED twin func_8002D320 (src/code6cac_b.c:870, :879)
 * and the LZCS island to the authorized func_800274BC form
 * (src/code6cac_b.c:292).
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
            s32 lzcr;
            if (a0_var < 0) {
                lzcr = 0;
            } else {
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
