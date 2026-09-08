/* candidate.c - func_80042C80, session 1 (recon).
 *
 * MATCHED. engine `sandbox --disable all` = 0 (build_insns 122 == target_insns 122),
 * verify-oracle ok (build SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa) with this
 * body in place in src/text1a_c.c. Zero rules, zero pins, zero volatile, zero asm,
 * zero FAKE constructs. Plain C in the same shape as the sibling func_80042874
 * (Judge PASS 2026-08-12, docs/grind/decisions.md:4905).
 *
 * Structure (measured, see evidence.md s1):
 *  - sinA/sinB/sinC/cosA are s16 locals whose first arithmetic use comes AFTER the
 *    `a1[2] = sinB;` store -> combine cannot fold lhu+extend across the store
 *    (combine.c:914-917 use_crosses_set_p on a MEM src) -> lhu; sll 16; sra 16, as in
 *    the target. cosB/cosC are s32 (extend at the load, no HI pseudo) -> lh.
 *  - All six Judge loads sit BEFORE the store in source; sched1 then sinks the store
 *    to the tail exactly as the target shows (Judge loads may-alias a1[], so a store
 *    placed before the cosB/cosC loads pins them below it: rejected form score 83).
 *  - Frame: vars=40 regs=2 (s1 saved but unused: reload spill-pool ever-live mark,
 *    reload1.c:2322) - both fall out of this body; the retired-chassis body had
 *    vars=48 (6 narrow locals incl. a u16 rawA stage) and scored 53.
 */
void func_80042C80(u16 *a0, s16 *a1) {
    s32 angA, angB, angC;
    s16 sinA, sinB, sinC, cosA;
    s32 cosB, cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 sab, sab12;
    s32 sab12_cosC, sab12_negsinC;
    s32 cosA_sinC, cosA_cosC;
    s32 cosA_negsinB;
    s32 negsinA_cosB;
    s32 cab12, cab12_cosC;
    s32 cosA_sinB;
    s32 sinA_sinC;
    s32 csb12_sinC;
    s32 cosA_cosB;
    s32 sinA_cosC;

    angB = a0[1];
    angC = a0[2];
    sinB = Judge[angB & 0xFFF];
    angA = a0[0];
    sinA = Judge[angA & 0xFFF];
    sinC = Judge[angC & 0xFFF];
    cosA = Judge[((s16)angA + 0x400) & 0xFFF];

    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;
    a1[2] = sinB;
    sab = sinA * sinB;
    cosB_negsinC = cosB * -sinC;
    sab12 = sab >> 12;
    sab12_cosC = sab12 * cosC;
    cosA_sinC = cosA * sinC;
    sab12_negsinC = sab12 * -sinC;
    cosA_cosC = cosA * cosC;
    cosA_negsinB = cosA * -sinB;
    negsinA_cosB = -sinA * cosB;
    cab12 = cosA_negsinB >> 12;
    cab12_cosC = cab12 * cosC;
    cosA_sinB = cosA * sinB;
    sinA_sinC = sinA * sinC;
    csb12_sinC = (cosA_sinB >> 12) * sinC;
    cosA_cosB = cosA * cosB;

    a1[0] = cosB_cosC >> 12;
    a1[1] = cosB_negsinC >> 12;
    a1[5] = negsinA_cosB >> 12;
    sinA_cosC = sinA * cosC;
    a1[8] = cosA_cosB >> 12;
    a1[3] = (sab12_cosC + cosA_sinC) >> 12;
    a1[4] = (sab12_negsinC + cosA_cosC) >> 12;
    a1[6] = (cab12_cosC + sinA_sinC) >> 12;
    a1[7] = (csb12_sinC + sinA_cosC) >> 12;
}
