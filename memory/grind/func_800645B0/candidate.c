/* func_800645B0 (src/text1b.c) -- CLOSING FORM, session 9 (synthesis).
 * `sandbox func_800645B0 --disable all` = score 0, target_insns 78,
 * build_insns 78, rules_dropped 1.  Measured s9; supersedes the standing
 * floor of 1 that held from session 2 through session 8.
 *
 * Chassis: the session-7 "JD" re-derivation -- the three word destinations are
 * one array of 3-word structs and D_800F0BCC a parallel s16 array, so the
 * stride arithmetic is written as expressions at each store site and GCC's own
 * k*12 expansion shares its k*2 with the halfword index.  That chassis alone
 * measures 3 / 78 with every register, the *3 operand order and the insn count
 * already correct; its whole residual is the emission order of the two sets at
 * the top of the nested `for` (sched.c's birthing_insn_p max_priority lift
 * fires on `addu idx,i,j` but not on the const-1 set, so the const-1 set is
 * emitted first and reorg.c copies it into the back-edge delay slot).
 *
 * The closing construct is the FAKE-annotated dead store `bit = 0;`.  It gives
 * `bit` a second set in a second basic block of the nested `for`, which makes
 * loop.c's count_loop_regs_set (loop.c:3036-3047) mark may_not_move, so
 * loop.c:649 never considers the const-1 set a movable and it stays inside the
 * loop; flow.c's life_analysis then deletes the dead store as it walks and
 * never counts it, so reg_n_sets[bit] == 1 and sched.c's adjust_priority lifts
 * the const-1 set alongside the index addu.  With both at max_priority the
 * INSN_LUID tie-break restores the target's emission order at zero added
 * instructions.  Sanctioned family: dead stores / self-assigns to locals
 * (.claude/rules/dead-store-fake-exception.md), FAKE-annotated per that rule's
 * prerequisite 3; lever-exhaustion is memory/grind/func_800645B0/.
 *
 * Negative controls measured the same session (tmp/grind/func_800645B0/s9/
 * sweep29.py): VB, the same body with `bit` single-set and no dead store, is
 * 12 / 80 -- loop.c hoists the const-1 into a fresh callee-save.  VG, the dead
 * store spelled `bit = 1;`, is also 12 / 80: cse1 runs BEFORE loop.c and folds
 * a same-value store away, so the store must carry a different value to be
 * visible to count_loop_regs_set.  VD (dead store at the head of the arm) and
 * VE (`bit = 2;`) both also reach 0 / 78, so the win is neither a placement
 * nor a value tuning.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 mask;
    s32 val;
    s32 last;
    s32 bit;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            bit = 1;
            mask = bit << idx;
            if (!(D_800A3444 & mask)) {
                last = rand();
                *((s32 *)(((s32)(&D_800F0D78)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((((idx << 1) + idx)) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + (idx << 1))) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                bit = 0; /* FAKE: dead store to a local; mechanism: loop.c
                 * count_loop_regs_set sets may_not_move for a reg set in two basic
                 * blocks of the loop, which keeps the const-1 set inside the inner
                 * loop (loop.c:649), while flow.c life_analysis deletes this store
                 * and never counts it, so reg_n_sets == 1 and sched.c
                 * adjust_priority lifts the const-1 set alongside the index addu;
                 * lever-exhaustion: memory/grind/func_800645B0/hypotheses.md
                 * (sessions 1-8, H10/H15/H16/H18/H24/H30/H37/H39) */
                break;
            }
        }
    }
    return 1;
}
