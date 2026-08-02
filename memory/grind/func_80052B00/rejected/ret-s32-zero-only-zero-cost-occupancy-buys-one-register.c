/* func_80052B00 — REJECTED FORM, grind session 7 (forensics, 2026-08-01).
 *
 * THE ONLY ZERO-COST REGISTER-OCCUPANCY CONSTRUCT THAT EXISTS FOR THIS BODY,
 * and it is worth exactly one register position — which is worth zero points.
 *
 * The s6 frontier named four untested constructs that might occupy
 * $2,$3,$5,$6,$7 without emitting an instruction (a $v0 return value, a DImode
 * return, setjmp, alloca). s7 measured all four plus varargs
 * (tmp/grind/func_80052B00/s7/ctl_occupancy.sh, dumps in s7/dumps/):
 *
 *   ctlH  s32 return 0          18 cc1 insns  regs $3,$5,$6,$7,$8,$9,$10,$11
 *         -> occupies $2 ONLY, and `move $2,$0` is absorbed by the previously
 *            empty `jr $ra` delay slot, so the emitted length is UNCHANGED.
 *            This is the zero-cost construct. It shifts the register window by
 *            exactly one position and the sandbox score stays 17.
 *   ctlI  long long return 0    19 insns      regs $5..$12   (+2, score 19)
 *   ctlJ  varargs `(s32*, ...)` 21 insns      regs $2,$3,$5..$10 — UNCHANGED.
 *         setup_incoming_varargs' `sw $5/$6/$7` homing stores do NOT put those
 *         registers into regs_ever_live in time for order_regs_for_reload, so
 *         varargs is dead as an occupancy lever AND costs +4. This kills the
 *         [[fake-varargs-explicit-homing]] idea for this function outright.
 *   ctlK  DImode return + varargs  23 insns   regs $5..$12 — varargs adds
 *         nothing on top of the DImode return.
 *   ctlL  setjmp                38 insns — the call destroys the leaf; all
 *         eight values are spilled to the stack across it.
 *   ctlM  alloca                45 insns — forces a frame pointer and pushes
 *         the eight values onto callee-saved $16..$23 with full save/restore.
 *
 * WHY EVEN ctlH IS REJECTED. func_80052B00 returns `void`; giving it an `s32`
 * return value that no caller reads is a fake signature whose only purpose is
 * to make $v0 appear in the function's RTL so reload will not spill to it —
 * a register pin expressed through the return type. It also puts `move $2,$0`
 * in the delay slot, where the target holds `ctc2 $t7, $7`, so it does not even
 * help: score 17, identical to the honest floor-17 body it is derived from.
 *
 * NET RESULT FOR THE LEDGER: the four constructs the s6 frontier flagged as the
 * last untested corner of the register axis are now all measured. Zero-cost
 * occupancy exists for exactly ONE of the five registers that must be occupied,
 * and every construct that occupies the other four emits at least one
 * instruction. See also
 * rejected/occ5-param-store-dimode-score7-breaks-abi.c (the priced form that
 * does reach $t0..$t7) and hypotheses.md H16 (why the target is therefore not a
 * fixed point of this compiler configuration at all).
 */
s32 func_80052B00(s32 *matrix) {
    s32 t0, t1, t2, t3, t4, t5, t6, t7;
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
