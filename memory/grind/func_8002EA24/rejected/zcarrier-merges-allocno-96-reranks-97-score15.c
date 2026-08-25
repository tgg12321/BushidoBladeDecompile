/* REJECTED (session 15, solver modality) -- score 15 at 104 insns (floor is 2).
 *
 * WHY IT WAS TRIED.  The ra_solver forward model (global.c, validated 10/10)
 * says the ONLY C-reachable perturbation that moves the neg_threshold allocno
 * (pseudo 103) to target's $t1 is a live-range overlap with the a0_var allocno
 * (pseudo 97) -- see tmp/grind/func_8002EA24/s15/depth1_sweep.py.  Sessions 8-9
 * enumerated nine "carriers" for a0_var (r_sq, rotated X, threshold, the sum of
 * squares, the delta temps, vin/vout, y, -threshold) but never `z`, the rotated
 * Z, which is the one value loaded BETWEEN the two range tests and therefore
 * overlaps 103's live range at zero instruction cost.  This body folds z into
 * a0_var: `a0_var = *(s32 *)(obj + 0x104); if (a0_var < neg_threshold || ...)`
 * and then `a0_var = max_y * max_y + a0_var * a0_var;`.  L1 is staged through
 * `y_low` (banked as byte-equivalent in candidate_alt_L1_via_ylow.c) because
 * `z` no longer exists.
 *
 * WHY IT IS DEAD (measured, with the mechanism).  The edge IS created --
 * `97 conflicts:` now contains 103 -- but 103 lands in $a1 (5), not $t1 (9).
 * Merging z into a0_var deletes allocno 96 and lifts 97's refs 13 -> 18 and its
 * live length 32 -> 39, which re-ranks 97 from THIRD to FIRST in the allocation
 * order; max_y (100) then falls $a1 -> $v1 and y (102) rises $v1 -> $a1, and
 * 103 takes the vacated $a1.  This is the same failure the session-9 `v2`
 * carrier showed, and with `z` measured the carrier family is now closed with a
 * general mechanism rather than by enumeration: EVERY value computed before the
 * range chain is ALREADY an allocno, so re-using one as a0_var's pre-chain value
 * creates the required edge and simultaneously DELETES an allocno, vacating the
 * register target needs; and any value that is not already an allocno is a new
 * computation, i.e. extra instructions.  Route B is therefore empty in C.
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
        if (max_y < neg_threshold || threshold < max_y) return 0;
        a0_var = *(s32 *)(obj + 0x104);
        if (a0_var < neg_threshold || threshold < a0_var) return 0;

        a0_var = max_y * max_y + a0_var * a0_var;
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
 *
 * SESSION-11 STRUCTURAL (body UNCHANGED; floor re-measured at 2 this session).
 * Session 10's H8' -- the `regs_someone_prefers` preference route -- is now
 * CLOSED, by an exhaustive enumeration of the possible recipients rather than
 * by another failed spelling.  Read off this body's own .greg, the allocnos
 * that CONFLICT with 103 (neg_threshold) are exactly {72 obj, 74 threshold,
 * 75 r_sq, 96 z, 100 max_y}, and the allocation order is
 * `101 96 97 100 109 108 72 102 117 103 74 99 75`.  Every candidate recipient
 * is therefore one of six cases, and all six are now measured:
 *   72 (obj)      -- preferences pruned by its own hard-reg self-conflict (s3).
 *   100 (max_y)   -- CONFLICTS with the donor 102 (`y`), and expand_preferences
 *                    is gated on `! CONFLICTP` in both directions, so nothing
 *                    propagates: the .greg for the max_y recipient is
 *                    BIT-IDENTICAL to the control's (score 5).
 *   74 / 75       -- the session-10 parameters.  They work (103 -> $t1) but the
 *                    symmetric IOR hands `y` their argument register (6 / 6).
 *   fresh local   -- poisons `103 preferences` and re-ranks 103 to first; the
 *                    session-10 spelling carried the first range test's boolean
 *                    and this session's carried the other half of the same
 *                    short-circuit (which never mentions neg_threshold) -- both
 *                    inert at 3, so the poisoning is not operand-level.
 *   96 (z)        -- the only recipient that satisfies BOTH side-conditions:
 *                    `96 preferences: 4` (forward leg fires) with
 *                    `102 preferences: 4` UNCHANGED (harmless reverse leg) and
 *                    `103 preferences` still empty.  It fails ONLY on rank: 96
 *                    is allocated SECOND, so it takes $a0 itself instead of
 *                    denying it, and the assignment cascades (score 16).
 * The two ways out of the last case are both measured dead.  Demoting 96 below
 * 103 needs floor_log2(n_refs)*n_refs/live_length to fall by 3-8x; cutting z's
 * references (L1 staged through `y_low`, then L1 removed entirely) leaves the
 * allocation order BIT-IDENTICAL, and z's live range cannot start earlier
 * because `*(s32 *)(obj + 0x104)` is WRITTEN by the GTE store block directly
 * above the load.  Blocking `y` from the parameter's argument register needs an
 * allocno that conflicts with 102, outranks it and holds $a2; the only
 * candidate (99 = min_y) can only be made to conflict by an earlier birth, and
 * allocno_compare puts live_length in the DENOMINATOR, so the same edit demotes
 * it to LAST, where it steals target's $t1 outright (score 13 at 105 insns).
 * A conflict placed on 102 to stop the reverse leg also stops the forward leg,
 * since the IOR is gated on `! CONFLICTP` symmetrically.
 * One positive finding: L1's staging variable is FREE.  Staging the returned 0
 * through `y_low` instead of `z` measures 2 on this body and 3 on the no-L3
 * body -- identical, at target's 104 instructions -- so the store-flag defeat
 * is a property of the two-statement arm, not of `z`.  Banked as
 * candidate_alt_L1_via_ylow.c for whoever rules on the construct.
 */
        if (y + a0_var < min_y) { y_low = 0; return y_low; }
        return 1;
    }
}
