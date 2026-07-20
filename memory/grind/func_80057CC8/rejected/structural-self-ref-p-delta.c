/* s21 STRUCTURAL kill: self-referential SET p = p + (off_next - off_prev)
 * to test whether pseudo 86's reg_n_deaths could be reduced to 1 by
 * making the second SET reference the first as source operand.
 * Result: sandbox --disable all = 64 (build_insns=115 vs target 111,
 * +4 regression). GCC 2.7.2 combine.c does NOT fold the (next-prev)
 * delta cancellation with the original off_prev inside p's value.
 * The self-ref adds a subtraction plus retention of prev_idx across
 * call1, worse than the plain two-independent-SET candidate.c form.
 * More importantly, flow.c still places REG_DEAD on p's use at the
 * subtraction (last use of "first" p value), so reg_n_deaths[86]
 * remains ==2 -- local-alloc bail-out (local-alloc.c:472) still fires.
 * This structurally confirms: any two-SET-of-p C form triggers
 * reg_n_deaths>=2 -> unconditional local-alloc skip -> pseudo 86 lands
 * in the global-alloc pool where its {v1} pref locks placement. */
    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((s32)p + ((((s32)(next_idx << 16) >> 16) << 2) - (((s32)(prev_idx << 16) >> 16) << 2)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
