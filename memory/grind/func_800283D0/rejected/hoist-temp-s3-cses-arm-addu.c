/* REJECTED (s2, 2026-08-26) — floor 28 -> 63, build_insns 215 -> 214.
 * Idea: lengthen pseudo 143 (temp_s3)'s reg_live_length from 14 to ~20 so
 * global.c allocno_compare drops its priority below pseudo 73 (the arg1 home)
 * and the s2/s3 assignment flips to target's. Executed by hoisting the
 * temp_s3 definition above the `if (temp_v1_3 == 0)` branch.
 * MEASURED: cse then folds the ==0 arm's own `arg0 + temp_a1_2*2 + 0x288`
 * address into temp_s3 (target keeps a SEPARATE `addu v0,v0,s0` there), the
 * pointer pseudo is renumbered away entirely (143 disappears from the lreg
 * report), and the whole callee-saved assignment re-shuffles: 72 moves s0->s1,
 * 77 moves s1->s2. Catastrophic. Do NOT hoist temp_s3 across that branch. */
                    s16 temp_v1_3 = *(s16 *)(temp_s4 + temp_s5 * 2 + 0x288);
                    u8 *temp_s3 = arg0 + (temp_a1_2 * 2);
                    if (temp_v1_3 == 0) {
