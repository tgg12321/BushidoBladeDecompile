/* func_8002EA24 -- ALTERNATIVE spelling of the banked candidate, score 2,
 * 104 instructions, measured this session (session 11).
 *
 * Identical to memory/grind/func_8002EA24/candidate.c except that lever L1 --
 * the two-statement last reject arm that defeats jump.c's store-flag
 * if-conversion and restores target's unfolded 0/1 diamond -- stages the
 * returned 0 through `y_low` instead of through `z`:
 *
 *     if (y + a0_var < min_y) { y_low = 0; return y_low; }
 *
 * `y_low` is the local holding *(s32 *)(obj + 0xB0); its value is consumed by
 * the min_y / max_y if-else immediately above and is provably dead at the
 * staging point, exactly as `z`'s is.  Measured: 2 on the L3 body and 3 on the
 * no-L3 body -- the same numbers as the `z` spelling, at target's instruction
 * count in both cases.
 *
 * Why it is banked: it shows L1's effect is a property of the TWO-STATEMENT
 * ARM (the single-set precondition of jump.c's store-flag transform), not of
 * any particular reused variable.  Whoever rules on the L1 construct should
 * know the choice of staging local is free, so the construct cannot be
 * defended or attacked on the grounds that `z` specifically was needed.
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

        /* max_y carries the rotated X here and the upper Y bound below -- one
         * local, two jobs, no extra statement (see L2 in the header). */
        max_y = *(s32 *)(obj + 0x100);
        /* FAKE: stages the first range test's boolean -- a real value, read by
         * the very next `if` -- through a0_var, whose own value (the squared
         * distance) is not set until after the chain and is therefore dead
         * here.  Mechanism: GCC 2.7.2 global.c allocation; a0_var is this
         * function's ONLY $a0-preferring allocno, so making it live across the
         * range-test chain is what denies $a0 to neg_threshold and lets
         * find_reg's first fit hand neg_threshold target's $t1.  Family:
         * [[staged-value-reused-variable]].  Lever-exhaustion: hypotheses.md
         * sessions 2-4 (accearly/accmid/accpre/accsplit/xzptr, accshare,
         * vinlive, minmaxearly, tshare/tshare1/zshare/negshare, xtop, and the
         * five alternative staging variables measured this session). */
        a0_var = max_y < neg_threshold;
        if (a0_var || threshold < max_y) return 0;
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
        /* FAKE: stages the return value 0 through z -- a real value, read by
         * the very next statement -- whose own value (the rotated Z) last
         * mattered at the `z * z` above and is dead here.  Mechanism: jump.c's
         * store-flag if-conversion requires a SINGLE-SET arm, so a two-statement
         * arm keeps target's unfolded 0/1 diamond instead of folding it to
         * `slt` + `xori $v0,$v0,1`.  Family: [[staged-value-reused-variable]]
         * (a live-value cousin of [[dead-store-fake-exception]], which documents
         * this exact symptom with a DEAD store; this form has none).
 */
        if (y + a0_var < min_y) { y_low = 0; return y_low; }
        return 1;
    }
}
