/* REJECTED s1 (2026-07-27): split counter at the decrement.
 * Sandbox --disable all: 17 (floor 13), build 25 insns vs target 26.
 *
 * KILL MECHANISM (measured, generalizes): with the pre-decrement value
 * (a4) live in its own pseudo, combine folds the loop guard
 *   cnt = a4 - 1; if (cnt != -1)  -->  beq a4,$0  with addu cnt,a4,-1
 * in the delay slot, and the pre-loop `li -1` disappears; a frame
 * subu/addu $sp,8 also appeared. Same fold family explains the v5
 * rejection's "beqz exit": ANY form separating the mask-chain value from
 * the dec+guard variable enables the fold. Therefore the counter MUST be
 * ONE pseudo from the 0x7FFF mask through the loop (min 11 refs,
 * pri ~24400) - counter-side priority reduction below the pointer's
 * 21176 is UNREACHABLE by splitting; the flip needs pointer-side refs.
 */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 cnt;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1 - 1;
    if (a4 & 0x8000) {
        a4 = a4 & 0x7FFF;
        *(v1 - 1) = a4;
        cnt = a4 - 1;
        if (cnt != -1) {
            do {
                *v1 -= (s32)a6;
                v1++;
                cnt--;
            } while (cnt != -1);
        }
    }
}
