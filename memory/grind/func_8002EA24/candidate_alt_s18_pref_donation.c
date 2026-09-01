/* func_8002EA24 -- SESSION-18 ALTERNATIVE CHASSIS (sandbox 2, 104/104 insns).
 *
 * NOT a replacement for candidate.c (same floor, 2).  Banked because its
 * RESIDUAL IS SOMEWHERE ELSE, and because it reaches the floor with the L3
 * staged-boolean FAKE DELETED.
 *
 * What it is.  The banked candidate buys neg_threshold's $t1 by making a0_var
 * (the function's only $a0-preferring allocno) live across the range-test
 * chain -- the L3 staging FAKE -- which costs the chain's boolean its own
 * register: our `slt $a0,$a1,$t1` vs target's `slt $v0,$a1,$t1`, the 2-insn
 * residual every banked score-2 body shares.  This body buys the same $t1 a
 * different way: GCC 2.7.2 global.c expand_preferences (global.c:843-870) IORs
 * the hard_reg_preference SETS of two allocnos whenever a single_set to one
 * carries a REG_DEAD note for the other and the two do NOT conflict.  Writing
 *
 *     threshold = max_y * max_y;          <- deaths: 100 (max_y): 100 CONFLICTS 74, leg blocked
 *     a0_var    = threshold + z * z;      <- deaths: 74 (threshold) -> the WANTED leg fires
 *                                            and 96 (z): 96 CONFLICTS 74, leg blocked
 *
 * hands allocno 74 (the dead `threshold` parameter) a SECOND hard-reg
 * preference, $a0, on top of its own $a2.  74 is lower priority than 103 and
 * already conflicts with it, so prune_preferences (global.c:877-929, reverse
 * priority order) folds $a0 into regs_someone_prefers[103]; find_reg's pass-0
 * first fit then skips $a0 and hands 103 target's $t1 -- with NO value staged
 * into a0_var, so the range-test boolean stays a plain $v0 temp exactly as
 * target has it.  Measured: the whole chain (target insns 30/31/39) matches for
 * the first time in 18 sessions.
 *
 * Residual (2 insns, both in the sum of squares):
 *     ours[46] mflo a2          tgt[46] mflo v0
 *     ours[49] addu a0,a2,v1    tgt[49] addu a0,v0,v1
 * i.e. the x*x product sits in 74's own argument register $a2 because 74 keeps
 * its $a2 preference, where target leaves $a2 dead from the 4th range test on.
 * The open question for the next session is whether the 74<->97 REG_DEAD
 * pairing can be obtained WITHOUT routing a value through 74 (v3/v4/v5 all
 * route one, and each lands that value in $a2).
 *
 * CONSTRUCT STATUS -- NOT SELF-APPROVED.  `threshold = max_y * max_y;` is a
 * borrow of an existing PARAMETER for a second, unrelated value; that is the
 * variable-reuse family (.claude/rules/defeat-licm-hoist-var-reuse.md, borrows
 * gated by .claude/rules/staged-value-reused-variable.md) and it needs a
 * FAKE annotation plus lever-exhaustion receipts before any submission.
 * No annotation is written here because this body is banked as EVIDENCE, not
 * submitted: it does not reach 0.  See memory/grind/func_8002EA24/evidence.md
 * [s18] and tmp/grind/func_8002EA24/s18/.
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
        if (max_y < neg_threshold || threshold < max_y) return 0;
        z = *(s32 *)(obj + 0x104);
        if (z < neg_threshold || threshold < z) return 0;

        threshold = max_y * max_y;
        a0_var = threshold + z * z;
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
        if (y + a0_var < min_y) { z = 0; return z; }
        return 1;
    }
}