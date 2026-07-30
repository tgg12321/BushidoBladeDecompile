/* REJECTED (session 11, structural) -- score 13 on the no-L3 base (control 3),
 * and 105 instructions: it costs an instruction as well as registers.
 *
 * The last escape from the reverse leg.  With a parameter recipient (session
 * 10's v1, score 6) `y` inherits $a2 and leaves target's $v1.  find_reg only
 * refuses `y` a register that is in `used`, and a non-conflicting holder does
 * not put it there -- so the ONLY way to keep `y` off $a2 is an allocno X with
 * (i) CONFLICTP(102, X), (ii) X ranked ABOVE 102 and (iii) X assigned $a2.
 * Blocking it with a conflict on 74 itself is self-defeating: expand_preferences
 * is gated on `! CONFLICTP` in BOTH directions, so a conflict that kills the
 * reverse leg kills the forward leg with it.
 *
 * The only allocno conflicting with 102 that could hold $a2 is 99 (min_y),
 * which currently ranks LAST-but-two and is assigned $a2 by first-fit.  To
 * promote it, this body gives min_y an earlier birth (`min_y = 0;` hoisted to
 * just above the z load, so it also overlaps 96 = z and would not simply take
 * $v1 out from under `y`).
 *
 * It backfires completely, and the mechanism is the same trap that killed the
 * session-3 accearly family: allocno_compare's priority has live_length in the
 * DENOMINATOR, so an earlier birth is a DEMOTION, not a promotion.  Dump:
 * 99 moves from 12th to LAST (`;; 13 regs to allocate: 101 97 96 100 109 108
 * 72 102 117 74 103 75 99`), now conflicts with 103, and lands in hard reg 9 --
 * i.e. it STEALS target's $t1 -- while 103 keeps $a0 and 102 keeps $a2.  The
 * hoist also re-ranks 74 ABOVE 103, breaking H8' condition (c) for the
 * recipient as well, and costs one instruction (105 vs target's 104).
 *
 * "Promote the blocker" and "make the blocker live earlier" are the same
 * source-level lever pointing in opposite allocator directions, so the escape
 * is closed.
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
        min_y = 0;
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
        y_low = *(s32 *)(obj + 0xB0);
        if (y_low < 0) {
            min_y = y_low;
        } else {
            max_y = y_low;
        }
        y = *(s32 *)(obj + 0x108);
        if (max_y < y - a0_var) return 0;
        threshold = y + a0_var;
        if (threshold < min_y) { z = 0; return z; }
        return 1;
    }
}
