/* REJECTED (session 11, structural) -- score 3 = the no-L3 control, inert.
 *
 * Session 10's fresh-local recipient failed because the only zero-cost
 * in-chain value it could carry was the FIRST range test's boolean, which
 * READS neg_threshold and so preference-connects the recipient to allocno 103
 * (dump: `103 preferences: 2 4`, cancelled by prune_preferences line 893).
 * This body tests the obvious repair: carry the OTHER half of the same
 * short-circuit instead -- `(r = threshold < max_y)`, which mentions 74 and 96
 * but never 103 -- and re-use `r` at the tail for `y + a0_var`.
 *
 * It fails the same way, which upgrades session 10's finding from
 * "this spelling poisons 103" to "any manufactured below-103 recipient
 * poisons 103": the dump again shows `103 preferences: 2 4`, and 103 is again
 * promoted to FIRST in the allocation order (`;; 14 regs to allocate: 103 101
 * 96 97 100 110 109 72 102 118 104 74 99 75`), landing in $v0 while the fresh
 * local takes $a0.  Note the poisoning survives even though the carried
 * boolean does not mention neg_threshold at all, so it is not an operand-level
 * effect -- adding any extra pseudo to this window re-ranks 103 and re-routes
 * the preference graph through it.
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
        s32 r;
        s32 neg_threshold = -threshold;

        max_y = *(s32 *)(obj + 0x100);
        if (max_y < neg_threshold || (r = threshold < max_y)) return 0;
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
        r = y + a0_var;
        if (r < min_y) { z = 0; return z; }
        return 1;
    }
}
