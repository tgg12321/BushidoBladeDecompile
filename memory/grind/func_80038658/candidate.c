/* func_80038658 — MATCHED (sandbox --disable all == 0, 55/55 insns, s1 2026-08-20)
 * Exact body as applied in src/code6cac_c_mid.c. CD-load/save state-machine
 * completion handler: dispatches on D_800A31F4 (state 4 = post-read,
 * state 6 = post-write), reaps func_800378A8()'s status, closes the file
 * handle, and posts a result code to D_800A379E.
 *
 * Form notes (why this exact spelling):
 * - Natural switch + per-arm direct stores: each `D_800A379E = N;` is an
 *   independent short-lived temp -> each lands in $v0 (first-free); the
 *   shared `sh` at .L8003870C is produced by jump2 cross-jump, matching
 *   target. A single accumulator variable through a shared label instead
 *   forces one multi-def pseudo that conflicts with $v0 and lands in $v1
 *   (9-diff rename cluster) — see rejected/shared-accumulator-v1.c.
 * - `if (ret == 1) {2} else {3}`: this sense gives target's
 *   `bne s0,$v0 -> .L870C (delay li 3); j .L870C (delay li 2)`.
 *   Writing `if (ret != 1)` gets inverted by GCC into beq with 2/3 swapped.
 * - fail-code assignment INSIDE the if-arm (`if (ret==0) { fail=1; goto
 *   fail_store; }`): keeps `fail` out of the close()-path CSE block (spelled
 *   before the branch, cse reuses fail(=1) as the `ret == 1` compare constant,
 *   extending fail across close() -> callee-save s0 cascade, +3 frame insns —
 *   see rejected/fail-before-branch-cse-reuse.c). reorg's relax_delay_slots
 *   folds the arm into `beqz -> .L871C` with the li in the delay slot,
 *   exactly target.
 */
void func_80038658(void) {
    s32 ret;
    s32 fail;

    switch (D_800A31F4) {
    case 4:
        ret = func_800378A8();
        if (ret == 0) {
            fail = 1;
            goto fail_store;
        }
        close(D_800A3794);
        if (ret == 1) {
            D_800A379E = 2;
        } else {
            D_800A379E = 3;
        }
        D_800A31F4 = 0;
        return;
    case 6:
        ret = func_800378A8();
        if (ret == 0) {
            fail = 4;
            goto fail_store;
        }
        close(D_800A3794);
        if (ret == 1) {
            D_800A379E = 5;
            if (func_8003800C(&D_800F34D8) == 0) {
                D_800A379E = 0xF;
            }
        } else {
            D_800A379E = 6;
        }
        D_800A31F4 = 0;
        return;
    }
    return;

fail_store:
    D_800A379E = fail;
}
