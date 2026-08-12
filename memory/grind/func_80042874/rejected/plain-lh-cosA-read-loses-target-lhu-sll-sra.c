/* REJECTED — func_80042874, session 1.
 * WHY DEAD: reading cosA directly (`cosA = Judge[((s16)angA + 0x400) & 0xFFF];`)
 * instead of staging it through a `u16 rawA` and casting `(s16)rawA` lets combine
 * fold the zero_extend/shift chain into a single sign_extend, so GCC emits one `lh`
 * where the target emits `lhu` + `sll 16` + `sra 16` (target insns 57/59/61).
 * MEASURED: sandbox --disable all = 6, build_insns 116 vs target 119 (three insns
 * short).  Everything else in the body was the scoring-0 form.
 * Do not re-propose: the staging local is load-bearing and its emitted effect is
 * present in the ORIGINAL bytes.
 */
    /* ... identical to candidate.c except: */
    cosA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[7] = sinA;
    /* (no `u16 rawA;` declaration, no `cosA = (s16)rawA;`) */
