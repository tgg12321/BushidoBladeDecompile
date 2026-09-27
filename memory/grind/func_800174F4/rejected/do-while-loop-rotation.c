/* REJECTED — func_800174F4, session 1.
 * WHY DEAD: floor 25 -> 34 (build_insns 135). Spelling the case-1/2 loop as a
 * do-while makes GCC rotate it differently AND emit `andi a0,a0,0xffff` (the
 * unsigned-short limit stops being provably narrow as the loop condition
 * operand); the limit also leaves the callee-saves entirely. Target's shape is
 * the goto-form bottom-tested loop with a separate entry guard.
 * Do not re-propose.
 */
            s32 v0;
            s0_var = 0;
            v0 = rand();
            v0 &= 3;
            s1_var = v0 + 4;
            if (s1_var != 0) {
                do {
                    s0_var++;
                    s2_var = (s32)func_8005D554((u8 *)s2_var, g_disp_enable);
                } while (s0_var < s1_var);
            }
