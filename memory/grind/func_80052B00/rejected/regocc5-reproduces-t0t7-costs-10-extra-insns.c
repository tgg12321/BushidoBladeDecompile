/* func_80052B00 — REJECTED FORM (grind session 6, forensics, 2026-08-01).
 *
 * WHY IT IS BANKED. This is the ONLY C form measured in six sessions that
 * reproduces the target's register naming EXACTLY. Compiled with the
 * instrumented cc1 (tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -mcpu=3000
 * -mips1 -mno-abicalls -fno-builtin -da) it emits
 *
 *     lw   $8,0($4)   lw $9,4($4)   lw $10,8($4)  lw $11,12($4)
 *     lw   $12,16($4) lw $13,20($4) lw $14,24($4) lw $15,28($4)
 *     ctc2 $8,$0  ctc2 $9,$1  ctc2 $10,$2 ctc2 $11,$3
 *     ctc2 $12,$4 ctc2 $13,$5 ctc2 $14,$6 ctc2 $15,$7
 *
 * $8..$15 IS $t0..$t7 — the target's register set, in the target's order, with
 * the target's memory offsets. Sessions 1-5 concluded from 19 hand spellings and
 * ~163k permuter iterations that the register set was immovable; that conclusion
 * was right about every form ANYONE HAD TRIED and wrong about the mechanism. The
 * set is movable. It just costs instructions the target does not have.
 *
 * WHY IT IS DEAD. The lever is register OCCUPANCY, not spelling. reload1.c:3606
 * `order_regs_for_reload()` builds `potential_reload_regs` from the hard regs
 * with zero uses, call-used first, in ASCENDING REGNO order (the `#else` branch —
 * `tools/gcc-2.7.2/config/mips/mips.h` defines no `REG_ALLOC_ORDER`). Registers
 * already live or explicitly used are pushed to the back (reload1.c:486 copies
 * `regs_ever_live` into `regs_explicitly_used`; :3651 adds `large+1` uses and
 * sets `bad_spill_regs`). In the honest 17-instruction body nothing occupies
 * $2,$3,$5,$6,$7, so reload's first eight spill choices are
 * {2,3,5,6,7,8,9,10} = {v0,v1,a1,a2,a3,t0,t1,t2}. To make the first eight choices
 * be {8..15} you must occupy exactly $2,$3,$5,$6,$7 — five simultaneously live
 * values — across the asm.
 *
 * Five live values cost five defining instructions and (to keep them live past
 * the asm) five consuming instructions: this form emits 5 extra `lw` + 5 extra
 * `sw` = 10 instructions on top of the target's 17. The target function is
 * exactly 17 instructions (8 lw + 8 ctc2 + jr). There is no room, and no C
 * construct produces a live value at zero instruction cost. The zero-cost
 * candidate was tested and killed the same session: extra UNUSED register
 * parameters (`s32 p1, s32 p2, s32 p3` arriving in $a1-$a3) do not set
 * `regs_ever_live`, so reload's spill list came back unchanged as
 * {2,3,5,6,7,8,9,10} (tmp/grind/func_80052B00/s6/ctlF_out.txt).
 *
 * So the register residual is now closed by ARITHMETIC rather than by absence of
 * evidence: the register set is reachable only from a state that adds >= 10
 * instructions to a body that must not grow by one.
 *
 * COLLATERAL SIGNIFICANCE FOR PROVENANCE. GCC cannot put these eight values in
 * $t0..$t7 while emitting only these seventeen instructions. The target does
 * both. That is affirmative evidence that the target's register names were not
 * produced by a compiler allocating registers for these loads — consistent with
 * the PsyQ libgte cop2-macro provenance already recorded for this family
 * (SetRotMatrix + SetTransMatrix fusion) and with the canonical-asm disposition
 * carried since session 1. It is evidence for the owner, not a self-authorization.
 *
 * Raw data: tmp/grind/func_80052B00/s6/ctlE_out.txt (this form),
 * ctlD_out.txt (the two-parameter variant that lands on $9..$15,$24 and shows the
 * ordering is positional, not magic), controls_out.txt, dumps/ctlE.c.greg.
 */
typedef signed int s32;
void func_80052B00_REJECTED_regocc5(s32 *matrix) {
    s32 a, b, c, d, e;
    s32 t0, t1, t2, t3, t4, t5, t6, t7;
    a = matrix[8];  b = matrix[9];  c = matrix[10];
    d = matrix[11]; e = matrix[12];
    t0 = matrix[0]; t1 = matrix[1]; t2 = matrix[2]; t3 = matrix[3];
    t4 = matrix[4]; t5 = matrix[5]; t6 = matrix[6]; t7 = matrix[7];
    __asm__ volatile (
        "ctc2 %0, $0\n\t"
        "ctc2 %1, $1\n\t"
        "ctc2 %2, $2\n\t"
        "ctc2 %3, $3\n\t"
        "ctc2 %4, $4\n\t"
        "ctc2 %5, $5\n\t"
        "ctc2 %6, $6\n\t"
        "ctc2 %7, $7"
        :: "r"(t0), "r"(t1), "r"(t2), "r"(t3),
           "r"(t4), "r"(t5), "r"(t6), "r"(t7));
    matrix[8] = a;  matrix[9] = b;  matrix[10] = c;
    matrix[11] = d; matrix[12] = e;
}
