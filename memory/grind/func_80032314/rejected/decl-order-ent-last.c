/* REJECTED — func_80032314, session 1, H2.
 * Why dead: declaration-order permutation of the three function-scope locals
 * has ZERO effect on the $a1/$a2/$a3 rotation. Measured floor 15 (identical to
 * the baseline 15 form), same 3-cycle, same 109 insns.
 * Do not re-propose any pure declaration-order permutation of ent/state/a0.
 *
 * The only delta from candidate.c was:
 *     u8 *ent;          ->    s32 state;
 *     s32 state;              s32 a0;
 *     s32 a0;                 u8 *ent;
 * everything else identical to memory/grind/func_80032314/candidate.c.
 */
