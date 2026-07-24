/* REJECTED — copy_end assigned inside the retry loop, before the first
 * func_800372F4 call (intent: cross the call -> callee-saved).
 * Measured sandbox --disable all = 22 (MUCH worse). Recomputing copy_end
 * each retry iteration disturbs the whole allocation/schedule. Dead. */
retry:
    func_80080258(2, index + cam_base, 0);
    copy_end = (Quad *)&sp_buf[0x40];   /* <-- inside loop: score 22 */
    v0 = ((s32 (*)())func_800372F4)(0x800, (s32)sp_buf, constant_80);
    if (v0 != 0) goto retry;
