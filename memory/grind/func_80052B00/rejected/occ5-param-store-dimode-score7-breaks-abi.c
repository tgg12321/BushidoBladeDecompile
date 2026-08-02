/* func_80052B00 — REJECTED FORM, grind session 7 (forensics, 2026-08-01).
 *
 * WHY IT IS BANKED. This is the single lowest-scoring form ever measured for
 * this function: `sandbox func_80052B00 --disable all` = **7**, against the
 * standing honest floor of 17. It is the ONLY form in seven sessions that
 * reproduces the target's exact register set $t0..$t7 on all sixteen
 * lw/ctc2 instructions AND does so at a price of only +6 emitted instructions.
 *
 * WHY IT IS REJECTED — it is not a spelling change, it is a different function.
 *   - It adds three parameters (`p1, p2, p3`) that no caller passes. On entry
 *     $a1,$a2,$a3 hold whatever the caller left there; the body then WRITES
 *     that garbage to matrix[8], matrix[9], matrix[10] — memory past the eight
 *     words the real function reads. That is live memory corruption, not a
 *     codegen hint.
 *   - It changes the return type from `void` to `long long`, so the function
 *     clobbers $v0 and $v1 that the real `void` function leaves alone.
 *   - Both changes exist for exactly one reason: to make five hard registers
 *     appear in the function's RTL so `reload1.c` will refuse to spill to them.
 *     That is a register pin expressed through the function signature — the
 *     same intent as `register T x asm("$N")`, one more spelling removed. It
 *     falls squarely under the cheats-by-any-spelling policy and must never be
 *     proposed as a form.
 *   - It cannot reach 0 in any case: the six extra instructions have no home in
 *     the target's 17, and the `jr $ra` delay slot is still `nop` (H1).
 *
 * WHAT IT PROVES (this is the reason to keep it). The register residual is NOT
 * an unreachable wall — it is a PRICED axis, and the price is score-POSITIVE:
 * paying 6 instructions to buy the right register names is worth 10 points net
 * (17 -> 7). Every prior session recorded the register axis as "unreachable";
 * the accurate statement is "reachable only by constructs that change the
 * function's contract". Full price curve in evidence.md §Session 7 and
 * tmp/grind/func_80052B00/s7/sweep_price_results.txt:
 *
 *     form                       score  build insns  ctc2 register sequence
 *     base-fused8 (the floor)      17        18      v0,v1,a1,a2,a3,t0,t1,t2
 *     ret-s32-zero                 17        18      v1,a1,a2,a3,t0,t1,t2,t3
 *     ret-dimode-zero              19        20      a1,a2,a3,t0,t1,t2,t3,t4
 *     occ3-param-store             20        21      v0,v1,t0,t1,t2,t3,t4,t5
 *     occ5-param-store-dimode       7        23      t0,t1,t2,t3,t4,t5,t6,t7  <- THIS
 *     occ5-locals-ctlE (s6 shape)  11        27      t0,t1,t2,t3,t4,t5,t6,t7
 *
 * MECHANISM (s6 + s7). `reload1.c:486` copies `regs_ever_live[]` into
 * `regs_explicitly_used[]`; `order_regs_for_reload()` (reload1.c:3606, dump
 * saved at s7/reload1_order_regs_for_reload.txt) gives every explicitly-used
 * hard register `large + 1` uses and `SET_HARD_REG_BIT (bad_spill_regs, i)`.
 * The eight asm operands are reload-generated (combine folded the loads into
 * the asm), so they take the lowest-numbered non-bad call-used registers. To
 * land on $8..$15 the function must make $2,$3,$5,$6,$7 all appear in its own
 * RTL — which is precisely what the fake return type and fake parameters do.
 */
long long func_80052B00(s32 *matrix, s32 p1, s32 p2, s32 p3) {
    s32 t0, t1, t2, t3, t4, t5, t6, t7;
    matrix[8] = p1;
    matrix[9] = p2;
    matrix[10] = p3;
    t0 = matrix[0];
    t1 = matrix[1];
    t2 = matrix[2];
    t3 = matrix[3];
    t4 = matrix[4];
    t5 = matrix[5];
    t6 = matrix[6];
    t7 = matrix[7];
    __asm__ volatile (
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\tctc2 %5, $5\n\tctc2 %6, $6\n\tctc2 %7, $7"
        :: "r"(t0), "r"(t1), "r"(t2), "r"(t3),
           "r"(t4), "r"(t5), "r"(t6), "r"(t7));
    return 0;
}
