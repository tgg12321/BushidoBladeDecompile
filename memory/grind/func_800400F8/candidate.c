/* func_800400F8 — s2 candidate. Sandbox distance 0 (measured s2, 2026-07-14).
 *
 * STATUS: PENDING RULING — this is the judge-banned construct family
 * (pre-comparison `s0 = 0;` + `if (s2[0] > s0)`) with the dead duplicate
 * `s0 = 0;` removed, so every statement is live. Do NOT apply as a
 * completion until the s2 ruling-request is answered. Mechanism notes in
 * evidence.md [s2]: the variable-compare's combine-leftover pseudo produces
 * the target's 0x28 frame (phantom slot sp+20); the if+do-while placement is
 * the only structure that keeps the beqz delay slot nop (may_trap_p barrier).
 */
void func_800400F8(s32 *a0) {
    s16 *s2;
    s16 *s1;
    s32 s0;
    s2 = (s16 *)a0[9];
    if (s2 != 0) {
        s0 = 0;
        if (s2[0] > s0) {
            s1 = s2;
            do {
                obj_Clear(s1[4]);
                s1 = (s16 *)((s32)s1 + 0xD0);
                s0++;
            } while (s0 < s2[0]);
        }
    }
}
