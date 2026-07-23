/* func_800692C0 — PURE-C MATCH. sandbox --disable all = 0 (67/67 insns,
 * rules_dropped=1: matches WITHOUT the regfix rule). Found by permuter s4.
 * Two sanctioned pure-C match devices close the s2/s3 floor-9 wall:
 *   (1) single-level do{}while(0) wrap around the preheader+loop+return flips
 *       the sum/bitpos RA tie to target (sum=$t2/$10, bitpos=$t1/$9) — the
 *       "unreachable in pure C" wall. do-while(0) is sanctioned for ANY codegen
 *       effect incl. RA (owner ruling 2026-07-06, [[do-while-zero-exception]]).
 *   (2) `s32 one; one = 1;` constant-holder SHARED across both `1` uses
 *       (*arg3 = one; sum += one << bitpos;) materializes the LICM shift operand
 *       `1` (const1 / `li $t6,1`) at the preheader = target's early slot, closing
 *       the last 2-insn residual. The SOTN `s32 one=1;` named-constant class
 *       ([[named-local-fake-exception]] / [[loop-rotation-two-shift]]),
 *       semantically-true C. Sharing across BOTH uses keeps it at 67 insns
 *       (using it on only one `1` splits into two `li` -> 68 insns / score 3).
 * FAKE-annotated at the construct site. */
s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 one;
    s32 sum;
    s32 i;
    s32 a3_off;
    s32 bitpos;
    u32 *p;
    u32 maskA;
    u32 maskB;
    u32 v;
    s32 c;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    one = 1;
    /* FAKE: single-level do{}while(0) wrap around the loop preheader+body seats
     * sum in $t2 and bitpos in $t1 (target's allocno tie), flipping the RA the
     * clean loop otherwise inverts. The `one` constant-holder materializes the
     * shift operand `1` at the preheader (schedules `li $t6,1` early, target's
     * slot). Pure-C match device (permuter s4). */
    do {
        a3_off = 0;
        bitpos = 0;
        p = &D_800A32D0;

        do {
            s32 idx4;
            arg3 = (s16 *)((s32)arg3 + a3_off);
            v = i * 4;
            maskB = *p << arg1;
            idx4 = v;
            maskA = *(u32 *)((s32)&D_800A32C8 + idx4) << arg1;
            if (*arg2 == 0) {
                v = *arg0;
                if (v & maskA) {
                    *arg3 = one;
                } else if (v & maskB) {
                    c = -1;
                    *arg3 = c;
                }
            } else {
                v = *arg0;
                if (v & maskA) {
                    c = 6;
                    *arg2 = c;
                } else if (v & maskB) {
                    c = -6;
                    *arg3 = c;
                }
                sval = *arg2;
                if (sval >= 6) {
                    sum += one << bitpos;
                    *arg3 = 0;
                    *arg2 = 0;
                } else if (sval < -5) {
                    c = 2;
                    sum += c << bitpos;
                    *arg3 = 0;
                    *arg2 = 0;
                }
            }
            a3_off += 2;
            bitpos += 0x10;
            p++;
            i++;
            *arg2 = (u16)*arg2 + (u16)*arg3;
            arg2++;
        } while (i < 2);

        return sum;
    } while (0);
}
