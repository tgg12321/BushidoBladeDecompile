/* REJECTED (session 10, synthesis) -- score 6 on the no-L3 base (control 3).
 *
 * H8's round-1 probe, and the FIRST form in ten sessions to produce target's
 * `negu $t1` / `slt $v0` pair WITHOUT the L3 staged boolean.  The tail's last
 * range test writes `y + a0_var` into the (dead) `threshold` parameter, so the
 * insn that KILLS allocno 102 (`y`, whose only register preference is hard reg
 * 4 = $a0, inherited from the incoming `obj` argument) also SETS allocno 74
 * (`threshold`).  102 and 74 do not conflict, so expand_preferences
 * (global.c:797-841) IORs their preference sets BOTH ways.  Measured in the
 * .greg dump: `74 preferences: 4 6` (was `6`), 74 ranks below 103 and conflicts
 * with it, so prune_preferences unions hard reg 4 into
 * regs_someone_prefers[103], find_reg's pass 0 excludes it, and 103 lands in
 * hard reg 9 = $t1 = TARGET with the first range test's boolean back in $v0
 * (pseudo 104 in 2).  This is exactly the configuration session 7 proved the
 * ORIGINAL compile must have had (an $a0-preferring allocno conflicting with
 * 103 from BELOW it in the priority order), reached for the first time.
 *
 * WHY IT STILL LOSES: the IOR is symmetric.  102 (`y`) inherits 74's own $a2
 * preference, and find_reg's own-preference override then puts `y` in $a2
 * instead of target's $v1 (dump: `102 preferences: 4 6`, 102 in 6), which
 * swaps `y` and the tail max_y value and additionally makes the last test
 * compute into $a2 instead of target's $v0 (`addu a2,a2,a0 / slt v0,a2,v1`
 * against target's `addu v0,v1,a0 / slt v0,v0,a2`).  Net 6 against the
 * control's 3.  The r_sq spelling (`r_sq = y + a0_var`) is the same story with
 * $a3 and also scores 6; both with L3 kept score 8.
 *
 * The route is NOT closed as a family -- see hypotheses.md H8' for the exact
 * remaining requirement (a pref-4 recipient below 103 that has NO
 * argument-register preference of its own to leak back into `y`).
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
        threshold = y + a0_var;
        if (threshold < min_y) { z = 0; return z; }
        return 1;
    }
}
