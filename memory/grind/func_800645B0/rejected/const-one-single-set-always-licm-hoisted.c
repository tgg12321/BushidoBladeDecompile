/* REJECTED — func_800645B0, grind session 3 (2026-08-12).
 *
 * WHY DEAD: giving the constant 1 its OWN single-set variable (so that
 * sched.c's birthing_insn_p bonus fires on `li val,1` as well as on
 * `addu idx,i,j`, which restores the target's loop-top emission order) is
 * ALWAYS defeated by loop.c: the set is a single-set loop invariant, so
 * scan_loop makes it a movable and move_movables hoists it out of the inner
 * loop into a fresh callee-save, adding a save/restore pair.
 *
 * THREE SPELLINGS MEASURED, ALL HOISTED (target is 78 insns):
 *   KA  named user variable `one`, used only for the shift          12 / 80
 *   KB  same, and `one` is also the function's return value
 *       (so loop.c's m->global is set)                              13 / 80
 *   TA  UNNAMED constant, `mask = 1 << idx;` (a compiler temp, which
 *       takes loop.c's REG_EQUAL / m->move_insn path instead of the
 *       user-variable path)                                         12 / 80
 * Control TB (the shipped form, `val` reused for the constant and the
 * D_800A3444 read) = 3 / 78 in the same CA body.
 *
 * MECHANISM: loop.c:695-716 admits the insn as a movable through any one of
 * its three alternatives — a compiler temp passes `! REG_USERVAR_P &&
 * ! REG_LOOP_TEST_P`, and a user variable whose only uses are in the set's own
 * basic block passes `reg_in_basic_block_p`, and in both cases `! maybe_never
 * && ! loop_reg_used_before_p` also holds because the set is the first insn of
 * the loop body.  move_movables' desirability test (loop.c:1631,
 * `threshold * savings * m->lifetime >= insn_count` with
 * `threshold = (loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`) is trivially
 * satisfied.  m->global does NOT block the move (KB).
 *
 * CONSEQUENCE: frontier item 2 of the session-2 ledger ("make the const-1
 * pseudo single-set AND unhoistable so BOTH loop-top insns are bonused") is
 * KILLED.  The only way to keep the constant inside the loop is to keep its
 * pseudo multi-set, which by construction denies it the birthing bonus.
 *
 * Sweeps: tmp/grind/func_800645B0/s3/sweep14.py (KA/KB/KD),
 *         tmp/grind/func_800645B0/s3/sweep17.py (TA/TB).
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 one;   /* single-set -> loop.c movable -> hoisted into a new $s4 */
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            one = 1;
            mask = one << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                /* ... three s32 stores at (wid << 2), s16 store at idx2 ... */
                val = D_800A3444;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
