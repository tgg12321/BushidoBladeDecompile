/* candidate.c - replay_camera_rob_back_loose3, session 2 (structural).
 *
 * CHEAT-FREE.  engine sandbox --disable all = 13  (was 26 cheat-free at s1;
 * HEAD's cheat-carrying form scored 17 but only via a register pin + a
 * volatile coercion, both removed here).  build_insns 114 == target_insns 114.
 *
 * WHAT CHANGED vs the s1 cheat-free baseline (rejected/nocheat-baseline-floor26.c):
 *   1. cosA is read through a `u16 rawA` staging local and sign-extended with
 *      an explicit (s16) cast -- the sanctioned narrow-view spelling.
 *   2. `a1[5] = -sinA;` is MOVED from the tail of the function to sit BETWEEN
 *      the rawA load and the (s16)rawA cast.
 *
 * WHY (2) IS LOAD-BEARING, and why (1) alone is not:
 * GCC 2.7.2 combine (simplify_shift_const) rewrites
 *   ashiftrt(ashift(zero_extend(mem),16),16)  ->  sign_extend(mem)   [= `lh`]
 * which is why s1 measured spelling (1) on its own -- and three other narrow-view
 * spellings -- at a flat 26 with a single `lh`.  s2 further measured that giving
 * rawA a real, flow-live SECOND use does NOT stop the fold (a low-half-only use
 * such as `a1[9] = rawA` folds anyway; even `a1[9] = rawA >> 8`, which needs the
 * zero-extended high bits, kept the `lh` and re-derived the unsigned value with
 * andi/srl).  What DOES stop it is combine's own memory-safety rule: can_combine_p
 * refuses to combine a MEM load into a later user when an intervening insn may
 * WRITE memory.  Putting any store between the load and the cast therefore leaves
 * target's three-instruction shape intact:
 *     lui at,%hi(Judge) ; addu at,at,a3 ; lhu v0,%lo(Judge)(at)
 *     sll v0,v0,16 ; sra v0,v0,16 ; mult v0,...
 * and it costs nothing, because sched1 runs AFTER combine and hoists the store
 * back out (target's own schedule fills that load-delay slot with `mflo t9`).
 * The store placed AFTER the cast (e6) is inert, confirming the position -- not
 * the mere presence -- of the store is the mechanism.
 *
 * The move is semantics-preserving: a1 (the output matrix) never aliases the
 * static sin/cos table Judge, and the relative order of the a1[] stores among
 * themselves is unchanged.
 *
 * REMAINING GAP -- 13 diffs, all mechanism B, register naming only, fully
 * mirror-image:  angC target $v1 / build $v0;  angC's index temp target $v0 /
 * build $v1;  sinAxsinB_12 target $v0 / build $a0.  No instruction differences
 * remain anywhere in the function.
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

    angC = a0[2];
    sinC = Judge[angC & 0xFFF];

    sinAxsinB_12 = (sinA * sinB) >> 12;

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
