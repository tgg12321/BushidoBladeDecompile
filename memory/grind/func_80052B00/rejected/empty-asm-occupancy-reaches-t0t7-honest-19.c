/* func_80052B00 — REJECTED FORM (grind session 10, synthesis, 2026-08-02).
 *
 * WHY IT IS BANKED. This is the FIRST form in ten sessions whose cop2 block is
 * byte-identical to the target's:  ctc2 $8,$0 / ctc2 $9,$1 / ... / ctc2 $15,$7,
 * i.e. the eight values land on $t0..$t7 with the target's exact CR mapping.
 * Sessions 3-9 recorded the register set {$2,$3,$5,$6,$7,$8,$9,$10} as invariant
 * across 19 hand spellings and ~164k permuter iterations; s6/H15 concluded that
 * BOTH allocation paths (reload's order_regs_for_reload and local-alloc's
 * find_free_reg) "draw from the same candidate pool {2,3,5,6,7,8,9,10}".
 * That conclusion is now measured to be an artifact of the controls used, not a
 * property of the compiler — see hypotheses.md H26. The pool is
 * (call-used GPRs) MINUS (registers occupied across the asm), and once
 * $2,$3,$5,$6,$7 are occupied it is exactly {$8..$15}.
 *
 * WHY IT IS REJECTED, not proposed. Every occupancy device in it is a cheat:
 *   - `__asm__ volatile ("" :: "r"(...))` — an empty-template asm with register
 *     operands has no emitted instruction and no semantic purpose; its ONLY
 *     effect is to force values into hard registers and to fence scheduling.
 *     That is the scheduling-barrier / register-occupancy family forbidden by
 *     .claude/rules/inline-asm-policy.md and gte-3x3.md, and it is the same
 *     device s6/ctlC and s7/ctlJ used as probes.
 *   - the three spare parameters p1,p2,p3 that no caller passes, plus the
 *     `long long` return on a function the target returns nothing from — the
 *     ABI-break class already rejected in s7 as
 *     rejected/occ5-param-store-dimode-score7-breaks-abi.c.
 *
 * AND IT DOES NOT EVEN SCORE. Measured this session with the honest
 * cheat-invisible sandbox (tmp/grind/func_80052B00/s10/score.py):
 *     HEAD (8 register pins)      score 18   (build 19 insns)
 *     banked pin-free fused8      score 17   (build 18 insns)   <-- the floor
 *     THIS FORM                   score 19   (build 20 insns)
 * The sandbox strips the empty asms before scoring, so the occupancy they buy
 * evaporates and only the `acc` arithmetic survives as four extra instructions.
 * The cheat is not merely inert here, it is score-NEGATIVE — one more concrete
 * illustration of the standing project invariant that cheating cannot move the
 * honest floor.
 *
 * WHAT IT COSTS EVEN AS A CHEAT. Emitted (cc1 -da, standalone TU, faithful per
 * H9): 21 instructions vs the target's 17. The eight `lw` carry the target's
 * (register, offset) pairs but in a different order, the four extra insns are
 * `move $3,$8 / sra $2,$8,31 / sra $5,$2,0 / sra $4,$2,31` (materialising the
 * long long that occupies $2/$3), and the `jr $ra` delay slot is still empty —
 * H1/H20/H24 are untouched by any of this.
 *
 * WHAT SURVIVES FOR THE NEXT SESSION. The mechanism, not the form: to put the
 * eight asm operands on $8..$15, five OTHER values must be live ACROSS the
 * fused asm so that $2,$3,$5,$6,$7 are unavailable. The open question is
 * whether any LEGAL C construct makes a value live across the asm without
 * emitting an instruction. Measured negatives so far: unused parameters do not
 * occupy at all (s6 ctlF/ctlG); an `s32` return occupies $2 only and pays for it
 * with the `jr $ra` delay slot (s7/H17, s10/retB); a `long long` return costs
 * +2 (s7) or +4 (here); volatile and empty-asm spellings are cheats.
 */
long long func_80052B00(s32 *matrix, s32 p1, s32 p2, s32 p3) {
    s32 t0,t1,t2,t3,t4,t5,t6,t7;
    long long acc = (long long)matrix[0];
    t7=matrix[7]; t6=matrix[6]; t5=matrix[5]; t4=matrix[4];
    t3=matrix[3]; t2=matrix[2]; t1=matrix[1]; t0=matrix[0];
    __asm__ volatile ("" :: "r"(p1), "r"(p2), "r"(p3));
    __asm__ volatile ("" :: "r"((s32)acc), "r"((s32)(acc >> 32)));
    __asm__ volatile (
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\tctc2 %5, $5\n\tctc2 %6, $6\n\tctc2 %7, $7"
        :: "r"(t0),"r"(t1),"r"(t2),"r"(t3),"r"(t4),"r"(t5),"r"(t6),"r"(t7));
    return acc;
}
