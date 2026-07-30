/* candidate.c - replay_camera_rob_back_loose3, session 3 (structural).
 *
 * CHEAT-FREE.  engine `sandbox --disable all` = 12  (s2 banked 13; s1's honest
 * cheat-free baseline was 26; HEAD's cheat-carrying form scored 17 only via a
 * register pin + a volatile coercion, both absent here).
 * build_insns 114 == target_insns 114.  Zero rules, zero pins, zero volatile,
 * zero inline asm.
 *
 * ------------------------------------------------------------------ HISTORY
 * s2's form (score 13) = the two changes that closed MECHANISM A:
 *   1. cosA is read through a `u16 rawA` staging local and sign-extended with
 *      an explicit (s16) cast (the sanctioned narrow-view spelling), and
 *   2. `a1[5] = -sinA;` is MOVED to sit BETWEEN the rawA load and that cast.
 *      Combine's can_combine_p refuses to combine a MEM load into a later user
 *      across an insn that may WRITE memory, so simplify_shift_const never
 *      sees the ashiftrt(ashift(zero_extend(mem),16),16) chain it would fold
 *      into sign_extend(mem) = `lh`.  Target's three-insn shape
 *      (lui at,%hi(Judge); addu at,at,a3; lhu v0,%lo(Judge)(at); sll 16; sra 16)
 *      therefore survives, at zero instruction cost, because sched1 runs after
 *      combine and hoists the store back out.
 *      Both halves are load-bearing: the store after the cast is inert, and the
 *      same interleave without the u16 staging local is inert (that load is
 *      already a sign_extend MEM, so there is no zero_extend to protect).
 *
 * ------------------------------------------------------- WHAT s3 ADDED (13 -> 12)
 * THE ONE CHANGE vs s2's candidate:  `angC = a0[2]; sinC = Judge[angC & 0xFFF];`
 * is moved from BEFORE the `sinAxsinB_12 = (sinA * sinB) >> 12;` statement to
 * AFTER it.
 *
 * Mechanism.  `a0[2]` is the LAST use of the parameter pointer `a0`, so its
 * position decides where the hard register $a0 dies.  Reading it after the
 * sinA*sinB statement keeps $a0 live across the multiply and, more importantly,
 * puts the angC load AFTER the mult in the pre-allocation insn stream.  Local
 * alloc then hands angC target's $v1 and its `& 0xFFF`/`<< 1` cos-index temp
 * target's $v0 -- the mirror-image swap that s1 and s2 both recorded as
 * unreachable is CLOSED, and the build now reproduces target's
 * `lhu v1,0x4(a0)` at insn 18 verbatim.
 *
 * s2 had only ever moved this read EARLIER (hoisting it next to angA/angB, which
 * measured 26 flat / 43 when the index chain went with it).  Moving it LATER was
 * the untried direction.  Reading it later still (after the whole cosA block,
 * variant m3) is much worse (90), and the equivalent delay applied to a0[1]
 * instead is inert (13) -- it is specifically a0[2]-as-last-use that matters.
 *
 * ------------------------------------------------------------ REMAINING GAP (12)
 * The residual is `sinAxsinB_12`: target keeps it in $v0, our build puts it in
 * $a0 (the parameter register, free once a0 dies).  It is a SCHEDULING-coupled
 * allocation, and s3 measured the coupling precisely:
 *   - target's stream is  `mult t2,t3` @17 ... `mflo t0` @22 ... `sra v0,t0,12` @26,
 *     i.e. mflo/sra are delayed past the cosC index chain, which occupies $v0 at
 *     insns 20-24 and DIES at 24; $v0 is then free for sinAxsinB_12 from 26 on.
 *   - this form schedules `mflo v0` @19 / `sra a0,v0,12` @20, i.e. BEFORE the
 *     index chain, so sinAxsinB_12's live range overlaps the temp's and $v0 is
 *     unavailable; local-alloc gives it the leftover $a0.
 *   - splitting the multiply from the shift (`sinAxsinB = sinA * sinB;` ... then
 *     `sinAxsinB_12 = sinAxsinB >> 12;` after the sinC read) reproduces target's
 *     SCHEDULE exactly (mflo t0 @22, sra @26) but flips angC back to $v0 and
 *     idxC to $v1, scoring 13.  The two halves have not been obtained together
 *     by any of the 30 spellings measured across s2+s3.
 */
extern s16 Judge[];
void replay_camera_rob_back_loose3(u16 *a0, s16 *a1) {
    s32 angA, angB;
    s16 sinA, sinB, sinC;
    s16 cosB, cosC;
    s32 sinAxsinB_12;
    s32 prod_sinC, prod_cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 cosA_sinB, cosA_sinC, cosA_cosC, cosA_cosB;
    s32 sinAxcosB;
    s32 sinAxcosB_12;
    s32 scb_sinC, neg_sinB_cosC;
    s32 scb_cosC;
    s32 sinB_sinC;
    s32 cosA;
    u16 rawA;
    s32 angC;

    angA = a0[0];
    angB = a0[1];

    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];

    sinAxsinB_12 = (sinA * sinB) >> 12;

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    prod_sinC = sinAxsinB_12 * sinC;

    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    prod_cosC = sinAxsinB_12 * cosC;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[5] = -sinA;
    cosA = (s16)rawA;

    cosB_negsinC = cosB * -sinC;

    cosA_sinB = cosA * sinB;
    cosA_sinC = cosA * sinC;

    sinAxcosB = sinA * cosB;
    cosA_cosC = cosA * cosC;

    sinAxcosB_12 = sinAxcosB >> 12;

    scb_sinC = sinAxcosB_12 * sinC;
    neg_sinB_cosC = -sinB * cosC;

    scb_cosC = sinAxcosB_12 * cosC;

    cosA_cosB = cosA * cosB;

    a1[2] = cosA_sinB >> 12;
    a1[3] = cosA_sinC >> 12;
    a1[4] = cosA_cosC >> 12;
    a1[8] = cosA_cosB >> 12;
    a1[0] = (prod_sinC + cosB_cosC) >> 12;
    a1[1] = (prod_cosC + cosB_negsinC) >> 12;
    a1[6] = (scb_sinC + neg_sinB_cosC) >> 12;

    sinB_sinC = sinB * sinC;
    a1[7] = (scb_cosC + sinB_sinC) >> 12;
}
