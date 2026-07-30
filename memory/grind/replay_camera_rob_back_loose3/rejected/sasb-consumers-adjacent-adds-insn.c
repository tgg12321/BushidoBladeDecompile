/* REJECTED -- sasb-consumers-adjacent-adds-insn
 *
 * s3: prod_sinC and prod_cosC made adjacent to shorten sinAxsinB_12's live range from 17
 * insns to ~4 (raising its qty_compare_1 priority from 0.176 so it would be allocated
 * early enough to claim $v0 instead of the leftover $a0).  25 / 115 -- it costs an
 * instruction.  The same with the shift written inline at both uses (r5) is also 25 / 115.
 * Writing the shift inline at both uses WITHOUT making them adjacent (r1/r2) is 13 / 114.
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
    s32 sinAxsinB;
    s32 idxB, idxC;

    angA = a0[0];
    angB = a0[1];
    sinA = Judge[angA & 0xFFF];
    sinB = Judge[angB & 0xFFF];
    sinAxsinB_12 = (sinA * sinB) >> 12;
    angC = a0[2];
    sinC = Judge[angC & 0xFFF];
    cosB = Judge[((s16)angB + 0x400) & 0xFFF];
    cosC = Judge[((s16)angC + 0x400) & 0xFFF];
    prod_sinC = sinAxsinB_12 * sinC;
    prod_cosC = sinAxsinB_12 * cosC;
    cosB_cosC = cosB * cosC;
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
