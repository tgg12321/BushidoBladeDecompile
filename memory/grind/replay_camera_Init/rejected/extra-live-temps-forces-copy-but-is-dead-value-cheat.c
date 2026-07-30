/* REJECTED (s2, 2026-07-30) — DIAGNOSTIC ONLY, never committable.
 * Semantics are deliberately changed (three extra loads folded into the
 * D_80101E78 computation); this file exists solely to record the CONFIRMED
 * mechanism it proved.
 *
 * WHAT IT PROVED (positive result, wrong means): the incoming-parameter home
 * copy for `a1` materialises as a REAL instruction — here `move v1,a1` at the
 * top of the function, exactly where target has `addu $a3,$a1,$zero` — as soon
 * as another simultaneously-live value claims hard reg $a1 over the copy's live
 * range. Measured: score 26 / 49 insns, and the disassembly shows
 *   3b0: move v1,a1      <- the home copy, no longer coalesced
 *   3e4: lw   a1,8(at)   <- a local temp took $a1
 * versus candidate.c, where `sw a1, %lo(D_80101E7C)` uses $a1 directly and the
 * copy is deleted.
 *
 * WHY IT IS STILL DEAD as a route to target:
 *  1. The three temporaries have no semantic purpose in this function; adding
 *     them is a dead-value / frame-coercion cheat by any spelling
 *     (no-new-park-categories "cheats by any spelling"). The function's real
 *     value set is exactly sval / cam_val / ec_val.
 *  2. Even ignoring (1), the copy lands in $v1 (hard reg 3), not target's $a3
 *     (hard reg 7). Reaching 7 requires hard regs 2,3,4,5,6 to all be
 *     unavailable to that allocno at its allocation point, i.e. FIVE values
 *     live across the a1-store region. This function has three.
 *
 * MECHANISM, named and line-cited (GCC 2.7.2):
 *  - global.c prune_preferences (the `if (allocno_size[...] <= allocno_size[...])
 *    AND_COMPL_HARD_REG_SET (temp, hard_reg_full_preferences[allocno]);` clause,
 *    global.c:893-895) explicitly REFUSES to place into regs_someone_prefers[A]
 *    any register that A itself prefers. The a1-home pseudo has
 *    hard_reg_full_preferences == {5} (confirmed in the .greg dump:
 *    `;; 73 preferences: 5`). Therefore $a1 can never be excluded from this
 *    allocno by the preference machinery.
 *  - global.c find_reg pass 0 masks used1 (conflicts) plus the complement of
 *    regs_used_so_far plus regs_someone_prefers, then takes the lowest free
 *    hard reg (no REG_ALLOC_ORDER is defined for MIPS in this tree).
 *    regs_used_so_far is seeded with ALL call_used_regs (global.c:352-355),
 *    so $a1..$a3 / $t0.. are all candidates from pass 0 onward.
 *  - Consequently the ONLY way to deny the a1-home pseudo hard reg 5 is a
 *    genuine entry in hard_reg_conflicts, which requires $a1 to be occupied by
 *    an overlapping live value. That is exactly what this diagnostic supplies —
 *    and exactly what the function's honest value set cannot supply.
 */
s32 replay_camera_Init(s32 a0, s32 a1) {
    extern u8 SpecialCam;
    s32 sval;
    s32 cam_val;
    s32 ec_val;
    s32 t1;
    s32 t2;
    s32 t3;

    if (D_80101E62 != 0) {
        return 0;
    }

    sval = ((s32)(a0 << 16)) >> 13;
    D_80101E60 = a0;
    cam_val = *(s32 *)((u8 *)&SpecialCam + sval);
    ec_val = *(s32 *)((u8 *)&D_8008EC38 + sval);
    t1 = *(s32 *)((u8 *)&SpecialCam + sval + 8);
    t2 = *(s32 *)((u8 *)&SpecialCam + sval + 12);
    t3 = *(s32 *)((u8 *)&SpecialCam + sval + 16);
    D_80101E6C = cam_val;
    D_80101E70 = ec_val;
    D_80101E7C = a1;
    D_80101E68 = 0;
    D_80101E62 = 2;
    D_80101E9E = 0;
    D_80101E78 = (u32)(D_80101E70 + 0x7FF + t1 + t2 + t3) >> 11;
    return 1;
}
