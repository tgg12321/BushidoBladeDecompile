/* REJECTED s1: retired-chassis body ported (volatile Judge cast -> u16 rawA stage for cosA only).
 * sandbox --disable all = 53; frame 56 (vars=48, one narrow local too many), only cosA got the
 * lhu+sll+sra shape; sinA/sinB/sinC folded to lh. Superseded by candidate.c. */
void func_80042C80(u16 *a0, s16 *a1) {
    s32 angB, angC;
    s16 cosB, cosC;
    s32 cosB_cosC, cosB_negsinC;
    s32 angA;
    s16 sinB;
    s16 sinA;
    s16 sinC;
    s32 sab;
    s32 sab12;
    s32 sab12_cosC, sab12_negsinC;
    s32 cosA;
    s32 cosA_sinC, cosA_cosC;
    s32 cosA_negsinB;
    s32 negsinA_cosB;
    s32 cab12, cab12_cosC;
    s32 cosA_sinB;
    s32 sinA_sinC;
    s32 csb12_sinC;
    s32 cosA_cosB;
    s32 sinA_cosC;
    u16 rawA;

    angB = a0[1];
    angC = a0[2];

    sinB = Judge[angB & 0xFFF];
    sinC = Judge[angC & 0xFFF];
    angA = a0[0];
    sinA = Judge[angA & 0xFFF];

    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];

    cosB_cosC = cosB * cosC;

    sab = sinA * sinB;

    cosB_negsinC = cosB * -sinC;
    sab12 = sab >> 12;

    sab12_cosC = sab12 * cosC;

    rawA = Judge[((s16)angA + 0x400) & 0xFFF];
    a1[2] = sinB;
    cosA = (s16)rawA;

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
