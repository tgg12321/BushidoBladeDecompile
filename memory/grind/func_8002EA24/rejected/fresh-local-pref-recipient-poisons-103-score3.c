/* REJECTED (session 10, synthesis) -- score 3 = the no-L3 control, inert.
 *
 * H8 round 2: a FRESH local `t` was supposed to be the preference recipient
 * with no argument-register preference to leak back into `y` (which is what
 * cost round 1 its 3 points).  It carries the first range test's boolean (so
 * that it is live across the chain and conflicts with allocno 103) and is then
 * re-used at the tail for `y + a0_var` (so that the insn setting it kills
 * `y`, allocno 102, the $a0-preference donor).
 *
 * The .greg dump says why it is inert, and the mechanism is general: the same
 * symmetric IOR in expand_preferences that feeds `t` also feeds the boolean's
 * $v0 preference and hard reg 4 back into allocno 103 ITSELF
 * (`103 preferences: 2 4`), and prune_preferences line 893 explicitly REMOVES
 * from regs_someone_prefers[A] every register A itself prefers (same-size
 * case).  103 therefore keeps $a0 in its free set; it is additionally
 * promoted to FIRST in the allocation order (`;; 14 regs to allocate: 103 101
 * 96 97 100 110 109 72 102 118 104 74 99 75`) by the extra references and
 * lands in $v0, with the fresh local `t` (allocno 104) taking $a0.
 *
 * Consequence for the frontier: any recipient that is ALSO connected to 103 by
 * a preference-propagation edge poisons the route.  The recipient must touch
 * `y` (or another pref-4 allocno) and NOT touch 103.
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
        s32 t;
        s32 neg_threshold = -threshold;

        max_y = *(s32 *)(obj + 0x100);
        t = max_y < neg_threshold;
        if (t || threshold < max_y) return 0;
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
        t = y + a0_var;
        if (t < min_y) { z = 0; return z; }
        return 1;
    }
}
