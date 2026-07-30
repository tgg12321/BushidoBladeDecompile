/* REJECTED (session 11, structural) -- score 16 on the no-L3 base (control 3),
 * 104 instructions, i.e. a pure register loss.
 *
 * THE MOST INFORMATIVE NEGATIVE OF THE PREFERENCE ROUTE.  `z` is the tail
 * preference recipient: `z = y + a0_var; if (z < min_y) { z = 0; return z; }`.
 * `z` is the ONLY allocno in this function that satisfies BOTH halves of
 * session 10's H8' side-conditions at once, and the .greg confirms it:
 *
 *     ;; 96 preferences: 4        <- the forward leg fired (z inherited hard
 *                                    reg 4 from `y`, allocno 102, which dies
 *                                    at the insn that sets z)
 *     ;; 102 preferences: 4       <- UNCHANGED.  The reverse leg is HARMLESS,
 *                                    because z owns no argument-register
 *                                    preference to hand back.  This is the
 *                                    failure mode that cost session 10's
 *                                    parameter recipients their 3 points, and
 *                                    it is measured absent here for the first
 *                                    time in eleven sessions.
 *     ;; 103 preferences:         <- still EMPTY, so prune_preferences line 893
 *                                    does NOT cancel the merge (the fresh-local
 *                                    recipients' failure mode is also absent).
 *
 * What kills it is the ONE remaining H8' condition, (c) rank-below-103.  `z` is
 * allocated SECOND (`;; 13 regs to allocate: 101 96 97 100 109 108 72 102 117
 * 103 74 99 75`), so instead of DENYING $a0 to 103 through
 * regs_someone_prefers it simply TAKES $a0 itself (96 in 4), and the whole
 * assignment cascades: 100 (max_y) falls from target's $a1 to $v1, 102 (y)
 * from target's $v1 to $a1, and 103 lands in $a1 rather than $t1.
 *
 * Demoting 96 below 103 is arithmetically out of reach.  allocno_compare ranks
 * by floor_log2(n_refs)*n_refs/live_length; 103 has 3 references, so 96 would
 * need roughly 3-8x 103's live length depending on how far z's reference count
 * can be cut.  Measured: cutting two of z's references by staging L1's return
 * through `y_low` instead (x1, score 16) and cutting L1 entirely (x2, score 19,
 * 102 insns) leave the allocation order BIT-IDENTICAL -- 96 still second.  And
 * z's live range cannot start any earlier, because the value it loads,
 * `*(s32 *)(obj + 0x104)`, is WRITTEN by the GTE store block (`swc2 $26,
 * 4($t4)`) immediately above it.  The one lever that would lengthen it is
 * blocked by the function's own dataflow.
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
        s32 neg_threshold = -threshold;

        max_y = *(s32 *)(obj + 0x100);
        if (max_y < neg_threshold || threshold < max_y) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        a0_var = max_y * max_y + z * z;
        if (r_sq < a0_var) return 0;
        a0_var = r_sq - a0_var;

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
        if (max_y < y - a0_var) return 0;
        z = y + a0_var;
        if (z < min_y) { z = 0; return z; }
        return 1;
    }
}
