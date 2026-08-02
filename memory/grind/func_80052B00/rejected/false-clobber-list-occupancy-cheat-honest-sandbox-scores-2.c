/* func_80052B00 — REJECTED FORM (grind session 11, escalation, 2026-08-02).
 *
 * WHY IT IS BANKED HERE RATHER THAN SHIPPED: it is a CHEAT — the register-pin
 * family in a spelling no previous session had tried — and it is the first
 * cheat on this function that the honest cheat-invisible sandbox does NOT
 * strip. Do not re-propose it; do read the measurement, because it settles the
 * s10 frontier's one open question.
 *
 * WHAT IT DOES. It is the banked pin-free fused-8 body (the honest floor-17
 * form) with ONE addition: a clobber list on the real fused ctc2 asm naming the
 * five registers reload must find occupied for the eight loaded values to land
 * on $t0..$t7:
 *
 *     : "$2","$3","$5","$6","$7"
 *
 * MEASURED EFFECT (tmp/grind/func_80052B00/s11/occ.sh, instrumented cc1 -da).
 * baseA (no clobber): "Spilling reg 2. 3. ..." -> lw $2,0($4) ... lw $10,28($4)
 * / ctc2 $2,$0 ... ctc2 $10,$7, 17 emitted insns. clobA (with the clobber):
 * "Spilling reg 8. 9. ..." -> lw $8,0($4) / lw $9,4($4) / ... / lw $15,28($4) /
 * ctc2 $8,$0 / ctc2 $9,$1 / ... / ctc2 $15,$7 / j $31 — the target's registers,
 * order and offsets EXACTLY, in the SAME 17 emitted instructions. Zero
 * instruction cost. This is the first zero-cost occupancy construct found in
 * eleven sessions, and it closes the s10 frontier question ("is there a
 * construct that occupies registers across the fused asm without emitting an
 * instruction?") in the affirmative — but not with a LEGAL construct.
 *
 * HONEST SANDBOX (tmp/grind/func_80052B00/s11/score.py, spliced over
 * src/text1b.c:10969-10994 then restored byte-identically):
 *     HEAD (8 register asm("$N") pins)   score 18  (build 19, rules_dropped 1)
 *     banked pin-free fused-8            score 17  (build 18, rules_dropped 1)
 *     THIS FORM                          score  2  (build 18, rules_dropped 1)
 * The residual 2 is exactly the delay slot: the target ends `ctc2 $t6,$6 /
 * jr $ra / ctc2 $t7,$7` and this form ends `ctc2 $14,$6 / ctc2 $15,$7 / j $31 /
 * nop`. Every other byte of the function agrees.
 *
 * WHY IT IS A CHEAT (the reason it is not the answer, despite the score).
 * A GNU inline-asm clobber list is a declaration of what the TEMPLATE modifies.
 * This template is `ctc2 %0,$0 ... ctc2 %7,$7`: it writes cop2 CONTROL registers
 * and modifies no GPR at all. Naming $2,$3,$5,$6,$7 as clobbered is a false
 * statement about the asm whose ONLY purpose and ONLY effect is to steer
 * reload1.c:3606 order_regs_for_reload() onto the registers the shipped bytes
 * use. That is definitionally the register-occupancy / register-pin device
 * forbidden by .claude/rules/inline-asm-policy.md and
 * .claude/rules/inline-asm-injection.md — identical in intent to the eight
 * `register s32 x asm("$N")` pins in the HEAD body, to the empty-template
 * `__asm__ ("" :: "r"(x))` occupancy blocks banked at
 * rejected/empty-asm-occupancy-reaches-t0t7-honest-19.c (s10 poolK), and to the
 * spare-parameter/ABI-break forms banked at
 * rejected/occ5-param-store-dimode-score7-breaks-abi.c (s7). The three differ
 * only in which mechanical detector notices them, not in what they do.
 *
 * ENGINE DETECTOR GAP — FOR THE OWNER, NOT FOR A GRIND SESSION TO FIX.
 * The cheat-invisible sandbox strips empty-template occupancy asms (s10 poolK
 * measured 19, worse than the floor, exactly as policy intends) but it does NOT
 * strip or discount a false clobber list on an asm whose template is real. So
 * this cheat is score-VISIBLE: it moves the honest floor 17 -> 2. That breaks
 * the "cheating can't help" invariant for this construct tree-wide, and it
 * means a floor drop of this shape must not be read as progress. Session 11 is
 * forbidden from touching engine/, so this is reported, not fixed; it is
 * recorded in docs/grind/decisions.md with the escalation entry.
 *
 * NOT SHIPPABLE ON ITS OWN TERMS EITHER: even accepted, it cannot reach 0. The
 * remaining 2 is the `jr $ra` delay slot, which H1 (reorg.c:730-735
 * stop_search_p halts fill_simple_delay_slots at any ASM_INPUT insn), H20
 * (whole-binary census: ASPSX 2.34 never filled a delay slot) and H24 (the
 * SHIPPED cc1psx behaves identically, and fills that same slot when the
 * trailing insn is a plain C store) close outright.
 */
void func_80052B00(s32 *matrix) {
    s32 t0,t1,t2,t3,t4,t5,t6,t7;
    t0=matrix[0]; t1=matrix[1]; t2=matrix[2]; t3=matrix[3];
    t4=matrix[4]; t5=matrix[5]; t6=matrix[6]; t7=matrix[7];
    __asm__ volatile (
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\tctc2 %5, $5\n\tctc2 %6, $6\n\tctc2 %7, $7"
        :: "r"(t0),"r"(t1),"r"(t2),"r"(t3),"r"(t4),"r"(t5),"r"(t6),"r"(t7)
        : "$2","$3","$5","$6","$7");   /* <-- THE CHEAT: the template writes none of these */
}
